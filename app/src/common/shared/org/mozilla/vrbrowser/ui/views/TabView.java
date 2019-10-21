package org.mozilla.vrbrowser.ui.views;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.graphics.drawable.RoundedBitmapDrawable;
import androidx.core.graphics.drawable.RoundedBitmapDrawableFactory;
import androidx.recyclerview.widget.RecyclerView;

import org.mozilla.geckoview.GeckoSession;
import org.mozilla.vrbrowser.R;
import org.mozilla.vrbrowser.browser.engine.Session;
import org.mozilla.vrbrowser.ui.widgets.WidgetPlacement;
import org.mozilla.vrbrowser.utils.AnimationHelper;
import org.mozilla.vrbrowser.utils.UrlUtils;

public class TabView extends LinearLayout implements GeckoSession.ContentDelegate, Session.BitmapChangedListener {
    protected RelativeLayout mTabCardView;
    protected RelativeLayout mTabAddView;
    protected View mTabActiveView;
    protected View mTabHoverOverlay;
    protected ImageView mPreview;
    protected TextView mURL;
    protected TextView mTitle;
    protected UIButton mCloseButton;
    protected ImageView mSelectionImage;
    protected ImageView mUnselectImage;
    protected Delegate mDelegate;
    protected Session mSession;
    protected ImageView mTabAddIcon;
    protected boolean mShowAddTab;
    private boolean mSelecting;
    private boolean mActive;
    private int mMinIconPadding;
    private int mMaxIconPadding;
    private boolean mPressed;
    private static final int ICON_ANIMATION_DURATION = 100;

    public interface Delegate {
        void onClose(TabView aSender);
        void onClick(TabView aSender);
        void onAdd(TabView aSender);
    }

    public TabView(Context context) {
        super(context);
    }

    public TabView(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    public TabView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();

        mTabAddView = findViewById(R.id.tabAddView);
        mTabCardView = findViewById(R.id.tabCardView);

        mSelectionImage = findViewById(R.id.tabViewSelected);
        mSelectionImage.setVisibility(View.GONE);

        mUnselectImage = findViewById(R.id.tabViewUnselect);
        mUnselectImage.setVisibility(View.GONE);

        mCloseButton = findViewById(R.id.tabViewCloseButton);
        mCloseButton.setVisibility(View.GONE);
        mCloseButton.setOnClickListener(v -> {
            v.requestFocusFromTouch();
            if (mDelegate != null) {
                mDelegate.onClose(this);
            }
        });
        mCloseButton.setOnHoverListener(mIconHoverListener);

        mPreview = findViewById(R.id.tabViewPreview);
        mURL = findViewById(R.id.tabViewUrl);
        mTitle = findViewById(R.id.tabViewTitle);
        mTitle.setVisibility(View.GONE);
        mTabAddIcon = findViewById(R.id.tabAddIcon);
        mTabActiveView = findViewById(R.id.tabViewActive);
        mTabHoverOverlay = findViewById(R.id.tabHoverOverlay);

        mMinIconPadding = WidgetPlacement.pixelDimension(getContext(), R.dimen.tabs_icon_padding_min);
        mMaxIconPadding = WidgetPlacement.pixelDimension(getContext(), R.dimen.tabs_icon_padding_max);

        this.setOnClickListener(mCardClickListener);
    }

