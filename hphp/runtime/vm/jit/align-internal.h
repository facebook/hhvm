/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_JIT_ALIGN_INTERNAL_H_
#define incl_HPHP_JIT_ALIGN_INTERNAL_H_

#include "hphp/runtime/vm/jit/alignment.h"
#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/util/data-block.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

template <class AImpl>
bool is_aligned(TCA frontier, Alignment alignment);

template <class AImpl>
void align(CodeBlock& cb, CGMeta* meta,
           Alignment alignment, AlignContext context);

/*
 * Used in align-*.cpp to generate an AlignInfo table within an arch namespace.
 */
#define DECLARE_ALIGN_TABLE(table)                                            \
  constexpr AlignInfo table[] = {                                             \
    { cache_line_size(),  cache_line_size(),      0 }, /* CacheLine */        \
    { cache_line_size(),  cache_line_size() / 2,  0 }, /* CacheLineRoundUp */ \
    { kJmpTargetAlign,    kJmpTargetAlign,        0 }, /* JmpTarget */        \
    { cache_line_size(),  smashableMovqLen(),     kSmashMovqImmOff },         \
    { cache_line_size(),  smashableCmpqLen(),     kSmashCmpqImmOff },         \
    { cache_line_size(),  smashableCallLen(),     0 },                        \
    { cache_line_size(),  smashableJmpLen(),      0 },                        \
    { cache_line_size(),  smashableJccLen(),      0 },                        \
  }

#define DEFINE_ALIGN_TABLE(table) \
  constexpr AlignInfo table[kNumAlignments]


///////////////////////////////////////////////////////////////////////////////

}}

#include "hphp/runtime/vm/jit/align-internal-inl.h"

#endif
