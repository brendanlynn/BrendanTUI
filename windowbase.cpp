#include "pch.h"
#include "framework.h"

#include "windowbase.h"
#include <exception>


namespace btui {
    void WindowBase::UpdateFunction() {
        MSG msg;
        while (GetMessageW(&msg, hwnd, 0, 0)) {
            switch (msg.message) {
                //stuff
            }
            if (stopThread) {
                //stuff (presumably)
                return;
            }
        }
    }

    WindowBase::WindowBase() {
        updateThread = std::thread(UpdateFunction);
    }
    WindowBase::~WindowBase() {
        stopThread = true;
        updateThread.join();
        //stuff
    }

    HWND WindowBase::GetHwnd() {
        return hwnd;
    }

    BufferSize WindowBase::BufferSize() {
        RECT rect;
        ::GetWindowRect(hwnd, &rect);

        btui::BufferSize bufSize;
        bufSize.width = (rect.right - rect.left) / 10; //placeholder
        bufSize.height = (rect.bottom - rect.top) / 15; //placeholder
        return bufSize;
    }
    BufferGridCell* WindowBase::CopyBufferOut() {
        std::lock_guard<std::mutex> lock(mtx);

        btui::BufferSize bufSize = lastBufferSize;
        BufferGridCell* outBuffer = new BufferGridCell[bufSize.width * bufSize.height];
        memcpy(outBuffer, buffer, sizeof(BufferGridCell) * bufSize.width * bufSize.height);

        return outBuffer;
    }
    bool WindowBase::CopyBufferOut(btui::BufferSize BufferSize, BufferGridCell* Buffer) {
        std::lock_guard<std::mutex> lock(mtx);

        if (lastBufferSize != BufferSize) return false;

        memcpy(Buffer, buffer, sizeof(BufferGridCell) * BufferSize.width * BufferSize.height);

        return true;
    }

    paintChars_t WindowBase::GetPaintFunc() {
        std::lock_guard<std::mutex> lock(mtx);

        return paintFunc;
    }
    void WindowBase::SetPaintFunc(paintChars_t PaintFunc) {
        std::lock_guard<std::mutex> lock(mtx);

        paintFunc = PaintFunc;
    }
    void WindowBase::Invalidate() {
        ::InvalidateRect(hwnd, NULL, allowTransparentBackgrounds ? TRUE : FALSE);
    }

    bool WindowBase::IsVisible() const {
        return hwnd && ::IsWindowVisible(hwnd);
    }
    void WindowBase::Show() {
        if (hwnd) {
            ::ShowWindow(hwnd, SW_SHOW);
            ::SetFocus(hwnd);
        }
    }
    void WindowBase::Hide() {
        if (hwnd) {
            ::ShowWindow(hwnd, SW_HIDE);
        }
    }

    bool WindowBase::IsMinimized() const {
        return hwnd && ::IsIconic(hwnd);
    }
    bool WindowBase::IsMaximized() const {
        return hwnd && ::IsZoomed(hwnd);
    }
    bool WindowBase::HasFocus() const {
        return hwnd && ::GetFocus() == hwnd;
    }

    void WindowBase::Minimize() {
        if (hwnd) {
            ::ShowWindow(hwnd, SW_MINIMIZE);
        }
    }
    void WindowBase::Maximize() {
        if (hwnd) {
            ::ShowWindow(hwnd, SW_MAXIMIZE);
        }
    }
    void WindowBase::CaptureFocus() {
        if (hwnd) {
            ::SetFocus(hwnd);
        }
    }

    std::wstring WindowBase::GetTitle() const {
        if (!hwnd) return L"";

        int len = ::GetWindowTextLengthW(hwnd);
        if (!len) return L"";

        std::wstring title(len, '\0');
        ::GetWindowTextW(hwnd, &title[0], title.size());

        return title;
    }
    void WindowBase::SetTitle(std::wstring Title) {
        if (hwnd) {
            ::SetWindowTextW(hwnd, Title.c_str());
        }
    }
}