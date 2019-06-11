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
import java.util.*;

/**
 * The penWnn Clause Converter class for Japanese IME.
 * 
 * @author Copyright (C) 2009 OMRON SOFTWARE CO., LTD.  All Rights Reserved.
 */
public class OpenWnnClauseConverterJAJP {
    /** Score(frequency value) of word in the learning dictionary */
    private static final int FREQ_LEARN = 600;
    /** Score(frequency value) of word in the user dictionary */
    private static final int FREQ_USER  = 500;

    /** Maximum limit length of input */
    public static final int MAX_INPUT_LENGTH = 50;

    /** search cache for unique independent words (jiritsugo) */
    private HashMap<String, ArrayList<WnnWord>> mIndepWordBag;
    /** search cache for all independent words (jiritsugo) */
    private HashMap<String, ArrayList<WnnWord>> mAllIndepWordBag;
    /** search cache for ancillary words (fuzokugo) */
    private HashMap<String, ArrayList<WnnWord>> mFzkPatterns;

    /** connect matrix for generating a clause */
    private byte[][] mConnectMatrix;

    /** dictionaries */
    private WnnDictionary mDictionary;

    /** candidates of conversion */
    private LinkedList mConvertResult;

    /** work area for consecutive clause conversion */
    private WnnSentence[] mSentenceBuffer;

    /** part of speech (default) */
    private WnnPOS mPosDefault;
    /** part of speech (end of clause/not end of sentence) */
    private WnnPOS mPosEndOfClause1;
    /** part of speech (end of clause/any place) */
    private WnnPOS mPosEndOfClause2;
    /** part of speech (end of sentence) */
    private WnnPOS mPosEndOfClause3;

    /** cost value of a clause */
    private static final int CLAUSE_COST = -1000;
    
    /** The candidate filter */
    private CandidateFilter mFilter = null;

    /**
     * Constructor
     */
    public OpenWnnClauseConverterJAJP() {
        mIndepWordBag  = new HashMap<String, ArrayList<WnnWord>>();
        mAllIndepWordBag  = new HashMap<String, ArrayList<WnnWord>>();
        mFzkPatterns   = new HashMap();
        mConvertResult = new LinkedList();

        mSentenceBuffer = new WnnSentence[MAX_INPUT_LENGTH];
    }

    /**
     * Set the dictionary
     * 
     * @param dict  The dictionary for phrase conversion
     */
    public void setDictionary(WnnDictionary dict) {
        /* get connect matrix */
        mConnectMatrix = dict.getConnectMatrix();

        /* clear dictionary settings */
        mDictionary = dict;
        dict.clearApproxPattern();

        /* clear work areas */
        mIndepWordBag.clear();
        mAllIndepWordBag.clear();
        mFzkPatterns.clear();
        
        /* get part of speech tags */
        mPosDefault      = dict.getPOS(WnnDictionary.POS_TYPE_MEISI);
        mPosEndOfClause1 = dict.getPOS(WnnDictionary.POS_TYPE_V1);
        mPosEndOfClause2 = dict.getPOS(WnnDictionary.POS_TYPE_V2);
        mPosEndOfClause3 = dict.getPOS(WnnDictionary.POS_TYPE_V3);
    }
    
    /**
     * Set the candidate filter
     * 
     * @param filter	The candidate filter
     */
    public void setFilter(CandidateFilter filter) {
    	mFilter = filter;
    }

    /**
     * Kana-to-Kanji conversion (single clause).
     * <br>
     * This method execute single clause conversion.
      *
     * @param input		The input string
     * @return			The candidates of conversion; {@code null} if an error occurs.
     */
     public Iterator convert(String input) {
        /* do nothing if no dictionary is specified */
        if (mConnectMatrix == null || mDictionary == null) {
            return null;
        }
        /* do nothing if the length of input exceeds the limit */
        if (input.length() > MAX_INPUT_LENGTH) {
            return null;
        }

        /* clear the candidates list */
        mConvertResult.clear();

        /* try single clause conversion */
        if (!singleClauseConvert(mConvertResult, input, mPosEndOfClause2, true)) {
            return null;
        }
        return mConvertResult.iterator();
    }

