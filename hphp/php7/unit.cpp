/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/php7/unit.h"

#include "hphp/util/match.h"

#include <stack>
#include <unordered_set>

namespace HPHP { namespace php7 {

Function::Function(Unit* parent, const std::string& name)
  : name(name),
    attr(),
    parent(parent) {
  entry = allocateBlock();
}

Block* Function::allocateBlock() {
  uint64_t id = blocks.size();

  auto block = std::make_unique<Block>();
  block->id = id;
  auto borrowed = block.get();
  blocks.emplace_back(std::move(block));

  return borrowed;
}

Block* Function::getBlock(uint64_t id) {
  if (id >= blocks.size()) {
    return nullptr;
  }

  return blocks[id].get();
}

namespace {

std::vector<Block*> exitTargets(const Block::ExitOp& exit) {
  using namespace bc;
  std::vector<Block*> targets;

  match<void>(exit, [&](const Jmp& j) { targets.push_back(j.imm1); },
              [&](const JmpNS& j) { targets.push_back(j.imm1); },
              [&](const JmpZ& j) { targets.push_back(j.imm1); },
              [&](const JmpNZ& j) { targets.push_back(j.imm1); },
              [&](const Switch& s) {
                targets.insert(targets.end(), s.imm3.begin(), s.imm3.end());
              },
              [&](const SSwitch& s) {
                const StringOffsetVector& sov = s.imm1;
                for (const auto& pair : sov.branches) {
                  targets.push_back(pair.second);
                }
                targets.push_back(sov.defaultBranch);
              },
              [&](const RetC& /*r*/) {}, [&](const RetV& /*r*/) {},
              [&](const Unwind& /*u*/) {}, [&](const Throw& /*t*/) {},
              [&](const Fatal& /*t*/) {});

  return targets;
}

} // namespace

std::vector<Block*> serializeControlFlowGraph(Block* entry) {
  // traverse the block graph in a DFS and build a reverse postorder
  std::vector<Block*> ordering;

  std::unordered_set<Block*> visited;
  std::unordered_set<Block*> added;

  std::stack<Block*> breadcrumbs;
  added.insert(entry);
  breadcrumbs.push(entry);

  // every node is visited twice, once to add its children to the stack, then
  // (after its children) to add it to the ordering
  while (!breadcrumbs.empty()) {
    // we *don't* pop here, since we want to visit a node again, after all its
    // children are fully processed, to add it to the ordering
    auto blk = breadcrumbs.top();

    if (0 != visited.count(blk)) {
      // we've seen this node before so it's either a leaf or all its
      // dependncies are added already
      breadcrumbs.pop();
      ordering.push_back(blk);
      continue;
    }

    const auto& exits = blk->exits;
    // iterate *backwards* to preserve the order of the exits
    for (auto iter = exits.rbegin(); iter != exits.rend(); iter++) {
      auto exit = *iter;
      for (Block* child : exitTargets(exit)) {
        // if this edge is not a back edge (or a self edge)
        if (0 == added.count(child)) {
          breadcrumbs.push(child);
          added.insert(child);
        }
      }
    }

    visited.insert(blk);
  }

  // reverse the ordering to get an actual topological order
  std::reverse(ordering.begin(), ordering.end());

  return ordering;
}

std::unique_ptr<Unit> makeFatalUnit(const std::string& filename,
                                    const std::string& msg) {
  auto unit = std::make_unique<Unit>();
  unit->name = filename;

  auto blk = unit->getPseudomain()->entry;
  blk->emit(bc::String{msg});
  blk->exit(bc::Fatal{FatalOp::Parse});
  return unit;
}
}} // HPHP::php7
