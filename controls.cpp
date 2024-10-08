#include "pch.h"
#include "framework.h"

#include "controls.h"
#include <optional>

struct CanvasIoInfo {
    union {
        struct {
            uint32_t ioWidth;
            uint32_t ioHeight;
        };
        btui::SizeU32 ioSize;
    };
    union {
        struct {
            uint32_t readX;
            uint32_t readY;
        };
        btui::PointU32 readPoint;
    };
    union {
        struct {
            uint32_t writeX;
            uint32_t writeY;
        };
        btui::PointU32 writePoint;
    };

    constexpr inline CanvasIoInfo()
        : ioSize(), readPoint(), writePoint() { }
};

std::optional<CanvasIoInfo> GetCanvasIoInfo(btui::SizeU32 CanvasSize, btui::SizeU32 PartitionSize, btui::Align VerticalAlign, btui::Align HorizontalAlign) {
    CanvasIoInfo ioInfo;
    
    ioInfo.ioWidth = min(CanvasSize.width, PartitionSize.width);
    ioInfo.ioHeight = min(CanvasSize.height, PartitionSize.height);

    switch (HorizontalAlign) {
    case btui::Align::Start:
        ioInfo.readX = 0;
        ioInfo.writeX = 0;
        break;
    case btui::Align::End:
        ioInfo.readX = CanvasSize.width - ioInfo.ioWidth;
        ioInfo.writeX = PartitionSize.width - ioInfo.ioWidth;
        break;
    case btui::Align::Middle:
        ioInfo.readX = CanvasSize.width / 2 - ioInfo.ioWidth / 2;
        ioInfo.writeX = PartitionSize.width / 2 - ioInfo.ioWidth / 2;
        break;
    default:
        return std::nullopt;
    }

    switch (VerticalAlign) {
    case btui::Align::Start:
        ioInfo.readY = 0;
        ioInfo.writeY = 0;
        break;
    case btui::Align::End:
        ioInfo.readY = CanvasSize.height - ioInfo.ioHeight;
        ioInfo.writeY = PartitionSize.height - ioInfo.ioHeight;
        break;
    case btui::Align::Middle:
        ioInfo.readY = CanvasSize.height / 2 - ioInfo.ioHeight / 2;
        ioInfo.writeY = PartitionSize.height / 2 - ioInfo.ioHeight / 2;
        break;
    default:
        return std::nullopt;
    }

    return ioInfo;
}

namespace btui {
    namespace controls {
        PointU32 Canvas::ControlCoordsToCanvasCoordsNoLock(PointU32 ControlCoords) {
            std::optional<CanvasIoInfo> ioInfoO = GetCanvasIoInfo(buffer.size, lastPartSize, verticalAlign, horizontalAlign);
            if (!ioInfoO.has_value()) return PointU32(0, 0);
            CanvasIoInfo ioInfo = *ioInfoO;

            if (!RectU32(ioInfo.writePoint, ioInfo.ioSize).IsPointWithin(ControlCoords))
                return PointU32(-1, -1);

            return ControlCoords - ioInfo.writePoint + ioInfo.readPoint;
        }
        PointU32 Canvas::CanvasCoordsToControlCoordsNoLock(PointU32 CanvasCoords) {
            std::optional<CanvasIoInfo> ioInfoO = GetCanvasIoInfo(buffer.size, lastPartSize, verticalAlign, horizontalAlign);
            if (!ioInfoO.has_value()) return PointU32(0, 0);
            CanvasIoInfo ioInfo = *ioInfoO;

            if (!RectU32(ioInfo.readPoint, ioInfo.ioSize).IsPointWithin(CanvasCoords))
                return PointU32(-1, -1);

            return CanvasCoords - ioInfo.readPoint + ioInfo.writePoint;
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

            std::optional<CanvasIoInfo> ioInfoO = GetCanvasIoInfo(buffer.size, Partition.size, verticalAlign, horizontalAlign);
            if (!ioInfoO.has_value()) return;
            CanvasIoInfo ioInfo = *ioInfoO;

            ioInfo.writeX += Partition.x;
            ioInfo.writeY += Partition.y;

            uint32_t rXMax = ioInfo.readX + ioInfo.ioWidth;
            uint32_t rYMax = ioInfo.readY + ioInfo.ioHeight;
            for (uint32_t iR = ioInfo.readX, iW = ioInfo.writeX; iR < rXMax; ++iR, ++iW)
            for (uint32_t jR = ioInfo.readY, jW = ioInfo.writeY; jR < rYMax; ++jR, ++jW) {
                Buffer.buffer[jW * Buffer.width + iW] = buffer.buffer[jR * Buffer.width + iR];
            }

            uint32_t writeXMax = ioInfo.writeX + ioInfo.ioWidth;
            uint32_t writeYMax = ioInfo.writeY + ioInfo.ioHeight;
            uint32_t partXMax = Partition.x + Partition.width;
            uint32_t partYMax = Partition.y + Partition.height;

            if (Partition.y < ioInfo.writeY)
                for (uint32_t i = Partition.x; i < partXMax; ++i)
                for (uint32_t j = Partition.y; j < ioInfo.writeY; ++j)
                    OverwriteWithBackgroundFill(Buffer.buffer[j * Buffer.width + i], backgroundFill);

            if (writeYMax < partYMax)
                for (uint32_t i = Partition.x; i < partXMax; ++i)
                for (uint32_t j = writeYMax; j < partYMax; ++j)
                    OverwriteWithBackgroundFill(Buffer.buffer[j * Buffer.width + i], backgroundFill);

            if (ioInfo.writeY < writeYMax)
                for (uint32_t i = Partition.x; i < ioInfo.writeX; ++i)
                for (uint32_t j = ioInfo.writeY; j < writeYMax; ++j)
                    OverwriteWithBackgroundFill(Buffer.buffer[j * Buffer.width + i], backgroundFill);

            if (ioInfo.writeY < writeYMax)
                for (uint32_t i = writeXMax; i < partXMax; ++i)
                for (uint32_t j = ioInfo.writeY; j < writeYMax; ++j)
                    OverwriteWithBackgroundFill(Buffer.buffer[j * Buffer.width + i], backgroundFill);
        }

