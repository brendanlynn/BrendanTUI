#include "pch.h"
#include "framework.h"

#include "windowbase.h"
#include <combaseapi.h>
#include <exception>
#include <shellapi.h>

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

inline int GetLParamX(LPARAM lParam) {
    return (int)(short)LOWORD(lParam);
}
inline int GetLParamY(LPARAM lParam) {
    return (int)(short)HIWORD(lParam);
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
        case WM_KEYDOWN: {
            KeyPressInfo info;
            info.keyChar = static_cast<wchar_t>(WParam);
            info.keyCode = static_cast<uint32_t>(WParam);
            info.shiftPressed = GetKeyState(VK_SHIFT) & 0x8000;
            info.ctrlPressed = GetKeyState(VK_CONTROL) & 0x8000;
            info.altPressed = GetKeyState(VK_MENU) & 0x8000;

            OnKeyPress(info);
            return 0;
        }
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN: {
            POINT mousePt;
            mousePt.x = GetLParamX(LParam);
            mousePt.y = GetLParamY(LParam);
            ScreenToClient(hwnd, &mousePt);

            MouseClickInfo info;
            info.x = mousePt.x / charWidth;
            info.y = mousePt.y / charWidth;
            info.leftButton = (Msg == WM_LBUTTONDOWN);
            info.rightButton = (Msg == WM_RBUTTONDOWN);

            OnMouseClick(info);
            return 0;
        }
        case WM_MOUSEMOVE: {
            POINT mousePt;
            mousePt.x = GetLParamX(LParam);
            mousePt.y = GetLParamY(LParam);
            ScreenToClient(hwnd, &mousePt);

            MouseMoveInfo info;
            info.x = mousePt.x / charWidth;
            info.y = mousePt.y / charWidth;

            OnMouseMove(info);
            return 0;
        }
        case WM_SIZE: {
            ResizeInfo info;
            info.newWidth = LOWORD(LParam);
            info.newHeight = HIWORD(LParam);

            //Should OnResize not be called if the *character* lengths are the same?

            OnResize(info);
            
            //Should the client area automatically be invalidated?

            return 0;
        }
        case WM_SHOWWINDOW: {
            WindowState newState = WParam ? GetWindowState() : WindowState::Hidden;

            if (newState != lastWindowState) {
                WindowStateChangeInfo info;
                info.newWindowState = newState;

                OnWindowStateChange(info);
            }

            return 0;
        }
        case WM_DROPFILES: {
            HDROP hDrop = (HDROP)WParam;
            FileDropInfo dropInfo;
            UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, nullptr, 0);
            dropInfo.filePaths.resize(fileCount);

            for (UINT i = 0; i < fileCount; ++i) {
                wchar_t filePath[MAX_PATH];
                DragQueryFile(hDrop, i, filePath, MAX_PATH);
                dropInfo.filePaths[i] = filePath;
            }

            POINT pt;
            DragQueryPoint(hDrop, &pt);
            ScreenToClient(hwnd, &pt);
            dropInfo.mouseX = pt.x / charWidth; // Convert to character column
            dropInfo.mouseY = pt.y / charHeight; // Convert to character row

            OnFileDrop(dropInfo);
            DragFinish(hDrop);
            return 0;
        }

        case WM_EXITSIZEMOVE: {
            ResizeCompleteInfo info;
            RECT rect;
            ::GetWindowRect(hwnd, &rect);
            info.newWidth = (rect.right - rect.left) / charWidth;
            info.newHeight = (rect.bottom - rect.top) / charHeight;

            OnResizeComplete(info);

            //Should the client area automatically be invalidated?

            return 0;
        }
        case WM_CLOSE: {
            CloseRequestInfo info;
            info.canCancel = true;

            if (OnCloseRequest(info)) {
                DestroyWindow(hwnd);
            }
            return 0;
        }
        case WM_MOUSEWHEEL: {
            MouseScrollInfo info;
            info.scrollAmount = GET_WHEEL_DELTA_WPARAM(WParam) / WHEEL_DELTA;

            OnMouseScroll(info);
            return 0;
        }
        case WM_CHAR: {
            TextInputInfo info;
            info.inputText = std::wstring(1, static_cast<wchar_t>(WParam));

            OnTextInput(info);
            return 0;
        }
        case WM_SETFOCUS: {
            FocusGainedInfo info;

            OnFocusGained(info);
            return 0;
        }
        case WM_KILLFOCUS: {
            FocusLostInfo info;

            OnFocusLost(info);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(Hwnd, &ps);

            // Get the dimensions of the client area
            RECT clientRect;
            GetClientRect(Hwnd, &clientRect);

            // Calculate buffer size (in characters)
            uint32_t width = (clientRect.right - clientRect.left) / charWidth;
            uint32_t height = (clientRect.bottom - clientRect.top) / charHeight;

            // Ensure buffer size matches the screen area and is initialized
            mtx.lock();
            if ((!lastBuffer) || lastBufferSize.width != width || lastBufferSize.height != height) {
                delete[] lastBuffer;
                lastBufferSize = { width, height };
                lastBuffer = new BufferGridCell[width * height];
            }
            PaintBuffer(width, height, lastBuffer);
            mtx.unlock();

            // Monospaced font for rendering
            HFONT hFont = CreateFontW(
                charHeight,          // Character height
                charWidth,           // Character width
                0,                   // Escapement
                0,                   // Orientation
                FW_NORMAL,           // Weight (normal)
                FALSE,               // Italic
                FALSE,               // Underline
                FALSE,               // Strikeout
                ANSI_CHARSET,        // Character set
                OUT_DEFAULT_PRECIS,  // Output precision
                CLIP_DEFAULT_PRECIS, // Clipping precision
                DEFAULT_QUALITY,     // Output quality
                FIXED_PITCH | FF_MODERN,  // Pitch and family (monospaced)
                L"Consolas"          // Font name
            );
            SelectObject(hdc, hFont);

            // Draw each character in the buffer grid
            for (uint32_t y = 0; y < height; ++y) {
                for (uint32_t x = 0; x < width; ++x) {
                    const BufferGridCell& cell = lastBuffer[y * width + x];
                    SetTextColor(hdc, cell.forecolor);
                    SetBkColor(hdc, cell.backcolor);

                    wchar_t charBuffer[2] = { cell.character, L'\0' };
                    TextOutW(hdc, x * charWidth, y * charHeight, charBuffer, 1);
                }
            }

            DeleteObject(hFont);
            EndPaint(Hwnd, &ps);
            return 0;
        }
        default:
            return DefWindowProc(Hwnd, Msg, WParam, LParam);
        }
    }

    WindowBase::WindowBase(HINSTANCE HInstance)
        : hInstance(HInstance), stopThread(false), allowTransparentBackgrounds(false), lastBuffer(0), lastBufferSize(0, 0), lastWindowState(WindowState::Hidden) {

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

        if (!lastBuffer) return 0;

        btui::BufferSize bufSize = lastBufferSize;
        BufferGridCell* outBuffer = new BufferGridCell[bufSize.width * bufSize.height];
        memcpy(outBuffer, lastBuffer, sizeof(BufferGridCell) * bufSize.width * bufSize.height);

        return outBuffer;
    }
    bool WindowBase::CopyBufferOut(btui::BufferSize BufferSize, BufferGridCell* Buffer) {
        std::lock_guard<std::mutex> lock(mtx);

        if (!lastBuffer) return 0;

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
    WindowState WindowBase::GetWindowState() const {
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
    void WindowBase::SetWindowState(WindowState State) {
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