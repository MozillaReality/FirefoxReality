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

import android.content.SharedPreferences;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

import jp.co.omronsoft.openwnn.CandidateFilter;
import jp.co.omronsoft.openwnn.ComposingText;
import jp.co.omronsoft.openwnn.OpenWnnDictionaryImpl;
import jp.co.omronsoft.openwnn.StrSegmentClause;
import jp.co.omronsoft.openwnn.WnnClause;
import jp.co.omronsoft.openwnn.WnnDictionary;
import jp.co.omronsoft.openwnn.WnnEngine;
import jp.co.omronsoft.openwnn.WnnSentence;
import jp.co.omronsoft.openwnn.WnnWord;

/**
 * The OpenWnn engine class for Japanese IME.
 * 
 * @author Copyright (C) 2009-2011 OMRON SOFTWARE CO., LTD.  All Rights Reserved.
 */
public class OpenWnnEngineJAJP implements WnnEngine {
    /** Current dictionary type */
    private int mDictType = DIC_LANG_INIT;
    /** Dictionary type (default) */
    public static final int DIC_LANG_INIT = 0;
    /** Dictionary type (Japanese standard) */
    public static final int DIC_LANG_JP = 0;
    /** Dictionary type (English standard) */
    public static final int DIC_LANG_EN = 1;
    /** Dictionary type (Japanese person's name) */
    public static final int DIC_LANG_JP_PERSON_NAME = 2;
    /** Dictionary type (User dictionary) */
    public static final int DIC_USERDIC = 3;
    /** Dictionary type (Japanese EISU-KANA conversion) */
    public static final int DIC_LANG_JP_EISUKANA = 4;
    /** Dictionary type (e-mail/URI) */
    public static final int DIC_LANG_EN_EMAIL_ADDRESS = 5;
    /** Dictionary type (Japanese postal address) */
    public static final int DIC_LANG_JP_POSTAL_ADDRESS = 6;

    /** Type of the keyboard */
    private int mKeyboardType = KEYBOARD_UNDEF;
    /** Keyboard type (not defined) */
    public static final int KEYBOARD_UNDEF = 0;
    /** Keyboard type (12-keys) */
    public static final int KEYBOARD_KEYPAD12 = 1;
    /** Keyboard type (Qwerty) */
    public static final int KEYBOARD_QWERTY = 2;
    
    /** Score(frequency value) of word in the learning dictionary */
    public static final int FREQ_LEARN = 600;
    /** Score(frequency value) of word in the user dictionary */
    public static final int FREQ_USER = 500;

    /** Maximum limit length of output */
    public static final int MAX_OUTPUT_LENGTH = 50;
    /** Limitation of predicted candidates */
    public static final int PREDICT_LIMIT = 100;

    /** OpenWnn dictionary */
    private WnnDictionary mDictionaryJP;

    /** Word list */
    private ArrayList<WnnWord> mConvResult;

    /** HashMap for checking duplicate word */
    private HashMap<String, WnnWord> mCandTable;

    /** Input string (Hiragana) */
    private String mInputHiragana;
    
    /** Input string (Romaji) */
    private String mInputRomaji;
    
    /** Number of output candidates */
    private int mOutputNum;
    
    /**
     * Where to get the next candidates from.<br>
     * (0:prefix search from the dictionary, 1:single clause converter, 2:Kana converter)
     */
    private int mGetCandidateFrom;
    
    /** Previously selected word */
    private WnnWord mPreviousWord;

    /** Converter for single/consecutive clause conversion */
    private OpenWnnClauseConverterJAJP mClauseConverter;

    /** Kana converter (for EISU-KANA conversion) */
    private KanaConverter mKanaConverter;

    /** Whether exact match search or prefix match search */
    private boolean mExactMatchMode;

    /** Whether displaying single clause candidates or not */
    private boolean mSingleClauseMode;

    /** A result of consecutive clause conversion */
    private WnnSentence mConvertSentence;
    
    /** The candidate filter */
    private CandidateFilter mFilter = null;

    /**
     * Constructor
     * 
     */
    public OpenWnnEngineJAJP() {
        /* load Japanese dictionary library */
        mDictionaryJP = new OpenWnnDictionaryImpl("libwnnjpndic.so" );
        if (!mDictionaryJP.isActive()) {
            mDictionaryJP = new OpenWnnDictionaryImpl("/system/lib/libwnnjpndic.so" );
        }

        /* clear dictionary settings */
        mDictionaryJP.clearApproxPattern();

        /* work buffers */
        mConvResult = new ArrayList<>();
        mCandTable = new HashMap<>();

        /* converters */
        mClauseConverter = new OpenWnnClauseConverterJAJP();
        mKanaConverter = new KanaConverter();
    }

