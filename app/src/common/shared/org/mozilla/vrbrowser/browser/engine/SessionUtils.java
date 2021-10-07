package org.mozilla.vrbrowser.browser.engine;

import androidx.annotation.Nullable;
import org.mozilla.vrbrowser.utils.SystemUtils;

class SessionUtils {
    private static final String LOGTAG = SystemUtils.createLogtag(SessionUtils.class);

    public static boolean isLocalizedContent(@Nullable String url) {
        return url != null && (url.startsWith("about:") || url.startsWith("data:"));
    }

}
