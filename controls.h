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

        class Canvas : Control {
            std::mutex mtx;

            BufferGrid buffer;
            Align horizontalAlign;
            Align verticalAlign;
            backgroundFill_t backgroundFill;
            SizeU32 lastPartSize;

            void DrawControl(BufferGrid Buffer, RectU32 Partition) override final;

        public:
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