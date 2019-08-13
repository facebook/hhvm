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

#ifndef incl_HPHP_PRINTIR_ANNOTATION_H_
#define incl_HPHP_PRINTIR_ANNOTATION_H_

#include <string>

#include <folly/dynamic.h>
#include <folly/DynamicConverter.h>
#include <folly/Optional.h>

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP { namespace printir {
using BlockId = unsigned int;

struct ParseError : std::runtime_error {
  explicit ParseError(const std::string& errStr) : std::runtime_error(errStr) {}
};

struct SSATmp {
  const uint32_t id;
  const std::string type;
};

struct PhiPseudoInstr {
  const std::vector<std::pair<SSATmp, BlockId>> srcs;
  const SSATmp dst;
};

struct TCRange {
  const jit::AreaIndex area;
  const jit::TCA start;
  const jit::TCA end;
  const std::string disasm;
};

struct Instr {
  const folly::Optional<std::string> rawMarker;
  const std::vector<PhiPseudoInstr> phiPseudoInstrs;
  const jit::Opcode opcode;
  const folly::Optional<std::string> typeParam;
  const folly::Optional<std::string> guard;
  const folly::Optional<std::string> extra;
  const uint32_t id;
  const folly::Optional<BlockId> taken;
  const folly::Optional<std::vector<TCRange>> tcRanges;
  const std::vector<SSATmp> dsts;
  // Only one of these two should be present, they are mutually exclusive
  const folly::Optional<std::vector<SSATmp>> srcs;
  const folly::Optional<std::string> counterName;
};

struct Block {
  const BlockId id;
  const bool isCatch;
  const jit::Block::Hint hint;
  const uint64_t profCount;
  const std::vector<BlockId> preds;
  const folly::Optional<BlockId> next;
  const std::vector<Instr> instrs;
  const jit::AreaIndex area;
};

struct SrcKey {
  const std::string funcStr;
  const std::string unitStr;
  const bool prologue;
  const int offset;
  const ResumeMode resumeMode;
  const bool hasThis;
};

struct TransContext {
  const jit::TransKind kind;
  const TransID id;
  const int optIndex;
  const SrcKey srcKey;
};

struct Unit {
  const TransContext transContext;
  const std::unordered_map<const jit::Opcode, uint32_t> opcodeStats;
  const std::unordered_map<BlockId, Block> blocks;
};

}} // namespace HPHP::printir

namespace folly {
using namespace HPHP;

template <typename T> struct DynamicConverter<Optional<T>> {
  static Optional<T> convert(const dynamic& opt);
};

template <> struct DynamicConverter<jit::Opcode> {
  static jit::Opcode convert(const dynamic&);
};

template <> struct DynamicConverter<printir::SSATmp> {
  static printir::SSATmp convert(const dynamic&);
};

template <> struct DynamicConverter<printir::PhiPseudoInstr> {
  static printir::PhiPseudoInstr convert(const dynamic&);
};

template <> struct DynamicConverter<printir::TCRange> {
  static printir::TCRange convert(const dynamic&);
};

template <> struct DynamicConverter<printir::Instr> {
  static printir::Instr convert(const dynamic&);
};

template <> struct DynamicConverter<printir::Block> {
  static printir::Block convert(const dynamic&);
};

template <> struct DynamicConverter<printir::TransContext> {
  static printir::TransContext convert(const dynamic&);
};

template <> struct DynamicConverter<printir::SrcKey> {
  static printir::SrcKey convert(const dynamic&);
};

template <> struct DynamicConverter<printir::Unit> {
  static printir::Unit convert(const dynamic&);
};

template <typename T> struct DynamicConstructor<Optional<T>> {
  static dynamic construct(const Optional<T>& opt);
};

template <> struct DynamicConstructor<jit::Opcode> {
  static dynamic construct(const jit::Opcode&);
};

template <> struct DynamicConstructor<printir::SSATmp> {
  static dynamic construct(const printir::SSATmp&);
};

template <> struct DynamicConstructor<printir::PhiPseudoInstr> {
  static dynamic construct(const printir::PhiPseudoInstr&);
};

template <> struct DynamicConstructor<printir::TCRange> {
  static dynamic construct(const printir::TCRange&);
};

template <> struct DynamicConstructor<printir::Instr> {
  static dynamic construct(const printir::Instr&);
};

template <> struct DynamicConstructor<printir::Block> {
  static dynamic construct(const printir::Block&);
};

template <> struct DynamicConstructor<printir::TransContext> {
  static dynamic construct(const printir::TransContext&);
};

template <> struct DynamicConstructor<printir::SrcKey> {
  static dynamic construct(const printir::SrcKey&);
};

template <> struct DynamicConstructor<printir::Unit> {
  static dynamic construct(const printir::Unit&);
};

} // namespace folly

#endif
