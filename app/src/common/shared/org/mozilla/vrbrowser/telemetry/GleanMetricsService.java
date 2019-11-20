package org.mozilla.vrbrowser.telemetry;

import android.content.Context;

import androidx.annotation.NonNull;

import org.mozilla.vrbrowser.BuildConfig;
import org.mozilla.vrbrowser.GleanMetrics.Distribution;
import org.mozilla.vrbrowser.GleanMetrics.Tabs;
import org.mozilla.vrbrowser.browser.SettingsStore;
import org.mozilla.vrbrowser.utils.DeviceType;
import org.mozilla.vrbrowser.utils.SystemUtils;

import java.util.HashMap;
import java.util.Map;

import mozilla.components.service.glean.Glean;
import mozilla.components.service.glean.config.Configuration;


public class GleanMetricsService {

    private final static String APP_NAME = "FirefoxReality";
    private static boolean initialized = false;
    private final static String LOGTAG = SystemUtils.createLogtag(GleanMetricsService.class);
    private static Context context = null;

    // We should call this at the application initial stage.
    public static void init(Context aContext) {
        if (initialized)
            return;

        context = aContext;
        initialized = true;

        final boolean telemetryEnabled = SettingsStore.getInstance(aContext).isTelemetryEnabled();
        if (telemetryEnabled) {
            GleanMetricsService.start();
        } else {
            GleanMetricsService.stop();
        }
        Configuration config = new Configuration(Configuration.DEFAULT_TELEMETRY_ENDPOINT, BuildConfig.BUILD_TYPE);
        Glean.INSTANCE.initialize(aContext, config);
    }

    // It would be called when users turn on/off the setting of telemetry.
    // e.g., SettingsStore.getInstance(context).setTelemetryEnabled();
    public static void start() {
        Glean.INSTANCE.setUploadEnabled(true);
        setStartupMetrics();
    }

    // It would be called when users turn on/off the setting of telemetry.
    // e.g., SettingsStore.getInstance(context).setTelemetryEnabled();
    public static void stop() {
        Glean.INSTANCE.setUploadEnabled(false);
    }

    private static void setStartupMetrics() {
        Distribution.INSTANCE.getChannelName().set(DeviceType.isOculusBuild() ? "oculusvr" : BuildConfig.FLAVOR_platform);
    }

    // Metrics

    public enum TabOpenedSource {
        CONTEXT_MENU,       // Tab opened from the browsers long click context menu
        TABS_DIALOG,        // Tab opened from the tabs dialog
        BOOKMARKS,          // Tab opened from the bookmarks panel
        HISTORY,            // Tab opened from the history panel
        FXA_LOGIN,          // Tab opened by the FxA login flow
        RECEIVED,           // Tab opened by FxA when a tab is received
        PRE_EXISTING,       // Tab opened as a result of restoring the last session
        BROWSER             // Tab opened by the browser as a result of a new window open
    }

    public static void tabOpenedEvent(@NonNull TabOpenedSource source) {
        Map<Tabs.openedKeys, String> map = new HashMap<>();
        map.put(Tabs.openedKeys.source, source.name());
        Tabs.INSTANCE.getOpened().record(map);
        Tabs.INSTANCE.getCounter().add();
    }

    public static void tabActivatedEvent() {
        Tabs.INSTANCE.getActivated().record();
    }

}
