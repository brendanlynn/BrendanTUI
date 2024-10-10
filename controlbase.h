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

        constexpr inline MouseClickControlInfo()
            : point() { }
    };
    struct MouseEnterControlInfo {
        union {
            struct {
                uint32_t x;
                uint32_t y;
            };
            PointU32 point;
        };

        constexpr inline MouseEnterControlInfo()
            : point() { }
    };
    struct MouseMoveControlInfo {
        union {
            struct {
                uint32_t x;
                uint32_t y;
            };
            PointU32 point;
        };

        constexpr inline MouseMoveControlInfo()
            : point() { }
    };
    struct MouseExitControlInfo { };
    struct FocusGainedControlInfo {
        Control* lastBeholder;

        constexpr inline FocusGainedControlInfo()
            : lastBeholder(0) { }
    };
    struct FocusLostControlInfo {
        Control* newBeholder;

        constexpr inline FocusLostControlInfo()
            : newBeholder(0) { }
    };
    struct MouseScrollControlInfo {
        int32_t scrollAmount;

        constexpr inline MouseScrollControlInfo()
            : scrollAmount(0) { }
    };
    struct TextInputControlInfo {
        std::wstring inputText;

        constexpr inline TextInputControlInfo()
            : inputText(L"") { }
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

        constexpr inline FileDropControlInfo()
            : point() { }
    };

    class FocusManager {
        std::atomic<Control*> focusedControl;
    public:
        inline FocusManager()
            : focusedControl(0) { }

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

    enum WrapStyle {
        NoWrap,
        WrapByChar,
        WrapByWord,
        WrapByWordAndStretch
    };

    using backgroundFill_t = std::variant<std::monostate, BufferGridCell, uint32_t>;

    void OverwriteWithBackgroundFill(BufferGridCell& Cell, const backgroundFill_t& BackgroundFill);
}