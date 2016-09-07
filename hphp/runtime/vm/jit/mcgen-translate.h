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

#ifndef incl_HPHP_JIT_MCGEN_TRANSLATE_H_
#define incl_HPHP_JIT_MCGEN_TRANSLATE_H_

#include "hphp/runtime/vm/jit/types.h"

namespace HPHP { namespace jit {

struct TransArgs;

namespace mcgen {

/*
 * Create a new translation based on args.
 *
 * The SrcKey and kind of this translation must be specified in args. The
 * TransID and region may optionally be specified as well. If the kind of region
 * requested is TransOptimize a TransID must be specified.
 *
 * Should the region be absent, an appropriate region for the designated kind
 * will be selected.
 */
TCA translate(TransArgs args);

/*
 * Create a translation for the SrcKey specified in args.
 *
 * If a translation for this SrcKey already exists it will be returned. The kind
 * of translation created will be selected based on the SrcKey specified.
 */
TCA createTranslation(TransArgs args);

/*
 * Find the translation for the SrcKey specified in args.
 *
 * If no translation exists or the current VM frame is a pseudomain then return
 * nullptr.
 */
TCA findTranslation(const TransArgs& args);

}}}

#endif
