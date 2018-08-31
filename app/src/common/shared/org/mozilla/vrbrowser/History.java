package org.mozilla.vrbrowser;

import android.content.Context;
import android.net.Uri;
import android.util.Base64;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.mozilla.geckoview.GeckoSession;
import org.mozilla.geckoview.GeckoSessionSettings;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.FileWriter;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;

public class History implements GeckoSession.ProgressDelegate {
    private HashMap<String, Item> mItems;
    private static final int MAX_ITEMS = 30;
    private static final String LOGTAG = "VRB";
    private static final String FILENAME = "history.json";

    private Context mContext;

    public static class Item {
        String url;
        String title;
        long timestamp;

        Item() {
            timestamp = System.currentTimeMillis();
        }

        Item(JSONObject aObject) {
            url = aObject.optString("u", "");
            title = aObject.optString("n", "");
            timestamp = aObject.optLong("t", System.currentTimeMillis());
        }

        JSONObject toJSON() {
            JSONObject obj = new JSONObject();
            try {
                obj.put("u", url);
                obj.put("n", title);
                obj.put("t", timestamp);
            }
            catch (JSONException ex) {
            }
            return obj;
        }
    }

    public History(Context aContext) {
        mContext = aContext;
        mItems = new HashMap<>();
        SessionStore.get().addProgressListener(this);
        restore();
    }

    public void updateContext(Context aContext) {
        mContext = aContext;
    }

    public void release() {
        SessionStore.get().removeProgressListener(this);
    }

    public String encodedJSON() {
        String value = getJSONString();
        String base64 = Base64.encodeToString(value.getBytes(), 0);
        return Uri.encode(base64);
    }

    public void clear() {
        mItems.clear();
        save();
    }

    private void save() {
        JSONArray array = new JSONArray();
        for (Item item: mItems.values()) {
            array.put(item.toJSON());
        }

        try (FileWriter file = new FileWriter(mContext.getFilesDir().getAbsolutePath() + "/" + FILENAME)) {
            file.write(getJSONString());
        }
        catch (Exception ex) {
            Log.e(LOGTAG, "Error saving history", ex);
        }
    }

    private void restore() {
        try (BufferedReader reader = new BufferedReader(new FileReader(mContext.getFilesDir().getAbsolutePath() + "/" + FILENAME))) {
            StringBuilder sb = new StringBuilder();
            String line = reader.readLine();

            while (line != null) {
                sb.append(line);
                sb.append("\n");
                line = reader.readLine();
            }
            try {
                JSONArray array = new JSONArray(sb.toString());
                mItems.clear();
                for (int i = 0; i < array.length(); ++i) {
                    Item item = new Item(array.getJSONObject(i));
                    if (item.url != null && item.url.length() > 0) {
                        mItems.put(key(item.url), item);
                    }
                }

            } catch (JSONException e) {
                Log.e(LOGTAG, "Error parsing history", e);
            }
        }
        catch (Exception ex) {
            Log.e(LOGTAG, "Error reading history", ex);
        }
    }

    private String getJSONString() {
        JSONArray array = new JSONArray();
        for (Item item: mItems.values()) {
            array.put(item.toJSON());
        }
        return array.toString();
    }

    private String key(String aUri) {
        String result = aUri.toLowerCase();
        int index = result.indexOf('#');
        if (index >= 0) {
            result = result.substring(0, index);
        }
        return result;
    }

    private void checkMaxSize() {
        if (mItems.size() <= MAX_ITEMS) {
            return;
        }

        ArrayList<Item> list = new ArrayList<>(mItems.values());
        list.sort(new Comparator<Item>() {
            @Override
            public int compare(Item o1, Item o2) {
                return (int)(o1.timestamp - o2.timestamp);
            }
        });

        for (Item item: list) {
            mItems.remove(key(item.url));
            if (mItems.size() >= MAX_ITEMS) {
                return;
            }
        }
    }

    // GeckoSession.ProgressDelegate

    @Override
    public void onPageStart(GeckoSession geckoSession, String s) {

    }

    @Override
    public void onPageStop(GeckoSession geckoSession, boolean b) {
        String uri = SessionStore.get().getUriFromSession(geckoSession);
        if (uri == null || uri.length() == 0) {
            return;
        }
        if (SessionStore.get().isHomeUri(uri)) {
            return;
        }
        if (geckoSession.getSettings().getBoolean(GeckoSessionSettings.USE_PRIVATE_MODE)) {
            // Do not store private mode history
            return;
        }

        Item item = mItems.get(key(uri));
        if (item == null) {
            item = new Item();
            item.url = uri;
            mItems.put(key(uri), item);
            checkMaxSize();
        }

        item.url = uri;
        item.title = SessionStore.get().getTitleFromSession(geckoSession);
        item.timestamp = System.currentTimeMillis();
        save();
    }

    @Override
    public void onProgressChange(GeckoSession geckoSession, int i) {

    }

    @Override
    public void onSecurityChange(GeckoSession geckoSession, SecurityInformation securityInformation) {

    }


}
