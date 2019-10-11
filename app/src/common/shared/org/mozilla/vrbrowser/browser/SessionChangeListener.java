package org.mozilla.vrbrowser.browser;

import org.mozilla.geckoview.GeckoSession;

public interface SessionChangeListener {
    default void onNewSession(GeckoSession aSession) {};
    default void onRemoveSession(GeckoSession aSession) {};
    default void onCurrentSessionChange(GeckoSession aOldSession, GeckoSession aSession) {};
}
