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

#include "hphp/runtime/vm/jit/print.h"

#include <folly/dynamic.h>
#include <folly/json.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

#include "hphp/util/abi-cxx.h"
#include "hphp/util/arch.h"
#include "hphp/util/disasm.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/text-color.h"
#include "hphp/util/text-util.h"

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/stats.h"

#include "hphp/runtime/vm/jit/array-access-profile.h"
#include "hphp/runtime/vm/jit/array-kind-profile.h"
#include "hphp/runtime/vm/jit/asm-info.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/call-target-profile.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/cls-cns-profile.h"
#include "hphp/runtime/vm/jit/decref-profile.h"
#include "hphp/runtime/vm/jit/incref-profile.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/guard-constraints.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/meth-profile.h"
#include "hphp/runtime/vm/jit/release-vv-profile.h"
#include "hphp/runtime/vm/jit/switch-profile.h"
#include "hphp/runtime/vm/jit/type-profile.h"

#include "hphp/ppc64-asm/asm-ppc64.h"
#include "hphp/ppc64-asm/dasm-ppc64.h"

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

  bool isContiguous(const InstAreaRange& other) const {
    // TODO(T52857006) - check assertions as described in D16623372
    return (m_instRange.end() == other.m_instRange.begin() &&
            m_area == other.m_area &&
            m_instIdx == other.m_instIdx);
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

bool dumpPrettyIR(int level) {
  return HPHP::Trace::moduleEnabledRelease(HPHP::Trace::printir, level) ||
         (RuntimeOption::EvalDumpIR >= level);
}

bool dumpJsonIR(int level) {
  return HPHP::Trace::moduleEnabledRelease(HPHP::Trace::printir_json, level) ||
         (RuntimeOption::EvalDumpIRJson >= level);
}

bool dumpRuntimeIR(int level) {
  return RuntimeOption::EvalDumpIR >= level ||
         RuntimeOption::EvalDumpIRJson >= level;
}

///////////////////////////////////////////////////////////////////////////////

}

static constexpr auto kIndent = 4;

