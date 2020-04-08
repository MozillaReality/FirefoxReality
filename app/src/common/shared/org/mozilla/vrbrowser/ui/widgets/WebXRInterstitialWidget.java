package org.mozilla.vrbrowser.ui.widgets;

import android.content.Context;
import android.graphics.drawable.AnimatedVectorDrawable;
import android.view.LayoutInflater;
import android.view.animation.Animation;
import android.view.animation.RotateAnimation;

import androidx.databinding.DataBindingUtil;

import org.mozilla.vrbrowser.R;
import org.mozilla.vrbrowser.VRBrowserActivity;
import org.mozilla.vrbrowser.databinding.WebxrInterstitialBinding;
import org.mozilla.vrbrowser.utils.DeviceType;

import java.util.ArrayList;

public class WebXRInterstitialWidget extends UIWidget implements WidgetManagerDelegate.WebXRListener {
    private static final int INTERSTITIAL_FORCE_TIME = 3000;
    private WebxrInterstitialBinding mBinding;
    private ArrayList<WebXRInterstitialController> mControllers = new ArrayList<>();
    private boolean firstEnterXR = true;
    private AnimatedVectorDrawable mSpinnerAnimation;

    public WebXRInterstitialWidget(Context aContext) {
        super(aContext);
        initialize();
    }

    @Override
    protected void initializeWidgetPlacement(WidgetPlacement aPlacement) {
        Context context = getContext();
        aPlacement.scene = WidgetPlacement.SCENE_WEBXR_INTERSTITIAL;
        aPlacement.width = WidgetPlacement.dpDimension(context, R.dimen.webxr_interstitial_width);
        aPlacement.height = WidgetPlacement.dpDimension(context, R.dimen.webxr_interstitial_height);
        aPlacement.worldWidth = WidgetPlacement.floatDimension(getContext(), R.dimen.window_world_width) * aPlacement.width / getWorldWidth();
        aPlacement.translationY = WidgetPlacement.unitFromMeters(context, R.dimen.webxr_interstitial_world_y);
        aPlacement.translationZ = WidgetPlacement.unitFromMeters(getContext(), R.dimen.webxr_interstitial_world_z);
        aPlacement.anchorX = 0.5f;
        aPlacement.anchorY = 0.5f;
        aPlacement.cylinder = false;
        aPlacement.visible = false;
    }

    private void initialize() {
        // AnimatedVectorDrawable doesn't work with a Hardware Accelerated canvas, we disable it for this view.
        setIsHardwareAccelerationEnabled(false);
        LayoutInflater inflater = LayoutInflater.from(getContext());
        mBinding = DataBindingUtil.inflate(inflater, R.layout.webxr_interstitial, this, true);
        mBinding.setLifecycleOwner((VRBrowserActivity)getContext());
        mSpinnerAnimation = (AnimatedVectorDrawable) mBinding.webxrSpinner.getDrawable();
        mWidgetManager.addWebXRListener(this);
    }

    @Override
    public void releaseWidget() {
        mWidgetManager.removeWebXRListener(this);
        super.releaseWidget();
    }

    private void addController(@DeviceType.Type int aDevice, @WebXRInterstitialController.Hand int aHand) {
        mControllers.add(new WebXRInterstitialController(getContext(), aDevice, aHand));
    }

    private void initializeControllers() {
        int deviceType = DeviceType.getType();
        if (deviceType == DeviceType.OculusGo) {
            addController(DeviceType.OculusGo, WebXRInterstitialController.HAND_NONE);
        } else if (deviceType == DeviceType.OculusQuest) {
            addController(DeviceType.OculusQuest, WebXRInterstitialController.HAND_LEFT);
            addController(DeviceType.OculusQuest, WebXRInterstitialController.HAND_RIGHT);
        } else if (deviceType == DeviceType.PicoNeo2) {
            addController(DeviceType.PicoNeo2, WebXRInterstitialController.HAND_LEFT);
            addController(DeviceType.PicoNeo2, WebXRInterstitialController.HAND_RIGHT);
        } else if (deviceType == DeviceType.PicoG2) {
            addController(DeviceType.PicoG2, WebXRInterstitialController.HAND_NONE);
        } else if (deviceType == DeviceType.ViveFocusPlus) {
            addController(DeviceType.ViveFocusPlus, WebXRInterstitialController.HAND_LEFT);
            addController(DeviceType.ViveFocusPlus, WebXRInterstitialController.HAND_RIGHT);
        }
        for (UIWidget controller: mControllers) {
            controller.getPlacement().parentHandle = getHandle();
            mWidgetManager.addWidget(controller);
        }
    }

    private void showControllers() {
        if (mControllers.size() == 0) {
            initializeControllers();
        }

        for (UIWidget widget: mControllers) {
            widget.show(KEEP_FOCUS);
        }
    }

    private void hideControllers() {
        for (UIWidget widget: mControllers) {
            widget.hide(KEEP_WIDGET);
        }
    }

    private void startAnimation() {
        mSpinnerAnimation.start();
    }

    private void stopAnimation() {
        mSpinnerAnimation.stop();
    }

    @Override
    public void onEnterWebXR() {
        startAnimation();
        show(KEEP_FOCUS);
        if (firstEnterXR) {
            firstEnterXR = false;
            showControllers();
            postDelayed(() -> {
                if (mWidgetManager != null) {
                    mWidgetManager.setWebXRIntersitialForced(false);
                }
            }, INTERSTITIAL_FORCE_TIME);
        }
    }


    @Override
    public void onExitWebXR() {
        stopAnimation();
        hideControllers();
        hide(KEEP_WIDGET);
    }
}
