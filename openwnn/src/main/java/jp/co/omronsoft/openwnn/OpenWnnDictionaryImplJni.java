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

/**
 * The implementation class of JNI wrapper for dictionary.
 *
 * @author Copyright (C) 2008, 2009 OMRON SOFTWARE CO., LTD.  All Rights Reserved.
 */
public class OpenWnnDictionaryImplJni {
    /*
     * DEFINITION OF CONSTANTS
     */
    /**
     * Constant about the approximate pattern (for JNI native library)
     *
     * @see jp.co.omronsoft.openwnn.WnnDictionary#APPROX_PATTERN_EN_TOUPPER
     * @see OpenWnnDictionaryImplJni#setApproxPattern
     */
    public static final int APPROX_PATTERN_EN_TOUPPER               = WnnDictionary.APPROX_PATTERN_EN_TOUPPER;
    /**
     * Constant about the approximate pattern (for JNI native library)
     *
     * @see jp.co.omronsoft.openwnn.WnnDictionary#APPROX_PATTERN_EN_TOLOWER
     * @see OpenWnnDictionaryImplJni#setApproxPattern
     */
    public static final int APPROX_PATTERN_EN_TOLOWER               = WnnDictionary.APPROX_PATTERN_EN_TOLOWER;
    /**
     * Constant about the approximate pattern (for JNI native library)
     *
     * @see jp.co.omronsoft.openwnn.WnnDictionary#APPROX_PATTERN_EN_QWERTY_NEAR
     * @see OpenWnnDictionaryImplJni#setApproxPattern
     */
    public static final int APPROX_PATTERN_EN_QWERTY_NEAR           = WnnDictionary.APPROX_PATTERN_EN_QWERTY_NEAR;
    /**
     * Constant about the approximate pattern (for JNI native library)
     *
     * @see jp.co.omronsoft.openwnn.WnnDictionary#APPROX_PATTERN_EN_QWERTY_NEAR_UPPER
     * @see OpenWnnDictionaryImplJni#setApproxPattern
     */
    public static final int APPROX_PATTERN_EN_QWERTY_NEAR_UPPER     = WnnDictionary.APPROX_PATTERN_EN_QWERTY_NEAR_UPPER;
    /**
     * Constant about the approximate pattern (for JNI native library)
     *
     * @see jp.co.omronsoft.openwnn.WnnDictionary#APPROX_PATTERN_JAJP_12KEY_NORMAL
     * @see OpenWnnDictionaryImplJni#setApproxPattern
     */
    public static final int APPROX_PATTERN_JAJP_12KEY_NORMAL        = WnnDictionary.APPROX_PATTERN_JAJP_12KEY_NORMAL;

    /**
     * Constant about the search operation (for JNI native library)
     *
     * @see jp.co.omronsoft.openwnn.WnnDictionary#SEARCH_EXACT
     * @see OpenWnnDictionaryImplJni#searchWord
     */
    public static final int SEARCH_EXACT                           = WnnDictionary.SEARCH_EXACT;
    /**
     * Constant about the search operation (for JNI native library)
     *
     * @see jp.co.omronsoft.openwnn.WnnDictionary#SEARCH_PREFIX
     * @see OpenWnnDictionaryImplJni#searchWord
     */
    public static final int SEARCH_PREFIX                          = WnnDictionary.SEARCH_PREFIX;
    /**
     * Constant about the search operation (for JNI native library)
     *
     * @see jp.co.omronsoft.openwnn.WnnDictionary#SEARCH_LINK
     * @see OpenWnnDictionaryImplJni#searchWord
     */
    public static final int SEARCH_LINK                            = WnnDictionary.SEARCH_LINK;

    /**
     * Constant about the sort order (for JNI native library)
     *
     * @see jp.co.omronsoft.openwnn.WnnDictionary#ORDER_BY_FREQUENCY
     * @see OpenWnnDictionaryImplJni#searchWord
     */
    public static final int ORDER_BY_FREQUENCY                     = WnnDictionary.ORDER_BY_FREQUENCY;
    /**
     * Constant about the sort order (for JNI native library)
     *
     * @see jp.co.omronsoft.openwnn.WnnDictionary#ORDER_BY_KEY
     * @see OpenWnnDictionaryImplJni#searchWord
     */
    public static final int ORDER_BY_KEY                           = WnnDictionary.ORDER_BY_KEY;