namespace get_json {
using folly::dynamic;

dynamic getSSATmp(const SSATmp* tmp) {
  return dynamic::object("id", tmp->id())
                        ("type", tmp->type().toString());
}

dynamic getLabel(const Block* block) {
  dynamic id = block->id();

  return dynamic::object("id", id)
                        ("isCatch", block->isCatch())
                        ("hint", blockHintName(block->hint()));
}


dynamic getOpcode(const IRInstruction* inst,
                  const GuardConstraints* constraints) {
  const dynamic typeParam = inst->hasTypeParam() ?
                            inst->typeParam().toString() :
                            dynamic(nullptr);
  const dynamic extra = inst->hasExtra() ?
                        showExtra(inst->op(), inst->rawExtra()) :
                        dynamic(nullptr);

  const bool isGuard = constraints &&
                       !inst->isTransient() &&
                       isGuardOp(inst->op());
  dynamic guard;
  if (isGuard) {
    auto const it = constraints->guards.find(inst);
    guard = (it == constraints->guards.end() ?
             "unused" :
             it->second.toString());
  } else {
    guard = dynamic(nullptr);
  }

  return dynamic::object("opcodeName", opcodeName(inst->op()))
                        ("typeParam", typeParam)
                        ("extra", extra)
                        ("guard", guard);
}

dynamic getSrcs(const IRInstruction* inst) {
  // TODO(T52857257)
  if (inst->op() == IncStat) {
    return dynamic::object("counterName",
                           Stats::g_counterNames[inst->src(0)->intVal()]);
  }
  dynamic srcs = dynamic::array;
  for (uint32_t i = 0, n = inst->numSrcs(); i < n; i++) {
    srcs.push_back(getSSATmp(inst->src(i)));
  }
  return srcs;
}

dynamic getDsts(const IRInstruction* inst) {
  dynamic dsts = dynamic::array;
  for (uint32_t i = 0, n = inst->numDsts(); i < n; i++) {
    dsts.push_back(getSSATmp(inst->dst(i)));
  }
  return dsts;
}

dynamic getIRInstruction(const IRInstruction& inst,
                         const GuardConstraints* guards) {
  dynamic result = dynamic::object;
  dynamic markerObj = dynamic::object;
  std::ostringstream mStr;
  std::ostringstream funcStr;
  auto const& newMarker = inst.marker();
  if (!newMarker.hasFunc()) {
    markerObj = dynamic(nullptr);
  } else {
    auto func = newMarker.func();
    func->prettyPrint(mStr, Func::PrintOpts());
    mStr << std::string(kIndent, ' ')
         << newMarker.show()
         << '\n';

    auto const bcOffset = newMarker.bcOff();
    // TODO(T46690139)
    func->unit()->prettyPrint(
      mStr, Unit::PrintOpts()
      .range(bcOffset, bcOffset+1)
      .noLineNumbers()
      .noFuncs()
      .indent(0));
    std::vector<std::string> vec;
    folly::split('\n', mStr.str(), vec);
    for (auto const& s : vec) {
      if (s.empty()) continue;
      funcStr << s << '\n';
    }
    markerObj["raw"] = funcStr.str();
  }
  result["marker"] = markerObj;

  dynamic phiPseudoInstrs = dynamic::array;
  if (inst.op() == DefLabel) {
    // print phi pseudo-instructions
    for (unsigned i = 0, n = inst.numDsts(); i < n; ++i) {
      dynamic phiPseudoInstr = dynamic::object;
      phiPseudoInstr["dst"] = getSSATmp(inst.dst(i));

      dynamic srcs = dynamic::array;
      inst.block()->forEachSrc(i, [&](IRInstruction* jmp, SSATmp*) {
          srcs.push_back(dynamic::object("src", getSSATmp(jmp->src(i)))
                                        ("label", getLabel(jmp->block())));
        });
      phiPseudoInstr["srcs"] = srcs;

      phiPseudoInstrs.push_back(phiPseudoInstr);
    }
  }
  result["phiPseudoInstrs"] = phiPseudoInstrs;

  const dynamic id = inst.isTransient() ? dynamic(nullptr) : inst.id();

  const Block* taken = inst.taken();
  const dynamic takenObj = taken ? getLabel(taken) : dynamic(nullptr);

  auto const offset = inst.marker().bcOff() + inst.iroff();
  result.update(dynamic::merge(getOpcode(&inst, guards),
                               dynamic::object("id", id)
                                              ("taken", takenObj)
                                              ("srcs", getSrcs(&inst))
                                              ("dsts", getDsts(&inst))
                                              ("offset", offset)));
  return result;
}

dynamic getTCRange(const AreaIndex area,
                   const TransKind kind,
                   const TcaRange& range) {
  std::ostringstream disasmStr;
  disasmRange(disasmStr, kind, range);
  auto const startStr = folly::sformat("{}", static_cast<void*>(range.begin()));
  auto const endStr = folly::sformat("{}", static_cast<void*>(range.end()));
  return dynamic::object("area", areaAsString(area))
                        ("start", startStr)
                        ("end", endStr)
                        ("disasm", disasmStr.str());
}

namespace {
// TODO(T52856776) - move into AnnotationData class
struct TargetProfileVisitor {
  explicit TargetProfileVisitor(const TransContext& ctx) : ctx(ctx) {}

  template<typename T>
  dynamic getProfileDynamic(const rds::Profile<T>& prof,
                            const dynamic& linkObj) const {
    return dynamic::object("offset", prof.bcOff)
                          ("name", folly::sformat("{}", prof.name))
                          ("data", linkObj);
  }

  dynamic operator() (const rds::Profile<jit::SwitchProfile>& prof) const {
    auto const link = rds::bind<jit::SwitchProfile, rds::Mode::Local>(prof);
    auto const func = ctx.initSrcKey.func();
    assert(func->contains(prof.bcOff));
    auto const funcBCOffset = func->unit()->at(func->base() + prof.bcOff);
    auto const bcSize = getImmVector(funcBCOffset).size();
    return getProfileDynamic(prof, link.get()->toDynamic(bcSize));
  }

  template<typename T>
  dynamic operator() (const rds::Profile<T>& prof) const {
    auto const link = rds::bind<T, rds::Mode::Local>(prof);
    return getProfileDynamic(prof, link.get()->toDynamic());;
  }

  template<typename T>
  dynamic operator() (T&&) const {
    assertx(false);
    return dynamic();
  }

