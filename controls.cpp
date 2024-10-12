#include "pch.h"
#include "framework.h"

#include "controls.h"
#include <optional>

namespace btui {
    namespace controls {
        PointU32 Canvas::ControlCoordsToCanvasCoordsNoLock(PointU32 ControlCoords) {
            return MakeCanvasIntoFrameMappingInfo(buffer.size, lastPartSize, horizontalAlign, verticalAlign).FrameCoordsToCanvasCoords(ControlCoords);
        }
        PointU32 Canvas::CanvasCoordsToControlCoordsNoLock(PointU32 CanvasCoords) {
            return MakeCanvasIntoFrameMappingInfo(buffer.size, lastPartSize, horizontalAlign, verticalAlign).CanvasCoordsToFrameCoords(CanvasCoords);
        }

        void Canvas::CopyInCanvasNoLock(const BufferGrid& NewBuffer) {
            BufferGrid oldBuffer = ExchangeCanvasNoLock(NewBuffer);

            delete[] oldBuffer.buffer;
        }
        BufferGrid Canvas::CopyOutCanvasNoLock() {
            uint32_t totalLength = buffer.width * buffer.height;

            BufferGrid outBuffer(buffer.width, buffer.height, new BufferGridCell[totalLength]);

            if (totalLength && buffer.buffer) memcpy(outBuffer.buffer, buffer.buffer, sizeof(BufferGridCell) * totalLength);

            return outBuffer;
        }
        BufferGrid Canvas::ExchangeCanvasNoLock(const BufferGrid& NewBuffer) {
            BufferGrid oldBuffer = buffer;

            uint32_t newTotalLength = NewBuffer.width * NewBuffer.height;

            if (buffer.width * buffer.height != newTotalLength)
                buffer.buffer = new BufferGridCell[newTotalLength];
            buffer.width = NewBuffer.width;
            buffer.height = NewBuffer.height;
            memcpy(buffer.buffer, NewBuffer.buffer, sizeof(BufferGridCell) * newTotalLength);

            return oldBuffer;
        }

        Align Canvas::GetHorizontalAlignNoLock() {
            return horizontalAlign;
        }
        void Canvas::SetHorizontalAlignNoLock(Align NewAlign) {
            horizontalAlign = NewAlign;
        }

        Align Canvas::GetVerticalAlignNoLock() {
            return verticalAlign;
        }
        void Canvas::SetVerticalAlignNoLock(Align NewAlign) {
            verticalAlign = NewAlign;
        }

        backgroundFill_t Canvas::GetBackgroundFillNoLock() {
            return backgroundFill;
        }
        void Canvas::SetBackgroundFillNoLock(backgroundFill_t NewFill) {
            backgroundFill = NewFill;
        }

        void Canvas::DrawControl(BufferGrid Buffer, RectU32 Partition) {
            std::lock_guard<std::mutex> lock(mtx);

            lastPartSize = Partition.size;

            DrawCanvasInFrame(Buffer, Partition, buffer, horizontalAlign, verticalAlign, backgroundFill);
        }

        Canvas::Canvas(FocusManager* FocusManager, std::function<void()> InvalidateFunc)
            : Control(FocusManager, InvalidateFunc), horizontalAlign(AlignMiddle), verticalAlign(AlignMiddle), backgroundFill(std::monostate{}) { }

        PointU32 Canvas::ControlCoordsToCanvasCoords(PointU32 ControlCoords) {
            std::lock_guard<std::mutex> lock(mtx);

            return ControlCoordsToCanvasCoordsNoLock(ControlCoords);
        }
        PointU32 Canvas::CanvasCoordsToControlCoords(PointU32 CanvasCoords) {
            std::lock_guard<std::mutex> lock(mtx);

            return CanvasCoordsToControlCoordsNoLock(CanvasCoords);
        }

        void Canvas::CopyInCanvas(const BufferGrid& NewBuffer) {
            std::lock_guard<std::mutex> lock(mtx);

            CopyInCanvasNoLock(NewBuffer);
        }
        BufferGrid Canvas::CopyOutCanvas() {
            std::lock_guard<std::mutex> lock(mtx);

            return CopyOutCanvasNoLock();
        }
        BufferGrid Canvas::ExchangeCanvas(const BufferGrid& NewBuffer) {
            std::lock_guard<std::mutex> lock(mtx);

            return ExchangeCanvasNoLock(NewBuffer);
        }

        Align Canvas::GetHorizontalAlign() {
            std::lock_guard<std::mutex> lock(mtx);

            return GetHorizontalAlignNoLock();
        }
        void Canvas::SetHorizontalAlign(Align NewAlign) {
            std::lock_guard<std::mutex> lock(mtx);

            SetHorizontalAlignNoLock(NewAlign);
        }

