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

    WindowBase::WindowBase(HINSTANCE HInstance) {
        hInstance = HInstance;
        className = GenerateGuidStr();

        WNDCLASS wc;
        wc.hInstance = HInstance;
        wc.lpszClassName = className.c_str();
        wc.lpfnWndProc = WindowProcStatic;

        details::WindowProcCallStruct callStr;
        callStr.classPtr = this;
        callStr.funcPtr = WindowProc;
        callStructPtr = std::make_unique<details::WindowProcCallStruct>(callStr);

        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(callStructPtr.get()));
        updateThread = std::thread(CallUpdateFunction, &WindowBase::UpdateFunction, this);

        //incomplete: many other variables are uninitialized
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
        bufSize.width = (rect.right - rect.left) / charWidth;
        bufSize.height = (rect.bottom - rect.top) / charHeight;
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