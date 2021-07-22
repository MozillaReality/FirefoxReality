package com.lgh.uvccamera.utils;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.YuvImage;

import java.io.ByteArrayOutputStream;

/**
 * 描述：图片工具类
 * 作者：liugh
 * 日期：2018/12/11
 * 版本：v2.0.0
 */
public class ImageUtil {

    /**
     * yuv数据转bitmap
     *
     * @param yuv
     * @param width
     * @param height
     * @return
     */
    public static Bitmap yuv2Bitmap(byte[] yuv, int width, int height) {
        Bitmap bitmap = null;
        try {
            ByteArrayOutputStream bos = new ByteArrayOutputStream(yuv.length);
            YuvImage yuvImage = new YuvImage(yuv, ImageFormat.NV21, width, height, null);
            boolean success = yuvImage.compressToJpeg(new Rect(0, 0, width, height), 100, bos);
            if (success) {
                byte[] buffer = bos.toByteArray();
                bitmap = BitmapFactory.decodeByteArray(buffer, 0, buffer.length);
            }
            bos.flush();
            bos.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return bitmap;
    }

    /**
     * yuv数据转bitmap
     *
     * @param yuv
     * @param width
     * @param height
     * @param rotation
     * @return
     */
    public static Bitmap yuv2Bitmap(byte[] yuv, int width, int height, float rotation) {
        Bitmap bitmap = yuv2Bitmap(yuv, width, height);
        return rotateBimap(bitmap, rotation);
    }

    /**
     * 旋转图片
     *
     * @param bitmap
     * @param rotation 旋转角度
     * @return
     */
    public static Bitmap rotateBimap(Bitmap bitmap, float rotation) {
        if (bitmap == null || rotation == 0) {
            return bitmap;
        }
        Matrix matrix = new Matrix();
        matrix.postRotate(rotation);
        Bitmap rotateBimap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);
        if (bitmap != null && !bitmap.isRecycled()) {
            bitmap.recycle();
            bitmap = null;
        }
        return rotateBimap;
    }

    /**
     * Bitmap缩放
     *
     * @param bitmap
     * @param newWidth
     * @param newHeight
     * @return
     */
    public static Bitmap scaleBitmap(Bitmap bitmap, int newWidth, int newHeight) {
        if (bitmap == null) {
            return null;
        }
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();
        float scaleWidth = (float) newWidth / (float) width;
        float scaleHeight = (float) newHeight / (float) height;
        Matrix matrix = new Matrix();
        matrix.postScale(scaleWidth, scaleHeight);
        Bitmap scaleBitmap = null;
        try {
            scaleBitmap = Bitmap.createBitmap(bitmap, 0, 0, width, height, matrix, true);
        } catch (OutOfMemoryError e) {
            while (scaleBitmap == null) {
                System.gc();
                System.runFinalization();
                scaleBitmap = Bitmap.createBitmap(bitmap, 0, 0, width, height, matrix, true);
            }
        }
        if (bitmap != scaleBitmap && !bitmap.isRecycled()) {
            bitmap.recycle();
            bitmap = null;
        }
        return scaleBitmap;
    }

}
