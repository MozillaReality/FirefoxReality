/* -*- Mode: Java; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.vrbrowser.ui.widgets;

import android.content.Context;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import org.mozilla.geckoview.MediaElement;
import org.mozilla.vrbrowser.R;
import org.mozilla.vrbrowser.browser.Media;
import org.mozilla.vrbrowser.browser.SettingsStore;
import org.mozilla.vrbrowser.ui.views.MediaSeekBar;
import org.mozilla.vrbrowser.ui.views.UIButton;
import org.mozilla.vrbrowser.ui.views.VolumeControl;

public class CurvatureWidget extends UIWidget {

    private static final String LOGTAG = "VRB";
    private SeekBar mSeekBar;

    public CurvatureWidget(Context aContext) {
        super(aContext);
        initialize(aContext);
    }

    public CurvatureWidget(Context aContext, AttributeSet aAttrs) {
        super(aContext, aAttrs);
        initialize(aContext);
    }

    public CurvatureWidget(Context aContext, AttributeSet aAttrs, int aDefStyle) {
        super(aContext, aAttrs, aDefStyle);
        initialize(aContext);
    }

    private void initialize(Context aContext) {
        inflate(aContext, R.layout.curvature_controls, this);

        mSeekBar = findViewById(R.id.curvatureSeekBar);
        mSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                postDelayed(CurvatureWidget.this::notifyCurvature, 100);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                postDelayed(CurvatureWidget.this::notifyCurvature, 100);
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                postDelayed(CurvatureWidget.this::notifyCurvature, 100);
            }
        });
        float ratio = SettingsStore.getInstance(getContext()).getCurvatureRatio();
        mSeekBar.setProgress((int)(ratio * mSeekBar.getMax()));
        mWidgetManager.setCurvatureRatio(ratio);
        mSeekBar.setSecondaryProgress((int)(mSeekBar.getMax() * 4680.0f / 8000.0f));
    }

    void notifyCurvature() {
        float ratio = (float)mSeekBar.getProgress() / (float) mSeekBar.getMax();
        mWidgetManager.setCurvatureRatio(ratio);
        SettingsStore.getInstance(getContext()).setCurvatureRatio(ratio);
    }

    @Override
    protected void initializeWidgetPlacement(WidgetPlacement aPlacement) {
        Context context = getContext();
        aPlacement.width = WidgetPlacement.dpDimension(context, R.dimen.tray_width) * 2;
        aPlacement.height = 22;
        aPlacement.anchorX = 0.5f;
        aPlacement.anchorY = 1.0f;
        aPlacement.parentAnchorX = 0.5f;
        aPlacement.parentAnchorY = 0.0f;
        aPlacement.cylinder = false;
    }

    public void setParentWidget(int aHandle) {
        mWidgetPlacement.parentHandle = aHandle;
    }

    @Override
    public void releaseWidget() {
        super.releaseWidget();
    }
}
