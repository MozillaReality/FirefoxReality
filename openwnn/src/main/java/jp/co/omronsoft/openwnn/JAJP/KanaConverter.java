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

import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import jp.co.omronsoft.openwnn.WnnDictionary;
import jp.co.omronsoft.openwnn.WnnPOS;
import jp.co.omronsoft.openwnn.WnnWord;

/**
 * The EISU-KANA converter class for Japanese IME.
 *
 * @author Copyright (C) 2009 OMRON SOFTWARE CO., LTD.  All Rights Reserved.
 */
public class KanaConverter {

	/** Conversion rule for half-width numeric */
    private static final HashMap<String,String> mHalfNumericMap = new HashMap<String,String>() {{
        put( "\u3042", "1");
        put( "\u3044", "11");
        put( "\u3046", "111");
        put( "\u3048", "1111");
        put( "\u304a", "11111");
        put( "\u3041", "111111");
        put( "\u3043", "1111111");
        put( "\u3045", "11111111");
        put( "\u3047", "111111111");
        put( "\u3049", "1111111111");
        put( "\u304b", "2");
        put( "\u304d", "22");
        put( "\u304f", "222");
        put( "\u3051", "2222");
        put( "\u3053", "22222");
        put( "\u3055", "3");
        put( "\u3057", "33");
        put( "\u3059", "333");
        put( "\u305b", "3333");
        put( "\u305d", "33333");
        put( "\u305f", "4");
        put( "\u3061", "44");
        put( "\u3064", "444");
        put( "\u3066", "4444");
        put( "\u3068", "44444");
        put( "\u3063", "444444");
        put( "\u306a", "5");
        put( "\u306b", "55");
        put( "\u306c", "555");
        put( "\u306d", "5555");
        put( "\u306e", "55555");
        put( "\u306f", "6");
        put( "\u3072", "66");
        put( "\u3075", "666");
        put( "\u3078", "6666");
        put( "\u307b", "66666");
        put( "\u307e", "7");
        put( "\u307f", "77");
        put( "\u3080", "777");
        put( "\u3081", "7777");
        put( "\u3082", "77777");
        put( "\u3084", "8");
        put( "\u3086", "88");
        put( "\u3088", "888");
        put( "\u3083", "8888");
        put( "\u3085", "88888");
        put( "\u3087", "888888");
        put( "\u3089", "9");
        put( "\u308a", "99");
        put( "\u308b", "999");
        put( "\u308c", "9999");
        put( "\u308d", "99999");
        put( "\u308f", "0");
        put( "\u3092", "00");
        put( "\u3093", "000");
        put( "\u308e", "0000");
        put( "\u30fc", "00000");
    }};

