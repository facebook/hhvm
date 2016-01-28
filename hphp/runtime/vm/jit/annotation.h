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
#ifndef ANNOTATION_H_
#define ANNOTATION_H_

#include "hphp/runtime/vm/jit/translator.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

/*
 * Note: the current name of this module/function is historical.  It
 * use to record cross-tracelet global information ("annotations") to
 * help bind call targets.  Now this information comes from static
 * analysis in FCallD opcodes.
 */
void annotate(NormalizedInstruction* instr);

//////////////////////////////////////////////////////////////////////

}}

#endif
