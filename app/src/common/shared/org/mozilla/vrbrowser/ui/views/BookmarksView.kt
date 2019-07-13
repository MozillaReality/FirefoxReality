/* -*- Mode: Java; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.vrbrowser.ui.views

import android.content.Context
import android.view.LayoutInflater
import android.view.View
import android.widget.FrameLayout

import org.mozilla.geckoview.AllowOrDeny
import org.mozilla.geckoview.GeckoResult
import org.mozilla.geckoview.GeckoSession
import org.mozilla.geckoview.WebRequestError
import org.mozilla.vrbrowser.R
import org.mozilla.vrbrowser.audio.AudioEngine
import org.mozilla.vrbrowser.browser.BookmarksStore
import org.mozilla.vrbrowser.browser.SessionStore
import org.mozilla.vrbrowser.databinding.BookmarksBinding
import org.mozilla.vrbrowser.ui.adapters.BookmarkAdapter
import org.mozilla.vrbrowser.ui.callbacks.BookmarkClickCallback
import org.mozilla.vrbrowser.ui.widgets.BookmarkListener
import org.mozilla.vrbrowser.utils.UIThreadExecutor

import java.util.ArrayList
import androidx.databinding.DataBindingUtil

import mozilla.components.concept.storage.BookmarkNode
import mozilla.components.concept.storage.BookmarkNodeType
import java.util.function.Consumer

class BookmarksView(context: Context) : FrameLayout(context), GeckoSession.NavigationDelegate, BookmarksStore.BookmarkListener {
    // Inflate this data binding layout
    private val mBinding: BookmarksBinding = DataBindingUtil.inflate(
        LayoutInflater.from(context), R.layout.bookmarks, this, true
    )
    private val mBookmarkListeners: MutableList<BookmarkListener> = ArrayList()
    private val mAudio: AudioEngine? = AudioEngine.fromContext(context)
    private var mIgnoreNextListener: Boolean = false

    private val mBookmarkClickCallback = object : BookmarkClickCallback {
        override fun onClick(bookmark: BookmarkNode) {
            mAudio?.playSound(AudioEngine.Sound.CLICK)

            when (bookmark.type) {
                // Ignore clicks on separators.
                BookmarkNodeType.SEPARATOR -> return
                // Load regular bookmarks.
                BookmarkNodeType.ITEM -> SessionStore.get().loadUri(bookmark.url)
                // TODO: display selected folder contents.
                BookmarkNodeType.FOLDER -> return
            }
        }

        override fun onDelete(bookmark: BookmarkNode) {
            mAudio?.playSound(AudioEngine.Sound.CLICK)

            mIgnoreNextListener = true
            SessionStore.get().bookmarkStore.deleteBookmarkById(bookmark.guid)
            mBookmarkAdapter.removeItem(bookmark)
            if (mBookmarkAdapter.itemCount() == 0) {
                mBinding.isEmpty = true
                mBinding.isLoading = false
                mBinding.executePendingBindings()
            }
        }
    }

    private val mBookmarkAdapter: BookmarkAdapter = BookmarkAdapter(mBookmarkClickCallback, context)

    init {
        SessionStore.get().addNavigationListener(this)

        mBinding.bookmarksList.adapter = mBookmarkAdapter
        mBinding.isLoading = true
        mBinding.executePendingBindings()
        syncBookmarks()
        SessionStore.get().bookmarkStore.addListener(this)

        visibility = View.GONE
    }

    fun addListeners(vararg listeners: BookmarkListener) {
        mBookmarkListeners.addAll(listeners.toList())
    }

    fun onDestroy() {
        mBookmarkListeners.clear()
        SessionStore.get().bookmarkStore.removeListener(this)
    }

    private fun notifyBookmarksShown() {
        mBookmarkListeners.forEach { it.onBookmarksShown() }
    }

    private fun notifyBookmarksHidden() {
        mBookmarkListeners.forEach { it.onBookmarksHidden() }
    }


    private fun syncBookmarks() {
        SessionStore.get().bookmarkStore.getBookmarks().thenAcceptAsync(Consumer {
            this.showBookmarks(it)
        }, UIThreadExecutor())
    }

    private fun showBookmarks(aBookmarks: List<BookmarkNode>?) {
        if (aBookmarks == null || aBookmarks.isEmpty()) {
            mBinding.isEmpty = true
            mBinding.isLoading = false

        } else {
            mBinding.isEmpty = false
            mBinding.isLoading = false
            mBookmarkAdapter.setBookmarkList(aBookmarks)
        }
        mBinding.executePendingBindings()
    }

    override fun setVisibility(visibility: Int) {
        super.setVisibility(visibility)

        if (visibility == View.VISIBLE) {
            notifyBookmarksShown()

        } else {
            notifyBookmarksHidden()
        }
    }

    // NavigationDelegate

    override fun onLocationChange(session: GeckoSession, url: String?) {
        if (visibility == View.VISIBLE && url != null && url != ABOUT_BLANK) {
            notifyBookmarksHidden()
        }
    }

    override fun onCanGoBack(session: GeckoSession, canGoBack: Boolean) {

    }

    override fun onCanGoForward(session: GeckoSession, canGoForward: Boolean) {

    }

    override fun onLoadRequest(session: GeckoSession, request: GeckoSession.NavigationDelegate.LoadRequest): GeckoResult<AllowOrDeny>? {
        return GeckoResult.ALLOW
    }

    override fun onNewSession(session: GeckoSession, uri: String): GeckoResult<GeckoSession>? {
        return null
    }

    override fun onLoadError(session: GeckoSession, uri: String?, error: WebRequestError): GeckoResult<String>? {
        return null
    }

    // BookmarksStore.BookmarkListener
    override fun onBookmarksUpdated() {
        if (mIgnoreNextListener) {
            mIgnoreNextListener = false
            return
        }
        syncBookmarks()
    }

    companion object {
        private const val ABOUT_BLANK = "about:blank"
    }
}
