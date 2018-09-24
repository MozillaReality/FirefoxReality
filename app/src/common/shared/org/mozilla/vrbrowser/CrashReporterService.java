package org.mozilla.vrbrowser;

import android.app.ActivityManager;
import android.app.IntentService;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.Process;
import android.os.SystemClock;
import android.support.annotation.Nullable;
import android.util.Log;
import org.mozilla.geckoview.GeckoRuntime;

public class CrashReporterService extends IntentService {

    private static final String LOGTAG = "VRB";

    public static final String CRASH_ACTION = "org.mozilla.vrbrowser.CRASH_ACTION";
    public static final String DATA_TAG = "intent";
    private static final int PID_CHECK_INTERVAL = 10;

    private Handler mHandler;

    public CrashReporterService() {
        super("CrashReporterService");
        mHandler = new Handler();
    }

    @Override
    protected void onHandleIntent(@Nullable final Intent intent) {
        boolean fatal = false;
        if (GeckoRuntime.ACTION_CRASHED.equals(intent.getAction())) {
            fatal = intent.getBooleanExtra(GeckoRuntime.EXTRA_CRASH_FATAL, false);
        }

        if (fatal) {
            Log.d(LOGTAG, "======> PARENT CRASH " + intent);
            final int pid = Process.myPid();
            final ActivityManager activityManager = (ActivityManager) this.getSystemService(Context.ACTIVITY_SERVICE);
            if (activityManager == null) {
                return;
            }
            while (true) {
                boolean otherProcessesFound = false;
                for (final ActivityManager.RunningAppProcessInfo info : activityManager.getRunningAppProcesses()) {
                    if (pid != info.pid) {
                        otherProcessesFound = true;
                    }
                    Log.e(LOGTAG, "Found PID " + info.pid);
                }

                if (!otherProcessesFound) {
                    intent.setClass(CrashReporterService.this, VRBrowserActivity.class).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    startActivity(intent);
                    System.exit(0);
                } else {
                    SystemClock.sleep(PID_CHECK_INTERVAL);
                }
            }

        } else {
            Log.d(LOGTAG, "======> CONTENT CRASH " + intent);
            Intent broadcastIntent = new Intent(CRASH_ACTION);
            broadcastIntent.putExtra(DATA_TAG, intent);
            sendBroadcast(broadcastIntent, getString(R.string.app_permission_name));
        }
    }
}