    /**
     * Consecutive clause conversion.
     *
     * @param input		The input string
     * @return			The result of consecutive clause conversion; {@code null} if fail.
     */
    public WnnSentence consecutiveClauseConvert(String input) {
        LinkedList clauses = new LinkedList();

        /* clear the cache which is not matched */
        for (int i = 0; i < input.length(); i++) {
            mSentenceBuffer[i] = null;
        }
        WnnSentence[] sentence = mSentenceBuffer;

        /* consecutive clause conversion */
        for (int start = 0; start < input.length(); start++) {
            if (start != 0 && sentence[start-1] == null) {
                continue;
            }

            /* limit the length of a clause */
            int end = input.length();
            if (end > start + 20) {
                end = start + 20;
            }
            /* make clauses */
            for ( ; end > start; end--) {
                int idx = end - 1;

                /* cutting a branch */
                if (sentence[idx] != null) {
                    if (start != 0) {
                        if (sentence[idx].frequency > sentence[start-1].frequency + CLAUSE_COST + FREQ_LEARN) {
                            /* there may be no way to be the best sequence from the 'start' */
                            break;
                        }
                    } else {
                        if (sentence[idx].frequency > CLAUSE_COST + FREQ_LEARN) {
                            /* there may be no way to be the best sequence from the 'start' */
                            break;
                        }
                    }
                }

                String key = input.substring(start, end);
                clauses.clear();
                WnnClause bestClause = null;
                if (end == input.length()) {
                    /* get the clause which can be the end of the sentence */
                    singleClauseConvert(clauses, key, mPosEndOfClause1, false);
                } else {
                    /* get the clause which is not the end of the sentence */
                    singleClauseConvert(clauses, key, mPosEndOfClause3, false);
                }
                if (clauses.isEmpty()) {
                    bestClause = defaultClause(key);
                } else {
                    bestClause = (WnnClause)clauses.get(0);
                }

                /* make a sub-sentence */
                WnnSentence ws;
                if (start == 0) {
                    ws = new WnnSentence(key, bestClause);
                } else {
                    ws = new WnnSentence(sentence[start-1], bestClause);
                }
                ws.frequency += CLAUSE_COST;

                /* update the best sub-sentence on the cache buffer */
                if (sentence[idx] == null || (sentence[idx].frequency < ws.frequency)) {
                    sentence[idx] = ws;
                }
            }
        }

        /* return the result of the consecutive clause conversion */
        if (sentence[input.length() - 1] != null) {
            return sentence[input.length() - 1];
        }
        return null;
    }

    /**
     * Consecutive clause conversion.
     *
     * @param resultList	Where to store the result
     * @param input			Input string
     * @return				{@code true} if success; {@code false} if fail.
     */
    private boolean consecutiveClauseConvert(LinkedList resultList, String input) {
        WnnSentence sentence = consecutiveClauseConvert(input);

        /* set the result of the consecutive clause conversion on the top of the list */
        if (sentence != null) {
            resultList.add(0, sentence);
            return true;
        }
        return false;
    }

    /**
     * Single clause conversion.
     *
     * @param clauseList	Where to store the results
     * @param input			Input string
     * @param terminal		Part of speech tag at the terminal
     * @param all			Get all candidates or not
     * @return				{@code true} if success; {@code false} if fail.
     */
    private boolean singleClauseConvert(LinkedList clauseList, String input, WnnPOS terminal, boolean all) {
        boolean ret = false;

        /* get clauses without ancillary word */
        ArrayList<WnnWord> stems = getIndependentWords(input, all);
        if (stems != null && (!stems.isEmpty())) {
            Iterator<WnnWord> stemsi = stems.iterator();
            while (stemsi.hasNext()) {
                WnnWord stem = stemsi.next();
                if (addClause(clauseList, input, stem, null, terminal, all)) {
                    ret = true;
                }
            }
        }

        /* get clauses with ancillary word */
        int max = CLAUSE_COST * 2;
        for (int split = 1; split < input.length(); split++) {
            /* get ancillary patterns */
            String str = input.substring(split);
            ArrayList<WnnWord> fzks = getAncillaryPattern(str);
            if (fzks == null || fzks.isEmpty()) {
                continue;
            }
            
            /* get candidates of stem in a clause */
            str = input.substring(0, split);
            stems = getIndependentWords(str, all);
            if (stems == null || stems.isEmpty()) {
                if (mDictionary.searchWord(WnnDictionary.SEARCH_PREFIX, WnnDictionary.ORDER_BY_FREQUENCY, str) <= 0) {
                    break;
                } else {
                    continue;
                }
            }
            /* make clauses */
            Iterator<WnnWord> stemsi = stems.iterator();
            while (stemsi.hasNext()) {
                WnnWord stem = stemsi.next();
                if (all || stem.frequency > max) {
                    Iterator<WnnWord> fzksi  = fzks.iterator();
                    while (fzksi.hasNext()) {
                        WnnWord fzk = fzksi.next();
                        if (addClause(clauseList, input, stem, fzk, terminal, all)) {
                            ret = true;
                            max = stem.frequency;
                        }
                    }
                }
            }
        }
        return ret;
    }

