/* -*- Mode: Java; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.vrbrowser.utils;

import android.net.Uri;
import android.util.Log;

import java.util.regex.Pattern;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;


// This class refers from mozilla-mobile/focus-android
public class UrlUtils {

    public static String stripCommonSubdomains(@Nullable String host) {
        if (host == null) {
            return null;
        }

        // In contrast to desktop, we also strip mobile subdomains,
        // since its unlikely users are intentionally typing them
        int start = 0;

        if (host.startsWith("www.")) {
            start = 4;
        } else if (host.startsWith("mobile.")) {
            start = 7;
        } else if (host.startsWith("m.")) {
            start = 2;
        }

        return host.substring(start);
    }

    public static boolean isYoutubeVideo(String aUri) {
        try {
            Uri uri = Uri.parse(aUri);
            String hostLower = uri.getHost().toLowerCase();
            String videoParameter = uri.getQueryParameter("v");
            if (videoParameter != null &&
                    (hostLower.contains(".youtube.com") || hostLower.contains(".youtube-nocookie.com"))) {
                return true;

            } else {
                return false;
            }

        } catch (Exception ex) {
            return false;
        }
    }

    public static String getYoutubeVideoId(String aUri) {
        try {
            Uri uri = Uri.parse(aUri);
            return uri.getQueryParameter("v");

        } catch (Exception ex) {
            return "";
        }
    }

    public static boolean isYoutubeRedirect(String aUri, String anotherUri) {
        if (aUri != null && anotherUri != null && !aUri.equals(anotherUri) &&
                (UrlUtils.isYoutubeVideo(aUri) && UrlUtils.isYoutubeVideo(anotherUri)) &&
                (UrlUtils.getYoutubeVideoId(aUri).equals(UrlUtils.getYoutubeVideoId(anotherUri)))) {
            return true;
        }

        return false;
    }

    private static Pattern domainPattern = Pattern.compile("^(http:\\/\\/www\\.|https:\\/\\/www\\.|http:\\/\\/|https:\\/\\/)?[a-z0-9]+([\\-\\.]{1}[a-z0-9]+)*\\.[a-z]{2,5}(:[0-9]{1,5})?(\\/.*)?$");
    public static boolean isDomain(String text) {
        return domainPattern.matcher(text).find();
    }
}
