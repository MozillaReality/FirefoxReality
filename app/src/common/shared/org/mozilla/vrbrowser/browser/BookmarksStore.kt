/* -*- Mode: Java; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.vrbrowser.browser

import android.content.Context
import android.os.Handler
import android.os.Looper
import android.util.Log
import androidx.lifecycle.ProcessLifecycleOwner
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.future.future
import mozilla.appservices.places.BookmarkRoot;
import mozilla.components.concept.storage.BookmarkNode
import mozilla.components.service.fxa.sync.SyncStatusObserver
import org.mozilla.vrbrowser.VRBrowserApplication
import java.util.concurrent.CompletableFuture

class BookmarksStore constructor(aContext: Context) {
    private var mListeners = ArrayList<BookmarkListener>()

    private val storage = (aContext.applicationContext as VRBrowserApplication).places.bookmarks

    // Bookmarks might have changed during sync, so notify our listeners.
    private val syncStatusObserver = object : SyncStatusObserver {
        override fun onStarted() {}

        override fun onIdle() {
            Log.d("BookmarksStore", "Detected that sync is finished, notifying listeners")
            notifyListeners()
        }

        override fun onError(error: Exception?) {}
    }

    init {
        (aContext.applicationContext as VRBrowserApplication).services.accountManager.registerForSyncEvents(
            syncStatusObserver, ProcessLifecycleOwner.get(), false
        )
    }

    interface BookmarkListener {
        fun onBookmarksUpdated()
    }

    fun addListener(aListener: BookmarkListener) {
        if (!mListeners.contains(aListener)) {
            mListeners.add(aListener)
        }
    }

    fun removeListener(aListener: BookmarkListener) {
        mListeners.remove(aListener)
    }

    fun removeAllListeners() {
        mListeners.clear()
    }

    fun getBookmarks(): CompletableFuture<List<BookmarkNode>?> = GlobalScope.future {
        storage.getTree(BookmarkRoot.Mobile.id, true)?.children?.toMutableList()
    }

    fun addBookmark(aURL: String, aTitle: String) = GlobalScope.future {
        storage.addItem(BookmarkRoot.Mobile.id, aURL, aTitle, null)
        notifyListeners()
    }

    fun deleteBookmarkByURL(aURL: String) = GlobalScope.future {
        val bookmark = getBookmarkByUrl(aURL)
        if (bookmark != null) {
            storage.deleteNode(bookmark.guid)
        }
        notifyListeners()
    }

    fun deleteBookmarkById(aId: String) = GlobalScope.future {
        storage.deleteNode(aId)
        notifyListeners()
    }

    fun isBookmarked(aURL: String): CompletableFuture<Boolean> = GlobalScope.future {
        getBookmarkByUrl(aURL) != null
    }


    private suspend fun getBookmarkByUrl(aURL: String): BookmarkNode? {
        val bookmarks: List<BookmarkNode>? = storage.getBookmarksWithUrl(aURL)
        if (bookmarks == null || bookmarks.isEmpty()) {
            return null
        }

        for (bookmark in bookmarks) {
            if (bookmark.url.equals(aURL)) {
                return bookmark
            }
        }

        return null
    }

    private fun notifyListeners() {
        if (mListeners.size > 0) {
            val listenersCopy = ArrayList(mListeners)
            Handler(Looper.getMainLooper()).post {
                for (listener in listenersCopy) {
                    listener.onBookmarksUpdated()
                }
            }
        }
    }
}