    /** Conversion rule for full-width numeric */
    private static final HashMap<String,String> mFullNumericMap = new HashMap<String,String>() {{
        put( "\u3042", "\uff11");
        put( "\u3044", "\uff11\uff11");
        put( "\u3046", "\uff11\uff11\uff11");
        put( "\u3048", "\uff11\uff11\uff11\uff11");
        put( "\u304a", "\uff11\uff11\uff11\uff11\uff11");
        put( "\u3041", "\uff11\uff11\uff11\uff11\uff11\uff11");
        put( "\u3043", "\uff11\uff11\uff11\uff11\uff11\uff11\uff11");
        put( "\u3045", "\uff11\uff11\uff11\uff11\uff11\uff11\uff11\uff11");
        put( "\u3047", "\uff11\uff11\uff11\uff11\uff11\uff11\uff11\uff11\uff11");
        put( "\u3049", "\uff11\uff11\uff11\uff11\uff11\uff11\uff11\uff11\uff11\uff11");
        put( "\u304b", "\uff12");
        put( "\u304d", "\uff12\uff12");
        put( "\u304f", "\uff12\uff12\uff12");
        put( "\u3051", "\uff12\uff12\uff12\uff12");
        put( "\u3053", "\uff12\uff12\uff12\uff12\uff12");
        put( "\u3055", "\uff13");
        put( "\u3057", "\uff13\uff13");
        put( "\u3059", "\uff13\uff13\uff13");
        put( "\u305b", "\uff13\uff13\uff13\uff13");
        put( "\u305d", "\uff13\uff13\uff13\uff13\uff13");
        put( "\u305f", "\uff14");
        put( "\u3061", "\uff14\uff14");
        put( "\u3064", "\uff14\uff14\uff14");
        put( "\u3066", "\uff14\uff14\uff14\uff14");
        put( "\u3068", "\uff14\uff14\uff14\uff14\uff14");
        put( "\u3063", "\uff14\uff14\uff14\uff14\uff14\uff14");
        put( "\u306a", "\uff15");
        put( "\u306b", "\uff15\uff15");
        put( "\u306c", "\uff15\uff15\uff15");
        put( "\u306d", "\uff15\uff15\uff15\uff15");
        put( "\u306e", "\uff15\uff15\uff15\uff15\uff15");
        put( "\u306f", "\uff16");
        put( "\u3072", "\uff16\uff16");
        put( "\u3075", "\uff16\uff16\uff16");
        put( "\u3078", "\uff16\uff16\uff16\uff16");
        put( "\u307b", "\uff16\uff16\uff16\uff16\uff16");
        put( "\u307e", "\uff17");
        put( "\u307f", "\uff17\uff17");
        put( "\u3080", "\uff17\uff17\uff17");
        put( "\u3081", "\uff17\uff17\uff17\uff17");
        put( "\u3082", "\uff17\uff17\uff17\uff17\uff17");
        put( "\u3084", "\uff18");
        put( "\u3086", "\uff18\uff18");
        put( "\u3088", "\uff18\uff18\uff18");
        put( "\u3083", "\uff18\uff18\uff18\uff18");
        put( "\u3085", "\uff18\uff18\uff18\uff18\uff18");
        put( "\u3087", "\uff18\uff18\uff18\uff18\uff18\uff18");
        put( "\u3089", "\uff19");
        put( "\u308a", "\uff19\uff19");
        put( "\u308b", "\uff19\uff19\uff19");
        put( "\u308c", "\uff19\uff19\uff19\uff19");
        put( "\u308d", "\uff19\uff19\uff19\uff19\uff19");
        put( "\u308f", "\uff10");
        put( "\u3092", "\uff10\uff10");
        put( "\u3093", "\uff10\uff10\uff10");
        put( "\u308e", "\uff10\uff10\uff10\uff10");
        put( "\u30fc", "\uff10\uff10\uff10\uff10\uff10");
    }};

