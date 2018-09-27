package org.mozilla.browser;

import android.view.Surface;

import org.mozilla.geckoview.GeckoDisplay;

class BrowserGeckoDisplay implements BrowserDisplay {

    private final GeckoDisplay mDisplay;

    public BrowserGeckoDisplay(GeckoDisplay geckoDisplay) {
        mDisplay = geckoDisplay;
    }

    @Override
    public void surfaceChanged(Surface surface, int aWidth, int aHeight) {
        mDisplay.surfaceChanged(surface, aWidth, aHeight);
    }

    @Override
    public void surfaceDestroyed() {
        mDisplay.surfaceDestroyed();
    }

    @Override
    public GeckoDisplay gecko() {
        return mDisplay;
    }
}
