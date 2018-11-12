package org.mozilla.vrbrowser.utils;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;

public class ThreadUtils {
    private static Handler sUiHandler;
    private static Handler sBackgroundHandler;
    private static HandlerThread sBackgroundThread;

    private static synchronized Handler getBackgroundHandler() {
        if (sBackgroundHandler == null) {
            sBackgroundThread = new HandlerThread("BackgroundThread");
            sBackgroundThread.start();
            sBackgroundHandler = new Handler(sBackgroundThread.getLooper());
        }

        return sBackgroundHandler;
    }

    private static synchronized Handler getUiHandler() {
        if (sUiHandler == null) {
            sUiHandler = new Handler(Looper.getMainLooper());
        }

        return sUiHandler;
    }

    public static void postDelayedToUiThread(Runnable runnable, long timeout) {
        getUiHandler().postDelayed(runnable, timeout);
    }

    public static void postToUiThread(Runnable runnable) {
        getUiHandler().post(runnable);
    }

    public static void postToBackgroundThread(Runnable runnable) {
        getBackgroundHandler().post(runnable);
    }
}
