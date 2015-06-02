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

#include "hphp/runtime/vm/jit/print.h"

#include <iostream>
#include <vector>
#include <algorithm>

#include "hphp/util/abi-cxx.h"
#include "hphp/util/text-color.h"
#include "hphp/util/text-util.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/code-gen.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/guard-constraints.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/cfg.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////
namespace {

// Helper for pretty-printing punctuation.
std::string punc(const char* str) {
  return folly::format("{}{}{}",
    color(ANSI_COLOR_DARK_GRAY), str, color(ANSI_COLOR_END)).str();
}

std::string constToString(Type t) {
  std::ostringstream os;
  os << color(ANSI_COLOR_LIGHT_BLUE)
     << t.constValString()
     << color(ANSI_COLOR_END);
  return os.str();
}

void printSrc(std::ostream& ostream, const IRInstruction* inst, uint32_t i) {
  SSATmp* src = inst->src(i);
  if (src != nullptr) {
    print(ostream, src);
  } else {
    ostream << color(ANSI_COLOR_RED)
            << "!!!NULL @ " << i
            << color(ANSI_COLOR_END)
            ;
  }
}

void printLabel(std::ostream& os, const Block* block) {
  os << color(ANSI_COLOR_MAGENTA);
  os << "B" << block->id();
  if (block->isCatch()) {
    os << "<Catch>";
  } else {
    switch (block->hint()) {
      case Block::Hint::Unused:        os << "<Unused>"; break;
      case Block::Hint::Unlikely:    os << "<Unlikely>"; break;
      case Block::Hint::Likely:      os << "<Likely>"; break;
      default:
        break;
    }
  }
  os << color(ANSI_COLOR_END);
}
} // namespace

//////////////////////////////////////////////////////////////////////

/*
 * IRInstruction
 */
void printOpcode(std::ostream& os, const IRInstruction* inst,
                 const GuardConstraints* constraints) {
  os << color(ANSI_COLOR_CYAN)
     << opcodeName(inst->op())
     << color(ANSI_COLOR_END)
     ;

  auto const hasTypeParam = inst->hasTypeParam();
  auto const hasExtra = inst->hasExtra();
  auto const isGuard =
    constraints && !inst->isTransient() && isGuardOp(inst->op());

  if (!hasTypeParam && !hasExtra && !isGuard) return;
  os << color(ANSI_COLOR_LIGHT_BLUE) << '<' << color(ANSI_COLOR_END);

  if (hasTypeParam) {
    os << color(ANSI_COLOR_GREEN)
       << inst->typeParam().toString()
       << color(ANSI_COLOR_END)
       ;
    if (hasExtra || isGuard) os << punc(",");
  }

  if (hasExtra) {
    os << color(ANSI_COLOR_GREEN)
       << showExtra(inst->op(), inst->rawExtra())
       << color(ANSI_COLOR_END);
    if (isGuard) os << punc(",");
  }

  if (isGuard) {
    auto it = constraints->guards.find(inst);
    os << (it == constraints->guards.end() ? "unused" : it->second.toString());
  }

  os << color(ANSI_COLOR_LIGHT_BLUE)
     << '>'
     << color(ANSI_COLOR_END);
}

void printSrcs(std::ostream& os, const IRInstruction* inst) {
  bool first = true;
  if (inst->op() == IncStat) {
    os << " " << Stats::g_counterNames[inst->src(0)->intVal()]
       << ", " << inst->src(1)->intVal();
    return;
  }
  for (uint32_t i = 0, n = inst->numSrcs(); i < n; i++) {
    if (!first) {
      os << punc(", ");
    } else {
      os << " ";
      first = false;
    }
    printSrc(os, inst, i);
  }
}

void printDsts(std::ostream& os, const IRInstruction* inst) {
  const char* sep = "";
  for (unsigned i = 0, n = inst->numDsts(); i < n; i++) {
    os << punc(sep);
    print(os, inst->dst(i));
    sep = ", ";
  }
}

