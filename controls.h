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

            virtual void DrawControl(BufferGrid Buffer, RectU32 Partition) override;

        public:
            Canvas(FocusManager* FocusManager, std::function<void()> InvalidateFunc, BufferGrid Buffer = BufferGrid(), Align HorizontalAlign = Align::Middle, Align VerticalAlign = Align::Middle, backgroundFill_t BackgroundFill = std::monostate{});

            PointU32 ControlCoordsToCanvasCoords(PointU32 ControlCoords);
            PointU32 CanvasCoordsToControlCoords(PointU32 CanvasCoords);

            virtual void CopyInCanvas(const BufferGrid& NewBuffer);
            BufferGrid CopyOutCanvas();
            virtual BufferGrid ExchangeCanvas(const BufferGrid& NewBuffer);

            Align GetHorizontalAlign();
            virtual void SetHorizontalAlign(Align NewAlign);

            Align GetVerticalAlign();
            virtual void SetVerticalAlign(Align NewAlign);

            backgroundFill_t GetBackgroundFill();
            virtual void SetBackgroundFill(backgroundFill_t NewFill);
        };

        class Label : public Control {
            std::wstring text;
            uint32_t textBackcolor;
            uint32_t textForecolor;
            Align textHorizontalAlign;
            Align textVerticalAlign;
            bool textWrap;
            bool textStretch;
            backgroundFill_t backgroundFill;
            
        protected:
            std::mutex mtx;

            virtual void DrawControl(BufferGrid Buffer, RectU32 Partition) override;

        public:
            inline Label(FocusManager* FocusManager, std::function<void()> InvalidateFunc, std::wstring Text = L"", uint32_t TextBackcolor = 0xFF000000, uint32_t TextForecolor = 0xFFFFFFFF, Align TextHorizontalAlign = Align::Middle, Align TextVerticalAlign = Align::Middle, bool TextWrap = true, bool TextStretch = true, backgroundFill_t BackgroundFill = std::monostate{})
                : Control(FocusManager, InvalidateFunc), text(Text), textBackcolor(TextBackcolor), textForecolor(TextForecolor), textHorizontalAlign(TextHorizontalAlign), textVerticalAlign(TextVerticalAlign), textWrap(TextWrap), textStretch(TextStretch), backgroundFill(BackgroundFill) { }

            std::wstring GetText();
            virtual void SetText(const std::wstring& NewText);

            uint32_t GetTextBackcolor();
            virtual void SetTextBackcolor(uint32_t NewBackcolor);

            uint32_t GetTextForecolor();
            virtual void SetTextForecolor(uint32_t NewForecolor);

            Align GetTextHorizontalAlign();
            virtual void SetTextHorizontalAlign(Align NewAlign);

            Align GetTextVerticalAlign();
            virtual void SetTextVerticalAlign(Align NewAlign);

            bool GetTextWrap();
            virtual void SetTextWrap(bool Wrap);

            bool GetTextStretch();
            virtual void SetTextStretch(bool Stretch);

            backgroundFill_t GetBackgroundFill();
            virtual void SetBackgroundFill(backgroundFill_t NewFill);
        };

        class Button : public Label {
            bool isCompressed;
            backgroundFill_t fillCompressed;
            backgroundFill_t fillDecompressed;

        protected:
            void SetCompressed(bool Compressed);
            
        public:
            Button(FocusManager* FocusManager, std::function<void()> InvalidateFunc, std::wstring Text = L"", uint32_t TextBackcolor = 0xFF000000, uint32_t TextForecolor = 0xFFFFFFFF, Align TextHorizontalAlign = Align::Middle, Align TextVerticalAlign = Align::Middle, bool TextWrap = true, bool TextStretch = true, backgroundFill_t BackgroundFillCompressed = std::monostate{}, backgroundFill_t BackgroundFillDecompressed = std::monostate{})
                : Label(FocusManager, InvalidateFunc, Text, TextBackcolor, TextForecolor, TextHorizontalAlign, TextVerticalAlign, TextWrap, TextStretch, BackgroundFillDecompressed), isCompressed(false), fillCompressed(BackgroundFillCompressed), fillDecompressed(BackgroundFillDecompressed) { }

            bool IsCompressed();

            void SetBackgroundFill(backgroundFill_t NewFill) override final; // Sets both.

            backgroundFill_t GetBackgroundFillCompressed();
            void SetBackgroundFillCompressed(backgroundFill_t NewFill);

            backgroundFill_t GetBackgroundFillDecompressed();
            void SetBackgroundFillDecompressed(backgroundFill_t NewFill);
        };
    }
}