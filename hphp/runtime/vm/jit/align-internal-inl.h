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


#include "hphp/util/data-block.h"

#include <folly/Bits.h>
#include <utility>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

bool is_aligned(TCA frontier, const AlignInfo& a) {
  assertx(a.nbytes <= a.align);
  assertx(folly::isPowTwo(a.align));

  auto const mask = a.align - 1;

  auto const ifrontier = reinterpret_cast<size_t>(frontier);
  auto const first_byte = ifrontier + a.offset;
  auto const last_byte  = ifrontier + a.nbytes - 1;

  return (first_byte & ~mask) == (last_byte & ~mask);
}

size_t align_gap(TCA frontier, const AlignInfo& a) {
  auto const ifrontier = reinterpret_cast<size_t>(frontier);
  auto const gap = a.align - ((a.align - 1) & (ifrontier + a.offset));

  return gap == a.align ? 0 : gap;
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

template <class AImpl>
bool is_aligned(TCA frontier, Alignment alignment) {
  auto const idx = static_cast<uint32_t>(alignment);

  return is_aligned(frontier, AImpl::s_table[idx]);
}

template <class AImpl>
void align(CodeBlock& cb, CGMeta* meta,
           Alignment alignment, AlignContext context) {
  auto const idx = static_cast<uint32_t>(alignment);

  auto const pad_for_align = [&] (const AlignInfo& ali) {
    if (is_aligned(cb.frontier(), ali)) return;
    AImpl::pad(cb, context, align_gap(cb.frontier(), ali));
  };

  pad_for_align(AImpl::s_table[idx]);

  assertx(is_aligned(cb.frontier(), AImpl::s_table[idx]));

  if (meta) {
    meta->alignments.emplace(
      cb.frontier(),
      std::make_pair(alignment, context)
    );
  }
}

///////////////////////////////////////////////////////////////////////////////

}}
