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

#include <folly/Format.h>
#include <folly/String.h>

namespace HPHP { namespace php7 {

namespace {
  std::string dump_pseudomain(const Function& func);
  std::string dump_block(const Block& func);
} // namespace

std::string dump_asm(const Unit& unit) {
  std::string out;
  folly::format(&out, ".filepath \"{}\";\n\n", unit.name);
  out.append(dump_pseudomain(*unit.getPseudomain()));
  return out;
}

namespace {

struct InstrVisitor {
  explicit InstrVisitor(std::string& out)
    : out(out) {}

  template <class Bytecode>
  void bytecode(const Bytecode& bc) {
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

  void imm(double n) {
    folly::format(&out, " {}", n);
  }

  void imm(const std::string& str) {
    folly::format(&out, " \"{}\"", folly::cEscape<std::string>(str));
  }

  void imm(Block* blk) {
    folly::format(&out, " L{}", blk->id);
  }


  template<class T>
  void imm(const T& imm) {
    out.append(" <immediate>");
  }

  std::string& out;
};

std::string dump_pseudomain(const Function& func) {
  std::string out;
  out.append(".main {\n");
  for (const auto& blk : func.blocks) {
    out.append(dump_block(*blk));
  }
  out.append("}");
  return out;
}

std::string dump_block(const Block& blk) {
  std::string out;
  folly::format(&out, "L{}:\n", blk.id);
  for (const auto& bc : blk.code) {
    out.append("  ");
    bc.visit(InstrVisitor(out));
  }

  return out;
}

} // namespace

}} // HPHP::php7
