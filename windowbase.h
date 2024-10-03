#pragma once

#include <string>
#include <functional>
#include <windows.h>

namespace btui {
    struct BufferGridCell {
        char character;
        uint32_t forecolor;
        uint32_t backcolor;
    };

    class WindowBase {
    private:
        // ?
    public:
        // These functions initialize a window. By
        // default, the window is hidden (not
        // visible).

        WindowBase();
        WindowBase(std::string Title);

        // The window is visible if the computer's
        // user may view it. Show() makes it visible
        // to the user, and also gives it focus.
        // Hide() hides it from the user.

        bool IsVisible();
        void Show();
        void Hide();

        // The following three are self-explanitory.

        bool IsMinimized();
        bool IsMaximized();
        bool HasFocus();

        // The following three are also self-explanitory.

        void Minimize();
        void Maximize();
        bool GetFocus();

        // The title of the window

        std::string GetTitle();
        void SetTitle(std::string Title);

        // These are the widths and heights of the buffers.

        uint32_t GetBufferWidth();
        uint32_t GetBufferHeight();

        // The back buffer is the buffer the programmer
        // is allowed to write to. The front buffer is
        // what is currently displayed. When WriteBuffer()
        // is called, the back and front buffer is
        // switched, and the characters in the new front
        // buffer (previously the back buffer) are now
        // written to the screen.
        //
        // The buffers are in row-major order and their
        // length is equal to the product of
        // GetBufferWidth() and GetBufferHeight().

        BufferGridCell* GetBackBuffer();
        const BufferGridCell* GetFrontBuffer();
        void WriteBuffer();

        // Gets the position of the cursor by buffer row
        // and column (column first). If the cursor is
        // not within the window, this returns ~0 for
        // both.

        std::pair<uint32_t, uint32_t> GetCursorPos();

        // Maybe some event handers? Clicks? Double
        // clicks? Scrolling? Key presses? Resizing?
        // Resizing is probably the most important. Then
        // keyboard and mouse stuff.
    };
}