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

#include "hphp/php7/analysis.h"

#include "hphp/util/match.h"

namespace HPHP { namespace php7 {

namespace {

using bc::Local;
using bc::NamedLocal;
using bc::UniqueLocal;

struct LocalSet {
  void add(const Local& local) {
    match<void>(local,
      [&](const NamedLocal& named) {
        names.insert(named.name);
      },
      [&](const UniqueLocal& unique) {
        uniqueIds.insert(unique.id);
      }
    );
  }

  void allocateUniqueIds() {
    uint32_t id = names.size();
    for (auto& idPtr : uniqueIds) {
      *idPtr = id++;
    }
  }

  std::unordered_set<std::string> names;
  std::unordered_set<std::shared_ptr<uint32_t>> uniqueIds;
};

struct LocalsVisitor : CFGVisitor {
  explicit LocalsVisitor(LocalSet& locals)
    : locals(locals) {}

  void beginTry() override {}

  void beginCatch() override {}

  void endRegion() override {}

  void block(Block* blk) override {
    for (const auto& bc : blk->code) {
      bc.visit(*this);
    }
  }

  template <class T>
  void bytecode(const T& bc) {
    bc.visit_imms(*this);
  }

  void imm(Local local) {
    locals.add(local);
  }

  template <class T>
  void imm(const T&) {}

  LocalSet& locals;
};

} // namespace

std::unordered_set<std::string> analyzeLocals(const Function& func) {
  LocalSet locals;
  for (const auto& param : func.params) {
    locals.add(bc::NamedLocal{param.name});
  }
  func.cfg.visit(LocalsVisitor(locals));
  locals.allocateUniqueIds();
  return locals.names;
}

////////////////////////////////////////////////////////////////////////////////

namespace {

struct ClassrefVisitor : CFGVisitor {
  explicit ClassrefVisitor() {}

  void beginTry() override {}

  void beginCatch() override {}

  void endRegion() override {}

  void block(Block* blk) override {
    for (auto& bc : blk->code) {
      bc.visit(*this);
    }
    end();
  }

  void end() const {
#ifndef NDEBUG
    // all classrefs must be free at block exit
    for (const bool free : m_slotsFree) {
      assert(free);
    }
#endif // NDEBUG
  }

  uint32_t classrefSlotsUsed() const {
    return m_slotsFree.size();
  }

  template <class T>
  void bytecode(T& bc) {
    bc.visit_imms(*this);
  }

  void imm(bc::ReadClassref& read) {
    // this clasref must have been correctly allocated
    assert(read.slot.allocated());
    // the slot is now available
    m_slotsFree[*read.slot.id] = true;
  }

  void imm(bc::WriteClassref& write) {
    // find the first empty slot
    for (uint32_t i = 0; i < m_slotsFree.size(); i++) {
      if (m_slotsFree[i]) {
        // assign this classref to this index
        // and mark the slot as used
        *write.slot.id = i;
        m_slotsFree[i] = false;
        return;
      }
    }
    // if there wasn't a free slot already allocated, grow the slot set
    *write.slot.id = m_slotsFree.size();
    m_slotsFree.emplace_back(false);
  }

  template <class T>
  void imm(const T&) {}

 private:
  std::vector<bool> m_slotsFree;
};

} // namespace

uint32_t analyzeClassrefs(const CFG& cfg) {
  ClassrefVisitor visitor;
  cfg.visit(visitor);
  visitor.end();
  return visitor.classrefSlotsUsed();
}

////////////////////////////////////////////////////////////////////////////////

namespace {

struct LabelUseVisitor : CFGVisitor {
  explicit LabelUseVisitor(std::unordered_multiset<Block*>& references)
    : references(references) {}

  void beginTry() override {}

  void beginCatch() override {}

  void endRegion() override {}

  void block(Block* blk) override {
    for (Block* target : blk->exitTargets()) {
      references.insert(target);
    }
  }

  std::unordered_multiset<Block*>& references;
};

struct BlockCoalescingVisitor : CFGVisitor {
  explicit BlockCoalescingVisitor(std::unordered_multiset<Block*> references)
    : references(std::move(references)) {}

  void beginTry() override {}

  void beginCatch() override {}

  void endRegion() override {}

  void block(Block* blk) override {
    // finds the given block's single unconditional exit if it has one
    const auto hasFallthrough = [&](Block* blk) -> Block* {
      // if this block has one exit ...
      if (blk->exits.size() == 1) {
        auto exit = blk->exits[0];
        // ... that's an unconditional jump ...
        if (auto jmp = boost::get<bc::Jmp>(&exit)) {
          Block* target = jmp->imm1;
          // ... to a block other than itself ...
          if (target != blk) {
            return target;
          }
        }
      }
      return nullptr;
    };

    // if this block has one unconditional exit
    if (auto target = hasFallthrough(blk)) {
      // and its jump target is referenced by only us
      // and is in the same region
      while (target
          && references.count(target) == 1
          && target->region == blk->region) {
        // coalesce these blocks into one
        blk->code.insert(
          blk->code.end(),
          target->code.begin(),
          target->code.end()
        );
        // target will be dead at this point but that's fine
        std::swap(blk->exits, target->exits);
        // repeat this process if we can
        target = hasFallthrough(blk);
      }
    }
  }

  std::unordered_multiset<Block*> references;
};

} // namespace

void simplifyCFG(CFG& cfg) {
  std::unordered_multiset<Block*> references;
  // mark where blocks are referenced by jumps
  cfg.visit(LabelUseVisitor(references));
  // coalesce blocks
  cfg.visit(BlockCoalescingVisitor(std::move(references)));
}

}} // namespace HPHP::php7
