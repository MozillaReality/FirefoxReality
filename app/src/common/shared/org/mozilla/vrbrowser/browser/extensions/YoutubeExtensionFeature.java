package org.mozilla.vrbrowser.browser.extensions;

import android.util.Log;

import androidx.annotation.NonNull;

import org.mozilla.vrbrowser.utils.SystemUtils;

import mozilla.components.concept.engine.webextension.WebExtensionEngine;

public class YoutubeExtensionFeature {

    private static final String LOGTAG = SystemUtils.createLogtag(YoutubeExtensionFeature.class);

    private static final String EXTENSION_ID = "mozacYoutube";
    private static final String EXTENSION_URL = "resource://android/assets/web_extensions/webcompat_youtube/";

    public static void install(@NonNull WebExtensionEngine engine) {

        engine.installWebExtension(EXTENSION_ID, EXTENSION_URL, false, false, (webExtension) -> {
            Log.i(LOGTAG, "Youtube Web Extension successfully installed");
            return null;
        }, (s, throwable) -> {
            Log.e(LOGTAG, "Error installing the Youtube Web Extension: " + throwable.getLocalizedMessage());
            return null;
        });
    }
}
