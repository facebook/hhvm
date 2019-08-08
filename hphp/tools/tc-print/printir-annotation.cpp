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

#include "hphp/tools/tc-print/printir-annotation.h"

#include <limits>

using std::string;
using std::vector;

namespace {
using namespace folly;

string typeToName(dynamic::Type type) {
  switch(type) {
    case dynamic::Type::NULLT:  return "null";
    case dynamic::Type::ARRAY:  return "array";
    case dynamic::Type::BOOL:   return "boolean";
    case dynamic::Type::DOUBLE: return "double";
    case dynamic::Type::INT64:  return "int64";
    case dynamic::Type::OBJECT: return "object";
    case dynamic::Type::STRING: return "string";
  }
  always_assert(false);
}

void typeError(const std::string& name,
               const dynamic::Type expectedType,
               const dynamic::Type actualType) {
  throw HPHP::printir::ParseError(
    sformat("{} must have type {}, got type {}",
            name,
            typeToName(expectedType),
            typeToName(actualType)));
}

void enumError(const std::string& enumName,
               const std::string& incorrectValue) {
  throw HPHP::printir::ParseError(
    sformat("\"{}\" is not a valid value for enum {}",
            incorrectValue,
            enumName));
}

HPHP::jit::TCA parseTCA(const std::string& raw) {
  if (raw == "(null)") return nullptr;

  unsigned long long ull;
  if (std::sscanf(raw.c_str(), "%llx", &ull) != 1) {
    throw HPHP::printir::ParseError(
      sformat("Unable to parse value \"{}\" as TCA", raw));
  }

  return reinterpret_cast<jit::TCA>(ull);
}

}

