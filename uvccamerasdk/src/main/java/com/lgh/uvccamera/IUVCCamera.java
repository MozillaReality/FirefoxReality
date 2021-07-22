package com.lgh.uvccamera;

import android.hardware.usb.UsbDevice;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.TextureView;

import com.lgh.uvccamera.callback.ConnectCallback;
import com.lgh.uvccamera.callback.PhotographCallback;
import com.lgh.uvccamera.callback.PictureCallback;
import com.lgh.uvccamera.callback.PreviewCallback;
import com.lgh.uvccamera.config.CameraConfig;
import com.serenegiant.usb.Size;
import com.serenegiant.usb.UVCCamera;

import java.util.List;

/**
 * 描述：uvc相机接口
 * 作者：liugh
 * 日期：2018/12/27
 * 版本：v2.0.0
 */
public interface IUVCCamera {
    /**
     * 注册usb插拔监听广播
     */
    void registerReceiver();

    /**
     * 注销usb插拔监听广播
     */
    void unregisterReceiver();

    /**
     * 检查是否插入了usb摄像头，用于先插入设备再打开页面的场景
     */
    void checkDevice();

    /**
     * 申请打开usb设备权限
     *
     * @param usbDevice
     */
    void requestPermission(UsbDevice usbDevice);

    /**
     * 连接usb设备
     *
     * @param usbDevice
     */
    void connectDevice(UsbDevice usbDevice);

    /**
     * 关闭usb设备
     */
    void closeDevice();

    /**
     * 打开相机
     */
    void openCamera();

    /**
     * 关闭相机
     */
    void closeCamera();

    /**
     * 设置相机预览控件，这里封装了相关注册注销广播、检测设备、释放资源等操作
     *
     * @param surfaceView
     */
    void setPreviewSurface(SurfaceView surfaceView);

    /**
     * 设置相机预览控件，这里封装了相关注册注销广播、检测设备、释放资源等操作
     *
     * @param surfaceView
     */
    void setPreviewTexture(TextureView textureView);

    /**
     * 设置相机预览旋转角度，有些摄像头上下反了
     *
     * @param rotation
     */
    void setPreviewRotation(float rotation);

    /**
     * 设置相机预览Surface
     *
     * @param surface
     */
    void setPreviewDisplay(Surface surface);

    /**
     * 设置预览尺寸
     *
     * @param width
     * @param height
     */
    void setPreviewSize(int width, int height);

    /**
     * 获取相机预览尺寸
     *
     * @return
     */
    Size getPreviewSize();

    /**
     * 获取相机支持的预览尺寸
     *
     * @return
     */
    List<Size> getSupportedPreviewSizes();

    /**
     * 开始预览
     */
    void startPreview();

    /**
     * 停止预览
     */
    void stopPreview();

    /**
     * 拍照
     */
    void takePicture();

    /**
     * 拍照
     *
     * @param pictureName 图片名称
     */
    void takePicture(String pictureName);

    /**
     * 设置usb设备连接回调
     *
     * @param callback
     */
    void setConnectCallback(ConnectCallback callback);

    /**
     * 设置预览回调
     *
     * @param callback
     */
    void setPreviewCallback(PreviewCallback callback);

    /**
     * 设置拍照按钮点击回调
     *
     * @param callback
     */
    void setPhotographCallback(PhotographCallback callback);

    /**
     * 设置拍照回调
     *
     * @param callback
     */
    void setPictureTakenCallback(PictureCallback callback);

    /**
     * uvc相机实例
     *
     * @return
     */
    UVCCamera getUVCCamera();

    /**
     * 是否已经打开相机
     *
     * @return
     */
    boolean isCameraOpen();

    /**
     * 配置信息
     *
     * @return
     */
    CameraConfig getConfig();

    /**
     * 删除图片缓存
     */
    void clearCache();

}
