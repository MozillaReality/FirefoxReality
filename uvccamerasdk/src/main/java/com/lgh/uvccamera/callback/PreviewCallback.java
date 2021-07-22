package com.lgh.uvccamera.callback;

/**
 * 描述：预览回调接口
 * 作者：liugh
 * 日期：2018/11/20
 * 版本：v2.0.0
 */
public interface PreviewCallback {
    /**
     * 预览流回调
     *
     * @param yuv yuv格式的数据流
     */
    void onPreviewFrame(byte[] yuv);
}
