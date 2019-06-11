/*
 * Copyright (C) 2008-2012  OMRON SOFTWARE Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package jp.co.omronsoft.openwnn;

import java.util.Iterator;
import java.util.ArrayList;

import android.util.Log;

/**
 * The container class of composing string.
 *
 * This interface is for the class includes information about the
 * input string, the converted string and its decoration.
 * {@link LetterConverter} and {@link WnnEngine} get the input string from it, and
 * store the converted string into it.
 *
 * @author Copyright (C) 2009 OMRON SOFTWARE CO., LTD.  All Rights Reserved.
 */
public class ComposingText {
    /**
     * Text layer 0.
     * <br>
     * This text layer holds key strokes.<br>
     * (ex) Romaji in Japanese.  Parts of Hangul in Korean.
     */
    public static final int LAYER0  = 0;
    /**
     * Text layer 1.
     * <br>
     * This text layer holds the result of the letter converter.<br>
     * (ex) Hiragana in Japanese. Pinyin in Chinese. Hangul in Korean.
     */
    public static final int LAYER1  = 1;
    /**
     * Text layer 2.
     * <br>
     * This text layer holds the result of the consecutive clause converter.<br>
     * (ex) the result of Kana-to-Kanji conversion in Japanese,
     *      Pinyin-to-Kanji conversion in Chinese, Hangul-to-Hanja conversion in Korean language.
     */
    public static final int LAYER2  = 2;
    /** Maximum number of layers */
    public static final int MAX_LAYER = 3;

    /** Composing text's layer data */
    protected ArrayList<StrSegment>[] mStringLayer;
    /** Cursor position */
    protected int[] mCursor;

    /**
     * Constructor
     */
    public ComposingText() {
        mStringLayer = new ArrayList[MAX_LAYER];
        mCursor = new int[MAX_LAYER];
        for (int i = 0; i < MAX_LAYER; i++) {
            mStringLayer[i] = new ArrayList<StrSegment>();
            mCursor[i] = 0;
        }
    }

    /**
     * Output internal information to the log.
     */
    public void debugout() {
        for (int i = 0; i < MAX_LAYER; i++) {
            Log.d("OpenWnn", "ComposingText["+i+"]");
            Log.d("OpenWnn", "  cur = " + mCursor[i]);
            String tmp = "";
            for (Iterator<StrSegment> it = mStringLayer[i].iterator(); it.hasNext();) {
                StrSegment ss = it.next();
                tmp += "(" + ss.string + "," + ss.from + "," + ss.to + ")";
            }
            Log.d("OpenWnn", "  str = "+tmp);
        }
    }

    /**
     * Get a {@link StrSegment} at the position specified.
     *
     * @param layer     Layer
     * @param pos       Position (<0 : the tail segment)
     *
     * @return          The segment; {@code null} if error occurs.
     */
    public StrSegment getStrSegment(int layer, int pos) {
        try {
            ArrayList<StrSegment> strLayer = mStringLayer[layer];
            if (pos < 0) {
                pos = strLayer.size() - 1;
            }
            if (pos >= strLayer.size() || pos < 0) {
                return null;
            }
            return strLayer.get(pos);
        } catch (Exception ex) {
            return null;
        }
    } 

    /**
     * Convert the range of segments to a string.
     *
     * @param layer     Layer
     * @param from      Convert range from
     * @param to        Convert range to
     * @return          The string converted; {@code null} if error occurs.
     */
    public String toString(int layer, int from, int to) {
        try {
            StringBuffer buf = new StringBuffer();
            ArrayList<StrSegment> strLayer = mStringLayer[layer];
            
            for (int i = from; i <= to; i++) {
                StrSegment ss = strLayer.get(i);
                buf.append(ss.string);
            }
            return buf.toString();
        } catch (Exception ex) {
            return null;
        }
    }

    /**
     * Convert segments of the layer to a string.
     *
     * @param layer     Layer
     * @return          The string converted; {@code null} if error occurs.
     */
    public String toString(int layer) {
        return this.toString(layer, 0, mStringLayer[layer].size() - 1);
    }

