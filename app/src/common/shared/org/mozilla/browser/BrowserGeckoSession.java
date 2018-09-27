package org.mozilla.browser;

import android.graphics.Rect;
import android.net.Uri;

import org.mozilla.geckoview.GeckoRuntime;
import org.mozilla.geckoview.GeckoSession;
import org.mozilla.geckoview.GeckoSessionSettings;
import org.mozilla.vrbrowser.SessionStore;

public class BrowserGeckoSession implements BrowserSession {

    private final GeckoSession mSession;
    private ProgressDelegate mProgressDelegate;
    private NavigationDelegate mNavigationDelegate;
    private ContentDelegate mContentDelegate;
    private SessionStore mStore;
    private BrowserGeckoPanZoomController mPanZoomController;

    public BrowserGeckoSession(SessionStore store) {
        mStore = store;
        mSession = new GeckoSession();
    }

    @Override
    public GeckoSession gecko() {
        return mSession;
    }

    @Override
    public boolean isGecko() {
        return true;
    }

    @Override
    public boolean isServo() {
        return false;
    }

    @Override
    public ProgressDelegate getProgressDelegate() {
        return mProgressDelegate;
    }

    @Override
    public void setProgressDelegate(ProgressDelegate delegate) {
        mProgressDelegate = delegate;
        mSession.setProgressDelegate(delegate.toGeckoProgress(mStore));
    }

    @Override
    public NavigationDelegate getNavigationDelegate() {
        return mNavigationDelegate;
    }

    @Override
    public void setNavigationDelegate(NavigationDelegate delegate) {
        mNavigationDelegate = delegate;
        mSession.setNavigationDelegate(delegate.toGeckoNavigation(mStore));
    }

    @Override
    public ContentDelegate getContentDelegate() {
        return mContentDelegate;
    }

    @Override
    public void setContentDelegate(ContentDelegate delegate) {
        mContentDelegate = delegate;
        mSession.setContentDelegate(delegate.toGeckoContent(mStore));
    }

    @Override
    public void loadUri(Uri uri) {
        mSession.loadUri(uri);
    }

    @Override
    public void loadUri(String uri) {
        mSession.loadUri(uri);
    }

    @Override
    public boolean isOpen() {
        return mSession.isOpen();
    }

    @Override
    public void reload() {
        mSession.reload();
    }

    @Override
    public void stop() {
        mSession.stop();
    }

    @Override
    public void goBack() {
        mSession.goBack();
    }

    @Override
    public void goForward() {
        mSession.goForward();
    }

    @Override
    public void getSurfaceBounds(Rect rect) {
        mSession.getSurfaceBounds(rect);
    }

    @Override
    public GeckoSessionSettings getSettings() {
        return mSession.getSettings();
    }

    @Override
    public void setActive(boolean active) {
        mSession.setActive(active);
    }

    @Override
    public void open(GeckoRuntime runtime) {
        mSession.open(runtime);
    }

    @Override
    public void exitFullScreen() {
        mSession.exitFullScreen();
    }

    @Override
    public void loadData(byte[] data, String s) {
        mSession.loadData(data, s);
    }

    @Override
    public void close() {
        mSession.close();
    }

    @Override
    public BrowserDisplay acquireDisplay() {
        return new BrowserGeckoDisplay(mSession.acquireDisplay());
    }

    @Override
    public void releaseDisplay(BrowserDisplay mDisplay) {
        mSession.releaseDisplay(mDisplay.gecko());
    }

    @Override
    public BrowserPanZoomController getPanZoomController() {
        if (mPanZoomController == null) {
            mPanZoomController = new BrowserGeckoPanZoomController(mSession.getPanZoomController());
        }
        return mPanZoomController;
    }
}
