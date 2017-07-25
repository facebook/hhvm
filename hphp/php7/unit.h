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

#ifndef incl_HPHP_PHP_UNIT_H
#define incl_HPHP_PHP_UNIT_H

#include "hphp/php7/bytecode.h"
#include "hphp/runtime/base/attr.h"

#include <boost/variant.hpp>

#include <string>
#include <vector>
#include <unordered_set>

namespace HPHP { namespace php7 {

struct Block;
struct Function;
struct Unit;

struct Block {
  // these are the last instructions in the block, they must be jumps or leave
  // the current function i.e. only these instructions:
  using ExitOp = boost::variant<
    bc::Jmp,
    bc::JmpNS,
    bc::JmpZ,
    bc::JmpNZ,
    bc::Switch,
    bc::SSwitch,
    bc::RetC,
    bc::RetV,
    bc::Unwind,
    bc::Throw,
    bc::Fatal
  >;

  void emit(bc::Jmp&&) = delete;
  void emit(bc::JmpNS&&) = delete;
  void emit(bc::JmpZ&&) = delete;
  void emit(bc::JmpNZ&&) = delete;
  void emit(bc::Switch&&) = delete;
  void emit(bc::SSwitch&&) = delete;
  void emit(bc::RetC&&) = delete;
  void emit(bc::RetV&&) = delete;
  void emit(bc::Unwind&&) = delete;
  void emit(bc::Throw&&) = delete;
  void emit(bc::Fatal&&) = delete;
  void emit(ExitOp&& op) = delete;

  void emit(Bytecode&& bc) {
    assert(!exited);
    code.push_back(std::move(bc));
  }

  void exit(ExitOp&& op) {
    exited = true;
    exits.push_back(std::move(op));
  }

  // identifies this block in its unit
  uint64_t id;

  // code associated with this block
  std::vector<Bytecode> code;
  std::vector<ExitOp> exits;
  bool exited{false};
};

// get the series of block pointers in the control graph that starts at `entry`
std::vector<Block*> serializeControlFlowGraph(Block* entry);

struct Function {
  struct Param {
    std::string name;
    bool byRef;
  };

  explicit Function(Unit* parent,
      const std::string& name);

  Block* getEntry() { return entry; }

  Block* allocateBlock();
  Block* getBlock(uint64_t id);

  std::string name;
  Attr attr;
  Block* entry;
  Unit* parent;
  std::vector<std::unique_ptr<Block>> blocks;
  std::vector<Param> params;
  std::unordered_set<std::string> locals;
};

struct Unit {
  explicit Unit()
    : pseudomain(std::make_unique<Function>(this, "")) {}

  Function* getPseudomain() const {
    return pseudomain.get();
  }

  Function* makeFunction(const std::string& name) {
    functions.emplace_back(std::make_unique<Function>(this, name));
    return functions.back().get();
  }

  std::string name;
  std::unique_ptr<Function> pseudomain;
  std::vector<std::unique_ptr<Function>> functions;
};

std::unique_ptr<Unit> makeFatalUnit(const std::string& filename,
                                    const std::string& msg);


}} // HPHP::php7

#endif // incl_HPHP_PHP_UNIT_H
