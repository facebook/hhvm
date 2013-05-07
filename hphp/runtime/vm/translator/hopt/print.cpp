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

#include "runtime/vm/translator/hopt/print.h"

#include "runtime/vm/translator/hopt/ir.h"
#include "runtime/vm/translator/hopt/linearscan.h"
#include "runtime/vm/translator/hopt/codegen.h"
#include "runtime/base/stats.h"
#include "util/disasm.h"
#include "util/text_color.h"

namespace HPHP { namespace VM { namespace JIT {

//////////////////////////////////////////////////////////////////////

// Helper for pretty-printing punctuation.
static std::string punc(const char* str) {
  return folly::format("{}{}{}",
    color(ANSI_COLOR_DARK_GRAY), str, color(ANSI_COLOR_END)).str();
}

//////////////////////////////////////////////////////////////////////

void printOpcode(std::ostream& os, const IRInstruction* inst) {
  os << color(ANSI_COLOR_CYAN)
     << opcodeName(inst->op())
     << color(ANSI_COLOR_END)
     ;

  auto type_param = inst->getTypeParam();
  if (type_param == Type::None && !inst->hasExtra()) {
    return;
  }
  os << color(ANSI_COLOR_LIGHT_BLUE) << '<' << color(ANSI_COLOR_END);
  if (type_param != Type::None) {
    os << color(ANSI_COLOR_GREEN)
       << type_param.toString()
       << color(ANSI_COLOR_END)
       ;
    if (inst->hasExtra()) {
      os << punc(",");
    }
  }
  if (inst->hasExtra()) {
    os << color(ANSI_COLOR_GREEN)
       << showExtra(inst->op(), inst->rawExtra())
       << color(ANSI_COLOR_END);
  }
  os << color(ANSI_COLOR_LIGHT_BLUE)
     << '>'
     << color(ANSI_COLOR_END);
}

void printDst(std::ostream& os, const IRInstruction* inst) {
  if (inst->getNumDsts() == 0) return;

  const char* sep = "";
  for (const SSATmp& dst : inst->getDsts()) {
    os << punc(sep);
    print(os, &dst, true);
    sep = ", ";
  }
  os << punc(" = ");
}

void printSrc(std::ostream& ostream, const IRInstruction* inst, uint32_t i) {
  SSATmp* src = inst->getSrc(i);
  if (src != nullptr) {
    if (inst->getId() != 0 && !src->isConst() &&
        src->getLastUseId() == inst->getId()) {
      ostream << "~";
    }
    print(ostream, src);
  } else {
    ostream << color(ANSI_COLOR_RED)
            << "!!!NULL @ " << i
            << color(ANSI_COLOR_END)
            ;
  }
}

void printSrcs(std::ostream& os, const IRInstruction* inst) {
  bool first = true;
  if (inst->op() == IncStat) {
    os << " " << Stats::g_counterNames[inst->getSrc(0)->getValInt()]
       << ", " << inst->getSrc(1)->getValInt();
    return;
  }
  for (uint32_t i = 0, n = inst->getNumSrcs(); i < n; i++) {
    if (!first) {
      os << punc(", ");
    } else {
      os << " ";
      first = false;
    }
    printSrc(os, inst, i);
  }
}

void printLabel(std::ostream& os, const Block* block) {
  os << color(ANSI_COLOR_MAGENTA);
  os << "L" << block->getId();
  if (block->getHint() == Block::Unlikely) os << "<Unlikely>";
  os << color(ANSI_COLOR_END);
}

void print(std::ostream& ostream, const IRInstruction* inst) {
  if (inst->op() == Marker) {
    auto* marker = inst->getExtra<Marker>();
    ostream << color(ANSI_COLOR_BLUE)
            << folly::format("--- bc {}, spOff {} ({})",
                             marker->bcOff,
                             marker->stackOff,
                             marker->func->fullName()->data())
            << color(ANSI_COLOR_END);
    return;
  }

  if (!inst->isTransient()) {
    ostream << color(ANSI_COLOR_YELLOW);
    if (!inst->getId()) {
      ostream << folly::format("({:02d}) ", inst->getIId());
    } else {
      ostream << folly::format("({:02d}@{:02d}) ", inst->getIId(),
                               inst->getId());
    }
    ostream << color(ANSI_COLOR_END);
  }
  printDst(ostream, inst);
  printOpcode(ostream, inst);
  printSrcs(ostream, inst);

  if (Block* taken = inst->getTaken()) {
    ostream << punc(" -> ");
    printLabel(ostream, taken);
  }

  if (TCA tca = inst->getTCA()) {
    ostream << punc(", ");
    if (tca == kIRDirectJccJmpActive) {
      ostream << "JccJmp_Exit ";
    }
    else if (tca == kIRDirectJccActive) {
      ostream << "Jcc_Exit ";
    }
    else if (tca == kIRDirectGuardActive) {
      ostream << "Guard_Exit ";
    }
    else {
      ostream << (void*)tca;
    }
  }
}

void print(const IRInstruction* inst) {
  print(std::cerr, inst);
  std::cerr << std::endl;
}

static void printConst(std::ostream& os, IRInstruction* inst) {
  os << color(ANSI_COLOR_LIGHT_BLUE);
  SCOPE_EXIT { os << color(ANSI_COLOR_END); };

  auto t = inst->getTypeParam();
  auto c = inst->getExtra<DefConst>();
  if (t == Type::Int) {
    os << c->as<int64_t>();
  } else if (t == Type::Dbl) {
    os << c->as<double>();
  } else if (t == Type::Bool) {
    os << (c->as<bool>() ? "true" : "false");
  } else if (t.isString()) {
    auto str = c->as<const StringData*>();
    os << "\""
       << Util::escapeStringForCPP(str->data(), str->size())
       << "\"";
  } else if (t.isArray()) {
    auto arr = inst->getExtra<DefConst>()->as<const ArrayData*>();
    if (arr->empty()) {
      os << "array()";
    } else {
      os << "Array(" << arr << ")";
    }
  } else if (t.isNull()) {
    os << t.toString();
  } else if (t.subtypeOf(Type::Func)) {
    auto func = c->as<const Func*>();
    os << "Func(" << (func ? func->fullName()->data() : "0") << ")";
  } else if (t.subtypeOf(Type::Cls)) {
    auto cls = c->as<const Class*>();
    os << "Cls(" << (cls ? cls->name()->data() : "0") << ")";
  } else if (t.subtypeOf(Type::NamedEntity)) {
    auto ne = c->as<const NamedEntity*>();
    os << "NamedEntity(" << ne << ")";
  } else if (t.subtypeOf(Type::TCA)) {
    TCA tca = c->as<TCA>();
    auto name = Util::getNativeFunctionName(tca);
    SCOPE_EXIT { free(name); };
    os << folly::format("TCA: {}({})", tca,
      boost::trim_copy(std::string(name)));
  } else if (t.subtypeOf(Type::None)) {
    os << "None:" << c->as<int64_t>();
  } else if (t.isPtr()) {
    os << folly::format("{}({:#x})", t.toString(), c->as<uint64_t>());
  } else if (t.subtypeOf(Type::CacheHandle)) {
    os << folly::format("CacheHandle({:#x})", c->as<int64_t>());
  } else {
    not_reached();
  }
}

void print(std::ostream& os, const SSATmp* tmp, bool printLastUse) {
  if (tmp->inst()->op() == DefConst) {
    printConst(os, tmp->inst());
    return;
  }
  os << color(ANSI_COLOR_WHITE);
  os << "t" << tmp->getId();
  os << color(ANSI_COLOR_END);
  if (printLastUse && tmp->getLastUseId() != 0) {
    os << color(ANSI_COLOR_GRAY)
       << "@" << tmp->getLastUseId() << "#" << tmp->getUseCount()
       << color(ANSI_COLOR_END);
  }
  if (tmp->isSpilled() || tmp->numAllocatedRegs() > 0) {
    os << color(ANSI_COLOR_BROWN) << '(';
    if (!tmp->isSpilled()) {
      for (int i = 0, sz = tmp->numAllocatedRegs(); i < sz; ++i) {
        if (i != 0) os << ",";
        os << reg::regname(Reg64(tmp->getReg(i)));
      }
    } else {
      for (int i = 0, sz = tmp->numNeededRegs(); i < sz; ++i) {
        if (i != 0) os << ",";
        os << tmp->getSpillInfo(i);
      }
    }
    os << ')' << color(ANSI_COLOR_END);
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

void print(const Trace* trace) {
  print(std::cout, trace, nullptr);
}

void print(std::ostream& os, const Trace* trace, const AsmInfo* asmInfo) {
  static const int kIndent = 4;
  Disasm disasm(Disasm::Options().indent(kIndent + 4)
                                 .printEncoding(dumpIREnabled(6))
                                 .color(color(ANSI_COLOR_BROWN)));

  // Print unlikely blocks at the end
  BlockList blocks, unlikely;
  for (Block* block : trace->getBlocks()) {
    if (block->getHint() == Block::Unlikely) {
      unlikely.push_back(block);
    } else {
      blocks.push_back(block);
    }
  }
  blocks.splice(blocks.end(), unlikely);

  for (Block* block : blocks) {
    TcaRange blockRange = asmInfo ? asmInfo->asmRanges[block] :
                          TcaRange(nullptr, nullptr);
    for (auto it = block->begin(); it != block->end();) {
      auto& inst = *it; ++it;

      if (inst.op() == Marker) {
        os << std::string(kIndent, ' ');
        JIT::print(os, &inst);
        os << '\n';

        // Don't print bytecode in a non-main trace.
        if (!trace->isMain()) continue;

        auto* marker = inst.getExtra<Marker>();
        uint32_t bcOffset = marker->bcOff;
        if (const auto* func = marker->func) {
          std::ostringstream uStr;
          func->unit()->prettyPrint(
            uStr, Unit::PrintOpts()
                  .range(bcOffset, bcOffset+1)
                  .noLineNumbers()
                  .indent(0));
          std::vector<std::string> vec;
          folly::split('\n', uStr.str(), vec);
          for (auto& s : vec) {
            os << color(ANSI_COLOR_BLUE) << s << color(ANSI_COLOR_END) << '\n';
          }
          continue;
        }
      }

      if (inst.op() == DefLabel) {
        os << std::string(kIndent - 2, ' ');
        printLabel(os, inst.getBlock());
        os << punc(":") << "\n";
        // print phi pseudo-instructions
        for (unsigned i = 0, n = inst.getNumDsts(); i < n; ++i) {
          os << std::string(kIndent +
                            folly::format("({}) ", inst.getIId()).str().size(),
                            ' ');
          JIT::print(os, inst.getDst(i), false);
          os << punc(" = ") << color(ANSI_COLOR_CYAN) << "phi "
             << color(ANSI_COLOR_END);
          bool first = true;
          inst.getBlock()->forEachSrc(i, [&](IRInstruction* jmp, SSATmp*) {
            if (!first) os << punc(", ");
            first = false;
            printSrc(os, jmp, i);
            os << punc("@");
            printLabel(os, jmp->getBlock());
          });
          os << '\n';
        }
      }

      os << std::string(kIndent, ' ');
      JIT::print(os, &inst);
      os << '\n';

      if (asmInfo) {
        TcaRange instRange = asmInfo->instRanges[inst];
        if (!instRange.empty()) {
          disasm.disasm(os, instRange.begin(), instRange.end());
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
        disasm.disasm(os, blockRange.start(), blockRange.end());
      }
      auto astubRange = asmInfo->astubRanges[block];
      if (!astubRange.empty()) {
        os << std::string(kIndent, ' ') << punc("AStubs:") << "\n";
        disasm.disasm(os, astubRange.start(), astubRange.end());
      }
      if (!blockRange.empty() || !astubRange.empty()) {
        os << '\n';
      }
    }
  }

  for (auto* exitTrace : trace->getExitTraces()) {
    os << "\n" << color(ANSI_COLOR_GREEN)
       << "    -------  Exit Trace  -------"
       << color(ANSI_COLOR_END) << '\n';
    print(os, exitTrace, asmInfo);
  }
}

void dumpTraceImpl(const Trace* trace, std::ostream& out,
                   const AsmInfo* asmInfo) {
  print(out, trace, asmInfo);
}

// Suggested captions: "before jiffy removal", "after goat saturation",
// etc.
void dumpTrace(int level, const Trace* trace, const char* caption,
               AsmInfo* ai) {
  if (dumpIREnabled(level)) {
    std::ostringstream str;
    auto bannerFmt = "{:-^40}\n";
    str << color(ANSI_COLOR_BLACK, ANSI_BGCOLOR_GREEN)
        << folly::format(bannerFmt, caption)
        << color(ANSI_COLOR_END)
        ;
    dumpTraceImpl(trace, str, ai);
    str << color(ANSI_COLOR_BLACK, ANSI_BGCOLOR_GREEN)
        << folly::format(bannerFmt, "")
        << color(ANSI_COLOR_END)
        ;
    HPHP::Trace::traceRelease("%s", str.str().c_str());
  }
}

}}}

