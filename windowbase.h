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
    struct ResizeInfo {
        uint32_t newWidth;
        uint32_t newHeight;
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

        // The following three are self-explanitory.

        bool IsMinimized() const;
        bool IsMaximized() const;
        bool HasFocus() const;

        // The following three are also self-explanitory.

        void Minimize();
        void Maximize();
        void CaptureFocus();

        // The title of the window

        std::wstring GetTitle() const;
        void SetTitle(std::wstring Title);
    protected:
        virtual void OnResize(const ResizeInfo& Info);
        virtual void OnMouseClick(const MouseClickInfo& Info);
        virtual void OnKeyPress(const KeyPressInfo& Info);
    };
}