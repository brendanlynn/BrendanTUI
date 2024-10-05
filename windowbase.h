#pragma once

#include <chrono>
#include <concepts>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <windows.h>

namespace btui {
    struct BufferGridCell {
        wchar_t character;
        uint32_t forecolor;
        uint32_t backcolor;
    };

    struct BufferSize {
        uint32_t width;
        uint32_t height;

        BufferSize() = default;
        BufferSize(uint32_t Width, uint32_t Height)
            : width(Width), height(Height) { }

        bool operator==(const BufferSize& Other) const {
            return width == Other.width && height == Other.height;
        }
        bool operator!=(const BufferSize& Other) const {
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
    struct MouseScrollInfo {
        int32_t scrollAmount;
    };
    struct CloseRequestInfo {
        bool canCancel;
    };
    struct TextInputInfo {
        std::wstring inputText;
    };
    struct FocusGainedInfo { };
    struct FocusLostInfo { };
    struct WindowStateChangeInfo {
        WindowState newWindowState;
    };
    struct IdleInfo {
        std::chrono::milliseconds idleDuration;
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
    };

    class WindowBase;

    namespace details {
        using windowProcMemberFunc_t = LRESULT(btui::WindowBase::*)(HWND, UINT, WPARAM, LPARAM);

        struct WindowProcCallStruct {
            btui::WindowBase* classPtr;
            windowProcMemberFunc_t funcPtr;
        };
    }

    class WindowBase {
        HWND hwnd;
        HINSTANCE hInstance;
        std::wstring className;

        std::mutex mtx;
        std::thread updateThread;
        bool stopThread;
        std::unique_ptr<details::WindowProcCallStruct> callStructPtr;

        bool allowTransparentBackgrounds;
        BufferGridCell* lastBuffer;
        btui::BufferSize lastBufferSize;

        void UpdateFunction();
        LRESULT WindowProc(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam);
    public:
        // These functions initialize or dispose
        // a window. By default, the window is
        // hidden (not visible).

        WindowBase(HINSTANCE HInstance);
        ~WindowBase();

        // Deleting all copy stuff.

        WindowBase(const WindowBase&) = delete;
        WindowBase& operator=(const WindowBase&) = delete;

        // Deleting all move stuff (too many pointers pointing back).

        WindowBase(WindowBase&&) = delete;
        WindowBase& operator=(WindowBase&&) = delete;

        // The HWND of the window.

        HWND GetHwnd();

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

        bool IsVisible() const;
        void Show();
        void Hide();

        // The following three are self-explainatory.

        bool IsMinimized() const;
        bool IsMaximized() const;
        WindowState GetState() const;
        bool HasFocus() const;

        // The following three are also self-explainatory.

        void Minimize();
        void Maximize();
        void Restore();
        void SetState(WindowState State);
        void CaptureFocus();

        // The title of the window

        std::wstring GetTitle() const;
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
        virtual void OnMouseScroll(const MouseScrollInfo& Info) { };
        virtual void OnTextInput(const TextInputInfo& Info) { };
        virtual void OnFocusGained(const FocusGainedInfo& Info) { };
        virtual void OnFocusLost(const FocusLostInfo& Info) { };
        virtual bool OnCloseRequest(const CloseRequestInfo& Info) { return true; };
        virtual void OnWindowStateChange(const WindowStateChangeInfo& Info) { };
        virtual void OnResize(const ResizeInfo& Info) { };
        virtual void OnResizeComplete(const ResizeCompleteInfo& Info) { };
        virtual void OnIdle(const IdleInfo& Info) { };
        virtual void OnFileDrop(const FileDropInfo& Info) { };
    };
}