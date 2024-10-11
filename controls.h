#pragma once

#include "controlbase.h"

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
            std::mutex mtx;

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
            Canvas(FocusManager* FocusManager, std::function<void()> InvalidateFunc, BufferGrid Buffer = BufferGrid(), Align HorizontalAlign = Align::Middle, Align VerticalAlign = Align::Middle, backgroundFill_t BackgroundFill = std::monostate{});

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
            std::mutex mtx;

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
            inline Label(FocusManager* FocusManager, std::function<void()> InvalidateFunc, std::wstring Text = L"", uint32_t TextBackcolor = 0xFF000000, uint32_t TextForecolor = 0xFFFFFFFF, Align TextHorizontalAlign = Align::Middle, Align TextVerticalAlign = Align::Middle, WrapStyle TextWrapStyle = WrapStyle::NoWrap, backgroundFill_t BackgroundFill = std::monostate{})
                : Control(FocusManager, InvalidateFunc), text(Text), textBackcolor(TextBackcolor), textForecolor(TextForecolor), textHorizontalAlign(TextHorizontalAlign), textVerticalAlign(TextVerticalAlign), textWrapStyle(TextWrapStyle), backgroundFill(BackgroundFill) { }

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
            Released,
            MouseOver,
            Compressed,
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
            Button(FocusManager* FocusManager, std::function<void()> InvalidateFunc, std::wstring Text = L"", uint32_t TextBackcolor = 0xFF000000, uint32_t TextForecolor = 0xFFFFFFFF, Align TextHorizontalAlign = Align::Middle, Align TextVerticalAlign = Align::Middle, WrapStyle TextWrapStyle = WrapStyle::NoWrap, backgroundFill_t BackgroundFillReleased = std::monostate{}, backgroundFill_t BackgroundFillMouseover = std::monostate{}, backgroundFill_t BackgroundFillCompressed = std::monostate{})
                : Label(FocusManager, InvalidateFunc, Text, TextBackcolor, TextForecolor, TextHorizontalAlign, TextVerticalAlign, TextWrapStyle, BackgroundFillReleased), buttonState(ButtonState::Released), fillReleased(BackgroundFillReleased), fillMouseover(BackgroundFillMouseover), fillCompressed(BackgroundFillCompressed) { }

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