    /**
     * Type of a part of speech (for JNI native library)
     * @see jp.co.omronsoft.openwnn.WnnDictionary#POS_TYPE_V1
     * @see OpenWnnDictionaryImplJni#getLeftPartOfSpeechSpecifiedType
     * @see OpenWnnDictionaryImplJni#getRightPartOfSpeechSpecifiedType
     */
    public static final int POS_TYPE_V1                             = WnnDictionary.POS_TYPE_V1;
    /**
     * Type of a part of speech (for JNI native library)
     * @see jp.co.omronsoft.openwnn.WnnDictionary#POS_TYPE_V2
     * @see OpenWnnDictionaryImplJni#getLeftPartOfSpeechSpecifiedType
     * @see OpenWnnDictionaryImplJni#getRightPartOfSpeechSpecifiedType
     */
    public static final int POS_TYPE_V2                             = WnnDictionary.POS_TYPE_V2;
    /**
     * Type of a part of speech (for JNI native library)
     * @see jp.co.omronsoft.openwnn.WnnDictionary#POS_TYPE_V3
     * @see OpenWnnDictionaryImplJni#getLeftPartOfSpeechSpecifiedType
     * @see OpenWnnDictionaryImplJni#getRightPartOfSpeechSpecifiedType
     */
    public static final int POS_TYPE_V3                             = WnnDictionary.POS_TYPE_V3;
    /**
     * Type of a part of speech (for JNI native library)
     * @see jp.co.omronsoft.openwnn.WnnDictionary#POS_TYPE_BUNTOU
     * @see OpenWnnDictionaryImplJni#getLeftPartOfSpeechSpecifiedType
     * @see OpenWnnDictionaryImplJni#getRightPartOfSpeechSpecifiedType
     */
    public static final int POS_TYPE_BUNTOU                         = WnnDictionary.POS_TYPE_BUNTOU;
    /**
     * Type of a part of speech (for JNI native library)
     * @see jp.co.omronsoft.openwnn.WnnDictionary#POS_TYPE_TANKANJI
     * @see OpenWnnDictionaryImplJni#getLeftPartOfSpeechSpecifiedType
     * @see OpenWnnDictionaryImplJni#getRightPartOfSpeechSpecifiedType
     */
    public static final int POS_TYPE_TANKANJI                       = WnnDictionary.POS_TYPE_TANKANJI;
    /**
     * Type of a part of speech (for JNI native library)
     * @see jp.co.omronsoft.openwnn.WnnDictionary#POS_TYPE_SUUJI
     * @see OpenWnnDictionaryImplJni#getLeftPartOfSpeechSpecifiedType
     * @see OpenWnnDictionaryImplJni#getRightPartOfSpeechSpecifiedType
     */
    public static final int POS_TYPE_SUUJI                          = WnnDictionary.POS_TYPE_SUUJI;
    /**
     * Type of a part of speech (for JNI native library)
     * @see jp.co.omronsoft.openwnn.WnnDictionary#POS_TYPE_MEISI
     * @see OpenWnnDictionaryImplJni#getLeftPartOfSpeechSpecifiedType
     * @see OpenWnnDictionaryImplJni#getRightPartOfSpeechSpecifiedType
     */
    public static final int POS_TYPE_MEISI                          = WnnDictionary.POS_TYPE_MEISI;
    /**
     * Type of a part of speech (for JNI native library)
     * @see jp.co.omronsoft.openwnn.WnnDictionary#POS_TYPE_JINMEI
     * @see OpenWnnDictionaryImplJni#getLeftPartOfSpeechSpecifiedType
     * @see OpenWnnDictionaryImplJni#getRightPartOfSpeechSpecifiedType
     */
    public static final int POS_TYPE_JINMEI                         = WnnDictionary.POS_TYPE_JINMEI;
    /**
     * Type of a part of speech (for JNI native library)
     * @see jp.co.omronsoft.openwnn.WnnDictionary#POS_TYPE_CHIMEI
     * @see OpenWnnDictionaryImplJni#getLeftPartOfSpeechSpecifiedType
     * @see OpenWnnDictionaryImplJni#getRightPartOfSpeechSpecifiedType
     */
    public static final int POS_TYPE_CHIMEI                         = WnnDictionary.POS_TYPE_CHIMEI;
    /**
     * Type of a part of speech (for JNI native library)
     * @see jp.co.omronsoft.openwnn.WnnDictionary#POS_TYPE_KIGOU
     * @see OpenWnnDictionaryImplJni#getLeftPartOfSpeechSpecifiedType
     * @see OpenWnnDictionaryImplJni#getRightPartOfSpeechSpecifiedType
     */
    public static final int POS_TYPE_KIGOU                          = WnnDictionary.POS_TYPE_KIGOU;

    /*
     * METHODS
     */
    /**
     * Create a internal work area. 
     * A internal work area is allocated dynamically, and the specified dictionary library is loaded.
     *
     * @param dicLibPath    The path of the dictionary library file
     * @return              The internal work area or null
     */
    public static final native long createWnnWork( String dicLibPath );

