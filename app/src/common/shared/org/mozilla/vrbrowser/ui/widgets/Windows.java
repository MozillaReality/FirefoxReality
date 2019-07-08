package org.mozilla.vrbrowser.ui.widgets;

import android.content.Context;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.reflect.TypeToken;

import org.mozilla.geckoview.GeckoSession;
import org.mozilla.vrbrowser.R;
import org.mozilla.vrbrowser.browser.SettingsStore;
import org.mozilla.vrbrowser.browser.engine.SessionStore;
import org.mozilla.vrbrowser.utils.InternalPages;

import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Reader;
import java.io.Writer;
import java.lang.reflect.Type;
import java.util.ArrayList;

public class Windows implements TrayListener, TopBarWidget.Delegate {

    private static final String WINDOWS_SAVE_FILENAME = "windows_state.json";

    class WindowState {
        WindowPlacement placement;
        SessionStore sessionStore;
        int currentSessionId;
        int textureWidth;
        int textureHeight;
        float worldWidth;

        public void load(WindowWidget aWindow) {
            placement = aWindow.getWindowPlacement();
            sessionStore = aWindow.getSessionStore();
            currentSessionId = aWindow.getSessionStore().getCurrentSessionId();
            textureWidth = aWindow.getPlacement().width;
            textureHeight = aWindow.getPlacement().height;
            worldWidth = aWindow.getPlacement().worldWidth;
        }
    }

    class WindowsState {
        WindowPlacement focusedWindowPlacement = WindowPlacement.FRONT;
        ArrayList<WindowState> regularWindowsState = new ArrayList<>();
        ArrayList<WindowState> privateWindowsState = new ArrayList<>();
        boolean privateMode = false;
    }

    private Context mContext;
    private WidgetManagerDelegate mWidgetManager;
    private Delegate mDelegate;
    private ArrayList<WindowWidget> mRegularWindows;
    private ArrayList<WindowWidget> mPrivateWindows;
    private WindowWidget mFocusedWindow;
    private static int sIndex;
    private boolean mPrivateMode = false;
    private static final int MAX_WINDOWS = 3;

    enum WindowPlacement{
        FRONT(0),
        LEFT(1),
        RIGHT(2);

        private final int value;

        WindowPlacement(final int aValue) {
            value = aValue;
        }

        public int getValue() { return value; }
    }

    public interface Delegate {
        void onFocusedWindowChanged(@NonNull WindowWidget aFocusedWindow, @Nullable WindowWidget aPrevFocusedWindow);
    }

    public Windows(Context aContext) {
        mContext = aContext;
        mWidgetManager = (WidgetManagerDelegate) aContext;
        mRegularWindows = new ArrayList<>();
        mPrivateWindows = new ArrayList<>();

        restoreWindows();
    }