    /**
     * Set dictionary for prediction.
     * 
     * @param strlen        Length of input string
     */
    private void setDictionaryForPrediction(int strlen) {
        WnnDictionary dict = mDictionaryJP;

        if (mDictType != DIC_LANG_JP_EISUKANA) {
            dict.clearApproxPattern();
            if (strlen == 0) {
                dict.setDictionary(2, 245, 245);
                dict.setDictionary(3, 100, 244);
                
                dict.setDictionary(WnnDictionary.INDEX_LEARN_DICTIONARY, FREQ_LEARN, FREQ_LEARN);
            } else {
                dict.setDictionary(0, 100, 400);
                if (strlen > 1) {
                    dict.setDictionary(1, 100, 400);
                }
                dict.setDictionary(2, 245, 245);
                dict.setDictionary(3, 100, 244);
                
                dict.setDictionary(WnnDictionary.INDEX_USER_DICTIONARY, FREQ_USER, FREQ_USER);
                dict.setDictionary(WnnDictionary.INDEX_LEARN_DICTIONARY, FREQ_LEARN, FREQ_LEARN);
                if (mKeyboardType != KEYBOARD_QWERTY) {
                    dict.setApproxPattern(WnnDictionary.APPROX_PATTERN_JAJP_12KEY_NORMAL);
                }
            }
        }
    }

    /**
     * Get a candidate.
     *
     * @param index     Index of a candidate.
     * @return          The candidate; {@code null} if there is no candidate.
     */
    private WnnWord getCandidate(int index) {
        WnnWord word;

        if (mGetCandidateFrom == 0) {
            if (mDictType == OpenWnnEngineJAJP.DIC_LANG_JP_EISUKANA) {
                /* skip to Kana conversion if EISU-KANA conversion mode */
                mGetCandidateFrom = 2;
            } else if (mSingleClauseMode) {
                /* skip to single clause conversion if single clause conversion mode */
                mGetCandidateFrom = 1;
            } else {
                if (mConvResult.size() < PREDICT_LIMIT) {
                    /* get prefix matching words from the dictionaries */
                    while (index >= mConvResult.size()) {
                        if ((word = mDictionaryJP.getNextWord()) == null) {
                            mGetCandidateFrom = 1;
                            break;
                        }
                        if (!mExactMatchMode || mInputHiragana.equals(word.stroke)) {
                            addCandidate(word);
                            if (mConvResult.size() >= PREDICT_LIMIT) {
                                mGetCandidateFrom = 1;
                                break;
                            }
                        }
                    }
                } else {
                    mGetCandidateFrom = 1;
                }
            }
        }

        /* get candidates by single clause conversion */
        if (mGetCandidateFrom == 1) {
            Iterator<?> convResult = mClauseConverter.convert(mInputHiragana);
            if (convResult != null) {
                while (convResult.hasNext()) {
                    addCandidate((WnnWord)convResult.next());
                }
            }
            /* end of candidates by single clause conversion */
            mGetCandidateFrom = 2;
        }
        
        /* get candidates from Kana converter */
        if (mGetCandidateFrom == 2) {
            List<WnnWord> addCandidateList
            = mKanaConverter.createPseudoCandidateList(mInputHiragana, mInputRomaji, mKeyboardType);
            
            Iterator<WnnWord> it = addCandidateList.iterator();
            while(it.hasNext()) {
                addCandidate(it.next());
            }

            mGetCandidateFrom = 3;
        }

        if (index >= mConvResult.size()) {
            return null;
        }
        return mConvResult.get(index);
    }

    /**
     * Add a candidate to the conversion result buffer.
     * <br>
     * This method adds a word to the result buffer if there is not
     * the same one in the buffer and the length of the candidate
     * string is not longer than {@code MAX_OUTPUT_LENGTH}.
     *
     * @param word      A word to be add
     * @return          {@code true} if the word added; {@code false} if not.
     */
    private boolean addCandidate(WnnWord word) {
        if (word.candidate == null || mCandTable.containsKey(word.candidate)
                || word.candidate.length() > MAX_OUTPUT_LENGTH) {
            return false;
        }
        if (mFilter != null && !mFilter.isAllowed(word)) {
            return false;
        }
        mCandTable.put(word.candidate, word);
        mConvResult.add(word);
        return true;
    }

    /**
     * Clear work area that hold candidates information.
     */
    private void clearCandidates() {
        mConvResult.clear();
        mCandTable.clear();
        mOutputNum = 0;
        mInputHiragana = null;
        mInputRomaji = null;
        mGetCandidateFrom = 0;
        mSingleClauseMode = false;
    }

    /**
     * Set dictionary type.
     *
     * @param type      Type of dictionary
     * @return          {@code true} if the dictionary is changed; {@code false} if not.
     */
    public boolean setDictionary(int type) {
        mDictType = type;
        return true;
    }

    /**
     * Set the search key and the search mode from {@link ComposingText}.
     *
     * @param text      Input text
     * @param maxLen    Maximum length to convert
     * @return          Length of the search key
     */
    private int setSearchKey(ComposingText text, int maxLen) {
        String input = text.toString(ComposingText.LAYER1);
        if (0 <= maxLen && maxLen <= input.length()) {
            input = input.substring(0, maxLen);
            mExactMatchMode = true;
        } else {
            mExactMatchMode = false;
        }

        if (input.length() == 0) {
            mInputHiragana = "";
            mInputRomaji = "";
            return 0;
        }

        mInputHiragana = input;
        mInputRomaji = text.toString(ComposingText.LAYER0);

        return input.length();
    }

