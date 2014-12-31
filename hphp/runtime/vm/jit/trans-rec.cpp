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

#include "hphp/runtime/vm/jit/trans-rec.h"

#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP { namespace jit {

TransRec::TransRec(SrcKey                      _src,
                   TransKind                   _kind,
                   TCA                         _aStart,
                   uint32_t                    _aLen,
                   TCA                         _acoldStart,
                   uint32_t                    _acoldLen,
                   TCA                         _afrozenStart,
                   uint32_t                    _afrozenLen,
                   RegionDescPtr               region,
                   std::vector<TransBCMapping> _bcMapping)
  : bcMapping(_bcMapping)
  , funcName(_src.func()->fullName()->data())
  , src(_src)
  , md5(_src.func()->unit()->md5())
  , aStart(_aStart)
  , acoldStart(_acoldStart)
  , afrozenStart(_afrozenStart)
  , aLen(_aLen)
  , acoldLen(_acoldLen)
  , afrozenLen(_afrozenLen)
  , bcStart(_src.offset())
  , id(0)
  , kind(_kind)
{
  if (funcName.empty()) funcName = "Pseudo-main";

  if (!region) return;

  assert(!region->empty());
  for (auto& block : region->blocks()) {
    auto sk = block->start();
    blocks.emplace_back(Block{sk.unit()->md5(), sk.offset(),
                              block->last().advanced().offset()});
  }

  auto& firstBlock = *region->blocks().front();
  auto guardRange = firstBlock.typePreds().equal_range(firstBlock.start());
  for (; guardRange.first != guardRange.second; ++guardRange.first) {
    guards.emplace_back(show(guardRange.first->second));
  }
}


std::string
TransRec::print(uint64_t profCount) const {
  std::string ret;
  std::string funcName = src.func()->fullName()->data();

  // Split up the call to prevent template explosion
  folly::format(
    &ret,
    "Translation {} {{\n"
    "  src.md5 = {}\n"
    "  src.funcId = {}\n"
    "  src.funcName = {}\n"
    "  src.resumed = {}\n"
    "  src.bcStart = {}\n"
    "  src.blocks = {}\n",
    id, md5, src.getFuncId(),
    funcName.empty() ? "Pseudo-main" : funcName,
    (int32_t)src.resumed(),
    src.offset(),
    blocks.size());

  for (auto const& block : blocks) {
    folly::format(
      &ret,
      "    {} {} {}\n",
      block.md5, block.bcStart, block.bcPast);
  }

  folly::format( &ret, "  src.guards = {}\n", guards.size());

  for (auto const& guard : guards) {
    folly::format( &ret, "    {}\n", guard);
  }

  folly::format(
    &ret,
    "  kind = {} ({})\n"
    "  aStart = {}\n"
    "  aLen = {:#x}\n"
    "  coldStart = {}\n"
    "  coldLen = {:#x}\n"
    "  frozenStart = {}\n"
    "  frozenLen = {:#x}\n",
    static_cast<uint32_t>(kind), show(kind),
    aStart, aLen,
    acoldStart, acoldLen,
    afrozenStart, afrozenLen);

  folly::format(
    &ret,
    "  profCount = {}\n"
    "  bcMapping = {}\n",
    profCount, bcMapping.size());

  for (auto const& info : bcMapping) {
    folly::format(
      &ret,
      "    {} {} {} {} {}\n",
      info.md5, info.bcStart,
      info.aStart, info.acoldStart, info.afrozenStart);
  }

  ret += "}\n\n";
  return ret;
}

Offset TransRec::bcPast() const {
  return blocks.empty() ? 0 : blocks.back().bcPast;
}

} }
