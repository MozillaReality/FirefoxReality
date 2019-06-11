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
 * The container class of a clause.
 *
 * @author Copyright (C) 2009, OMRON SOFTWARE CO., LTD.  All Rights Reserved.
 */
public class WnnClause extends WnnWord {

    /**
     * Constructor
     *
     * @param candidate The string of the clause
     * @param stroke    The reading of the clause
     * @param posTag    The part of speech of the clause
     * @param frequency The frequency of the clause
     */
    public WnnClause(String candidate, String stroke, WnnPOS posTag, int frequency) {
        super(candidate,
              stroke,
              posTag,
              frequency);
    }

    /**
     * Constructor
     *
     * @param stroke   The reading of the clause
     * @param stem     The independent word part of the clause
     */
    public WnnClause (String stroke, WnnWord stem) {
        super(stem.id,
              stem.candidate,
              stroke,
              stem.partOfSpeech,
              stem.frequency,
              0);
    }

    /**
     * Constructor
     *
     * @param stroke   The reading of the clause
     * @param stem     The independent word part of the clause
     * @param fzk      The ancillary word part of the clause
     */
    public WnnClause (String stroke, WnnWord stem, WnnWord fzk) {
        super(stem.id,
              stem.candidate + fzk.candidate,
              stroke,
              new WnnPOS(stem.partOfSpeech.left, fzk.partOfSpeech.right),
              stem.frequency,
              1);
    }
}
