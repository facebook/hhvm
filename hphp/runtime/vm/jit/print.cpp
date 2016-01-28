/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/util/disasm.h"
#include "hphp/util/text-color.h"
#include "hphp/util/text-util.h"

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/base/stats.h"

#include "hphp/runtime/vm/jit/asm-info.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/guard-constraints.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/mc-generator.h"

#include "hphp/vixl/a64/disasm-a64.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

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
      case Block::Hint::Unused:      os << "<Unused>"; break;
      case Block::Hint::Unlikely:    os << "<Unlikely>"; break;
      case Block::Hint::Likely:      os << "<Likely>"; break;
      default:
        break;
    }
  }
  os << color(ANSI_COLOR_END);
}

// Simple tuple-like class used to order instructions for printing.
struct InstAreaRange {
  // order by instruction index, area then instruction range.
  bool operator<(const InstAreaRange& other) const {
    return (m_instIdx < other.m_instIdx ||
            (m_instIdx == other.m_instIdx &&
             (m_area < other.m_area ||
              (m_area == other.m_area &&
               (m_instRange.begin() < other.m_instRange.begin() ||
                (m_instRange.begin() == other.m_instRange.begin() &&
                 (m_instRange.end() < other.m_instRange.end())))))));
  }

  InstAreaRange(size_t instIdx,
                AreaIndex area,
                TcaRange instRange)
      : m_instIdx(instIdx),
        m_area(area),
        m_instRange(instRange)
    { }

  size_t m_instIdx{0};
  AreaIndex m_area{AreaIndex::Main};
  TcaRange m_instRange;
};

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////
// IRInstruction.

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

///////////////////////////////////////////////////////////////////////////////
// SSATmp.

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

///////////////////////////////////////////////////////////////////////////////
// Block.

static constexpr auto kIndent = 4;

void disasmRange(std::ostream& os, TCA begin, TCA end) {
  assertx(begin <= end);
  bool const dumpIR = dumpIREnabled(kExtraLevel);

  switch (arch()) {
    case Arch::X64: {
      Disasm disasm(Disasm::Options().indent(kIndent + 4)
                    .printEncoding(dumpIR)
                    .color(color(ANSI_COLOR_BROWN)));
      disasm.disasm(os, begin, end);
      return;
    }

    case Arch::ARM: {
      vixl::Decoder dec;
      vixl::PrintDisassembler disasm(os, kIndent + 4, dumpIR,
                                     color(ANSI_COLOR_BROWN));
      dec.AppendVisitor(&disasm);
      for (; begin < end; begin += vixl::kInstructionSize) {
        dec.Decode(vixl::Instruction::Cast(begin));
      }
      return;
    }

    case Arch::PPC64:
      not_implemented();
  }
  not_reached();
}

template <typename T>
std::vector<TcaRange> makeSortedRanges(const T& itrPair) {
  std::vector<TcaRange> ranges;
  for (auto itr = itrPair.first; itr != itrPair.second; ++itr) {
    ranges.push_back(itr->second);
  }
  std::sort(ranges.begin(),
            ranges.end(),
            [](const TcaRange& a, const TcaRange& b) {
              return a.begin() < b.begin();
            });
  return ranges;
}

void printIRInstruction(std::ostream& os,
                        const IRInstruction& inst,
                        const GuardConstraints* guards,
                        BCMarker& curMarker,
                        const char*& markerEndl) {
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
}

