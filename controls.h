#pragma once

#include "controlbase.h"

namespace btui {
    namespace controls {
        // For reference (borders and table style):
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
            uint32_t backgroundColor;
            SizeU32 lastPartSize;

        protected:
            virtual void DrawControl(BufferGrid Buffer, RectU32 Partition) override;

        public:
            PointU32 ControlCoordsToCanvasCoords(PointU32 ControlCoords);
            PointU32 CanvasCoordsToControlCoords(PointU32 CanvasCoords);

            void CopyInCanvas(const BufferGrid& NewBuffer);
            BufferGrid CopyOutCanvas();
            BufferGrid ExchangeCanvas(const BufferGrid& NewBuffer);

            Align GetHorizontalAlign();
            void SetHorizontalAlign(Align NewAlign);
            Align ExchangeHorizontalAlign(Align NewAlign);

            Align GetVerticalAlign();
            void SetVerticalAlign(Align NewAlign);
            Align ExchangeVerticalAlign(Align NewAlign);

            uint32_t GetBackgroundColor();
            void SetBackgroundColor(uint32_t NewColor);
            uint32_t ExchangeBackgroundColor(uint32_t NewColor);
        };
    }
}