    /**
     * Free the internal work area.
     * The specified work area and the loaded dictionary library is free.
     *
     * @param work      The internal work area
     * @return          0 if processing is successful; <0 if an error occur
     */
    public static final native int freeWnnWork( long work );

    /**
     * Clear all dictionary information.
     *
     * @see jp.co.omronsoft.openwnn.WnnDictionary#clearDictionary
     * @param work      The internal work area
     * @return          0 if processing is successful; <0 if an error occur
     */
    public static final native int clearDictionaryParameters( long work );

    /**
     * Set a dictionary information.
     *
     * @see jp.co.omronsoft.openwnn.WnnDictionary#setDictionary
     * @param work      The internal work area
     * @param index     The index of dictionary
     * @param base      The base frequency or -1
     * @param high      The maximum frequency or -1
     * @return           0 if processing is successful; <0 otherwise
     */
    public static final native int setDictionaryParameter( long work, int index, int base, int high );

    /**
     * Search a word from dictionaries.
     *
     * @see jp.co.omronsoft.openwnn.WnnDictionary#searchWord
     * @param work          The internal work area
     * @param operation     The search operation (see "Constant about the search operation")
     * @see jp.co.omronsoft.openwnn.WnnDictionary#SEARCH_EXACT
     * @see jp.co.omronsoft.openwnn.WnnDictionary#SEARCH_PREFIX
     * @param order         The sort order (see "Constant about the sort order")
     * @see jp.co.omronsoft.openwnn.WnnDictionary#ORDER_BY_FREQUENCY
     * @see jp.co.omronsoft.openwnn.WnnDictionary#ORDER_BY_KEY
     * @param keyString     The key string
     * @return              0 if no result is found; 1 if a result is found; <0 if an error occur
     *
     */
    public static final native int searchWord(long work, int operation, int order, String keyString );

    /**
     * Retrieve a word information.
     * A word information is stored to the internal work area. To retrieve a detail information,
     * use {@code getStroke()}, {@code getCandidate()}, {@code getFreqeuency(),} or other {@code get...()} method.
     *
     * @see jp.co.omronsoft.openwnn.WnnDictionary#getNextWord
     * @param work      The internal work area
     * @param length    >0 if only the result of specified length is retrieved; 0 if no condition exist
     * @return          0 if no result is retrieved; >0 if a result is retrieved; <0 if an error occur
     */
    public static final native int getNextWord( long work, int length );

    /**
     * Retrieve the key string from the current word information.
     *
     * @see OpenWnnDictionaryImplJni#getNextWord
     * @param work      The internal work area
     * @return          The Key string
     */
    public static final native String getStroke( long work );

    /**
     * Retrieve the candidate string from the current word information.
     *
     * @see OpenWnnDictionaryImplJni#getNextWord
     * @param work      The internal work area
     * @return          The candidate string
     */
    public static final native String getCandidate( long work );

    /**
     * Retrieve the frequency from the current word information.
     *
     * @see OpenWnnDictionaryImplJni#getNextWord
     * @param work      The internal work area
     * @return          The frequency
     */
    public static final native int getFrequency( long work );

    /**
     * Retrieve the part of speech at left side from the current word information.
     *
     * @param work      The internal work area
     * @return          The part of speech
     */
    public static final native int getLeftPartOfSpeech( long work );

    /**
     * Retrieve the part of speech at right side from the current word information.
     *
     * @param work      The internal work area
     * @return          The part of speech
     */
    public static final native int getRightPartOfSpeech( long work );

    /**
     * Clear approximate patterns.
     *
     * @see jp.co.omronsoft.openwnn.WnnDictionary#clearApproxPattern
     * @param work      The internal work area.
     */
    public static final native void clearApproxPatterns( long work );

    /**
     * Set a approximate pattern.
     *
     * @see jp.co.omronsoft.openwnn.WnnDictionary#setApproxPattern
     * @param work      The internal work area
     * @param src       The string (before)
     * @param dst       The string (after)
     * @return          0 if processing is successful; <0 if an error occur
     */
    public static final native int setApproxPattern( long work, String src, String dst );

    /**
     * Set a predefined approximate pattern.
     *
     * @see jp.co.omronsoft.openwnn.WnnDictionary#setApproxPattern
     * @param work              The internal work area
     * @param approxPattern     The index of predefined approximate pattern (See "Constant about the approximate pattern")
     * @see jp.co.omronsoft.openwnn.WnnDictionary#APPROX_PATTERN_EN_TOUPPER
     * @see jp.co.omronsoft.openwnn.WnnDictionary#APPROX_PATTERN_EN_TOLOWER
     * @see jp.co.omronsoft.openwnn.WnnDictionary#APPROX_PATTERN_EN_QWERTY_NEAR
     * @see jp.co.omronsoft.openwnn.WnnDictionary#APPROX_PATTERN_EN_QWERTY_NEAR_UPPER
     * @return                  0 if processing is successful; <0 if an error occur
     */
    public static final native int setApproxPattern( long work, int approxPattern );

