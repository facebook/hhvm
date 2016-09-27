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


#ifndef incl_HPHP_UTIL_CODE_CACHE_H_
#define incl_HPHP_UTIL_CODE_CACHE_H_

#include "hphp/runtime/vm/jit/types.h"

#include "hphp/util/data-block.h"

namespace HPHP { namespace jit {

/*
 * CodeCache contains our Translation Cache, which is partitioned into 5
 * sections:
 *   - hot: Hot code from AttrHot Funcs.
 *   - main: Hot code from non-AttrHot Funcs.
 *   - cold: Cold code from all Funcs.
 *   - frozen: Code that is almost never used, and cold code from profiling
       translations.
 *   - prof: Profiling translations.
 *
 * There is also a 'data' section containing things like jump tables, floating
 * point constants, and other values that must be directly addressable from
 * translated code.
 *
 * CodeCache only allows direct access to const references to its
 * (Code|Data)Blocks. Any user that wants to emit code or data must call the
 * view() function, which returns a struct of mutable references, appropriately
 * selected based on hotness and whether profiling is active.
 */
struct CodeCache {
  struct View;

  static constexpr uint32_t kNullptrOffset =
    std::numeric_limits<uint32_t>::max();

  /* Code block sizes read from configs. */
  static uint64_t AHotSize;
  static uint64_t ASize;
  static uint64_t AProfSize;
  static uint64_t AColdSize;
  static uint64_t AFrozenSize;

  static uint64_t GlobalDataSize;

  static uint64_t AMaxUsage;
  static uint64_t AColdMaxUsage;
  static uint64_t AFrozenMaxUsage;

  static bool MapTCHuge;

  static uint32_t AutoTCShift;

  static uint32_t TCNumHugeHotMB;
  static uint32_t TCNumHugeColdMB;

  CodeCache();

  template<typename L>
  void forEachBlock(L body) const {
    body("hot", m_hot);
    body("main", m_main);
    body("prof", m_prof);
    body("cold", m_cold);
    body("frozen", m_frozen);
  }

  size_t codeSize() const { return m_codeSize; }

  /*
   * Returns the total amount of code emitted to all blocks. Not synchronized,
   * so the value may be stale by the time this function returns.
   */
  size_t totalUsed() const;

  CodeAddress base() const { return m_base; }
  CodeAddress toAddr(uint32_t offset) const {
    if (offset == kNullptrOffset) return nullptr;

    auto const addr = m_base + offset;
    assert_flog(addr >= m_base && addr < m_base + kNullptrOffset,
                "{} outside [{}, {})", addr, m_base, m_base + kNullptrOffset);
    return addr;
  }
  uint32_t toOffset(ConstCodeAddress addr) const {
    if (addr == nullptr) return kNullptrOffset;

    assert_flog(addr >= m_base && addr < m_base + kNullptrOffset,
                "{} outside [{}, {})", addr, m_base, m_base + kNullptrOffset);
    return addr - m_base;
  }
  bool isValidCodeAddress(ConstCodeAddress addr) const;

  /*
   * Prevent or allow writing to the code section of this CodeCache.
   */
  void protect();
  void unprotect();

  const CodeBlock& hot()    const { return m_hot; }
  const CodeBlock& main()   const { return m_main; }
  const CodeBlock& cold()   const { return m_cold; }
  const CodeBlock& frozen() const { return m_frozen; }
  const CodeBlock& prof()   const { return m_prof; }

  const DataBlock& data() const { return m_data; }

  /*
   * Return a View of this CodeCache, selecting blocks approriately depending
   * on the kind of translation to be emitted.
   */
  View view(TransKind kind = TransKind::Invalid);

  /*
   * Return the block containing addr.
   */
  const CodeBlock& blockFor(CodeAddress addr) const;
        CodeBlock& blockFor(CodeAddress addr);

  /*
   * Set a flag preventing further use of hot because it has filled up.
   */
  void disableHot() { m_useHot = false; }

  /*
   * Returns whether or not the hot code area is enabled.
   */
  bool hotEnabled() const { return m_useHot; }

private:
  CodeAddress m_base;
  size_t m_codeSize;
  size_t m_totalSize;
  bool m_useHot;

  CodeBlock m_main;
  CodeBlock m_cold;
  CodeBlock m_hot;
  CodeBlock m_prof;
  CodeBlock m_frozen;
  DataBlock m_data;
};

struct CodeCache::View {
  View(CodeBlock& main, CodeBlock& cold, CodeBlock& frozen, DataBlock& data)
    : m_main(main)
    , m_cold(cold)
    , m_frozen(frozen)
    , m_data(data)
  {}

  CodeBlock& main()   { return m_main; }
  CodeBlock& cold()   { return m_cold; }
  CodeBlock& frozen() { return m_frozen; }
  DataBlock& data()   { return m_data; }

  const CodeBlock& main()   const { return m_main; }
  const CodeBlock& cold()   const { return m_cold; }
  const CodeBlock& frozen() const { return m_frozen; }
  const DataBlock& data()   const { return m_data; }

private:
  CodeBlock& m_main;
  CodeBlock& m_cold;
  CodeBlock& m_frozen;
  DataBlock& m_data;
};

}}

#endif
