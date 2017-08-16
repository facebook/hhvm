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

#include "hphp/php7/analysis.h"
#include "hphp/php7/cfg.h"
#include "hphp/util/match.h"

#include <folly/Format.h>
#include <folly/String.h>

namespace HPHP { namespace php7 {

namespace {

std::string dump_pseudomain(const Function& func);
std::string dump_function(const Function& func);
std::string dump_class(const Class& cls);
std::string dump_method(const Function& func);
std::string dump_blocks(const Function& func);
std::string dump_function_body(const Function& func);

} // namespace

std::string dump_asm(const Unit& unit) {
  std::string out;
  folly::format(&out, ".filepath \"{}\";\n\n", unit.name);
  out.append(dump_pseudomain(*unit.getPseudomain()));
  for (const auto& func : unit.functions) {
    out.append(dump_function(*func));
  }
  for (const auto& cls : unit.classes) {
    out.append(dump_class(*cls));
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
    match<void>(local,
      [&](const bc::NamedLocal& named){
        folly::format(&out, " ${}", named.name);
      },
      [&](const bc::UniqueLocal& unique){
        folly::format(&out, " _{}", *unique.id);
      }
    );
  }

  void imm(const std::vector<Block*>& jmps) {
    out.append(" <");
    for (const auto& blk : jmps) {
      folly::format(&out, " L{}", blk->id);
    }
    out.append(" >");
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

  void imm(QueryMOp op) {
    out.append(" ");
    switch (op) {
#define OP(name) case QueryMOp::name: out.append( #name ); break;
      QUERY_M_OPS
#undef OP
    }
  }

  void imm(MOpMode op) {
    out.append(" ");
    switch (op) {
#define MODE(name) case MOpMode::name: out.append( #name ); break;
      M_OP_MODES
#undef MODE
    }
  }

  void imm(SwitchKind op) {
    out.append(" ");
    switch (op) {
#define KIND(name) case SwitchKind::name: out.append( #name ); break;
      SWITCH_KINDS
#undef KIND
    }
  }

  void imm(ObjMethodOp op) {
    out.append(" ");
    switch (op) {
#define OBJMETHOD_OP(name) case ObjMethodOp::name: out.append( #name ); break;
      OBJMETHOD_OPS
#undef OBJMETHOD_OP
    }
  }

  void imm(const bc::MemberKey& mk) {
    using namespace bc;
    out.append(" ");

    const auto writeType = [&] (MemberType t) {
      switch (t) {
        case MemberType::Element:
          out.append("E");
          return;
        case MemberType::Property:
          out.append("P");
          return;
      }
    };

    match<void>(mk,
      [&](const CellMember& m) {
        writeType(m.type);
        folly::format(&out, "C:{}", m.location);
      },
      [&](const LocalMember& m) {
        writeType(m.type);
        out.append("L:");
        imm(m.local);
      },
      [&](const ImmMember& m) {
        writeType(m.type);
        folly::format(&out, "T:\"{}\"", folly::cEscape<std::string>(m.name));
      },
      [&](const ImmIntElem& m) {
        folly::format(&out, "EI:{}", m.val);
      },
      [&](const NewElem& m) {
        out.append("W");
      });
  }

  template<class T>
  void imm(const T& /* imm */) {
    out.append(" <immediate>");
  }

  std::string& out;
};

// This is just a visitor for instructions and exits that will omit a jump
// (Jmp, JmpNS) iff the block that is the jump target follows immediately after
// the jump instruction
struct AssemblyVisitor : public boost::static_visitor<void>
                       , public CFGVisitor {

  explicit AssemblyVisitor(std::string& out)
    : out(out)
    , instr(out)
  {}

  ~AssemblyVisitor() {
    end();
  }

  void doIndent() {
    for (int i = 0 ; i < indent; i++) {
      out.append("  ");
    }
  }

  void beginTry() override {
    end();
    doIndent();
    out.append(".try {\n");
    indent++;
  }

  void beginCatch() override {
    end();
    indent--;
    doIndent();
    out.append("} .catch {\n");
    indent++;
  }

  void endRegion() override {
    end();
    indent--;
    doIndent();
    out.append("}\n");

  }

  void block(Block* blk) override {
    label(blk);
    for (const auto& bc : blk->code) {
      bc.visit(*this);
    }
    for (const auto& ex : blk->exits) {
      exit(ex);
    }
  }

  void label(Block* blk) {
    // if there was an unconditional jump and its target was *not* this block
    // actually emit the instruction
    if (nextUnconditionalDestination
        && nextUnconditionalDestination != blk) {
      bytecode(bc::Jmp{nextUnconditionalDestination});
    }
    nextUnconditionalDestination = nullptr;
    doIndent();
    folly::format(&out, "L{}:\n", blk->id);
  }

  void end() {
    if (nextUnconditionalDestination) {
      bytecode(bc::Jmp{nextUnconditionalDestination});
      nextUnconditionalDestination = nullptr;
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
    doIndent();
    bc.visit(instr);
  }

  void exit(const ExitOp& exit) {
    boost::apply_visitor(*this, exit);
  }

  std::string& out;
  InstrVisitor instr;
  Block* nextUnconditionalDestination{nullptr};
  unsigned indent{1};
};

std::string dump_pseudomain(const Function& func) {
  std::string out;
  out.append(".main {\n");
  out.append(dump_function_body(func));
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
  out.append(dump_function_body(func));
  out.append("}\n\n");
  return out;
}

std::string dump_class(const Class& cls) {
  std::string out;
  out.append(".class ");
  out.append(cls.name);
  out.append(" {\n");
  for (const auto& method : cls.methods) {
    out.append(dump_method(*method));
  }
  out.append("}\n\n");
  return out;

}

std::string dump_method(const Function& func) {
  std::string out;
  out.append(".method [");

  if (func.attr & Attr::AttrPublic) {
    out.append(" public");
  }
  if (func.attr & Attr::AttrProtected) {
    out.append(" protected");
  }
  if (func.attr & Attr::AttrPrivate) {
    out.append(" private");
  }
  if (func.attr & Attr::AttrStatic) {
    out.append(" static");
  }
  if (func.attr & Attr::AttrAbstract) {
    out.append(" abstract");
  }
  if (func.attr & Attr::AttrFinal) {
    out.append(" final");
  }

  out.append(" ] ");
  out.append(func.name);
  out.append("(");
  for (const auto& param : func.params) {
    folly::format(&out, " {}${},",
        param.byRef ? "&" : "",
        param.name);
  }
  out.append(") {\n");
  out.append(dump_function_body(func));
  out.append("}\n\n");
  return out;

}

std::string dump_function_body(const Function& func) {
  std::string out;
  auto locals = analyzeLocals(func);
  out.append("  .declvars");
  for (const auto& name : locals) {
    folly::format(&out, " ${}", name);
  }
  out.append(";\n");
  func.cfg.visit(AssemblyVisitor(out));
  return out;
}

} // namespace

}} // HPHP::php7
