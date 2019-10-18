package org.mozilla.vrbrowser.ui.views;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.util.AttributeSet;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.mozilla.geckoview.GeckoSession;
import org.mozilla.vrbrowser.R;
import org.mozilla.vrbrowser.browser.engine.Session;
import org.mozilla.vrbrowser.utils.UrlUtils;

public class TabView extends LinearLayout implements GeckoSession.ContentDelegate, Session.BitmapChangedListener {
    protected RelativeLayout mTabCardView;
    protected RelativeLayout mTabAddView;
    protected RelativeLayout mTabActiveView;
    protected View mTabHoverOverlay;
    protected ImageView mPreview;
    protected TextView mURL;
    protected TextView mTitle;
    protected UIButton mCloseButton;
    protected ImageView mSelectionImage;
    protected Delegate mDelegate;
    protected Session mSession;
    protected ImageView mTabAddIcon;
    protected boolean mShowAddTab;
    private boolean mSelecting;
    private boolean mActive;

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

        mSelectionImage = findViewById(R.id.tabViewSelection);
        mSelectionImage.setVisibility(View.GONE);

        mCloseButton = findViewById(R.id.tabViewCloseButton);
        mCloseButton.setVisibility(View.GONE);
        mCloseButton.setOnClickListener(v -> {
            v.requestFocusFromTouch();
            if (mDelegate != null) {
                mDelegate.onClose(this);
            }
        });

        mPreview = findViewById(R.id.tabViewPreview);
        mURL = findViewById(R.id.tabViewUrl);
        mTitle = findViewById(R.id.tabViewTitle);
        mTitle.setVisibility(View.GONE);
        mTabAddIcon = findViewById(R.id.tabAddIcon);
        mTabActiveView = findViewById(R.id.tabViewActive);
        mTabHoverOverlay = findViewById(R.id.tabHoverOverlay);

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
            mPreview.setImageDrawable(null);
        }

        mURL.setText(UrlUtils.stripProtocol(aSession.getCurrentUri()));
        if (!mShowAddTab) {
            mTitle.setText(aSession.getCurrentTitle());
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
            updateState(isHovered(), isSelected());
        }
    }

    @Override
    public void setSelected(boolean selected) {
        super.setSelected(selected);
        updateState(isHovered(), selected);
    }

    @Override
    public void onHoverChanged(boolean aHovered) {
        super.onHoverChanged(aHovered);
        boolean hovered = aHovered || mCloseButton.isHovered();
        updateState(hovered, isSelected());

    }

    private void updateState(boolean aHovered, boolean aSelected) {
        mCloseButton.setVisibility(aHovered && !aSelected && !mSelecting ? View.VISIBLE : View.GONE);
        mTitle.setVisibility(aHovered && !aSelected ? View.VISIBLE : View.GONE);
        mTabHoverOverlay.setVisibility((aHovered || aSelected) ? View.VISIBLE : View.GONE);
        mSelectionImage.setVisibility(aSelected ? View.VISIBLE : View.GONE);
        if (mShowAddTab) {
            mTabAddView.setHovered(aHovered && !mSelecting);
            mTabAddIcon.setHovered(aHovered && !mSelecting);
        }
        mTabActiveView.setVisibility(mActive && !aSelected ? View.VISIBLE : View.GONE);
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
        }
    }
}
