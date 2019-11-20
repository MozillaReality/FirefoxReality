package org.mozilla.vrbrowser.telemetry.metrics;

import androidx.annotation.NonNull;

import org.mozilla.vrbrowser.GleanMetrics.Fxa;

import java.util.HashMap;
import java.util.Map;

public class FxA {

    public static void signIn() {
        Fxa.INSTANCE.getSignIn().add();
    }

    public static void signSuccess(boolean result) {
        Fxa.INSTANCE.getSignInResult().set(result);
    }

    public static void signOut() {
        Fxa.INSTANCE.getSignOut().add();
    }

    public static void bookmarksSyncStatus(boolean status) {
        Fxa.INSTANCE.getBookmarksSyncStatus().set(status);
    }

    public static void historySyncStatus(boolean status) {
        Fxa.INSTANCE.getHistorySyncStatus().set(status);
    }

    public static void sentTab(boolean result) {
        Fxa.INSTANCE.getSentTab().set(result);
    }

    public static void receivedTab(@NonNull mozilla.components.concept.sync.DeviceType source) {
        Map<Fxa.receivedTabKeys, String> map = new HashMap<>();
        map.put(Fxa.receivedTabKeys.source, source.name());
        Fxa.INSTANCE.getReceivedTab().record();
    }
}
