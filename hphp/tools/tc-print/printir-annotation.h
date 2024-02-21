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

#include <folly/json/dynamic.h>
#include <folly/json/DynamicConverter.h>

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP::printir {
using BlockId = unsigned int;
using InstrId = uint32_t;

struct Instr;
struct Block;
struct Unit;

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

  InstrId parentInstrId;
  BlockId parentBlockId;
};

struct TCRange {
  jit::AreaIndex area;
  jit::TCA start;
  jit::TCA end;
  std::string disasm;

  InstrId parentInstrId;
  BlockId parentBlockId;
};

struct Profile {
  const Offset offset;
  const std::string name;
  const folly::dynamic data;
};

struct Instr {
  const InstrId id;
  const Optional<std::string> rawMarker;
  std::vector<PhiPseudoInstr> phiPseudoInstrs;
  const jit::Opcode opcode;
  const Optional<std::string> typeParam;
  const Optional<std::string> guard;
  const Optional<std::string> extra;
  const Optional<BlockId> taken;
  std::vector<TCRange> tcRanges;
  const std::vector<SSATmp> dsts;
  const Offset offset;
  const std::vector<Profile> profileData;
  // Only one of these two should be present, they are mutually exclusive
  const Optional<std::vector<SSATmp>> srcs;
  const Optional<std::string> counterName;
  const int startLine;

  BlockId parentBlockId;
};

struct Block {
  const BlockId id;
  const bool isCatch;
  const jit::Block::Hint hint;
  const uint64_t profCount;
  const std::vector<BlockId> preds;
  const Optional<BlockId> next;
  std::vector<Instr> instrs;
  const jit::AreaIndex area;
};

struct SrcKey {
  const std::string funcStr;
  const std::string unitStr;
  const bool prologue;
  const int offset;
  const ResumeMode resumeMode;
  const bool hasThis;
  const int startLine;
};

struct TransContext {
  const jit::TransKind kind;
  const TransID id;
  const int optIndex;
  const SrcKey srcKey;
  const std::string funcName;
  const std::string sourceFile;
  const int startLine;
  const int endLine;
};

struct InliningDecision {
  const bool wasInlined;
  const Offset offset;
  const Optional<std::string> callerName;
  const Optional<std::string> calleeName;
  const std::string reason;
};

struct Unit {
  const TransContext transContext;
  const std::unordered_map<BlockId, Block> blocks;
  const std::vector<InliningDecision> inliningDecisions;
};

} // namespace HPHP::printir

namespace folly {
using namespace HPHP;

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

template <> struct DynamicConverter<printir::Profile> {
  static printir::Profile convert(const dynamic&);
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

template <> struct DynamicConverter<printir::InliningDecision> {
  static printir::InliningDecision convert(const dynamic&);
};

template <> struct DynamicConverter<printir::Unit> {
  static printir::Unit convert(const dynamic&);
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

template <> struct DynamicConstructor<printir::Profile> {
  static dynamic construct(const printir::Profile&);
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

template <> struct DynamicConstructor<printir::InliningDecision> {
  static dynamic construct(const printir::InliningDecision&);
};

template <> struct DynamicConstructor<printir::Unit> {
  static dynamic construct(const printir::Unit&);
};

} // namespace folly

#endif
