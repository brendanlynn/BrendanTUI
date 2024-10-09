#include "pch.h"
#include "framework.h"

#include "controlbase.h"

namespace btui {
    Control* FocusManager::GetFocusedControl() {
        return focusedControl.load();
    }
    void FocusManager::SetFocusedControl(Control* NewFocus) {
        Control* pastFocus = focusedControl.exchange(NewFocus);

        if (pastFocus) {
            FocusLostControlInfo info;
            info.newBeholder = NewFocus;

            pastFocus->OnFocusLost(info);
        }

        if (NewFocus) {
            FocusGainedControlInfo info;
            info.lastBeholder = pastFocus;

            NewFocus->OnFocusGained(info);
        }
    }
    void OverwriteWithBackgroundFill(BufferGridCell& Cell, const backgroundFill_t& BackgroundFill) {
        switch (BackgroundFill.index()) {
        case 1:
            Cell = std::get<BufferGridCell>(BackgroundFill);
            break;
        case 2: {
            uint32_t color = std::get<uint32_t>(BackgroundFill);
            Cell.backcolor = OverlayColor(Cell.backcolor, color);
            Cell.forecolor = OverlayColor(Cell.forecolor, color);
            break;
        }
        }
    }
}