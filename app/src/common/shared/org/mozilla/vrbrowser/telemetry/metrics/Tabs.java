package org.mozilla.vrbrowser.telemetry.metrics;

import androidx.annotation.NonNull;

import java.util.HashMap;
import java.util.Map;

public class Tabs {

    public enum TabSource {
        CONTEXT_MENU,       // Tab opened from the browsers long click context menu
        TABS_DIALOG,        // Tab opened from the tabs dialog
        BOOKMARKS,          // Tab opened from the bookmarks panel
        HISTORY,            // Tab opened from the history panel
        FXA_LOGIN,          // Tab opened by the FxA login flow
        RECEIVED,           // Tab opened by FxA when a tab is received
        PRE_EXISTING,       // Tab opened as a result of restoring the last session
        BROWSER             // Tab opened by the browser as a result of a new window open
    }

    public static void openedEvent(@NonNull TabSource source) {
        Map<org.mozilla.vrbrowser.GleanMetrics.Tabs.openedKeys, String> map = new HashMap<>();
        map.put(org.mozilla.vrbrowser.GleanMetrics.Tabs.openedKeys.source, source.name());
        org.mozilla.vrbrowser.GleanMetrics.Tabs.INSTANCE.getOpened().record(map);
        org.mozilla.vrbrowser.GleanMetrics.Tabs.INSTANCE.getCounter().add();
    }

    public static void activatedEvent() {
        org.mozilla.vrbrowser.GleanMetrics.Tabs.INSTANCE.getActivated().record();
    }
}