void print(std::ostream& os, const Block* block, AreaIndex area,
           const AsmInfo* asmInfo, const GuardConstraints* guards,
           BCMarker* markerPtr) {
  BCMarker dummy;
  BCMarker& curMarker = markerPtr ? *markerPtr : dummy;

  os << '\n' << std::string(kIndent - 3, ' ');
  printLabel(os, block);
  os << punc(":") << " [profCount=" << block->profCount() << "]";

  switch (block->hint()) {
    case Block::Hint::Unused:   os << "<Unused>";   break;
    case Block::Hint::Unlikely: os << "<Unlikely>"; break;
    case Block::Hint::Likely:   os << "<Likely>";   break;
    default: break;
  }

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

  if (asmInfo) {
    std::vector<const IRInstruction*> instrs;
    std::vector<InstAreaRange> instRanges;
    TcaRange lastRange[kNumAreas];
    size_t instIdx = 0;

    // Collect all the instruction ranges for the current block and sort
    // them.  Entries will be sorted by instruction index, area then machine
    // code pc.  This reflects the order in which code will be printed.
    // IR instructions with no associated assembly will still get entries
    // in the instRanges vector so they will be printed too (they just get
    // an empty machine code range).
    for (auto it = block->begin(); it != block->end(); ++it, ++instIdx) {
      const auto& inst = *it;
      const size_t lastInstRangesSize = instRanges.size();
      instrs.push_back(&inst);  // Map back to IRInstruction from index.

      for (auto i = 0; i < kNumAreas; ++i) {
        const auto instArea = static_cast<AreaIndex>(i);
        auto rngs = asmInfo->instRangesForArea(instArea)[inst];
        for (auto itr = rngs.first; itr != rngs.second; ++itr) {
          instRanges.push_back(InstAreaRange(instIdx,
                                             instArea,
                                             itr->second));
          lastRange[(int)instArea] = itr->second;
        }
      }

      // Add an entry for IRInstructions that have no associated machine
      // code.  Use the end address of the last instruction range to assign
      // an empty range to this element.
      if (instRanges.size() == lastInstRangesSize) {
        instRanges.push_back(
          InstAreaRange(instIdx,
                        AreaIndex::Main,
                        TcaRange(lastRange[(int)AreaIndex::Main].end(),
                                 lastRange[(int)AreaIndex::Main].end())));
      }
    }

    std::sort(instRanges.begin(), instRanges.end());

    const IRInstruction* lastInst = nullptr;
    AreaIndex lastArea = AreaIndex::Main;
    bool printArea = false;
    for (auto itr = instRanges.begin(); itr != instRanges.end(); ++itr) {
      auto currInstIdx = itr->m_instIdx;
      auto currInst = instrs[currInstIdx];
      auto currArea = itr->m_area;
      auto instRange = itr->m_instRange;
      if (lastInst != currInst) {
        if (lastInst && !lastRange[(int)currArea].empty()) os << "\n";
        printIRInstruction(os, *currInst, guards, curMarker, markerEndl);
        printArea = true;
        lastInst = currInst;
        lastRange[(int)currArea] = TcaRange{nullptr,nullptr};
      }
      if (printArea || currArea != lastArea) {
        if (!instRange.empty()) {
          if (!printArea) os << "\n";
          os << std::string(kIndent + 4, ' ') << areaAsString(currArea);
          os << ":\n";
        }
        lastArea = currArea;
        printArea = false;
        lastRange[(int)currArea] = TcaRange{nullptr,nullptr};
      }

      const auto lastEnd = lastRange[(int)currArea].end();

      if (lastEnd && lastEnd != instRange.begin()) {
        // There may be gaps between instruction ranges that have been
        // added by the relocator, e.g. adding nops.  This check will
        // determine if the gap belongs to another instruction or not.
        // If it doesn't belong to any other instruction then print it.
        if (!asmInfo->instRangeExists(currArea,
                                      TcaRange(lastEnd, instRange.begin()))) {
          disasmRange(os, lastEnd, instRange.begin());
        } else {
          os << "\n";
        }
      }
      disasmRange(os, instRange.begin(), instRange.end());
      lastRange[(int)currArea] = instRange;
    }
    os << "\n";
  } else {
    for (auto it = block->begin(); it != block->end(); ++it) {
      printIRInstruction(os, *it, guards, curMarker, markerEndl);
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

///////////////////////////////////////////////////////////////////////////////
// Unit.

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
      mcg->annotations().emplace_back(caption, str.str());
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

}}
