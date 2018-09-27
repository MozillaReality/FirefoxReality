package org.mozilla.browser;

import android.view.Surface;

import org.mozilla.geckoview.GeckoDisplay;

public interface BrowserDisplay {
    void surfaceChanged(Surface mSurface, int aWidth, int aHeight);

    void surfaceDestroyed();

    GeckoDisplay gecko();
}
