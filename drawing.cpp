#include "pch.h"
#include "framework.h"

#include "drawing.h"

namespace btui {
    void OverwriteWithBackgroundFill(BufferGridCell& Cell, const backgroundFill_t& BackgroundFill) {
        switch (BackgroundFill.index()) {
        case 1:
            Cell = std::get<BufferGridCell>(BackgroundFill);
            break;
        case 2:
            {
                uint32_t color = std::get<uint32_t>(BackgroundFill);
                Cell.backcolor = OverlayColor(Cell.backcolor, color);
                Cell.forecolor = OverlayColor(Cell.forecolor, color);
            }
            break;
        }
    }

    void DrawCanvasInFrame(BufferGrid WindowBuffer, RectU32 FrameRect, BufferGrid Canvas, Align HorizontalAlign, Align VerticalAlign, backgroundFill_t BackgroundFill) {
        CanvasIntoFrameMappingInfo mapInfo = MakeCanvasIntoFrameMappingInfo(Canvas.size, FrameRect.size, HorizontalAlign, VerticalAlign);

        mapInfo.outX += FrameRect.x;
        mapInfo.outY += FrameRect.y;

        uint32_t rXMax = mapInfo.inX + mapInfo.ioWidth;
        uint32_t rYMax = mapInfo.inY + mapInfo.ioHeight;
        for (uint32_t iR = mapInfo.inX, iW = mapInfo.outX; iR < rXMax; ++iR, ++iW)
        for (uint32_t jR = mapInfo.inY, jW = mapInfo.outY; jR < rYMax; ++jR, ++jW)
                WindowBuffer.buffer[jW * WindowBuffer.width + iW] = Canvas.buffer[jR * Canvas.width + iR];

        uint32_t writeXMax = mapInfo.outX + mapInfo.ioWidth;
        uint32_t writeYMax = mapInfo.outY + mapInfo.ioHeight;
        uint32_t partXMax = FrameRect.x + FrameRect.width;
        uint32_t partYMax = FrameRect.y + FrameRect.height;

        if (FrameRect.y < mapInfo.outY)
            for (uint32_t i = FrameRect.x; i < partXMax; ++i)
            for (uint32_t j = FrameRect.y; j < mapInfo.outY; ++j)
                OverwriteWithBackgroundFill(WindowBuffer.buffer[j * WindowBuffer.width + i], BackgroundFill);

        if (writeYMax < partYMax)
            for (uint32_t i = FrameRect.x; i < partXMax; ++i)
            for (uint32_t j = writeYMax; j < partYMax; ++j)
                OverwriteWithBackgroundFill(WindowBuffer.buffer[j * WindowBuffer.width + i], BackgroundFill);

        if (mapInfo.outY < writeYMax)
            for (uint32_t i = FrameRect.x; i < mapInfo.outX; ++i)
            for (uint32_t j = mapInfo.outY; j < writeYMax; ++j)
                OverwriteWithBackgroundFill(WindowBuffer.buffer[j * WindowBuffer.width + i], BackgroundFill);

        if (mapInfo.outY < writeYMax)
            for (uint32_t i = writeXMax; i < partXMax; ++i)
            for (uint32_t j = mapInfo.outY; j < writeYMax; ++j)
                OverwriteWithBackgroundFill(WindowBuffer.buffer[j * WindowBuffer.width + i], BackgroundFill);
    }

