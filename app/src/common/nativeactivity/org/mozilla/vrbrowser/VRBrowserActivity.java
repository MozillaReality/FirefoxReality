/* -*- Mode: Java; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.vrbrowser;

import android.app.NativeActivity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.graphics.SurfaceTexture;
import android.view.Surface;
import android.view.SurfaceView;

import org.mozilla.gecko.GeckoSession;

public class VRBrowserActivity extends NativeActivity {
    static String LOGTAG = "VRBrowser";
    static final String DEFAULT_URL = "https://mozvr.com";
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private SurfaceView mSurfaceView;
    private static GeckoSession mSession;
    private static Surface mBrowserSurface;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.e(LOGTAG,"in onCreate");
        super.onCreate(savedInstanceState);
        if (mSession == null) {
            mSession = new GeckoSession();
        }
        getWindow().takeSurface(null);
        SurfaceView surfaceView = new SurfaceView(this);
        surfaceView.getHolder().addCallback(this);

        setContentView(surfaceView);
        loadFromIntent(getIntent());
    }

    @Override
    protected void onNewIntent(final Intent intent) {
        Log.e(LOGTAG,"In onNewIntent");
        super.onNewIntent(intent);
        setIntent(intent);
        final String action = intent.getAction();
        if (Intent.ACTION_VIEW.equals(action)) {
            if (intent.getData() != null) {
                loadFromIntent(intent);
            }
        }
    }

    private void loadFromIntent(final Intent intent) {
        final Uri uri = intent.getData();
        Log.e(LOGTAG, "Load URI from intent: " + (uri != null ? uri.toString() : DEFAULT_URL));
        String uriValue = (uri != null ? uri.toString() : DEFAULT_URL);
        mSession.loadUri(uriValue);
    }

    @Override
    protected void onPause() {
        Log.e(LOGTAG, "In onPause");
        super.onPause();
    }

    @Override
    protected void onResume() {
        Log.e(LOGTAG, "in onResume");
        super.onResume();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    private void setSurfaceTexture(String aName, final SurfaceTexture aTexture, final int aWidth, final int aHeight) {
        runOnUiThread(new Runnable() {
            public void run() {
                createBrowser(aTexture, aWidth, aHeight);
            }
        });
    }

    private void createBrowser(SurfaceTexture aTexture, int aWidth, int aHeight) {
        if (aTexture != null) {
            Log.e(LOGTAG,"In createBrowser");
            aTexture.setDefaultBufferSize(aWidth, aHeight);
            mBrowserSurface = new Surface(aTexture);
            mSession.acquireDisplay().surfaceChanged(mBrowserSurface, aWidth, aHeight);
            mSession.openWindow(this);
        }
    }
}
