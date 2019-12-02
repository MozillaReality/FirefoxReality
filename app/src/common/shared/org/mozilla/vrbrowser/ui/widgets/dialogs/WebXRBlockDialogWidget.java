/* -*- Mode: Java; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.vrbrowser.ui.widgets.dialogs;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.databinding.DataBindingUtil;

import org.mozilla.vrbrowser.R;
import org.mozilla.vrbrowser.databinding.PopupBlockDialogBinding;
import org.mozilla.vrbrowser.databinding.WebxrBlockDialogBinding;

public class WebXRBlockDialogWidget extends BaseAppDialogWidget {
    protected WebxrBlockDialogBinding mWebXRBinding;
    public WebXRBlockDialogWidget(Context aContext) {
        super(aContext);
    }

    @Override
    protected void initialize(Context aContext) {
        super.initialize(aContext);

        LayoutInflater inflater = LayoutInflater.from(aContext);

        // Inflate this data binding layout
        mWebXRBinding = DataBindingUtil.inflate(inflater, R.layout.webxr_block_dialog, mBinding.dialogContent, true);

        setButtons(new int[] {
                R.string.webxr_block_dialog_dont_allow,
                R.string.webxr_block_dialog_allow
        });
        setSeparatorsVisible(false);
        mBinding.headerLayout.setVisibility(View.GONE);
        ViewGroup.LayoutParams params = mBinding.content.getLayoutParams();
        params.height = aContext.getResources().getDimensionPixelSize(R.dimen.webxr_permission_dialog_height);
        mBinding.content.setLayoutParams(params);
    }

    @Override
    public void setDescription(String title) {
        mWebXRBinding.contentDescription.setText(title);
    }
}
