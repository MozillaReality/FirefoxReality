package org.mozilla.browser;

import android.view.MotionEvent;

import org.mozilla.gecko.gfx.PanZoomController;

class BrowserGeckoPanZoomController implements BrowserPanZoomController {
    PanZoomController mPanZoomController;
    public BrowserGeckoPanZoomController(PanZoomController panZoomController) {
        mPanZoomController = panZoomController;
    }

    @Override
    public boolean onTouchEvent(MotionEvent aEvent) {
        return mPanZoomController.onTouchEvent(aEvent);
    }

    @Override
    public boolean onMotionEvent(MotionEvent aEvent) {
        return mPanZoomController.onMotionEvent(aEvent);
    }
}