        Align Canvas::GetVerticalAlign() {
            std::lock_guard<std::mutex> lock(mtx);

            return GetVerticalAlignNoLock();
        }
        void Canvas::SetVerticalAlign(Align NewAlign) {
            std::lock_guard<std::mutex> lock(mtx);

            SetVerticalAlignNoLock(NewAlign);
        }

        backgroundFill_t Canvas::GetBackgroundFill() {
            std::lock_guard<std::mutex> lock(mtx);

            return GetBackgroundFillNoLock();
        }
        void Canvas::SetBackgroundFill(backgroundFill_t NewFill) {
            std::lock_guard<std::mutex> lock(mtx);

            SetBackgroundFillNoLock(NewFill);
        }

        std::wstring Label::GetTextNoLock() {
            return text;
        }
        void Label::SetTextNoLock(std::wstring NewText) {
            text = NewText;
        }

        uint32_t Label::GetTextBackcolorNoLock() {
            return textBackcolor;
        }
        void Label::SetTextBackcolorNoLock(uint32_t NewBackcolor) {
            textBackcolor = NewBackcolor;
        }

        uint32_t Label::GetTextForecolorNoLock() {
            return textForecolor;
        }
        void Label::SetTextForecolorNoLock(uint32_t NewForecolor) {
            textForecolor = NewForecolor;
        }

        Align Label::GetTextHorizontalAlignNoLock() {
            return textHorizontalAlign;
        }
        void Label::SetTextHorizontalAlignNoLock(Align NewAlign) {
            textHorizontalAlign = NewAlign;
        }

        Align Label::GetTextVerticalAlignNoLock() {
            return textVerticalAlign;
        }
        void Label::SetTextVerticalAlignNoLock(Align NewAlign) {
            textVerticalAlign = NewAlign;
        }

        WrapStyle Label::GetTextWrapStyleNoLock() {
            return textWrapStyle;
        }
        void Label::SetTextWrapStyleNoLock(WrapStyle WrapStyle) {
            textWrapStyle = WrapStyle;
        }

        backgroundFill_t Label::GetBackgroundFillNoLock() {
            return backgroundFill;
        }
        void Label::SetBackgroundFillNoLock(backgroundFill_t NewFill) {
            backgroundFill = NewFill;
        }

        void Label::DrawControl(BufferGrid Buffer, RectU32 Partition) {
            std::lock_guard<std::mutex> lock(mtx);

            DrawTextInFrame(Buffer, Partition, text, textBackcolor, textForecolor, textHorizontalAlign, textVerticalAlign, textWrapStyle, backgroundFill);
        }

        Label::Label(FocusManager* FocusManager, std::function<void()> InvalidateFunc)
            : Control(FocusManager, InvalidateFunc), text(L""), textBackcolor(0xFF000000), textForecolor(0xFFFFFFFF), textHorizontalAlign(AlignMiddle), textVerticalAlign(AlignMiddle), textWrapStyle(WrapStyleNoWrap), backgroundFill(std::monostate{}) { }

        std::wstring Label::GetText() {
            std::lock_guard<std::mutex> lock(mtx);

            return GetTextNoLock();
        }
        void Label::SetText(std::wstring NewText) {
            std::lock_guard<std::mutex> lock(mtx);

            SetTextNoLock(NewText);
        }

        uint32_t Label::GetTextBackcolor() {
            std::lock_guard<std::mutex> lock(mtx);

            return GetTextBackcolorNoLock();
        }
        void Label::SetTextBackcolor(uint32_t NewBackcolor) {
            std::lock_guard<std::mutex> lock(mtx);

            SetTextBackcolorNoLock(NewBackcolor);
        }

        uint32_t Label::GetTextForecolor() {
            std::lock_guard<std::mutex> lock(mtx);

            return GetTextForecolorNoLock();
        }
        void Label::SetTextForecolor(uint32_t NewForecolor) {
            std::lock_guard<std::mutex> lock(mtx);

            SetTextForecolorNoLock(NewForecolor);
        }

        Align Label::GetTextHorizontalAlign() {
            std::lock_guard<std::mutex> lock(mtx);

            return GetTextHorizontalAlignNoLock();
        }
        void Label::SetTextHorizontalAlign(Align NewAlign) {
            std::lock_guard<std::mutex> lock(mtx);

            SetTextHorizontalAlignNoLock(NewAlign);
        }

        Align Label::GetTextVerticalAlign() {
            std::lock_guard<std::mutex> lock(mtx);

            return GetTextVerticalAlignNoLock();
        }
        void Label::SetTextVerticalAlign(Align NewAlign) {
            std::lock_guard<std::mutex> lock(mtx);

            SetTextVerticalAlignNoLock(NewAlign);
        }

