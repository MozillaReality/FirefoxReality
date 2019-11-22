package org.mozilla.vrbrowser.ui.widgets.dialogs;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;

import org.mozilla.vrbrowser.ui.widgets.UIWidget;
import org.mozilla.vrbrowser.ui.widgets.WidgetManagerDelegate;
import org.mozilla.vrbrowser.utils.ViewUtils;

public abstract class UIDialog extends UIWidget implements WidgetManagerDelegate.FocusChangeListener, WidgetManagerDelegate.WorldClickListener {
    public UIDialog(Context aContext) {
        super(aContext);
        initialize();
    }

    public UIDialog(Context aContext, AttributeSet aAttrs) {
        super(aContext, aAttrs);
        initialize();
    }

    public UIDialog(Context aContext, AttributeSet aAttrs, int aDefStyle) {
        super(aContext, aAttrs, aDefStyle);
        initialize();
    }

    private void initialize() {
        mWidgetManager.addFocusChangeListener(this);
        mWidgetManager.addWorldClickListener(this);
    }

    @Override
    public void releaseWidget() {
        mWidgetManager.removeWorldClickListener(this);
        mWidgetManager.removeFocusChangeListener(this);
        super.releaseWidget();
    }

    @Override
    public boolean isDialog() {
        return true;
    }

    @Override
    public void show(int aShowFlags) {
        super.show(aShowFlags);

        mWidgetManager.pushWorldBrightness(this, WidgetManagerDelegate.DEFAULT_DIM_BRIGHTNESS);
    }

    @Override
    public void hide(int aHideFlags) {
        super.hide(aHideFlags);

        mWidgetManager.popWorldBrightness(this);
    }

    // WidgetManagerDelegate.FocusChangeListener

    @Override
    public void onGlobalFocusChanged(View oldFocus, View newFocus) {
        if (oldFocus == this && isVisible()) {
            onDismiss();
        }
    }

    @Override
    public void onWorldClick() {
        if (this.isVisible()) {
            onDismiss();
        }
    }
}
