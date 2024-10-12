#pragma once

#include "windowbase.h"
#include <cstdint>
#include <variant>

namespace btui {
    enum Align {
        AlignStart,
        AlignMiddle,
        AlignEnd
    };

    enum WrapStyle {
        WrapStyleNoWrap,
        WrapStyleWrapByChar,
        WrapStyleWrapByWord,
        WrapStyleWrapByWordAndStretch
    };

    using backgroundFill_t = std::variant<std::monostate, BufferGridCell, uint32_t>;

    void OverwriteWithBackgroundFill(BufferGridCell& Cell, const backgroundFill_t& BackgroundFill);

    struct CanvasIntoFrameMappingInfo1D {
        uint32_t ioLength;
        uint32_t inStartCoord;
        uint32_t outStartCoord;

        constexpr inline CanvasIntoFrameMappingInfo1D()
            : ioLength(0), inStartCoord(0), outStartCoord(0) { }
        constexpr inline CanvasIntoFrameMappingInfo1D(uint32_t IoLength, uint32_t InStartCoord, uint32_t OutStartCoord)
            : ioLength(IoLength), inStartCoord(InStartCoord), outStartCoord(OutStartCoord) { }

        constexpr inline uint32_t FrameCoordToCanvasCoord(uint32_t FrameCoord) {
            if (FrameCoord >= outStartCoord && FrameCoord < outStartCoord + ioLength)
                return FrameCoord - outStartCoord + inStartCoord;
            else
                return -1;
        }
        constexpr inline uint32_t CanvasCoordToFrameCoord(uint32_t CanvasCoord) {
            if (CanvasCoord >= inStartCoord && CanvasCoord < inStartCoord + ioLength)
                return CanvasCoord - inStartCoord + outStartCoord;
            else
                return -1;
        }
    };

    constexpr inline CanvasIntoFrameMappingInfo1D MakeCanvasIntoFrameMappingInfo1D(uint32_t CanvasLength, uint32_t FrameLength, Align DimensionAlign) {
        CanvasIntoFrameMappingInfo1D ioInfo;

        ioInfo.ioLength = min(CanvasLength, FrameLength);

        switch (DimensionAlign) {
        case btui::AlignStart:
            ioInfo.inStartCoord = 0;
            ioInfo.outStartCoord = 0;
            break;
        case btui::AlignEnd:
            ioInfo.inStartCoord = CanvasLength - ioInfo.ioLength;
            ioInfo.outStartCoord = FrameLength - ioInfo.ioLength;
            break;
        case btui::AlignMiddle:
            ioInfo.inStartCoord = (CanvasLength >> 1) - (ioInfo.ioLength >> 1);
            ioInfo.outStartCoord = (FrameLength >> 1) - (ioInfo.ioLength >> 1);
            break;
        default:
            return CanvasIntoFrameMappingInfo1D(0, 0, 0);
        }

        return ioInfo;
    }

    struct CanvasIntoFrameMappingInfo {
        union {
            struct {
                uint32_t ioWidth;
                uint32_t ioHeight;
            };
            SizeU32 ioSize;
        };
        union {
            struct {
                uint32_t inX;
                uint32_t inY;
            };
            PointU32 inPoint;
        };
        union {
            struct {
                uint32_t outX;
                uint32_t outY;
            };
            PointU32 outPoint;
        };

        constexpr inline CanvasIntoFrameMappingInfo()
            : ioSize(), inPoint(), outPoint() { }
        constexpr inline CanvasIntoFrameMappingInfo(uint32_t IoWidth, uint32_t IoHeight, uint32_t InX, uint32_t InY, uint32_t OutX, uint32_t OutY)
            : ioWidth(IoWidth), ioHeight(IoHeight), inX(InX), inY(InY), outX(OutX), outY(OutY) { }
        constexpr inline CanvasIntoFrameMappingInfo(SizeU32 IoSize, PointU32 InPoint, PointU32 OutPoint)
            : ioSize(IoSize), inPoint(InPoint), outPoint(OutPoint) { }
        constexpr inline CanvasIntoFrameMappingInfo(CanvasIntoFrameMappingInfo1D XInfo, CanvasIntoFrameMappingInfo1D YInfo)
            : ioWidth(XInfo.ioLength), ioHeight(YInfo.ioLength), inX(XInfo.inStartCoord), inY(YInfo.inStartCoord), outX(XInfo.outStartCoord), outY(YInfo.outStartCoord) { }

        constexpr inline CanvasIntoFrameMappingInfo1D GetInfoX() {
            return CanvasIntoFrameMappingInfo1D(ioWidth, inX, outX);
        }
        constexpr inline CanvasIntoFrameMappingInfo1D GetInfoY() {
            return CanvasIntoFrameMappingInfo1D(ioHeight, inY, outY);
        }

        constexpr inline RectU32 GetInRect() {
            return RectU32(inPoint, ioSize);
        }
        constexpr inline RectU32 GetOutRect() {
            return RectU32(outPoint, ioSize);
        }

        constexpr inline PointU32 FrameCoordsToCanvasCoords(PointU32 FrameCoords) {
            if (GetOutRect().IsPointWithin(FrameCoords))
                return FrameCoords - outPoint + inPoint;
            else
                return PointU32(-1, -1);
        }
        constexpr inline PointU32 CanvasCoordsToFrameCoords(PointU32 CanvasCoords) {
            if (GetInRect().IsPointWithin(CanvasCoords))
                return CanvasCoords - inPoint + outPoint;
            else
                return PointU32(-1, -1);
        }
    };

    constexpr inline CanvasIntoFrameMappingInfo MakeCanvasIntoFrameMappingInfo(SizeU32 CanvasSize, SizeU32 FrameSize, Align HorizontalAlign, Align VerticalAlign) {
        return CanvasIntoFrameMappingInfo(
            MakeCanvasIntoFrameMappingInfo1D(CanvasSize.width, FrameSize.width, HorizontalAlign),
            MakeCanvasIntoFrameMappingInfo1D(CanvasSize.height, FrameSize.height, VerticalAlign)
        );
    }

    void DrawCanvasInFrame(BufferGrid WindowBuffer, RectU32 FrameRect, BufferGrid Canvas, Align HorizontalAlign, Align VerticalAlign, backgroundFill_t BackgroundFill);

    void DrawTextInFrame(BufferGrid WindowBuffer, RectU32 FrameRect, const std::wstring& Text, uint32_t TextBackcolor, uint32_t TextForecolor, Align TextHorizontalAlign, Align TextVerticalAlign, WrapStyle TextWrapStyle, backgroundFill_t BackgroundFill);
}