  const TransContext& ctx;
};
}

dynamic getBlock(const Block* block,
                 const TransKind kind,
                 const AsmInfo* asmInfo,
                 const GuardConstraints* guards,
                 const IRUnit& unit) {
  dynamic result = dynamic::object;

  result["label"] = getLabel(block);
  result["profCount"] = block->profCount();

  dynamic predIds = dynamic::array;

  auto const& preds = block->preds();
  if (!preds.empty()) {
    for (auto const& edge : preds) {
      predIds.push_back(edge.from()->id());
    }
  }
  result["preds"] = predIds;
  result["next"] = block->next() ? getLabel(block->next()) : dynamic(nullptr);
  result["instrs"] = dynamic::array;

  if (block->empty()) return result;

  if (asmInfo) {
    std::vector<const IRInstruction*> instrs;
    std::vector<InstAreaRange> instRanges;
    std::array<TcaRange, kNumAreas> lastRange;
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
        auto const& areaRanges = asmInfo->instRangesForArea(instArea);
        auto const& rngs = areaRanges[inst];
        for (auto itr = rngs.first; itr != rngs.second; ++itr) {
          auto const range = TcaRange(itr->second.start() + areaRanges.offset,
                                      itr->second.end() + areaRanges.offset);
          instRanges.push_back(InstAreaRange(instIdx, instArea, range));
          lastRange[(int)instArea] = range;
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

    std::vector<InstAreaRange> collatedInstRanges;
    for (auto const& inst : instRanges) {
      if (collatedInstRanges.empty()) {
        collatedInstRanges.push_back(inst);
        continue;
      }

      auto& prevRange = collatedInstRanges.back();
      if (prevRange.isContiguous(inst)) {
        prevRange.m_instRange = TcaRange(prevRange.m_instRange.begin(),
                                         inst.m_instRange.end());
      } else {
        collatedInstRanges.push_back(inst);
      }
    }
    instRanges = collatedInstRanges;

    const IRInstruction* lastInst = nullptr;
    AreaIndex lastArea = AreaIndex::Main;
    bool printArea = false;

    dynamic currInstrObj = (dynamic) nullptr;

    for (auto itr = instRanges.begin(); itr != instRanges.end(); ++itr) {
      auto const currInstIdx = itr->m_instIdx;
      auto const currInst = instrs[currInstIdx];
      auto const currArea = itr->m_area;
      auto const instRange = itr->m_instRange;
      if (lastInst != currInst) {
        if (!currInstrObj.isNull()) {
          result["instrs"].push_back(currInstrObj);
        }
        currInstrObj = dynamic::merge(getIRInstruction(*currInst, guards),
                                      dynamic::object("tc_ranges",
                                                      dynamic::array));
        printArea = true;
        lastInst = currInst;
        lastRange[(int)currArea] = TcaRange(nullptr,nullptr);
      }
      if (printArea || currArea != lastArea) {
        lastArea = currArea;
        printArea = false;
        lastRange[(int)currArea] = TcaRange(nullptr,nullptr);
      }

      const auto lastEnd = lastRange[(int)currArea].end();

      if (lastEnd && lastEnd != instRange.begin()) {
        // There may be gaps between instruction ranges that have been
        // added by the relocator, e.g. adding nops.  This check will
        // determine if the gap belongs to another instruction or not.
        // If it doesn't belong to any other instruction then print it.
        auto const offset = asmInfo->instRangesForArea(currArea).offset;
        auto const gapRange = TcaRange(lastEnd - offset,
                                       instRange.begin() - offset);
        if (!asmInfo->instRangeExists(currArea, gapRange)) {
          currInstrObj["tc_ranges"].push_back(getTCRange(currArea,
                                                         kind,
                                                         gapRange));
        }
      }
      currInstrObj["tc_ranges"].push_back(getTCRange(currArea,
                                                     kind,
                                                     instRange));
      lastRange[(int)currArea] = instRange;
    }
    if (!currInstrObj.isNull()) {
      result["instrs"].push_back(currInstrObj);
    }
  } else {
    for (auto it = block->begin(); it != block->end(); ++it) {
      result["instrs"].push_back(dynamic::merge(getIRInstruction(*it, guards),
                                                dynamic::object("tc_ranges",
                                                                dynamic())));
    }
  }

  auto const& ctx = unit.context();

  for (auto& inst : result["instrs"]) {
    auto const& offset = inst["offset"].asInt();
    dynamic profileObjs = dynamic::array;
    for (auto const& key : unit.annotationData->profileKeys) {
      auto const profile = boost::apply_visitor(TargetProfileVisitor(ctx), key);
      if (profile["offset"].asInt() == offset) profileObjs.push_back(profile);
    }
    inst["profileData"] = profileObjs;
  }

  return result;
}

dynamic getSrcKey(const SrcKey& sk) {
  return dynamic::object("func", sk.func()->name()->slice())
                        ("unit", sk.unit()->filepath()->slice())
                        ("prologue", sk.prologue())
                        ("offset", sk.offset())
                        ("resumeMode", resumeModeShortName(sk.resumeMode()))
                        ("hasThis", sk.hasThis());
}

dynamic getTransContext(const TransContext& ctx) {
  auto const func = ctx.initSrcKey.func();
  return dynamic::object("kind", show(ctx.kind))
                        ("id", ctx.transID)
                        ("optIndex", ctx.optIndex)
                        ("srcKey", getSrcKey(ctx.initSrcKey))
                        ("funcName", func->fullDisplayName()->data())
                        ("sourceFile", func->filename()->data())
                        ("startLine", func->line1())
                        ("endLine", func->line2());
}

dynamic getUnit(const IRUnit& unit,
                const AsmInfo* asmInfo,
                const GuardConstraints* guards) {
  dynamic result = dynamic::object;

  auto const& ctx = unit.context();
  auto const kind = ctx.kind;
  result["translation"] = getTransContext(ctx);

  result["inliningDecisions"] = unit.annotationData ?
    unit.annotationData->getInliningDynamic() :
    dynamic::array();

  auto blocks = rpoSortCfg(unit);
  // Partition into main, cold and frozen, without changing relative order.
  auto const cold = std::stable_partition(blocks.begin(), blocks.end(),
    [&] (Block* b) {
      return b->hint() == Block::Hint::Neither ||
             b->hint() == Block::Hint::Likely;
    }
  );
  auto const frozen = std::stable_partition(cold, blocks.end(),
    [&] (Block* b) { return b->hint() == Block::Hint::Unlikely; }
  );

  dynamic blockObjs = dynamic::array;

  AreaIndex currArea = AreaIndex::Main;
  for (auto it = blocks.begin(); it != blocks.end(); ++it) {
    if (it == cold) {
      currArea = AreaIndex::Cold;
    }
    if (it == frozen) {
      currArea = AreaIndex::Frozen;
    }

    blockObjs.push_back(dynamic::merge(
                          getBlock(*it, kind, asmInfo, guards, unit),
                          dynamic::object("area", areaAsString(currArea))));
  }
  result["blocks"] = blockObjs;
  return result;
}

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
    os << " " << Stats::g_counterNames[inst->src(0)->intVal()];
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

void disasmRange(std::ostream& os,
                 TransKind kind,
                 TCA begin,
                 TCA end,
                 bool useColor) {
  assertx(begin <= end);
  int const indent = kIndent + 4;
  bool const printEncoding = dumpIREnabled(kind, kAsmEncodingLevel);
  char const* colorStr = useColor ? color(ANSI_COLOR_BROWN) : "";

  switch (arch()) {
    case Arch::X64: {
      Disasm disasm(Disasm::Options().indent(indent)
                    .printEncoding(printEncoding)
                    .color(colorStr));
      disasm.disasm(os, begin, end);
      return;
    }

    case Arch::ARM: {
      vixl::Decoder dec;
      vixl::PrintDisassembler disasm(os, indent, printEncoding, colorStr);
      disasm.setShouldDereferencePCRelativeLiterals(true);
      dec.AppendVisitor(&disasm);
      for (; begin < end; begin += vixl::kInstructionSize) {
        dec.Decode(vixl::Instruction::Cast(begin));
      }
      return;
    }

    case Arch::PPC64: {
      ppc64_asm::Disassembler disasm(printEncoding, true, indent, colorStr);
      for (; begin < end; begin += ppc64_asm::instr_size_in_bytes) {
        disasm.disassembly(os, begin);
      }
      return;
    }
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
        func->prettyPrint(mStr, Func::PrintOpts());
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

void print(std::ostream& os, const Block* block, TransKind kind,
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
        auto const& areaRanges = asmInfo->instRangesForArea(instArea);
        auto const& rngs = areaRanges[inst];
        for (auto itr = rngs.first; itr != rngs.second; ++itr) {
          auto range = TcaRange { itr->second.start() + areaRanges.offset,
                                  itr->second.end() + areaRanges.offset };
          instRanges.push_back(InstAreaRange(instIdx, instArea, range));
          lastRange[(int)instArea] = range;
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
        auto const offset = asmInfo->instRangesForArea(currArea).offset;
        if (!asmInfo->instRangeExists(currArea,
                                      TcaRange(lastEnd - offset,
                                               instRange.begin() - offset))) {
          disasmRange(os, kind, lastEnd, instRange.begin(), true);
        } else {
          os << "\n";
        }
      }
      disasmRange(os, kind, instRange.begin(), instRange.end(), true);
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
  print(std::cerr, block, TransKind::Optimize);
  std::cerr << std::endl;
}

std::string Block::toString() const {
  std::ostringstream out;
  print(out, this, TransKind::Optimize);
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
  auto const kind = unit.context().kind;
  os << "TransKind: " << show(kind) << "\n";
  if (unit.context().kind == TransKind::Optimize) {
    os << "OptIndex : " << unit.context().optIndex << "\n";
  }
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

  if (dumpIREnabled(kind, kExtraExtraLevel)) printOpcodeStats(os, blocks);

  // Print the block CFG above the actual code.

  auto const retreating_edges = findRetreatingEdges(unit);
  os << "digraph G {\n";
  for (auto block : blocks) {
    if (block->empty()) continue;
    if (dotBodies) {
      if (block->hint() != Block::Hint::Unlikely &&
          block->hint() != Block::Hint::Unused) {
        // Include the IR in the body of the node
        std::ostringstream out;
        print(out, block, kind, asmInfo, guards, &curMarker);
        auto bodyRaw = out.str();
        std::string body;
        body.reserve(bodyRaw.size() * 1.25);
        for (auto c : bodyRaw) {
          if (c == '\n')      body += "\\n";
          else if (c == '"')  body += "\\\"";
          else if (c == '\\') body += "\\\\";
          else                body += c;
        }
        os << folly::format("B{} [shape=box,label=\"{}\"]\n",
                            block->id(), body);
      }
    } else {
      const auto color = [&] {
        switch (block->hint()) {
          case Block::Hint::Likely :  return "red";
          case Block::Hint::Neither:  return "orange";
          case Block::Hint::Unlikely: return "blue";
          case Block::Hint::Unused:   return "gray";
        }
        not_reached();
      }();
      os << folly::format(
        "B{} [shape=box,color={},label=\"B{}\\ncount={}\"]\n",
        block->id(), color, block->id(), block->profCount()
      );
    }

    auto next = block->nextEdge();
    auto taken = block->takenEdge();
    if (!next && !taken) continue;
    auto edge_color = [&] (Edge* edge) {
      auto const target = edge->to();
      return
        target->isCatch() ? " [color=gray]" :
        target->hint() == Block::Hint::Unlikely ? " [color=blue]" :
        retreating_edges.count(edge) ? " [color=red]" : "";
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

  curMarker = BCMarker();
  for (auto it = blocks.begin(); it != blocks.end(); ++it) {
    if (it == cold) {
      os << folly::format("\n{:-^60}", "cold blocks");
    }
    if (it == frozen) {
      os << folly::format("\n{:-^60}", "frozen blocks");
    }
    print(os, *it, kind, asmInfo, guards, &curMarker);
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
void printUnit(int level, const IRUnit& unit, const char* caption,
               AsmInfo* ai,
               const GuardConstraints* guards, Annotations* annotations) {
  if (dumpIREnabled(unit.context().kind, level)) {
    std::ostringstream str;
    if (dumpPrettyIR(level)) {
      str << banner(caption);
      print(str, unit, ai, guards);
      str << banner("");
      if (HPHP::Trace::moduleEnabledRelease(HPHP::Trace::printir, level)) {
        HPHP::Trace::traceRelease("%s\n", str.str().c_str());
      }
    } else if (dumpJsonIR(level)) {
      str << "json:" << get_json::getUnit(unit, ai, guards);
      if (HPHP::Trace::moduleEnabledRelease(HPHP::Trace::printir_json, level)) {
        HPHP::Trace::traceRelease("%s\n", str.str().c_str());
      }
    }
    if (annotations && dumpRuntimeIR(level)) {
      annotations->emplace_back(caption, str.str());
    }
  }
}

bool dumpIREnabled(TransKind kind, int level /* = 1 */) {
  return HPHP::Trace::moduleEnabledRelease(HPHP::Trace::printir, level) ||
         HPHP::Trace::moduleEnabledRelease(HPHP::Trace::printir_json, level) ||
         (dumpRuntimeIR(level) &&
          mcgen::dumpTCAnnotation(kind));
}

///////////////////////////////////////////////////////////////////////////////

}}
