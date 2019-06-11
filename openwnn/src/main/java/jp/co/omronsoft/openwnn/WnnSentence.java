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

import java.lang.StringBuffer;
import java.util.ArrayList;
import java.util.Iterator;

/**
 * The container class of a sentence.
 *
 * @author Copyright (C) 2009, OMRON SOFTWARE CO., LTD.  All Rights Reserved.
 */
public class WnnSentence extends WnnWord {
    /** The array of clauses */
    public ArrayList<WnnClause> elements;

    /**
     * Constructor
     *
     * @param input     The string of reading
     * @param clauses   The array of clauses of this sentence
     */
    public WnnSentence(String input, ArrayList<WnnClause> clauses) {
        if (clauses == null || clauses.isEmpty()) {
            this.id = 0;
            this.candidate = "";
            this.stroke = "";
            this.frequency = 0;
            this.partOfSpeech = new WnnPOS();
            this.attribute = 0;
        } else {
            this.elements = clauses;
            WnnClause headClause = clauses.get(0);

            if (clauses.size() == 1) {
                this.id = headClause.id;
                this.candidate = headClause.candidate;
                this.stroke = input;
                this.frequency = headClause.frequency;
                this.partOfSpeech = headClause.partOfSpeech;
                this.attribute = headClause.attribute;
            } else {
                StringBuffer candidate = new StringBuffer();
                Iterator<WnnClause> ci = clauses.iterator();
                while (ci.hasNext()) {
                    WnnClause clause = ci.next();
                    candidate.append(clause.candidate);
                }
                WnnClause lastClause = clauses.get(clauses.size() - 1);
                
                this.id = headClause.id;
                this.candidate = candidate.toString();
                this.stroke = input;
                this.frequency = headClause.frequency;
                this.partOfSpeech = new WnnPOS(headClause.partOfSpeech.left, lastClause.partOfSpeech.right);
                this.attribute = 2;
            }
        }
    }

    /**
     * Constructor
     *
     * @param input     The string of reading
     * @param clause    The clauses of this sentence
     */
    public WnnSentence(String input, WnnClause clause) {
        this.id = clause.id;
        this.candidate = clause.candidate;
        this.stroke = input;
        this.frequency = clause.frequency;
        this.partOfSpeech = clause.partOfSpeech;
        this.attribute = clause.attribute;

        this.elements = new ArrayList<WnnClause>();
        this.elements.add(clause);
    }

    /**
     * Constructor
     *
     * @param prev      The previous clauses
     * @param clause    The clauses of this sentence
     */
    public WnnSentence(WnnSentence prev, WnnClause clause) {
        this.id = prev.id;
        this.candidate = prev.candidate + clause.candidate;
        this.stroke = prev.stroke + clause.stroke;
        this.frequency = prev.frequency + clause.frequency;
        this.partOfSpeech = new WnnPOS(prev.partOfSpeech.left, clause.partOfSpeech.right);
        this.attribute = prev.attribute;

        this.elements = new ArrayList<WnnClause>();
        this.elements.addAll(prev.elements);
        this.elements.add(clause);
    }

    /**
     * Constructor
     *
     * @param head      The top clause of this sentence
     * @param tail      The following sentence
     */
    public WnnSentence(WnnClause head, WnnSentence tail) {
        if (tail == null) {
            /* single clause */
            this.id = head.id;
            this.candidate = head.candidate;
            this.stroke = head.stroke;
            this.frequency = head.frequency;
            this.partOfSpeech = head.partOfSpeech;
            this.attribute = head.attribute;
            this.elements = new ArrayList<WnnClause>();
            this.elements.add(head);
        } else {
            /* consecutive clauses */
            this.id = head.id;
            this.candidate = head.candidate + tail.candidate;
            this.stroke = head.stroke + tail.stroke;
            this.frequency = head.frequency + tail.frequency;
            this.partOfSpeech = new WnnPOS(head.partOfSpeech.left, tail.partOfSpeech.right);
            this.attribute = 2;
            
            this.elements = new ArrayList<WnnClause>();
            this.elements.add(head);
            this.elements.addAll(tail.elements);
        }
    }
}
