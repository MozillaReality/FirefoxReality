/* -*- Mode: Java; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.vrbrowser.ui.widgets.dialogs;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.databinding.DataBindingUtil;

import org.mozilla.vrbrowser.R;
import org.mozilla.vrbrowser.VRBrowserApplication;
import org.mozilla.vrbrowser.browser.Accounts;
import org.mozilla.vrbrowser.browser.engine.SessionStore;
import org.mozilla.vrbrowser.databinding.SendTabsDisplayBinding;
import org.mozilla.vrbrowser.ui.widgets.WidgetPlacement;
import org.mozilla.vrbrowser.ui.widgets.WindowWidget;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import mozilla.components.concept.sync.Device;
import mozilla.components.concept.sync.DeviceCapability;

public class SendTabDialogWidget extends SettingDialogWidget {

    private SendTabsDisplayBinding mSendTabsDialogBinding;
    private Accounts mAccounts;
    private List<Device> mDevicesList = new ArrayList<>();

    public SendTabDialogWidget(@NonNull Context aContext) {
        super(aContext);
    }

    @Override
    protected void initialize(@NonNull Context aContext) {
        super.initialize(aContext);

        LayoutInflater inflater = LayoutInflater.from(aContext);

        // Inflate this data binding layout
        mSendTabsDialogBinding = DataBindingUtil.inflate(inflater, R.layout.send_tabs_display, mBinding.content, true);

        mAccounts = ((VRBrowserApplication)getContext().getApplicationContext()).getAccounts();

        mBinding.headerLayout.setTitle(getResources().getString(R.string.send_tab_dialog_title));
        mBinding.headerLayout.setDescription(R.string.send_tab_dialog_description);
        mBinding.footerLayout.setFooterButtonText(R.string.send_tab_dialog_button);
        mBinding.footerLayout.setFooterButtonClickListener(v -> {
            Device device = mDevicesList.get(mSendTabsDialogBinding.devicesList.getCheckedRadioButtonId());
            String uri = SessionStore.get().getActiveSession().getCurrentUri();
            String title = SessionStore.get().getActiveSession().getCurrentTitle();
            // At some point we will support sending multiple devices or all of them
            mAccounts.sendTabs(Collections.singletonList(device), uri, title);

            // Dime the window and show the check mark for 2 seconds
            WindowWidget window = mWidgetManager.getFocusedWindow();
            window.setTabSentCheckVisible(true);
            postDelayed(() -> {
                window.setTabSentCheckVisible(false);
            }, 2000);

            onDismiss();
        });

        mWidgetPlacement.width = WidgetPlacement.dpDimension(getContext(), R.dimen.cache_app_dialog_width);
    }

    @Override
    public void show(int aShowFlags) {
        mSendTabsDialogBinding.devicesList.setChecked(0, true);

        // In the future we might be able to narrow down the list filtering by other capabilities
        mDevicesList = mAccounts.devicesByCapability(Collections.singletonList(DeviceCapability.SEND_TAB));

        mSendTabsDialogBinding.setIsEmpty(mDevicesList.isEmpty());
        mBinding.footerLayout.setFooterButtonVisibility(mDevicesList.isEmpty() ? View.GONE : View.VISIBLE);

        List<String> devicesNamesList = new ArrayList<>();
        mDevicesList.forEach((device) -> devicesNamesList.add(device.getDisplayName()));
        mSendTabsDialogBinding.devicesList.setOptions(devicesNamesList.toArray(new String[]{}));

        super.show(aShowFlags);
    }
}