    /** Conversion rule for half-width Katakana */
    private static final HashMap<String,String> mHalfKatakanaMap = new HashMap<String,String>() {{
        put( "\u3042", "\uff71");
        put( "\u3044", "\uff72");
        put( "\u3046", "\uff73");
        put( "\u3048", "\uff74");
        put( "\u304a", "\uff75");
        put( "\u3041", "\uff67");
        put( "\u3043", "\uff68");
        put( "\u3045", "\uff69");
        put( "\u3047", "\uff6a");
        put( "\u3049", "\uff6b");
        put( "\u30f4\u3041", "\uff73\uff9e\uff67");
        put( "\u30f4\u3043", "\uff73\uff9e\uff68");
        put( "\u30f4", "\uff73\uff9e");
        put( "\u30f4\u3047", "\uff73\uff9e\uff6a");
        put( "\u30f4\u3049", "\uff73\uff9e\uff6b");
        put( "\u304b", "\uff76");
        put( "\u304d", "\uff77");
        put( "\u304f", "\uff78");
        put( "\u3051", "\uff79");
        put( "\u3053", "\uff7a");
        put( "\u304c", "\uff76\uff9e");
        put( "\u304e", "\uff77\uff9e");
        put( "\u3050", "\uff78\uff9e");
        put( "\u3052", "\uff79\uff9e");
        put( "\u3054", "\uff7a\uff9e");
        put( "\u3055", "\uff7b");
        put( "\u3057", "\uff7c");
        put( "\u3059", "\uff7d");
        put( "\u305b", "\uff7e");
        put( "\u305d", "\uff7f");
        put( "\u3056", "\uff7b\uff9e");
        put( "\u3058", "\uff7c\uff9e");
        put( "\u305a", "\uff7d\uff9e");
        put( "\u305c", "\uff7e\uff9e");
        put( "\u305e", "\uff7f\uff9e");
        put( "\u305f", "\uff80");
        put( "\u3061", "\uff81");
        put( "\u3064", "\uff82");
        put( "\u3066", "\uff83");
        put( "\u3068", "\uff84");
        put( "\u3063", "\uff6f");
        put( "\u3060", "\uff80\uff9e");
        put( "\u3062", "\uff81\uff9e");
        put( "\u3065", "\uff82\uff9e");
        put( "\u3067", "\uff83\uff9e");
        put( "\u3069", "\uff84\uff9e");
        put( "\u306a", "\uff85");
        put( "\u306b", "\uff86");
        put( "\u306c", "\uff87");
        put( "\u306d", "\uff88");
        put( "\u306e", "\uff89");
        put( "\u306f", "\uff8a");
        put( "\u3072", "\uff8b");
        put( "\u3075", "\uff8c");
        put( "\u3078", "\uff8d");
        put( "\u307b", "\uff8e");
        put( "\u3070", "\uff8a\uff9e");
        put( "\u3073", "\uff8b\uff9e");
        put( "\u3076", "\uff8c\uff9e");
        put( "\u3079", "\uff8d\uff9e");
        put( "\u307c", "\uff8e\uff9e");
        put( "\u3071", "\uff8a\uff9f");
        put( "\u3074", "\uff8b\uff9f");
        put( "\u3077", "\uff8c\uff9f");
        put( "\u307a", "\uff8d\uff9f");
        put( "\u307d", "\uff8e\uff9f");
        put( "\u307e", "\uff8f");
        put( "\u307f", "\uff90");
        put( "\u3080", "\uff91");
        put( "\u3081", "\uff92");
        put( "\u3082", "\uff93");
        put( "\u3084", "\uff94");
        put( "\u3086", "\uff95");
        put( "\u3088", "\uff96");
        put( "\u3083", "\uff6c");
        put( "\u3085", "\uff6d");
        put( "\u3087", "\uff6e");
        put( "\u3089", "\uff97");
        put( "\u308a", "\uff98");
        put( "\u308b", "\uff99");
        put( "\u308c", "\uff9a");
        put( "\u308d", "\uff9b");
        put( "\u308f", "\uff9c");
        put( "\u3092", "\uff66");
        put( "\u3093", "\uff9d");
        put( "\u308e", "\uff9c");
        put( "\u30fc", "\uff70");
    }};