    /**
     * Add valid clause to the candidates list.
     *
     * @param clauseList	Where to store the results
     * @param input			Input string
     * @param stem			Stem of the clause (a independent word)
     * @param fzk			Ancillary pattern
     * @param terminal		Part of speech tag at the terminal
     * @param all			Get all candidates or not
     * @return				{@code true} if add the clause to the list; {@code false} if not.
     */
    private boolean addClause(LinkedList<WnnClause> clauseList, String input, WnnWord stem, WnnWord fzk,
                              WnnPOS terminal, boolean all) {
        WnnClause clause = null;
        /* check if the part of speech is valid */
        if (fzk == null) {
            if (connectible(stem.partOfSpeech.right, terminal.left)) {
                clause = new WnnClause(input, stem);
            }
        } else {
            if (connectible(stem.partOfSpeech.right, fzk.partOfSpeech.left)
                && connectible(fzk.partOfSpeech.right, terminal.left)) {
                clause = new WnnClause(input, stem, fzk);
            }
        }
        if (clause == null) {
            return false;
        }
        if (mFilter != null && !mFilter.isAllowed(clause)) {
        	return false;
        }

        /* store to the list */
        if (clauseList.isEmpty()) {
            /* add if the list is empty */
            clauseList.add(0, clause);
            return true;
        } else {
            if (!all) {
                /* reserve only the best clause */
                WnnClause best = clauseList.get(0);
                if (best.frequency < clause.frequency) {
                    clauseList.set(0, clause);
                    return true;
                }
            } else {
                /* reserve all clauses */
                Iterator clauseListi = clauseList.iterator();
                int index = 0;
                while (clauseListi.hasNext()) {
                    WnnClause clausei = (WnnClause)clauseListi.next();
                    if (clausei.frequency < clause.frequency) {
                        break;
                    }
                    index++;
                }
                clauseList.add(index, clause);
                return true;
            }
        }

        return false;
    }

    /**
     * Check the part-of-speeches are connectable.
     *
     * @param right		Right attribute of the preceding word/clause
     * @param left		Left attribute of the following word/clause
     * @return			{@code true} if there are connectable; {@code false} if otherwise
     */
    private boolean connectible(int right, int left) {
        try {
            if (mConnectMatrix[left][right] != 0) {
                return true;
            }
        } catch (Exception ex) {
        }
        return false;
    }

