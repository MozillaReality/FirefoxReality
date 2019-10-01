package org.mozilla.vrbrowser.ui.callbacks;

import android.view.View;

import androidx.annotation.NonNull;

import mozilla.components.concept.storage.BookmarkNode;

public interface BookmarksCallback {
    default void onClearBookmarks(@NonNull View view) {}
    default void onSyncBookmarks(@NonNull View view) {}
    default void onShowContextMenu(@NonNull View view, BookmarkNode item, boolean isLastVisibleItem) {}
}