    private OnClickListener mCardClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            if (mDelegate != null) {
                if (!mShowAddTab && mSession != null) {
                    mDelegate.onClick(TabView.this);
                } else {
                    mDelegate.onAdd(TabView.this);
                }
            }
        }
    };

    public void attachToSession(@Nullable Session aSession) {
        if (mSession != null) {
            mSession.removeContentListener(this);
            mSession.removeBitmapChangedListener(this);
        }
        setAddTabMode(false);
        mSession = aSession;
        mSession.addContentListener(this);
        mSession.addBitmapChangedListener(this);
        mShowAddTab = false;
        Bitmap bitmap = aSession.getBitmap();
        if (bitmap != null) {
            mPreview.setImageBitmap(bitmap);
        } else {
            mPreview.setImageResource(R.drawable.ic_icon_tabs_placeholder);
        }

        mURL.setText(UrlUtils.stripProtocol(aSession.getCurrentUri()));
        if (!mShowAddTab) {
            if (mSession.getCurrentUri().equals(mSession.getHomeUri())) {
                mTitle.setText(getResources().getString(R.string.url_home_title, getResources().getString(R.string.app_name)));
            } else {
                mTitle.setText(aSession.getCurrentTitle());
            }
        }
    }

    public Session getSession() {
        return mSession;
    }

    public void setDelegate(Delegate aDelegate) {
        mDelegate = aDelegate;
    }

    public void setAddTabMode(boolean aShow) {
        if (mShowAddTab == aShow) {
            return;
        }
        mShowAddTab = aShow;
        if (mShowAddTab) {
            mTabCardView.setVisibility(View.GONE);
            mTabAddView.setVisibility(View.VISIBLE);
        } else {
            mTabCardView.setVisibility(View.VISIBLE);
            mTabAddView.setVisibility(View.GONE);
        }
    }

    public void setSelecting(boolean aSelecting) {
        mSelecting = aSelecting;
        if (!mSelecting) {
            setSelected(false);
        }
        if (mShowAddTab) {
            setOnClickListener(mSelecting ? null : mCardClickListener);
        }
    }

    public void setActive(boolean aActive) {
        if (mActive != aActive) {
            mActive = aActive;
            updateState();
        }
    }

    @Override
    public void setSelected(boolean selected) {
        super.setSelected(selected);
        updateState();
        if (selected) {
            mSelectionImage.setVisibility(View.VISIBLE);
            mUnselectImage.setVisibility(View.GONE);
        }
    }

    @Override
    public void onHoverChanged(boolean aHovered) {
        super.onHoverChanged(aHovered);
        updateState();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        boolean result = super.onTouchEvent(event);
        if (event.getAction() == MotionEvent.ACTION_DOWN) {
            mPressed = true;
            updateState();
        } else if (event.getAction() == MotionEvent.ACTION_UP || event.getAction() == MotionEvent.ACTION_CANCEL) {
            mPressed = false;
            updateState();
        }
        return result;
    }

    private void updateState() {
        boolean hovered = isHovered() || mCloseButton.isHovered();
        boolean interacted = hovered || mPressed;
        boolean selected = isSelected();

        mCloseButton.setVisibility(interacted && !selected && !mSelecting ? View.VISIBLE : View.GONE);
        mTitle.setVisibility(interacted && !selected ? View.VISIBLE : View.GONE);
        mTabHoverOverlay.setVisibility((interacted || selected) ? View.VISIBLE : View.GONE);
        mTabHoverOverlay.setHovered(hovered || selected);
        mTabHoverOverlay.setPressed(mPressed);
        mTabHoverOverlay.setBackgroundResource(mSelecting && !selected ? R.drawable.tab_overlay_selected : R.drawable.tab_overlay);

        mSelectionImage.setVisibility(selected && !interacted ? View.VISIBLE : View.GONE);
        mUnselectImage.setVisibility(selected && interacted ? View.VISIBLE : View.GONE);
        if (mShowAddTab) {
            mTabAddView.setHovered(hovered && !mSelecting);
            mTabAddView.setPressed(mPressed && !mSelecting);
            mTabAddIcon.setHovered(mTabAddView.isHovered());
            mTabAddIcon.setPressed(mTabAddView.isPressed());
            if (mTabAddIcon.isHovered() && mTabAddIcon.getPaddingLeft() != mMinIconPadding) {
                AnimationHelper.animateViewPadding(mTabAddIcon, mTabAddIcon.getPaddingLeft(), mMinIconPadding, ICON_ANIMATION_DURATION);
            } else if (!mTabAddIcon.isHovered() && mTabAddIcon.getPaddingLeft() != mMaxIconPadding) {
                AnimationHelper.animateViewPadding(mTabAddIcon, mTabAddIcon.getPaddingLeft(), mMaxIconPadding, ICON_ANIMATION_DURATION);
            }
        }
        mTabActiveView.setVisibility(mActive && !selected ? View.VISIBLE : View.GONE);
    }

    @Override
    public void onTitleChange(@NonNull GeckoSession session, @Nullable String title) {
        if (!mShowAddTab) {
            mTitle.setText(title);
        }
    }

    @Override
    public void onBitmapChanged(Bitmap aBitmap) {
        if (aBitmap != null) {
            mPreview.setImageBitmap(aBitmap);
        } else {
            mPreview.setImageResource(R.drawable.ic_icon_tabs_placeholder);
        }
    }

    private View.OnHoverListener mIconHoverListener = (view, motionEvent) -> {
        int ev = motionEvent.getActionMasked();
        switch (ev) {
            case MotionEvent.ACTION_HOVER_ENTER:
                AnimationHelper.animateViewPadding(view,
                        mMaxIconPadding,
                        mMinIconPadding,
                        ICON_ANIMATION_DURATION);
                return false;

            case MotionEvent.ACTION_HOVER_EXIT:
                AnimationHelper.animateViewPadding(view,
                        mMinIconPadding,
                        mMaxIconPadding,
                        ICON_ANIMATION_DURATION,
                        null);
                return false;
        }

        return false;
    };
}
