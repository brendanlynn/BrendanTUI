﻿#pragma once

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
            virtual BufferGrid CopyOutCanvas();
            virtual BufferGrid ExchangeCanvas(const BufferGrid& NewBuffer);

            virtual Align GetHorizontalAlign();
            virtual void SetHorizontalAlign(Align NewAlign);

            virtual Align GetVerticalAlign();
            virtual void SetVerticalAlign(Align NewAlign);

            virtual backgroundFill_t GetBackgroundFill();
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

            virtual std::wstring GetText();
            virtual void SetText(const std::wstring& NewText);

            virtual uint32_t GetTextBackcolor();
            virtual void SetTextBackcolor(uint32_t NewBackcolor);

            virtual uint32_t GetTextForecolor();
            virtual void SetTextForecolor(uint32_t NewForecolor);

            virtual Align GetTextHorizontalAlign();
            virtual void SetTextHorizontalAlign(Align NewAlign);

            virtual Align GetTextVerticalAlign();
            virtual void SetTextVerticalAlign(Align NewAlign);

            virtual bool GetTextWrap();
            virtual void SetTextWrap(bool Wrap);

            virtual bool GetTextStretch();
            virtual void SetTextStretch(bool Stretch);

            virtual backgroundFill_t GetBackgroundFill();
            virtual void SetBackgroundFill(backgroundFill_t NewFill);
        };

        class Button : public Label {
            bool isCompressed;
            backgroundFill_t fillCompressed;
            backgroundFill_t fillUncompressed;

        public:
            Button(FocusManager* FocusManager, std::function<void()> InvalidateFunc, std::wstring Text = L"", uint32_t TextBackcolor = 0xFF000000, uint32_t TextForecolor = 0xFFFFFFFF, Align TextHorizontalAlign = Align::Middle, Align TextVerticalAlign = Align::Middle, bool TextWrap = true, bool TextStretch = true, backgroundFill_t BackgroundFillCompressed = std::monostate{}, backgroundFill_t BackgroundFillUncompressed = std::monostate{})
                : Label(FocusManager, InvalidateFunc, Text, TextBackcolor, TextForecolor, TextHorizontalAlign, TextVerticalAlign, TextWrap, TextStretch, BackgroundFillUncompressed), isCompressed(false), fillCompressed(BackgroundFillCompressed), fillUncompressed(BackgroundFillUncompressed) { }

            bool IsCompressed();

            void SetBackgroundFill(backgroundFill_t NewFill) override; // Sets both.

            backgroundFill_t GetBackgroundFillCompressed();
            void SetBackgroundFillCompressed(backgroundFill_t NewSurrounding);

            backgroundFill_t GetBackgroundFillUncompressed();
            void SetBackgroundFillUncompressed(backgroundFill_t NewSurrounding);
        };
    }
}