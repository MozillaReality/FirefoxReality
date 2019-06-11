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

package jp.co.omronsoft.openwnn.JAJP;

import jp.co.omronsoft.openwnn.LetterConverter;
import jp.co.omronsoft.openwnn.ComposingText;
import jp.co.omronsoft.openwnn.StrSegment;
import java.util.HashMap;
import android.content.SharedPreferences;

/**
 * The Romaji to full-width Katakana converter class for Japanese IME.
 *
 * @author Copyright (C) 2009 OMRON SOFTWARE CO., LTD.  All Rights Reserved.
 */
public class RomkanFullKatakana implements LetterConverter {
    /** HashMap for Romaji-to-Kana conversion (Japanese mode) */
    private static final HashMap<String, String> mRomkanTable = new HashMap<String, String>() {{
        put("la", "\u30a1");        put("xa", "\u30a1");        put("a", "\u30a2");
        put("li", "\u30a3");        put("lyi", "\u30a3");       put("xi", "\u30a3");
        put("xyi", "\u30a3");       put("i", "\u30a4");         put("yi", "\u30a4");
        put("ye", "\u30a4\u30a7");      put("lu", "\u30a5");        put("xu", "\u30a5");
        put("u", "\u30a6");         put("whu", "\u30a6");       put("wu", "\u30a6");
        put("wha", "\u30a6\u30a1");     put("whi", "\u30a6\u30a3");     put("wi", "\u30a6\u30a3");
        put("we", "\u30a6\u30a7");      put("whe", "\u30a6\u30a7");     put("who", "\u30a6\u30a9");
        put("le", "\u30a7");        put("lye", "\u30a7");       put("xe", "\u30a7");
        put("xye", "\u30a7");       put("e", "\u30a8");         put("lo", "\u30a9");
        put("xo", "\u30a9");        put("o", "\u30aa");         put("ca", "\u30ab");
        put("lka", "\u30f5");       put("xka", "\u30f5");
        put("ka", "\u30ab");        put("ga", "\u30ac");        put("ki", "\u30ad");
        put("kyi", "\u30ad\u30a3");     put("kye", "\u30ad\u30a7");     put("kya", "\u30ad\u30e3");
        put("kyu", "\u30ad\u30e5");     put("kyo", "\u30ad\u30e7");     put("gi", "\u30ae");
        put("gyi", "\u30ae\u30a3");     put("gye", "\u30ae\u30a7");     put("gya", "\u30ae\u30e3");
        put("gyu", "\u30ae\u30e5");     put("gyo", "\u30ae\u30e7");     put("cu", "\u30af");
        put("ku", "\u30af");        put("qu", "\u30af");        put("kwa", "\u30af\u30a1");
        put("qa", "\u30af\u30a1");      put("qwa", "\u30af\u30a1");     put("qi", "\u30af\u30a3");
        put("qwi", "\u30af\u30a3");     put("qyi", "\u30af\u30a3");     put("qwu", "\u30af\u30a5");
        put("qe", "\u30af\u30a7");      put("qwe", "\u30af\u30a7");     put("qye", "\u30af\u30a7");
        put("qo", "\u30af\u30a9");      put("qwo", "\u30af\u30a9");     put("qya", "\u30af\u30e3");
        put("qyu", "\u30af\u30e5");     put("qyo", "\u30af\u30e7");     put("gu", "\u30b0");
        put("gwa", "\u30b0\u30a1");     put("gwi", "\u30b0\u30a3");     put("gwu", "\u30b0\u30a5");
        put("gwe", "\u30b0\u30a7");     put("gwo", "\u30b0\u30a9");
        put("lke", "\u30f6");       put("xke", "\u30f6");       put("ke", "\u30b1");
        put("ge", "\u30b2");        put("co", "\u30b3");        put("ko", "\u30b3");
        put("go", "\u30b4");        put("sa", "\u30b5");        put("za", "\u30b6");
        put("ci", "\u30b7");        put("shi", "\u30b7");       put("si", "\u30b7");
        put("syi", "\u30b7\u30a3");     put("she", "\u30b7\u30a7");     put("sye", "\u30b7\u30a7");
        put("sha", "\u30b7\u30e3");     put("sya", "\u30b7\u30e3");     put("shu", "\u30b7\u30e5");
        put("syu", "\u30b7\u30e5");     put("sho", "\u30b7\u30e7");     put("syo", "\u30b7\u30e7");
        put("ji", "\u30b8");        put("zi", "\u30b8");        put("jyi", "\u30b8\u30a3");
        put("zyi", "\u30b8\u30a3");     put("je", "\u30b8\u30a7");      put("jye", "\u30b8\u30a7");
        put("zye", "\u30b8\u30a7");     put("ja", "\u30b8\u30e3");      put("jya", "\u30b8\u30e3");
        put("zya", "\u30b8\u30e3");     put("ju", "\u30b8\u30e5");      put("jyu", "\u30b8\u30e5");
        put("zyu", "\u30b8\u30e5");     put("jo", "\u30b8\u30e7");      put("jyo", "\u30b8\u30e7");
        put("zyo", "\u30b8\u30e7");     put("su", "\u30b9");        put("swa", "\u30b9\u30a1");
        put("swi", "\u30b9\u30a3");     put("swu", "\u30b9\u30a5");     put("swe", "\u30b9\u30a7");
        put("swo", "\u30b9\u30a9");     put("zu", "\u30ba");        put("ce", "\u30bb");
        put("se", "\u30bb");        put("ze", "\u30bc");        put("so", "\u30bd");
        put("zo", "\u30be");        put("ta", "\u30bf");        put("da", "\u30c0");
        put("chi", "\u30c1");       put("ti", "\u30c1");        put("cyi", "\u30c1\u30a3");
        put("tyi", "\u30c1\u30a3");     put("che", "\u30c1\u30a7");     put("cye", "\u30c1\u30a7");
        put("tye", "\u30c1\u30a7");     put("cha", "\u30c1\u30e3");     put("cya", "\u30c1\u30e3");
        put("tya", "\u30c1\u30e3");     put("chu", "\u30c1\u30e5");     put("cyu", "\u30c1\u30e5");
        put("tyu", "\u30c1\u30e5");     put("cho", "\u30c1\u30e7");     put("cyo", "\u30c1\u30e7");
        put("tyo", "\u30c1\u30e7");     put("di", "\u30c2");        put("dyi", "\u30c2\u30a3");
        put("dye", "\u30c2\u30a7");     put("dya", "\u30c2\u30e3");     put("dyu", "\u30c2\u30e5");
        put("dyo", "\u30c2\u30e7");     put("ltsu", "\u30c3");      put("ltu", "\u30c3");
        put("xtu", "\u30c3");       put("", "\u30c3");          put("tsu", "\u30c4");
        put("tu", "\u30c4");        put("tsa", "\u30c4\u30a1");     put("tsi", "\u30c4\u30a3");
        put("tse", "\u30c4\u30a7");     put("tso", "\u30c4\u30a9");     put("du", "\u30c5");
        put("te", "\u30c6");        put("thi", "\u30c6\u30a3");     put("the", "\u30c6\u30a7");
        put("tha", "\u30c6\u30e3");     put("thu", "\u30c6\u30e5");     put("tho", "\u30c6\u30e7");
        put("de", "\u30c7");        put("dhi", "\u30c7\u30a3");     put("dhe", "\u30c7\u30a7");
        put("dha", "\u30c7\u30e3");     put("dhu", "\u30c7\u30e5");     put("dho", "\u30c7\u30e7");
        put("to", "\u30c8");        put("twa", "\u30c8\u30a1");     put("twi", "\u30c8\u30a3");
        put("twu", "\u30c8\u30a5");     put("twe", "\u30c8\u30a7");     put("two", "\u30c8\u30a9");
        put("do", "\u30c9");        put("dwa", "\u30c9\u30a1");     put("dwi", "\u30c9\u30a3");
        put("dwu", "\u30c9\u30a5");     put("dwe", "\u30c9\u30a7");     put("dwo", "\u30c9\u30a9");
        put("na", "\u30ca");        put("ni", "\u30cb");        put("nyi", "\u30cb\u30a3");
        put("nye", "\u30cb\u30a7");     put("nya", "\u30cb\u30e3");     put("nyu", "\u30cb\u30e5");
        put("nyo", "\u30cb\u30e7");     put("nu", "\u30cc");        put("ne", "\u30cd");
        put("no", "\u30ce");        put("ha", "\u30cf");        put("ba", "\u30d0");
        put("pa", "\u30d1");        put("hi", "\u30d2");        put("hyi", "\u30d2\u30a3");
        put("hye", "\u30d2\u30a7");     put("hya", "\u30d2\u30e3");     put("hyu", "\u30d2\u30e5");
        put("hyo", "\u30d2\u30e7");     put("bi", "\u30d3");        put("byi", "\u30d3\u30a3");
        put("bye", "\u30d3\u30a7");     put("bya", "\u30d3\u30e3");     put("byu", "\u30d3\u30e5");
        put("byo", "\u30d3\u30e7");     put("pi", "\u30d4");        put("pyi", "\u30d4\u30a3");
        put("pye", "\u30d4\u30a7");     put("pya", "\u30d4\u30e3");     put("pyu", "\u30d4\u30e5");
        put("pyo", "\u30d4\u30e7");     put("fu", "\u30d5");        put("hu", "\u30d5");
        put("fa", "\u30d5\u30a1");      put("fwa", "\u30d5\u30a1");     put("fi", "\u30d5\u30a3");
        put("fwi", "\u30d5\u30a3");     put("fyi", "\u30d5\u30a3");     put("fwu", "\u30d5\u30a5");
        put("fe", "\u30d5\u30a7");      put("fwe", "\u30d5\u30a7");     put("fye", "\u30d5\u30a7");
        put("fo", "\u30d5\u30a9");      put("fwo", "\u30d5\u30a9");     put("fya", "\u30d5\u30e3");
        put("fyu", "\u30d5\u30e5");     put("fyo", "\u30d5\u30e7");     put("bu", "\u30d6");
        put("pu", "\u30d7");        put("he", "\u30d8");        put("be", "\u30d9");
        put("pe", "\u30da");        put("ho", "\u30db");        put("bo", "\u30dc");
        put("po", "\u30dd");        put("ma", "\u30de");        put("mi", "\u30df");
        put("myi", "\u30df\u30a3");     put("mye", "\u30df\u30a7");     put("mya", "\u30df\u30e3");
        put("myu", "\u30df\u30e5");     put("myo", "\u30df\u30e7");     put("mu", "\u30e0");
        put("me", "\u30e1");        put("mo", "\u30e2");        put("lya", "\u30e3");
        put("xya", "\u30e3");       put("ya", "\u30e4");        put("lyu", "\u30e5");
        put("xyu", "\u30e5");       put("yu", "\u30e6");        put("lyo", "\u30e7");
        put("xyo", "\u30e7");       put("yo", "\u30e8");        put("ra", "\u30e9");
        put("ri", "\u30ea");        put("ryi", "\u30ea\u30a3");     put("rye", "\u30ea\u30a7");
        put("rya", "\u30ea\u30e3");     put("ryu", "\u30ea\u30e5");     put("ryo", "\u30ea\u30e7");
        put("ru", "\u30eb");        put("re", "\u30ec");        put("ro", "\u30ed");
        put("lwa", "\u30ee");       put("xwa", "\u30ee");       put("wa", "\u30ef");
        put("wo", "\u30f2");        put("nn", "\u30f3");        put("xn", "\u30f3");
        put("vu", "\u30f4");        put("va", "\u30f4\u30a1");      put("vi", "\u30f4\u30a3");
        put("vyi", "\u30f4\u30a3");     put("ve", "\u30f4\u30a7");      put("vye", "\u30f4\u30a7");
        put("vo", "\u30f4\u30a9");      put("vya", "\u30f4\u30e3");     put("vyu", "\u30f4\u30e5");
        put("vyo", "\u30f4\u30e7");     
        put("bb", "\u30c3b");   put("cc", "\u30c3c");   put("dd", "\u30c3d");
        put("ff", "\u30c3f");   put("gg", "\u30c3g");   put("hh", "\u30c3h");
        put("jj", "\u30c3j");   put("kk", "\u30c3k");   put("ll", "\u30c3l");
        put("mm", "\u30c3m");   put("pp", "\u30c3p");   put("qq", "\u30c3q");
        put("rr", "\u30c3r");   put("ss", "\u30c3s");   put("tt", "\u30c3t");
        put("vv", "\u30c3v");   put("ww", "\u30c3w");   put("xx", "\u30c3x");
        put("yy", "\u30c3y");   put("zz", "\u30c3z");   put("nb", "\u30f3b");
        put("nc", "\u30f3c");   put("nd", "\u30f3d");   put("nf", "\u30f3f");
        put("ng", "\u30f3g");   put("nh", "\u30f3h");   put("nj", "\u30f3j");
        put("nk", "\u30f3k");   put("nm", "\u30f3m");   put("np", "\u30f3p");
        put("nq", "\u30f3q");   put("nr", "\u30f3r");   put("ns", "\u30f3s");
        put("nt", "\u30f3t");   put("nv", "\u30f3v");   put("nw", "\u30f3w");
        put("nx", "\u30f3x");   put("nz", "\u30f3z");   put("nl", "\u30f3l");   
        put("-", "\u30fc"); put(".", "\u3002"); put(",", "\u3001"); put("?", "\uff1f"); put("/", "\u30fb");
    }};

