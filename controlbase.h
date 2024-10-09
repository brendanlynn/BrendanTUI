#pragma once

#include "windowbase.h"
#include <variant>

namespace btui {
    class Control;

    struct MouseClickControlInfo {
        union {
            struct {
                uint32_t x;
                uint32_t y;
            };
            PointU32 point;
        };
    };
    struct MouseEnterControlInfo {
        union {
            struct {
                uint32_t x;
                uint32_t y;
            };
            PointU32 point;
        };
    };
    struct MouseMoveControlInfo {
        union {
            struct {
                uint32_t x;
                uint32_t y;
            };
            PointU32 point;
        };
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
        union {
            struct {
                uint32_t mouseX; //column of the char the mouse was on when it dropped the file
                uint32_t mouseY; //row of the char the mouse was on when it dropped the file
            };
            PointU32 point;
        };
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
        virtual void DrawControl(BufferGrid Buffer, RectU32 Partition) = 0;

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

    using backgroundFill_t = std::variant<std::monostate, BufferGridCell, uint32_t>;

    void OverwriteWithBackgroundFill(BufferGridCell& Cell, const backgroundFill_t& BackgroundFill);
}