    /**
     * Update the upper layer's data.
     *
     * @param layer         The base layer
     * @param mod_from      Modified from
     * @param mod_len       Length after modified (# of StrSegments from {@code mod_from})
     * @param org_len       Length before modified (# of StrSegments from {@code mod_from})
     */
    private void modifyUpper(int layer, int mod_from, int mod_len, int org_len) {
        if (layer >= MAX_LAYER - 1) {
            /* no layer above */
            return;
        }

        int uplayer = layer + 1;
        ArrayList<StrSegment> strUplayer = mStringLayer[uplayer];
        if (strUplayer.size() <= 0) {
            /* 
             * if there is no element on above layer,
             * add a element includes whole elements of the lower layer.
             */
            strUplayer.add(new StrSegment(toString(layer), 0, mStringLayer[layer].size() - 1));
            modifyUpper(uplayer, 0, 1, 0);
            return;
        }

        int mod_to = mod_from + ((mod_len == 0)? 0 : (mod_len - 1));
        int org_to = mod_from + ((org_len == 0)? 0 : (org_len - 1));
        StrSegment last = strUplayer.get(strUplayer.size() - 1);
        if (last.to < mod_from) {
            /* add at the tail */
            last.to = mod_to;
            last.string = toString(layer, last.from, last.to);
            modifyUpper(uplayer, strUplayer.size()-1, 1, 1);
            return;
        }

        int uplayer_mod_from = -1;
        int uplayer_org_to = -1;
        for (int i = 0; i < strUplayer.size(); i++) {
            StrSegment ss = strUplayer.get(i);
            if (ss.from > mod_from) {
                if (ss.to <= org_to) {
                    /* the segment is included */
                    if (uplayer_mod_from < 0) {
                        uplayer_mod_from = i;
                    }
                    uplayer_org_to = i;
                } else {
                    /* included in this segment */
                    uplayer_org_to = i;
                    break;
                }
            } else {
                if (org_len == 0 && ss.from == mod_from) {
                    /* when an element is added */
                    uplayer_mod_from = i - 1;
                    uplayer_org_to   = i - 1;
                    break;
                } else {
                    /* start from this segment */
                    uplayer_mod_from = i;
                    uplayer_org_to = i;
                    if (ss.to >= org_to) {
                        break;
                    }
                }
            }
        }
        
        int diff = mod_len - org_len;
        if (uplayer_mod_from >= 0) {
            /* update an element */
            StrSegment ss = strUplayer.get(uplayer_mod_from);
            int last_to = ss.to;
            int next = uplayer_mod_from + 1;
            for (int i = next; i <= uplayer_org_to; i++) {
                ss = strUplayer.get(next);
                if (last_to > ss.to) {
                    last_to = ss.to;
                }
                strUplayer.remove(next);
            }
            ss.to = (last_to < mod_to)? mod_to : (last_to + diff);
            
            ss.string = toString(layer, ss.from, ss.to);
            
            for (int i = next; i < strUplayer.size(); i++) {
                ss = strUplayer.get(i);
                ss.from += diff;
                ss.to   += diff;
            }
            
            modifyUpper(uplayer, uplayer_mod_from, 1, uplayer_org_to - uplayer_mod_from + 1);
        } else {
            /* add an element at the head */
            StrSegment ss = new StrSegment(toString(layer, mod_from, mod_to),
                                           mod_from, mod_to); 
            strUplayer.add(0, ss);
            for (int i = 1; i < strUplayer.size(); i++) {
                ss = strUplayer.get(i);
                ss.from += diff;
                ss.to   += diff;
            }
            modifyUpper(uplayer, 0, 1, 0);
        }
        
        return;
    }

    /**
     * Insert a {@link StrSegment} at the cursor position.
     * 
     * @param layer Layer to insert
     * @param str   String
     **/
    public void insertStrSegment(int layer, StrSegment str) {
        int cursor = mCursor[layer];
        mStringLayer[layer].add(cursor, str);
        modifyUpper(layer, cursor, 1, 0);
        setCursor(layer, cursor + 1);
    }
    
    /**
     * Insert a {@link StrSegment} at the cursor position(without merging to the previous segment).
     * <p>
     * @param layer1        Layer to insert
     * @param layer2        Never merge to the previous segment from {@code layer1} to {@code layer2}.
     * @param str           String
     **/
    public void insertStrSegment(int layer1, int layer2, StrSegment str) {
        mStringLayer[layer1].add(mCursor[layer1], str);
        mCursor[layer1]++;
        
        for (int i = layer1 + 1; i <= layer2; i++) {
            int pos = mCursor[i-1] - 1;
            StrSegment tmp = new StrSegment(str.string, pos, pos);
            ArrayList<StrSegment> strLayer = mStringLayer[i];
            strLayer.add(mCursor[i], tmp);
            mCursor[i]++;
            for (int j = mCursor[i]; j < strLayer.size(); j++) {
                StrSegment ss = strLayer.get(j);
                ss.from++;
                ss.to++;
            }
        }
        int cursor = mCursor[layer2];
        modifyUpper(layer2, cursor - 1, 1, 0);
        setCursor(layer2, cursor);
    }
    
