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

#include "hphp/util/disasm.h"
#include "hphp/util/text-color.h"
#include "hphp/util/abi-cxx.h"
#include "hphp/vixl/a64/disasm-a64.h"
#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/layout.h"
#include "hphp/runtime/vm/jit/code-gen-x64.h"
#include "hphp/runtime/vm/jit/block.h"

namespace HPHP {  namespace JIT {

//////////////////////////////////////////////////////////////////////
namespace {

// Helper for pretty-printing punctuation.
static std::string punc(const char* str) {
  return folly::format("{}{}{}",
    color(ANSI_COLOR_DARK_GRAY), str, color(ANSI_COLOR_END)).str();
}

static std::string constToString(Type t, const ConstData* c) {
  std::ostringstream os;
  os << color(ANSI_COLOR_LIGHT_BLUE);
  SCOPE_EXIT { os << color(ANSI_COLOR_END); };

  if (t == Type::Int) {
    os << c->as<int64_t>();
  } else if (t == Type::Dbl) {
    // don't format doubles as integers.
    auto d = c->as<double>();
    auto s = folly::format("{}", d).str();
    if (!strchr(s.c_str(), '.') && !strchr(s.c_str(), 'e')) {
      os << folly::format("{:.1f}", d);
    } else {
      os << s;
    }
  } else if (t == Type::Bool) {
    os << (c->as<bool>() ? "true" : "false");
  } else if (t.isString()) {
    auto str = c->as<const StringData*>();
    os << "\""
       << Util::escapeStringForCPP(str->data(), str->size())
       << "\"";
  } else if (t.isArray()) {
    auto arr = c->as<const ArrayData*>();
    if (arr->empty()) {
      os << "array()";
    } else {
      os << "Array(" << arr << ")";
    }
  } else if (t.isNull() || t <= Type::Nullptr) {
    os << t.toString();
  } else if (t <= Type::Func) {
    auto func = c->as<const Func*>();
    os << "Func(" << (func ? func->fullName()->data() : "0") << ")";
  } else if (t <= Type::Cls) {
    auto cls = c->as<const Class*>();
    os << "Cls(" << (cls ? cls->name()->data() : "0") << ")";
  } else if (t <= Type::Cctx) {
    auto cls = reinterpret_cast<const Class*>(c->as<uintptr_t>() - 1);
    os << "Cctx(" << (cls ? cls->name()->data() : "0") << ")";
  } else if (t <= Type::NamedEntity) {
    auto ne = c->as<const NamedEntity*>();
    os << "NamedEntity(" << ne << ")";
  } else if (t <= Type::TCA) {
    TCA tca = c->as<TCA>();
    auto name = getNativeFunctionName(tca);
    const char* hphp = "HPHP::";

    if (!name.compare(0, strlen(hphp), hphp)) {
      name = name.substr(strlen(hphp));
    }
    auto pos = name.find_first_of('(');
    if (pos != std::string::npos) {
      name = name.substr(0, pos);
    }
    os << folly::format("TCA: {}({})", tca, boost::trim_copy(name));
  } else if (t <=Type::None) {
    os << "None:" << c->as<int64_t>();
  } else if (t.isPtr()) {
    os << folly::format("{}({:#x})", t.toString(), c->as<uint64_t>());
  } else if (t <= Type::RDSHandle) {
    os << folly::format("RDS::Handle({:#x})", c->as<int64_t>());
  } else {
    not_reached();
  }
  return os.str();
}

const PhysLoc* srcLoc(const RegAllocInfo* regs,
                      const IRInstruction* inst, unsigned i) {
  return regs ? &(*regs)[inst].src(i) : nullptr;
}

const PhysLoc* dstLoc(const RegAllocInfo* regs,
                      const IRInstruction* inst, unsigned i) {
  return regs ? &(*regs)[inst].dst(i) : nullptr;
}

void printSrc(std::ostream& ostream, const IRInstruction* inst, uint32_t i,
              const RegAllocInfo* regs) {
  SSATmp* src = inst->src(i);
  if (src != nullptr) {
    print(ostream, src, srcLoc(regs, inst, i));
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
                 const GuardConstraints* guards) {
  os << color(ANSI_COLOR_CYAN)
     << opcodeName(inst->op())
     << color(ANSI_COLOR_END)
     ;

  auto const hasTypeParam = inst->hasTypeParam();
  auto const hasExtra = inst->hasExtra();
  auto const isGuard = guards && !inst->isTransient() && isGuardOp(inst->op());

  if (!hasTypeParam && !hasExtra && !isGuard) return;
  os << color(ANSI_COLOR_LIGHT_BLUE) << '<' << color(ANSI_COLOR_END);

  if (hasTypeParam) {
    os << color(ANSI_COLOR_GREEN)
       << inst->typeParam().toString()
       << color(ANSI_COLOR_END)
       ;
    if (hasExtra || isGuard) os << punc(",");
  }

  if (inst->op() == LdConst) {
    os << constToString(inst->typeParam(), inst->extra<LdConst>());
  } else {
    if (hasExtra) {
      os << color(ANSI_COLOR_GREEN)
         << showExtra(inst->op(), inst->rawExtra())
         << color(ANSI_COLOR_END);
      if (isGuard) os << punc(",");
    }
  }

  if (isGuard) {
    auto it = guards->find(inst);
    os << (it == guards->end() ? "unused" : it->second.toString());
  }

  os << color(ANSI_COLOR_LIGHT_BLUE)
     << '>'
     << color(ANSI_COLOR_END);
}

void printSrcs(std::ostream& os, const IRInstruction* inst,
               const RegAllocInfo* regs) {
  bool first = true;
  if (inst->op() == IncStat) {
    os << " " << Stats::g_counterNames[inst->src(0)->getValInt()]
       << ", " << inst->src(1)->getValInt();
    return;
  }
  for (uint32_t i = 0, n = inst->numSrcs(); i < n; i++) {
    if (!first) {
      os << punc(", ");
    } else {
      os << " ";
      first = false;
    }
    printSrc(os, inst, i, regs);
  }
}

void printDsts(std::ostream& os, const IRInstruction* inst,
               const RegAllocInfo* regs) {
  const char* sep = "";
  for (unsigned i = 0, n = inst->numDsts(); i < n; i++) {
    os << punc(sep);
    print(os, inst->dst(i), dstLoc(regs, inst, i));
    sep = ", ";
  }
}

void printInstr(std::ostream& ostream, const IRInstruction* inst,
                const RegAllocInfo* regs, const GuardConstraints* guards) {
  printDsts(ostream, inst, regs);
  if (inst->numDsts()) ostream << punc(" = ");
  printOpcode(ostream, inst, guards);
  printSrcs(ostream, inst, regs);
}

void print(std::ostream& ostream, const IRInstruction* inst,
           const RegAllocInfo* regs, const GuardConstraints* guards) {
  if (!inst->isTransient()) {
    ostream << color(ANSI_COLOR_YELLOW);
    ostream << folly::format("({:02d}) ", inst->id());
    ostream << color(ANSI_COLOR_END);
  }
  printInstr(ostream, inst, regs);
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

/*
 * SSATmp
 */
std::ostream& operator<<(std::ostream& os, const PhysLoc& loc) {
  auto sz = loc.numAllocated();
  if (!sz) return os;
  os << '(';
  auto delim = "";
  for (int i = 0; i < sz; ++i) {
    if (!loc.spilled()) {
      PhysReg reg = loc.reg(i);
      if (arch() == Arch::X64) {
        auto name = reg.type() == PhysReg::GP ? reg::regname(Reg64(reg)) :
          reg::regname(RegXMM(reg));
        os << delim << name;
      } else if (arch() == Arch::ARM) {
        auto prefix =
          reg.isGP() ? (vixl::Register(reg).size() == vixl::kXRegSize
                        ? 'x' : 'w')
          : (vixl::FPRegister(reg).size() == vixl::kSRegSize
             ? 's' : 'd');
        os << delim << prefix << int(RegNumber(reg));
      } else {
        not_implemented();
      }
    } else {
      os << delim << "spill[" << loc.slot(i) << "]";
    }
    delim = ",";
  }
  os << ')';
  return os;
}

void printPhysLoc(std::ostream& os, const PhysLoc& loc) {
  if (loc.numAllocated() > 0) {
    os << color(ANSI_COLOR_BROWN) << loc << color(ANSI_COLOR_END);
  }
}

std::string ShuffleData::show() const {
  std::ostringstream os;
  auto delim = "";
  for (unsigned i = 0; i < size; ++i) {
    os << delim;
    printPhysLoc(os, dests[i]);
    delim = ",";
  }
  return os.str();
}

void print(std::ostream& os, const SSATmp* tmp, const PhysLoc* loc) {
  if (tmp->inst()->op() == DefConst) {
    os << constToString(tmp->inst()->typeParam(),
                        tmp->inst()->extra<DefConst>());
    return;
  }
  os << color(ANSI_COLOR_WHITE);
  os << "t" << tmp->id();
  os << color(ANSI_COLOR_END);
  if (loc) {
    printPhysLoc(os, *loc);
  }
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

static void disasmRange(std::ostream& os, TCA begin, TCA end) {
  if (arch() == Arch::X64) {
    Disasm disasm(Disasm::Options().indent(kIndent + 4)
                  .printEncoding(dumpIREnabled(kExtraLevel))
                  .color(color(ANSI_COLOR_BROWN)));
    disasm.disasm(os, begin, end);
  } else if (arch() == Arch::ARM) {
    using namespace vixl;
    Decoder dec;
    PrintDisassembler disasm(os, kIndent + 4, dumpIREnabled(kExtraLevel),
                             color(ANSI_COLOR_BROWN));
    dec.AppendVisitor(&disasm);
    assert(begin <= end);
    for (; begin < end; begin += kInstructionSize) {
      dec.Decode(Instruction::Cast(begin));
    }
  }
}

void print(std::ostream& os, const Block* block,
           const RegAllocInfo* regs, const AsmInfo* asmInfo,
           const GuardConstraints* guards, BCMarker* markerPtr) {
  BCMarker dummy;
  BCMarker& curMarker = markerPtr ? *markerPtr : dummy;

  TcaRange blockRange = asmInfo ? asmInfo->asmRanges[block] :
    TcaRange(nullptr, nullptr);

  os << '\n' << std::string(kIndent - 3, ' ');
  printLabel(os, block);
  os << punc(":");
  auto& preds = block->preds();
  if (!preds.empty()) {
    os << " (preds";
    for (auto const& edge : preds) {
      os << " B" << edge.inst()->block()->id();
    }
    os << ')';
  }
  os << "\n";

  const char* markerEndl = "";
  for (auto it = block->begin(); it != block->end();) {
    auto& inst = *it; ++it;

    if (inst.marker() != curMarker) {
      std::ostringstream mStr;
      auto const& newMarker = inst.marker();
      auto func = newMarker.func;
      if (!func) {
        os << color(ANSI_COLOR_BLUE)
           << std::string(kIndent, ' ')
           << "--- invalid marker"
           << color(ANSI_COLOR_END)
           << '\n';
      } else {
        if (func != curMarker.func) {
          func->prettyPrint(mStr, Func::PrintOpts().noFpi());
        }
        mStr << std::string(kIndent, ' ')
             << newMarker.show()
             << '\n';

        auto bcOffset = newMarker.bcOff;
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
        JIT::print(os, dst, dstLoc(regs, &inst, i));
        os << punc(" = ") << color(ANSI_COLOR_CYAN) << "phi "
           << color(ANSI_COLOR_END);
        bool first = true;
        inst.block()->forEachSrc(i, [&](IRInstruction* jmp, SSATmp*) {
            if (!first) os << punc(", ");
            first = false;
            printSrc(os, jmp, i, regs);
            os << punc("@");
            printLabel(os, jmp->block());
          });
        os << '\n';
      }
    }

    os << std::string(kIndent, ' ');
    JIT::print(os, &inst, regs, guards);
    os << '\n';

    if (asmInfo) {
      TcaRange instRange = asmInfo->instRanges[inst];
      if (!instRange.empty()) {
        disasmRange(os, instRange.begin(), instRange.end());
        os << '\n';
        assert(instRange.end() >= blockRange.start() &&
               instRange.end() <= blockRange.end());
        blockRange = TcaRange(instRange.end(), blockRange.end());
      }
    }
  }

  if (asmInfo) {
    // print code associated with this block that isn't tied to any
    // instruction.  This includes code after the last isntruction (e.g.
    // jmp to next block), and AStubs code.
    if (!blockRange.empty()) {
      os << std::string(kIndent, ' ') << punc("A:") << "\n";
      disasmRange(os, blockRange.start(), blockRange.end());
    }
    auto astubRange = asmInfo->astubRanges[block];
    if (!astubRange.empty()) {
      os << std::string(kIndent, ' ') << punc("AStubs:") << "\n";
      disasmRange(os, astubRange.start(), astubRange.end());
    }
    if (!blockRange.empty() || !astubRange.empty()) {
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
  print(std::cerr, block);
  std::cerr << std::endl;
}

std::string Block::toString() const {
  std::ostringstream out;
  print(out, this);
  return out.str();
}

//////////////////////////////////////////////////////////////////////

/*
 * Unit
 */
void print(std::ostream& os, const IRUnit& unit,
           const RegAllocInfo* regs, const AsmInfo* asmInfo,
           const GuardConstraints* guards) {
  // Print the block CFG above the actual code.
  os << "digraph G {\n";
  for (Block* block : layoutBlocks(unit).blocks) {
    auto* next = block->next();
    auto* taken = block->taken();
    if (!next && !taken) continue;
    if (next) {
      os << folly::format("B{} -> B{}", block->id(), next->id());
      if (taken) os << "; ";
    }
    if (taken) os << folly::format("B{} -> B{}", block->id(), taken->id());
    os << "\n";
  }
  os << "}\n";
  // For nice-looking dumps, we want to remember curMarker between blocks.
  BCMarker curMarker;
  for (Block* block : layoutBlocks(unit).blocks) {
    print(os, block, regs, asmInfo, guards, &curMarker);
  }
}

void print(const IRUnit& unit) {
  print(std::cerr, unit);
  std::cerr << std::endl;
}

std::string IRUnit::toString() const {
  std::ostringstream out;
  print(out, *this);
  return out.str();
}

// Suggested captions: "before jiffy removal", "after goat saturation",
// etc.
void dumpTrace(int level, const IRUnit& unit, const char* caption,
               const RegAllocInfo* regs, AsmInfo* ai,
               const GuardConstraints* guards) {
  if (dumpIREnabled(level)) {
    std::ostringstream str;
    auto bannerFmt = "{:-^80}\n";
    str << color(ANSI_COLOR_BLACK, ANSI_BGCOLOR_GREEN)
        << folly::format(bannerFmt, caption)
        << color(ANSI_COLOR_END)
        ;
    print(str, unit, regs, ai, guards);
    str << color(ANSI_COLOR_BLACK, ANSI_BGCOLOR_GREEN)
        << folly::format(bannerFmt, "")
        << color(ANSI_COLOR_END)
        ;
    HPHP::Trace::traceRelease("%s\n", str.str().c_str());
  }
}

}}
