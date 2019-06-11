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
 * The container class of {@link StrSegment} which includes a clause information for Japanese IME.
 *
 * @author Copyright (C) 2009 OMRON SOFTWARE CO., LTD. All rights reserved.
 */
public class StrSegmentClause extends StrSegment {
    /** Clause information */
    public WnnClause clause;
    
    /**
     * Constructor
     *
     * @param clause    The clause
     * @param from      The start position
     * @param to        The end position
     */
    public StrSegmentClause(WnnClause clause, int from, int to) {
        super(clause.candidate, from, to);
        this.clause = clause;
    }
}
