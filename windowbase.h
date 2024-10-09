#pragma once

#include <atomic>
#include <chrono>
#include <concepts>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#include <windows.h>

namespace btui {
    static constexpr inline uint32_t FromRGB(uint8_t R, uint8_t G, uint8_t B) {
        return (uint32_t)R << 16 | (uint32_t)G << 8 | (uint32_t)B;
    }
    static constexpr inline uint32_t FromARGB(uint8_t A, uint8_t R, uint8_t G, uint8_t B) {
        return (uint32_t)A << 24 | (uint32_t)R << 16 | (uint32_t)G << 8 | (uint32_t)B;
    }

    static constexpr inline uint32_t OverlayColor(uint32_t Bottom, uint32_t Top) {
        constexpr uint32_t maskA = 0xFF000000;
        constexpr uint32_t maskR = 0x00FF0000;
        constexpr uint32_t maskG = 0x0000FF00;
        constexpr uint32_t maskB = 0x000000FF;

        uint32_t bottomA = Bottom & maskA;
        uint32_t bottomR = Bottom & maskR;
        uint32_t bottomG = Bottom & maskG;
        uint32_t bottomB = Bottom & maskB;

        uint32_t topA = Top & maskA;
        uint32_t topR = Top & maskR;
        uint32_t topG = Top & maskG;
        uint32_t topB = Top & maskB;

        uint32_t topAlpha = topA >> 24;
        uint32_t topAlphaInv = 0xFF - topA;

        bottomR *= topAlphaInv;
        bottomG *= topAlphaInv;
        bottomB *= topAlphaInv;

        topR *= topAlpha;
        topG *= topAlpha;
        topB *= topAlpha;

        uint32_t outColor = (
            ((bottomR + topR) & maskA) |
            ((bottomG + topG) & maskR) |
            ((bottomB + topB) & maskG)
            ) >> 8;

        outColor |= (topA + bottomA * ((maskA - topA) >> 8)) & maskA;

        return outColor;
    }

    struct BufferGridCell {
        wchar_t character;
        uint32_t forecolor;
        uint32_t backcolor;

        constexpr inline BufferGridCell()
            : character(L' '), forecolor(0xFFFFFFFF), backcolor(0xFF000000) { }
        constexpr inline BufferGridCell(wchar_t Character, uint32_t Forecolor, uint32_t Backcolor)
            : character(Character), forecolor(Forecolor), backcolor(Backcolor) { }
    };

    struct SizeU32 {
        uint32_t width;
        uint32_t height;

        constexpr inline SizeU32()
            : width(0), height(0) { }
        constexpr inline SizeU32(uint32_t Width, uint32_t Height)
            : width(Width), height(Height) { }

        inline constexpr bool operator==(const SizeU32& Other) const {
            return width == Other.width && height == Other.height;
        }
        inline constexpr bool operator!=(const SizeU32& Other) const {
            return width != Other.width || height != Other.height;
        }
    };

    struct BufferGrid {
        union {
            struct { uint32_t width, height; };
            SizeU32 size;
        };
        BufferGridCell* buffer;

        constexpr inline BufferGrid()
            : width(0), height(0), buffer(0) { }
        constexpr inline BufferGrid(uint32_t Width, uint32_t Height, BufferGridCell* Buffer)
            : width(Width), height(Height), buffer(Buffer) { }
        constexpr inline BufferGrid(SizeU32 BufferSize, BufferGridCell* Buffer)
            : size(BufferSize), buffer(Buffer) { }
    };

    struct PointU32 {
        uint32_t x;
        uint32_t y;

        constexpr inline PointU32()
            : x(0), y(0) { }
        constexpr inline PointU32(uint32_t X, uint32_t Y)
            : x(X), y(Y) { }

        constexpr inline bool operator==(const PointU32& Other) const {
            return x == Other.x && y == Other.y;
        }
        constexpr inline bool operator!=(const PointU32& Other) const {
            return x != Other.x || y != Other.y;
        }