    /**
     * Get the specified approximate pattern.
     * @param work      The internal work area
     * @param src       The string (before)
     * @return          The string array (after)
     */
    public static final native String[] getApproxPattern( long work, String src );
    
    /**
     * Clear the current word information.
     *
     * @param work      The internal work area
     */
    public static final native void clearResult( long work );

    /**
     * Set the part of speech at left side to the current word information.
     *
     * @param work          The internal work area
     * @param partOfSpeech  The part of speech
     * @return              0 if processing is successful; <0 if an error occur
     *
     */
    public static final native int setLeftPartOfSpeech( long work, int partOfSpeech );
    /**
     * Set the part of speech at right side to the current word information.
     *
     * @param work          The internal work area
     * @param partOfSpeech  The part of speech
     * @return              0 if processing is successful; <0 if an error occur
     *
     */
    public static final native int setRightPartOfSpeech( long work, int partOfSpeech );

    /**
     * Set the key string to the current word information.
     *
     * @param work          The internal work area
     * @param stroke        The key string
     * @return              0 if processing is successful; <0 if an error occur
     *
     */
    public static final native int setStroke( long work, String stroke );
    /**
     * Set the candidate string to the current word information.
     *
     * @param work          The internal work area
     * @param candidate     The candidate string
     * @return              0 if processing is successful; <0 if an error occur
     *
     */
    public static final native int setCandidate( long work, String candidate );

    /**
     * Set the previous word information from the current word information.
     *
     * @param work          The internal work area
     * @return              0 if processing is successful; <0 if an error occur
     */
    public static final native int selectWord( long work );

    /**
     * Retrieve the connect array
     *
     * @param work                  The internal work area
     * @param leftPartOfSpeech      The part of speech at left side
     * @return                      The connect array
     */
    public static final native byte[] getConnectArray( long work, int leftPartOfSpeech );

    /**
     * Retrieve the number of the part of speeches at left side.
     *
     * @return              The number
     */
    public static final native int getNumberOfLeftPOS( long work );
    /**
     * Retrieve the number of the part of speeches at right side.
     *
     * @return              The number
     */
    public static final native int getNumberOfRightPOS( long work );

    /**
     * Retrieve the specified part of speech at left side.
     *
     * @param work          The internal work area
     * @param type          The type of a part of speech
     * @return              0 if type is not found; <0 if an error occur; >0 The part of speech
     */
    public static final native int getLeftPartOfSpeechSpecifiedType( long work, int type );

    /**
     * Retrieve the specified part of speech at right side.
     *
     * @param work          The internal work area
     * @param type          The type of a part of speech
     * @return              0 if type is not found; <0 if an error occur; >0 The part of speech
     * @see OpenWnnDictionaryImplJni#POS_TYPE_V1
     * @see OpenWnnDictionaryImplJni#POS_TYPE_V2
     * @see OpenWnnDictionaryImplJni#POS_TYPE_V3
     * @see OpenWnnDictionaryImplJni#POS_TYPE_BUNTOU
     * @see OpenWnnDictionaryImplJni#POS_TYPE_TANKANJI
     * @see OpenWnnDictionaryImplJni#POS_TYPE_SUUJI
     * @see OpenWnnDictionaryImplJni#POS_TYPE_MEISI
     * @see OpenWnnDictionaryImplJni#POS_TYPE_JINMEI
     * @see OpenWnnDictionaryImplJni#POS_TYPE_CHIMEI
     * @see OpenWnnDictionaryImplJni#POS_TYPE_KIGOU
     */
    public static final native int getRightPartOfSpeechSpecifiedType( long work, int type );

    /**
     * Create the string array that is used by operation of query
     *  
     * @param work                  The internal work area
     * @param keyString             The key string
     * @param maxBindsOfQuery       The maximum number of binds of query
     * @param maxPatternOfApprox    The maximum number of approximate patterns per character
     * @return                     The string array for binding
     */
    public static final native String[] createBindArray( long work, String keyString, int maxBindsOfQuery, int maxPatternOfApprox );

    /**
     * Create the string which used query parameter
     *  
     * @param work                  The internal work area
     * @param maxBindsOfQuery       The maximum number of binds of query
     * @param maxPatternOfApprox    The maximum number of approximate patterns per character
     * @param keyColumnName        The name of the key column
     * @return                     The string for querying
     */
    public static final native String createQueryStringBase( long work, int maxBindsOfQuery, int maxPatternOfApprox, String keyColumnName );
}