void printInstr(std::ostream& ostream, const IRInstruction* inst,
                const GuardConstraints* guards) {
  printDsts(ostream, inst);
  if (inst->numDsts()) ostream << punc(" = ");
  printOpcode(ostream, inst, guards);
  printSrcs(ostream, inst);
}

void print(std::ostream& ostream, const IRInstruction* inst,
           const GuardConstraints* guards) {
  if (!inst->isTransient()) {
    ostream << color(ANSI_COLOR_YELLOW);
    ostream << folly::format("({:02d}) ", inst->id());
    ostream << color(ANSI_COLOR_END);
  }
  printInstr(ostream, inst, guards);
  if (Block* taken = inst->taken()) {
    ostream << punc(" -> ");
    printLabel(ostream, taken);
  }
}

void print(const IRInstruction* inst) {
  print(std::cerr, inst);
  std::cerr << std::endl;
}

//////////////////////////////////////////////////////////////////////

void print(std::ostream& os, const SSATmp* tmp) {
  if (tmp->inst()->is(DefConst)) {
    os << constToString(tmp->type());
    return;
  }
  os << color(ANSI_COLOR_WHITE);
  os << "t" << tmp->id();
  os << color(ANSI_COLOR_END);
  os << punc(":")
     << color(ANSI_COLOR_GREEN)
     << tmp->type().toString()
     << color(ANSI_COLOR_END)
     ;
}

void print(const SSATmp* tmp) {
  print(std::cerr, tmp);
  std::cerr << std::endl;
}

//////////////////////////////////////////////////////////////////////

/*
 * Block
 */
static constexpr auto kIndent = 4;

void disasmRange(std::ostream& os, TCA begin, TCA end) {
  mcg->backEnd().disasmRange(os, kIndent, dumpIREnabled(kExtraLevel),
                             begin, end);
}

