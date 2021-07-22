package com.lgh.uvccamera.config;

import com.lgh.uvccamera.bean.PicturePath;
import com.lgh.uvccamera.utils.LogUtil;

/**
 * 描述：相关配置
 * 作者：liugh
 * 日期：2018/12/27
 * 版本：v2.0.0
 */
public class CameraConfig {
    private PicturePath mPicturePath = PicturePath.APPCACHE; // 图片保存路径
    private String mDirName = "uvccamera"; // 图片保存目录名称
    private int mVendorId = 0; // 需要根据供应商id过滤则设置，不需要过滤设为0
    private int mProductId = 0; // 需要根据产品id过滤则设置，不需要过滤设为0

    public CameraConfig isDebug(boolean debug) {
        LogUtil.allowD = debug;
        LogUtil.allowE = debug;
        LogUtil.allowI = debug;
        LogUtil.allowV = debug;
        LogUtil.allowW = debug;
        LogUtil.allowWtf = debug;
        return this;
    }

    public PicturePath getPicturePath() {
        return mPicturePath;
    }

    public CameraConfig setPicturePath(PicturePath mPicturePath) {
        this.mPicturePath = mPicturePath;
        return this;
    }

    public String getDirName() {
        return mDirName;
    }

    public CameraConfig setDirName(String mDirName) {
        this.mDirName = mDirName;
        return this;
    }

    public int getVendorId() {
        return mVendorId;
    }

    public CameraConfig setVendorId(int mVendorId) {
        this.mVendorId = mVendorId;
        return this;
    }

    public int getProductId() {
        return mProductId;
    }

    public CameraConfig setProductId(int mProductId) {
        this.mProductId = mProductId;
        return this;
    }

    @Override
    public String toString() {
        return "CameraConfig{" +
                "mPicturePath=" + mPicturePath +
                ", mDirName='" + mDirName + '\'' +
                ", mVendorId=" + mVendorId +
                ", mProductId=" + mProductId +
                '}';
    }
}
