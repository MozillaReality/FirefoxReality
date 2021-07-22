package com.lgh.uvccamera.utils;

import android.content.Context;
import android.graphics.Bitmap;
import android.os.Environment;

import java.io.File;
import java.io.FileOutputStream;

/**
 * 描述：文件存取工具类
 * 作者：liugh
 * 日期：2018/12/25
 * 版本：v2.0.0
 */
public class FileUtil {

    /**
     * 判断当前系统中是否存在外部存储器（一般为SD卡）
     *
     * @return 当前系统中是否存在外部存储器
     */
    public static boolean hasExternalStorage() {
        return Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED);
    }

    /**
     * 获取外部存储器（一般为SD卡）的路径
     *
     * @return 外部存储器的绝对路径
     */
    public static String getExternalStoragePath() {
        return Environment.getExternalStorageDirectory().getAbsolutePath();
    }

    /**
     * 获取SD卡目录
     *
     * @param foderName
     * @param fileName
     * @return
     */
    public static File getSDCardDir(String foderName) {
        if (!hasExternalStorage()) {
            return null;
        }
        return new File(getExternalStoragePath() + File.separator + foderName);
    }

    /**
     * 获取SD卡文件
     *
     * @param foderName
     * @param fileName
     * @return
     */
    public static File getSDCardFile(String foderName, String fileName) {
        File foder = getSDCardDir(foderName);
        if (foder == null) {
            return null;
        }
        if (!foder.exists()) {
            if (!foder.mkdirs()) {
                return null;
            }
        }
        return new File(foder, fileName);
    }

    /**
     * 获取缓存目录
     *
     * @param context
     * @param dirName
     * @return
     */
    public static String getDiskCacheDir(Context context, String dirName) {
        String cachePath;
        if ((Environment.MEDIA_MOUNTED.equals(Environment.getExternalStorageState())
                || !Environment.isExternalStorageRemovable())
                && context.getExternalCacheDir() != null) {
            cachePath = context.getExternalCacheDir().getPath();
        } else {
            cachePath = context.getCacheDir().getPath();
        }
        return cachePath + File.separator + dirName;
    }

    /**
     * 获取缓存目录文件
     *
     * @param context
     * @param dirName
     * @param fileName
     * @return
     */
    public static File getCacheFile(Context context, String dirName, String fileName) {
        File dirFile = new File(getDiskCacheDir(context, dirName));
        if (!dirFile.exists()) {
            if (!dirFile.mkdirs()) {
                LogUtil.d("failed to create directory");
                return null;
            }
        }
        return new File(dirFile.getPath() + File.separator + fileName);
    }

    /**
     * 删除文件或文件夹
     *
     * @param dirFile
     * @return
     */
    public static boolean deleteFile(File dirFile) {
        if (!dirFile.exists()) {
            return false;
        }
        if (dirFile.isFile()) {
            return dirFile.delete();
        } else {
            for (File file : dirFile.listFiles()) {
                deleteFile(file);
            }
        }
        return dirFile.delete();
    }

    /**
     * 将yuv格式byte数组转化成jpeg图片并保存
     *
     * @param file
     * @param yuv
     * @param width
     * @param height
     */
    public static String saveYuv2Jpeg(File file, byte[] yuv, int width, int height) {
        return saveBitmap(file, ImageUtil.yuv2Bitmap(yuv, width, height));
    }

    /**
     * 将yuv格式byte数组转化成jpeg图片并保存
     *
     * @param file
     * @param yuv
     * @param width
     * @param height
     * @param rotation
     */
    public static String saveYuv2Jpeg(File file, byte[] yuv, int width, int height, float rotation) {
        return saveBitmap(file, ImageUtil.yuv2Bitmap(yuv, width, height, rotation));
    }

    /**
     * 保存bitmap
     *
     * @param file
     * @param bitmap
     */
    public static String saveBitmap(File file, Bitmap bitmap) {
        if (file == null || bitmap == null) {
            return null;
        }
        try {
            FileOutputStream fos = new FileOutputStream(file);
            bitmap.compress(Bitmap.CompressFormat.JPEG, 100, fos);
            if (bitmap != null && !bitmap.isRecycled()) {
                bitmap.recycle();
                bitmap = null;
            }
            fos.flush();
            fos.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return file.getAbsolutePath();
    }

}