    /**
     * Clear the previous word's information.
     */
    public void clearPreviousWord() {
        mPreviousWord = null;
    }

    /**
     * Set keyboard type.
     * 
     * @param keyboardType      Type of keyboard
     */
    public void setKeyboardType(int keyboardType) {
        mKeyboardType = keyboardType;
    }

    /**
     * Set the candidate filter
     * 
     * @param filter    The candidate filter
     */
    public void setFilter(CandidateFilter filter) {
        mFilter = filter;
        mClauseConverter.setFilter(filter);
    }
    
    /***********************************************************************
     * WnnEngine's interface
     **********************************************************************/
    /** @see WnnEngine#init */
    public void init() {
        clearPreviousWord();
        mClauseConverter.setDictionary(mDictionaryJP);
        mKanaConverter.setDictionary(mDictionaryJP);
    }

    /** @see WnnEngine#close */
    public void close() {}

    /** @see WnnEngine#predict */
    public int predict(ComposingText text, int minLen, int maxLen) {
        clearCandidates();
        if (text == null) { return 0; }

        /* set mInputHiragana and mInputRomaji */
        int len = setSearchKey(text, maxLen);

        /* set dictionaries by the length of input */
        setDictionaryForPrediction(len);

        if (len == 0) {
            /* search by previously selected word */
            return mDictionaryJP.searchWord(WnnDictionary.SEARCH_LINK, WnnDictionary.ORDER_BY_FREQUENCY,
                                            mInputHiragana, mPreviousWord);
        } else {
            if (mExactMatchMode) {
                /* exact matching */
                mDictionaryJP.searchWord(WnnDictionary.SEARCH_EXACT, WnnDictionary.ORDER_BY_FREQUENCY,
                                         mInputHiragana);
            } else {
                /* prefix matching */
                mDictionaryJP.searchWord(WnnDictionary.SEARCH_PREFIX, WnnDictionary.ORDER_BY_FREQUENCY,
                                         mInputHiragana);
            }
            return 1;
        }
    }

    /** @see WnnEngine#convert */
    public int convert(ComposingText text) {
        clearCandidates();

        if (text == null) {
            return 0;
        }

        int cursor = text.getCursor(ComposingText.LAYER1);
        String input;
        WnnClause head = null;
        if (cursor > 0) {
            /* convert previous part from cursor */
            input = text.toString(ComposingText.LAYER1, 0, cursor - 1);
            Iterator headCandidates = mClauseConverter.convert(input);
            if ((headCandidates == null) || (!headCandidates.hasNext())) {
                return 0;
            }
            head = new WnnClause(input, (WnnWord)headCandidates.next());

            /* set the rest of input string */
            input = text.toString(ComposingText.LAYER1, cursor, text.size(ComposingText.LAYER1) - 1);
        } else {
            /* set whole of input string */
            input = text.toString(ComposingText.LAYER1);
        }

        WnnSentence sentence = null;
        if (input.length() != 0) {
            sentence = mClauseConverter.consecutiveClauseConvert(input);
        }
        if (head != null) {
            sentence = new WnnSentence(head, sentence);
        }
        if (sentence == null) {
            return 0;
        }

        StrSegmentClause[] ss = new StrSegmentClause[sentence.elements.size()];
        int pos = 0;
        int idx = 0;
        Iterator<WnnClause> it = sentence.elements.iterator();
        while(it.hasNext()) {
            WnnClause clause = it.next();
            int len = clause.stroke.length();
            ss[idx] = new StrSegmentClause(clause, pos, pos + len - 1);
            pos += len;
            idx += 1;
        }
        text.setCursor(ComposingText.LAYER2, text.size(ComposingText.LAYER2));
        text.replaceStrSegment(ComposingText.LAYER2, ss, 
                               text.getCursor(ComposingText.LAYER2));
        mConvertSentence = sentence;

        return 0;
    }
    
    /** @see WnnEngine#searchWords */
    public int searchWords(String key) {
        clearCandidates();
        return 0;
    }

    /** @see WnnEngine#getNextCandidate */
    public WnnWord getNextCandidate() {
        if (mInputHiragana == null) {
            return null;
        }
        WnnWord word = getCandidate(mOutputNum);
        if (word != null) {
            mOutputNum++;
        }
        return word;
    }

    /** @see WnnEngine#setPreferences */
    public void setPreferences(SharedPreferences pref) {}

    /** @see WnnEngine#breakSequence */
    public void breakSequence()  {
        clearPreviousWord();
    }

    /** @see WnnEngine#makeCandidateListOf */
    public int makeCandidateListOf(int clausePosition)  {
        clearCandidates();

        if ((mConvertSentence == null) || (mConvertSentence.elements.size() <= clausePosition)) {
            return 0;
        }
        mSingleClauseMode = true;
        WnnClause clause = mConvertSentence.elements.get(clausePosition);
        mInputHiragana = clause.stroke;
        mInputRomaji = clause.candidate;

        return 1;
    }

}