    /** Max length of the target text */
    private static final int MAX_LENGTH = 4;


    /**
     * Default constructor
     */
    public RomkanFullKatakana() {
        super();
    }

    /** @see LetterConverter#convert */
    public boolean convert(ComposingText text) {
        return convert(text, mRomkanTable);
    }

    /**
     * convert Romaji to Full Katakana
     *
     * @param text              The input/output text
     * @param table             HashMap for Romaji-to-Kana conversion
     * @return                  {@code true} if conversion is compleated; {@code false} if not
     */
    public static boolean convert(ComposingText text, HashMap<String, String> table) {
        int cursor = text.getCursor(1);

        if (cursor <= 0) {
            return false;
        }

        StrSegment[] str = new StrSegment[MAX_LENGTH];
        int start = MAX_LENGTH;
        int checkLength = Math.min(cursor, MAX_LENGTH);
        for (int i = 1; i <= checkLength; i++) {
            str[MAX_LENGTH - i] = text.getStrSegment(1, cursor - i);
            start--;
        }

        StringBuffer key = new StringBuffer();
        while (start < MAX_LENGTH) {
            for (int i = start; i < MAX_LENGTH; i++) {
                key.append(str[i].string);
            }
            boolean upper = Character.isUpperCase(key.charAt(key.length() - 1));
            String match = table.get(key.toString().toLowerCase());
            if (match != null) {
                if (upper) {
                    match = match.toUpperCase();
                }
                StrSegment[] out;
                if (match.length() == 1) {
                    out = new StrSegment[1];
                    out[0] = new StrSegment(match, str[start].from, str[MAX_LENGTH - 1].to);
                    text.replaceStrSegment(ComposingText.LAYER1, out, MAX_LENGTH - start);
                } else {
                    out = new StrSegment[2];
                    out[0] = new StrSegment(match.substring(0, match.length() - 1),
                                            str[start].from, str[MAX_LENGTH - 1].to - 1);
                    out[1] = new StrSegment(match.substring(match.length() - 1),
                                            str[MAX_LENGTH - 1].to, str[MAX_LENGTH - 1].to);
                    text.replaceStrSegment(1, out, MAX_LENGTH - start);
                }
                return true;
            }
            start++;
            key.delete(0, key.length());
        }

        return false;
    }

    /** @see LetterConverter#setPreferences */
    public void setPreferences(SharedPreferences pref) {}
}
