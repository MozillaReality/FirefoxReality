package org.mozilla.vrbrowser.browser.extensions;

import android.content.Context;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.mozilla.geckoview.AllowOrDeny;
import org.mozilla.geckoview.GeckoResult;
import org.mozilla.geckoview.GeckoSession;
import org.mozilla.geckoview.WebExtensionController;
import org.mozilla.vrbrowser.VRBrowserApplication;
import org.mozilla.vrbrowser.browser.SessionChangeListener;
import org.mozilla.vrbrowser.browser.engine.Session;
import org.mozilla.vrbrowser.browser.engine.SessionStore;
import org.mozilla.vrbrowser.utils.SystemUtils;

import java.util.Collections;
import java.util.List;
import java.util.WeakHashMap;
import java.util.stream.Collectors;

import kotlin.Unit;
import kotlin.jvm.functions.Function0;
import kotlin.jvm.functions.Function1;
import kotlin.jvm.functions.Function2;
import mozilla.components.concept.engine.webextension.ActionHandler;
import mozilla.components.concept.engine.webextension.BrowserAction;
import mozilla.components.concept.engine.webextension.EnableSource;
import mozilla.components.concept.engine.webextension.WebExtension;
import mozilla.components.concept.engine.webextension.WebExtensionDelegate;
import mozilla.components.concept.engine.webextension.WebExtensionEngine;
import mozilla.components.concept.engine.webextension.WebExtensionEngineSession;
import mozilla.components.feature.accounts.FxaCapability;
import mozilla.components.feature.accounts.FxaWebChannelFeature;

import static java.util.Collections.emptyList;

public class WebExtensionsManager implements SessionChangeListener, WebExtensionEngine {

    private static final String LOGTAG = SystemUtils.createLogtag(WebExtensionsManager.class);

    private FxaWebChannelFeature mFxAWebChannelsFeature;
    private SessionStore mSessionStore;
    private WebExtensionDelegate mWebExtensionDelegate;

    public WebExtensionsManager(@NonNull Context context, @NonNull SessionStore store) {
        mSessionStore = store;

        VimeoExtensionFeature.install(this);
        YoutubeExtensionFeature.install(this);

        mFxAWebChannelsFeature = new FxaWebChannelFeature(
                context,
                null,
                this,
                null,
                ((VRBrowserApplication) context.getApplicationContext()).getServices().getAccountManager(),
                Collections.singleton(FxaCapability.CHOOSE_WHAT_TO_SYNC));
        mFxAWebChannelsFeature.start();
    }

    private ActionHandler webExtensionActionHandler = new ActionHandler() {
        @Nullable
        @Override
        public WebExtensionEngineSession onToggleBrowserActionPopup(@NonNull WebExtension webExtension, @NonNull BrowserAction browserAction) {
            if (mWebExtensionDelegate != null) {
                Session session = mSessionStore.createSession(false);
                return mWebExtensionDelegate.onToggleBrowserActionPopup(webExtension, session, browserAction);
            }

            return null;
        }

        @Override
        public void onBrowserAction(@NonNull WebExtension webExtension, @org.jetbrains.annotations.Nullable WebExtensionEngineSession webExtensionEngineSession, @NonNull BrowserAction browserAction) {
            if (mWebExtensionDelegate != null) {
                mWebExtensionDelegate.onBrowserActionDefined(webExtension, browserAction);
            }
        }

    };

    // SessionChangeListener

    @Override
    public void onSessionOpened(Session aSession) {
        mFxAWebChannelsFeature.onSessionAdded(aSession);
    }

    @Override
    public void onSessionClosed(Session aSession) {
        mFxAWebChannelsFeature.onSessionRemoved(aSession);
    }

    // WebExtensionEngine

    @Override
    public void enableWebExtension(@NonNull WebExtension webExtension, @NonNull EnableSource enableSource, @NonNull Function1<? super WebExtension, Unit> onSuccess, @NonNull Function1<? super Throwable, Unit> onError) {
        // TODO
        onSuccess.invoke(null);
    }

    @Override
    public void disableWebExtension(@NonNull WebExtension webExtension, @NonNull EnableSource enableSource, @NonNull Function1<? super WebExtension, Unit> onSuccess, @NonNull Function1<? super Throwable, Unit> onError) {
        // TODO
        onSuccess.invoke(null);
    }

    @Override
    public void installWebExtension(@NonNull String id, @NonNull String url, boolean allowContentMessaging, boolean supportActions, @NonNull Function1<? super WebExtension, Unit> onSuccess, @NonNull Function2<? super String, ? super Throwable, Unit> onError) {
        GeckoWebExtension ext = new GeckoWebExtension(id, url, mSessionStore.getRuntime().getWebExtensionController(), allowContentMessaging);
        if (ext.isBuiltIn()) {
            if (ext.getSupportActions()) {
                // We currently have to install the global action handler before we
                // install the extension which is why this is done here directly.
                // This code can be removed from the engine once the new GV addon
                // management API (specifically installBuiltIn) lands. Then the
                // global handlers will be invoked with the latest state whenever
                // they are registered:
                // https://bugzilla.mozilla.org/show_bug.cgi?id=1599897
                // https://bugzilla.mozilla.org/show_bug.cgi?id=1582185
                ext.registerActionHandler(webExtensionActionHandler);
            }

            // For now we have to use registerWebExtension for builtin extensions until we get the
            // new installBuiltIn call on the controller: https://bugzilla.mozilla.org/show_bug.cgi?id=1601067
            mSessionStore.getRuntime().registerWebExtension(ext.getNativeExtension()).then(aVoid -> {
                if (mWebExtensionDelegate != null) {
                    mWebExtensionDelegate.onInstalled(ext);
                }
                onSuccess.invoke(ext);
                return GeckoResult.ALLOW;

            }, throwable -> {
                onError.invoke(ext.getId(), throwable);
                return GeckoResult.DENY;
            });
        } else {
            mSessionStore.getRuntime().getWebExtensionController().install(ext.getUrl()).then(aExtension -> {
                GeckoWebExtension installedExtension = new GeckoWebExtension(aExtension.id, aExtension.location, mSessionStore.getRuntime().getWebExtensionController(), true);
                if (mWebExtensionDelegate != null) {
                    mWebExtensionDelegate.onInstalled(installedExtension);
                }
                installedExtension.registerActionHandler(webExtensionActionHandler);
                onSuccess.invoke(installedExtension);
                return GeckoResult.ALLOW;

            }, throwable -> {
                onError.invoke(ext.getId(), throwable);
                return GeckoResult.DENY;
            });
        }
    }