    /** Conversion rule for full-width Katakana */
    private static final HashMap<String,String> mFullKatakanaMap = new HashMap<String,String>() {{
        put( "\u3042", "\u30a2");
        put( "\u3044", "\u30a4");
        put( "\u3046", "\u30a6");
        put( "\u3048", "\u30a8");
        put( "\u304a", "\u30aa");
        put( "\u3041", "\u30a1");
        put( "\u3043", "\u30a3");
        put( "\u3045", "\u30a5");
        put( "\u3047", "\u30a7");
        put( "\u3049", "\u30a9");
        put( "\u30f4\u3041", "\u30f4\u30a1");
        put( "\u30f4\u3043", "\u30f4\u30a3");
        put( "\u30f4", "\u30f4");
        put( "\u30f4\u3047", "\u30f4\u30a7");
        put( "\u30f4\u3049", "\u30f4\u30a9");
        put( "\u304b", "\u30ab");
        put( "\u304d", "\u30ad");
        put( "\u304f", "\u30af");
        put( "\u3051", "\u30b1");
        put( "\u3053", "\u30b3");
        put( "\u304c", "\u30ac");
        put( "\u304e", "\u30ae");
        put( "\u3050", "\u30b0");
        put( "\u3052", "\u30b2");
        put( "\u3054", "\u30b4");
        put( "\u3055", "\u30b5");
        put( "\u3057", "\u30b7");
        put( "\u3059", "\u30b9");
        put( "\u305b", "\u30bb");
        put( "\u305d", "\u30bd");
        put( "\u3056", "\u30b6");
        put( "\u3058", "\u30b8");
        put( "\u305a", "\u30ba");
        put( "\u305c", "\u30bc");
        put( "\u305e", "\u30be");
        put( "\u305f", "\u30bf");
        put( "\u3061", "\u30c1");
        put( "\u3064", "\u30c4");
        put( "\u3066", "\u30c6");
        put( "\u3068", "\u30c8");
        put( "\u3063", "\u30c3");
        put( "\u3060", "\u30c0");
        put( "\u3062", "\u30c2");
        put( "\u3065", "\u30c5");
        put( "\u3067", "\u30c7");
        put( "\u3069", "\u30c9");
        put( "\u306a", "\u30ca");
        put( "\u306b", "\u30cb");
        put( "\u306c", "\u30cc");
        put( "\u306d", "\u30cd");
        put( "\u306e", "\u30ce");
        put( "\u306f", "\u30cf");
        put( "\u3072", "\u30d2");
        put( "\u3075", "\u30d5");
        put( "\u3078", "\u30d8");
        put( "\u307b", "\u30db");
        put( "\u3070", "\u30d0");
        put( "\u3073", "\u30d3");
        put( "\u3076", "\u30d6");
        put( "\u3079", "\u30d9");
        put( "\u307c", "\u30dc");
        put( "\u3071", "\u30d1");
        put( "\u3074", "\u30d4");
        put( "\u3077", "\u30d7");
        put( "\u307a", "\u30da");
        put( "\u307d", "\u30dd");
        put( "\u307e", "\u30de");
        put( "\u307f", "\u30df");
        put( "\u3080", "\u30e0");
        put( "\u3081", "\u30e1");
        put( "\u3082", "\u30e2");
        put( "\u3084", "\u30e4");
        put( "\u3086", "\u30e6");
        put( "\u3088", "\u30e8");
        put( "\u3083", "\u30e3");
        put( "\u3085", "\u30e5");
        put( "\u3087", "\u30e7");
        put( "\u3089", "\u30e9");
        put( "\u308a", "\u30ea");
        put( "\u308b", "\u30eb");
        put( "\u308c", "\u30ec");
        put( "\u308d", "\u30ed");
        put( "\u308f", "\u30ef");
        put( "\u3092", "\u30f2");
        put( "\u3093", "\u30f3");
        put( "\u308e", "\u30ee");
        put( "\u30fc", "\u30fc");
    }};

    /** Conversion rule for half-width alphabet */
    private static final HashMap<String,String> mHalfAlphabetMap = new HashMap<String,String>() {{
        put( "\u3042", ".");
        put( "\u3044", "@");
        put( "\u3046", "-");
        put( "\u3048", "_");
        put( "\u304a", "/");
        put( "\u3041", ":");
        put( "\u3043", "~");
        put( "\u304b", "A");
        put( "\u304d", "B");
        put( "\u304f", "C");
        put( "\u3055", "D");
        put( "\u3057", "E");
        put( "\u3059", "F");
        put( "\u305f", "G");
        put( "\u3061", "H");
        put( "\u3064", "I");
        put( "\u306a", "J");
        put( "\u306b", "K");
        put( "\u306c", "L");
        put( "\u306f", "M");
        put( "\u3072", "N");
        put( "\u3075", "O");
        put( "\u307e", "P");
        put( "\u307f", "Q");
        put( "\u3080", "R");
        put( "\u3081", "S");
        put( "\u3084", "T");
        put( "\u3086", "U");
        put( "\u3088", "V");
        put( "\u3089", "W");
        put( "\u308a", "X");
        put( "\u308b", "Y");
        put( "\u308c", "Z");
        put( "\u308f", "-");
    }};

