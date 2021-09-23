/* -*- Mode: Java; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.vrbrowser.ui.widgets.settings;

import android.content.Context;
import android.graphics.Point;
import android.view.LayoutInflater;
import android.view.View;

import androidx.databinding.DataBindingUtil;

import org.mozilla.vrbrowser.R;
import org.mozilla.vrbrowser.browser.SettingsStore;
import org.mozilla.vrbrowser.databinding.OptionsProxyBinding;
import org.mozilla.vrbrowser.ui.views.settings.RadioGroupSetting;
import org.mozilla.vrbrowser.ui.views.settings.SwitchSetting;
import org.mozilla.vrbrowser.ui.widgets.WidgetManagerDelegate;
import org.mozilla.vrbrowser.ui.widgets.WidgetPlacement;

class ProxyOptionsView extends SettingsView {

    private OptionsProxyBinding mBinding;
    private String mDefaultProxyUrl;
    private String mDefaultProxyPort;

    public ProxyOptionsView(Context aContext, WidgetManagerDelegate aWidgetManager) {
        super(aContext, aWidgetManager);
        initialize(aContext);
    }

    private void initialize(Context aContext) {
        updateUI();
    }

    @Override
    protected void updateUI() {
        super.updateUI();

        LayoutInflater inflater = LayoutInflater.from(getContext());

        // Inflate this data binding layout
        mBinding = DataBindingUtil.inflate(inflater, R.layout.options_proxy, this, true);

        mScrollbar = mBinding.scrollbar;

        // Header
        mBinding.headerLayout.setBackClickListener(view -> onDismiss());

        // Footer
        mBinding.footerLayout.setFooterButtonClickListener(mResetListener);

        // Options
        mBinding.curvedDisplaySwitch.setOnCheckedChangeListener(mCurvedDisplayListener);
        setCurvedDisplay(SettingsStore.getInstance(getContext()).getCylinderDensity() > 0.0f, false);

        mDefaultProxyUrl = getContext().getString(R.string.proxy_url);
        mDefaultProxyPort = getContext().getString(R.string.proxy_port);

//        mBinding.proxyUrlEdit.setHint1(getContext().getString(R.string.homepage_hint, getContext().getString(R.string.app_name)));
        mBinding.proxyUrlEdit.setDefaultFirstValue(mDefaultProxyUrl);
        mBinding.proxyUrlEdit.setFirstText(SettingsStore.getInstance(getContext()).getProxyUrl());
        mBinding.proxyUrlEdit.setOnClickListener(mProxyUrlListener);

        mBinding.proxyPortEdit.setDefaultFirstValue(mDefaultProxyUrl);
        mBinding.proxyPortEdit.setFirstText(SettingsStore.getInstance(getContext()).getProxyPort());
        mBinding.proxyPortEdit.setOnClickListener(mProxyPortListener);

//        setProxy(SettingsStore.getInstance(getContext()).getProxyUrl(), SettingsStore.getInstance(getContext()).getProxyPort());
    }

    @Override
    public void onHidden() {
        if (!isEditing()) {
            super.onHidden();
        }
    }

    @Override
    protected void onDismiss() {
        if (!isEditing()) {
            super.onDismiss();
        }
    }

    @Override
    public boolean isEditing() {
        boolean editing = false;

        if (mBinding.proxyUrlEdit.isEditing()) {
            editing = true;
            mBinding.proxyUrlEdit.cancel();
        }

        if (mBinding.proxyPortEdit.isEditing()) {
            editing = true;
            mBinding.proxyPortEdit.cancel();
        }

        return editing;
    }

    private OnClickListener mProxyUrlListener = (view) -> {
        if (!mBinding.proxyUrlEdit.getFirstText().isEmpty()) {
            setProxy(mBinding.proxyUrlEdit.getFirstText(), mBinding.proxyUrlEdit.getFirstText());

        } else {
            setProxy(mDefaultProxyUrl, mDefaultProxyPort);
        }
    };

    private OnClickListener mProxyPortListener = (view) -> {
        if (!mBinding.proxyPortEdit.getFirstText().isEmpty()) {
            setProxy(mBinding.proxyPortEdit.getFirstText(), mBinding.proxyPortEdit.getFirstText());

        } else {
            setProxy(mDefaultProxyUrl, mDefaultProxyPort);
        }
    };

    private SwitchSetting.OnCheckedChangeListener mCurvedDisplayListener = (compoundButton, enabled, apply) ->
            setCurvedDisplay(enabled, true);

    private OnClickListener mResetListener = (view) -> {
        boolean restart = false;

        setProxy(mDefaultProxyUrl, mDefaultProxyPort);
        setCurvedDisplay(false, true);

        if (restart) {
            showRestartDialog();
        }
    };

    private void setCurvedDisplay(boolean value, boolean doApply) {
        mBinding.curvedDisplaySwitch.setOnCheckedChangeListener(null);
        mBinding.curvedDisplaySwitch.setValue(value, false);
        mBinding.curvedDisplaySwitch.setOnCheckedChangeListener(mCurvedDisplayListener);

        if (doApply) {
            float density = value ? SettingsStore.CYLINDER_DENSITY_ENABLED_DEFAULT : 0.0f;
            SettingsStore.getInstance(getContext()).setCylinderDensity(density);
            mWidgetManager.setCylinderDensity(density);
        }
    }

    private void setProxy(String newProxyUrl, String newProxyPort) {
        mBinding.proxyUrlEdit.setOnClickListener(null);
        mBinding.proxyUrlEdit.setFirstText(newProxyUrl);

        mBinding.proxyPortEdit.setOnClickListener(null);
        mBinding.proxyPortEdit.setFirstText(newProxyPort);

        System.setProperty(newProxyUrl, newProxyPort);

        mBinding.proxyUrlEdit.setOnClickListener(mProxyUrlListener);
        mBinding.proxyPortEdit.setOnClickListener(mProxyPortListener);
    }

    @Override
    public void onGlobalFocusChanged(View oldFocus, View newFocus) {
        if (oldFocus != null) {
            if (mBinding.proxyUrlEdit.contains(oldFocus) && mBinding.proxyUrlEdit.isEditing()) {
                mBinding.proxyUrlEdit.cancel();
            }

            if (mBinding.proxyPortEdit.contains(oldFocus) && mBinding.proxyPortEdit.isEditing()) {
                mBinding.proxyPortEdit.cancel();
            }
        }
    }

    @Override
    public Point getDimensions() {
        return new Point( WidgetPlacement.dpDimension(getContext(), R.dimen.settings_dialog_width),
                WidgetPlacement.dpDimension(getContext(), R.dimen.display_options_height));
    }

    @Override
    protected SettingViewType getType() {
        return SettingViewType.PROXY;
    }

}