    /**
     * Replace segments at the range specified.
     *
     * @param layer     Layer
     * @param str       String segment array to replace
     * @param from      Replace from
     * @param to        Replace to
     **/
    protected void replaceStrSegment0(int layer, StrSegment[] str, int from, int to) {
        ArrayList<StrSegment> strLayer = mStringLayer[layer];

        if (from < 0 || from > strLayer.size()) {
            from = strLayer.size();
        }
        if (to < 0 || to > strLayer.size()) {
            to = strLayer.size();
        }
        for (int i = from; i <= to; i++) {
            strLayer.remove(from);
        }
        for (int i = str.length - 1; i >= 0; i--) {
            strLayer.add(from, str[i]);
        }
        
        modifyUpper(layer, from, str.length, to - from + 1);
    }
    
    /**
     * Replace segments at the range specified.
     *
     * @param layer     Layer
     * @param str       String segment array to replace
     * @param num       Size of string segment array
     **/
    public void replaceStrSegment(int layer, StrSegment[] str, int num) {
        int cursor = mCursor[layer];
        replaceStrSegment0(layer, str, cursor - num, cursor - 1);
        setCursor(layer, cursor + str.length - num);
    }
    
    /**
     * Replace the segment at the cursor.
     *
     * @param layer     Layer
     * @param str       String segment to replace
     **/
    public void replaceStrSegment(int layer, StrSegment[] str) {
        int cursor = mCursor[layer];
        replaceStrSegment0(layer, str, cursor - 1, cursor - 1);
        setCursor(layer, cursor + str.length - 1);
    }

    /**
     * Delete segments.
     * 
     * @param layer Layer
     * @param from  Delete from
     * @param to    Delete to
     **/
    public void deleteStrSegment(int layer, int from, int to) {
        int[] fromL = new int[] {-1, -1, -1};
        int[] toL   = new int[] {-1, -1, -1};

        ArrayList<StrSegment> strLayer2 = mStringLayer[2];
        ArrayList<StrSegment> strLayer1 = mStringLayer[1];
        
        if (layer == 2) {
            fromL[2] = from;
            toL[2]   = to;
            fromL[1] = strLayer2.get(from).from;
            toL[1]   = strLayer2.get(to).to;
            fromL[0] = strLayer1.get(fromL[1]).from;
            toL[0]   = strLayer1.get(toL[1]).to;
        } else if (layer == 1) {
            fromL[1] = from;
            toL[1]   = to;
            fromL[0] = strLayer1.get(from).from;
            toL[0]   = strLayer1.get(to).to;
        } else {
            fromL[0] = from;
            toL[0]   = to;
        }

        int diff = to - from + 1;
        for (int lv = 0; lv < MAX_LAYER; lv++) {
            if (fromL[lv] >= 0) {
                deleteStrSegment0(lv, fromL[lv], toL[lv], diff);
            } else {
                int boundary_from = -1;
                int boundary_to   = -1;
                ArrayList<StrSegment> strLayer = mStringLayer[lv];
                for (int i = 0; i < strLayer.size(); i++) {
                    StrSegment ss = strLayer.get(i);
                    if ((ss.from >= fromL[lv-1] && ss.from <= toL[lv-1]) ||
                        (ss.to >= fromL[lv-1] && ss.to <= toL[lv-1]) ) {
                        if (fromL[lv] < 0) {
                            fromL[lv] = i;
                            boundary_from = ss.from;
                        }
                        toL[lv] = i;
                        boundary_to = ss.to;
                    } else if (ss.from <= fromL[lv-1] && ss.to >= toL[lv-1]) {
                        boundary_from = ss.from;
                        boundary_to   = ss.to;
                        fromL[lv] = i;
                        toL[lv] = i;
                        break;
                    } else if (ss.from > toL[lv-1]) {
                        break;
                    }
                }
                if (boundary_from != fromL[lv-1] || boundary_to != toL[lv-1]) {
                    deleteStrSegment0(lv, fromL[lv] + 1, toL[lv], diff);
                    boundary_to -= diff;
                    StrSegment[] tmp = new StrSegment[] {
                        (new StrSegment(toString(lv-1), boundary_from, boundary_to))
                    };
                    replaceStrSegment0(lv, tmp, fromL[lv], fromL[lv]);
                    return;
                } else {
                    deleteStrSegment0(lv, fromL[lv], toL[lv], diff);
                }
            }
            diff = toL[lv] - fromL[lv] + 1;
        }
    }

