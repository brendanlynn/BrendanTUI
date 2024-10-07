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
    static inline uint32_t FromRGB(uint8_t R, uint8_t G, uint8_t B) {
        return (uint32_t)R << 16 | (uint32_t)G << 8 | (uint32_t)B;
    }
    static inline uint32_t FromARGB(uint8_t A, uint8_t R, uint8_t G, uint8_t B) {
        return (uint32_t)A << 24 | (uint32_t)R << 16 | (uint32_t)G << 8 | (uint32_t)B;
    }

    struct BufferGridCell {
        wchar_t character;
        uint32_t forecolor;
        uint32_t backcolor;

        BufferGridCell() = default;
        inline BufferGridCell(wchar_t Character, uint32_t Forecolor, uint32_t Backcolor)
            : character(Character), forecolor(Forecolor), backcolor(Backcolor) { }
    };

    struct BufferSize {
        uint32_t width;
        uint32_t height;

        BufferSize() = default;
        inline BufferSize(uint32_t Width, uint32_t Height)
            : width(Width), height(Height) { }

        inline bool operator==(const BufferSize& Other) const {
            return width == Other.width && height == Other.height;
        }
        inline bool operator!=(const BufferSize& Other) const {
            return width != Other.width || height != Other.height;
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
        uint32_t x; //column of the char clicked
        uint32_t y; //row of the char clicked
        bool leftButton;
        bool rightButton;
    };
    struct MouseMoveInfo {
        uint32_t x; //column of the char moved to
        uint32_t y; //row of the char moved to
    };
    struct MouseEnterInfo {
        uint32_t x; //column of the char entered to
        uint32_t y; //row of the char entered to
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
        uint32_t newWidth; //count of characters, *not pixels*
        uint32_t newHeight; //count of characters, *not pixels*
    };
    struct ResizeCompleteInfo {
        uint32_t newWidth; //count of characters, *not pixels*
        uint32_t newHeight; //count of characters, *not pixels*
    };
    struct FileDropInfo {
        std::vector<std::wstring> filePaths;
        uint32_t mouseX; //column of the char the mouse was on when it dropped the file
        uint32_t mouseY; //row of the char the mouse was on when it dropped the file
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
        btui::BufferSize lastBufferSize;
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

        btui::BufferSize BufferSize();
        BufferGridCell* CopyBufferOut();
        bool CopyBufferOut(btui::BufferSize BufferSize, BufferGridCell* Buffer);

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

        virtual void PaintBuffer(uint32_t Width, uint32_t Height, BufferGridCell* Buffer) = 0;

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