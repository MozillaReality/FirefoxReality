package org.mozilla.browser;

import android.view.MotionEvent;

public interface BrowserPanZoomController {
    boolean onTouchEvent(MotionEvent aEvent);

    boolean onMotionEvent(MotionEvent aEvent);
}
