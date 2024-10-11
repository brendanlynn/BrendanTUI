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

        static constexpr inline RectU32 Intersection(const RectU32& Rect1, const RectU32& Rect2) {
            RectU32 out;

            uint32_t r1EndX = Rect1.x + Rect1.width;
            uint32_t r1EndY = Rect1.y + Rect1.height;
            uint32_t r2EndX = Rect2.x + Rect2.width;
            uint32_t r2EndY = Rect2.y + Rect2.height;

            out.x = max(Rect1.x, Rect2.x);
            out.y = max(Rect1.y, Rect2.y);
            uint32_t outEndX = min(r1EndX, r2EndX);
            uint32_t outEndY = min(r1EndY, r2EndY);

            if (outEndX <= out.x || outEndY <= out.y) {
                out.x = 0;
                out.y = 0;
            }
            else {
                out.width = outEndX - out.x;
                out.height = outEndY - out.y;
            }
            return out;
        }

        constexpr bool IsPointWithin(PointU32 Point) const {
            return Point.x < x || Point.y < y || Point.x >= x + width || Point.y >= y + height;
        }
    };

    enum WindowState {
        WindowStateHidden,
        WindowStateMinimized,
        WindowStateMaximized,
        WindowStateRestored
    };

    enum CursorType {
        CursorTypeInvisible,
        CursorTypeArrow,
        CursorTypeHand,
        CursorTypeIBeam,
        CursorTypeCrosshairs,
        CursorTypeNo,
        CursorTypeResizeNS,
        CursorTypeResizeEW,
        CursorTypeResizeNESW,
        CursorTypeResizeNWSE,
        CursorTypeResizeOmni,
        CursorTypeStarting,
        CursorTypeWait,
        CursorTypeHelp,
        CursorTypeUpArrow
    };

    struct KeyPressInfo {
        wchar_t keyChar;
        uint32_t keyCode;
        bool shiftPressed;
        bool ctrlPressed;
        bool altPressed;

        constexpr inline KeyPressInfo()
            : keyChar('\0'), keyCode(0), shiftPressed(0), ctrlPressed(0), altPressed(0) { }
    };
    struct MouseDownInfo {
        union {
            struct {
                uint32_t x; //column of the char clicked
                uint32_t y; //row of the char clicked
            };
            PointU32 point;
        };
        bool leftButton;
        bool rightButton;

        constexpr inline MouseDownInfo()
            : point(), leftButton(0), rightButton(0) { }
    };
    struct MouseUpInfo {
        union {
            struct {
                uint32_t x; //column of the char clicked
                uint32_t y; //row of the char clicked
            };
            PointU32 point;
        };
        bool leftButton;
        bool rightButton;

        constexpr inline MouseUpInfo()
            : point(), leftButton(0), rightButton(0) { }
    };
    struct MouseMoveInfo {
        union {
            struct {
                uint32_t x; //column of the char moved to
                uint32_t y; //row of the char moved to
            };
            PointU32 point;
        };

        constexpr inline MouseMoveInfo()
            : point() { }
    };
    struct MouseEnterInfo {
        union {
            struct {
                uint32_t x; //column of the char entered to
                uint32_t y; //row of the char entered to
            };
            PointU32 point;
        };

        constexpr inline MouseEnterInfo()
            : point() { }
    };
    struct MouseExitInfo { };
    struct MouseScrollInfo {
        int32_t scrollAmount;

        constexpr inline MouseScrollInfo()
            : scrollAmount(0) { }
    };
    struct CloseRequestInfo {
        bool canCancel;

        constexpr inline CloseRequestInfo()
            : canCancel(0) { }
    };
    struct DisposedInfo { };
    struct TextInputInfo {
        std::wstring inputText;

        constexpr inline TextInputInfo()
            : inputText(L"") { }
    };
    struct FocusGainedInfo { };
    struct FocusLostInfo { };
    struct WindowStateChangeInfo {
        WindowState newWindowState;

        constexpr inline WindowStateChangeInfo()
            : newWindowState((WindowState)~0) { }
    };
    struct ResizeInfo {
        union {
            struct {
                uint32_t newWidth; //count of characters, *not pixels*
                uint32_t newHeight; //count of characters, *not pixels*
            };
            SizeU32 newSize;
        };

        constexpr inline ResizeInfo()
            : newSize() { }
    };
    struct ResizeCompleteInfo {
        union {
            struct {
                uint32_t newWidth; //count of characters, *not pixels*
                uint32_t newHeight; //count of characters, *not pixels*
            };
            SizeU32 newSize;
        };

        constexpr inline ResizeCompleteInfo()
            : newSize() { }
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

        inline FileDropInfo()
            : filePaths(), point() { }
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

        uint32_t backColor;
        bool mouseContained;
        BufferGridCell* lastBuffer;
        SizeU32 lastBufferSize;
        WindowState lastWindowState;
        CursorType cursorType;

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

        // The background of the window

        uint32_t GetBackgroundColor();
        void SetBackgroundColor(uint32_t Color);

        // The type of cursor

        CursorType GetCursorType();
        void SetCursorType(CursorType Type);
    protected:
        // Client repaint (buffer should be in
        // row-major order).

        virtual void PaintBuffer(BufferGrid Buffer) = 0;

        // Events

        virtual void OnKeyPress(const KeyPressInfo& Info) { };
        virtual void OnMouseDown(const MouseDownInfo& Info) { };
        virtual void OnMouseUp(const MouseUpInfo& Info) { };
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