/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/data-block.h"

namespace HPHP {

#if defined(__APPLE__) || defined(__CYGWIN__)
extern const void* __hot_start;
extern const void* __hot_end;
#else
extern "C" {
void __attribute__((__weak__)) __hot_start();
void __attribute__((__weak__)) __hot_end();
}
#endif

struct CodeCache {
  enum class Selection {
    Default,   // 'main'
    Hot,       // 'hot'
    Profile,   // 'prof' -- highest precedence
  };

  struct Selector;

  CodeCache();
  ~CodeCache();

  CodeBlock& blockFor(CodeAddress addr);

  template<typename L>
  void forEachBlock(L body) const {
    body("hot", m_hot);
    body("main", m_main);
    body("prof", m_prof);
    body("cold", m_cold);
    body("frozen", m_frozen);
  }

  size_t codeSize() const { return m_codeSize; }

  // Returns the total amount of code emitted to all blocks. Not synchronized,
  // so the value may be stale by the time this function returns.
  size_t totalUsed() const;

  size_t mainUsed() const { return m_main.used(); }
  size_t profUsed() const { return m_prof.used(); }

  CodeAddress base() const { return m_base; }
  bool isValidCodeAddress(CodeAddress addr) const;

  void protect();
  void unprotect();

  CodeBlock& main();
  const CodeBlock& main() const {
    return const_cast<CodeCache&>(*this).main();
  }

  CodeBlock& cold();
  const CodeBlock& cold() const {
    return const_cast<CodeCache&>(*this).cold();
  }

  CodeBlock& frozen();
  const CodeBlock& frozen() const {
    return const_cast<CodeCache&>(*this).frozen();
  }

  CodeBlock& realCold()   { return m_cold;   }
  CodeBlock& realFrozen() { return m_frozen; }

  const CodeBlock& realCold()   const { return m_cold;   }
  const CodeBlock& realFrozen() const { return m_frozen; }

  DataBlock& data() { return m_data; }

  // Read-only access for MCGenerator::dumpTCCode()/dumpTCData()
  const CodeBlock& prof() const { return m_prof; }
  const CodeBlock& hot() const { return m_hot; }

  void lock() { m_lock = true; }
  void unlock() { m_lock = false; }

private:
  CodeAddress m_base;
  size_t m_codeSize;
  size_t m_totalSize;
  Selection m_selection;

  /*
   * Code blocks for emitting different kinds of code.
   *
   * See comment in runtime/vm/jit/block.h to see the meanings of different
   * Block Hints.
   *
   * Code blocks with either a 'Likely' or 'Neither' Block Hint are emitted
   * in m_main. Code blocks with an 'Unlikely' Block Hint are emitted in
   * m_cold (except for profiling translations, see below). Code blocks
   * with an 'Unused' Block Hint are emitted in m_frozen.
   *
   * The m_hot section is used for emitting optimzed translations of
   * 'Hot' functions (functions marked with AttrHot). The m_prof is used
   * for emitting profiling translations of Hot functions. Also, for profiling
   * translations, the m_frozen section is used for 'Unlikely' blocks instead
   * of m_cold.
   *
   */
  CodeBlock m_main;   // used for hot code of non-AttrHot functions
  CodeBlock m_cold;   // used for cold or one time use code
  CodeBlock m_hot;    // used for hot code of AttrHot functions
  CodeBlock m_prof;   // used for hot code of profiling translations
  CodeBlock m_frozen; // used for code that is (almost) never used
  DataBlock m_data;   // data to be used by translated code
  bool      m_lock;   // don't allow access to main() or cold()
};

struct CodeCache::Selector {
  struct Args {
    explicit Args(CodeCache& cache)
      : m_cache(cache)
      , m_hot(false)
      , m_profile(false)
    {}

    Args& hot(bool isHot)      { m_hot = isHot; return *this; }
    Args& profile(bool isProf) { m_profile = isProf; return *this; }

   private:
    friend class Selector;

    CodeCache& m_cache;
    bool m_hot;
    bool m_profile;
  };

  explicit Selector(const Args& args);
  ~Selector();

 private:
  CodeCache& m_cache;
  const Selection m_oldSelection;
};

}

#endif
