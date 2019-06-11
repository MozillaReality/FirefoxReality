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
 * The container class of a word.
 *
 * @author Copyright (C) 2008-2009, OMRON SOFTWARE CO., LTD.  All Rights Reserved.
 */
public class WnnWord {
    /** The word's Id */
    public int      id;
    /** The string of this word. */
    public String   candidate;
    /** The reading of this word. */
    public String   stroke;
    /** The score of this word. */
    public int      frequency;
    /** The part of speech this word. */
    public WnnPOS   partOfSpeech;
    /** The attribute of this word when it is assumed a candidate. */
    public int      attribute;

    /**
     * Constructor
     */
    public WnnWord() {
        this(0, "", "", new WnnPOS(), 0, 0);
    }

    /**
     * Constructor
     *
     * @param candidate     The string of word
     * @param stroke        The reading of word
     */
    public WnnWord(String candidate, String stroke) {
        this(0, candidate, stroke, new WnnPOS(), 0, 0);
    }

    /**
     * Constructor
     *
     * @param candidate     The string of word
     * @param stroke        The reading of word
     * @param frequency     The score of word
     */
    public WnnWord(String candidate, String stroke, int frequency) {
        this(0, candidate, stroke, new WnnPOS(), frequency, 0);
    }

    /**
     * Constructor
     *
     * @param candidate     The string of word
     * @param stroke        The reading of word
     * @param posTag        The part of speech of word
     */
    public WnnWord(String candidate, String stroke, WnnPOS posTag) {
        this(0, candidate, stroke, posTag, 0, 0);
    }

    /**
     * Constructor
     *
     * @param candidate     The string of word
     * @param stroke        The reading of word
     * @param posTag        The part of speech of word
     * @param frequency     The score of word
     */
    public WnnWord(String candidate, String stroke, WnnPOS posTag, int frequency) {
        this(0, candidate, stroke, posTag, frequency, 0);
    }

    /**
     * Constructor
     *
     * @param id            The ID of word
     * @param candidate     The string of word
     * @param stroke        The reading of word
     * @param posTag        The part of speech of word
     * @param frequency     The score of word
     */
    public WnnWord(int id, String candidate, String stroke, WnnPOS posTag, int frequency) {
        this(id, candidate, stroke, posTag, frequency, 0);
    }

    /**
     * Constructor
     *
     * @param id            The ID of word
     * @param candidate     The string of word
     * @param stroke        The reading of word
     * @param posTag        The part of speech of word
     * @param frequency     The score of word
     * @param attribute     The attribute of word
     */
    public WnnWord(int id, String candidate, String stroke, WnnPOS posTag, int frequency, int attribute) {
        this.id = id;
        this.candidate = candidate;
        this.stroke = stroke;
        this.frequency = frequency;
        this.partOfSpeech = posTag;
        this.attribute = attribute;
    }
}