    private void saveState() {
        File file = new File(mContext.getFilesDir(), WINDOWS_SAVE_FILENAME);
        try (Writer writer = new FileWriter(file)) {
            WindowsState state = new WindowsState();
            state.privateMode = mPrivateMode;
            state.focusedWindowPlacement = mFocusedWindow.getWindowPlacement();
            state.regularWindowsState = new ArrayList<>();
            for (WindowWidget window : mRegularWindows) {
                WindowState windowState = new WindowState();
                windowState.load(window);
                state.regularWindowsState.add(windowState);
            }
            for (WindowWidget window : mPrivateWindows) {
                WindowState windowState = new WindowState();
                windowState.load(window);
                state.privateWindowsState.add(windowState);
            }
            Gson gson = new GsonBuilder().setPrettyPrinting().create();
            gson.toJson(state, writer);

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private WindowsState restoreState() {
        WindowsState restored = null;

        File file = new File(mContext.getFilesDir(), WINDOWS_SAVE_FILENAME);
        try (Reader reader = new FileReader(file)) {
            Gson gson = new GsonBuilder().create();
            Type type = new TypeToken<WindowsState>() {}.getType();
            restored = gson.fromJson(reader, type);

        } catch (IOException e) {
            Log.w(getClass().getCanonicalName(), "Error restoring persistent windows state", e);

        } finally {
            boolean deleted = file.delete();
            if (deleted)
                Log.d(getClass().getCanonicalName(), "Persistent windows state successfully restored");
            else
                Log.d(getClass().getCanonicalName(), "Persistent window state couldn't be deleted");
        }

        return restored;
    }

    public void setDelegate(Delegate aDelegate) {
        mDelegate = aDelegate;
    }

    public WindowWidget getFocusedWindow() {
        return mFocusedWindow;
    }

    public WindowWidget addWindow() {
        if (getCurrentWindows().size() >= MAX_WINDOWS) {
            showMaxWindowsMessage();
            return null;
        }

        WindowWidget frontWindow = getFrontWindow();
        WindowWidget leftWindow = getLeftWindow();
        WindowWidget rightWindow = getRightWindow();

        WindowWidget newWindow = createWindow();
        WindowWidget focusedWindow = getFocusedWindow();

        if (frontWindow == null) {
            // First window
            placeWindow(newWindow, WindowPlacement.FRONT);
        } else if (leftWindow == null && rightWindow == null) {
            // Opening a new window from one window
            placeWindow(newWindow, WindowPlacement.FRONT);
            placeWindow(frontWindow, WindowPlacement.LEFT);
        } else if (leftWindow != null && focusedWindow == leftWindow) {
            // Opening a new window from left window
            placeWindow(newWindow, WindowPlacement.FRONT);
            placeWindow(frontWindow, WindowPlacement.RIGHT);
        } else if (leftWindow != null && focusedWindow == frontWindow) {
            // Opening a new window from front window
            placeWindow(newWindow, WindowPlacement.RIGHT);
        } else if (rightWindow != null && focusedWindow == rightWindow) {
            // Opening a new window from right window
            placeWindow(newWindow, WindowPlacement.FRONT);
            placeWindow(frontWindow, WindowPlacement.LEFT);
        } else if (rightWindow != null && focusedWindow == frontWindow) {
            // Opening a new window from right window
            placeWindow(newWindow, WindowPlacement.LEFT);
        }

        updateMaxWindowScales();
        mWidgetManager.addWidget(newWindow);
        focusWindow(newWindow);
        updateViews();
        return newWindow;
    }

    private WindowWidget addWindow(WindowState aState) {
        if (getCurrentWindows().size() >= MAX_WINDOWS) {
            showMaxWindowsMessage();
            return null;
        }

        WindowWidget newWindow = createWindow();
        newWindow.getPlacement().width = aState.textureWidth;
        newWindow.getPlacement().height = aState.textureHeight;
        newWindow.getPlacement().worldWidth = aState.worldWidth;
        placeWindow(newWindow, aState.placement);

        mWidgetManager.addWidget(newWindow);
        return newWindow;
    }

    public void closeWindow(@NonNull WindowWidget aWindow) {
        WindowWidget frontWindow = getFrontWindow();
        WindowWidget leftWindow = getLeftWindow();
        WindowWidget rightWindow = getRightWindow();

        if (leftWindow == aWindow) {
            removeWindow(leftWindow);
            if (mFocusedWindow == leftWindow) {
                focusWindow(frontWindow);
            }
        } else if (rightWindow == aWindow) {
            removeWindow(rightWindow);
            if (mFocusedWindow == rightWindow) {
                focusWindow(frontWindow);
            }
        } else if (frontWindow == aWindow) {
            removeWindow(frontWindow);
            if (rightWindow != null) {
                placeWindow(rightWindow, WindowPlacement.FRONT);
            } else if (leftWindow != null) {
                placeWindow(leftWindow, WindowPlacement.FRONT);
            }

            if (mFocusedWindow == frontWindow && !getCurrentWindows().isEmpty()) {
                focusWindow(getFrontWindow());
            }

        }

        boolean empty = getCurrentWindows().isEmpty();
        if (empty && isInPrivateMode()) {
            // Exit private mode if the only window is closed.
            exitPrivateMode();
        } else if (empty) {
            // Ensure that there is at least one window.
            addWindow();
        }

        updateViews();
    }

    public void moveWindowRight(@NonNull WindowWidget aWindow) {
        WindowWidget frontWindow = getFrontWindow();
        WindowWidget leftWindow = getLeftWindow();
        WindowWidget rightWindow = getRightWindow();

        if (aWindow == leftWindow) {
            placeWindow(leftWindow, WindowPlacement.FRONT);
            placeWindow(frontWindow, WindowPlacement.LEFT);
        } else if (aWindow == frontWindow) {
            if (rightWindow != null) {
                placeWindow(rightWindow, WindowPlacement.FRONT);
            } else if (leftWindow != null) {
                placeWindow(leftWindow, WindowPlacement.FRONT);
            }
            placeWindow(frontWindow, WindowPlacement.RIGHT);
        }
        updateViews();
    }

    public void moveWindowLeft(@NonNull WindowWidget aWindow) {
        WindowWidget frontWindow = getFrontWindow();
        WindowWidget leftWindow = getLeftWindow();
        WindowWidget rightWindow = getRightWindow();

        if (aWindow == rightWindow) {
            placeWindow(rightWindow, WindowPlacement.FRONT);
            placeWindow(frontWindow, WindowPlacement.RIGHT);
        } else if (aWindow == frontWindow) {
            if (leftWindow != null) {
                placeWindow(leftWindow, WindowPlacement.FRONT);
            } else if (rightWindow != null) {
                placeWindow(rightWindow, WindowPlacement.FRONT);
            }
            placeWindow(frontWindow, WindowPlacement.LEFT);
        }
        updateViews();
    }

    public void focusWindow(@NonNull WindowWidget aWindow) {
        if (aWindow != mFocusedWindow) {
            WindowWidget prev = mFocusedWindow;
            mFocusedWindow = aWindow;
            mFocusedWindow.setActiveWindow();
            if (mDelegate != null) {
                mDelegate.onFocusedWindowChanged(mFocusedWindow, prev);
            }
        }
    }

    public void pauseCompositor() {
        for (WindowWidget window: mRegularWindows) {
            window.pauseCompositor();
        }
        for (WindowWidget window: mPrivateWindows) {
            window.pauseCompositor();
        }
    }

    public void resumeCompositor() {
        for (WindowWidget window: mRegularWindows) {
            window.resumeCompositor();
        }
        for (WindowWidget window: mPrivateWindows) {
            window.resumeCompositor();
        }
    }

    public void onDestroy() {
        saveState();

        for (WindowWidget window: mRegularWindows) {
            window.close();
        }
        for (WindowWidget window: mPrivateWindows) {
            window.close();
        }
    }

    public boolean isInPrivateMode() {
        return mPrivateMode;
    }


    public void enterPrivateMode() {
        if (mPrivateMode) {
            return;
        }
        mPrivateMode = true;
        for (WindowWidget window: mRegularWindows) {
            setWindowVisible(window, false);
        }
        for (WindowWidget window: mPrivateWindows) {
            setWindowVisible(window, true);
        }

        if (mPrivateWindows.size() == 0) {
            addWindow();
        } else {
            focusWindow(getFrontWindow());
        }
        mWidgetManager.pushWorldBrightness(this, WidgetManagerDelegate.DEFAULT_DIM_BRIGHTNESS);
    }

    public void exitPrivateMode() {
        if (!mPrivateMode) {
            return;
        }
        mPrivateMode = false;
        for (WindowWidget window: mRegularWindows) {
            setWindowVisible(window, true);
        }
        for (WindowWidget window: mPrivateWindows) {
            setWindowVisible(window, false);
        }
        focusWindow(getFrontWindow());
        mWidgetManager.popWorldBrightness(this);
    }

    public boolean handleBack() {
        if (mFocusedWindow == null) {
            return false;
        }
        if (mFocusedWindow.getSessionStore().canGoBack()) {
            mFocusedWindow.getSessionStore().goBack();
            return true;
        } else if (isInPrivateMode()) {
            exitPrivateMode();
            return true;
        }

        return false;
    }

    private void updateMaxWindowScales() {
        float maxScale = 3;
        if (getCurrentWindows().size() >= 3) {
            maxScale = 1.5f;
        } else if (getCurrentWindows().size() == 2) {
            maxScale = 2.0f;
        }

        for (WindowWidget window: getCurrentWindows()) {
            window.setMaxWindowScale(maxScale);
        }
    }

    private void showMaxWindowsMessage() {
        mFocusedWindow.showAlert("", mContext.getString(R.string.max_windows_message, String.valueOf(MAX_WINDOWS)), new GeckoSession.PromptDelegate.AlertCallback() {
            @Override
            public void dismiss() {

            }
        });
    }

    private ArrayList<WindowWidget> getCurrentWindows() {
        return mPrivateMode ? mPrivateWindows : mRegularWindows;
    }

    private WindowWidget getWindowWithPlacement(WindowPlacement aPlacement) {
        for (WindowWidget window: getCurrentWindows()) {
            if (window.getWindowPlacement() == aPlacement) {
                return window;
            }
        }
        return null;
    }

    private WindowWidget getFrontWindow() {
        return getWindowWithPlacement(WindowPlacement.FRONT);
    }

    private WindowWidget getLeftWindow() {
        return getWindowWithPlacement(WindowPlacement.LEFT);
    }

    private WindowWidget getRightWindow() {
        return getWindowWithPlacement(WindowPlacement.RIGHT);
    }

    private void restoreWindows() {
        WindowsState windowsState = restoreState();
        if (windowsState != null) {
            for (WindowState windowState : windowsState.regularWindowsState) {
                WindowWidget window = addWindow(windowState);
                window.getSessionStore().restore(windowState.sessionStore, windowState.currentSessionId);
            }
            mPrivateMode = true;
            for (WindowState windowState : windowsState.privateWindowsState) {
                WindowWidget window = addWindow(windowState);
                window.getSessionStore().restore(windowState.sessionStore, windowState.currentSessionId);
            }
            mPrivateMode = false;
            if (windowsState.privateMode) {
                enterPrivateMode();
            }

            WindowWidget windowToFocus = getWindowWithPlacement(windowsState.focusedWindowPlacement);
            if (windowToFocus == null) {
                windowToFocus = getFrontWindow();
                if (windowToFocus == null && getCurrentWindows().size() > 0) {
                    windowToFocus = getCurrentWindows().get(0);
                }
            }
            if (windowToFocus != null) {
                focusWindow(windowToFocus);
            }

        }
        if (getCurrentWindows().size() == 0) {
            WindowWidget window = addWindow();
            focusWindow(window);
        }
        updateMaxWindowScales();
        updateViews();
    }

    private void removeWindow(@NonNull WindowWidget aWindow) {
        mWidgetManager.removeWidget(aWindow);
        mRegularWindows.remove(aWindow);
        mPrivateWindows.remove(aWindow);
        aWindow.getTopBar().setVisible(false);
        aWindow.getTopBar().setDelegate((TopBarWidget.Delegate) null);
        aWindow.close();
        updateMaxWindowScales();
    }

    private void setWindowVisible(WindowWidget aWindow, boolean aVisible) {
        aWindow.setVisible(aVisible);
        aWindow.getTopBar().setVisible(aVisible);
    }

    private void placeWindow(WindowWidget aWindow, WindowPlacement aPosition) {
        WidgetPlacement placement = aWindow.getPlacement();
        aWindow.setWindowPlacement(aPosition);
        switch (aPosition) {
            case FRONT:
                placement.anchorX = 0.5f;
                placement.anchorY = 0.0f;
                placement.rotation = 0;
                placement.rotationAxisX = 0;
                placement.rotationAxisY = 0;
                placement.rotationAxisZ = 0;
                placement.translationX = 0.0f;
                placement.translationY = WidgetPlacement.unitFromMeters(mContext, R.dimen.window_world_y);
                placement.translationZ = WidgetPlacement.unitFromMeters(mContext, R.dimen.window_world_z);
                break;
            case LEFT:
                placement.anchorX = 1.0f;
                placement.anchorY = 0.0f;
                placement.parentAnchorX = 0.0f;
                placement.parentAnchorY = 0.0f;
                placement.rotationAxisY = 1.0f;
                placement.rotation = (float) Math.toRadians(WidgetPlacement.floatDimension(mContext, R.dimen.multi_window_angle));
                placement.translationX = -WidgetPlacement.dpDimension(mContext, R.dimen.multi_window_padding);
                placement.translationY = 0.0f;
                placement.translationZ = 0.0f;
                break;
            case RIGHT:
                placement.anchorX = 0.0f;
                placement.anchorY = 0.0f;
                placement.parentAnchorX = 1.0f;
                placement.parentAnchorY = 0.0f;
                placement.rotationAxisY = 1.0f;
                placement.rotation = (float) Math.toRadians(-WidgetPlacement.floatDimension(mContext, R.dimen.multi_window_angle));
                placement.translationX = WidgetPlacement.dpDimension(mContext, R.dimen.multi_window_padding);
                placement.translationY = 0.0f;
                placement.translationZ = 0.0f;
        }
    }

    private void updateViews() {
        WindowWidget frontWindow = getFrontWindow();
        WindowWidget leftWindow = getLeftWindow();
        WindowWidget rightWindow = getRightWindow();
        // Make sure that left or right window have the correct parent
        if (frontWindow != null && leftWindow != null) {
            leftWindow.getPlacement().parentHandle = frontWindow.getHandle();
        }
        if (frontWindow != null &&  rightWindow != null) {
            rightWindow.getPlacement().parentHandle = frontWindow.getHandle();
        }
        if (frontWindow != null) {
            frontWindow.getPlacement().parentHandle = -1;
        }

        updateTopBars();
        ArrayList<WindowWidget> windows = getCurrentWindows();
        // Sort windows so frontWindow is the first one. Required for proper native matrix updates.
        windows.sort((o1, o2) -> o1 == frontWindow ? -1 : 0);
        for (WindowWidget window: getCurrentWindows()) {
            mWidgetManager.updateWidget(window);
            mWidgetManager.updateWidget(window.getTopBar());
            window.switchBookmarks();
            window.switchBookmarks();
        }
    }

    private void updateTopBars() {
        ArrayList<WindowWidget> windows = getCurrentWindows();
        WindowWidget leftWindow = getLeftWindow();
        WindowWidget rightWindow = getRightWindow();
        boolean visible = windows.size() > 1 || isInPrivateMode();
        for (WindowWidget window: windows) {
            window.getTopBar().setVisible(visible);
            if (visible) {
                window.getTopBar().setMoveLeftButtonEnabled(window != leftWindow);
                window.getTopBar().setMoveRightButtonEnabled(window != rightWindow);
            }
        }
    }

    private WindowWidget createWindow() {
        int newWindowId = sIndex++;
        WindowWidget window = new WindowWidget(mContext, newWindowId, mPrivateMode);
        if (mPrivateMode) {
            InternalPages.PageResources pageResources = InternalPages.PageResources.create(R.raw.private_mode, R.raw.private_style);
            window.getSessionStore().getCurrentSession().loadData(InternalPages.createAboutPage(mContext, pageResources), "text/html");
        } else {
            window.getSessionStore().loadUri(SettingsStore.getInstance(mContext).getHomepage());
        }
        getCurrentWindows().add(window);
        window.getTopBar().setDelegate(this);

        return window;
    }

    // Tray Listener
    @Override
    public void onBookmarksClicked() {
        mFocusedWindow.switchBookmarks();
    }

    @Override
    public void onPrivateBrowsingClicked() {
        if (mPrivateMode) {
            exitPrivateMode();
        } else {
            enterPrivateMode();
        }
    }

    @Override
    public void onAddWindowClicked() {
        addWindow();
    }

    // TopBarWidget Delegate
    @Override
    public void onCloseClicked(TopBarWidget aWidget) {
        WindowWidget window = aWidget.getAttachedWindow();
        if (window != null) {
            closeWindow(window);
        }
    }

    @Override
    public void onMoveLeftClicked(TopBarWidget aWidget) {
        WindowWidget window = aWidget.getAttachedWindow();
        if (window != null) {
            moveWindowLeft(window);
        }
    }

    @Override
    public void onMoveRightClicked(TopBarWidget aWidget) {
        WindowWidget window = aWidget.getAttachedWindow();
        if (window != null) {
            moveWindowRight(window);
        }
    }

}