void print(std::ostream& os, const Block* block, AreaIndex area,
           const AsmInfo* asmInfo, const GuardConstraints* guards,
           BCMarker* markerPtr) {
  BCMarker dummy;
  BCMarker& curMarker = markerPtr ? *markerPtr : dummy;

  TcaRange blockRange = asmInfo ? asmInfo->blockRangesForArea(area)[block]
                                : TcaRange { nullptr, nullptr };

  os << '\n' << std::string(kIndent - 3, ' ');
  printLabel(os, block);
  os << punc(":");
  auto& preds = block->preds();
  if (!preds.empty()) {
    os << " (preds";
    for (auto const& edge : preds) {
      os << " B" << edge.from()->id();
    }
    os << ')';
  }
  os << "\n";

  if (block->empty()) {
    os << std::string(kIndent, ' ') << "empty block\n";
    return;
  }

  const char* markerEndl = "";
  for (auto it = block->begin(); it != block->end();) {
    auto& inst = *it; ++it;

    if (inst.marker() != curMarker) {
      std::ostringstream mStr;
      auto const& newMarker = inst.marker();
      if (!newMarker.hasFunc()) {
        os << color(ANSI_COLOR_BLUE)
           << std::string(kIndent, ' ')
           << "--- invalid marker"
           << color(ANSI_COLOR_END)
           << '\n';
      } else {
        auto func = newMarker.func();
        if (!curMarker.hasFunc() || func != curMarker.func()) {
          func->prettyPrint(mStr, Func::PrintOpts().noFpi());
        }
        mStr << std::string(kIndent, ' ')
             << newMarker.show()
             << '\n';

        auto bcOffset = newMarker.bcOff();
        func->unit()->prettyPrint(
          mStr, Unit::PrintOpts()
          .range(bcOffset, bcOffset+1)
          .noLineNumbers()
          .noFuncs()
          .indent(0));
        std::vector<std::string> vec;
        folly::split('\n', mStr.str(), vec);
        os << markerEndl;
        markerEndl = "\n";
        for (auto& s : vec) {
          if (s.empty()) continue;
          os << color(ANSI_COLOR_BLUE) << s << color(ANSI_COLOR_END) << '\n';
        }
      }

      curMarker = newMarker;
    }

    if (inst.op() == DefLabel) {
      // print phi pseudo-instructions
      for (unsigned i = 0, n = inst.numDsts(); i < n; ++i) {
        os << std::string(kIndent +
                          folly::format("({}) ", inst.id()).str().size(),
                          ' ');
        auto dst = inst.dst(i);
        jit::print(os, dst);
        os << punc(" = ") << color(ANSI_COLOR_CYAN) << "phi "
           << color(ANSI_COLOR_END);
        bool first = true;
        inst.block()->forEachSrc(i, [&](IRInstruction* jmp, SSATmp*) {
            if (!first) os << punc(", ");
            first = false;
            printSrc(os, jmp, i);
            os << punc("@");
            printLabel(os, jmp->block());
          });
        os << '\n';
      }
    }

    os << std::string(kIndent, ' ');
    jit::print(os, &inst, guards);
    os << '\n';

    if (asmInfo) {
      // There can be asm ranges in areas other than the one this blocks claims
      // to be in so we have to iterate all the areas to be sure to get
      // everything.
      for (auto i = 0; i < kNumAreas; ++i) {
        AreaIndex currentArea = static_cast<AreaIndex>(i);
        TcaRange instRange = asmInfo->instRangesForArea(currentArea)[inst];
        if (!instRange.empty()) {
          os << std::string(kIndent + 4, ' ') << areaAsString(currentArea);
          os << ":\n";
          disasmRange(os, instRange.begin(), instRange.end());
          os << '\n';
          if (currentArea == area) {
            // FIXME: this used to be an assertion
            auto things_are_ok =
              instRange.end() >= blockRange.start() &&
              instRange.end() <= blockRange.end();
            if (things_are_ok) {
              blockRange = TcaRange(instRange.end(), blockRange.end());
            } else {
              // Don't crash; do something broken instead.
              os << "<note: print range is probably incorrect right now>\n";
            }
          }
        }
      }
    }
  }

  if (asmInfo) {
    // Print code associated with the block that isn't tied to any instruction.
    if (!blockRange.empty()) {
      os << std::string(kIndent, ' ') << punc("A:") << "\n";
      disasmRange(os, blockRange.start(), blockRange.end());
      os << '\n';
    }
  }

  os << std::string(kIndent - 2, ' ');
  auto next = block->empty() ? nullptr : block->next();
  if (next) {
    os << punc("-> ");
    printLabel(os, next);
    os << '\n';
  } else {
    os << "no fallthrough\n";
  }
}

void print(const Block* block) {
  print(std::cerr, block, AreaIndex::Main);
  std::cerr << std::endl;
}

std::string Block::toString() const {
  std::ostringstream out;
  print(out, this, AreaIndex::Main);
  return out.str();
}

//////////////////////////////////////////////////////////////////////

void printOpcodeStats(std::ostream& os, const BlockList& blocks) {
  uint32_t counts[kNumOpcodes];
  memset(counts, 0, sizeof(counts));

  for (auto block : blocks) {
    for (auto& inst : *block) ++counts[static_cast<size_t>(inst.op())];
  }

  os << "\nopcode counts:\n";
  for (unsigned i = 0; i < kNumOpcodes; ++i) {
    if (counts[i] == 0) continue;
    auto op = safe_cast<Opcode>(i);
    os << folly::format("{:>5} {}\n", counts[i], opcodeName(op));
  }
  os << '\n';
}

/*
 * Unit
 */
