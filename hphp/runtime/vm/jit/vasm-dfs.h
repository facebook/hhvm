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

#ifndef incl_HPHP_JIT_VASM_DFS_H_
#define incl_HPHP_JIT_VASM_DFS_H_

#include "hphp/runtime/vm/jit/vasm-x64.h"
#include <boost/dynamic_bitset.hpp>

namespace HPHP { namespace jit {

// visit reachable blocks calling pre and post on each one.
struct DfsWalker {
  explicit DfsWalker(const Vunit& u)
    : unit(u)
    , visited(u.blocks.size())
  {}
  template<class Pre, class Post> void dfs(Vlabel b, Pre pre, Post post) {
    if (visited.test(b)) return;
    visited.set(b);
    pre(b);
    for (auto s : succs(unit.blocks[b])) {
      dfs(s, pre, post);
    }
    post(b);
  }
  template<class Pre, class Post> void dfs(Pre pre, Post post) {
    dfs(unit.entry, pre, post);
  }
private:
  const Vunit& unit;
  boost::dynamic_bitset<> visited;
};

// visit reachable blocks in postorder, calling post on each one.
struct PostorderWalker {
  template<class Post> void dfs(Post post) {
    m_dfs.dfs([](Vlabel){}, post);
  }
  explicit PostorderWalker(const Vunit& u) : m_dfs{u} {}
private:
  DfsWalker m_dfs;
};

}}
#endif
