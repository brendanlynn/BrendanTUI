#pragma once

#include "controlbase.h"
#include "drawing.h"

namespace btui {
    namespace controls {
        // For reference, (borders and table style)
        //
        // ┌────────────┬────────────┐
        // │ Cell 1.1   │ Cell 1.2   │
        // ├────────────┼────────────┤
        // │ Cell 2.1   │ Cell 2.2   │
        // ├────────────┼────────────┤
        // │ Cell 3.1   │ Cell 3.2   │
        // └────────────┴────────────┘

        class Canvas : public Control {
            BufferGrid buffer;
            Align horizontalAlign;
            Align verticalAlign;
            backgroundFill_t backgroundFill;
            SizeU32 lastPartSize;

        protected:
            PointU32 ControlCoordsToCanvasCoordsNoLock(PointU32 ControlCoords);
            PointU32 CanvasCoordsToControlCoordsNoLock(PointU32 CanvasCoords);

            virtual void CopyInCanvasNoLock(const BufferGrid& NewBuffer);
            BufferGrid CopyOutCanvasNoLock();
            virtual BufferGrid ExchangeCanvasNoLock(const BufferGrid& NewBuffer);

            Align GetHorizontalAlignNoLock();
            virtual void SetHorizontalAlignNoLock(Align NewAlign);

            Align GetVerticalAlignNoLock();
            virtual void SetVerticalAlignNoLock(Align NewAlign);

            backgroundFill_t GetBackgroundFillNoLock();
            virtual void SetBackgroundFillNoLock(backgroundFill_t NewFill);

            virtual void DrawControl(BufferGrid Buffer, RectU32 Partition) override;

        public:
            Canvas(FocusManager* FocusManager, std::function<void()> InvalidateFunc);

            PointU32 ControlCoordsToCanvasCoords(PointU32 ControlCoords);
            PointU32 CanvasCoordsToControlCoords(PointU32 CanvasCoords);

            void CopyInCanvas(const BufferGrid& NewBuffer);
            BufferGrid CopyOutCanvas();
            BufferGrid ExchangeCanvas(const BufferGrid& NewBuffer);

            Align GetHorizontalAlign();
            void SetHorizontalAlign(Align NewAlign);

            Align GetVerticalAlign();
            void SetVerticalAlign(Align NewAlign);

            backgroundFill_t GetBackgroundFill();
            void SetBackgroundFill(backgroundFill_t NewFill);
        };

        class Label : public Control {
            std::wstring text;
            uint32_t textBackcolor;
            uint32_t textForecolor;
            Align textHorizontalAlign;
            Align textVerticalAlign;
            WrapStyle textWrapStyle;
            backgroundFill_t backgroundFill;

        protected:
            std::wstring GetTextNoLock();
            virtual void SetTextNoLock(std::wstring NewText);

            uint32_t GetTextBackcolorNoLock();
            virtual void SetTextBackcolorNoLock(uint32_t NewBackcolor);

            uint32_t GetTextForecolorNoLock();
            virtual void SetTextForecolorNoLock(uint32_t NewForecolor);

            Align GetTextHorizontalAlignNoLock();
            virtual void SetTextHorizontalAlignNoLock(Align NewAlign);

            Align GetTextVerticalAlignNoLock();
            virtual void SetTextVerticalAlignNoLock(Align NewAlign);

            WrapStyle GetTextWrapStyleNoLock();
            virtual void SetTextWrapStyleNoLock(WrapStyle WrapStyle);

            backgroundFill_t GetBackgroundFillNoLock();
            virtual void SetBackgroundFillNoLock(backgroundFill_t NewFill);

            virtual void DrawControl(BufferGrid Buffer, RectU32 Partition) override;

        public:
            Label(FocusManager* FocusManager, std::function<void()> InvalidateFunc);

            std::wstring GetText();
            void SetText(std::wstring NewText);

            uint32_t GetTextBackcolor();
            void SetTextBackcolor(uint32_t NewBackcolor);

            uint32_t GetTextForecolor();
            void SetTextForecolor(uint32_t NewForecolor);

            Align GetTextHorizontalAlign();
            void SetTextHorizontalAlign(Align NewAlign);

            Align GetTextVerticalAlign();
            void SetTextVerticalAlign(Align NewAlign);

            WrapStyle GetTextWrapStyle();
            void SetTextWrapStyle(WrapStyle WrapStyle);

            backgroundFill_t GetBackgroundFill();
            void SetBackgroundFill(backgroundFill_t NewFill);
        };

        enum ButtonState {
            ButtonStateReleased,
            ButtonStateMouseOver,
            ButtonStateCompressed,
        };

        class Button : public Label {
            ButtonState buttonState;
            backgroundFill_t fillReleased;
            backgroundFill_t fillMouseover;
            backgroundFill_t fillCompressed;

        protected:
            ButtonState GetButtonStateNoLock();
            virtual void SetButtonStateNoLock(ButtonState State);

            backgroundFill_t GetBackgroundFillReleasedNoLock();
            virtual void SetBackgroundFillReleasedNoLock(backgroundFill_t NewFill);

            backgroundFill_t GetBackgroundFillMouseoverNoLock();
            virtual void SetBackgroundFillMouseoverNoLock(backgroundFill_t NewFill);

            backgroundFill_t GetBackgroundFillCompressedNoLock();
            virtual void SetBackgroundFillCompressedNoLock(backgroundFill_t NewFill);

            virtual void SetBackgroundFillNoLock(backgroundFill_t NewFill) override;

            virtual void OnMouseDown(const MouseDownControlInfo& Info) override;
            virtual void OnMouseUp(const MouseUpControlInfo& Info) override;
            virtual void OnMouseEnter(const MouseEnterControlInfo& Info) override;
            virtual void OnMouseExit(const MouseExitControlInfo& Info) override;

            virtual void SetButtonState(ButtonState State);

        public:
            Button(FocusManager* FocusManager, std::function<void()> InvalidateFunc);

            ButtonState GetButtonState();

            backgroundFill_t GetBackgroundFillReleased();
            void SetBackgroundFillReleased(backgroundFill_t NewFill);

            backgroundFill_t GetBackgroundFillMouseover();
            void SetBackgroundFillMouseover(backgroundFill_t NewFill);

            backgroundFill_t GetBackgroundFillCompressed();
            void SetBackgroundFillCompressed(backgroundFill_t NewFill);
        };
    }
}