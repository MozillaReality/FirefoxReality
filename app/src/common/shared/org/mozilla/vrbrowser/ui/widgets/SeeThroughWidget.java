package org.mozilla.vrbrowser.ui.widgets;

import android.content.Context;
import android.content.res.Configuration;
import android.hardware.usb.UsbDevice;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.TextureView;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import com.lgh.uvccamera.UVCCameraProxy;
import com.lgh.uvccamera.bean.PicturePath;
import com.lgh.uvccamera.callback.ConnectCallback;
import com.lgh.uvccamera.callback.PhotographCallback;
import com.lgh.uvccamera.callback.PictureCallback;
import com.lgh.uvccamera.callback.PreviewCallback;
import com.serenegiant.usb.Size;

import org.mozilla.geckoview.GeckoSession;
import org.mozilla.vrbrowser.R;
import org.mozilla.vrbrowser.browser.SettingsStore;
import org.mozilla.vrbrowser.ui.widgets.dialogs.PromptDialogWidget;

import java.util.ArrayList;
import java.util.List;

public class SeeThroughWidget extends UIWidget implements GeckoSession.NavigationDelegate,
        GeckoSession.ContentDelegate {

    private static final String TAG = "SeeThroughWidget";
    private TextureView mTextureView;
    private UVCCameraProxy mUVCCamera;
    private Spinner mSpinner;
    private boolean isFirst = true;

    public SeeThroughWidget(Context aContext) {
        super(aContext);
        initialize(aContext);
    }

    private void initialize(Context aContext) {
        inflate(aContext, R.layout.see_through, this);

        initView();
        initUVCCamera();
    }

    private void initView() {
        mTextureView = findViewById(R.id.textureView);
//        mSurfaceView = findViewById(R.id.surfaceView);
//        mSurfaceView.setZOrderOnTop(true);
//        mImageView1 = findViewById(R.id.imag1);
        mSpinner = findViewById(R.id.spinner);
        mSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, final int position, long id) {
                if (isFirst) {
                    isFirst = false;
                    return;
                }
                Log.i(TAG, "position-->" + position);
                mUVCCamera.stopPreview();
                List<Size> list = mUVCCamera.getSupportedPreviewSizes();
                if (!list.isEmpty()) {
                    mUVCCamera.setPreviewSize(list.get(position).width, list.get(position).height);
                    mUVCCamera.startPreview();
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });
    }

    private void initUVCCamera() {
        mUVCCamera = new UVCCameraProxy(getContext());
        // 已有默认配置，不需要可以不设置
        mUVCCamera.getConfig()
                .isDebug(true)
                .setPicturePath(PicturePath.APPCACHE)
                .setDirName("uvccamera")
                .setProductId(0)
                .setVendorId(0);
        mUVCCamera.setPreviewTexture(mTextureView);
//        mUVCCamera.setPreviewSurface(mSurfaceView);
//        mUVCCamera.registerReceiver();

        mUVCCamera.setConnectCallback(new ConnectCallback() {
            @Override
            public void onAttached(UsbDevice usbDevice) {
                mUVCCamera.requestPermission(usbDevice);
            }

            @Override
            public void onGranted(UsbDevice usbDevice, boolean granted) {
                if (granted) {
                    mUVCCamera.connectDevice(usbDevice);
                }
            }

            @Override
            public void onConnected(UsbDevice usbDevice) {
                mUVCCamera.openCamera();
            }

            @Override
            public void onCameraOpened() {
                showAllPreviewSizes();
                mUVCCamera.setPreviewSize(640, 480);
                mUVCCamera.startPreview();
            }

            @Override
            public void onDetached(UsbDevice usbDevice) {
                mUVCCamera.closeCamera();
            }
        });

        mUVCCamera.setPhotographCallback(new PhotographCallback() {
            @Override
            public void onPhotographClick() {
                mUVCCamera.takePicture();
//                mUVCCamera.takePicture("test.jpg");
            }
        });

        mUVCCamera.setPreviewCallback(new PreviewCallback() {
            @Override
            public void onPreviewFrame(byte[] yuv) {

            }
        });

        mUVCCamera.setPictureTakenCallback(new PictureCallback() {
            @Override
            public void onPictureTaken(String path) {
//                path1 = path;
//                mImageView1.setImageURI(null);
//                mImageView1.setImageURI(Uri.parse(path));
            }
        });
    }

    private void showAllPreviewSizes() {
        isFirst = true;
        List<Size> previewList = mUVCCamera.getSupportedPreviewSizes();
        List<String> previewStrs = new ArrayList<>();
        for (Size size : previewList) {
            previewStrs.add(size.width + " * " + size.height);
        }
        ArrayAdapter adapter = new ArrayAdapter(getContext(), android.R.layout.simple_spinner_item, previewStrs);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpinner.setAdapter(adapter);
    }

    public void updateUI() {
        removeAllViews();
    }



    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);

        updateUI();
    }

    @Override
    public void releaseWidget() {
        super.releaseWidget();
    }

    @Override
    protected void initializeWidgetPlacement(WidgetPlacement aPlacement) {
        aPlacement.visible = false;
        aPlacement.width =  WidgetPlacement.dpDimension(getContext(), R.dimen.prompt_dialog_width);
        aPlacement.height = WidgetPlacement.dpDimension(getContext(), R.dimen.prompt_dialog_height);
        aPlacement.anchorX = 0.5f;
        aPlacement.anchorY = 0.5f;
        aPlacement.parentAnchorX = 0.5f;
        aPlacement.parentAnchorY = 0.0f;
        aPlacement.translationY = WidgetPlacement.unitFromMeters(getContext(), R.dimen.settings_world_y) -
                WidgetPlacement.unitFromMeters(getContext(), R.dimen.window_world_y);
        aPlacement.translationZ = WidgetPlacement.unitFromMeters(getContext(), R.dimen.settings_world_z) -
                WidgetPlacement.unitFromMeters(getContext(), R.dimen.window_world_z);
    }

    public void setPlacementForKeyboard(int aHandle) {
        mWidgetPlacement.parentHandle = aHandle;
        mWidgetPlacement.translationY = 0;
        mWidgetPlacement.translationZ = 0;
    }

    @Override
    public void show(@ShowFlags int aShowFlags) {
        if (SettingsStore.getInstance(getContext()).isSpeechDataCollectionEnabled() ||
                SettingsStore.getInstance(getContext()).isSpeechDataCollectionReviewed()) {
            mWidgetPlacement.parentHandle = mWidgetManager.getFocusedWindow().getHandle();
            super.show(aShowFlags);

        } else {
            mWidgetManager.getFocusedWindow().showDialog(
                    getResources().getString(R.string.voice_samples_collect_data_dialog_title, getResources().getString(R.string.app_name)),
                    R.string.voice_samples_collect_dialog_description2,
                    new int[]{
                            R.string.voice_samples_collect_dialog_do_not_allow,
                            R.string.voice_samples_collect_dialog_allow},
                    (index, isChecked) -> {
                        SettingsStore.getInstance(getContext()).setSpeechDataCollectionReviewed(true);
                        if (index == PromptDialogWidget.POSITIVE) {
                            SettingsStore.getInstance(getContext()).setSpeechDataCollectionEnabled(true);
                        }
                        new Handler(Looper.getMainLooper()).post(() -> show(aShowFlags));
                    },
                    () -> mWidgetManager.openNewTabForeground(getResources().getString(R.string.private_policy_url)));
        }
    }

    @Override
    public void hide(@HideFlags int aHideFlags) {
        super.hide(aHideFlags);
    }
}
