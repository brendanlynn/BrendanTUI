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

        bool operator==(const BufferSize& Other) const {
            return width == Other.width && height == Other.height;
        }
        bool operator!=(const BufferSize& Other) const {
            return width != Other.width || height != Other.height;
        }
    };

    enum WindowState {
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
        uint32_t x;
        uint32_t y;
        bool leftButton;
        bool rightButton;
    };
    struct MouseMoveInfo {
        uint32_t x;
        uint32_t y;
    };
    struct MouseDragInfo {
        uint32_t startX;
        uint32_t startY;
        uint32_t endX;
        uint32_t endY;
        bool leftButton;
        bool rightButton;
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
        uint32_t newWidth;
        uint32_t newHeight;
    };
    struct ResizeCompleteInfo {
        uint32_t newWidth;
        uint32_t newHeight;
    };
    struct ClipboardCopyInfo { };
    struct ClipboardPasteInfo {
        std::wstring pastedText;
    };
    struct FileDropInfo {
        std::vector<std::wstring> filePaths;
    };

    using paintChars_t = std::function<void(uint32_t Width, uint32_t Height, wchar_t* Buffer)>;

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
        BufferGridCell* buffer;
        btui::BufferSize lastBufferSize;
        paintChars_t paintFunc;

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

        // Functions for painting and invalidation of
        // the client area.

        paintChars_t GetPaintFunc();
        void SetPaintFunc(paintChars_t PaintFunc);
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

        bool GetAllowTransparentBackgrounds();
        void SetAllowTransparentBackgrounds(bool Allow);
    protected:
        // Events

        virtual void OnKeyPress(const KeyPressInfo& Info) { };
        virtual void OnMouseClick(const MouseClickInfo& Info) { };
        virtual void OnMouseMove(const MouseMoveInfo& Info) { };
        virtual void OnMouseDrag(const MouseDragInfo& Info) { };
        virtual void OnMouseScroll(const MouseScrollInfo& Info) { };
        virtual void OnTextInput(const TextInputInfo& Info) { };
        virtual void OnFocusGained(const FocusGainedInfo& Info) { };
        virtual void OnFocusLost(const FocusLostInfo& Info) { };
        virtual bool OnCloseRequest(const CloseRequestInfo& Info) { };
        virtual void OnWindowStateChange(const WindowStateChangeInfo& Info) { };
        virtual void OnResize(const ResizeInfo& Info) { };
        virtual void OnResizeComplete(const ResizeCompleteInfo& Info) { };
        virtual void OnIdle(const IdleInfo& Info) { };
        virtual void OnFileDrop(const FileDropInfo& Info) { };
        virtual void OnClipboardCopy(const ClipboardCopyInfo& Info) { };
        virtual void OnClipboardPaste(const ClipboardPasteInfo& Info) { };
    };
}