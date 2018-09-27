package org.mozilla.browser;

import android.graphics.Rect;
import android.net.Uri;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import org.mozilla.geckoview.GeckoResult;
import org.mozilla.geckoview.GeckoRuntime;
import org.mozilla.geckoview.GeckoSession;
import org.mozilla.geckoview.GeckoSessionSettings;
import org.mozilla.vrbrowser.SessionStore;

public interface BrowserSession {

    boolean isGecko();

    boolean isServo();

    GeckoSession gecko();

    ProgressDelegate getProgressDelegate();

    void setProgressDelegate(ProgressDelegate delegate);

    NavigationDelegate getNavigationDelegate();

    void setNavigationDelegate(NavigationDelegate delegate);

    ContentDelegate getContentDelegate();

    void setContentDelegate(ContentDelegate delegate);

    void loadUri(Uri uri);

    void loadUri(String uri);

    boolean isOpen();

    void reload();

    void stop();

    void goBack();

    void goForward();

    void getSurfaceBounds(final Rect rect);

    GeckoSessionSettings getSettings();

    void setActive(boolean active);

    void open(GeckoRuntime mRuntime);

    void exitFullScreen();

    void loadData(byte[] aboutPage, String s);

    void close();

    BrowserDisplay acquireDisplay();

    void releaseDisplay(BrowserDisplay mDisplay);

    BrowserPanZoomController getPanZoomController();

    interface NavigationDelegate {
        void onLocationChange(BrowserSession session, String s);
        void onCanGoBack(BrowserSession session, boolean b);
        void onCanGoForward(BrowserSession session, boolean b);
        GeckoResult<Boolean> onLoadRequest(@NonNull BrowserSession session, @NonNull String s, int i, int i1);
        GeckoResult<GeckoSession> onNewSession(@NonNull BrowserSession session, @NonNull String s);
        GeckoResult<String> onLoadError(BrowserSession session, String s, int i, int i1);

        default GeckoSession.NavigationDelegate toGeckoNavigation(SessionStore store) {
            return  new GeckoNavigationDelegate(store, this);
        }
    }

    interface ProgressDelegate {
        void onPageStart(BrowserSession session, String s);
        void onPageStop(BrowserSession session, boolean b);
        void onProgressChange(BrowserSession session, int i);
        void onSecurityChange(BrowserSession session, GeckoSession.ProgressDelegate.SecurityInformation securityInformation);

        default GeckoSession.ProgressDelegate toGeckoProgress(SessionStore store) {
            return  new GeckoProgressDelegate(store, this);
        }
    }

    interface ContentDelegate {
        void onTitleChange(BrowserSession session, String s);
        void onFocusRequest(BrowserSession session);
        void onCloseRequest(BrowserSession session);
        void onFullScreen(BrowserSession session, boolean b);
        void onContextMenu(BrowserSession session, int i, int i1, String s, int i2, String s1);
        void onExternalResponse(BrowserSession session, GeckoSession.WebResponseInfo webResponseInfo);
        void onCrash(BrowserSession session);

        default GeckoSession.ContentDelegate toGeckoContent(SessionStore store) {
            return  new GeckoContentDelegate(store, this);
        }
    }

    class GeckoNavigationDelegate implements GeckoSession.NavigationDelegate {

        private final NavigationDelegate mNavigationDelegate;
        private final SessionStore mStore;

        GeckoNavigationDelegate(SessionStore store, NavigationDelegate navigationDelegate) {
            mStore = store;
            mNavigationDelegate = navigationDelegate;
        }

        private BrowserSession toBrowserSession(GeckoSession geckoSession) {
            return mStore.getSession(geckoSession.hashCode());
        }


        @Override
        public void onLocationChange(GeckoSession geckoSession, String s) {
            mNavigationDelegate.onLocationChange(toBrowserSession(geckoSession), s);
        }

        @Override
        public void onCanGoBack(GeckoSession geckoSession, boolean b) {
            mNavigationDelegate.onCanGoBack(toBrowserSession(geckoSession), b);
        }