        Canvas::Canvas(FocusManager* FocusManager, std::function<void()> InvalidateFunc, BufferGrid Buffer, Align HorizontalAlign, Align VerticalAlign, backgroundFill_t BackgroundFill)
            : Control(FocusManager, InvalidateFunc), horizontalAlign(HorizontalAlign), verticalAlign(VerticalAlign), backgroundFill(BackgroundFill) {
            uint32_t product = buffer.width * buffer.height;
            if (product && Buffer.buffer) {
                Buffer.width = buffer.width;
                Buffer.height = buffer.height;
                Buffer.buffer = new BufferGridCell[product];
                memcpy(buffer.buffer, Buffer.buffer, sizeof(BufferGridCell) * product);
            }
        }

        PointU32 Canvas::ControlCoordsToCanvasCoords(PointU32 ControlCoords) {
            std::lock_guard<std::mutex> lock(mtx);

            return ControlCoordsToCanvasCoordsNoLock(ControlCoords);
        }
        PointU32 Canvas::CanvasCoordsToControlCoords(PointU32 CanvasCoords) {
            std::lock_guard<std::mutex> lock(mtx);

            return CanvasCoordsToControlCoords(CanvasCoords);
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

            std::vector<std::wstring> lines;
            
            switch (textWrapStyle) {
            case WrapStyle::NoWrap:
                {
                    std::wstringstream currentStream;
                    uint32_t size = 0;
                    bool writing = true;
                    for (wchar_t c : text) {
                        if (c == L'\n') {
                            lines.push_back(currentStream.str());
                            currentStream.clear();
                            size = 0;
                            writing = true;
                            continue;
                        }
                        if (size >= Partition.width) {
                            writing = false;
                            continue;
                        }
                        if (!writing) continue;
                        if (c == L'\r') continue;

                        currentStream << c;
                        size++;
                    }
                    lines.push_back(currentStream.str());
                }
                break;
            case WrapStyle::WrapByChar:
                {
                    std::wstringstream currentStream;
                    uint32_t size = 0;
                    for (wchar_t c : text) {
                        if (c == L'\n' || size >= Partition.width) {
                            lines.push_back(currentStream.str());
                            currentStream.clear();
                            size = 0;
                            continue;
                        }
                        if (c == L'\r') continue;

                        currentStream << c;
                        size++;
                    }
                    lines.push_back(currentStream.str());
                }
                break;
            case WrapStyle::WrapByWord:
            case WrapStyle::WrapByWordAndStretch:
                {
                    std::vector<bool> wrappedRecord;
                    std::wstringstream currentStream;
                    uint32_t size = 0;
                    uint32_t lastSpace = 0;
                    bool notFirst = false;
                    bool lastWasWhitespace = true;
                    for (uint32_t i = 0; i < text.size(); ++i) {
                        wchar_t c = text[i];

                        if (c == L'\r') continue;
                        if (c == L'\n') {
                            if (lastWasWhitespace) {
                                notFirst = true;
                                size = 0;
                                lines.push_back(currentStream.str());
                                wrappedRecord.push_back(false);
                                currentStream.clear();
                                lastSpace = i;
                                continue;
                            }
                            if (notFirst) currentStream << L' ';
                            for (uint32_t j = lastSpace + 1; j < i; ++j)
                                currentStream << text[j];
                            notFirst = false;
                            size = 0;
                            lines.push_back(currentStream.str());
                            wrappedRecord.push_back(false);
                            currentStream.clear();
                            lastSpace = i;
                            lastWasWhitespace = true;
                            continue;
                        }
                        if (c == L' ') {
                            if (lastWasWhitespace) {
                                lastSpace = i;
                                continue;
                            }
                            if (size >= Partition.width) {
                                if (notFirst) currentStream << L' ';
                                for (uint32_t j = lastSpace + 1; j < i; ++j)
                                    currentStream << text[j];
                                notFirst = false;
                                size = 0;
                                lines.push_back(currentStream.str());
                                wrappedRecord.push_back(true);
                                currentStream.clear();
                                lastSpace = i;
                                lastWasWhitespace = true;
                                continue;
                            }
                            if (notFirst) {
                                currentStream << L' ';
                                size++;
                            }
                            for (uint32_t j = lastSpace + 1; j < i; ++j)
                                currentStream << text[j];
                            size += i - lastSpace - 1;
                            lastSpace = i;
                            lastWasWhitespace = true;
                            notFirst = true;
                            continue;
                        }
                        if (size >= Partition.width) {
                            if (!notFirst) {
                                for (uint32_t j = lastSpace + 1; j < i; ++j)
                                    currentStream << text[j];
                            }
                            lines.push_back(currentStream.str());
                            wrappedRecord.push_back(true);
                            currentStream.clear();
                            notFirst = false;
                            size = 0;
                            lastWasWhitespace = false;
                            continue;
                        }

                        size++;
                        lastWasWhitespace = false;
                    }
                    if (notFirst) currentStream << L' ';
                    for (uint32_t i = lastSpace + 1; i < text.size(); ++i)
                        currentStream << text[i];
                    lines.push_back(currentStream.str());
                    wrappedRecord.push_back(false);

                    if (textWrapStyle == WrapStyle::WrapByWordAndStretch) {
                        for (size_t lineIdx = 0; lineIdx < lines.size(); ++lineIdx) {
                            std::wstring& line = lines[lineIdx];
                            bool lineWasWrapped = wrappedRecord[lineIdx];

                            if (!lineWasWrapped) continue;

                            uint32_t remainder = Partition.width - line.size();
                            if (!remainder) continue;

                            uint32_t spaceCount = 0;
                            for (wchar_t c : line)
                                if (c == L' ') spaceCount++;
                            if (!spaceCount) continue;

                            uint32_t perSpaceAddBase = remainder / spaceCount;
                            uint32_t perSpaceAddBaseRemainder = remainder % spaceCount;

                            uint32_t remainderDistribution = spaceCount / perSpaceAddBaseRemainder;
                            uint32_t remainderDistributionRemainder = spaceCount % perSpaceAddBaseRemainder;
                            uint32_t remainderDistributionRemainderUntil = spaceCount - remainderDistributionRemainder;

                            std::wstringstream newLineStream;
                            uint32_t countOfSpacesSeenSoFar = 0;
                            for (wchar_t c : line) {
                                newLineStream << c;
                                if (c == L' ') {
                                    countOfSpacesSeenSoFar++;
                                    if (countOfSpacesSeenSoFar > remainderDistributionRemainderUntil)
                                        newLineStream << L' ';
                                    for (uint32_t i = 0; i < perSpaceAddBase; ++i)
                                        newLineStream << L' ';
                                }
                            }

                            line = newLineStream.str();
                        }
                    }
                }
                break;
            }

            std::optional<CanvasIoInfo> ioInfoO = GetCanvasIoInfo(SizeU32(Partition.width, lines.size()), Partition.size, textVerticalAlign, Align::Start);
            if (!ioInfoO.has_value()) return;
            CanvasIoInfo ioInfo = *ioInfoO;

            ioInfo.writeX += Partition.x;
            ioInfo.writeY += Partition.y;

            uint32_t rYEnd = ioInfo.readY + ioInfo.ioHeight;
            uint32_t wYEnd = ioInfo.writeY + ioInfo.ioHeight;
            uint32_t pXEnd = Partition.x + Partition.width;
            uint32_t pYEnd = Partition.y + Partition.height;
            switch (textHorizontalAlign) {
            case Align::Start:
                for (uint32_t rJ = ioInfo.readY, wJ = ioInfo.writeY; rJ < rYEnd; rJ++, wJ++) {
                    const std::wstring& line = lines[rJ];
                    for (uint32_t lI = 0, wI = ioInfo.writeX; lI < line.size(); lI++, wI++) {
                        BufferGridCell& cell = Buffer.buffer[wJ * Buffer.width + wI];

                        cell.character = line[lI];
                        cell.forecolor = textForecolor;
                        cell.backcolor = textBackcolor;
                    }
                    if (backgroundFill.index()) {
                        for (uint32_t wI = ioInfo.writeX + line.size(); wI < pXEnd; ++wI) {
                            BufferGridCell& cell = Buffer.buffer[wJ * Buffer.width + wI];

                            OverwriteWithBackgroundFill(cell, backgroundFill);
                        }
                    }
                }
                break;
            case Align::Middle:
                for (uint32_t rJ = ioInfo.readY, wJ = ioInfo.writeY; rJ < rYEnd; rJ++, wJ++) {
                    const std::wstring& line = lines[rJ];
                    uint32_t wIStart = Partition.width / 2 - line.size() / 2 + ioInfo.writeX;
                    for (uint32_t lI = 0, wI = ioInfo.writeX; lI < line.size(); lI++, wI++) {
                        BufferGridCell& cell = Buffer.buffer[wJ * Buffer.width + wI];

                        cell.character = line[lI];
                        cell.forecolor = textForecolor;
                        cell.backcolor = textBackcolor;
                    }
                    uint32_t wIEnd = wIStart + line.size();
                    if (backgroundFill.index()) {
                        for (uint32_t wI = Partition.x; wI < wIStart; ++wI) {
                            BufferGridCell& cell = Buffer.buffer[wJ * Buffer.width + wI];

                            OverwriteWithBackgroundFill(cell, backgroundFill);
                        }
                        for (uint32_t wI = wIEnd; wI < pXEnd; ++wI) {
                            BufferGridCell& cell = Buffer.buffer[wJ * Buffer.width + wI];

                            OverwriteWithBackgroundFill(cell, backgroundFill);
                        }
                    }
                }
                break;
            case Align::End:
                for (uint32_t rJ = ioInfo.readY, wJ = ioInfo.writeY; rJ < rYEnd; rJ++, wJ++) {
                    const std::wstring& line = lines[rJ];
                    uint32_t wIStart = pXEnd - line.size();
                    for (uint32_t lI = 0, wI = wIStart; lI < line.size(); lI++, wI++) {
                        BufferGridCell& cell = Buffer.buffer[wJ * Buffer.width + wI];

                        cell.character = line[lI];
                        cell.forecolor = textForecolor;
                        cell.backcolor = textBackcolor;
                    }
                    if (backgroundFill.index()) {
                        for (uint32_t wI = Partition.x; wI < wIStart; ++wI) {
                            BufferGridCell& cell = Buffer.buffer[wJ * Buffer.width + wI];

                            OverwriteWithBackgroundFill(cell, backgroundFill);
                        }
                    }
                }
                break;
            }
            if (backgroundFill.index()) {
                for (uint32_t wI = Partition.x; wI < pXEnd; ++wI) {
                    for (uint32_t wJ = Partition.y; wJ < ioInfo.writeY; ++wJ) {
                        BufferGridCell& cell = Buffer.buffer[wJ * Buffer.width + wI];

                        OverwriteWithBackgroundFill(cell, backgroundFill);
                    }
                    for (uint32_t wJ = wYEnd; wJ < pYEnd; ++wJ) {
                        BufferGridCell& cell = Buffer.buffer[wJ * Buffer.width + wI];

                        OverwriteWithBackgroundFill(cell, backgroundFill);
                    }
                }
            }
        }

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
            
            buttonState = ButtonState::Compressed;
        }
        void Button::OnMouseUp(const MouseUpControlInfo& Info) {
            std::lock_guard<std::mutex> lock(mtx);

            buttonState = ButtonState::MouseOver;
        }
        void Button::OnMouseEnter(const MouseEnterControlInfo& Info) {
            std::lock_guard<std::mutex> lock(mtx);

            buttonState = ButtonState::MouseOver;
        }
        void Button::OnMouseExit(const MouseExitControlInfo& Info) {
            std::lock_guard<std::mutex> lock(mtx);

            buttonState = ButtonState::Released;
        }

        ButtonState Button::GetButtonState() {
            std::lock_guard<std::mutex> lock(mtx);

            return GetButtonStateNoLock();
        }
        void Button::SetButtonState(ButtonState State) {
            std::lock_guard<std::mutex> lock(mtx);

            SetButtonStateNoLock(State);
        }

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