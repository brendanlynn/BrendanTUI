#include <brendantui/windowbase.h>

#include <combaseapi.h>
#include <shellapi.h>
#include <windows.h>

constexpr uint32_t charWidth = 10;
constexpr uint32_t charHeight = 15;

std::wstring GenerateGuidStr() {
    GUID guid;
    CoCreateGuid(&guid);

    wchar_t buffer[41];
    swprintf_s(buffer, 41, L"%08x-%04x-%04x-%04x-%012llx",
        guid.Data1, guid.Data2, guid.Data3,
        (guid.Data4[0] << 8) | guid.Data4[1],
        *(reinterpret_cast<unsigned long long*>(&guid.Data4[2])));

    return std::wstring(buffer);
}

LRESULT CALLBACK WindowProcStaticPlaceholder(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam) {
    return DefWindowProc(Hwnd, Msg, WParam, LParam);
}

inline int GetLParamX(LPARAM lParam) {
    return (int)(short)LOWORD(lParam);
}
inline int GetLParamY(LPARAM lParam) {
    return (int)(short)HIWORD(lParam);
}

inline COLORREF ConvertToColorref(uint32_t Color) {
    return RGB((Color >> 16) & 255, (Color >> 8) & 255, Color & 255);
}

namespace btui {
    void WindowBase::UpdateFunction(bool* Initialized) {
        WNDCLASSEXW wc = {};
        wc.hInstance = hInstance;
        wc.lpszClassName = className.c_str();
        wc.lpfnWndProc = WindowProcStaticPlaceholder;

        RegisterClassExW(&wc);

        hwnd = CreateWindowExW(
            0,                   // Optional window styles
            className.c_str(),   // Window class
            L"",                 // Window title
            WS_OVERLAPPEDWINDOW, // Window style
            CW_USEDEFAULT,       // Window initial X
            CW_USEDEFAULT,       // Window initial Y
            CW_USEDEFAULT,       // Window initial width
            CW_USEDEFAULT,       // Window initial height
            nullptr,             // Parent window
            nullptr,             // Menu
            hInstance,           // Instance handle
            nullptr              // Additional application data
        );

        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProcStatic));

        *Initialized = true;

        MSG msg;
        while (isRunning.load()) {
            while (PeekMessageW(&msg, hwnd, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            ProcessTasks();

            std::this_thread::yield();
        }

        CancelTasks();

        {
            std::lock_guard<std::mutex> lock(mtx);

            if (hwnd) DestroyWindow(hwnd);

            UnregisterClassW(className.c_str(), hInstance);

            hwnd = 0;

            delete[] lastBuffer;
            lastBuffer = 0;
        }

        DisposedInfo info;

        OnDisposed(info);
    }
    void WindowBase::ProcessTasks() {
        std::unique_lock<std::mutex> lock(mtx);
        while (!taskQueue.empty()) {
            auto task = std::move(taskQueue.front());
            taskQueue.pop();
            lock.unlock();
            task.Run();
            lock.lock();
        }
    }
    void WindowBase::CancelTasks() {
        std::lock_guard<std::mutex> lock(mtx);
        while (!taskQueue.empty()) {
            auto task = std::move(taskQueue.front());
            taskQueue.pop();
            task.Cancel();
        }
        taskQueue = std::queue<details::QueuedTask>();
    }
    bool WindowBase::InvokeOnWindowThread(std::function<void()> Func) {
        if (std::this_thread::get_id() == updateThread.get_id()) {
            Func();
            return true;
        }
        else {
            int statusCode = 0;

            details::QueuedTask task(std::move(Func), &statusCode);

            {
                std::lock_guard<std::mutex> lock(mtx);

                if (!isRunning) return false;

                taskQueue.push(std::move(task));
            }

            while (!statusCode) std::this_thread::yield();

            return statusCode == 1;
        }
    }

    LRESULT CALLBACK WindowBase::WindowProcStatic(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam) {
        WindowBase* pThis = reinterpret_cast<WindowBase*>(GetWindowLongPtr(Hwnd, GWLP_USERDATA));

        return pThis->WindowProc(Hwnd, Msg, WParam, LParam);
    }
    LRESULT WindowBase::WindowProc(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam) {
        switch (Msg) {
        case WM_SETCURSOR:
            if (LOWORD(LParam) == HTCLIENT) {
                CursorType cursorTypeLocal;
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    cursorTypeLocal = cursorType;
                }

                HCURSOR hCursor;
                switch (cursorType) {
                case CursorTypeArrow:
                    hCursor = LoadCursor(NULL, IDC_ARROW);
                    break;
                case CursorTypeHand:
                    hCursor = LoadCursor(NULL, IDC_HAND);
                    break;
                case CursorTypeIBeam:
                    hCursor = LoadCursor(NULL, IDC_IBEAM);
                    break;
                case CursorTypeCrosshairs:
                    hCursor = LoadCursor(NULL, IDC_CROSS);
                    break;
                case CursorTypeNo:
                    hCursor = LoadCursor(NULL, IDC_NO);
                    break;
                case CursorTypeResizeNS:
                    hCursor = LoadCursor(NULL, IDC_SIZENS);
                    break;
                case CursorTypeResizeEW:
                    hCursor = LoadCursor(NULL, IDC_SIZEWE);
                    break;
                case CursorTypeResizeNESW:
                    hCursor = LoadCursor(NULL, IDC_SIZENESW);
                    break;
                case CursorTypeResizeNWSE:
                    hCursor = LoadCursor(NULL, IDC_SIZENWSE);
                    break;
                case CursorTypeResizeOmni:
                    hCursor = LoadCursor(NULL, IDC_SIZEALL);
                    break;
                case CursorTypeStarting:
                    hCursor = LoadCursor(NULL, IDC_APPSTARTING);
                    break;
                case CursorTypeWait:
                    hCursor = LoadCursor(NULL, IDC_WAIT);
                    break;
                case CursorTypeHelp:
                    hCursor = LoadCursor(NULL, IDC_HELP);
                    break;
                case CursorTypeUpArrow:
                    hCursor = LoadCursor(NULL, IDC_UPARROW);
                    break;
                default:
                    hCursor = LoadCursor(NULL, IDC_ICON);
                    break;
                }

                SetCursor(hCursor);

                return 0;
            }
            break;
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

            MouseDownInfo info;
            info.x = mousePt.x / charWidth;
            info.y = mousePt.y / charHeight;
            info.leftButton = (Msg == WM_LBUTTONDOWN);
            info.rightButton = (Msg == WM_RBUTTONDOWN);

            OnMouseDown(info);
            return 0;
        }
        case WM_LBUTTONUP:
        case WM_RBUTTONUP: {
            POINT mousePt;
            mousePt.x = GetLParamX(LParam);
            mousePt.y = GetLParamY(LParam);
            ScreenToClient(hwnd, &mousePt);

            MouseUpInfo info;
            info.x = mousePt.x / charWidth;
            info.y = mousePt.y / charHeight;
            info.leftButton = (Msg == WM_LBUTTONUP);
            info.rightButton = (Msg == WM_RBUTTONUP);

            OnMouseUp(info);
            return 0;
        }
        case WM_MOUSEMOVE: {
            POINT mousePt;
            mousePt.x = GetLParamX(LParam);
            mousePt.y = GetLParamY(LParam);

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);

            if (mousePt.x < clientRect.left || mousePt.y < clientRect.top) {
                if (mouseContained) {
                    mouseContained = false;

                    MouseExitInfo info;

                    OnMouseExit(info);
                }

                return 0;
            }

            uint32_t charX = (mousePt.x - clientRect.left) / charWidth;
            uint32_t charY = (mousePt.y - clientRect.top) / charHeight;
            uint32_t bufWidth = (clientRect.right - clientRect.left) / charWidth;
            uint32_t bufHeight = (clientRect.bottom - clientRect.top) / charHeight;

            if (charX >= bufWidth || charY >= bufHeight) {
                if (mouseContained) {
                    mouseContained = false;

                    MouseExitInfo info;

                    OnMouseExit(info);
                }

                return 0;
            }

            if (mouseContained) {
                MouseMoveInfo info;
                info.x = charX;
                info.y = charY;

                OnMouseMove(info);
            }
            else {
                mouseContained = true;

                MouseEnterInfo info;
                info.x = charX;
                info.y = charY;

                OnMouseEnter(info);
            }

            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hwnd;
            tme.dwHoverTime = 0;
            TrackMouseEvent(&tme);

            return 0;
        }
        case WM_MOUSELEAVE: {
            if (mouseContained) {
                mouseContained = false;

                MouseExitInfo info;

                OnMouseExit(info);
            }

            return 0;
        }
        case WM_SIZE: {
            ResizeInfo info;
            info.newWidth = LOWORD(LParam) / charWidth;
            info.newHeight = HIWORD(LParam) / charHeight;

            OnResize(info);

            Invalidate();

            return 0;
        }
        case WM_SHOWWINDOW: {
            WindowState newState = WParam ? GetWindowState() : WindowStateHidden;

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
            UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
            dropInfo.filePaths.resize(fileCount);

            for (UINT i = 0; i < fileCount; ++i) {
                wchar_t filePath[MAX_PATH];
                DragQueryFileW(hDrop, i, filePath, MAX_PATH);
                dropInfo.filePaths[i] = filePath;
            }

            POINT pt;
            DragQueryPoint(hDrop, &pt);
            ScreenToClient(hwnd, &pt);
            dropInfo.mouseX = pt.x / charWidth;
            dropInfo.mouseY = pt.y / charHeight;

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

            return 0;
        }
        case WM_CLOSE: {
            CloseRequestInfo info;
            info.canCancel = true;

            if (OnCloseRequest(info)) {
                isRunning.store(false);
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
            isRunning.store(false);
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(Hwnd, &ps);

            // Get the dimensions of the client area
            RECT clientRect;
            GetClientRect(Hwnd, &clientRect);

            // Create a memory device context
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

            // Fill the background in the memory DC
            FillRect(memDC, &clientRect, CreateSolidBrush(ConvertToColorref(backColor)));

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
            PaintBuffer(BufferGrid(width, height, lastBuffer));
            mtx.unlock();

            // Monospaced font for rendering
            HFONT hFont = CreateFontW(
                charHeight,              // Character height
                charWidth,               // Character width
                0,                       // Escapement
                0,                       // Orientation
                FW_NORMAL,               // Weight (normal)
                FALSE,                   // Italic
                FALSE,                   // Underline
                FALSE,                   // Strikeout
                ANSI_CHARSET,            // Character set
                OUT_DEFAULT_PRECIS,      // Output precision
                CLIP_DEFAULT_PRECIS,     // Clipping precision
                DEFAULT_QUALITY,         // Output quality
                FIXED_PITCH | FF_MODERN, // Pitch and family (monospaced)
                L"Consolas"              // Font name
            );
            SelectObject(memDC, hFont);

            // Draw each character in the buffer grid to the memory DC
            for (uint32_t y = 0; y < height; ++y) {
                for (uint32_t x = 0; x < width; ++x) {
                    const BufferGridCell& cell = lastBuffer[y * width + x];
                    SetTextColor(memDC, ConvertToColorref(cell.forecolor));
                    SetBkColor(memDC, ConvertToColorref(cell.backcolor));

                    wchar_t charBuffer[2] = { cell.character, L'\0' };
                    TextOutW(memDC, x * charWidth, y * charHeight, charBuffer, 1);
                }
            }

            // BitBlt the memory DC to the window's DC
            BitBlt(hdc, 0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, memDC, 0, 0, SRCCOPY);

            // Cleanup
            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);
            DeleteObject(hFont);
            EndPaint(Hwnd, &ps);
            return 0;
        }
        }
        return DefWindowProc(Hwnd, Msg, WParam, LParam);
    }

    WindowBase::WindowBase(HINSTANCE HInstance)
        : hInstance(HInstance), isRunning(true), lastBuffer(0), lastBufferSize(0, 0), lastWindowState(WindowStateHidden), mouseContained(false), backColor(0xFF000000), cursorType(CursorTypeArrow), isJoined(false) {

        className = GenerateGuidStr();

        bool initialized = false;
        updateThread = std::thread(&WindowBase::UpdateFunction, this, &initialized);
        while (!initialized) std::this_thread::yield();
    }
    WindowBase::~WindowBase() { Dispose(); }

    void WindowBase::Dispose() {
        isRunning.store(false);
        if (!isJoined.exchange(true)) {
            if (updateThread.joinable())
                updateThread.join();
        }
    }
    bool WindowBase::Running() const {
        return isRunning.load();
    }
    bool WindowBase::Disposed() const {
        return !isRunning.load();
    }

    SizeU32 WindowBase::BufferSize() {
        RECT winRect;
        bool success = InvokeOnWindowThread([this, &winRect]() {
            ::GetWindowRect(hwnd, &winRect);
        });

        if (success) {
            SizeU32 bufSize;
            bufSize.width = (winRect.right - winRect.left) / charWidth;
            bufSize.height = (winRect.bottom - winRect.top) / charHeight;
            return bufSize;
        }
        else return SizeU32(0, 0);
    }
    BufferGrid WindowBase::CopyBufferOut() {
        std::lock_guard<std::mutex> lock(mtx);

        if (!lastBuffer) return BufferGrid(0, 0, 0);

        SizeU32 bufSize = lastBufferSize;
        BufferGridCell* outBuffer = new BufferGridCell[bufSize.width * bufSize.height];
        memcpy(outBuffer, lastBuffer, sizeof(BufferGridCell) * bufSize.width * bufSize.height);

        return BufferGrid(bufSize, outBuffer);
    }
    bool WindowBase::CopyBufferOut(SizeU32 BufferSize, BufferGridCell* Buffer) {
        std::lock_guard<std::mutex> lock(mtx);

        if (!lastBuffer) return false;

        if (lastBufferSize != BufferSize) return false;

        memcpy(Buffer, lastBuffer, sizeof(BufferGridCell) * BufferSize.width * BufferSize.height);

        return true;
    }
    bool WindowBase::CopyBufferOut(BufferGrid Buffer) {
        return CopyBufferOut(Buffer.size, Buffer.buffer);
    }

    void WindowBase::Invalidate() {
        InvokeOnWindowThread([this]() {
            ::InvalidateRect(hwnd, NULL, FALSE);
        });
    }

    bool WindowBase::IsVisible() {
        bool visible;
        bool success = InvokeOnWindowThread([this, &visible]() {
            visible = ::IsWindowVisible(hwnd);
        });
        return success && visible;
    }
    void WindowBase::Show() {
        InvokeOnWindowThread([this]() {
            ::ShowWindow(hwnd, SW_SHOW);
            ::SetFocus(hwnd);
        });
    }
    void WindowBase::Hide() {
        InvokeOnWindowThread([this]() {
            ::ShowWindow(hwnd, SW_HIDE);
        });
    }

    bool WindowBase::IsMinimized() {
        bool val;
        bool success = InvokeOnWindowThread([this, &val]() {
            val = ::IsIconic(hwnd);
        });
        return success && val;
    }
    bool WindowBase::IsMaximized() {
        bool val;
        bool success = InvokeOnWindowThread([this, &val]() {
            val = ::IsZoomed(hwnd);
        });
        return success && val;
    }
    WindowState WindowBase::GetWindowState() {
        WindowState winState;
        bool success = InvokeOnWindowThread([this, &winState]() {
            if (!::IsWindowVisible(hwnd))
                winState = WindowStateHidden;
            else if (::IsIconic(hwnd))
                winState = WindowStateMinimized;
            else if (::IsZoomed(hwnd))
                winState = WindowStateMaximized;
            else
                winState = WindowStateRestored;
        });

        return success ? winState : WindowStateHidden;
    }
    bool WindowBase::HasFocus() {
        bool hasFocus;
        bool success = InvokeOnWindowThread([this, &hasFocus]() {
            hasFocus = ::GetFocus() == hwnd;
        });
        return success && hasFocus;
    }

    void WindowBase::Minimize() {
        InvokeOnWindowThread([this]() {
            ::ShowWindow(hwnd, SW_MINIMIZE);
        });
    }
    void WindowBase::Maximize() {
        InvokeOnWindowThread([this]() {
            ::ShowWindow(hwnd, SW_MAXIMIZE);
        });
    }
    void WindowBase::Restore() {
        InvokeOnWindowThread([this]() {
            ::ShowWindow(hwnd, SW_RESTORE);
        });
    }
    void WindowBase::SetWindowState(WindowState State) {
        switch (State) {
        case WindowStateHidden:
            Hide();
            break;
        case WindowStateMinimized:
            Minimize();
            break;
        case WindowStateMaximized:
            Maximize();
            break;
        case WindowStateRestored:
            Restore();
            break;
        }
    }
    void WindowBase::CaptureFocus() {
        InvokeOnWindowThread([this]() {
            ::SetFocus(hwnd);
        });
    }

    std::wstring WindowBase::GetTitle() {
        std::wstring title;
        bool success = InvokeOnWindowThread([this, &title]() {
            int len = ::GetWindowTextLengthW(hwnd);
            if (!len) {
                title = L"";
                return;
            }

            title = std::wstring(len, '\0');
            ::GetWindowTextW(hwnd, &title[0], title.size());
        });

        return success ? title : L"";
    }
    void WindowBase::SetTitle(std::wstring Title) {
        InvokeOnWindowThread([this, &Title]() {
            ::SetWindowTextW(hwnd, Title.c_str());
        });
    }

    uint32_t WindowBase::GetBackgroundColor() {
        std::lock_guard<std::mutex> lock(mtx);

        return backColor;
    }
    void WindowBase::SetBackgroundColor(uint32_t Color) {
        std::lock_guard<std::mutex> lock(mtx);

        backColor = Color;
    }

    CursorType WindowBase::GetCursorType() {
        std::lock_guard<std::mutex> lock(mtx);

        return cursorType;
    }
    void WindowBase::SetCursorType(CursorType Type) {
        std::lock_guard<std::mutex> lock(mtx);

        cursorType = Type;
    }
}
