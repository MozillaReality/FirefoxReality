/* -*- Mode: Java; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.vrbrowser.ui.views;

import android.animation.ObjectAnimator;
import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.FrameLayout;

import androidx.annotation.NonNull;
import androidx.databinding.DataBindingUtil;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;
import org.mozilla.vrbrowser.R;
import org.mozilla.vrbrowser.browser.AccountsManager;
import org.mozilla.vrbrowser.browser.BookmarksStore;
import org.mozilla.vrbrowser.browser.SettingsStore;
import org.mozilla.vrbrowser.browser.engine.Session;
import org.mozilla.vrbrowser.browser.engine.SessionStore;
import org.mozilla.vrbrowser.databinding.BookmarksBinding;
import org.mozilla.vrbrowser.ui.adapters.BookmarkAdapter;
import org.mozilla.vrbrowser.ui.callbacks.BookmarkItemCallback;
import org.mozilla.vrbrowser.ui.callbacks.BookmarksCallback;
import org.mozilla.vrbrowser.utils.UIThreadExecutor;

import java.util.ArrayList;
import java.util.List;

import mozilla.appservices.places.BookmarkRoot;
import mozilla.components.concept.storage.BookmarkNode;
import mozilla.components.concept.sync.AccountObserver;
import mozilla.components.concept.sync.AuthType;
import mozilla.components.concept.sync.OAuthAccount;
import mozilla.components.concept.sync.Profile;
import mozilla.components.service.fxa.SyncEngine;
import mozilla.components.service.fxa.sync.SyncReason;
import mozilla.components.service.fxa.sync.SyncStatusObserver;

public class BookmarksView extends FrameLayout implements BookmarksStore.BookmarkListener {

    private BookmarksBinding mBinding;
    private ObjectAnimator mSyncingAnimation;
    private AccountsManager mAccountManager;
    private BookmarkAdapter mBookmarkAdapter;
    private boolean mIgnoreNextListener;
    private boolean mIsSyncEnabled;
    private boolean mIsSignedIn;
    private ArrayList<BookmarksCallback> mBookmarksViewListeners;

    public BookmarksView(Context aContext) {
        super(aContext);
        initialize(aContext);
    }

    public BookmarksView(Context aContext, AttributeSet aAttrs) {
        super(aContext, aAttrs);
        initialize(aContext);
    }

    public BookmarksView(Context aContext, AttributeSet aAttrs, int aDefStyle) {
        super(aContext, aAttrs, aDefStyle);
        initialize(aContext);
    }

    @SuppressLint("ClickableViewAccessibility")
    private void initialize(Context aContext) {
        LayoutInflater inflater = LayoutInflater.from(aContext);

        mBookmarksViewListeners = new ArrayList<>();

        // Inflate this data binding layout
        mBinding = DataBindingUtil.inflate(inflater, R.layout.bookmarks, this, true);
        mBinding.setCallback(mBookmarksCallback);
        mBookmarkAdapter = new BookmarkAdapter(mBookmarkItemCallback, aContext);
        mBinding.bookmarksList.setAdapter(mBookmarkAdapter);
        mBinding.bookmarksList.setOnTouchListener((v, event) -> {
            v.requestFocusFromTouch();
            return false;
        });
        mBinding.setIsLoading(true);
        mBinding.executePendingBindings();

        Drawable[] drawables = mBinding.syncButton.getCompoundDrawables();
        mSyncingAnimation = ObjectAnimator.ofInt(drawables[0], "level", 0, 10000);
        mSyncingAnimation.setRepeatCount(ObjectAnimator.INFINITE);

        updateBookmarks();
        SessionStore.get().getBookmarkStore().addListener(this);

        mAccountManager = SessionStore.get().getAccountsManager();
        mAccountManager.addAccountListener(mAccountListener);
        mAccountManager.addSyncListener(mSyncListener);

        mIsSyncEnabled = mAccountManager.supportedSyncEngines().contains(SyncEngine.Bookmarks.INSTANCE);

        updateCurrentAccountState();

        setVisibility(GONE);

        mBinding.button.setOnClickListener(v -> mBinding.flipper.showNext());

        setOnTouchListener((v, event) -> {
            v.requestFocusFromTouch();
            return false;
        });
    }

    public void onDestroy() {
        SessionStore.get().getBookmarkStore().removeListener(this);
        mAccountManager.removeAccountListener(mAccountListener);
        mAccountManager.removeSyncListener(mSyncListener);
    }

    private final BookmarkItemCallback mBookmarkItemCallback = new BookmarkItemCallback() {
        @Override
        public void onClick(View view, BookmarkNode item) {
            mBinding.bookmarksList.requestFocusFromTouch();

            Session session = SessionStore.get().getActiveSession();
            session.loadUri(item.getUrl());
        }

        @Override
        public void onDelete(View view, BookmarkNode item) {
            mBinding.bookmarksList.requestFocusFromTouch();

            mIgnoreNextListener = true;
            SessionStore.get().getBookmarkStore().deleteBookmarkById(item.getGuid());
            mBookmarkAdapter.removeItem(item);
            if (mBookmarkAdapter.itemCount() == 0) {
                mBinding.setIsEmpty(true);
                mBinding.setIsLoading(false);
                mBinding.executePendingBindings();
            }
        }

        @Override
        public void onMore(View view, BookmarkNode item) {
            mBinding.bookmarksList.requestFocusFromTouch();

            int rowPosition = mBookmarkAdapter.getItemPosition(item.getGuid());
            RecyclerView.ViewHolder row = mBinding.bookmarksList.findViewHolderForLayoutPosition(rowPosition);
            boolean isLastVisibleItem = false;
            if (mBinding.bookmarksList.getLayoutManager() instanceof LinearLayoutManager) {
                LinearLayoutManager layoutManager = (LinearLayoutManager) mBinding.bookmarksList.getLayoutManager();
                int lastVisibleItem = layoutManager.findLastCompletelyVisibleItemPosition();
                if (rowPosition == layoutManager.findLastVisibleItemPosition() && rowPosition != lastVisibleItem) {
                    isLastVisibleItem = true;
                }
            }

            mBinding.getCallback().onShowContextMenu(
                    row.itemView,
                    item,
                    isLastVisibleItem);
        }
    };

    private BookmarksCallback mBookmarksCallback = new BookmarksCallback() {
        @Override
        public void onClearBookmarks(@NonNull View view) {
            mBookmarksViewListeners.forEach((listener) -> listener.onClearBookmarks(view));
        }

        @Override
        public void onSyncBookmarks(@NonNull View view) {
            switch(mAccountManager.getAccountStatus()) {
                case NEEDS_RECONNECT:
                case SIGNED_OUT:
                    mAccountManager.getAuthenticationUrlAsync().thenAcceptAsync((url) -> {
                        if (url != null) {
                            mAccountManager.setLoginOrigin(AccountsManager.LoginOrigin.BOOKMARKS);
                            SessionStore.get().getActiveStore().loadUri(url);
                        }
                    });
                    break;

                case SIGNED_IN:
                    mAccountManager.syncNowAsync(SyncReason.User.INSTANCE, false);

                    mBookmarksViewListeners.forEach((listener) -> listener.onSyncBookmarks(view));
                    break;

                default:
                    throw new IllegalStateException("Unexpected value: " + mAccountManager.getAccountStatus());
            }
        }

        @Override
        public void onShowContextMenu(@NonNull View view, BookmarkNode item, boolean isLastVisibleItem) {
            mBookmarksViewListeners.forEach((listener) -> listener.onShowContextMenu(view, item, isLastVisibleItem));
        }
    };

    public void addBookmarksListener(@NonNull BookmarksCallback listener) {
        if (!mBookmarksViewListeners.contains(listener)) {
            mBookmarksViewListeners.add(listener);
        }
    }

    public void removeBookmarksListener(@NonNull BookmarksCallback listener) {
        mBookmarksViewListeners.remove(listener);
    }

    private SyncStatusObserver mSyncListener = new SyncStatusObserver() {
        @Override
        public void onStarted() {
            mBinding.syncButton.setEnabled(false);
            mSyncingAnimation.start();
        }

        @Override
        public void onIdle() {
            mBinding.syncButton.setEnabled(true);
            mSyncingAnimation.cancel();
            updateUi();
        }

        @Override
        public void onError(@Nullable Exception e) {
            mBinding.syncButton.setEnabled(true);
            mSyncingAnimation.cancel();
            mBinding.syncDescription.setText(getContext().getString(R.string.fxa_account_last_no_synced));
        }
    };

    private void updateCurrentAccountState() {
        switch(mAccountManager.getAccountStatus()) {
            case NEEDS_RECONNECT:
            case SIGNED_OUT:
                mIsSignedIn = false;
                updateUi();
                break;

            case SIGNED_IN:
                mIsSignedIn = true;
                updateUi();
                break;

            default:
                throw new IllegalStateException("Unexpected value: " + mAccountManager.getAccountStatus());
        }
    }

    private AccountObserver mAccountListener = new AccountObserver() {

        @Override
        public void onAuthenticated(@NotNull OAuthAccount oAuthAccount, @NotNull AuthType authType) {
            mIsSignedIn = true;
            updateUi();
        }

        @Override
        public void onProfileUpdated(@NotNull Profile profile) {
        }

        @Override
        public void onLoggedOut() {
            mIsSignedIn = false;
            updateUi();
        }

        @Override
        public void onAuthenticationProblems() {
            mIsSignedIn = false;
            updateUi();
        }
    };

    private void updateUi() {
        if (mIsSignedIn) {
            mBinding.syncButton.setText(R.string.bookmarks_sync);
            mBinding.syncDescription.setVisibility(VISIBLE);

            mIsSyncEnabled = mAccountManager.supportedSyncEngines().contains(SyncEngine.Bookmarks.INSTANCE);
            if (mIsSyncEnabled) {
                mBinding.syncButton.setEnabled(true);
                mBinding.syncDescription.setVisibility(VISIBLE);

                long lastSync = mAccountManager.getLastSync();
                if (lastSync == 0) {
                    mBinding.syncDescription.setText(getContext().getString(R.string.fxa_account_last_no_synced));

                } else {
                    long timeDiff = System.currentTimeMillis() - lastSync;
                    if (timeDiff < 60000) {
                        mBinding.syncDescription.setText(getContext().getString(R.string.fxa_account_last_synced_now));

                    } else {
                        mBinding.syncDescription.setText(getContext().getString(R.string.fxa_account_last_synced, timeDiff / 60000));
                    }
                }

            } else {
                mBinding.syncButton.setEnabled(false);
                mBinding.syncDescription.setVisibility(GONE);
            }

        } else {
            mBinding.syncButton.setText(R.string.fxa_account_sing_to_sync);
            mBinding.syncDescription.setVisibility(GONE);
        }
    }

    private void updateBookmarks() {
        SessionStore.get().getBookmarkStore().getBookmarks(BookmarkRoot.Mobile.getId()).thenAcceptAsync(this::showBookmarks, new UIThreadExecutor());
    }

    private void showBookmarks(List<BookmarkNode> aBookmarks) {
        if (aBookmarks == null || aBookmarks.size() == 0) {
            mBinding.setIsEmpty(true);
            mBinding.setIsLoading(false);

        } else {
            mBinding.setIsEmpty(false);
            mBinding.setIsLoading(false);
            mBookmarkAdapter.setBookmarkList(aBookmarks);
            mBinding.bookmarksList.post(() -> mBinding.bookmarksList.smoothScrollToPosition(
                    mBookmarkAdapter.getItemCount() > 0 ? mBookmarkAdapter.getItemCount() - 1 : 0));
        }
        mBinding.executePendingBindings();
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        super.onLayout(changed, left, top, right, bottom);

        double width = Math.ceil(getWidth()/getContext().getResources().getDisplayMetrics().density);
        mBookmarkAdapter.setNarrow(width < SettingsStore.WINDOW_WIDTH_DEFAULT);
    }

    // BookmarksStore.BookmarksViewListener
    @Override
    public void onBookmarksUpdated() {
        if (mIgnoreNextListener) {
            mIgnoreNextListener = false;
            return;
        }
        updateBookmarks();
    }

    @Override
    public void onBookmarkAdded() {
        if (mIgnoreNextListener) {
            mIgnoreNextListener = false;
            return;
        }
        updateBookmarks();
    }
}