    /**
     * Get all exact matched ancillary words(Fuzokugo) list.
     *
     * @param input		Search key
     * @return			List of ancillary words
     */
    private ArrayList<WnnWord> getAncillaryPattern(String input) {
        if (input.length() == 0) {
            return null;
        }

        HashMap<String,ArrayList<WnnWord>> fzkPat = mFzkPatterns;
        ArrayList<WnnWord> fzks = fzkPat.get(input);
        if (fzks != null) {
            return fzks;
        }

        /* set dictionaries */
        WnnDictionary dict = mDictionary;
        dict.clearApproxPattern();
        dict.setDictionary(6, 400, 500);

        for (int start = input.length() - 1; start >= 0; start--) {
            String key = input.substring(start);

            fzks = fzkPat.get(key);
            if (fzks != null) {
                continue;
            }

            fzks = new ArrayList<WnnWord>();
            mFzkPatterns.put(key, fzks);

            /* search ancillary words */
            dict.searchWord(WnnDictionary.SEARCH_EXACT, WnnDictionary.ORDER_BY_FREQUENCY, key);
            WnnWord word;
            while ((word = dict.getNextWord()) != null) {
                fzks.add(word);
            }

            /* concatenate sequence of ancillary words */
            for (int end = input.length() - 1; end > start; end--) {
                ArrayList<WnnWord> followFzks = fzkPat.get(input.substring(end));
                if (followFzks == null ||  followFzks.isEmpty()) {
                    continue;
                }
                dict.searchWord(WnnDictionary.SEARCH_EXACT, WnnDictionary.ORDER_BY_FREQUENCY, input.substring(start, end));
                while ((word = dict.getNextWord()) != null) {
                    Iterator<WnnWord> followFzksi = followFzks.iterator();
                    while (followFzksi.hasNext()) {
                        WnnWord follow = followFzksi.next();
                        if (connectible(word.partOfSpeech.right, follow.partOfSpeech.left)) {
                            fzks.add(new WnnWord(key, key, new WnnPOS(word.partOfSpeech.left, follow.partOfSpeech.right)));
                        }
                    }
                }
            }
        }
        return fzks;
    }

    /**
     * Get all exact matched independent words(Jiritsugo) list.
     *
     * @param input    Search key
     * @param all      {@code true} if list all words; {@code false} if list words which has an unique part of speech tag.
     * @return			List of words; {@code null} if {@code input.length() == 0}.
     */
    private ArrayList<WnnWord> getIndependentWords(String input, boolean all) {
        if (input.length() == 0) {
            return null;
        }

        ArrayList<WnnWord> words = (all)? mAllIndepWordBag.get(input) : mIndepWordBag.get(input);
        
        if (words == null) {
            /* set dictionaries */
            WnnDictionary dict = mDictionary;
            dict.clearApproxPattern();
            dict.setDictionary(4, 0, 10);   
            dict.setDictionary(5, 400, 500);
            dict.setDictionary(WnnDictionary.INDEX_USER_DICTIONARY, FREQ_USER, FREQ_USER); 
            dict.setDictionary(WnnDictionary.INDEX_LEARN_DICTIONARY, FREQ_LEARN, FREQ_LEARN);

            words = new ArrayList<WnnWord>();
            WnnWord word;
            if (all) {
            	mAllIndepWordBag.put(input, words);
                dict.searchWord(WnnDictionary.SEARCH_EXACT, WnnDictionary.ORDER_BY_FREQUENCY, input);
                /* store all words */
                while ((word = dict.getNextWord()) != null) {
                    if (input.equals(word.stroke)) {
                        words.add(word);
                    }
                }
            } else {
            	mIndepWordBag.put(input, words);
                dict.searchWord(WnnDictionary.SEARCH_EXACT, WnnDictionary.ORDER_BY_FREQUENCY, input);
                /* store a word which has an unique part of speech tag */
                while ((word = dict.getNextWord()) != null) {
                    if (input.equals(word.stroke)) {
                        Iterator<WnnWord> list = words.iterator();
                        boolean found = false;
                        while (list.hasNext()) {
                            WnnWord w = list.next();
                                if (w.partOfSpeech.right == word.partOfSpeech.right) {
                                    found = true;
                                    break;
                                }
                        }
                        if (!found) {
                            words.add(word);
                        }
                        if (word.frequency < 400) {
                            break;
                        }
                    }
                }
            }
            addAutoGeneratedCandidates(input, words, all);
        }
        return words;
    }
    
    /**
     * Add some words not including in the dictionary.
     * <br>
     * This method adds some words which are not in the dictionary.
     *
     * @param input     Input string
     * @param wordList  List to store words
     * @param all       Get all candidates or not
     */
    private void addAutoGeneratedCandidates(String input, ArrayList wordList, boolean all) {
        wordList.add(new WnnWord(input, input, mPosDefault, (CLAUSE_COST - 1) * input.length()));
    }

    /**
     * Get a default clause.
     * <br>
     * This method generates a clause which has a string same as input
     * and the default part-of-speech tag.
     *
     * @param input    Input string
     * @return			Default clause
     */
    private WnnClause defaultClause(String input) {
        return (new WnnClause(input, input, mPosDefault, (CLAUSE_COST - 1) * input.length()));
    }
}