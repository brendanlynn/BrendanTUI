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

    using paintChars_t = std::function<void(uint32_t Width, uint32_t Height, wchar_t* Buffer)>;

    class WindowBase {
        HWND hwnd;
        HINSTANCE hInstance;

        std::mutex mtx;
        std::thread updateThread;
        bool stopThread;

        bool allowTransparentBackgrounds;
        BufferGridCell* buffer;
        btui::BufferSize lastBufferSize;
        paintChars_t paintFunc;

        void UpdateFunction();
    public:
        // These functions initialize or dispose
        // a window. By default, the window is
        // hidden (not visible).

        WindowBase();
        ~WindowBase();

        // Deleting all copy stuff

        WindowBase(const WindowBase&) = delete;
        WindowBase& operator=(const WindowBase&) = delete;

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

        // Maybe some event handers? Clicks? Double
        // clicks? Scrolling? Key presses? Resizing?
        // Resizing is probably the most important. Then
        // keyboard and mouse stuff.
    };
}