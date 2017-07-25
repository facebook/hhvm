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

#include "hphp/php7/hhas.h"

#include "hphp/util/match.h"

#include <folly/Format.h>
#include <folly/String.h>

namespace HPHP { namespace php7 {

namespace {
  std::string dump_pseudomain(const Function& func);
  std::string dump_function(const Function& func);
  std::string dump_blocks(std::vector<Block*> blocks);

  std::string dump_declvars(const std::unordered_set<std::string>& locals);
} // namespace

std::string dump_asm(const Unit& unit) {
  std::string out;
  folly::format(&out, ".filepath \"{}\";\n\n", unit.name);
  out.append(dump_pseudomain(*unit.getPseudomain()));
  for (const auto& func : unit.functions) {
    out.append(dump_function(*func));
  }
  return out;
}

namespace {

struct InstrVisitor {
  explicit InstrVisitor(std::string& out)
    : out(out) {}

  template <class Bytecode>
  void bytecode(const Bytecode& bc) {
    out.append("  ");
    out.append(Bytecode::name());
    bc.visit_imms(*this);
    out.append("\n");
  }

  void imm(uint64_t blockid) {
    folly::format(&out, " {}", blockid);
  }

  void imm(int64_t intimm) {
    folly::format(&out, " {}", intimm);
  }

  void imm(uint32_t intimm) {
    folly::format(&out, " {}", intimm);
  }

  void imm(double n) {
    folly::format(&out, " {}", n);
  }

  void imm(const std::string& str) {
    folly::format(&out, " \"{}\"", folly::cEscape<std::string>(str));
  }

  void imm(Block* blk) {
    folly::format(&out, " L{}", blk->id);
  }

  void imm(const bc::Local& local) {
    folly::format(&out, " ${}", local.name);
  }

  void imm(IncDecOp op) {
    out.append(" ");
    switch (op) {
#define INCDEC_OP(name) case IncDecOp::name: out.append( #name ); break;
      INCDEC_OPS
#undef INCDEC_OP
    }
  }

  void imm(SetOpOp op) {
    out.append(" ");
    switch (op) {
#define SETOP_OP(name, _) case SetOpOp::name: out.append( #name ); break;
      SETOP_OPS
#undef SETOP_OP
    }
  }

  void imm(FatalOp op) {
    out.append(" ");
    switch (op) {
#define FATAL_OP(name) case FatalOp::name: out.append( #name ); break;
      FATAL_OPS
#undef FATAL_OP
    }
  }

  template <class T>
  void imm(const T& /*imm*/) {
    out.append(" <immediate>");
  }

  std::string& out;
};

// This is just a visitor for instructions and exits that will omit a jump
// (Jmp, JmpNS) iff the block that is the jump target follows immediately after
// the jump instruction
struct CFGVisitor : public boost::static_visitor<void> {
  explicit CFGVisitor(std::string& out)
    : out(out)
    , instr(out)
  {}

  void beginBlock(Block* blk) {
    // if there was an unconditional jump and its target was *not* this block
    // actually emit the instruction
    if (nextUnconditionalDestination
        && nextUnconditionalDestination != blk) {
      bytecode(bc::Jmp{nextUnconditionalDestination});
    }
    nextUnconditionalDestination = nullptr;
    folly::format(&out, "L{}:\n", blk->id);
  }

  void end() {
    if (nextUnconditionalDestination) {
      instr.bytecode(bc::Jmp{nextUnconditionalDestination});
    }
  }

  void operator()(const bc::Jmp& j) {
    nextUnconditionalDestination = j.imm1;
  }

  void operator()(const bc::JmpNS& j) {
    nextUnconditionalDestination = j.imm1;
  }

  template<class Exit>
  void operator()(const Exit& e) {
    bytecode(e);
  }

  void bytecode(const Bytecode& bc) {
    bc.visit(instr);
  }

  void exit(const Block::ExitOp& exit) {
    boost::apply_visitor(*this, exit);
  }

  std::string& out;
  InstrVisitor instr;
  Block* nextUnconditionalDestination{nullptr};
};

std::string dump_pseudomain(const Function& func) {
  std::string out;
  out.append(".main {\n");
  out.append(dump_declvars(func.locals));
  out.append(dump_blocks(serializeControlFlowGraph(func.entry)));
  out.append("}\n\n");
  return out;
}

std::string dump_function(const Function& func) {
  std::string out;
  out.append(".function ");
  out.append(func.name);
  out.append("(");
  for (const auto& param : func.params) {
    folly::format(&out, " {}${},",
        param.byRef ? "&" : "",
        param.name);
  }
  out.append(") {\n");
  out.append(dump_declvars(func.locals));
  out.append(dump_blocks(serializeControlFlowGraph(func.entry)));
  out.append("}\n\n");
  return out;

}

std::string dump_blocks(std::vector<Block*> blocks) {
  std::string out;
  CFGVisitor visitor(out);

  for (auto blk : blocks) {
    visitor.beginBlock(blk);
    for (const auto& bc : blk->code) {
      bc.visit(visitor);
    }
    for (const auto& exit : blk->exits) {
      visitor.exit(exit);
    }
  }

  visitor.end();
  return out;
}

std::string dump_declvars(const std::unordered_set<std::string>& locals) {
  std::string out;

  out.append("  .declvars");
  for (const auto& local : locals) {
    folly::format(&out, " ${}", local);
  }
  out.append(";\n");

  return out;
}

} // namespace

}} // HPHP::php7