        @Override
        public void onCanGoForward(GeckoSession geckoSession, boolean b) {
            mNavigationDelegate.onCanGoForward(toBrowserSession(geckoSession), b);
        }

        @Nullable
        @Override
        public GeckoResult<Boolean> onLoadRequest(@NonNull GeckoSession geckoSession, @NonNull String s, int i, int i1) {
            return mNavigationDelegate.onLoadRequest(toBrowserSession(geckoSession), s, i, i1);
        }

        @Nullable
        @Override
        public GeckoResult<GeckoSession> onNewSession(@NonNull GeckoSession geckoSession, @NonNull String s) {
            return mNavigationDelegate.onNewSession(toBrowserSession(geckoSession), s);
        }

        @Override
        public GeckoResult<String> onLoadError(GeckoSession geckoSession, String s, int i, int i1) {
            return mNavigationDelegate.onLoadError(toBrowserSession(geckoSession), s, i, i1);
        }
    }


    class GeckoProgressDelegate implements GeckoSession.ProgressDelegate {

        private final ProgressDelegate mProgressDelegate;
        private final SessionStore mStore;

        GeckoProgressDelegate(SessionStore store, ProgressDelegate progressDelegate) {
            mStore = store;
            mProgressDelegate = progressDelegate;
        }

        private BrowserSession toBrowserSession(GeckoSession geckoSession) {
            return mStore.getSession(geckoSession.hashCode());
        }

        @Override
        public void onPageStart(GeckoSession geckoSession, String s) {
            mProgressDelegate.onPageStart(toBrowserSession(geckoSession), s);
        }

        @Override
        public void onPageStop(GeckoSession geckoSession, boolean b) {
            mProgressDelegate.onPageStop(toBrowserSession(geckoSession), b);
        }

        @Override
        public void onProgressChange(GeckoSession geckoSession, int i) {
            mProgressDelegate.onProgressChange(toBrowserSession(geckoSession), i);
        }

        @Override
        public void onSecurityChange(GeckoSession geckoSession, SecurityInformation securityInformation) {
            mProgressDelegate.onSecurityChange(toBrowserSession(geckoSession), securityInformation);
        }
    }

    class GeckoContentDelegate implements GeckoSession.ContentDelegate {

        private final ContentDelegate mContentDelegate;
        private final SessionStore mStore;

        GeckoContentDelegate(SessionStore store, ContentDelegate contentDelegate) {
            mStore = store;
            mContentDelegate = contentDelegate;
        }

        private BrowserSession toBrowserSession(GeckoSession geckoSession) {
            return mStore.getSession(geckoSession.hashCode());
        }

        @Override
        public void onTitleChange(GeckoSession geckoSession, String s) {
            mContentDelegate.onTitleChange(toBrowserSession(geckoSession), s);
        }

        @Override
        public void onFocusRequest(GeckoSession geckoSession) {
            mContentDelegate.onFocusRequest(toBrowserSession(geckoSession));
        }

        @Override
        public void onCloseRequest(GeckoSession geckoSession) {
            mContentDelegate.onCloseRequest(toBrowserSession(geckoSession));
        }

        @Override
        public void onFullScreen(GeckoSession geckoSession, boolean b) {
            mContentDelegate.onFullScreen(toBrowserSession(geckoSession), b);
        }

        @Override
        public void onContextMenu(GeckoSession geckoSession, int i, int i1, String s, int i2, String s1) {
            mContentDelegate.onContextMenu(toBrowserSession(geckoSession), i, i1, s, i2, s1);
        }

        @Override
        public void onExternalResponse(GeckoSession geckoSession, GeckoSession.WebResponseInfo webResponseInfo) {
            mContentDelegate.onExternalResponse(toBrowserSession(geckoSession), webResponseInfo);
        }

        @Override
        public void onCrash(GeckoSession geckoSession) {
            mContentDelegate.onCrash(toBrowserSession(geckoSession));
        }
    }

}
