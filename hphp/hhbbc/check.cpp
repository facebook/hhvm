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

  for (DEBUG_ONLY auto& factored : b.factoredExits) {
    assert(factored->kind == php::Block::Kind::CatchEntry ||
           factored->kind == php::Block::Kind::Fault);
  }

  switch (b.kind) {
  case php::Block::Kind::Fault:
    // Fault funclets do not point to exception nodes, since their
    // bytecode will not be in the protected offset range.
    assert(b.exnNode == nullptr);
    break;
  case php::Block::Kind::CatchEntry:
    break;
  case php::Block::Kind::Normal:
    break;
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

  /*
   * Each dv init entry has a single successor, which is the next one,
   * and the last one has a single successor which is the main entry.
   *
   * This also is effectively asserting that each dv initializer is a
   * single basic block.  The bytecode specification implies this
   * right now, but it's probably more restrictive than we really need.
   */
  using namespace folly::gen;
  auto entries =
    from(f.dvEntries)
      | filter([&] (borrowed_ptr<php::Block> b) { return b != nullptr; })
      | distinct
      | as<std::vector>();
  entries.push_back(f.mainEntry);
  assert(!entries.empty());
  for (auto it = begin(entries); boost::next(it) != end(entries); ++it) {
    DEBUG_ONLY int count = 0;
    forEachSuccessor(**it, [&] (const php::Block& b) {
      ++count;
      assert(&b == &**boost::next(it) && "dvinit entry chain is wrong");
    });
    assert(count == 1);
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
  seenIds.resize(node->id + 1);
  assert(!seenIds[node->id]);
  seenIds[node->id] = true;

  // Parent pointers should point to the node that has a given node as
  // a child.
  assert(node->parent == expectedParent);

  for (auto& c : node->children) {
    checkExnTreeBasic(seenIds, borrow(c), node);
  }
}

void checkFaultEntryRec(const boost::dynamic_bitset<>& reachableSet,
                        const php::Block& faultEntry,
                        const php::ExnNode& exnNode) {
  // The fault entry must always have been marked as a Fault block,
  // even if it wasn't reachable.
  assert(faultEntry.kind == php::Block::Kind::Fault);

  // Sometimes even a fault funclet entry is completely unreachable.
  // In this case the rest of the assertions do not apply.  (See
  // find_fault_funclets in parse.cpp.)
  if (!reachableSet[faultEntry.id]) return;

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

  if (ends_with_unwind(faultEntry)) return;
  forEachNormalSuccessor(faultEntry, [&] (const php::Block& succ) {
    checkFaultEntryRec(reachableSet, succ, exnNode);
  });
}

void checkExnTreeMore(const boost::dynamic_bitset<>& reachableSet,
                      borrowed_ptr<const ExnNode> node) {
  // All catch entries and fault entries have a few things to assert.
  match<void>(
    node->info,
    [&] (const FaultRegion& fr) {
      checkFaultEntryRec(reachableSet, *fr.faultEntry, *node);
    },
    [&] (const TryRegion& tr) {
      for (DEBUG_ONLY auto& kv : tr.catches) {
        assert(kv.second->kind == php::Block::Kind::CatchEntry);
      }
    }
  );

  for (auto& c : node->children) checkExnTreeMore(reachableSet, borrow(c));
}

bool checkExnTree(const php::Func& f) {
  boost::dynamic_bitset<> seenIds;
  for (auto& n : f.exnNodes) checkExnTreeBasic(seenIds, borrow(n), nullptr);

  // ExnNode ids are contiguous.
  for (size_t i = 0; i < seenIds.size(); ++i) assert(seenIds[i] == true);

  // Some of the assertions below only apply to reachable blocks.
  //
  // NB. this will claim blocks from internal control flow in a DV
  // entry are unreachable.  The bytecode.spec basically bans control
  // flow in a DV entry right now, but we may eventually want to lift
  // that restriction.
  boost::dynamic_bitset<> reachableBlocks(f.blocks.size());
  for (auto& b : rpoSortAddDVs(f)) reachableBlocks[b->id] = true;

  // The following assertions come after the above, because if the
  // tree is totally clobbered it's easy for the wrong ones to fire.
  for (auto& n : f.exnNodes) checkExnTreeMore(reachableBlocks, borrow(n));
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
