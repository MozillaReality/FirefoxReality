package org.mozilla.vrbrowser.utils;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;
import android.util.LruCache;
import androidx.annotation.NonNull;
import com.jakewharton.disklrucache.DiskLruCache;

import org.mozilla.vrbrowser.VRBrowserApplication;

import java.io.File;
import java.io.IOException;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Executor;

public class BitmapCache {
    private Context mContext;
    private LruCache<String, Bitmap> mMemoryCache;
    private DiskLruCache mDiskCache;
    private Executor mIOExecutor;
    private final Object mLock = new Object();
    private static final int DISK_CACHE_SIZE = 1024 * 1024 * 100; // 100MB
    private static final String LOGTAG = SystemUtils.createLogtag(BitmapCache.class);

    public static BitmapCache getInstance(Context aContext) {
        return ((VRBrowserApplication)aContext.getApplicationContext()).getBitmapCache();
    }

    public BitmapCache(@NonNull Context aContext, @NonNull Executor aIOExecutor) {
        mContext = aContext;
        mIOExecutor = aIOExecutor;
        initMemoryCache();
        initDiskCache();
    }

    void initMemoryCache() {
        // Get  available VM memory in KB.
        final int maxMemory = (int) (Runtime.getRuntime().maxMemory() / 1024);
        // Use 1/8th of the available memory for this memory cache.
        final int cacheSize = maxMemory / 8;

        mMemoryCache = new LruCache<String, Bitmap>(cacheSize) {
            @Override
            protected int sizeOf(String key, Bitmap bitmap) {
                // Use KB as the size of the item
                return bitmap.getByteCount() / 1024;
            }
        };
    }

    void initDiskCache() {
        String path = mContext.getCacheDir() + File.separator + "snapshots";
        mIOExecutor.execute(() -> {
            try {
                mDiskCache = DiskLruCache.open(new File(path), 1, 1, DISK_CACHE_SIZE);
            }
            catch (Exception ex) {
                Log.e(LOGTAG, "Failed to initialize DiskLruCache:" + ex.getMessage());
            }
        });
    }

    public void addBitmap(@NonNull String aKey, @NonNull Bitmap aBitmap) {
        mMemoryCache.put(aKey, aBitmap);
        runIO(() -> {
            DiskLruCache.Editor editor = null;
            try {
                editor = mDiskCache.edit(aKey);
                if (editor != null) {
                    aBitmap.compress(Bitmap.CompressFormat.PNG, 95, editor.newOutputStream(0));
                    editor.commit();
                }
            }
            catch (Exception ex) {
                Log.e(LOGTAG, "Failed to add Bitmap to DiskLruCache:" + ex.getMessage());
                if (editor != null) {
                    try {
                        editor.abort();
                    }
                    catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }
        });
    }

    public @NonNull CompletableFuture<Bitmap> getBitmap(@NonNull String aKey) {
        Bitmap cached = mMemoryCache.get(aKey);
        if (cached != null) {
            return CompletableFuture.completedFuture(cached);
        } else {
            CompletableFuture<Bitmap> result = new CompletableFuture<>();
            runIO(() -> {
                try (DiskLruCache.Snapshot snapshot = mDiskCache.get(aKey)){
                    if (snapshot != null) {
                        Bitmap bitmap = BitmapFactory.decodeStream(snapshot.getInputStream(0));
                        if (bitmap != null) {
                            result.complete(bitmap);
                            return;
                        }
                    }
                }
                catch (Exception ex) {
                    Log.e(LOGTAG, "Failed to get Bitmap from DiskLruCache:" + ex.getMessage());
                }

                result.complete(null);
            });
            return result;
        }
    }

    public void removeBitmap(@NonNull String aKey) {
        mMemoryCache.remove(aKey);
        runIO(() -> {
            try {
                mDiskCache.remove(aKey);
            } catch (Exception ex) {
                Log.e(LOGTAG, "Failed to remove Bitmap from DiskLruCache:" + ex.getMessage());
            }
        });
    }

    private void runIO(Runnable aRunnable) {
        mIOExecutor.execute(() -> {
            if (mDiskCache != null) {
                synchronized (mLock) {
                    aRunnable.run();
                }
            }
        });
    }

}
