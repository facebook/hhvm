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

const size_t kTrampolinesBlockSize = 8 << 12;

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
    body("stubs", m_stubs);
    body("trampolines", m_trampolines);
  }

  size_t codeSize() const { return m_codeSize; }

  CodeAddress base() const { return m_base; }
  bool isValidCodeAddress(CodeAddress addr) const;

  void protect();
  void unprotect();

  CodeBlock& main();
  const CodeBlock& main() const {
    return const_cast<CodeCache&>(*this).main();
  }

  CodeBlock& stubs();
  const CodeBlock& stubs() const {
    return const_cast<CodeCache&>(*this).stubs();
  }
  CodeBlock& trampolines()             { return m_trampolines; }
  const CodeBlock& trampolines() const { return m_trampolines; }

  DataBlock& data() { return m_data; }

  // Read-only access for MCGenerator::dumpTCCode()
  const CodeBlock& prof() const { return m_prof; }

private:
  CodeAddress m_base;
  CodeAddress m_mainBase;
  size_t m_codeSize;
  size_t m_totalSize;
  Selection m_selection;

  CodeBlock m_main;        // used for hot code of non-AttrHot functions
  CodeBlock m_stubs;       // used for cold or one time use code
  CodeBlock m_hot;         // used for hot code of AttrHot functions
  CodeBlock m_prof;        // used for hot code of profiling translations
  CodeBlock m_trampolines; // used to enable static calls to distant code
  DataBlock m_data;        // data to be used by translated code
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
