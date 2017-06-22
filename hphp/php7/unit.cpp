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

#include <hphp/php7/unit.h>

namespace HPHP { namespace PHP7 {

Function::Function(Unit* parent, const std::string& name)
  : name(name),
    attr(),
    parent(parent) {
  entry = allocateBlock();
}

Block* Function::allocateBlock() {
  uint64_t id = blocks.size();

  auto block = std::make_unique<Block>();
  block->id = id;
  auto borrowed = block.get();
  blocks.emplace_back(std::move(block));

  return borrowed;
}

Block* Function::getBlock(uint64_t id) {
  if (id >= blocks.size()) {
    return nullptr;
  }

  return blocks[id].get();
}

}} // HPHP::PHP7
