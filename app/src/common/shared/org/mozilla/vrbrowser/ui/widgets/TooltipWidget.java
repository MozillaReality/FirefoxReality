package org.mozilla.vrbrowser.ui.widgets;

import android.content.Context;
import android.graphics.PointF;
import android.graphics.Rect;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.TextView;

import org.mozilla.vrbrowser.R;
import org.mozilla.vrbrowser.utils.ViewUtils;

public class TooltipWidget extends UIWidget {

    private View mTargetView;
    private UIWidget mParentWidget;
    protected TextView mText;
    private PointF mTranslation;
    private float mRatio;
    private int mPaddingH;
    private int mPaddingV;

    public TooltipWidget(Context aContext) {
        super(aContext);

        initialize();
    }

    private void initialize() {
        inflate(getContext(), R.layout.tooltip, this);

        mText = findViewById(R.id.tooltipText);

        ViewGroup layout = findViewById(R.id.layout);
        mPaddingH = layout.getPaddingStart() + layout.getPaddingEnd();
        mPaddingV = layout.getPaddingTop() + layout.getPaddingBottom();
    }

    @Override
    protected void initializeWidgetPlacement(WidgetPlacement aPlacement) {
        aPlacement.visible = false;
        aPlacement.width =  0;
        aPlacement.height = 0;
        aPlacement.parentAnchorX = 0.0f;
        aPlacement.parentAnchorY = 1.0f;
        aPlacement.anchorX = 0.5f;
        aPlacement.anchorY = 0.5f;
        aPlacement.translationZ = WidgetPlacement.unitFromMeters(getContext(), R.dimen.tooltip_z_distance);
    }

    @Override
    public void show(@ShowFlags int aShowFlags) {
        measure(MeasureSpec.makeMeasureSpec(0, MeasureSpec.UNSPECIFIED),
                MeasureSpec.makeMeasureSpec(0, MeasureSpec.UNSPECIFIED));
        mWidgetPlacement.translationX = mTranslation.x * (mRatio / mWidgetPlacement.density);
        mWidgetPlacement.translationY = mTranslation.y * (mRatio / mWidgetPlacement.density);
        mWidgetPlacement.width = (int)((getMeasuredWidth() + mPaddingH)/mWidgetPlacement.density);
        mWidgetPlacement.height = (int)((getMeasuredHeight() + mPaddingV)/mWidgetPlacement.density);
        super.show(aShowFlags);

        ViewTreeObserver viewTreeObserver = getViewTreeObserver();
        if (viewTreeObserver.isAlive()) {
            viewTreeObserver.addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
                @Override
                public void onGlobalLayout() {
                    getViewTreeObserver().removeOnGlobalLayoutListener(this);
                    mWidgetPlacement.width = (int)(getWidth() / mWidgetPlacement.density);
                    mWidgetPlacement.height = (int)(getHeight() / mWidgetPlacement.density);
                    mWidgetManager.updateWidget(TooltipWidget.this);
                }
            });
        }
    }

    public void setLayoutParams(View targetView) {
        this.setLayoutParams(targetView, ViewUtils.TooltipPosition.BOTTOM);
    }

    public void setLayoutParams(View targetView, ViewUtils.TooltipPosition position) {
        this.setLayoutParams(targetView, position, mWidgetPlacement.density);
    }

    public void setLayoutParams(View targetView, ViewUtils.TooltipPosition position, float density) {
        mTargetView = targetView;
        mParentWidget = ViewUtils.getParentWidget(mTargetView);
        if (mParentWidget != null) {
            mRatio = WidgetPlacement.worldToWidgetRatio(mParentWidget);

            Rect offsetViewBounds = new Rect();
            getDrawingRect(offsetViewBounds);
            mParentWidget.offsetDescendantRectToMyCoords(mTargetView, offsetViewBounds);

            mWidgetPlacement.parentHandle = mParentWidget.getHandle();
            // At the moment we only support showing tooltips on top or bottom of the target view
            if (position == ViewUtils.TooltipPosition.BOTTOM) {
                mWidgetPlacement.density = density;
                mWidgetPlacement.anchorY = 1.0f;
                mWidgetPlacement.parentAnchorY = 0.0f;
                float densityRatio = mWidgetPlacement.density / getContext().getResources().getDisplayMetrics().density;
                mTranslation = new PointF(
                        (offsetViewBounds.left + mTargetView.getWidth() / 2) * densityRatio,
                        -offsetViewBounds.top * densityRatio);
            } else {
                mWidgetPlacement.density = density;
                mWidgetPlacement.anchorY = 0.0f;
                mWidgetPlacement.parentAnchorY = 1.0f;
                float densityRatio = mWidgetPlacement.density / getContext().getResources().getDisplayMetrics().density;
                mTranslation = new PointF(
                        (offsetViewBounds.left + mTargetView.getWidth() / 2) * densityRatio,
                        offsetViewBounds.top * densityRatio);
            }
        }
    }

    public void setText(String text) {
        mText.setText(text);
    }

}