    @Override
    public void uninstallWebExtension(@NonNull WebExtension webExtension, @NonNull Function0<Unit> onSuccess, @NonNull Function2<? super String, ? super Throwable, Unit> onError) {
        mSessionStore.getRuntime().getWebExtensionController().uninstall(((GeckoWebExtension)webExtension).getNativeExtension()).then(aVoid -> {
            if (mWebExtensionDelegate != null) {
                mWebExtensionDelegate.onUninstalled(webExtension);
            }

            onSuccess.invoke();
            return GeckoResult.ALLOW;

        }, throwable -> {
            onError.invoke(webExtension.getId(), throwable);
            return GeckoResult.DENY;
        });
    }

    @Override
    public void updateWebExtension(@NonNull WebExtension webExtension, @NonNull Function1<? super WebExtension, Unit> onSuccess, @NonNull Function2<? super String, ? super Throwable, Unit> onError) {
        // GeckoView support for updating extensions hasn't been implemented yet
        // TODO https://bugzilla.mozilla.org/show_bug.cgi?id=1599581
        onSuccess.invoke(null);
    }

    @Override
    public void listInstalledWebExtensions(@NonNull Function1<? super List<? extends WebExtension>, Unit> onSuccess, @NonNull Function1<? super Throwable, Unit> onError) {
        mSessionStore.getRuntime().getWebExtensionController().list().then(aExtensions -> {
            List<WebExtension> extensions = emptyList();
            if (aExtensions != null) {
                extensions = aExtensions.stream().map(extension -> new GeckoWebExtension(extension.id, extension.location, mSessionStore.getRuntime().getWebExtensionController(), true)).collect(Collectors.toList());
            }

            extensions.forEach(extension -> {
                extension.registerActionHandler(webExtensionActionHandler);
            });
            onSuccess.invoke(extensions);
            return GeckoResult.ALLOW;

        }, throwable -> {
            onError.invoke(throwable);
            return GeckoResult.DENY;
        });
    }

    @Override
    public void registerWebExtensionDelegate(@NonNull WebExtensionDelegate webExtensionDelegate) {
        mSessionStore.getRuntime().getWebExtensionController().setTabDelegate(new WebExtensionController.TabDelegate() {
            // We use this map to find the engine session of a given gecko
            // session, as we currently have no other way of accessing the
            // list of engine sessions. This will change once the engine
            // gets access to the browser store:
            // https://github.com/mozilla-mobile/android-components/issues/4965
            private WeakHashMap<Session, String> tabs = new WeakHashMap<>();

            @NonNull
            @Override
            public GeckoResult<GeckoSession> onNewTab(@Nullable org.mozilla.geckoview.WebExtension webExtension, @Nullable String url) {
                Session session = mSessionStore.createSession(mSessionStore.getActiveSession().isPrivateMode());
                GeckoWebExtension extension = null;
                if (webExtension != null) {
                    extension = new GeckoWebExtension(webExtension.id, webExtension.location, mSessionStore.getRuntime().getWebExtensionController(), true);
                }

                webExtensionDelegate.onNewTab(extension, url != null ? url : "", session);
                if (webExtension != null) {
                    tabs.put(session, webExtension.id);
                }

                return GeckoResult.fromValue(session.getGeckoSession());
            }

            @NonNull
            @Override
            public GeckoResult<AllowOrDeny> onCloseTab(@Nullable org.mozilla.geckoview.WebExtension webExtension, @NonNull GeckoSession geckoSession) {
                Session webExtensionGeckoEngineSession = null;
                for (Session engineSession : tabs.keySet()) {
                    if (engineSession.getGeckoSession() == geckoSession) {
                        webExtensionGeckoEngineSession = engineSession;
                    }
                }

                if (webExtensionGeckoEngineSession == null) {
                    return GeckoResult.DENY;
                }

                if (webExtension != null && tabs.get(webExtensionGeckoEngineSession).equals(webExtension.id)) {
                    GeckoWebExtension geckoWebExtension = new GeckoWebExtension(webExtension.id, webExtension.location, mSessionStore.getRuntime().getWebExtensionController(), true);
                    if (webExtensionDelegate.onCloseTab(geckoWebExtension, webExtensionGeckoEngineSession)) {
                        return GeckoResult.ALLOW;

                    } else {
                        return GeckoResult.DENY;
                    }

                } else {
                    return GeckoResult.DENY;
                }
            }
        });
    }
}