    /** Conversion rule for full-width alphabet */
    private static final HashMap<String,String> mFullAlphabetMap = new HashMap<String,String>() {{
        put( "\u3042", "\uff0e");
        put( "\u3044", "\uff20");
        put( "\u3046", "\u30fc");
        put( "\u3048", "\uff3f");
        put( "\u304a", "\uff0f");
        put( "\u3041", "\uff1a");
        put( "\u3043", "\u301c");
        put( "\u304b", "\uff21");
        put( "\u304d", "\uff22" );
        put( "\u304f", "\uff23");
        put( "\u3055", "\uff24" );
        put( "\u3057", "\uff25" );
        put( "\u3059", "\uff26" );
        put( "\u305f", "\uff27");
        put( "\u3061", "\uff28" );
        put( "\u3064", "\uff29");
        put( "\u306a", "\uff2a");
        put( "\u306b", "\uff2b" );
        put( "\u306c", "\uff2c" );
        put( "\u306f", "\uff2d");
        put( "\u3072", "\uff2e");
        put( "\u3075", "\uff2f");
        put( "\u307e", "\uff30");
        put( "\u307f", "\uff31");
        put( "\u3080", "\uff32");
        put( "\u3081", "\uff33" );
        put( "\u3084", "\uff34" );
        put( "\u3086", "\uff35" );
        put( "\u3088", "\uff36" );
        put( "\u3089", "\uff37" );
        put( "\u308a", "\uff38" );
        put( "\u308b", "\uff39");
        put( "\u308c", "\uff3a" );
        put( "\u308f", "\u30fc" );
    }};

    /** Conversion rule for full-width alphabet (QWERTY mode) */
    private static final HashMap<String,String> mFullAlphabetMapQwety = new HashMap<String,String>() {{
        put( "a", "\uff41");
        put( "b", "\uff42");
        put( "c", "\uff43");
        put( "d", "\uff44");
        put( "e", "\uff45");
        put( "f", "\uff46");
        put( "g", "\uff47");
        put( "h", "\uff48");
        put( "i", "\uff49");
        put( "j", "\uff4a");
        put( "k", "\uff4b");
        put( "l", "\uff4c");
        put( "m", "\uff4d");
        put( "n", "\uff4e");
        put( "o", "\uff4f");
        put( "p", "\uff50");
        put( "q", "\uff51");
        put( "r", "\uff52");
        put( "s", "\uff53");
        put( "t", "\uff54");
        put( "u", "\uff55");
        put( "v", "\uff56");
        put( "w", "\uff57");
        put( "x", "\uff58");
        put( "y", "\uff59");
        put( "z", "\uff5a");

        put( "A", "\uff21");
        put( "B", "\uff22");
        put( "C", "\uff23");
        put( "D", "\uff24");
        put( "E", "\uff25");
        put( "F", "\uff26");
        put( "G", "\uff27");
        put( "H", "\uff28");
        put( "I", "\uff29");
        put( "J", "\uff2a");
        put( "K", "\uff2b");
        put( "L", "\uff2c");
        put( "M", "\uff2d");
        put( "N", "\uff2e");
        put( "O", "\uff2f");
        put( "P", "\uff30");
        put( "Q", "\uff31");
        put( "R", "\uff32");
        put( "S", "\uff33");
        put( "T", "\uff34");
        put( "U", "\uff35");
        put( "V", "\uff36");
        put( "W", "\uff37");
        put( "X", "\uff38");
        put( "Y", "\uff39");
        put( "Z", "\uff3a");
    }};

    /** Decimal format using comma */
    private static final DecimalFormat mFormat = new DecimalFormat("###,###");

    /** List of the generated candidates */
    private List<WnnWord> mAddCandidateList;
    /** Work area for generating string */
    private StringBuffer mStringBuff;

    /** part of speech (default) */
    private WnnPOS mPosDefault;
    /** part of speech (number) */
    private WnnPOS mPosNumber;
    /** part of speech (symbol) */
    private WnnPOS mPosSymbol;
    
    /**
     * Constructor
     */
    public KanaConverter() {
        mAddCandidateList = new ArrayList<WnnWord>();
        mStringBuff = new StringBuffer();
    }

    /**
     * Set The dictionary.
     * <br>
     * {@link KanaConverter} gets part-of-speech tags from the dictionary.
     * 
     * @param dict  The dictionary
     */
    public void setDictionary(WnnDictionary dict) {
        /* get part of speech tags */
        mPosDefault  = dict.getPOS(WnnDictionary.POS_TYPE_MEISI);
        mPosNumber   = dict.getPOS(WnnDictionary.POS_TYPE_SUUJI);
        mPosSymbol   = dict.getPOS(WnnDictionary.POS_TYPE_KIGOU);
    }