namespace folly {
using namespace HPHP;

template <typename T> Optional<T> DynamicConverter<Optional<T>>::
convert(const dynamic& d) {
  return d.isNull() ? none : Optional<T>(convertTo<T>(d));
}

jit::Opcode DynamicConverter<jit::Opcode>::
convert(const dynamic& d) {
  auto const rawOpcode = convertTo<string>(d);
  auto const maybeOpcode = jit::nameToOpcode(rawOpcode);
  if (!maybeOpcode) enumError("Opcode", rawOpcode);

  return *maybeOpcode;
}

printir::SSATmp DynamicConverter<printir::SSATmp>::
convert(const dynamic& d) {
  if (!d.isObject()) {
    typeError("SSATmp", dynamic::Type::OBJECT, d.type());
  }

  auto const id = convertTo<uint32_t>(d.getDefault("id", {}));
  auto const typeStr = convertTo<string>(d.getDefault("type", {}));

  return printir::SSATmp{id, typeStr};
}

printir::PhiPseudoInstr DynamicConverter<printir::PhiPseudoInstr>::
convert(const dynamic& d) {
  if (!d.isObject()) {
    typeError("PhiPseudoInstr", dynamic::Type::OBJECT, d.type());
  }

  vector<std::pair<printir::SSATmp, printir::BlockId>> srcs;
  for (auto const& item : d.getDefault("srcs", {})) {
    auto const src = convertTo<printir::SSATmp>(item["src"]);
    auto const id = convertTo<printir::BlockId>(
      item.getDefault("label").getDefault("id", {}));

    srcs.emplace_back(src, id);
  }

  auto const dst = convertTo<printir::SSATmp>(d.getDefault("dst", {}));

  return printir::PhiPseudoInstr{srcs, dst};
}

printir::TCRange DynamicConverter<printir::TCRange>::
convert(const dynamic& d) {
  if (!d.isObject()) {
    typeError("TCRange", dynamic::Type::OBJECT, d.type());
  }

  auto const rawArea = convertTo<string>(d.getDefault("area", {}));
  auto const maybeArea = jit::nameToAreaIndex(rawArea);
  if (!maybeArea) enumError("AreaIndex", rawArea);
  auto const area = *maybeArea;

  auto const start = parseTCA(convertTo<string>(d.getDefault("start", {})));
  auto const end = parseTCA(convertTo<string>(d.getDefault("end", {})));
  auto const disasm = convertTo<string>(d.getDefault("disasm",{}));

  return printir::TCRange{area, start, end, disasm};
}

printir::Instr DynamicConverter<printir::Instr>::
convert(const dynamic& d) {
  if (!d.isObject()) {
    typeError("Instr", dynamic::Type::OBJECT, d.type());
  }

  auto const maybeMarker = convertTo<Optional<string>>(
    d.getDefault("marker").getDefault("raw", {}));
  auto const phiPseudoInstrs = convertTo<vector<printir::PhiPseudoInstr>>(
    d.getDefault("phiPseudoInstrs", dynamic::array));
  auto const opcode = convertTo<jit::Opcode>(d.getDefault("opcodeName", {}));
  auto const typeParam = convertTo<Optional<string>>(
    d.getDefault("typeParam", {}));
  auto const guard = convertTo<Optional<string>>(d.getDefault("guard", {}));
  auto const extra = convertTo<Optional<string>>(d.getDefault("extra", {}));
  auto const id = convertTo<Optional<uint32_t>>(d.getDefault("id", {}));
  auto const dsts = convertTo<vector<printir::SSATmp>>(
    d.getDefault("dsts", {}));
  auto const tcRanges = convertTo<Optional<vector<printir::TCRange>>>(
    d.getDefault("tc_ranges", {}));
  auto const maybeTaken = d.getDefault("taken", {});
  auto const taken = convertTo<Optional<printir::BlockId>>(
    (maybeTaken.isNull() ? dynamic() : maybeTaken.getDefault("id", {})));

  // TODO(elijahrivera) - rewrite to maybe use boost::variant?
  //                      other options to standardize?
  Optional<vector<printir::SSATmp>> srcs;
  Optional<string> counterName;
  auto const& d_srcs = d.getDefault("srcs", {});
  if (d_srcs.isArray()) {
    srcs = convertTo<vector<printir::SSATmp>>(d_srcs);
  } else if (d_srcs.isObject()) {
    counterName = convertTo<Optional<string>>(
      d_srcs.getDefault("counterName", {}));
  } else {
    // Unique error case, no one "expected" type, but we know this current shape
    // is incorrect
    throw HPHP::printir::ParseError(
      sformat("Field \"srcs\" has incorrect type {}", d_srcs.typeName()));
  }

  return printir::Instr{maybeMarker, phiPseudoInstrs, opcode, typeParam, guard,
                        extra, id, taken, tcRanges, dsts, srcs, counterName};
}

printir::Block DynamicConverter<printir::Block>::
convert(const dynamic& d) {
  if (!d.isObject()) typeError("Block", dynamic::Type::OBJECT, d.type());

  auto const& label = d.getDefault("label");
  auto const id = convertTo<printir::BlockId>(label.getDefault("id", {}));
  auto const isCatch = convertTo<bool>(label.getDefault("isCatch", {}));

  auto const rawHint = convertTo<string>(label.getDefault("hint", {}));
  auto const maybeHint = jit::nameToHint(rawHint);
  if (!maybeHint) enumError("Hint", rawHint);
  auto const hint = *maybeHint;

  auto const rawArea = convertTo<string>(d.getDefault("area", {}));
  auto const maybeArea = jit::nameToAreaIndex(rawArea);
  if (!maybeArea) enumError("AreaIndex", rawArea);
  auto const area = *maybeArea;

  auto const profCount = convertTo<uint64_t>(d.getDefault("profCount", {}));
  auto const preds = convertTo<vector<printir::BlockId>>(
    d.getDefault("preds", {}));
  auto const instrs = convertTo<vector<printir::Instr>>(
    d.getDefault("instrs", {}));
  auto const maybeNext = d.getDefault("next", {});
  auto const maybeNextId = convertTo<Optional<printir::BlockId>>(
    (maybeNext.isNull() ? dynamic() : maybeNext.getDefault("id", {})));

  return printir::Block{id, isCatch, hint, profCount, preds, maybeNextId,
                        instrs, area};
}

printir::SrcKey DynamicConverter<printir::SrcKey>::
convert(const dynamic& d) {
  if (!d.isObject()) typeError("SrcKey", dynamic::Type::OBJECT, d.type());

  auto const funcStr = convertTo<string>(d.getDefault("func", {}));
  auto const unitStr = convertTo<string>(d.getDefault("unit", {}));
  auto const prologue = convertTo<bool>(d.getDefault("prologue", {}));
  auto const offset = convertTo<int>(d.getDefault("offset", {}));
  auto const hasThis = convertTo<bool>(d.getDefault("hasThis", {}));

  auto const rawResumeMode = convertTo<string>(d.getDefault("resumeMode", {}));
  auto const maybeResumeMode = nameToResumeMode(rawResumeMode);
  if (!maybeResumeMode) enumError("ResumeMode", rawResumeMode);
  auto const resumeMode = *maybeResumeMode;

  return printir::SrcKey{funcStr, unitStr, prologue, offset, resumeMode,
                         hasThis};
}

printir::TransContext DynamicConverter<printir::TransContext>::
convert(const dynamic& d) {
  if (!d.isObject()) typeError("TransContext", dynamic::Type::OBJECT, d.type());

  auto const rawTransKind = convertTo<string>(d.getDefault("kind", {}));
  auto const maybeTransKind = jit::nameToTransKind(rawTransKind);
  if (!maybeTransKind) enumError("TransKind", rawTransKind);
  auto const transKind = *maybeTransKind;

  auto const id = convertTo<TransID>(d.getDefault("id", {}));
  auto const optIndex = convertTo<int>(d.getDefault("optIndex", {}));
  auto const srcKey = convertTo<printir::SrcKey>(d.getDefault("srcKey", {}));

  return printir::TransContext{transKind, id, optIndex, srcKey};
}

printir::Unit DynamicConverter<printir::Unit>::
convert(const dynamic& d) {
  if (!d.isObject()) typeError("Unit", dynamic::Type::OBJECT, d.type());

  auto const transContext = convertTo<printir::TransContext>(
    d.getDefault("translation", {}));
  auto const opcodeStats =
  convertTo<std::unordered_map<const jit::Opcode, uint32_t>>(
    d.getDefault("opcodeStats", {}));

  std::unordered_map<unsigned int, printir::Block> blocks;
  for (auto const& block : d.getDefault("blocks", {})) {
    auto const convertedBlock = convertTo<printir::Block>(block);
    auto const blockId = convertedBlock.id;
    auto const success = blocks.emplace(blockId,
                                        convertedBlock);
    if (!success.second) {
      throw HPHP::printir::ParseError(
        sformat("Block has repeated id: {}", blockId));
    }
  }

  return printir::Unit{transContext, opcodeStats, blocks};
}

template <typename T> dynamic DynamicConstructor<Optional<T>>::
construct(const Optional<T>& opt) {
  return opt ? toDynamic(*opt) : dynamic();
}

dynamic DynamicConstructor<jit::Opcode>::
construct(const jit::Opcode& opc) {
  return jit::opcodeName(opc);
}

dynamic DynamicConstructor<printir::SSATmp>::
construct(const printir::SSATmp& s) {
  return dynamic::object("id", s.id)
                        ("type", s.type);
}

dynamic DynamicConstructor<printir::PhiPseudoInstr>::
construct(const printir::PhiPseudoInstr& p) {
  return dynamic::object("srcs", toDynamic(p.srcs))
                        ("dst", toDynamic(p.dst));
}

dynamic DynamicConstructor<printir::TCRange>::
construct(const printir::TCRange& t) {
  return dynamic::object("area", jit::areaAsString(t.area))
                        ("start", sformat("{}", static_cast<void*>(t.start)))
                        ("end", sformat("{}", static_cast<void*>(t.end)))
                        ("disasm", t.disasm);
}

dynamic DynamicConstructor<printir::Instr>::
construct(const printir::Instr& i) {
  return dynamic::object("rawMarker", toDynamic(i.rawMarker))
                        ("phiPseudoInstrs", toDynamic(i.phiPseudoInstrs))
                        ("opcode", toDynamic(i.opcode))
                        ("typeParam", toDynamic(i.typeParam))
                        ("guard", toDynamic(i.guard))
                        ("extra", toDynamic(i.extra))
                        ("id", toDynamic(i.id))
                        ("taken", toDynamic(i.taken))
                        ("tcRanges", toDynamic(i.tcRanges))
                        ("dsts", toDynamic(i.dsts))
                        ("srcs", toDynamic(i.srcs))
                        ("counterName", toDynamic(i.counterName));
}

dynamic DynamicConstructor<printir::Block>::
construct(const printir::Block& b) {
  return dynamic::object("id", b.id)
                        ("isCatch", b.isCatch)
                        ("hint", jit::blockHintName(b.hint))
                        ("profCount", b.profCount)
                        ("next", toDynamic(b.next))
                        ("instrs", toDynamic(b.instrs));
}

dynamic DynamicConstructor<printir::SrcKey>::
construct(const printir::SrcKey& s) {
  return dynamic::object("funcStr", s.funcStr)
                        ("unitStr", s.unitStr)
                        ("prologue", s.prologue)
                        ("offset", s.offset)
                        ("resumeMode", resumeModeShortName(s.resumeMode))
                        ("hasThis", s.hasThis);
}

dynamic DynamicConstructor<printir::TransContext>::
construct(const printir::TransContext& t) {
  return dynamic::object("kind", show(t.kind))
                        ("id", t.id)
                        ("optIndex", t.optIndex)
                        ("srcKey", toDynamic(t.srcKey));
}

dynamic DynamicConstructor<printir::Unit>::
construct(const printir::Unit &u) {
  dynamic blockMap = dynamic::object();
  for (auto const& item: u.blocks) {
    blockMap[sformat("{}", item.first)] = toDynamic(item.second);
  }

  return dynamic::object("transContext", toDynamic(u.transContext))
                        ("opcodeStats", toDynamic(u.opcodeStats))
                        ("blocks", blockMap);
}

} // namespace folly
