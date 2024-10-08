#include "pch.h"
#include "framework.h"

#include "control.h"

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
}