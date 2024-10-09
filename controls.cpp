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

        PointU32 Canvas::ControlCoordsToCanvasCoords(PointU32 ControlCoords) {
            std::lock_guard<std::mutex> lock(mtx);

            std::optional<CanvasIoInfo> ioInfoO = GetCanvasIoInfo(buffer.size, lastPartSize, verticalAlign, horizontalAlign);
            if (!ioInfoO.has_value()) return PointU32(0, 0);
            CanvasIoInfo ioInfo = *ioInfoO;

            if (!RectU32(ioInfo.writePoint, ioInfo.ioSize).IsPointWithin(ControlCoords))
                return PointU32(-1, -1);

            return ControlCoords - ioInfo.writePoint + ioInfo.readPoint;
        }
        PointU32 Canvas::CanvasCoordsToControlCoords(PointU32 CanvasCoords) {
            std::lock_guard<std::mutex> lock(mtx);

            std::optional<CanvasIoInfo> ioInfoO = GetCanvasIoInfo(buffer.size, lastPartSize, verticalAlign, horizontalAlign);
            if (!ioInfoO.has_value()) return PointU32(0, 0);
            CanvasIoInfo ioInfo = *ioInfoO;

            if (!RectU32(ioInfo.readPoint, ioInfo.ioSize).IsPointWithin(CanvasCoords))
                return PointU32(-1, -1);

            return CanvasCoords - ioInfo.readPoint + ioInfo.writePoint;
        }

        void Canvas::CopyInCanvas(const BufferGrid& NewBuffer) {
            BufferGrid oldBuffer = ExchangeCanvas(NewBuffer);

            delete[] oldBuffer.buffer;
        }
        BufferGrid Canvas::CopyOutCanvas() {
            std::lock_guard<std::mutex> lock(mtx);

            uint32_t totalLength = buffer.width * buffer.height;

            BufferGrid outBuffer(buffer.width, buffer.height, new BufferGridCell[totalLength]);

            memcpy(outBuffer.buffer, buffer.buffer, sizeof(BufferGridCell) * totalLength);

            return outBuffer;
        }
        BufferGrid Canvas::ExchangeCanvas(const BufferGrid& NewBuffer) {
            std::lock_guard<std::mutex> lock(mtx);

            BufferGrid oldBuffer = buffer;

            uint32_t newTotalLength = NewBuffer.width * NewBuffer.height;

            if (buffer.width * buffer.height != newTotalLength)
                buffer.buffer = new BufferGridCell[newTotalLength];
            buffer.width = NewBuffer.width;
            buffer.height = NewBuffer.height;
            memcpy(buffer.buffer, NewBuffer.buffer, sizeof(BufferGridCell) * newTotalLength);

            return oldBuffer;
        }

        Align Canvas::GetHorizontalAlign() {
            std::lock_guard<std::mutex> lock(mtx);

            return horizontalAlign;
        }
        void Canvas::SetHorizontalAlign(Align NewAlign) {
            std::lock_guard<std::mutex> lock(mtx);

            horizontalAlign = NewAlign;
        }

        Align Canvas::GetVerticalAlign() {
            std::lock_guard<std::mutex> lock(mtx);

            return verticalAlign;
        }
        void Canvas::SetVerticalAlign(Align NewAlign) {
            std::lock_guard<std::mutex> lock(mtx);

            verticalAlign = NewAlign;
        }

        backgroundFill_t Canvas::GetBackgroundFill() {
            std::lock_guard<std::mutex> lock(mtx);

            return backgroundFill;
        }
        void Canvas::SetBackgroundFill(backgroundFill_t NewFill) {
            std::lock_guard<std::mutex> lock(mtx);

            backgroundFill = NewFill;
        }
    }
}