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

CFG&& CFG::then(LinkTarget target) {
  return then(CFG(target));
}

CFG&& CFG::thenThrow() {
  return then(ThrowTarget{});
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

CFG&& CFG::makeExitsReal() {
  for (auto& linkage : m_unresolvedLinks) {
    linkage.trampoline->exit(Jmp{
      match<Block*>(linkage.target,
        [&](const ReturnTarget& ret) {
          auto blk = makeBlock();
          switch (ret.flavor) {
            case Cell:
              blk->exit(RetC{});
              break;
            case Ref:
              blk->exit(RetV{});
              break;
            default:
              panic("bad return flavor");
          }
          return blk;
        },
        [&](const ThrowTarget& /*throw*/) {
          auto blk = makeBlock();
          blk->exit(Throw{});
          return blk;
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
          return nullptr;
        },
        [&](const LabelTarget& label) {
          panic("unresolved label" + label.name);
          return nullptr;
        }
    )});
  }
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

}} // namespace HPHP::php7
