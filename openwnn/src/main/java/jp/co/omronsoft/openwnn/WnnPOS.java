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
 * The container class of a part of speech.
 *
 * @author Copyright (C) 2008-2009, OMRON SOFTWARE CO., LTD.  All Rights Reserved.
 */
public class WnnPOS {
    /** The part of speech at left side */
    public int left = 0;

    /** The part of speech at right side */
    public int right = 0;

    /**
     * Constructor
     */
    public WnnPOS() {}

    /**
     * Constructor
     *
     * @param left      The part of speech at left side
     * @param right     The part of speech at right side
     */
    public WnnPOS(int left, int right) {
        this.left  = left;
        this.right = right;
    }
}