        WrapStyle Label::GetTextWrapStyle() {
            std::lock_guard<std::mutex> lock(mtx);

            return GetTextWrapStyleNoLock();
        }
        void Label::SetTextWrapStyle(WrapStyle WrapStyle) {
            std::lock_guard<std::mutex> lock(mtx);

            SetTextWrapStyleNoLock(WrapStyle);
        }

        backgroundFill_t Label::GetBackgroundFill() {
            std::lock_guard<std::mutex> lock(mtx);

            return GetBackgroundFillNoLock();
        }
        void Label::SetBackgroundFill(backgroundFill_t NewFill) {
            std::lock_guard<std::mutex> lock(mtx);

            SetBackgroundFillNoLock(NewFill);
        }

        ButtonState Button::GetButtonStateNoLock() {
            return buttonState;
        }
        void Button::SetButtonStateNoLock(ButtonState State) {
            buttonState = State;
        }

        backgroundFill_t Button::GetBackgroundFillReleasedNoLock() {
            return fillReleased;
        }
        void Button::SetBackgroundFillReleasedNoLock(backgroundFill_t NewFill) {
            fillReleased = NewFill;
        }

        backgroundFill_t Button::GetBackgroundFillMouseoverNoLock() {
            return fillMouseover;
        }
        void Button::SetBackgroundFillMouseoverNoLock(backgroundFill_t NewFill) {
            fillMouseover = NewFill;
        }

        backgroundFill_t Button::GetBackgroundFillCompressedNoLock() {
            return fillCompressed;
        }
        void Button::SetBackgroundFillCompressedNoLock(backgroundFill_t NewFill) {
            fillCompressed = NewFill;
        }

        void Button::SetBackgroundFillNoLock(backgroundFill_t NewFill) {
            fillReleased = NewFill;
            fillMouseover = NewFill;
            fillCompressed = NewFill;
            Label::SetBackgroundFillNoLock(NewFill);
        }

        void Button::OnMouseDown(const MouseDownControlInfo& Info) {
            std::lock_guard<std::mutex> lock(mtx);

            buttonState = ButtonStateCompressed;
        }
        void Button::OnMouseUp(const MouseUpControlInfo& Info) {
            std::lock_guard<std::mutex> lock(mtx);

            buttonState = ButtonStateMouseOver;
        }
        void Button::OnMouseEnter(const MouseEnterControlInfo& Info) {
            std::lock_guard<std::mutex> lock(mtx);

            buttonState = ButtonStateMouseOver;
        }
        void Button::OnMouseExit(const MouseExitControlInfo& Info) {
            std::lock_guard<std::mutex> lock(mtx);

            buttonState = ButtonStateReleased;
        }

        ButtonState Button::GetButtonState() {
            std::lock_guard<std::mutex> lock(mtx);

            return GetButtonStateNoLock();
        }
        void Button::SetButtonState(ButtonState State) {
            std::lock_guard<std::mutex> lock(mtx);

            SetButtonStateNoLock(State);
        }

        Button::Button(FocusManager* FocusManager, std::function<void()> InvalidateFunc)
            : Label(FocusManager, InvalidateFunc), buttonState(ButtonStateReleased), fillReleased(std::monostate{}), fillMouseover(std::monostate{}), fillCompressed(std::monostate{}) { }

        backgroundFill_t Button::GetBackgroundFillReleased() {
            std::lock_guard<std::mutex> lock(mtx);

            return GetBackgroundFillReleasedNoLock();
        }
        void Button::SetBackgroundFillReleased(backgroundFill_t NewFill) {
            std::lock_guard<std::mutex> lock(mtx);

            SetBackgroundFillReleasedNoLock(NewFill);
        }

        backgroundFill_t Button::GetBackgroundFillMouseover() {
            std::lock_guard<std::mutex> lock(mtx);

            return GetBackgroundFillMouseoverNoLock();
        }
        void Button::SetBackgroundFillMouseover(backgroundFill_t NewFill) {
            std::lock_guard<std::mutex> lock(mtx);

            SetBackgroundFillMouseoverNoLock(NewFill);
        }

        backgroundFill_t Button::GetBackgroundFillCompressed() {
            std::lock_guard<std::mutex> lock(mtx);

            return GetBackgroundFillCompressedNoLock();
        }
        void Button::SetBackgroundFillCompressed(backgroundFill_t NewFill) {
            std::lock_guard<std::mutex> lock(mtx);

            SetBackgroundFillCompressedNoLock(NewFill);
        }
    }
}