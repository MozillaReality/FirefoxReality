/* -*- Mode: Java; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.vrbrowser.ui.widgets.settings;

import android.content.Context;
import android.view.LayoutInflater;

import androidx.databinding.DataBindingUtil;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;
import org.mozilla.vrbrowser.R;
import org.mozilla.vrbrowser.browser.AccountsManager;
import org.mozilla.vrbrowser.browser.engine.SessionStore;
import org.mozilla.vrbrowser.databinding.OptionsFxaAccountBinding;
import org.mozilla.vrbrowser.ui.views.settings.SwitchSetting;
import org.mozilla.vrbrowser.ui.widgets.WidgetManagerDelegate;
import org.mozilla.vrbrowser.utils.SystemUtils;

import mozilla.components.concept.sync.AccountObserver;
import mozilla.components.concept.sync.AuthType;
import mozilla.components.concept.sync.OAuthAccount;
import mozilla.components.concept.sync.Profile;
import mozilla.components.service.fxa.SyncEngine;
import mozilla.components.service.fxa.sync.SyncReason;
import mozilla.components.service.fxa.sync.SyncStatusObserver;

class FxAAccountOptionsView extends SettingsView {

    private static final String LOGTAG = SystemUtils.createLogtag(FxAAccountOptionsView.class);

    private static final long SYNC_REFRESH_TIME = 10L;

    private OptionsFxaAccountBinding mBinding;
    private AccountsManager mAccountManager;

    public FxAAccountOptionsView(Context aContext, WidgetManagerDelegate aWidgetManager) {
        super(aContext, aWidgetManager);
        initialize(aContext);
    }

    private void initialize(Context aContext) {
        LayoutInflater inflater = LayoutInflater.from(aContext);

        // Inflate this data binding layout
        mBinding = DataBindingUtil.inflate(inflater, R.layout.options_fxa_account, this, true);

        mScrollbar = mBinding.scrollbar;

        // Header
        mBinding.headerLayout.setBackClickListener(view -> onDismiss());

        mAccountManager = SessionStore.get().getAccountsManager();

        mBinding.signButton.setOnClickListener(view -> mAccountManager.logoutAsync());
        mBinding.bookmarksSyncSwitch.setOnCheckedChangeListener(mBookmarksSyncListener);

        mBinding.historySyncSwitch.setOnCheckedChangeListener(mHistorySyncListener);

        updateCurrentAccountState();

        // Footer
        mBinding.footerLayout.setResetClickListener(v -> resetOptions());
    }

    @Override
    public void onShown() {
        super.onShown();

        mAccountManager.addAccountListener(mAccountListener);
        mAccountManager.addSyncListener(mSyncListener);

        setBookmarksSync(mAccountManager.isEngineEnabled(SyncEngine.Bookmarks.INSTANCE), false);
        setHistorySync(mAccountManager.isEngineEnabled(SyncEngine.History.INSTANCE), false);
    }

    @Override
    public void onHidden() {
        super.onHidden();

        mAccountManager.removeAccountListener(mAccountListener);
        mAccountManager.removeSyncListener(mSyncListener);
    }

    private SwitchSetting.OnCheckedChangeListener mBookmarksSyncListener = (compoundButton, value, apply) -> setBookmarksSync(value, apply);

    private SwitchSetting.OnCheckedChangeListener mHistorySyncListener = (compoundButton, value, apply) -> setHistorySync(value, apply);

    private void resetOptions() {
        setBookmarksSync(true, true);
        setHistorySync(true, true);
    }

    private void setBookmarksSync(boolean value, boolean doApply) {
        updateBookmarkSwitch(value);
        if (doApply) {
            mAccountManager.setSyncStatus(SyncEngine.Bookmarks.INSTANCE, value);
            mAccountManager.syncNowAsync(SyncReason.EngineChange.INSTANCE, false);
        }
    }

    private void updateBookmarkSwitch(boolean value) {
        mBinding.bookmarksSyncSwitch.setOnCheckedChangeListener(null);
        mBinding.bookmarksSyncSwitch.setValue(value, false);
        mBinding.bookmarksSyncSwitch.setOnCheckedChangeListener(mBookmarksSyncListener);
    }

    private void setHistorySync(boolean value, boolean doApply) {
        updateHistorySwitch(value);
        if (doApply) {
            mAccountManager.setSyncStatus(SyncEngine.History.INSTANCE, value);
            mAccountManager.syncNowAsync(SyncReason.EngineChange.INSTANCE, false);
        }
    }

    private void updateHistorySwitch(boolean value) {
        mBinding.historySyncSwitch.setOnCheckedChangeListener(null);
        mBinding.historySyncSwitch.setValue(value, false);
        mBinding.historySyncSwitch.setOnCheckedChangeListener(mHistorySyncListener);
    }

    private SyncStatusObserver mSyncListener = new SyncStatusObserver() {
        @Override
        public void onStarted() {

        }

        @Override
        public void onIdle() {
            mBinding.bookmarksSyncSwitch.setValue(mAccountManager.isEngineEnabled(SyncEngine.Bookmarks.INSTANCE), false);
            mBinding.historySyncSwitch.setValue(mAccountManager.isEngineEnabled(SyncEngine.History.INSTANCE), false);
        }

        @Override
        public void onError(@Nullable Exception e) {

        }
    };

    void updateCurrentAccountState() {
        switch(mAccountManager.getAccountStatus()) {
            case NEEDS_RECONNECT:
                mBinding.signButton.setButtonText(R.string.settings_fxa_account_reconnect);
                break;

            case SIGNED_IN:
                mBinding.signButton.setButtonText(R.string.settings_fxa_account_sign_out);
                Profile profile = mAccountManager.accountProfile();
                if (profile != null) {
                    updateProfile(profile);

                } else {
                    mAccountManager.updateProfileAsync().thenAcceptAsync((u) -> updateProfile(mAccountManager.accountProfile()));
                }
                break;

            case SIGNED_OUT:
                mBinding.signButton.setButtonText(R.string.settings_fxa_account_sign_in);
                break;

            default:
                throw new IllegalStateException("Unexpected value: " + mAccountManager.getAccountStatus());
        }
    }

    private void updateProfile(Profile profile) {
        if (profile != null) {
            mBinding.accountEmail.setText(profile.getEmail());
        }
    }

    private AccountObserver mAccountListener = new AccountObserver() {

        @Override
        public void onAuthenticated(@NotNull OAuthAccount oAuthAccount, @NotNull AuthType authType) {
            mBinding.signButton.setButtonText(R.string.settings_fxa_account_sign_out);
        }

        @Override
        public void onProfileUpdated(@NotNull Profile profile) {
            post(() -> mBinding.accountEmail.setText(profile.getEmail()));
        }

        @Override
        public void onLoggedOut() {
            post(() -> FxAAccountOptionsView.this.onDismiss());
        }

        @Override
        public void onAuthenticationProblems() {
            post(() -> FxAAccountOptionsView.this.onDismiss());
        }
    };

}