        constexpr inline PointU32& operator+=(const PointU32& Other) {
            x += Other.x;
            y += Other.y;
            return *this;
        }
        constexpr inline PointU32& operator-=(const PointU32& Other) {
            x -= Other.x;
            y -= Other.y;
            return *this;
        }
        constexpr inline PointU32 operator+(const PointU32& Other) const {
            return PointU32(*this) += Other;
        }
        constexpr inline PointU32 operator-(const PointU32& Other) const {
            return PointU32(*this) -= Other;
        }
    };

    struct RectU32 {
        union {
            struct {
                uint32_t x;
                uint32_t y;
            };
            PointU32 point;
        };
        union {
            struct {
                uint32_t width;
                uint32_t height;
            };
            SizeU32 size;
        };

        constexpr inline RectU32()
            : x(0), y(0), width(0), height(0) { }
        constexpr inline RectU32(uint32_t X, uint32_t Y, uint32_t Width, uint32_t Height)
            : x(X), y(Y), width(Width), height(Height) { }
        constexpr inline RectU32(PointU32 Point, SizeU32 Size)
            : point(Point), size(Size) { }

        constexpr inline bool operator==(const RectU32& Other) const {
            return x == Other.x && y == Other.y && width == Other.width && height == Other.height;
        }
        constexpr inline bool operator!=(const RectU32& Other) const {
            return x != Other.x || y != Other.y || width != Other.width || height != Other.height;
        }

        constexpr inline RectU32& operator*=(const RectU32& Other) {
            uint32_t endX = x + width;
            uint32_t endY = y + height;
            uint32_t otherEndX = Other.x + Other.width;
            uint32_t otherEndY = Other.y + Other.height;

            x = max(x, Other.x);
            y = max(y, Other.y);
            uint32_t outEndX = min(endX, otherEndX);
            uint32_t outEndY = min(endY, otherEndY);

            if (outEndX <= x || outEndY <= y) {
                x = 0;
                y = 0;
                width = 0;
                height = 0;
            }
            else {
                width = outEndX - x;
                height = outEndY - y;
            }
        }

        constexpr inline RectU32 operator*(const RectU32& Other) const {
            return RectU32(*this) *= Other;
        }

        constexpr bool IsPointWithin(PointU32 Point) const {
            return Point.x < x || Point.y < y || Point.x >= x + width || Point.y >= y + height;
        }
    };

    enum WindowState {
        Hidden,
        Minimized,
        Maximized,
        Restored
    };

    struct KeyPressInfo {
        wchar_t keyChar;
        uint32_t keyCode;
        bool shiftPressed;
        bool ctrlPressed;
        bool altPressed;
    };
    struct MouseClickInfo {
        union {
            struct {
                uint32_t x; //column of the char clicked
                uint32_t y; //row of the char clicked
            };
            PointU32 point;
        };
        bool leftButton;
        bool rightButton;
    };
    struct MouseMoveInfo {
        union {
            struct {
                uint32_t x; //column of the char moved to
                uint32_t y; //row of the char moved to
            };
            PointU32 point;
        };
    };
    struct MouseEnterInfo {
        union {
            struct {
                uint32_t x; //column of the char entered to
                uint32_t y; //row of the char entered to
            };
            PointU32 point;
        };
    };
    struct MouseExitInfo { };
    struct MouseScrollInfo {
        int32_t scrollAmount;
    };
    struct CloseRequestInfo {
        bool canCancel;
    };
    struct DisposedInfo { };
    struct TextInputInfo {
        std::wstring inputText;
    };
    struct FocusGainedInfo { };
    struct FocusLostInfo { };
    struct WindowStateChangeInfo {
        WindowState newWindowState;
    };
    struct ResizeInfo {
        union {
            struct {
                uint32_t newWidth; //count of characters, *not pixels*
                uint32_t newHeight; //count of characters, *not pixels*
            };
            SizeU32 newSize;
        };
    };
    struct ResizeCompleteInfo {
        union {
            struct {
                uint32_t newWidth; //count of characters, *not pixels*
                uint32_t newHeight; //count of characters, *not pixels*
            };
            SizeU32 newSize;
        };
    };
    struct FileDropInfo {
        std::vector<std::wstring> filePaths;
        union {
            struct {
                uint32_t mouseX; //column of the char the mouse was on when it dropped the file
                uint32_t mouseY; //row of the char the mouse was on when it dropped the file
            };
            PointU32 point;
        };
    };