    /**
     * Delete segments (internal method).
     * 
     * @param layer     Layer
     * @param from      Delete from
     * @param to        Delete to
     * @param diff      Differential
     **/
    private void deleteStrSegment0(int layer, int from, int to, int diff) {
        ArrayList<StrSegment> strLayer = mStringLayer[layer];
        if (diff != 0) {
            for (int i = to + 1; i < strLayer.size(); i++) {
                StrSegment ss = strLayer.get(i);
                ss.from -= diff;
                ss.to   -= diff;
            }
        }
        for (int i = from; i <= to; i++) {
            strLayer.remove(from);
        }
    }

    /**
     * Delete a segment at the cursor.
     * 
     * @param layer         Layer
     * @param rightside     {@code true} if direction is rightward at the cursor, {@code false} if direction is leftward at the cursor
     * @return              The number of string segments in the specified layer
     **/
    public int delete(int layer, boolean rightside) {
        int cursor = mCursor[layer];
        ArrayList<StrSegment> strLayer = mStringLayer[layer];

        if (!rightside && cursor > 0) {
            deleteStrSegment(layer, cursor-1, cursor-1);
            setCursor(layer, cursor - 1);
        } else if (rightside && cursor < strLayer.size()) {
            deleteStrSegment(layer, cursor, cursor);
            setCursor(layer, cursor);
        }
        return strLayer.size();
    }

    /**
     * Get the string layer.
     *
     * @param layer     Layer
     * @return          {@link ArrayList} of {@link StrSegment}; {@code null} if error.
     **/
    public ArrayList<StrSegment> getStringLayer(int layer) {
        try {
            return mStringLayer[layer];
        } catch (Exception ex) {
            return null;
        }
    }

    /**
     * Get upper the segment which includes the position.
     * 
     * @param layer     Layer   
     * @param pos       Position        
     * @return      Index of upper segment
     */
    private int included(int layer, int pos) {
        if (pos == 0) {
            return 0;
        }
        int uplayer = layer + 1;
        int i;
        ArrayList<StrSegment> strLayer = mStringLayer[uplayer];
        for (i = 0; i < strLayer.size(); i++) {
            StrSegment ss = strLayer.get(i);
            if (ss.from <= pos && pos <= ss.to) {
                break;
            }
        }
        return i;
    }

    /**
     * Set the cursor.
     * 
     * @param layer     Layer
     * @param pos       Position of cursor
     * @return      New position of cursor
     */
    public int setCursor(int layer, int pos) {
        if (pos > mStringLayer[layer].size()) {
            pos = mStringLayer[layer].size();
        }
        if (pos < 0) {
            pos = 0;
        }
        if (layer == 0) {
            mCursor[0] = pos;
            mCursor[1] = included(0, pos);
            mCursor[2] = included(1, mCursor[1]);
        } else if (layer == 1) {
            mCursor[2] = included(1, pos);
            mCursor[1] = pos;
            mCursor[0] = (pos > 0)? mStringLayer[1].get(pos - 1).to+1 : 0;
        } else {
            mCursor[2] = pos;
            mCursor[1] = (pos > 0)? mStringLayer[2].get(pos - 1).to+1 : 0;
            mCursor[0] = (mCursor[1] > 0)? mStringLayer[1].get(mCursor[1] - 1).to+1 : 0;
        }
        return pos;
    }

    /**
     * Move the cursor.
     *
     * @param layer     Layer
     * @param diff      Relative position from current cursor position
     * @return      New position of cursor
     **/
    public int moveCursor(int layer, int diff) {
        int c = mCursor[layer] + diff;

        return setCursor(layer, c);
    }

    /**
     * Get the cursor position.
     *
     * @param layer     Layer
     * @return cursor   Current position of cursor
     **/
    public int getCursor(int layer) {
        return mCursor[layer];
    }

    /**
     * Get the number of segments.
     *
     * @param layer     Layer
     * @return          Number of segments
     **/
    public int size(int layer) {
        return mStringLayer[layer].size();
    }

    /**
     * Clear all information.
     */
    public void clear() {
        for (int i = 0; i < MAX_LAYER; i++) {
            mStringLayer[i].clear();
            mCursor[i] = 0;
        }
    }
}
