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
            std::mutex mtx;

            BufferGrid buffer;
            Align horizontalAlign;
            Align verticalAlign;
            backgroundFill_t backgroundFill;
            SizeU32 lastPartSize;

        protected:
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
        //class Label : Control {
        //    std::mutex mtx;
        //
        //    std::wstring text;
        //    uint32_t textBackcolor;
        //    uint32_t textForecolor;
        //    Align textHorizontalAlign;
        //    Align textVerticalAlign;
        //    backgroundFill_t surrounding;
        //};
        //class Button : Control {
        //    std::mutex mtx;
        //
        //    std::wstring text;
        //    uint32_t textBackcolor;
        //    uint32_t textForecolor;
        //    Align textHorizontalAlign;
        //    Align textVerticalAlign;
        //    backgroundFill_t surroundingSelected;
        //    backgroundFill_t surroundingUnselected;
        //};
        //class Table : Control {
        //    std::mutex mtx;
        //
        //    uint32_t columnCount;
        //    uint32_t rowCount;
        // 
        //    // Other stuff, maybe? Merged cells?
        //    // Pointers to child controls? (NOT
        //    // UNIQUE POINTERS!!!, just Control*-
        //    // pointers. :). Something else will
        //    // have ownership/responsibility.)
        //};
    }
}