    namespace details {
        struct QueuedTask {
            std::function<void()> func;
            int* statusCode; //0 for pending; 1 for complete; 2 for cancelled

            inline QueuedTask(std::function<void()> Func, int* StatusCodePointer)
                : func(std::move(Func)), statusCode(StatusCodePointer) { }

            inline void Run() {
                func();
                *statusCode = 1;
            }
            inline void Cancel() {
                *statusCode = 2;
            }
        };
    }

    class WindowBase {
        HWND hwnd;
        HINSTANCE hInstance;
        std::wstring className;

        std::mutex mtx;
        std::thread updateThread;
        std::atomic<bool> isRunning;
        std::queue<details::QueuedTask> taskQueue;

        bool allowTransparentBackgrounds;
        bool mouseContained;
        BufferGridCell* lastBuffer;
        SizeU32 lastBufferSize;
        WindowState lastWindowState;

        void UpdateFunction(bool* Initialized);
        static LRESULT CALLBACK WindowProcStatic(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam);
        LRESULT WindowProc(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam);
        void ProcessTasks();
        void CancelTasks();
        bool InvokeOnWindowThread(std::function<void()> Func);
    public:
        // These functions initialize or dispose
        // a window. By default, the window is
        // hidden (not visible).

        WindowBase(HINSTANCE HInstance);
        ~WindowBase();

        void Dispose();
        bool Running() const;
        bool Disposed() const;

        // Deleting all copy stuff.

        WindowBase(const WindowBase&) = delete;
        WindowBase& operator=(const WindowBase&) = delete;

        // Deleting all move stuff (too many pointers pointing back).

        WindowBase(WindowBase&&) = delete;
        WindowBase& operator=(WindowBase&&) = delete;

        // The width and height of the buffer, as
        // well as functionality to copy it out.

        SizeU32 BufferSize();
        BufferGrid CopyBufferOut();
        bool CopyBufferOut(SizeU32 BufferSize, BufferGridCell* Buffer);
        bool CopyBufferOut(BufferGrid Buffer);

        // Invalidates the client area for redrawing.

        void Invalidate();

        // The window is visible if the computer's
        // user may view it. Show() makes it visible
        // to the user, and also gives it focus.
        // Hide() hides it from the user.

        bool IsVisible();
        void Show();
        void Hide();

        // The following three are self-explainatory.

        bool IsMinimized();
        bool IsMaximized();
        WindowState GetWindowState();
        bool HasFocus();

        // The following three are also self-explainatory.

        void Minimize();
        void Maximize();
        void Restore();
        void SetWindowState(WindowState State);
        void CaptureFocus();

        // The title of the window

        std::wstring GetTitle();
        void SetTitle(std::wstring Title);

        // Disabling transparent backgrounds increases
        // efficiency. But if the user wants them,
        // okey dokey. It's simple enough to implement.

        bool GetAllowTransparentBackgrounds();
        void SetAllowTransparentBackgrounds(bool Allow);
    protected:
        // Client repaint (buffer should be in
        // row-major order).

        virtual void PaintBuffer(BufferGrid Buffer) = 0;

        // Events

        virtual void OnKeyPress(const KeyPressInfo& Info) { };
        virtual void OnMouseClick(const MouseClickInfo& Info) { };
        virtual void OnMouseMove(const MouseMoveInfo& Info) { };
        virtual void OnMouseEnter(const MouseEnterInfo& Info) { };
        virtual void OnMouseExit(const MouseExitInfo& Info) { };
        virtual void OnMouseScroll(const MouseScrollInfo& Info) { };
        virtual void OnTextInput(const TextInputInfo& Info) { };
        virtual void OnFocusGained(const FocusGainedInfo& Info) { };
        virtual void OnFocusLost(const FocusLostInfo& Info) { };
        virtual bool OnCloseRequest(const CloseRequestInfo& Info) { return true; };
        virtual void OnDisposed(const DisposedInfo& Info) { };
        virtual void OnWindowStateChange(const WindowStateChangeInfo& Info) { };
        virtual void OnResize(const ResizeInfo& Info) { };
        virtual void OnResizeComplete(const ResizeCompleteInfo& Info) { };
        virtual void OnFileDrop(const FileDropInfo& Info) { };
    };
}