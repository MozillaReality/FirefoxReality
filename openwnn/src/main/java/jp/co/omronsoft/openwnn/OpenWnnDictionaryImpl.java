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
 * The implementation class of WnnDictionary interface (JNI wrapper class).
 *
 * @author Copyright (C) 2008, 2009 OMRON SOFTWARE CO., LTD.  All Rights Reserved.
 */
public class OpenWnnDictionaryImpl implements WnnDictionary {
    /*
     * DEFINITION FOR JNI
     */
    static {
        /* Load the dictionary search library */ 
        System.loadLibrary( "wnndict" );
    }

    /*
     * DEFINITION OF PRIVATE FIELD
     */
    /** Internal work area for the dictionary search library */
    protected long mWnnWork = 0;

    /** The number of queried items */
    protected int mCountCursor = 0;

    /*
     * DEFINITION OF METHODS
     */
    /**
     * The constructor of this class with writable dictionary.
     *
     * Create a internal work area and the writable dictionary for the search engine. It is allocated for each object.
     *
     * @param dicLibPath    The dictionary library file path
     */
    public OpenWnnDictionaryImpl(String dicLibPath ) {
        /* Create the internal work area */
        this.mWnnWork = OpenWnnDictionaryImplJni.createWnnWork( dicLibPath );
    }

    /**
     * The finalizer of this class.
     * Destroy the internal work area for the search engine.
     */
    protected void finalize( ) {
        /* Free the internal work area */
        if( this.mWnnWork != 0 ) {
            OpenWnnDictionaryImplJni.freeWnnWork( this.mWnnWork );
            this.mWnnWork = 0;
        }
    }
    
    public boolean isActive() {
        return (this.mWnnWork != 0);
    }

    /**
     * @see jp.co.omronsoft.openwnn.WnnDictionary#setDictionary
     */
    public int setDictionary(int index, int base, int high ) {
        if( this.mWnnWork != 0 ) {
            return OpenWnnDictionaryImplJni.setDictionaryParameter( this.mWnnWork, index, base, high );
        } else {
            return -1;
        }
    }

    /**
     * @see jp.co.omronsoft.openwnn.WnnDictionary#searchWord
     */
    public int searchWord( int operation, int order, String keyString ) {
        /* Unset the previous word information */
        OpenWnnDictionaryImplJni.clearResult( this.mWnnWork );

        /* Search to fixed dictionary */
        if( this.mWnnWork != 0 ) {
            int ret = OpenWnnDictionaryImplJni.searchWord( this.mWnnWork, operation, order, keyString );
            if (mCountCursor > 0) {
                ret = 1;
            }
            return ret;
        } else {
            return -1;
        }
    }

    /**
     * @see jp.co.omronsoft.openwnn.WnnDictionary#searchWord
     */
    public int searchWord( int operation, int order, String keyString, WnnWord wnnWord ) {
        if( wnnWord == null || wnnWord.partOfSpeech == null ) {
            return -1;
        }

        /* Search to fixed dictionary with link information */
        OpenWnnDictionaryImplJni.clearResult( this.mWnnWork );
        OpenWnnDictionaryImplJni.setStroke( this.mWnnWork, wnnWord.stroke );
        OpenWnnDictionaryImplJni.setCandidate( this.mWnnWork, wnnWord.candidate );
        OpenWnnDictionaryImplJni.setLeftPartOfSpeech( this.mWnnWork, wnnWord.partOfSpeech.left );
        OpenWnnDictionaryImplJni.setRightPartOfSpeech( this.mWnnWork, wnnWord.partOfSpeech.right );
        OpenWnnDictionaryImplJni.selectWord( this.mWnnWork );

        if( this.mWnnWork != 0 ) {
            int ret = OpenWnnDictionaryImplJni.searchWord( this.mWnnWork, operation, order, keyString );
            if (mCountCursor > 0) {
                ret = 1;
            }
            return ret;
        } else {
            return -1;
        }
    }

