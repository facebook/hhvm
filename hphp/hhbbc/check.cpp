/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include <boost/next_prior.hpp>

#include "folly/experimental/Gen.h"

#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/cfg.h"

namespace HPHP { namespace HHBBC { namespace php {

namespace {

//////////////////////////////////////////////////////////////////////

bool checkBlock(const php::Block& b) {
  assert(!b.hhbcs.empty());

  // No instructions in the middle of a block should have taken edges,
  // or be an unconditional Jmp.
  for (auto it = begin(b.hhbcs); it != end(b.hhbcs); ++it) {
    assert(it->op != Op::Jmp && "unconditional Jmp mid-block");
    if (boost::next(it) == end(b.hhbcs)) break;
    forEachTakenEdge(*it, [&] (php::Block& b) {
      assert(!"Instruction in middle of block had a jump target");
    });
  }

  // The factored exit list contains unique elements.
  std::set<borrowed_ptr<const php::Block>> exitSet;
  std::copy(begin(b.factoredExits), end(b.factoredExits),
            std::inserter(exitSet, begin(exitSet)));
  assert(exitSet.size() == b.factoredExits.size());

  return true;
}

bool checkParams(const php::Func& f) {
  assert(f.params.size() <= f.locals.size());
  for (uint32_t i = 0; i < f.locals.size(); ++i) {
    assert(f.locals[i]->id == i);
  }
  for (uint32_t i = 0; i < f.iters.size(); ++i) {
    assert(f.iters[i]->id == i);
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
                     borrowed_ptr<const php::Block> target) {
  return std::find(begin(c), end(c), target) != end(c);
}

void checkExnTreeBasic(boost::dynamic_bitset<>& seenIds,
                       borrowed_ptr<const ExnNode> node,
                       borrowed_ptr<const ExnNode> expectedParent) {
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
    checkExnTreeBasic(seenIds, borrow(c), node);
  }
}

void checkFaultEntryRec(boost::dynamic_bitset<>& seenBlocks,
                        const php::Block& faultEntry,
                        const php::ExnNode& exnNode) {
  // Loops in fault funclets could cause us to revisit the same block,
  // so we track the ones we've seen.
  if (seenBlocks.size() < faultEntry.id + 1) {
    seenBlocks.resize(faultEntry.id + 1);
  }
  if (seenBlocks[faultEntry.id]) return;
  seenBlocks[faultEntry.id] = true;

  /*
   * All funclets aren't in the main section.
   */
  assert(faultEntry.section != php::Block::Section::Main);

  /*
   * The fault blocks should all have factored exits to parent
   * catches/faults, if there are any.
   *
   * Note: for now this is an invariant, but if we start pruning
   * factoredExits this might need to change.
   */
  for (auto parent = exnNode.parent; parent; parent = parent->parent) {
    match<void>(
      parent->info,
      [&] (const TryRegion& tr) {
        for (DEBUG_ONLY auto& c : tr.catches) {
          assert(has_edge_linear(faultEntry.factoredExits, c.second));
        }
      },
      [&] (const FaultRegion& fr) {
        assert(has_edge_linear(faultEntry.factoredExits, fr.faultEntry));
      }
    );
  }

  // Note: right now we're only asserting about normal successors, but
  // there can be exception-only successors for catch blocks inside of
  // fault funclets for finally handlers.  (Just going un-asserted for
  // now.)
  forEachNormalSuccessor(faultEntry, [&] (const php::Block& succ) {
    checkFaultEntryRec(seenBlocks, succ, exnNode);
  });
}

void checkExnTreeMore(borrowed_ptr<const ExnNode> node) {
  // Fault entries have a few things to assert.
  match<void>(
    node->info,
    [&] (const FaultRegion& fr) {
      boost::dynamic_bitset<> seenBlocks;
      checkFaultEntryRec(seenBlocks, *fr.faultEntry, *node);
    },
    [&] (const TryRegion& tr) {}
  );

  for (auto& c : node->children) checkExnTreeMore(borrow(c));
}

bool checkExnTree(const php::Func& f) {
  boost::dynamic_bitset<> seenIds;
  for (auto& n : f.exnNodes) checkExnTreeBasic(seenIds, borrow(n), nullptr);

  // ExnNode ids are contiguous.
  for (size_t i = 0; i < seenIds.size(); ++i) assert(seenIds[i] == true);

  // The following assertions come after the above, because if the
  // tree is totally clobbered it's easy for the wrong ones to fire.
  for (auto& n : f.exnNodes) checkExnTreeMore(borrow(n));
  return true;
}

//////////////////////////////////////////////////////////////////////

}

bool check(const php::Func& f) {
  assert(checkParams(f));
  for (DEBUG_ONLY auto& block : f.blocks) assert(checkBlock(*block));

  /*
   * Some of these relationships may change as async/await
   * implementation progresses.  Asserting them now so they are
   * revisited here if they aren't true anymore.
   */
  if (f.isClosureBody)          assert(!f.top);
  //if (f.isGeneratorBody)        assert(f.top);  // TODO_L this fires
  //if (f.isGeneratorFromClosure) assert(f.top);  // TODO_L dunno this too
  if (f.isGeneratorFromClosure) assert(f.isGeneratorBody);
  if (f.isAsync)                assert(f.isGeneratorBody ||
                                       f.generatorBodyName);
  if (f.isPairGenerator)        assert(f.isGeneratorBody);

  boost::dynamic_bitset<> seenId(f.nextBlockId);
  for (auto& block : f.blocks) {
    assert(checkBlock(*block));

    // All blocks have unique ids in a given function; not necessarily
    // consecutive.
    assert(block->id < f.nextBlockId);
    assert(!seenId.test(block->id));
    seenId.set(block->id);
  }

  assert(checkExnTree(f));

  return true;
}

bool check(const php::Class& c) {
  for (DEBUG_ONLY auto& m : c.methods) assert(check(*m));
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