void print(std::ostream& os, const IRUnit& unit, const AsmInfo* asmInfo,
           const GuardConstraints* guards) {
  // For nice-looking dumps, we want to remember curMarker between blocks.
  BCMarker curMarker;
  static bool dotBodies = getenv("HHIR_DOT_BODIES");

  auto blocks = rpoSortCfg(unit);
  // Partition into main, cold and frozen, without changing relative order.
  auto cold = std::stable_partition(blocks.begin(), blocks.end(),
    [&] (Block* b) {
      return b->hint() == Block::Hint::Neither ||
             b->hint() == Block::Hint::Likely;
    }
  );
  auto frozen = std::stable_partition(cold, blocks.end(),
    [&] (Block* b) { return b->hint() == Block::Hint::Unlikely; }
  );

  if (dumpIREnabled(kExtraExtraLevel)) printOpcodeStats(os, blocks);

  // Print the block CFG above the actual code.

  auto const retreating_edges = findRetreatingEdges(unit);
  os << "digraph G {\n";
  for (auto block : blocks) {
    if (block->empty()) continue;
    if (dotBodies && block->hint() != Block::Hint::Unlikely &&
        block->hint() != Block::Hint::Unused) {
      // Include the IR in the body of the node
      std::ostringstream out;
      print(out, block, AreaIndex::Main, asmInfo, guards, &curMarker);
      auto bodyRaw = out.str();
      std::string body;
      body.reserve(bodyRaw.size() * 1.25);
      for (auto c : bodyRaw) {
        if (c == '\n')      body += "\\n";
        else if (c == '"')  body += "\\\"";
        else if (c == '\\') body += "\\\\";
        else                body += c;
      }
      os << folly::format("B{} [shape=\"box\" label=\"{}\"]\n",
                          block->id(), body);
    }

    auto next = block->nextEdge();
    auto taken = block->takenEdge();
    if (!next && !taken) continue;
    auto edge_color = [&] (Edge* edge) {
      auto const target = edge->to();
      return
        target->isCatch() ? " [color=blue]" :
        target->isExit() ? " [color=cyan]" :
        retreating_edges.count(edge) ? " [color=red]" :
        target->hint() == Block::Hint::Unlikely ? " [color=green]" : "";
    };
    auto show_edge = [&] (Edge* edge) {
      os << folly::format(
        "B{} -> B{}{}",
        block->id(),
        edge->to()->id(),
        edge_color(edge)
      );
    };
    if (next) {
      show_edge(next);
      if (taken) os << "; ";
    }
    if (taken) show_edge(taken);
    os << "\n";
  }
  os << "}\n";

  AreaIndex currentArea = AreaIndex::Main;
  curMarker = BCMarker();
  for (auto it = blocks.begin(); it != blocks.end(); ++it) {
    if (it == cold) {
      os << folly::format("\n{:-^60}", "cold blocks");
      currentArea = AreaIndex::Cold;
    }
    if (it == frozen) {
      os << folly::format("\n{:-^60}", "frozen blocks");
      currentArea = AreaIndex::Frozen;
    }
    print(os, *it, currentArea, asmInfo, guards, &curMarker);
  }
}

void print(const IRUnit& unit) {
  print(std::cerr, unit);
  std::cerr << std::endl;
}

std::string show(const IRUnit& unit) {
  std::ostringstream out;
  print(out, unit);
  return out.str();
}

std::string banner(const char* caption) {
  return folly::sformat(
    "{}{:-^80}{}\n",
    color(ANSI_COLOR_BLACK, ANSI_BGCOLOR_GREEN),
    caption,
    color(ANSI_COLOR_END)
  );
}

// Suggested captions: "before jiffy removal", "after goat saturation",
// etc.
void printUnit(int level, const IRUnit& unit, const char* caption, AsmInfo* ai,
               const GuardConstraints* guards) {
  if (dumpIREnabled(level)) {
    std::ostringstream str;
    str << banner(caption);
    print(str, unit, ai, guards);
    str << banner("");
    if (HPHP::Trace::moduleEnabledRelease(HPHP::Trace::printir, level)) {
      HPHP::Trace::traceRelease("%s\n", str.str().c_str());
    }
    if (RuntimeOption::EvalDumpIR >= level) {
      mcg->annotations().emplace_back(caption, std::move(str.str()));
    }
  }
}

}}
