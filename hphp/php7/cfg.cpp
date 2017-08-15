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

#include "hphp/php7/cfg.h"

#include "hphp/php7/compiler.h"
#include "hphp/util/match.h"

#include <boost/variant.hpp>

namespace HPHP { namespace php7 {

using namespace bc;

CFG::CFG(Bytecode bc)
  : m_entry(makeBlock())
  , m_continuation(m_entry) {
  then(bc);
}

CFG::CFG(std::initializer_list<Bytecode> list)
  : m_entry(makeBlock())
  , m_continuation(m_entry) {
  for (const auto& bc : list) {
    then(bc);
  }
}

CFG::CFG(LinkTarget target)
  : m_entry(makeBlock())
  , m_continuation(nullptr) {
  m_unresolvedLinks.push_back({m_entry, target});
}

Block* CFG::makeBlock() {
  m_blocks.push_back(std::make_unique<Block>());
  auto blk = m_blocks.back().get();
  blk->id = m_maxId++;
  return blk;
}

void CFG::merge(CFG cfg) {
  for (auto& blk : cfg.m_blocks) {
    blk->id += m_maxId;
    m_blocks.emplace_back(std::move(blk));
  }
  // copy defined labels from cfg
  for (const auto& label : cfg.m_labels) {
    // if we already had a label with this name, we have a problem!
    if (m_labels.count(label.first) > 0) {
      panic("CFG defines duplicate label " + label.first);
    }
    link(label.first, label.second);
  }
  // copy unresolved links from cfg
  for (auto& linkage : cfg.m_unresolvedLinks) {
    if (auto labelTarget = boost::get<LabelTarget>(&linkage.target)) {
      auto iter = m_labels.find(labelTarget->name);

      if (iter != m_labels.end()) {
        linkage.trampoline->exit(Jmp{iter->second});
        continue;
      }
    }
    // if we couldn't resolve this link, add it to ours
    m_unresolvedLinks.push_back(std::move(linkage));
  }

  // add top regions from cfg to ours
  for (auto& region : cfg.m_topRegions) {
    m_topRegions.emplace_back(std::move(region));
  }

  m_maxId += cfg.m_maxId;
}

CFG&& CFG::then(CFG cfg) {
  if (!m_continuation) {
    m_continuation = cfg.m_continuation;
    merge(std::move(cfg));
    return self();
  }
  m_continuation->exit(Jmp{cfg.m_entry});
  m_continuation = cfg.m_continuation;
  merge(std::move(cfg));
  return self();
}

CFG&& CFG::then(const std::string& label) {
  return then(CFG(LabelTarget{label}));
}

CFG&& CFG::thenJmp(Block* target) {
  if (!m_continuation) {
    return self();
  }

  m_continuation->exit(Jmp{target});
  m_continuation = target;
  return self();
}

CFG&& CFG::thenExitRaw(ExitOp eo) {
  if (!m_continuation) {
    return self();
  }

  m_continuation->exit(eo);
  return self();
}

CFG&& CFG::then(Bytecode bc) {
  if (!m_continuation) {
    return self();
  }

  if (m_continuation->exited) {
    auto next = makeBlock();
    next->emit(bc);
    thenJmp(next);
  } else {
    m_continuation->emit(bc);
  }

  return self();
}

CFG&& CFG::branchZ(CFG cfg) {
  thenExitRaw(JmpZ{cfg.m_entry});
  merge(std::move(cfg));
  return self();
}
CFG&& CFG::branchZ(const std::string& label) {
  return branchZ(CFG(LabelTarget{label}));
}

CFG&& CFG::branchNZ(CFG cfg) {
  thenExitRaw(JmpNZ{cfg.m_entry});
  merge(std::move(cfg));
  return self();
}
CFG&& CFG::branchNZ(const std::string& label) {
  return branchNZ(CFG(LabelTarget{label}));
}

CFG&& CFG::continueFrom(Block* blk) {
  m_continuation = blk;
  return self();
}

CFG&& CFG::then(LinkTarget target) {
  return then(CFG(target));
}

CFG&& CFG::thenThrow() {
  return thenExitRaw(Throw{});
}

CFG&& CFG::thenReturn(Flavor flavor) {
  return then(ReturnTarget{flavor});
}

CFG&& CFG::thenContinue() {
  return then(LoopTarget::Continue);
}

CFG&& CFG::thenBreak() {
  return then(LoopTarget::Break);
}

CFG&& CFG::thenLabel(const std::string& name) {
  return then(LabelTarget{name});
}

CFG&& CFG::link(const std::string& name, Block* dest) {
  m_labels.insert({name, dest});

  resolveLinks([&](Linkage& link) -> Block* {
    if (link.target == LinkTarget{LabelTarget{name}}) {
      return dest;
    }
    return nullptr;
  });

  return self();
}

CFG&& CFG::strip(const std::string& name) {
  m_labels.erase(name);
  return self();
}

CFG&& CFG::replace(const std::string& label, CFG cfg) {
  link(label, cfg.m_entry);
  strip(label);
  merge(std::move(cfg));
  return self();
}

CFG&& CFG::makeExitsReal() {
  // we must avoid creating a new block for any exit since this would change
  // the region that we, e.g. throw from
  for (auto& linkage : m_unresolvedLinks) {
    auto trampoline = linkage.trampoline;
    match<void>(linkage.target,
      [&](const ReturnTarget& ret) {
        switch (ret.flavor) {
          case Cell:
            trampoline->exit(RetC{});
            break;
          case Ref:
            trampoline->exit(RetV{});
            break;
          default:
            panic("bad return flavor");
        }
      },
      [&](const LoopTarget& loopTarget) {
        switch(loopTarget) {
          case Break:
            throw LanguageException(
              "'break' not in the 'loop' or 'switch' context"
            );
          case Continue:
            throw LanguageException(
              "'continue' not in the 'loop' or 'switch' context"
            );
        }
        panic("bad loop target");
      },
      [&](const LabelTarget& label) {
        panic("unresolved label" + label.name);
      }
    );
  }
  m_unresolvedLinks.clear();
  return self();
}

CFG&& CFG::linkLoop(CFG breakTarget, CFG continueTarget) {
  resolveLinks([&](Linkage& link) -> Block* {
    if (auto loopTarget = boost::get<LoopTarget>(&link.target)) {
      switch(*loopTarget) {
        case Break:
          return breakTarget.m_entry;
        case Continue:
          return continueTarget.m_entry;
      }
    }
    return nullptr;
  });

  merge(std::move(breakTarget));
  merge(std::move(continueTarget));

  return self();
}

CFG&& CFG::addExnHandler(CFG handler) {
  // create a region and put all our blocks in it
  auto protRegion = std::make_unique<Region>(Region::Kind::Protected);
  protRegion->handler = handler.m_entry;
  inRegion(std::move(protRegion));

  auto cont = makeBlock();
  merge(handler
    .thenJmp(cont)
    .inRegion(std::make_unique<Region>(Region::Kind::Catch)));
  return thenJmp(cont);
}

CFG&& CFG::inRegion(std::unique_ptr<Region> reg) {
  // place any blocks without a region in this region
  for (auto& blk : m_blocks) {
    if (!blk->region) {
      blk->region = reg.get();
    }
  }
  // place all top regions in this region
  for (auto& r : m_topRegions) {
    reg->addChild(std::move(r));
  }
  // the only top region is now reg
  m_topRegions.clear();
  m_topRegions.push_back(std::move(reg));
  return self();
}

namespace {

/* Find all blocks this exit can target */
std::vector<Block*> exitTargets(const ExitOp& exit) {
  std::vector<Block*> targets;

  match<void>(exit,
    [&](const Jmp& j) { targets.push_back(j.imm1); },
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
    [&](const RetC& /*r*/) {},
    [&](const RetV& /*r*/) {},
    [&](const Unwind& /*u*/) {},
    [&](const Throw& /*t*/) {},
    [&](const Fatal& /*t*/) {}
  );

  return targets;
}

/* Compute the regions you have to enter to move from prev to next
 *
 * E.g if regions are nested like this:
 *   (A (B (C) (D)) (E))
 * enteredRegions(A, E) -> [E]
 * enteredRegions(A, D) -> [B, D]
 * enteredRegions(nullptr, C) -> [A, B, C]
 *
 * prev may be null, in which case this returns all ancestors of next */
std::vector<Region*> enteredRegions(Region* prev, Region* next) {
  std::vector<Region*> entered;

  while (next != prev) {
    entered.push_back(next);
    next = next->parent;
  }

  std::reverse(entered.begin(), entered.end());

  return entered;
}

} // namespace

void CFG::visit(CFGVisitor&& visitor) const {
  std::unordered_set<Block*> visited;
  std::deque<Block*> breadcrumbs;

  auto region = m_entry->region;
  breadcrumbs.push_front(m_entry);

  while (region || !breadcrumbs.empty()) {
    // find the next block in breadcrumbs that is still in the current region
    // or to the next fault funclet if we're not in any region
    auto iter = std::find_if(breadcrumbs.begin(), breadcrumbs.end(),
        [&](Block* blk) {
          return region->containsBlock(blk);
        });

    // our only paths are out of the current region, so leave this region
    // and enter its catch handler, if it has one
    if (iter == breadcrumbs.end()) {
      switch (region->kind) {
        case Region::Kind::Protected:
          assert(region->handler->region->kind == Region::Kind::Catch);
          assert(region->handler->region->parent == region->parent);
          visitor.beginCatch();
          if (0 == visited.count(region->handler)) {
            breadcrumbs.push_front(region->handler);
            visited.insert(region->handler);
          }
          region = region->handler->region;
          break;
        case Region::Kind::Catch:
          visitor.endRegion();
          region = region->parent;
          break;
        case Region::Kind::Entry:
          region = region->parent;
          break;
      }
      continue;
    }

    auto blk = *iter;
    breadcrumbs.erase(iter);

    // if we enter regions, make sure to let the visitor know
    for (const auto& reg : enteredRegions(region, blk->region)) {
      switch (reg->kind) {
        case Region::Kind::Protected:
          visitor.beginTry();
          break;
        case Region::Kind::Catch:
        case Region::Kind::Entry:
          break;
      }
    }

    region = blk->region;
    visitor.block(blk);

    // add all of the exit targets to the queue
    const auto& exits = blk->exits;
    for (auto riter = exits.rbegin(); riter != exits.rend(); riter++) {
      for (Block* child : exitTargets(*riter)) {
        if (0 == visited.count(child)) {
          breadcrumbs.push_front(child);
          visited.insert(child);
        }
      }
    }
  }

}

}} // namespace HPHP::php7
