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

#ifndef incl_HPHP_JIT_ALIGNMENT_H_
#define incl_HPHP_JIT_ALIGNMENT_H_

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * Whether we're aligning at a live or a dead code point.
 *
 * Live alignments must use nop gaps; alignment requests in dead contexts are
 * allowed to use various trap instructions.
 */
enum class AlignContext : uint32_t { Live, Dead };

/*
 * What to align to.
 *
 * Not all of these will actually require alignment on all targets.
 */
enum class Alignment : uint32_t {
  /*
   * Align to the start of the next cache line unconditionally (unless we are
   * already aligned to one).
   */
  CacheLine = 0,

  /*
   * If we are in the second half of a cache line, align to the next one.
   */
  CacheLineRoundUp,

  /*
   * Align to the nearest valid jmp target.
   */
  JmpTarget,

  /*
   * Alignments needed by smashable instructions.
   */
  SmashMovq,
  SmashCmpq,
  SmashCall,
  SmashJmp,
  SmashJcc,
  SmashJccAndJmp,
};

constexpr auto kNumAlignments =
  static_cast<size_t>(Alignment::SmashJccAndJmp) + 1;

/*
 * Under most architectures, the Alignments can be expressed by stipulating
 * that the code region given by
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

///////////////////////////////////////////////////////////////////////////////

}}

#endif
