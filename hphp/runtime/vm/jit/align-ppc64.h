/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_JIT_ALIGN_PPC64_H_
#define incl_HPHP_JIT_ALIGN_PPC64_H_

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/alignment.h"

#include "hphp/util/data-block.h"

namespace HPHP { namespace jit { namespace ppc64 {

///////////////////////////////////////////////////////////////////////////////

/*
 * Mirrors the API of align.h.
 */

bool is_aligned(TCA frontier, Alignment alignment);

void align(CodeBlock& cb, Alignment alignment, AlignContext context,
           bool fixups = true);

constexpr size_t cache_line_size() { return 128; }

/*
 * All the Alignments can be expressed by stipulating that the code region
 * given by
 *
 *    [frontier + offset, nbytes)
 *
 * fits into the nearest `align'-aligned and -sized line.
 */
struct AlignInfo {
  size_t align;
  size_t nbytes;
  size_t offset;
};

/*
 * Get the AlignInfo for `alignment'; used by relocation.
 */
const AlignInfo& alignment_info(Alignment alignment);

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
