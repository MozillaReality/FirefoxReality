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
 * The information container class of segment in a string.
 * 
 * This class defines information of a segment in a string, such as a character, a word or a clause.
 * It is used to represent the layers of the composing text ({@link ComposingText}).
 *
 * @author Copyright (C) 2009 OMRON SOFTWARE CO., LTD. All rights reserved.
 */
public class StrSegment  {
    /** The string */
    public String string;
    /** The start position */
    public int from;
    /** The end position */
    public int to;

    /**
     * Constructor
     */
    public StrSegment() {
        this(null, -1, -1);
    }

    /**
     * Constructor
     *
     * @param str       The string
     */
    public StrSegment(String str) {
        this(str, -1, -1);
    }

    /**
     * Constructor
     *
     * @param chars     The array of characters
     */
    public StrSegment(char[] chars) {
        this(new String(chars), -1, -1);
    }

    /**
     * Constructor
     *
     * @param str       The string
     * @param from      The start position
     * @param to        The end position
     */
    public StrSegment(String str, int from, int to) {
        this.string = str;
        this.from = from;
        this.to = to;
    }
}
