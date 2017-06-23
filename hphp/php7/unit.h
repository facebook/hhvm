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

#include <vector>
#include <string>

namespace HPHP { namespace php7 {

struct Block;
struct Function;
struct Unit;

struct Block {
  void emit(Bytecode&& bc) {
    code.push_back(bc);
  }

  // identifies this block in its unit
  uint64_t id;

  // code associated with this block
  std::vector<Bytecode> code;

  Block* fallthrough = nullptr;
};

struct Function {
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
};

struct Unit {
  explicit Unit()
    : pseudomain(std::make_unique<Function>(this, "")) {}

  Function* getPseudomain() const {
    return pseudomain.get();
  }

  std::string name;
  std::unique_ptr<Function> pseudomain;
  std::vector<std::unique_ptr<Function>> functions;
};


}} // HPHP::php7

#endif // incl_HPHP_PHP_UNIT_H
