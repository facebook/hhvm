/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_JIT_TRANS_DB_H_
#define incl_HPHP_JIT_TRANS_DB_H_

#include "hphp/runtime/vm/jit/trans-rec.h"
#include "hphp/runtime/vm/jit/types.h"

/*
 * Translation DB.
 *
 * We maintain mappings from TCAs and TransIDs to translation information,
 * for debugging purposes only.  Outside of debug builds and processes with
 * TC dumps enabled, these routines do no work, and their corresponding data
 * structures are unused.
 *
 * Note that PGO always has a coherent notion of TransID---the so-called
 * `profTransID', which is just the region block ID (which are globally
 * unique).  This is completely distinct from the Translator's TransID.
 */
namespace HPHP { namespace jit { namespace transdb {

/*
 * Whether the TransDB structures should be used.
 *
 * True only for debug builds or when TC dumps are enabled.
 */
bool enabled();

/*
 * Get a TransRec by TCA or TransID.
 *
 * Return nullptr if the TransDB is not enabled.
 */
const TransRec* getTransRec(TCA tca);
const TransRec* getTransRec(TransID transId);

/*
 * Add a translation.
 *
 * Does nothing but trace if the TransDB is not enabled.
 */
void addTranslation(const TransRec& transRec);

/*
 * Get the number of translations added (0 if the TransDB is not enabled).
 */
size_t getNumTranslations();

}}}

#endif