    void DrawTextInFrame(BufferGrid WindowBuffer, RectU32 FrameRect, const std::wstring& Text, uint32_t TextBackcolor, uint32_t TextForecolor, Align TextHorizontalAlign, Align TextVerticalAlign, WrapStyle TextWrapStyle, backgroundFill_t BackgroundFill) {
        std::vector<std::wstring> lines;

        switch (TextWrapStyle) {
        case WrapStyleNoWrap:
            {
                std::wstringstream currentStream;
                uint32_t size = 0;
                bool writing = true;
                for (wchar_t c : Text) {
                    if (c == L'\n') {
                        lines.push_back(currentStream.str());
                        currentStream.str(L"");
                        size = 0;
                        writing = true;
                        continue;
                    }
                    if (size >= FrameRect.width) {
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
        case WrapStyleWrapByChar:
            {
                std::wstringstream currentStream;
                uint32_t size = 0;
                for (wchar_t c : Text) {
                    if (c == L'\n' || size >= FrameRect.width) {
                        lines.push_back(currentStream.str());
                        currentStream.str(L"");
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
        case WrapStyleWrapByWord:
        case WrapStyleWrapByWordAndStretch:
            {
                std::vector<bool> wrappedRecord;
                std::wstringstream currentStream;
                uint32_t size = 0;
                uint32_t lastSpace = 0;
                bool notFirst = false;
                bool lastWasWhitespace = true;
                for (uint32_t i = 0; i < Text.size(); ++i) {
                    wchar_t c = Text[i];

                    if (c == L'\r') continue;
                    if (c == L'\n') {
                        if (lastWasWhitespace) {
                            notFirst = true;
                            size = 0;
                            lines.push_back(currentStream.str());
                            wrappedRecord.push_back(false);
                            currentStream.str(L"");
                            lastSpace = i;
                            continue;
                        }
                        if (notFirst) currentStream << L' ';
                        for (uint32_t j = lastSpace + 1; j < i; ++j)
                            currentStream << Text[j];
                        notFirst = false;
                        size = 0;
                        lines.push_back(currentStream.str());
                        wrappedRecord.push_back(false);
                        currentStream.str(L"");
                        lastSpace = i;
                        lastWasWhitespace = true;
                        continue;
                    }
                    if (c == L' ') {
                        if (lastWasWhitespace) {
                            lastSpace = i;
                            continue;
                        }
                        if (size >= FrameRect.width) {
                            if (notFirst) currentStream << L' ';
                            for (uint32_t j = lastSpace + 1; j < i; ++j)
                                currentStream << Text[j];
                            notFirst = false;
                            size = 0;
                            lines.push_back(currentStream.str());
                            wrappedRecord.push_back(true);
                            currentStream.str(L"");
                            lastSpace = i;
                            lastWasWhitespace = true;
                            continue;
                        }
                        if (notFirst) {
                            currentStream << L' ';
                            size++;
                        }
                        for (uint32_t j = lastSpace + 1; j < i; ++j)
                            currentStream << Text[j];
                        size += i - lastSpace - 1;
                        lastSpace = i;
                        lastWasWhitespace = true;
                        notFirst = true;
                        continue;
                    }
                    if (size >= FrameRect.width) {
                        if (!notFirst) {
                            for (uint32_t j = lastSpace + 1; j < i; ++j)
                                currentStream << Text[j];
                        }
                        lines.push_back(currentStream.str());
                        wrappedRecord.push_back(true);
                        currentStream.str(L"");
                        notFirst = false;
                        size = 0;
                        lastWasWhitespace = false;
                        continue;
                    }

                    size++;
                    lastWasWhitespace = false;
                }
                if (notFirst) currentStream << L' ';
                for (uint32_t i = lastSpace + 1; i < Text.size(); ++i)
                    currentStream << Text[i];
                lines.push_back(currentStream.str());
                wrappedRecord.push_back(false);

                if (TextWrapStyle == WrapStyleWrapByWordAndStretch) {
                    for (size_t lineIdx = 0; lineIdx < lines.size(); ++lineIdx) {
                        std::wstring& line = lines[lineIdx];
                        bool lineWasWrapped = wrappedRecord[lineIdx];

                        if (!lineWasWrapped) continue;

                        uint32_t rem = FrameRect.width - line.size();
                        if (!rem) continue;

                        uint32_t spaceCount = 0;
                        for (wchar_t c : line)
                            if (c == L' ') spaceCount++;
                        if (!spaceCount) continue;

                        uint32_t perSpaceAddBase = rem / spaceCount;
                        uint32_t perSpaceAddBaseRem = rem % spaceCount;

                        uint32_t remDis = spaceCount / perSpaceAddBaseRem;
                        uint32_t remDisRem = spaceCount % perSpaceAddBaseRem;
                        uint32_t remDisRemUntil = spaceCount - remDisRem;

                        std::wstringstream newLineStream;
                        uint32_t countOfSpacesSeenSoFar = 0;
                        for (wchar_t c : line) {
                            newLineStream << c;
                            if (c == L' ') {
                                countOfSpacesSeenSoFar++;
                                if (countOfSpacesSeenSoFar > remDisRemUntil)
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

        CanvasIntoFrameMappingInfo1D mapInfo = MakeCanvasIntoFrameMappingInfo1D(lines.size(), FrameRect.height, TextVerticalAlign);
        mapInfo.outStartCoord += FrameRect.y;

        uint32_t rYEnd = mapInfo.inStartCoord + mapInfo.ioLength;
        uint32_t wYEnd = mapInfo.outStartCoord + mapInfo.ioLength;
        uint32_t pXEnd = FrameRect.x + FrameRect.width;
        uint32_t pYEnd = FrameRect.y + FrameRect.height;
        switch (TextHorizontalAlign) {
        case AlignStart:
            for (uint32_t rJ = mapInfo.inStartCoord, wJ = mapInfo.outStartCoord; rJ < rYEnd; rJ++, wJ++) {
                if (wJ >= WindowBuffer.height) break;

                const std::wstring& line = lines[rJ];
                for (uint32_t lI = 0, wI = FrameRect.x; lI < line.size(); lI++, wI++) {
                    if (wI >= WindowBuffer.width) break;

                    BufferGridCell& cell = WindowBuffer.buffer[wJ * WindowBuffer.width + wI];

                    cell.character = line[lI];
                    cell.forecolor = TextForecolor;
                    cell.backcolor = TextBackcolor;
                }
                if (BackgroundFill.index()) {
                    for (uint32_t wI = FrameRect.x + line.size(); wI < pXEnd; ++wI) {
                        if (wI >= WindowBuffer.width) break;

                        BufferGridCell& cell = WindowBuffer.buffer[wJ * WindowBuffer.width + wI];

                        OverwriteWithBackgroundFill(cell, BackgroundFill);
                    }
                }
            }
            break;
        case AlignMiddle:
            for (uint32_t rJ = mapInfo.inStartCoord, wJ = mapInfo.outStartCoord; rJ < rYEnd; rJ++, wJ++) {
                if (wJ >= WindowBuffer.height) break;

                const std::wstring& line = lines[rJ];
                uint32_t wIStart = FrameRect.width / 2 - line.size() / 2 + FrameRect.x;
                for (uint32_t lI = 0, wI = FrameRect.x; lI < line.size(); lI++, wI++) {
                    if (wI >= WindowBuffer.width) break;

                    BufferGridCell& cell = WindowBuffer.buffer[wJ * WindowBuffer.width + wI];

                    cell.character = line[lI];
                    cell.forecolor = TextForecolor;
                    cell.backcolor = TextBackcolor;
                }
                uint32_t wIEnd = wIStart + line.size();
                if (BackgroundFill.index()) {
                    for (uint32_t wI = FrameRect.x; wI < wIStart; ++wI) {
                        if (wI >= WindowBuffer.width) break;

                        BufferGridCell& cell = WindowBuffer.buffer[wJ * WindowBuffer.width + wI];

                        OverwriteWithBackgroundFill(cell, BackgroundFill);
                    }
                    for (uint32_t wI = wIEnd; wI < pXEnd; ++wI) {
                        if (wI >= WindowBuffer.width) break;

                        BufferGridCell& cell = WindowBuffer.buffer[wJ * WindowBuffer.width + wI];

                        OverwriteWithBackgroundFill(cell, BackgroundFill);
                    }
                }
            }
            break;
        case AlignEnd:
            for (uint32_t rJ = mapInfo.inStartCoord, wJ = mapInfo.outStartCoord; rJ < rYEnd; rJ++, wJ++) {
                if (wJ >= WindowBuffer.height) break;

                const std::wstring& line = lines[rJ];
                uint32_t wIStart = pXEnd - line.size();
                for (uint32_t lI = 0, wI = wIStart; lI < line.size(); lI++, wI++) {
                    if (wI >= WindowBuffer.width) break;

                    BufferGridCell& cell = WindowBuffer.buffer[wJ * WindowBuffer.width + wI];

                    cell.character = line[lI];
                    cell.forecolor = TextForecolor;
                    cell.backcolor = TextBackcolor;
                }
                if (BackgroundFill.index()) {
                    for (uint32_t wI = FrameRect.x; wI < wIStart; ++wI) {
                        if (wI >= WindowBuffer.width) break;

                        BufferGridCell& cell = WindowBuffer.buffer[wJ * WindowBuffer.width + wI];

                        OverwriteWithBackgroundFill(cell, BackgroundFill);
                    }
                }
            }
            break;
        }
        if (BackgroundFill.index()) {
            for (uint32_t wI = FrameRect.x; wI < pXEnd; ++wI) {
                if (wI >= WindowBuffer.width) break;

                for (uint32_t wJ = FrameRect.y; wJ < mapInfo.outStartCoord; ++wJ) {
                    if (wJ >= WindowBuffer.height) break;

                    BufferGridCell& cell = WindowBuffer.buffer[wJ * WindowBuffer.width + wI];

                    OverwriteWithBackgroundFill(cell, BackgroundFill);
                }
                for (uint32_t wJ = wYEnd; wJ < pYEnd; ++wJ) {
                    if (wJ >= WindowBuffer.height) break;

                    BufferGridCell& cell = WindowBuffer.buffer[wJ * WindowBuffer.width + wI];

                    OverwriteWithBackgroundFill(cell, BackgroundFill);
                }
            }
        }
    }
}