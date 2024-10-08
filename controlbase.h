#pragma once

#include "windowbase.h"

namespace btui {
    class Control;

    struct MouseClickControlInfo {
        uint32_t x;
        uint32_t y;
    };
    struct MouseEnterControlInfo {
        uint32_t x;
        uint32_t y;
    };
    struct MouseMoveControlInfo {
        uint32_t x;
        uint32_t y;
    };
    struct MouseExitControlInfo { };
    struct FocusGainedControlInfo {
        Control* lastBeholder;
    };
    struct FocusLostControlInfo {
        Control* newBeholder;
    };
    struct MouseScrollControlInfo {
        int32_t scrollAmount;
    };
    struct TextInputControlInfo {
        std::wstring inputText;
    };
    struct FileDropControlInfo {
        std::vector<std::wstring> filePaths;
        uint32_t mouseX; //column of the char the mouse was on when it dropped the file
        uint32_t mouseY; //row of the char the mouse was on when it dropped the file
    };

    class FocusManager {
        std::atomic<Control*> focusedControl;
    public:
        virtual Control* GetFocusedControl();
        virtual void SetFocusedControl(Control* NewFocus);
    };

    class Control {
        FocusManager* focusManager;
        std::function<void()> invalidateFunc;

    protected:
        Control(FocusManager* FocusManager, std::function<void()> InvalidateFunc)
            : focusManager(FocusManager), invalidateFunc(std::move(InvalidateFunc)) { }

        FocusManager* GetFocusManager() {
            return focusManager;
        }
        void CallInvalidate() {
            invalidateFunc();
        }
    public:
        virtual void DrawControl(WindowBuffer Buffer, BufferPartition Partition) = 0;

        virtual void OnFocusGained(const FocusGainedControlInfo& Info) { }
        virtual void OnFocusLost(const FocusLostControlInfo& Info) { }
        virtual void OnMouseClick(const MouseClickControlInfo& Info) { }
        virtual void OnMouseEnter(const MouseEnterControlInfo& Info) { }
        virtual void OnMouseMove(const MouseMoveControlInfo& Info) { }
        virtual void OnMouseExit(const MouseExitControlInfo& Info) { }
        virtual void OnKeyPress(const MouseScrollControlInfo& Info) { };
        virtual void OnTextInput(const TextInputControlInfo& Info) { }
        virtual void OnMouseScroll(const MouseScrollControlInfo& Info) { };
        virtual void OnFileDrop(const FileDropControlInfo& Info) { };
    };

    enum Align {
        Start,
        Middle,
        End
    };
}