    /**
     * @see jp.co.omronsoft.openwnn.WnnDictionary#getNextWord
     */
    public WnnWord getNextWord( ) {
        return getNextWord( 0 );
    }

    /**
     * @see jp.co.omronsoft.openwnn.WnnDictionary#getNextWord
     */
    public WnnWord getNextWord( int length ) {
        if( this.mWnnWork != 0 ) {
            /* Get the result from fixed dictionary */
            int res = OpenWnnDictionaryImplJni.getNextWord( this.mWnnWork, length );
            if( res > 0 ) {
                WnnWord result = new WnnWord( );
                if( result != null ) {
                    result.stroke               = OpenWnnDictionaryImplJni.getStroke( this.mWnnWork );
                    result.candidate            = OpenWnnDictionaryImplJni.getCandidate( this.mWnnWork );
                    result.frequency            = OpenWnnDictionaryImplJni.getFrequency( this.mWnnWork );
                    result.partOfSpeech.left    = OpenWnnDictionaryImplJni.getLeftPartOfSpeech( this.mWnnWork );
                    result.partOfSpeech.right   = OpenWnnDictionaryImplJni.getRightPartOfSpeech( this.mWnnWork );
                }
                return result;
            } else if ( res == 0 ) {
                /* No result is found. */
                return null;
            } else {
                /* An error occur (It is regarded as "No result is found".) */
                return null;
            }
        } else {
            return null;
        }
    }

    /**
     * @see jp.co.omronsoft.openwnn.WnnDictionary#clearApproxPattern
     */
    public void clearApproxPattern( ) {
        if( this.mWnnWork != 0 ) {
            OpenWnnDictionaryImplJni.clearApproxPatterns( this.mWnnWork );
        }
    }

    /**
     * @see jp.co.omronsoft.openwnn.WnnDictionary#setApproxPattern
     */
    public int setApproxPattern( String src, String dst ) {
        if( this.mWnnWork != 0 ) {
            return OpenWnnDictionaryImplJni.setApproxPattern( this.mWnnWork, src, dst );
        } else {
            return -1;
        }
    }

    /**
     * @see jp.co.omronsoft.openwnn.WnnDictionary#setApproxPattern
     */
    public int setApproxPattern( int approxPattern ) {
        if( this.mWnnWork != 0 ) {
            return OpenWnnDictionaryImplJni.setApproxPattern( this.mWnnWork, approxPattern );
        } else {
            return -1;
        }
    }

    /**
     * @see jp.co.omronsoft.openwnn.WnnDictionary#getConnectMatrix
     */
    public byte[][] getConnectMatrix( ) {
        byte[][]    result;
        int         lcount, i;

        if (this.mWnnWork != 0) {
            /* 1-origin */
            lcount = OpenWnnDictionaryImplJni.getNumberOfLeftPOS( this.mWnnWork );
            result = new byte[ lcount + 1 ][ ];

            if( result != null ) {
                for( i = 0 ; i < lcount + 1 ; i++ ) {
                    result[ i ] = OpenWnnDictionaryImplJni.getConnectArray( this.mWnnWork, i );

                    if( result[ i ] == null ) {
                        return null;
                    }
                }
            }
        } else {
            result = new byte[1][1];
        }
        return result;
    }

    /**
     * @see jp.co.omronsoft.openwnn.WnnDictionary#getPOS
     */
    public WnnPOS getPOS( int type ) {
        WnnPOS result = new WnnPOS( );

        if( this.mWnnWork != 0 && result != null ) {
            result.left  = OpenWnnDictionaryImplJni.getLeftPartOfSpeechSpecifiedType( this.mWnnWork, type );
            result.right = OpenWnnDictionaryImplJni.getRightPartOfSpeechSpecifiedType( this.mWnnWork, type );

            if( result.left < 0 || result.right < 0 ) {
                return null;
            }
        }
        return result;
    }
}
