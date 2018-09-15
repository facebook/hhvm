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

bool DEBUG_ONLY checkBlock(const php::Block& b) {
  if (b.id == NoBlockId) {
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

  // If the block has an exnNode, it should always have a throw edge to it (and
  // nothing else).
  if (b.exnNode) {
    assert(b.throwExits.size() == 1);
    match<void>(
      b.exnNode->info,
      [&] (const CatchRegion& cr) {
        assert(cr.catchEntry == b.throwExits[0]);
      },
      [&] (const FaultRegion& fr) {
        assert(fr.faultEntry == b.throwExits[0]);
      }
    );
  }

  if (b.section == php::Block::Section::Main) {
    // Non-fault funclets should not have unwind exits. It can only have a throw
    // exit if it has an exnNode (which was checked above).
    assert(!ends_with_unwind(b));
    assert(b.unwindExits.empty());
    assert(b.exnNode || b.throwExits.empty());
  } else if (!ends_with_unwind(b)) {
    // Only blocks which end with an unwind can have unwind exits.
    assert(b.unwindExits.empty());
  }

  // The exit lists contains unique elements.
  hphp_fast_set<BlockId> exitSet;
  std::copy(begin(b.throwExits), end(b.throwExits),
            std::inserter(exitSet, begin(exitSet)));
  assert(exitSet.size() == b.throwExits.size());

  exitSet.clear();
  std::copy(begin(b.unwindExits), end(b.unwindExits),
            std::inserter(exitSet, begin(exitSet)));
  assert(exitSet.size() == b.unwindExits.size());

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

void checkExnTreeBasic(boost::dynamic_bitset<>& seenIds,
                       const ExnNode* node,
                       const ExnNode* expectedParent) {
  // All exnNode ids must be unique.
  if (seenIds.size() < node->id + 1) {
    seenIds.resize(node->id + 1);
  }
  assert(!seenIds[node->id]);
  seenIds[node->id] = true;

  // Parent pointers should point to the node that has a given node as
  // a child.
  assert(node->parent == expectedParent);

  for (auto& c : node->children) {
    checkExnTreeBasic(seenIds, c.get(), node);
  }
}

void checkFaultEntryRec(const php::Func& func,
                        boost::dynamic_bitset<>& seenBlocks,
                        BlockId faultEntryId,
                        const php::ExnNode& exnNode) {

  auto const& faultEntry = *func.blocks[faultEntryId];
  assert(faultEntry.id == faultEntryId);
  // Loops in fault funclets could cause us to revisit the same block,
  // so we track the ones we've seen.
  if (seenBlocks.size() < faultEntryId + 1) {
    seenBlocks.resize(faultEntryId + 1);
  }
  if (seenBlocks[faultEntryId]) return;
  seenBlocks[faultEntryId] = true;

  /*
   * All funclets aren't in the main section.
   */
  assert(faultEntry.section != php::Block::Section::Main);

  // Note: right now we're only asserting about normal successors, but
  // there can be exception-only successors for catch blocks inside of
  // fault funclets for finally handlers.  (Just going un-asserted for
  // now.)
  forEachNormalSuccessor(faultEntry, [&] (BlockId succ) {
    checkFaultEntryRec(func, seenBlocks, succ, exnNode);
  });
}

void checkExnTreeMore(const php::Func& func, const ExnNode* node) {
  match<void>(
    node->info,
    [&](const FaultRegion& fr) {
      boost::dynamic_bitset<> seenBlocks;
      checkFaultEntryRec(func, seenBlocks, fr.faultEntry, *node);
    },
    [] (const CatchRegion&) {}
  );

  for (auto& c : node->children) checkExnTreeMore(func, c.get());
}

bool DEBUG_ONLY checkExnTree(const php::Func& f) {
  boost::dynamic_bitset<> seenIds;
  for (auto& n : f.exnNodes) checkExnTreeBasic(seenIds, n.get(), nullptr);

  // ExnNode ids are contiguous.
  for (size_t i = 0; i < seenIds.size(); ++i) assert(seenIds[i] == true);

  // The following assertions come after the above, because if the
  // tree is totally clobbered it's easy for the wrong ones to fire.
  for (auto& n : f.exnNodes) checkExnTreeMore(f, n.get());
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
  for (DEBUG_ONLY auto& block : f.blocks) assert(checkBlock(*block));

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

  DEBUG_ONLY Attr pcm = AttrParamCoerceModeNull | AttrParamCoerceModeFalse;
  assert((f.attrs & pcm) != pcm); // not both

  boost::dynamic_bitset<> seenId(f.blocks.size());
  for (auto& block : f.blocks) {
    if (block->id == NoBlockId) continue;

    // All blocks have unique ids in a given function; not necessarily
    // consecutive.
    assert(block->id < f.blocks.size());
    assert(!seenId.test(block->id));
    seenId.set(block->id);
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
    assert(c.methods.size() == 1 || (c.methods[1]->attrs & AttrIsInOutWrapper));
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
