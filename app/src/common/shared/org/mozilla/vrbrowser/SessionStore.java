/* -*- Mode: Java; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.vrbrowser;

import android.content.Context;
import android.graphics.Rect;
import android.os.SystemClock;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.Log;
import android.view.inputmethod.CursorAnchorInfo;
import android.view.inputmethod.ExtractedText;
import android.view.inputmethod.ExtractedTextRequest;

import org.mozilla.browser.BrowserGeckoSession;
import org.mozilla.browser.BrowserSession;
import org.mozilla.gecko.GeckoAppShell;
import org.mozilla.gecko.GeckoProfile;
import org.mozilla.geckoview.*;
import org.mozilla.geckoview.GeckoSession;
import org.mozilla.vrbrowser.telemetry.TelemetryWrapper;
import org.mozilla.geckoview.GeckoSession.ProgressDelegate.*;
import org.mozilla.geckoview.GeckoSession.NavigationDelegate.*;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.*;

public class SessionStore implements BrowserSession.NavigationDelegate, BrowserSession.ProgressDelegate,
        BrowserSession.ContentDelegate, GeckoSession.TrackingProtectionDelegate, GeckoSession.TextInputDelegate,
        GeckoSession.PromptDelegate {

    private static SessionStore mInstance;
    private static final String LOGTAG = "VRB";
    public static SessionStore get() {
        if (mInstance == null) {
            mInstance = new SessionStore();
        }
        return mInstance;
    }
    private static final String HOME_WITHOUT_REGION_ORIGIN = "https://webxr.today/";
    public static final String PRIVATE_BROWSING_URI = "about:privatebrowsing";
    public static final int NO_SESSION_ID = -1;

    private LinkedList<BrowserSession.NavigationDelegate> mNavigationListeners;
    private LinkedList<BrowserSession.ProgressDelegate> mProgressListeners;
    private LinkedList<BrowserSession.ContentDelegate> mContentListeners;
    private LinkedList<SessionChangeListener> mSessionChangeListeners;
    private LinkedList<GeckoSession.TextInputDelegate> mTextInputListeners;
    private LinkedList<GeckoSession.PromptDelegate> mPromptListeners;
    private final long MIN_LOAD_TIME = 40;
    private long startLoadTime = 0;

    public interface SessionChangeListener {
        void onNewSession(BrowserSession aBrowserSession, int aId);
        void onRemoveSession(BrowserSession aBrowserSession, int aId);
        void onCurrentSessionChange(BrowserSession aBrowserSession, int aId);
    }

    class State {
        boolean mCanGoBack;
        boolean mCanGoForward;
        boolean mIsLoading;
        boolean mIsInputActive;
        GeckoSession.ProgressDelegate.SecurityInformation mSecurityInformation;
        String mUri;
        String mTitle;
        boolean mFullScreen;
        BrowserSession mBrowserSession;
    }

    private GeckoRuntime mRuntime;
    private BrowserSession mCurrentBrowserSession;
    private HashMap<Integer, State> mSessions;
    private Deque<Integer> mSessionsStack;
    private Deque<Integer> mPrivateSessionsStack;
    private GeckoSession.PermissionDelegate mPermissionDelegate;
    private int mPreviousSessionId = SessionStore.NO_SESSION_ID;
    private String mRegion;
    private Context mContext;

    private SessionStore() {
        mNavigationListeners = new LinkedList<>();
        mProgressListeners = new LinkedList<>();
        mContentListeners = new LinkedList<>();
        mSessionChangeListeners = new LinkedList<>();
        mTextInputListeners = new LinkedList<>();
        mPromptListeners = new LinkedList<>();

        mSessions = new LinkedHashMap<>();
        mSessionsStack = new ArrayDeque<>();
        mPrivateSessionsStack = new ArrayDeque<>();
    }

    public void clearListeners() {
        mNavigationListeners.clear();
        mProgressListeners.clear();
        mContentListeners.clear();
        mSessionChangeListeners.clear();
        mTextInputListeners.clear();
    }

    private InternalPages.PageResources errorPageResourcesForCategory(@LoadErrorCategory int category) {
        switch (category) {
            case GeckoSession.NavigationDelegate.ERROR_CATEGORY_UNKNOWN: {
                return InternalPages.PageResources.create(R.raw.error_pages, R.raw.error_style);
            }
            case GeckoSession.NavigationDelegate.ERROR_CATEGORY_SECURITY: {
                return InternalPages.PageResources.create(R.raw.error_pages, R.raw.error_style);
            }
            case GeckoSession.NavigationDelegate.ERROR_CATEGORY_NETWORK: {
                return InternalPages.PageResources.create(R.raw.error_pages, R.raw.error_style);
            }
            case GeckoSession.NavigationDelegate.ERROR_CATEGORY_CONTENT: {
                return InternalPages.PageResources.create(R.raw.error_pages, R.raw.error_style);
            }
            case GeckoSession.NavigationDelegate.ERROR_CATEGORY_URI: {
                return InternalPages.PageResources.create(R.raw.error_pages, R.raw.error_style);
            }
            case GeckoSession.NavigationDelegate.ERROR_CATEGORY_PROXY: {
                return InternalPages.PageResources.create(R.raw.error_pages, R.raw.error_style);
            }
            case GeckoSession.NavigationDelegate.ERROR_CATEGORY_SAFEBROWSING: {
                return InternalPages.PageResources.create(R.raw.error_pages, R.raw.error_style);
            }
            default: {
                return InternalPages.PageResources.create(R.raw.error_pages, R.raw.error_style);
            }
        }
    }

    public void setContext(Context aContext) {
        if (mRuntime == null) {
            // FIXME: Once GeckoView has a prefs API
            vrPrefsWorkAround(aContext);
            GeckoRuntimeSettings.Builder runtimeSettingsBuilder = new GeckoRuntimeSettings.Builder();
//            runtimeSettingsBuilder.javaCrashReportingEnabled(SettingsStore.getInstance(aContext).isCrashReportingEnabled());
//            runtimeSettingsBuilder.nativeCrashReportingEnabled(SettingsStore.getInstance(aContext).isCrashReportingEnabled());
            runtimeSettingsBuilder.trackingProtectionCategories(GeckoSession.TrackingProtectionDelegate.CATEGORY_AD | GeckoSession.TrackingProtectionDelegate.CATEGORY_SOCIAL | GeckoSession.TrackingProtectionDelegate.CATEGORY_ANALYTIC);
            runtimeSettingsBuilder.consoleOutput(SettingsStore.getInstance(aContext).isConsoleLogsEnabled());
            runtimeSettingsBuilder.displayDensityOverride(SettingsStore.getInstance(aContext).getDisplayDensity());
            runtimeSettingsBuilder.remoteDebuggingEnabled(SettingsStore.getInstance(aContext).isRemoteDebuggingEnabled());
            runtimeSettingsBuilder.displayDpiOverride(SettingsStore.getInstance(aContext).getDisplayDpi());
            runtimeSettingsBuilder.screenSizeOverride(SettingsStore.getInstance(aContext).getMaxWindowWidth(),
                    SettingsStore.getInstance(aContext).getMaxWindowHeight());

            if (BuildConfig.DEBUG) {
                runtimeSettingsBuilder.arguments(new String[] { "-purgecaches" });
            }

            mRuntime = GeckoRuntime.create(aContext, runtimeSettingsBuilder.build());

        } else {
            mRuntime.attachTo(aContext);
        }

        mContext = aContext;
    }

    public void dumpAllState(Integer sessionId) {
        dumpAllState(getSession(sessionId));
    }

    private boolean isLocalizedContent(@Nullable String url) {
        return url != null && (url.startsWith("about:") || url.startsWith("data:"));
    }

    private void dumpAllState(BrowserSession aBrowserSession) {
        for (BrowserSession.NavigationDelegate listener: mNavigationListeners) {
            dumpState(aBrowserSession, listener);
        }
        for (BrowserSession.ProgressDelegate listener: mProgressListeners) {
            dumpState(aBrowserSession, listener);
        }
        for (BrowserSession.ContentDelegate listener: mContentListeners) {
            dumpState(aBrowserSession, listener);
        }
    }

    private void dumpState(BrowserSession aBrowserSession, BrowserSession.NavigationDelegate aListener) {
        boolean canGoForward = false;
        boolean canGoBack = false;
        String uri = "";
        if (aBrowserSession != null) {
            State state = mSessions.get(aBrowserSession.hashCode());
            if (state != null) {
                canGoBack = state.mCanGoBack;
                canGoForward = state.mCanGoForward;
                uri = state.mUri;
            }
        }
        aListener.onCanGoBack(aBrowserSession, canGoBack);
        aListener.onCanGoForward(aBrowserSession, canGoForward);
        aListener.onLocationChange(aBrowserSession, uri);
    }

    private void dumpState(BrowserSession aBrowserSession, BrowserSession.ProgressDelegate aListener) {
        boolean isLoading = false;
        GeckoSession.ProgressDelegate.SecurityInformation securityInfo = null;
        String uri = "";
        if (aBrowserSession != null) {
            State state = mSessions.get(aBrowserSession.hashCode());
            if (state != null) {
                isLoading = state.mIsLoading;
                securityInfo = state.mSecurityInformation;
                uri = state.mUri;
            }
        }
        if (isLoading) {
            aListener.onPageStart(aBrowserSession, uri);
        }

        if (securityInfo != null) {
            aListener.onSecurityChange(aBrowserSession, securityInfo);
        }
    }

    public void dumpState(BrowserSession aBrowserSession, BrowserSession.ContentDelegate aListener) {
        String title = "";
        if (aBrowserSession != null) {
            State state = mSessions.get(aBrowserSession.hashCode());
            if (state != null) {
                title = state.mTitle;
            }
        }

        aListener.onTitleChange(aBrowserSession, title);
    }

    public void addNavigationListener(BrowserSession.NavigationDelegate aListener) {
        mNavigationListeners.add(aListener);
        dumpState(mCurrentBrowserSession, aListener);
    }

    public void removeNavigationListener(BrowserSession.NavigationDelegate aListener) {
        mNavigationListeners.remove(aListener);
    }

    public void addProgressListener(BrowserSession.ProgressDelegate aListener) {
        mProgressListeners.add(aListener);
        dumpState(mCurrentBrowserSession, aListener);
    }

    public void removeProgressListener(BrowserSession.ProgressDelegate aListener) {
        mProgressListeners.remove(aListener);
    }

    public void addContentListener(BrowserSession.ContentDelegate aListener) {
        mContentListeners.add(aListener);
        dumpState(mCurrentBrowserSession, aListener);
    }

    public void removeContentListener(BrowserSession.ContentDelegate aListener) {
        mContentListeners.remove(aListener);
    }

    public void addSessionChangeListener(SessionChangeListener aListener) {
        mSessionChangeListeners.add(aListener);
    }

    public void removeSessionChangeListener(SessionChangeListener aListener) {
        mSessionChangeListeners.remove(aListener);
    }

    public void addTextInputListener(GeckoSession.TextInputDelegate aListener) {
        mTextInputListeners.add(aListener);
    }

    public void removeTextInputListener(GeckoSession.TextInputDelegate aListener) {
        mTextInputListeners.remove(aListener);
    }

    public void addPromptListener(GeckoSession.PromptDelegate aListener) {
        mPromptListeners.add(aListener);
    }

    public void removePromptListener(GeckoSession.PromptDelegate aListener) {
        mPromptListeners.remove(aListener);
    }

    public class SessionSettings {
        public boolean multiprocess = SettingsStore.getInstance(mContext).isMultiprocessEnabled();
        public boolean privateMode = false;
        public boolean trackingProtection = true;
        public boolean suspendMediaWhenInactive = true;
        public int userAgentMode = SettingsStore.getInstance(mContext).getUaMode();
        public boolean servo = false;
    }

    public int createSession() {
        return createSession(new SessionSettings());
    }
    public int createSession(SessionSettings aSettings) {
        State state = new State();

//        if (!aSettings.servo) {
//            state.mBrowserSession = new BrowserGeckoSession();
//        } else {
//            state.mBrowserSession = new BrowserServoSession(mContext);
//        }

        state.mBrowserSession = new BrowserGeckoSession(this);

        int result = state.mBrowserSession.hashCode();
        mSessions.put(result, state);
        state.mBrowserSession.getSettings().setBoolean(GeckoSessionSettings.USE_MULTIPROCESS, aSettings.multiprocess);
        state.mBrowserSession.getSettings().setBoolean(GeckoSessionSettings.USE_PRIVATE_MODE, aSettings.privateMode);
        state.mBrowserSession.getSettings().setBoolean(GeckoSessionSettings.USE_TRACKING_PROTECTION, aSettings.trackingProtection);
        state.mBrowserSession.getSettings().setBoolean(GeckoSessionSettings.SUSPEND_MEDIA_WHEN_INACTIVE, aSettings.suspendMediaWhenInactive);
        state.mBrowserSession.getSettings().setInt(GeckoSessionSettings.USER_AGENT_MODE, aSettings.userAgentMode);
        state.mBrowserSession.setNavigationDelegate(this);
        state.mBrowserSession.setProgressDelegate(this);
        state.mBrowserSession.setContentDelegate(this);

        if (state.mBrowserSession.isGecko()) {
            state.mBrowserSession.gecko().setPromptDelegate(this);
            state.mBrowserSession.gecko().getTextInput().setDelegate(this);
            state.mBrowserSession.gecko().setPermissionDelegate(mPermissionDelegate);
            state.mBrowserSession.gecko().setTrackingProtectionDelegate(this);
        }

        for (SessionChangeListener listener: mSessionChangeListeners) {
            listener.onNewSession(state.mBrowserSession, result);
        }

        return result;
    }

    public void removeSession(int aSessionId) {
        BrowserSession session = getSession(aSessionId);
        if (session != null) {
            session.setContentDelegate(null);
            session.setNavigationDelegate(null);
            session.setProgressDelegate(null);

            if (session.isGecko()) {
                session.gecko().getTextInput().setDelegate(null);
            }
            mSessions.remove(aSessionId);
            for (SessionChangeListener listener: mSessionChangeListeners) {
                listener.onRemoveSession(session, aSessionId);
            }
            session.setActive(false);
            session.stop();
        }
    }

    private void pushSession(int aSessionId) {
        boolean isPrivateMode  = mCurrentBrowserSession.getSettings().getBoolean(GeckoSessionSettings.USE_PRIVATE_MODE);
        if (isPrivateMode)
            mPrivateSessionsStack.push(aSessionId);
        else
            mSessionsStack.push(aSessionId);
    }

    private Integer popSession() {
        boolean isPrivateMode  = mCurrentBrowserSession.getSettings().getBoolean(GeckoSessionSettings.USE_PRIVATE_MODE);
        if (isPrivateMode)
            return mPrivateSessionsStack.pop();
        else
            return mSessionsStack.pop();
    }

    private Integer peekSession() {
        boolean isPrivateMode  = mCurrentBrowserSession.getSettings().getBoolean(GeckoSessionSettings.USE_PRIVATE_MODE);
        if (isPrivateMode)
            return mPrivateSessionsStack.peek();
        else
            return mSessionsStack.peek();
    }

    public BrowserSession getSession(int aId) {
        State result = mSessions.get(aId);
        if (result == null) {
            return null;
        }
        return result.mBrowserSession;
    }

    public Integer getSessionId(BrowserSession aBrowserSession) {
        for (Map.Entry<Integer, State> entry : mSessions.entrySet()) {
            if (entry.getValue().mBrowserSession == aBrowserSession) {
                return  entry.getKey();
            }
        }
        return null;
    }

    public Integer getSessionId(GeckoSession aGeckoSession) {
        return getSessionId(getSession(aGeckoSession.hashCode()));
    }

    public String getUriFromSession(BrowserSession aBrowserSession) {
        Integer sessionId = getSessionId(aBrowserSession);
        if (sessionId == null) {
            return "";
        }
        State state = mSessions.get(sessionId);
        if (state != null) {
            return state.mUri;
        }

        return "";
    }

    public List<Integer> getSessions() {
        return new ArrayList<>(mSessions.keySet());
    }

    public List<Integer> getSessionsByPrivateMode(boolean aUsingPrivateMode) {
        ArrayList<Integer> result = new ArrayList<>();
        for (Integer sessionId : mSessions.keySet()) {
            BrowserSession browserSession = getSession(sessionId);
            if (browserSession != null && browserSession.getSettings().getBoolean(GeckoSessionSettings.USE_PRIVATE_MODE) == aUsingPrivateMode) {
                result.add(sessionId);
            }
        }
        return result;
    }

    public void setCurrentSession(int aId) {
        if (mRuntime == null) {
            Log.e(LOGTAG, "SessionStore failed to set current session, GeckoRuntime is null");
            return;
        }

        Log.d(LOGTAG, "Creating session: " + aId);

        if (mCurrentBrowserSession != null) {
            mCurrentBrowserSession.setActive(false);
        }

        mCurrentBrowserSession = null;
        State state = mSessions.get(aId);
        if (state != null) {
            mCurrentBrowserSession = state.mBrowserSession;
            if (!mCurrentBrowserSession.isOpen()) {
                mCurrentBrowserSession.open(mRuntime);
            }
            for (SessionChangeListener listener: mSessionChangeListeners) {
                listener.onCurrentSessionChange(mCurrentBrowserSession, aId);
            }
        }
        dumpAllState(mCurrentBrowserSession);

        if (mCurrentBrowserSession != null)
            mCurrentBrowserSession.setActive(true);
    }

    public void setRegion(String aRegion) {
        Log.d(LOGTAG, "SessionStore setRegion: " + aRegion);
        mRegion = aRegion != null ? aRegion.toLowerCase() : "worldwide";

        // There is a region update and the home is already loaded
        if (mCurrentBrowserSession != null && isHomeUri(getCurrentUri())) {
            mCurrentBrowserSession.loadUri("javascript:window.location.replace('" + getHomeUri() + "');");
        }
    }

    public String getHomeUri() {
        String result = SessionStore.HOME_WITHOUT_REGION_ORIGIN;
        if (mRegion != null) {
            result = SessionStore.HOME_WITHOUT_REGION_ORIGIN + "?region=" + mRegion;
        }
        return result;
    }

    public Boolean isHomeUri(String aUri) {
        return aUri != null && aUri.toLowerCase().startsWith(SessionStore.HOME_WITHOUT_REGION_ORIGIN);
    }

    public String getCurrentUri() {
        String result = "";
        if (mCurrentBrowserSession != null) {
            State state = mSessions.get(mCurrentBrowserSession.hashCode());
            if (state == null) {
                return result;
            }
            result = state.mUri;
        }
        return result;
    }

    public boolean isInputActive(int aSessionId) {
        SessionStore.State state = mSessions.get(aSessionId);
        if (state != null) {
            return state.mIsInputActive;
        }
        return false;
    }

    public boolean canGoBack() {
        if (mCurrentBrowserSession == null) {
            return false;
        }

        State state = mSessions.get(mCurrentBrowserSession.hashCode());
        if (state != null) {
            return state.mCanGoBack;
        }

        return false;
    }

    public void goBack() {
        if (mCurrentBrowserSession == null) {
             return;
        }
        if (isInFullScreen()) {
            exitFullScreen();
        } else {
            mCurrentBrowserSession.goBack();
        }
    }

    public void goForward() {
        if (mCurrentBrowserSession == null) {
            return;
        }
        mCurrentBrowserSession.goForward();
    }

    public void setActive(boolean aActive) {
        if (mCurrentBrowserSession == null) {
            return;
        }
        mCurrentBrowserSession.setActive(aActive);
    }


    public void reload() {
        if (mCurrentBrowserSession == null) {
            return;
        }
        mCurrentBrowserSession.reload();
    }

    public void stop() {
        if (mCurrentBrowserSession == null) {
            return;
        }
        mCurrentBrowserSession.stop();
    }

    public void loadUri(String aUri) {
        if (mCurrentBrowserSession == null) {
            return;
        }

        if (aUri == null) {
            aUri = getHomeUri();
        }
        Log.d(LOGTAG, "Loading URI: " + aUri);
        mCurrentBrowserSession.loadUri(aUri);
    }

    public void reloadWithServo() {
        String uri = getCurrentUri();
        SessionStore.SessionSettings settings = new SessionStore.SessionSettings();
        settings.servo = true;
        int id = createSession(settings);
        setCurrentSession(id);
        loadUri(uri);
    }

    public boolean isInFullScreen() {
        if (mCurrentBrowserSession == null) {
            return false;
        }

        State state = mSessions.get(mCurrentBrowserSession.hashCode());
        if (state != null) {
            return state.mFullScreen;
        }

        return false;
    }

    public void exitFullScreen() {
        if (mCurrentBrowserSession == null) {
            return;
        }
        mCurrentBrowserSession.exitFullScreen();
    }

    public BrowserSession getCurrentSession() {
        return mCurrentBrowserSession;
    }

    public int getCurrentSessionId() {
        if (mCurrentBrowserSession == null) {
            return NO_SESSION_ID;
        }
        return mCurrentBrowserSession.hashCode();
    }

    public void setPermissionDelegate(GeckoSession.PermissionDelegate aDelegate) {
        mPermissionDelegate = aDelegate;
        for (HashMap.Entry<Integer, State> entry : mSessions.entrySet()) {
            BrowserSession session = entry.getValue().mBrowserSession;
            if (session.isGecko()) {
                session.gecko().setPermissionDelegate(aDelegate);
            }
        }
    }

    private void vrPrefsWorkAround(Context aContext) {
        File path = GeckoProfile.initFromArgs(aContext, null).getDir();
        String prefFileName = path.getAbsolutePath() + File.separator + "user.js";
        Log.i(LOGTAG, "Creating file: " + prefFileName);
        try (FileOutputStream out = new FileOutputStream(prefFileName)) {
            out.write("pref(\"dom.vr.enabled\", true);\n".getBytes());
            out.write("pref(\"dom.vr.external.enabled\", true);\n".getBytes());
            out.write("pref(\"webgl.enable-surface-texture\", true);\n".getBytes());
            out.write("pref(\"apz.allow_double_tap_zooming\", false);\n".getBytes());
            out.write("pref(\"dom.webcomponents.customelements.enabled\", false);\n".getBytes());
        } catch (FileNotFoundException e) {
            Log.e(LOGTAG, "Unable to create file: '" + prefFileName + "' got exception: " + e.toString());
        } catch (IOException e) {
            Log.e(LOGTAG, "Unable to write file: '" + prefFileName + "' got exception: " + e.toString());
        }
    }

    public void switchPrivateMode() {
        if (mCurrentBrowserSession == null)
            return;

        boolean isPrivateMode = mCurrentBrowserSession.getSettings().getBoolean(GeckoSessionSettings.USE_PRIVATE_MODE);
        if (!isPrivateMode) {
            if (mPreviousSessionId == SessionStore.NO_SESSION_ID) {
                mPreviousSessionId = getCurrentSessionId();

                SessionStore.SessionSettings settings = new SessionStore.SessionSettings();
                settings.privateMode = true;
                int id = createSession(settings);
                setCurrentSession(id);

                InternalPages.PageResources pageResources = InternalPages.PageResources.create(R.raw.private_mode, R.raw.private_style);
                getCurrentSession().loadData(InternalPages.createAboutPage(mContext, pageResources), "text/html");

            } else {
                int sessionId = getCurrentSessionId();
                setCurrentSession(mPreviousSessionId);
                mPreviousSessionId = sessionId;
            }

        } else {
            int sessionId = getCurrentSessionId();
            setCurrentSession(mPreviousSessionId);
            mPreviousSessionId = sessionId;
        }
    }

    public void setUaMode(int mode) {
        if (mCurrentBrowserSession != null) {
            mCurrentBrowserSession.getSettings().setInt(GeckoSessionSettings.USER_AGENT_MODE, mode);
            mCurrentBrowserSession.reload();
        }
    }

    public void exitPrivateMode() {
        if (mCurrentBrowserSession == null)
            return;

        boolean isPrivateMode  = mCurrentBrowserSession.getSettings().getBoolean(GeckoSessionSettings.USE_PRIVATE_MODE);
        if (isPrivateMode) {
            int privateSessionId = getCurrentSessionId();
            setCurrentSession(mPreviousSessionId);
            mPreviousSessionId = SessionStore.NO_SESSION_ID;

            // Remove current private_mode session
            removeSession(privateSessionId);

            // Remove all the stacked private_mode sessions
            for (Iterator<Integer> it = mPrivateSessionsStack.iterator(); it.hasNext();) {
                int sessionId = it.next();
                removeSession(sessionId);
            }
            mPrivateSessionsStack.clear();
        }
    }

    public boolean isCurrentSessionPrivate() {
        if (mCurrentBrowserSession != null)
            return mCurrentBrowserSession.getSettings().getBoolean(GeckoSessionSettings.USE_PRIVATE_MODE);

        return false;
    }

    public boolean canUnstackSession() {
        Integer prevSessionId = peekSession();

        return prevSessionId != null;
    }

    public void unstackSession() {
        Integer prevSessionId = popSession();
        if (prevSessionId != null) {
            int currentSession = getCurrentSessionId();
            setCurrentSession(prevSessionId);
            removeSession(currentSession);
        }
    }

    public void setConsoleOutputEnabled(boolean enabled) {
        if (mRuntime != null) {
            mRuntime.getSettings().setConsoleOutputEnabled(enabled);
        }
    }

    public void setMaxWindowSize(int width, int height) {
        GeckoAppShell.setScreenSizeOverride(new Rect(0, 0, width, height));
    }

    public void setServo(final boolean enabled) {
      if (!enabled) {
        // FIXME: reset session to gecko
      }
    }

    public void setMultiprocess(final boolean enabled) {
        if (mCurrentBrowserSession != null && mCurrentBrowserSession.isGecko()) {
            GeckoSession session = mCurrentBrowserSession.gecko();
            final GeckoResult<GeckoSession.SessionState> state = session.saveState();
            state.then(new GeckoResult.OnValueListener<GeckoSession.SessionState, Object>() {
                @Nullable
                @Override
                public GeckoResult<Object> onValue(@Nullable GeckoSession.SessionState value) throws Throwable {
                    if (value != null) {
                        mCurrentBrowserSession.stop();
                        mCurrentBrowserSession.close();

                        int oldSessionId = getCurrentSessionId();
                        int sessionId = createSession();
                        GeckoSession session = getSession(sessionId).gecko();
                        session.getSettings().setBoolean(GeckoSessionSettings.USE_MULTIPROCESS, enabled);
                        session.restoreState(value);
                        setCurrentSession(sessionId);
                        removeSession(oldSessionId);
                    }

                    return null;
                }
            }, new GeckoResult.OnExceptionListener<Object>() {
                @Nullable
                @Override
                public GeckoResult<Object> onException(@NonNull Throwable exception) throws Throwable {
                    Log.e(LOGTAG, "State saving exception while setting multiprocess mode: " + exception.getLocalizedMessage());
                    return null;
                }
            });
        }
    }

    public void setRemoteDebugging(final boolean enabled) {
        if (mRuntime != null) {
            mRuntime.getSettings().setRemoteDebuggingEnabled(enabled);
        }
    }

    @Override
    public void onLocationChange(BrowserSession aBrowserSession, String aUri) {
        Log.d(LOGTAG, "SessionStore onLocationChange: " + aUri);
        State state = mSessions.get(aBrowserSession.hashCode());
        if (state == null) {
            Log.e(LOGTAG, "Unknown session!");
            return;
        }

        state.mUri = aUri;

        for (BrowserSession.NavigationDelegate listener: mNavigationListeners) {
            listener.onLocationChange(aBrowserSession, aUri);
        }

        // The homepage finishes loading after the region has been updated
        if (mRegion != null && aUri.equalsIgnoreCase(SessionStore.HOME_WITHOUT_REGION_ORIGIN)) {
            aBrowserSession.loadUri("javascript:window.location.replace('" + getHomeUri() + "');");
        }
    }

    @Override
    public void onCanGoBack(BrowserSession aBrowserSession, boolean aCanGoBack) {
        Log.d(LOGTAG, "SessionStore onCanGoBack: " + (aCanGoBack ? "true" : "false"));
        State state = mSessions.get(aBrowserSession.hashCode());
        if (state == null) {
            return;
        }
        state.mCanGoBack = aCanGoBack;
        for (BrowserSession.NavigationDelegate listener: mNavigationListeners) {
            listener.onCanGoBack(aBrowserSession, aCanGoBack);
        }
    }

    @Override
    public void onCanGoForward(BrowserSession aBrowserSession, boolean aCanGoForward) {
        Log.d(LOGTAG, "SessionStore onCanGoForward: " + (aCanGoForward ? "true" : "false"));
        State state = mSessions.get(aBrowserSession.hashCode());
        if (state == null) {
            return;
        }
        state.mCanGoForward = aCanGoForward;
        for (BrowserSession.NavigationDelegate listener: mNavigationListeners) {
            listener.onCanGoForward(aBrowserSession, aCanGoForward);
        }
    }

    @Nullable
    public GeckoResult<Boolean> onLoadRequest(@NonNull BrowserSession aSession,
                                 @NonNull String aUri,
                                 @TargetWindow int target,
                                 @LoadRequestFlags int flags) {
        GeckoResult<Boolean> result = new GeckoResult<>();
        if (aUri.equalsIgnoreCase(PRIVATE_BROWSING_URI)) {
            switchPrivateMode();
            result.complete(true);
        } else {
            result.complete(false);
        }

        return result;
    }

    @Override
    public GeckoResult<GeckoSession> onNewSession(@NonNull BrowserSession aSession, @NonNull String aUri) {
        Log.d(LOGTAG, "SessionStore onNewSession: " + aUri);

        pushSession(getCurrentSessionId());

        int sessionId;
        boolean isPreviousPrivateMode = mCurrentBrowserSession.getSettings().getBoolean(GeckoSessionSettings.USE_PRIVATE_MODE);
        if (isPreviousPrivateMode) {
            SessionStore.SessionSettings settings = new SessionStore.SessionSettings();
            settings.privateMode = true;
            sessionId = createSession(settings);

        } else {
            sessionId = createSession();
        }

        mCurrentBrowserSession = null;
        State state = mSessions.get(sessionId);
        if (state != null) {
            mCurrentBrowserSession = state.mBrowserSession;
            for (SessionChangeListener listener : mSessionChangeListeners) {
                listener.onCurrentSessionChange(mCurrentBrowserSession, sessionId);
            }
        }
        dumpAllState(mCurrentBrowserSession);

        BrowserSession session = getSession(sessionId);
        assert(session.isGecko());
        return GeckoResult.fromValue(session.gecko());
    }

    @Override
    public GeckoResult<String> onLoadError(BrowserSession session, String uri, int category, int error) {
        Log.d(LOGTAG, "SessionStore onLoadError: " + uri);

        InternalPages.PageResources pageResources = errorPageResourcesForCategory(category);
        return GeckoResult.fromValue(InternalPages.createErrorPage(mContext, uri, pageResources, category, error));
    }

    // Progress Listener
    @Override
    public void onPageStart(BrowserSession aSession, String aUri) {
        Log.d(LOGTAG, "SessionStore onPageStart");
        State state = mSessions.get(aSession.hashCode());
        if (state == null) {
            return;
        }
        state.mIsLoading = true;
        startLoadTime = SystemClock.elapsedRealtime();
        for (BrowserSession.ProgressDelegate listener: mProgressListeners) {
            listener.onPageStart(aSession, aUri);
        }
    }

    @Override
    public void onPageStop(BrowserSession aSession, boolean b) {
        Log.d(LOGTAG, "SessionStore onPageStop");
        State state = mSessions.get(aSession.hashCode());
        if (state == null) {
            return;
        }

        state.mIsLoading = false;
        long elapsedLoad = SystemClock.elapsedRealtime() - startLoadTime;
        if (elapsedLoad > MIN_LOAD_TIME && !isLocalizedContent(state.mUri)) {
            Log.i(LOGTAG, "Sent load to histogram");
            TelemetryWrapper.addLoadToHistogram(state.mUri, elapsedLoad);
        }
        for (BrowserSession.ProgressDelegate listener: mProgressListeners) {
            listener.onPageStop(aSession, b);
        }
    }

    @Override
    public void onProgressChange(BrowserSession session, int progress) {

    }

    @Override
    public void onSecurityChange(BrowserSession aSession, SecurityInformation aInformation) {
        Log.d(LOGTAG, "SessionStore onPageStop");
        State state = mSessions.get(aSession.hashCode());
        if (state == null) {
            return;
        }

        state.mSecurityInformation = aInformation;
        for (BrowserSession.ProgressDelegate listener: mProgressListeners) {
            listener.onSecurityChange(aSession, aInformation);
        }
    }

    // Content Delegate
    @Override
    public void onTitleChange(BrowserSession aSession, String aTitle) {
        Log.d(LOGTAG, "SessionStore onTitleChange");
        State state = mSessions.get(aSession.hashCode());
        if (state == null) {
            return;
        }

        state.mTitle = aTitle;
        for (BrowserSession.ContentDelegate listener: mContentListeners) {
            listener.onTitleChange(aSession, aTitle);
        }
    }

    @Override
    public void onFocusRequest(BrowserSession aSession) {
        Log.d(LOGTAG, "SessionStore onFocusRequest");
    }

    @Override
    public void onCloseRequest(BrowserSession aSession) {
        int sessionId = getSessionId(aSession);
        if (getCurrentSessionId() == sessionId) {
            unstackSession();
        }
    }

    @Override
    public void onFullScreen(BrowserSession aSession, boolean aFullScreen) {
        Log.d(LOGTAG, "SessionStore onFullScreen");
        State state = mSessions.get(aSession.hashCode());
        if (state == null) {
            return;
        }
        state.mFullScreen = aFullScreen;
        for (BrowserSession.ContentDelegate listener: mContentListeners) {
            listener.onFullScreen(aSession, aFullScreen);
        }
    }

    @Override
    public void onContextMenu(BrowserSession aSession, int i, int i1, String s, int i2, String s1) {

    }

    @Override
    public void onExternalResponse(BrowserSession session, GeckoSession.WebResponseInfo response) {

    }

    @Override
    public void onCrash(BrowserSession session) {

    }

    // TextInput Delegate - Gecko Only

    @Override
    public void restartInput(@NonNull GeckoSession aSession, int reason) {
        if (aSession == mCurrentBrowserSession) {
            for (GeckoSession.TextInputDelegate listener : mTextInputListeners) {
                listener.restartInput(aSession, reason);
            }
        }
    }

    @Override
    public void showSoftInput(@NonNull GeckoSession aSession) {
        SessionStore.State state = mSessions.get(getSessionId(aSession));
        if (state != null) {
            state.mIsInputActive = true;
        }
        if (aSession == mCurrentBrowserSession) {
            for (GeckoSession.TextInputDelegate listener : mTextInputListeners) {
                listener.showSoftInput(aSession);
            }
        }
    }

    @Override
    public void hideSoftInput(@NonNull GeckoSession aSession) {
        SessionStore.State state = mSessions.get(getSessionId(aSession));
        if (state != null) {
            state.mIsInputActive = false;
        }
        if (aSession == mCurrentBrowserSession) {
            for (GeckoSession.TextInputDelegate listener : mTextInputListeners) {
                listener.hideSoftInput(aSession);
            }
        }
    }

    @Override
    public void updateSelection(@NonNull GeckoSession aSession, int selStart, int selEnd, int compositionStart, int compositionEnd) {
        if (aSession == mCurrentBrowserSession) {
            for (GeckoSession.TextInputDelegate listener : mTextInputListeners) {
                listener.updateSelection(aSession, selStart, selEnd, compositionStart, compositionEnd);
            }
        }
    }

    @Override
    public void updateExtractedText(@NonNull GeckoSession aSession, @NonNull ExtractedTextRequest request, @NonNull ExtractedText text) {
        if (aSession == mCurrentBrowserSession) {
            for (GeckoSession.TextInputDelegate listener : mTextInputListeners) {
                listener.updateExtractedText(aSession, request, text);
            }
        }
    }

    @Override
    public void updateCursorAnchorInfo(@NonNull GeckoSession aSession, @NonNull CursorAnchorInfo info) {
        if (aSession == mCurrentBrowserSession) {
            for (GeckoSession.TextInputDelegate listener : mTextInputListeners) {
                listener.updateCursorAnchorInfo(aSession, info);
            }
        }
    }

    @Override
    public void notifyAutoFill(GeckoSession session, int notification, int virtualId) {

    }

    @Override
    public void onTrackerBlocked(GeckoSession session, String uri, int categories) {
        if ((categories & GeckoSession.TrackingProtectionDelegate.CATEGORY_AD) != 0) {
          Log.i(LOGTAG, "Blocking Ad: " + uri);
        }

        if ((categories & GeckoSession.TrackingProtectionDelegate.CATEGORY_ANALYTIC) != 0) {
            Log.i(LOGTAG, "Blocking Analytic: " + uri);
        }

        if ((categories & GeckoSession.TrackingProtectionDelegate.CATEGORY_CONTENT) != 0) {
            Log.i(LOGTAG, "Blocking Content: " + uri);
        }

        if ((categories & GeckoSession.TrackingProtectionDelegate.CATEGORY_SOCIAL) != 0) {
            Log.i(LOGTAG, "Blocking Social: " + uri);
        }
    }

    // PromptDelegate - Gecko Only
    @Override
    public void onAlert(GeckoSession session, String title, String msg, AlertCallback callback) {
        if (session == mCurrentBrowserSession) {
            for (GeckoSession.PromptDelegate listener : mPromptListeners) {
                listener.onAlert(session, title, msg, callback);
            }
        }
    }

    @Override
    public void onButtonPrompt(GeckoSession session, String title, String msg, String[] btnMsg, ButtonCallback callback) {
        if (session == mCurrentBrowserSession) {
            for (GeckoSession.PromptDelegate listener : mPromptListeners) {
                listener.onButtonPrompt(session, title, msg, btnMsg, callback);
            }
        }
    }

    @Override
    public void onTextPrompt(GeckoSession session, String title, String msg, String value, TextCallback callback) {
        if (session == mCurrentBrowserSession) {
            for (GeckoSession.PromptDelegate listener : mPromptListeners) {
                listener.onTextPrompt(session, title, msg, value, callback);
            }
        }
    }

    @Override
    public void onAuthPrompt(GeckoSession session, String title, String msg, AuthOptions options, AuthCallback callback) {
        if (session == mCurrentBrowserSession) {
            for (GeckoSession.PromptDelegate listener : mPromptListeners) {
                listener.onAuthPrompt(session, title, msg, options, callback);
            }
        }
    }

    @Override
    public void onChoicePrompt(GeckoSession session, String title, String msg, int type, Choice[] choices, ChoiceCallback callback) {
        if (session == mCurrentBrowserSession) {
            for (GeckoSession.PromptDelegate listener : mPromptListeners) {
                listener.onChoicePrompt(session, title, msg, type, choices, callback);
            }
        }
    }

    @Override
    public void onColorPrompt(GeckoSession session, String title, String value, TextCallback callback) {
        if (session == mCurrentBrowserSession) {
            for (GeckoSession.PromptDelegate listener : mPromptListeners) {
                listener.onColorPrompt(session, title, value, callback);
            }
        }
    }

    @Override
    public void onDateTimePrompt(GeckoSession session, String title, int type, String value, String min, String max, TextCallback callback) {
        if (session == mCurrentBrowserSession) {
            for (GeckoSession.PromptDelegate listener : mPromptListeners) {
                listener.onDateTimePrompt(session, title, type, value, min, max, callback);
            }
        }
    }

    @Override
    public void onFilePrompt(GeckoSession session, String title, int type, String[] mimeTypes, FileCallback callback) {
        if (session == mCurrentBrowserSession) {
            for (GeckoSession.PromptDelegate listener : mPromptListeners) {
                listener.onFilePrompt(session, title, type, mimeTypes, callback);
            }
        }
    }

}
