package com.lgh.uvccamera.usb;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;

import com.lgh.uvccamera.callback.ConnectCallback;
import com.lgh.uvccamera.config.CameraConfig;
import com.lgh.uvccamera.utils.LogUtil;

import java.util.HashMap;

/**
 * 描述：usb插拔监听、连接工具类
 * 作者：liugh
 * 日期：2018/9/17
 * 版本：v2.0.0
 */
public class UsbMonitor implements IMonitor {
    private static final String ACTION_USB_DEVICE_PERMISSION = "ACTION_USB_DEVICE_PERMISSION";
    private Context mContext;
    private UsbManager mUsbManager;
    private USBReceiver mUsbReceiver;
    private UsbController mUsbController;
    private ConnectCallback mConnectCallback;
    private CameraConfig mConfig;

    public UsbMonitor(Context context, CameraConfig config) {
        this.mContext = context;
        this.mConfig = config;
        this.mUsbManager = (UsbManager) context.getSystemService(context.USB_SERVICE);
    }

    /**
     * 注册usb插拔监听广播
     */
    @Override
    public void registerReceiver() {
        LogUtil.i("registerReceiver");
        if (mUsbReceiver == null) {
            IntentFilter filter = new IntentFilter();
            filter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
            filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
            filter.addAction(ACTION_USB_DEVICE_PERMISSION);
            mUsbReceiver = new USBReceiver();
            mContext.registerReceiver(mUsbReceiver, filter);
        }
    }

    /**
     * 注销usb插拔监听广播
     */
    @Override
    public void unregisterReceiver() {
        LogUtil.i("unregisterReceiver");
        if (mUsbReceiver != null) {
            mContext.unregisterReceiver(mUsbReceiver);
            mUsbReceiver = null;
        }
    }

    @Override
    public void checkDevice() {
        LogUtil.i("checkDevice");
        UsbDevice usbDevice = getUsbCameraDevice();
        if (isTargetDevice(usbDevice) && mConnectCallback != null) {
            mConnectCallback.onAttached(usbDevice);
        }
    }

    @Override
    public void requestPermission(UsbDevice usbDevice) {
        LogUtil.i("requestPermission-->" + usbDevice);
        if (mUsbManager.hasPermission(usbDevice)) {
            if (mConnectCallback != null) {
                mConnectCallback.onGranted(usbDevice, true);
            }
        } else {
            PendingIntent pendingIntent = PendingIntent.getBroadcast(mContext, 0, new Intent(ACTION_USB_DEVICE_PERMISSION), 0);
            mUsbManager.requestPermission(usbDevice, pendingIntent);
        }
    }

    @Override
    public void connectDevice(UsbDevice usbDevice) {
        LogUtil.i("connectDevice-->" + usbDevice);
        mUsbController = new UsbController(mUsbManager, usbDevice);
        if (mUsbController.open() != null && mConnectCallback != null) {
            mConnectCallback.onConnected(usbDevice);
        }
    }

    @Override
    public void closeDevice() {
        LogUtil.i("closeDevice");
        if (mUsbController != null) {
            mUsbController.close();
            mUsbController = null;
        }
    }

    @Override
    public UsbController getUsbController() {
        return mUsbController;
    }

    @Override
    public UsbDeviceConnection getConnection() {
        if (mUsbController != null) {
            return mUsbController.getConnection();
        }
        return null;
    }

    public void setConnectCallback(ConnectCallback callback) {
        this.mConnectCallback = callback;
    }

    /**
     * 是否存在usb摄像头
     *
     * @return
     */
    public boolean hasUsbCamera() {
        return getUsbCameraDevice() != null;
    }

    /**
     * 获取usb摄像头设备
     *
     * @return
     */
    public UsbDevice getUsbCameraDevice() {
        HashMap<String, UsbDevice> deviceMap = mUsbManager.getDeviceList();
        if (deviceMap != null) {
            for (UsbDevice usbDevice : deviceMap.values()) {
                if (isUsbCamera(usbDevice)) {
                    return usbDevice;
                }
            }
        }
        return null;
    }

    /**
     * 判断某usb设备是否摄像头，usb摄像头的大小类是239-2
     *
     * @param usbDevice
     * @return
     */
    public boolean isUsbCamera(UsbDevice usbDevice) {
        return usbDevice != null && 239 == usbDevice.getDeviceClass() && 2 == usbDevice.getDeviceSubclass();
    }

    /**
     * 是否目标设备，是相机并且产品id和供应商id跟配置的一致
     *
     * @param usbDevice
     * @return
     */
    public boolean isTargetDevice(UsbDevice usbDevice) {
        if (!isUsbCamera(usbDevice)
                || mConfig == null
                || (mConfig.getProductId() != 0 && mConfig.getProductId() != usbDevice.getProductId())
                || (mConfig.getVendorId() != 0 && mConfig.getVendorId() != usbDevice.getVendorId())) {
            LogUtil.i("No target camera device");
            return false;
        }
        LogUtil.i("Find target camera device");
        return true;
    }

    /**
     * usb插拔广播监听类
     */
    private class USBReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            UsbDevice usbDevice = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
            LogUtil.i("usbDevice-->" + usbDevice);
            if (!isTargetDevice(usbDevice) || mConnectCallback == null) {
                return;
            }

            switch (intent.getAction()) {
                case UsbManager.ACTION_USB_DEVICE_ATTACHED:
                    LogUtil.i("onAttached");
                    mConnectCallback.onAttached(usbDevice);
                    break;

                case ACTION_USB_DEVICE_PERMISSION:
                    boolean granted = intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false);
                    mConnectCallback.onGranted(usbDevice, granted);
                    LogUtil.i("onGranted-->" + granted);
                    break;

                case UsbManager.ACTION_USB_DEVICE_DETACHED:
                    LogUtil.i("onDetached");
                    mConnectCallback.onDetached(usbDevice);
                    break;

                default:
                    break;
            }
        }
    }

}
