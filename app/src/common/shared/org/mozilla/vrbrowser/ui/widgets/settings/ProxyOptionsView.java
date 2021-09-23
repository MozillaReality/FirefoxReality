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
import org.mozilla.vrbrowser.ui.views.settings.SwitchSetting;
import org.mozilla.vrbrowser.ui.widgets.WidgetManagerDelegate;
import org.mozilla.vrbrowser.ui.widgets.WidgetPlacement;

import java.util.Properties;

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
        mBinding.proxySwitch.setOnCheckedChangeListener(mProxySwitchListener);
        setProxySwitch(SettingsStore.getInstance(getContext()).getCylinderDensity() > 0.0f, false);

        mDefaultProxyUrl = getContext().getString(R.string.proxy_url);
        mDefaultProxyPort = getContext().getString(R.string.proxy_port);

        mBinding.proxyUrlEdit.setHint1(SettingsStore.getInstance(getContext()).getProxyUrl());
        mBinding.proxyUrlEdit.setDefaultFirstValue(mDefaultProxyUrl);
        mBinding.proxyUrlEdit.setFirstText(SettingsStore.getInstance(getContext()).getProxyUrl());
        mBinding.proxyUrlEdit.setOnClickListener(mProxyUrlListener);
        setProxyUrl(SettingsStore.getInstance(getContext()).getProxyUrl());

        mBinding.proxyPortEdit.setHint1(SettingsStore.getInstance(getContext()).getProxyPort());
        mBinding.proxyPortEdit.setDefaultFirstValue(mDefaultProxyPort);
        mBinding.proxyPortEdit.setFirstText(SettingsStore.getInstance(getContext()).getProxyPort());
        mBinding.proxyPortEdit.setOnClickListener(mProxyPortListener);
        setProxyPort(SettingsStore.getInstance(getContext()).getProxyPort());
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
            setProxyUrl(mBinding.proxyUrlEdit.getFirstText());

        } else {
            setProxyUrl(mDefaultProxyUrl);
        }
    };

    private OnClickListener mProxyPortListener = (view) -> {
        if (!mBinding.proxyPortEdit.getFirstText().isEmpty()) {
            setProxyPort(mBinding.proxyPortEdit.getFirstText());

        } else {
            setProxyPort(mDefaultProxyPort);
        }
    };

    private SwitchSetting.OnCheckedChangeListener mProxySwitchListener = (compoundButton, enabled, apply) ->
            setProxySwitch(enabled, true);

    private OnClickListener mResetListener = (view) -> {
        boolean restart = false;

        setProxyUrl(mDefaultProxyUrl);
        setProxyPort(mDefaultProxyPort);

        if (restart) {
            showRestartDialog();
        }
    };

    private void setProxySwitch(boolean value, boolean doApply) {
        mBinding.proxySwitch.setOnCheckedChangeListener(null);
        mBinding.proxySwitch.setValue(value, false);
        mBinding.proxySwitch.setOnCheckedChangeListener(mProxySwitchListener);

        // set proxy
        if (doApply) {
            String url = mBinding.proxyUrlEdit.getFirstText();
            String port = mBinding.proxyPortEdit.getFirstText();

            Properties prop = System.getProperties();
            prop.put("proxyHost", url);
            prop.put("proxyPort", port);
            prop.put("proxySet", "true");

        } else {
            Properties prop = System.getProperties();
            prop.put("proxySet", "false");
        }
    }

    private void setProxyUrl(String newProxyUrl) {
        mBinding.proxyUrlEdit.setOnClickListener(null);
        mBinding.proxyUrlEdit.setFirstText(newProxyUrl);
        mBinding.proxyUrlEdit.setHint1(newProxyUrl);
        SettingsStore.getInstance(getContext()).setProxyUrl(newProxyUrl);
        mBinding.proxyUrlEdit.setOnClickListener(mProxyUrlListener);
    }

    private void setProxyPort(String newProxyPort) {
        mBinding.proxyPortEdit.setOnClickListener(null);
        mBinding.proxyPortEdit.setFirstText(newProxyPort);
        mBinding.proxyPortEdit.setHint1(newProxyPort);
        SettingsStore.getInstance(getContext()).setProxyPort(newProxyPort);
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
