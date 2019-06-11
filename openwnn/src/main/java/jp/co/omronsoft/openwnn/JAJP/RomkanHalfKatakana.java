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

import jp.co.omronsoft.openwnn.*;
import java.util.HashMap;
import android.content.SharedPreferences;

/**
 * The Romaji to half-width Kataka converter class for Japanese IME.
 *
 * @author Copyright (C) 2009 OMRON SOFTWARE CO., LTD.  All Rights Reserved.
 */
public class RomkanHalfKatakana implements LetterConverter {
    /** HashMap for Romaji-to-Kana conversion (Japanese mode) */
    private static final HashMap<String, String> mRomkanTable = new HashMap<String, String>() {{
        put("la", "\uff67");         put("xa", "\uff67");         put("a", "\uff71");
        put("li", "\uff68");         put("lyi", "\uff68");        put("xi", "\uff68");
        put("xyi", "\uff68");        put("i", "\uff72");          put("yi", "\uff72");
        put("ye", "\uff72\uff6a");        put("lu", "\uff69");         put("xu", "\uff69");
        put("u", "\uff73");          put("whu", "\uff73");        put("wu", "\uff73");
        put("wha", "\uff73\uff67");       put("whi", "\uff73\uff68");       put("wi", "\uff73\uff68");
        put("we", "\uff73\uff6a");        put("whe", "\uff73\uff6a");       put("who", "\uff73\uff6b");
        put("le", "\uff6a");         put("lye", "\uff6a");        put("xe", "\uff6a");
        put("xye", "\uff6a");        put("e", "\uff74");          put("lo", "\uff6b");
        put("xo", "\uff6b");         put("o", "\uff75");          put("ca", "\uff76");
        put("ka", "\uff76");         put("ga", "\uff76\uff9e");        put("ki", "\uff77");
        put("kyi", "\uff77\uff68");       put("kye", "\uff77\uff6a");       put("kya", "\uff77\uff6c");
        put("kyu", "\uff77\uff6d");       put("kyo", "\uff77\uff6e");       put("gi", "\uff77\uff9e");
        put("gyi", "\uff77\uff9e\uff68");      put("gye", "\uff77\uff9e\uff6a");      put("gya", "\uff77\uff9e\uff6c");
        put("gyu", "\uff77\uff9e\uff6d");      put("gyo", "\uff77\uff9e\uff6e");      put("cu", "\uff78");
        put("ku", "\uff78");         put("qu", "\uff78");         put("kwa", "\uff78\uff67");
        put("qa", "\uff78\uff67");        put("qwa", "\uff78\uff67");       put("qi", "\uff78\uff68");
        put("qwi", "\uff78\uff68");       put("qyi", "\uff78\uff68");       put("qwu", "\uff78\uff69");
        put("qe", "\uff78\uff6a");        put("qwe", "\uff78\uff6a");       put("qye", "\uff78\uff6a");
        put("qo", "\uff78\uff6b");        put("qwo", "\uff78\uff6b");       put("qya", "\uff78\uff6c");
        put("qyu", "\uff78\uff6d");       put("qyo", "\uff78\uff6e");       put("gu", "\uff78\uff9e");
        put("gwa", "\uff78\uff9e\uff67");      put("gwi", "\uff78\uff9e\uff68");      put("gwu", "\uff78\uff9e\uff69");
        put("gwe", "\uff78\uff9e\uff6a");      put("gwo", "\uff78\uff9e\uff6b");      put("ke", "\uff79");
        put("ge", "\uff79\uff9e");        put("co", "\uff7a");         put("ko", "\uff7a");
        put("go", "\uff7a\uff9e");        put("sa", "\uff7b");         put("za", "\uff7b\uff9e");
        put("ci", "\uff7c");         put("shi", "\uff7c");        put("si", "\uff7c");
        put("syi", "\uff7c\uff68");       put("she", "\uff7c\uff6a");       put("sye", "\uff7c\uff6a");
        put("sha", "\uff7c\uff6c");       put("sya", "\uff7c\uff6c");       put("shu", "\uff7c\uff6d");
        put("syu", "\uff7c\uff6d");       put("sho", "\uff7c\uff6e");       put("syo", "\uff7c\uff6e");
        put("ji", "\uff7c\uff9e");        put("zi", "\uff7c\uff9e");        put("jyi", "\uff7c\uff9e\uff68");
        put("zyi", "\uff7c\uff9e\uff68");      put("je", "\uff7c\uff9e\uff6a");       put("jye", "\uff7c\uff9e\uff6a");
        put("zye", "\uff7c\uff9e\uff6a");      put("ja", "\uff7c\uff9e\uff6c");       put("jya", "\uff7c\uff9e\uff6c");
        put("zya", "\uff7c\uff9e\uff6c");      put("ju", "\uff7c\uff9e\uff6d");       put("jyu", "\uff7c\uff9e\uff6d");
        put("zyu", "\uff7c\uff9e\uff6d");      put("jo", "\uff7c\uff9e\uff6e");       put("jyo", "\uff7c\uff9e\uff6e");
        put("zyo", "\uff7c\uff9e\uff6e");      put("su", "\uff7d");         put("swa", "\uff7d\uff67");
        put("swi", "\uff7d\uff68");       put("swu", "\uff7d\uff69");       put("swe", "\uff7d\uff6a");
        put("swo", "\uff7d\uff6b");       put("zu", "\uff7d\uff9e");        put("ce", "\uff7e");
        put("se", "\uff7e");         put("ze", "\uff7e\uff9e");        put("so", "\uff7f");
        put("zo", "\uff7f\uff9e");        put("ta", "\uff80");         put("da", "\uff80\uff9e");
        put("chi", "\uff81");        put("ti", "\uff81");         put("cyi", "\uff81\uff68");
        put("tyi", "\uff81\uff68");       put("che", "\uff81\uff6a");       put("cye", "\uff81\uff6a");
        put("tye", "\uff81\uff6a");       put("cha", "\uff81\uff6c");       put("cya", "\uff81\uff6c");
        put("tya", "\uff81\uff6c");       put("chu", "\uff81\uff6d");       put("cyu", "\uff81\uff6d");
        put("tyu", "\uff81\uff6d");       put("cho", "\uff81\uff6e");       put("cyo", "\uff81\uff6e");
        put("tyo", "\uff81\uff6e");       put("di", "\uff81\uff9e");        put("dyi", "\uff81\uff9e\uff68");
        put("dye", "\uff81\uff9e\uff6a");      put("dya", "\uff81\uff9e\uff6c");      put("dyu", "\uff81\uff9e\uff6d");
        put("dyo", "\uff81\uff9e\uff6e");      put("ltsu", "\uff6f");       put("ltu", "\uff6f");
        put("xtu", "\uff6f");        put("", "\uff6f");           put("tsu", "\uff82");
        put("tu", "\uff82");         put("tsa", "\uff82\uff67");       put("tsi", "\uff82\uff68");
        put("tse", "\uff82\uff6a");       put("tso", "\uff82\uff6b");       put("du", "\uff82\uff9e");
        put("te", "\uff83");         put("thi", "\uff83\uff68");       put("the", "\uff83\uff6a");
        put("tha", "\uff83\uff6c");       put("thu", "\uff83\uff6d");       put("tho", "\uff83\uff6e");
        put("de", "\uff83\uff9e");        put("dhi", "\uff83\uff9e\uff68");      put("dhe", "\uff83\uff9e\uff6a");
        put("dha", "\uff83\uff9e\uff6c");      put("dhu", "\uff83\uff9e\uff6d");      put("dho", "\uff83\uff9e\uff6e");
        put("to", "\uff84");         put("twa", "\uff84\uff67");       put("twi", "\uff84\uff68");
        put("twu", "\uff84\uff69");       put("twe", "\uff84\uff6a");       put("two", "\uff84\uff6b");
        put("do", "\uff84\uff9e");        put("dwa", "\uff84\uff9e\uff67");      put("dwi", "\uff84\uff9e\uff68");
        put("dwu", "\uff84\uff9e\uff69");      put("dwe", "\uff84\uff9e\uff6a");      put("dwo", "\uff84\uff9e\uff6b");
        put("na", "\uff85");         put("ni", "\uff86");         put("nyi", "\uff86\uff68");
        put("nye", "\uff86\uff6a");       put("nya", "\uff86\uff6c");       put("nyu", "\uff86\uff6d");
        put("nyo", "\uff86\uff6e");       put("nu", "\uff87");         put("ne", "\uff88");
        put("no", "\uff89");         put("ha", "\uff8a");         put("ba", "\uff8a\uff9e");
        put("pa", "\uff8a\uff9f");        put("hi", "\uff8b");         put("hyi", "\uff8b\uff68");
        put("hye", "\uff8b\uff6a");       put("hya", "\uff8b\uff6c");       put("hyu", "\uff8b\uff6d");
        put("hyo", "\uff8b\uff6e");       put("bi", "\uff8b\uff9e");        put("byi", "\uff8b\uff9e\uff68");
        put("bye", "\uff8b\uff9e\uff6a");      put("bya", "\uff8b\uff9e\uff6c");      put("byu", "\uff8b\uff9e\uff6d");
        put("byo", "\uff8b\uff9e\uff6e");      put("pi", "\uff8b\uff9f");        put("pyi", "\uff8b\uff9f\uff68");
        put("pye", "\uff8b\uff9f\uff6a");      put("pya", "\uff8b\uff9f\uff6c");      put("pyu", "\uff8b\uff9f\uff6d");
        put("pyo", "\uff8b\uff9f\uff6e");      put("fu", "\uff8c");         put("hu", "\uff8c");
        put("fa", "\uff8c\uff67");        put("fwa", "\uff8c\uff67");       put("fi", "\uff8c\uff68");
        put("fwi", "\uff8c\uff68");       put("fyi", "\uff8c\uff68");       put("fwu", "\uff8c\uff69");
        put("fe", "\uff8c\uff6a");        put("fwe", "\uff8c\uff6a");       put("fye", "\uff8c\uff6a");
        put("fo", "\uff8c\uff6b");        put("fwo", "\uff8c\uff6b");       put("fya", "\uff8c\uff6c");
        put("fyu", "\uff8c\uff6d");       put("fyo", "\uff8c\uff6e");       put("bu", "\uff8c\uff9e");
        put("pu", "\uff8c\uff9f");        put("he", "\uff8d");         put("be", "\uff8d\uff9e");
        put("pe", "\uff8d\uff9f");        put("ho", "\uff8e");         put("bo", "\uff8e\uff9e");
        put("po", "\uff8e\uff9f");        put("ma", "\uff8f");         put("mi", "\uff90");
        put("myi", "\uff90\uff68");       put("mye", "\uff90\uff6a");       put("mya", "\uff90\uff6c");
        put("myu", "\uff90\uff6d");       put("myo", "\uff90\uff6e");       put("mu", "\uff91");
        put("me", "\uff92");         put("mo", "\uff93");         put("lya", "\uff6c");
        put("xya", "\uff6c");        put("ya", "\uff94");         put("lyu", "\uff6d");
        put("xyu", "\uff6d");        put("yu", "\uff95");         put("lyo", "\uff6e");
        put("xyo", "\uff6e");        put("yo", "\uff96");         put("ra", "\uff97");
        put("ri", "\uff98");         put("ryi", "\uff98\uff68");       put("rye", "\uff98\uff6a");
        put("rya", "\uff98\uff6c");       put("ryu", "\uff98\uff6d");       put("ryo", "\uff98\uff6e");
        put("ru", "\uff99");         put("re", "\uff9a");         put("ro", "\uff9b");
        put("lwa", "\uff9c");        put("xwa", "\uff9c");        put("wa", "\uff9c");
        put("wo", "\uff66");         put("nn", "\uff9d");         put("xn", "\uff9d");
        put("vu", "\uff73\uff9e");        put("va", "\uff73\uff9e\uff67");       put("vi", "\uff73\uff9e\uff68");
        put("vyi", "\uff73\uff9e\uff68");      put("ve", "\uff73\uff9e\uff6a");       put("vye", "\uff73\uff9e\uff6a");
        put("vo", "\uff73\uff9e\uff6b");       put("vya", "\uff73\uff9e\uff6c");      put("vyu", "\uff73\uff9e\uff6d");
        put("vyo", "\uff73\uff9e\uff6e");      
        put("bb", "\uff6fb");        put("cc", "\uff6fc");        put("dd", "\uff6fd");
        put("ff", "\uff6ff");        put("gg", "\uff6fg");        put("hh", "\uff6fh");
        put("jj", "\uff6fj");        put("kk", "\uff6fk");        put("ll", "\uff6fl");
        put("mm", "\uff6fm");        put("pp", "\uff6fp");        put("qq", "\uff6fq");
        put("rr", "\uff6fr");        put("ss", "\uff6fs");        put("tt", "\uff6ft");
        put("vv", "\uff6fv");        put("ww", "\uff6fw");        put("xx", "\uff6fx");
        put("yy", "\uff6fy");        put("zz", "\uff6fz");        put("nb", "\uff9db");
        put("nc", "\uff9dc");        put("nd", "\uff9dd");        put("nf", "\uff9df");
        put("ng", "\uff9dg");        put("nh", "\uff9dh");        put("nj", "\uff9dj");
        put("nk", "\uff9dk");        put("nm", "\uff9dm");        put("np", "\uff9dp");
        put("nq", "\uff9dq");        put("nr", "\uff9dr");        put("ns", "\uff9ds");
        put("nt", "\uff9dt");        put("nv", "\uff9dv");        put("nw", "\uff9dw");
        put("nx", "\uff9dx");        put("nz", "\uff9dz");        put("nl", "\uff9dl");
        put("-", "\uff70"); put(".", "\uff61"); put(",", "\uff64"); put("/", "\uff65");
    }};

    /**
     * Default constructor
     */
    public RomkanHalfKatakana() {
        super();
    }

    /** @see LetterConverter#convert */
    public boolean convert(ComposingText text) {
        return RomkanFullKatakana.convert(text, mRomkanTable);
    }

    /** @see LetterConverter#setPreferences */
    public void setPreferences(SharedPreferences pref) {}
}
