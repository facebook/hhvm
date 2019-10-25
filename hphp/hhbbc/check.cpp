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

#include <boost/dynamic_bitset.hpp>
#include <algorithm>
#include <iterator>
#include <set>

#include <folly/gen/Base.h>

#include "hphp/runtime/vm/unit-util.h"

#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/unit-util.h"
#include "hphp/hhbbc/cfg.h"
#include "hphp/hhbbc/class-util.h"

namespace HPHP { namespace HHBBC { namespace php {

namespace {

const StaticString s_Closure("Closure");
const StaticString s_invoke("__invoke");

//////////////////////////////////////////////////////////////////////

bool DEBUG_ONLY checkBlock(const php::Func& f, const php::Block& b) {
  if (b.dead) {
    // the block was deleted
    return true;
  }
  assert(!b.hhbcs.empty());

  // No instructions in the middle of a block should have taken edges,
  // or be an unconditional Jmp.
  for (auto it = begin(b.hhbcs); it != end(b.hhbcs); ++it) {
    assert(it->op != Op::Jmp && "unconditional Jmp mid-block");
    if (std::next(it) == end(b.hhbcs)) break;
    forEachTakenEdge(*it, [&](BlockId /*blk*/) {
      assert(!"Instruction in middle of block had a jump target");
    });
  }

  // A block should either have a matching exnNodeId and throwExit, or neither
  // should be set.
  assert(b.throwExit == (b.exnNodeId != NoExnNodeId
    ? f.exnNodes[b.exnNodeId].region.catchEntry
    : NoBlockId));

  return true;
}

bool DEBUG_ONLY checkParams(const php::Func& f) {
  assert(f.params.size() <= f.locals.size());
  for (uint32_t i = 0; i < f.locals.size(); ++i) {
    assert(f.locals[i].id == i);
  }

  // dvInit pointers are consistent in the parameters vector and on
  // the func.
  for (uint32_t i = 0; i < f.params.size(); ++i) {
    assert(f.params[i].dvEntryPoint == f.dvEntries[i]);
  }

  return true;
}

// N is usually small ... ;)
template<class Container>
bool has_edge_linear(const Container& c,
                     BlockId target) {
  return std::find(begin(c), end(c), target) != end(c);
}

void checkExnTreeBasic(const php::Func& f,
                       boost::dynamic_bitset<>& seenIds,
                       const ExnNode* node,
                       ExnNodeId expectedParent) {
  // All exnNode ids must be unique.
  if (seenIds.size() < node->idx + 1) {
    seenIds.resize(node->idx + 1);
  }
  assert(!seenIds[node->idx]);
  seenIds[node->idx] = true;

  // Parent pointers should point to the node that has a given node as
  // a child.
  assert(node->parent == expectedParent);

  for (auto& c : node->children) {
    checkExnTreeBasic(f, seenIds, &f.exnNodes[c], node->idx);
  }
}

bool DEBUG_ONLY checkExnTree(const php::Func& f) {
  boost::dynamic_bitset<> seenIds;
  ExnNodeId idx{0};
  for (auto& n : f.exnNodes) {
    if (n.parent == NoExnNodeId && n.idx != NoExnNodeId) {
      assertx(n.idx == idx);
      checkExnTreeBasic(f, seenIds, &n, NoExnNodeId);
    }
    idx++;
  }

  // ExnNode ids are contiguous.
  for (size_t i = 0; i < seenIds.size(); ++i) {
    assert(seenIds[i] == true || f.exnNodes[i].idx == NoExnNodeId);
  }

  return true;
}

bool DEBUG_ONLY checkName(SString name) {
  return isNSNormalized(name);
}

//////////////////////////////////////////////////////////////////////

}

bool check(const php::Func& f) {
  assert(checkParams(f));
  assert(checkName(f.name));
  for (DEBUG_ONLY auto& block : f.blocks) assert(checkBlock(f, *block));

  /*
   * Some of these relationships may change as async/await
   * implementation progresses.  Asserting them now so they are
   * revisited here if they aren't true anymore.
   */
  if (f.isClosureBody)          assert(!f.top);
  if (f.isPairGenerator)        assert(f.isGenerator);

  if (f.isClosureBody) {
    assert(f.cls &&
           f.cls->parentName &&
           f.cls->parentName->isame(s_Closure.get()));
  }

  assert(checkExnTree(f));
  return true;
}

bool check(const php::Class& c) {
  assert(checkName(c.name));
  for (DEBUG_ONLY auto& m : c.methods) assert(check(*m));

  // Some invariants about Closure classes.
  auto const isClo = is_closure(c);
  if (c.closureContextCls) {
    assert(c.closureContextCls->unit == c.unit);
    assert(isClo);
  }
  if (isClo) {
    assert(c.methods.size() == 1 || c.methods.size() == 2);
    assert(c.methods[0]->name->isame(s_invoke.get()));
    assert(c.methods[0]->isClosureBody);
    assert(c.methods.size() == 1);
  } else {
    assert(!c.closureContextCls);
  }

  return true;
}

bool check(const php::Unit& u) {
  assert(check(*u.pseudomain));
  for (DEBUG_ONLY auto& c : u.classes)   assert(check(*c));
  for (DEBUG_ONLY auto& f : u.funcs)     assert(check(*f));
  return true;
}

bool check(const php::Program& p) {
  for (DEBUG_ONLY auto& u : p.units) assert(check(*u));
  return true;
}

//////////////////////////////////////////////////////////////////////

}}}