    /**
     * Create the pseudo candidate list
     * <br>
     * @param inputHiragana     The input string (Hiragana)
     * @param inputRomaji       The input string (Romaji)
     * @param keyBoardMode      The mode of keyboard
     * @return                  The candidate list
     */
    public List<WnnWord> createPseudoCandidateList(String inputHiragana, String inputRomaji, int keyBoardMode) {
        List<WnnWord> list = mAddCandidateList;

        list.clear();
        if (inputHiragana.length() == 0) {
        	return list;
        }

        /* Create pseudo candidates for all keyboard type */
        /* Hiragana(reading) / Full width katakana / Half width katakana */
        list.add(new WnnWord(inputHiragana, inputHiragana));
        if (createCandidateString(inputHiragana, mFullKatakanaMap, mStringBuff)) {
            list.add(new WnnWord(mStringBuff.toString(), inputHiragana, mPosDefault));
        }
        if (createCandidateString(inputHiragana, mHalfKatakanaMap, mStringBuff)) {
            list.add(new WnnWord(mStringBuff.toString(), inputHiragana, mPosDefault));
        }

        createPseudoCandidateListForQwerty(inputHiragana, inputRomaji);

        return list;
    }

    /**
     * Create the pseudo candidate list for Qwerty keyboard
     * <br>
     * @param inputHiragana     The input string (Hiragana)
     * @param inputRomaji       The input string (Romaji)
     */
    private void createPseudoCandidateListForQwerty(String inputHiragana, String inputRomaji) {
        List<WnnWord> list = mAddCandidateList;

        /* Create pseudo candidates for half width alphabet */
        String convHanEijiLower = inputRomaji.toLowerCase();
        list.add(new WnnWord(inputRomaji, inputHiragana, mPosDefault));
        list.add(new WnnWord(convHanEijiLower, inputHiragana, mPosSymbol));
        list.add(new WnnWord(convertCaps(convHanEijiLower), inputHiragana, mPosSymbol));
        list.add(new WnnWord(inputRomaji.toUpperCase(), inputHiragana, mPosSymbol));

        /* Create pseudo candidates for the full width alphabet */
        if (createCandidateString(inputRomaji, mFullAlphabetMapQwety, mStringBuff)) {
            String convZenEiji = mStringBuff.toString();
            String convZenEijiLower = convZenEiji.toLowerCase(Locale.JAPAN);
            list.add(new WnnWord(convZenEiji, inputHiragana, mPosSymbol));
            list.add(new WnnWord(convZenEijiLower, inputHiragana, mPosSymbol));
            list.add(new WnnWord(convertCaps(convZenEijiLower), inputHiragana, mPosSymbol));
            list.add(new WnnWord(convZenEiji.toUpperCase(Locale.JAPAN), inputHiragana, mPosSymbol));
        }
    }

    /**
     * Create the candidate string
     * <br>
     * @param input     The input string
     * @param map       The hash map
     * @param outBuf    The output string
     * @return          {@code true} if success
     */
    private boolean createCandidateString(String input, HashMap<String,String> map, StringBuffer outBuf) {
        if (outBuf.length() > 0) {
            outBuf.delete(0, outBuf.length());
        }
        for (int index = 0; index < input.length(); index++) {
            String convChar = map.get(input.substring(index, index + 1));
            if (convChar == null) {
                return false;
            }
            outBuf.append(convChar);
        }
        return true;
    }

    /**
     * Convert into both small and capital letter
     * <br>
     * @param moji  The input string
     * @return      The converted string
     */
    private String convertCaps(String moji) {
        String tmp = "";
        if (moji != null && moji.length() > 0) {
            tmp = moji.substring(0, 1).toUpperCase(Locale.JAPAN)
                    + moji.substring(1).toLowerCase(Locale.JAPAN);
        }
        return tmp;
    }

    /**
     * Convert the numeric into formatted string
     * <br>
     * @param numComma  The value
     * @return          {@code true} if success
     */
    private String convertNumber(String numComma) {
        try {
            return mFormat.format(Double.parseDouble(numComma));
        } catch (NumberFormatException e) {
            return null;
        }
    }
}
