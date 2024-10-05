#include "pch.h"
#include "framework.h"

#include "windowbase.h"
#include <combaseapi.h>
#include <exception>

constexpr uint32_t charWidth = 10; //placeholder
constexpr uint32_t charHeight = 15; //placeholder

std::wstring GenerateGuidStr() {
    GUID guid;
    CoCreateGuid(&guid);

    wchar_t buffer[40];
    swprintf_s(buffer, sizeof(buffer), L"{%08x-%04x-%04x-%04x-%012llx}",
        guid.Data1, guid.Data2, guid.Data3,
        (guid.Data4[0] << 8) | guid.Data4[1],
        *(reinterpret_cast<unsigned long long*>(&guid.Data4[2])));

    return std::wstring(buffer);
}

LRESULT CALLBACK WindowProcStatic(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam) {
    btui::details::WindowProcCallStruct* funcWCont = reinterpret_cast<btui::details::WindowProcCallStruct*>(GetWindowLongPtr(Hwnd, GWLP_USERDATA));

    return (funcWCont->classPtr->*(funcWCont->funcPtr))(Hwnd, Msg, WParam, LParam);
}

void CallUpdateFunction(void (btui::WindowBase::* UpdateFunc)(), btui::WindowBase* Class) {
    (Class->*UpdateFunc)();
}

namespace btui {
    void WindowBase::UpdateFunction() {
        MSG msg;
        while (GetMessageW(&msg, hwnd, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    LRESULT WindowBase::WindowProc(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam) {
        switch (Msg) {
            // stuff
        }
    }

    WindowBase::WindowBase(HINSTANCE HInstance)
        : hInstance(HInstance), stopThread(false), allowTransparentBackgrounds(false), lastBuffer(0), lastBufferSize(0, 0) {

        className = GenerateGuidStr();

        WNDCLASS wc = {};
        wc.hInstance = HInstance;
        wc.lpszClassName = className.c_str();
        wc.lpfnWndProc = WindowProcStatic;

        details::WindowProcCallStruct callStr;
        callStr.classPtr = this;
        callStr.funcPtr = &WindowBase::WindowProc;
        callStructPtr = std::make_unique<details::WindowProcCallStruct>(callStr);

        RegisterClass(&wc);

        hwnd = CreateWindowEx(
            0,                       // Optional window styles
            className.c_str(),       // Window class
            L"",                     // Window title
            WS_OVERLAPPEDWINDOW,     // Window style

            // Position and size
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

            nullptr,                 // Parent window    
            nullptr,                 // Menu
            HInstance,               // Instance handle
            nullptr                  // Additional application data
        );

        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(callStructPtr.get()));
        updateThread = std::thread(CallUpdateFunction, &WindowBase::UpdateFunction, this);
    }
    WindowBase::~WindowBase() {
        stopThread = true;
        if (updateThread.joinable())
            updateThread.join();

        if (hwnd) DestroyWindow(hwnd);

        delete[] lastBuffer;
        lastBuffer = 0;

        UnregisterClassW(className.c_str(), hInstance);
    }

    HWND WindowBase::GetHwnd() {
        return hwnd;
    }

    BufferSize WindowBase::BufferSize() {
        RECT rect;
        ::GetWindowRect(hwnd, &rect);

        btui::BufferSize bufSize;
        bufSize.width = (rect.right - rect.left) / charWidth;
        bufSize.height = (rect.bottom - rect.top) / charHeight;
        return bufSize;
    }
    BufferGridCell* WindowBase::CopyBufferOut() {
        std::lock_guard<std::mutex> lock(mtx);

        btui::BufferSize bufSize = lastBufferSize;
        BufferGridCell* outBuffer = new BufferGridCell[bufSize.width * bufSize.height];
        memcpy(outBuffer, lastBuffer, sizeof(BufferGridCell) * bufSize.width * bufSize.height);

        return outBuffer;
    }
    bool WindowBase::CopyBufferOut(btui::BufferSize BufferSize, BufferGridCell* Buffer) {
        std::lock_guard<std::mutex> lock(mtx);

        if (lastBufferSize != BufferSize) return false;

        memcpy(Buffer, lastBuffer, sizeof(BufferGridCell) * BufferSize.width * BufferSize.height);

        return true;
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
    WindowState WindowBase::GetState() const {
        if (!IsVisible())
            return WindowState::Hidden;
        else if (IsMinimized())
            return WindowState::Minimized;
        else if (IsMaximized())
            return WindowState::Maximized;
        else
            return WindowState::Restored;
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
    void WindowBase::Restore() {
        if (hwnd) {
            ::ShowWindow(hwnd, SW_RESTORE);
        }
    }
    void WindowBase::SetState(WindowState State) {
        switch (State) {
        case WindowState::Hidden:
            Hide();
            break;
        case WindowState::Minimized:
            Minimize();
            break;
        case WindowState::Maximized:
            Maximize();
            break;
        case WindowState::Restored:
            Restore();
            break;
        default:
            throw std::exception("Invalid WindowState param.");
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

    bool WindowBase::GetAllowTransparentBackgrounds() {
        std::lock_guard<std::mutex> lock(mtx);

        return allowTransparentBackgrounds;
    }
    void WindowBase::SetAllowTransparentBackgrounds(bool Allow) {
        std::lock_guard<std::mutex> lock(mtx);

        allowTransparentBackgrounds = Allow;
    }
}