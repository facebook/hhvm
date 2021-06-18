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

#include "hphp/runtime/vm/unit-emitter.h"

#include "hphp/compiler/builtin_symbols.h"
#include "hphp/parser/location.h"
#include "hphp/system/systemlib.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/variable-serializer.h"

#include "hphp/runtime/ext/std/ext_std_variable.h"

#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/disas.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/jit/perf-counters.h"
#include "hphp/runtime/vm/litstr-table.h"
#include "hphp/runtime/vm/litarray-table.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/preclass.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/record-emitter.h"
#include "hphp/runtime/vm/repo-autoload-map-builder.h"
#include "hphp/runtime/vm/type-alias-emitter.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/verifier/check.h"

#include "hphp/util/alloc.h"
#include "hphp/util/logger.h"
#include "hphp/util/read-only-arena.h"
#include "hphp/util/sha1.h"
#include "hphp/util/trace.h"

#include <boost/algorithm/string/predicate.hpp>

#include <folly/Memory.h>
#include <folly/FileUtil.h>

#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhbc);

///////////////////////////////////////////////////////////////////////////////

UnitEmitter::UnitEmitter(const SHA1& sha1,
                         const SHA1& bcSha1,
                         const Native::FuncTable& nativeFuncs,
                         bool useGlobalIds)
  : m_useGlobalIds(useGlobalIds)
  , m_nativeFuncs(nativeFuncs)
  , m_sha1(sha1)
  , m_bcSha1(bcSha1)
  , m_nextFuncSn(0)
{}

UnitEmitter::~UnitEmitter() {
  for (auto& pce : m_pceVec) delete pce;
  for (auto& re : m_reVec) delete re;
}

///////////////////////////////////////////////////////////////////////////////
// Litstrs and Arrays.

const StringData* UnitEmitter::lookupLitstr(Id id) const {
  if (!isUnitId(id)) {
    return LitstrTable::get().lookupLitstrId(id);
  }
  auto unitId = decodeUnitId(id);
  assertx(unitId < m_litstrs.size());
  return m_litstrs[unitId];
}

const ArrayData* UnitEmitter::lookupArray(Id id) const {
  if (!isUnitId(id)) {
    return LitarrayTable::get().lookupLitarrayId(id);
  }
  auto unitId = decodeUnitId(id);
  assertx(unitId < m_arrays.size());
  return m_arrays[unitId];
}

const RepoAuthType::Array* UnitEmitter::lookupArrayType(Id id) const {
  return RuntimeOption::RepoAuthoritative
           ? globalArrayTypeTable().lookup(id)
           : m_arrayTypeTable.lookup(id);
}

void UnitEmitter::repopulateArrayTypeTable(
  const ArrayTypeTable::Builder& builder) {
  m_arrayTypeTable.repopulate(builder);
}

Id UnitEmitter::mergeLitstr(const StringData* litstr) {
  if (m_useGlobalIds) {
    return LitstrTable::get().mergeLitstr(litstr);
  }
  return encodeUnitId(mergeUnitLitstr(litstr));
}

Id UnitEmitter::mergeUnitLitstr(const StringData* litstr) {
  auto it = m_litstr2id.find(litstr);
  if (it == m_litstr2id.end()) {
    const StringData* str = makeStaticString(litstr);
    Id id = m_litstrs.size();
    m_litstrs.push_back(str);
    m_litstr2id[str] = id;
    return id;
  } else {
    return it->second;
  }
}

Id UnitEmitter::mergeArray(const ArrayData* a) {
  assertx(a->isStatic());
  if (m_useGlobalIds) {
    return LitarrayTable::get().mergeLitarray(a);
  }
  return encodeUnitId(mergeUnitArray(a));
}

Id UnitEmitter::mergeUnitArray(const ArrayData* a) {
  assertx(a->isStatic());
  auto const id = static_cast<Id>(m_arrays.size());
  m_array2id.emplace(a, id);
  m_arrays.push_back(a);
  return id;
}


///////////////////////////////////////////////////////////////////////////////
// FuncEmitters.

FuncEmitter* UnitEmitter::newFuncEmitter(const StringData* name) {
  auto fe = std::make_unique<FuncEmitter>(*this, m_nextFuncSn++, m_fes.size(),
                                          name);
  m_fes.push_back(std::move(fe));
  return m_fes.back().get();
}

FuncEmitter* UnitEmitter::newMethodEmitter(const StringData* name,
                                           PreClassEmitter* pce) {
  return new FuncEmitter(*this, m_nextFuncSn++, name, pce);
}

Func* UnitEmitter::newFunc(const FuncEmitter* fe, Unit& unit,
                           const StringData* name, Attr attrs,
                           int numParams) {
  Func *func = nullptr;
  if (attrs & AttrIsMethCaller) {
    auto const pair = Func::getMethCallerNames(name);
    func = new (Func::allocFuncMem(numParams)) Func(
      unit, name, attrs, pair.first, pair.second);
  } else {
    func = new (Func::allocFuncMem(numParams)) Func(unit, name, attrs);
  }
  return func;
}


///////////////////////////////////////////////////////////////////////////////
// PreClassEmitters.

PreClassEmitter* UnitEmitter::newPreClassEmitter(
  const std::string& name
) {
  auto pce = new PreClassEmitter(*this, m_pceVec.size(), name);
  m_pceVec.push_back(pce);
  return pce;
}

RecordEmitter* UnitEmitter::newRecordEmitter(const std::string& name) {
  auto const re = new RecordEmitter(*this, m_reVec.size(), name);
  m_reVec.push_back(re);
  return re;
}

Id UnitEmitter::pceId(folly::StringPiece clsName) {
  Id id = 0;
  for (auto p : m_pceVec) {
    if (p->name()->slice() == clsName) return id;
    id++;
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////////////
// Type aliases.

TypeAliasEmitter* UnitEmitter::newTypeAliasEmitter(const std::string& name) {
  auto te = std::make_unique<TypeAliasEmitter>(*this, m_typeAliases.size(), name);
  m_typeAliases.push_back(std::move(te));
  return m_typeAliases.back().get();
}

///////////////////////////////////////////////////////////////////////////////
// Constants.

Id UnitEmitter::addConstant(const Constant& c) {
  Id id = m_constants.size();
  TRACE(1, "Add Constant %d %s %d\n", id, c.name->data(), c.attrs);
  m_constants.push_back(c);
  return id;
}

///////////////////////////////////////////////////////////////////////////////
// Initialization and execution.

ServiceData::ExportedCounter* g_hhbc_size =
  ServiceData::createCounter("vm.hhbc-size");

bool UnitEmitter::check(bool verbose) const {
  return Verifier::checkUnit(
    this,
    verbose ? Verifier::kVerbose : Verifier::kStderr
  );
}

bool needs_extended_line_table() {
  return RuntimeOption::RepoDebugInfo &&
    (RuntimeOption::EvalDumpHhas ||
     RuntimeOption::EnableHphpdDebugger ||
     RuntimeOption::EnableVSDebugger ||
     RuntimeOption::EnableDebuggerServer);
}

std::unique_ptr<Unit> UnitEmitter::create() const {
  INC_TPC(unit_load);

  tracing::BlockNoTrace _{"unit-create"};

  static const bool kVerify = debug || RuntimeOption::EvalVerify ||
    RuntimeOption::EvalVerifyOnly || RuntimeOption::EvalFatalOnVerifyError;
  static const bool kVerifyVerboseSystem =
    getenv("HHVM_VERIFY_VERBOSE_SYSTEM");
  static const bool kVerifyVerbose =
    kVerifyVerboseSystem || getenv("HHVM_VERIFY_VERBOSE");

  const bool isSystemLib = FileUtil::isSystemName(m_filepath->slice());
  const bool doVerify =
    kVerify || boost::ends_with(m_filepath->data(), ".hhas");
  if (doVerify) {
    // The verifier needs the bytecode available, but we don't want to
    // necessarily force it to load (otherwise it would defeat the
    // point of lazy loading when we're using the verifier). So, load
    // the bytecode for all functions, but reset it back to the tokens
    // when we're done.
    std::lock_guard lock{m_verifyLock};
    std::vector<std::pair<FuncEmitter*, Func::BCPtr::Token>> tokens;
    SCOPE_EXIT {
      for (auto& p : tokens) {
        p.first->setBcToken(p.second, p.first->bcPos());
      }
    };
    if (RO::RepoAuthoritative) {
      for (auto& fe : m_fes) {
        auto const token = fe->loadBc();
        if (!token) continue;
        tokens.emplace_back(std::make_pair(fe.get(), *token));
      }
      for (auto& pce : m_pceVec) {
        for (auto& fe : pce->methods()) {
          auto const token = fe->loadBc();
          if (!token) continue;
          tokens.emplace_back(std::make_pair(fe, *token));
        }
      }
    }

    auto const verbose = isSystemLib ? kVerifyVerboseSystem : kVerifyVerbose;
    if (!check(verbose)) {
      if (!verbose) {
        std::cerr << folly::format(
          "Verification failed for unit {}. Re-run with "
          "HHVM_VERIFY_VERBOSE{}=1 to see more details.\n",
          m_filepath->data(), isSystemLib ? "_SYSTEM" : ""
        );
      }
      if (RuntimeOption::EvalVerifyOnly) {
        if (!isSystemLib) {
          std::fflush(stdout);
          _Exit(1);
        }
      } else if (RuntimeOption::EvalFatalOnVerifyError) {
        return createFatalUnit(
          const_cast<StringData*>(m_filepath),
          m_sha1,
          FatalOp::Parse,
          "A bytecode verification error was detected"
        )->create();
      }
    }
    if (!isSystemLib && RuntimeOption::EvalVerifyOnly) {
      std::fflush(stdout);
      _Exit(0);
    }
  }

  std::unique_ptr<Unit> u {
    RuntimeOption::RepoAuthoritative && !RuntimeOption::SandboxMode &&
      m_litstrs.empty() && m_arrayTypeTable.empty() && m_arrays.empty() ?
    new Unit : new UnitExtended
  };

  if (m_fatalUnit) {
    FatalInfo info{m_fatalLoc, m_fatalOp, m_fatalMsg};
    u->m_fatalInfo = std::make_unique<FatalInfo>(info);
  }

  u->m_sn = m_sn;
  u->m_origFilepath = m_filepath;
  u->m_sha1 = m_sha1;
  u->m_bcSha1 = m_bcSha1;
  for (auto const& pce : m_pceVec) {
    u->m_preClasses.push_back(PreClassPtr(pce->create(*u)));
  }
  for (auto const& re : m_reVec) {
    u->m_preRecords.push_back(PreRecordDescPtr(re->create(*u)));
  }
  for (auto const& te : m_typeAliases) {
    u->m_typeAliases.push_back(te->create(*u));
  }
  u->m_constants = m_constants;
  u->m_metaData = m_metaData;
  u->m_fileAttributes = m_fileAttributes;
  u->m_ICE = m_ICE;

  size_t ix = 0;
  for (auto& fe : m_fes) {
    auto const func = fe->create(*u, nullptr);
    assertx(ix == fe->id());
    u->m_funcs.push_back(func);
    ix++;
  }

  if (u->m_extended) {
    auto ux = u->getExtended();
    for (auto s : m_litstrs) {
      ux->m_namedInfo.emplace_back(LowStringPtr{s});
    }
    ux->m_arrays = m_arrays;
    ux->m_arrayTypeTable = m_arrayTypeTable;

    // If prefetching is enabled, store the symbol refs in the Unit so
    // the prefetcher can claim them. Reset the atomic flag to mark
    // them available. Otherwise set the atomic flag was already
    // claimed as a shortcut.
    if (!RO::RepoAuthoritative && unitPrefetchingEnabled()) {
      ux->m_symbolRefsForPrefetch = m_symbol_refs;
      ux->m_symbolRefsPrefetched.clear();
    } else {
      ux->m_symbolRefsPrefetched.test_and_set();
    }
  } else {
    assertx(!m_litstrs.size());
    assertx(m_arrayTypeTable.empty());
  }

  if (RuntimeOption::EvalDumpHhas > 1 ||
    (SystemLib::s_inited && RuntimeOption::EvalDumpHhas == 1)) {
    auto const& hhaspath = RuntimeOption::EvalDumpHhasToFile;
    if (!hhaspath.empty()) {
      static std::atomic<bool> first_unit{true};
      auto const flags = O_WRONLY | O_CREAT | (first_unit ? O_TRUNC : O_APPEND);
      if (!folly::writeFile(disassemble(u.get()), hhaspath.c_str(), flags)) {
        Logger::Error("Failed to write hhas to %s", hhaspath.c_str());
        _Exit(1);
      }
      first_unit = false;
    } else {
      std::printf("%s", disassemble(u.get()).c_str());
      std::fflush(stdout);
    }
    if (SystemLib::s_inited) {
      _Exit(0);
    }
  }

  if (RuntimeOption::EvalDumpBytecode) {
    // Dump human-readable bytecode.
    Trace::traceRelease("%s", u->toString().c_str());
  }

  return u;
}

///////////////////////////////////////////////////////////////////////////////

template<class SerDe>
void UnitEmitter::serdeMetaData(SerDe& sd) {
  sd(m_metaData)
    (m_fileAttributes)
    (m_symbol_refs)
    (m_bcSha1)
    (m_fatalUnit)
    ;
  if (m_fatalUnit) {
    sd(m_fatalLoc)
      (m_fatalOp)
      (m_fatalMsg)
      ;
  }

  if (RuntimeOption::EvalLoadFilepathFromUnitCache) {
    assertx(!RO::RepoAuthoritative);
    /* May be different than the unit origin: e.g. for hhas files. */
    sd(m_filepath);
  }
}

template <typename SerDe>
void UnitEmitter::serde(SerDe& sd, bool lazy) {
  MemoryManager::SuppressOOM so(*tl_heap);

  serdeMetaData(sd);
  // These are not touched by serdeMetaData:
  sd(m_ICE);

  auto const seq = [&] (auto const& c, auto const& r, auto const& w) {
    if constexpr (SerDe::deserializing) {
      size_t size;
      sd(size);
      for (size_t i = 0; i < size; ++i) r(sd, i);
    } else {
      sd(c.size());
      for (auto const& x : c) w(sd, x);
    }
  };

  // Literal strings
  seq(
    m_litstrs,
    [&] (auto& sd, size_t i) {
      const StringData* s;
      sd(s);
      auto const id UNUSED = mergeUnitLitstr(s);
      assertx(id == i);
    },
    [&] (auto& sd, const StringData* s) { sd(s); }
  );

  // Arrays
  seq(
    m_arrays,
    [&] (auto& sd, size_t i) {
      std::string key;
      sd(key);

      auto v = [&]{
        VariableUnserializer vu{
          key.data(),
          key.size(),
          VariableUnserializer::Type::Internal
        };
        vu.setUnitFilename(m_filepath);
        return vu.unserialize();
      }();
      assertx(v.isArray());
      auto ad = v.detach().m_data.parr;
      ArrayData::GetScalarArray(&ad);
      auto const id DEBUG_ONLY = mergeUnitArray(ad);
      assertx(id == i);
    },
    [&] (auto& sd, const ArrayData* a) {
      auto const str = [&]{
        VariableSerializer vs{VariableSerializer::Type::Internal};
        vs.setUnitFilename(m_filepath);
        return
          vs.serializeValue(VarNR(const_cast<ArrayData*>(a)), false)
          .toCppString();
      }();
      sd(str);
    }
  );

  // HHBBC array types
  sd(m_arrayTypeTable);

  // Pre-class emitters
  seq(
    m_pceVec,
    [&] (auto& sd, size_t i) {
      std::string name;
      sd(name);
      auto pce = newPreClassEmitter(name);
      pce->serdeMetaData(sd);
      assertx(pce->id() == i);
    },
    [&] (auto& sd, PreClassEmitter* pce) {
      auto const nm = StripIdFromAnonymousClassName(pce->name()->slice());
      sd(nm.toString());
      pce->serdeMetaData(sd);
    }
  );

  // Record emitters
  seq(
    m_reVec,
    [&] (auto& sd, size_t i) {
      std::string name;
      sd(name);
      auto re = newRecordEmitter(name);
      re->serdeMetaData(sd);
      assertx(re->id() == i);
    },
    [&] (auto& sd, RecordEmitter* re) {
      auto const nm = StripIdFromAnonymousClassName(re->name()->slice());
      sd(nm.toString());
      re->serdeMetaData(sd);
    }
  );

  // Type aliases
  seq(
    m_typeAliases,
    [&] (auto& sd, size_t i) {
      std::string name;
      sd(name);
      auto te = newTypeAliasEmitter(name);
      te->serdeMetaData(sd);
      assertx(te->id() == i);
    },
    [&] (auto& sd, const std::unique_ptr<TypeAliasEmitter>& te) {
      sd(te->name()->toCppString());
      te->serdeMetaData(sd);
    }
  );

  // Constants
  seq(
    m_constants,
    [&] (auto& sd, size_t i) {
      Constant cns;
      sd(cns.name);
      sd(cns);
      if (type(cns.val) == KindOfUninit) {
        cns.val.m_data.pcnt = reinterpret_cast<MaybeCountable*>(Unit::getCns);
      }
      auto const id UNUSED = addConstant(cns);
      assertx(id == i);
    },
    [&] (auto& sd, const Constant& cns) {
      sd(cns.name);
      sd(cns);
    }
  );

  // Func emitters (we cannot use seq for these because they come from
  // both the pre-class emitters and m_fes.
  if constexpr (SerDe::deserializing) {
    size_t total;
    sd(total);
    for (size_t i = 0; i < total; ++i) {
      Id pceId;
      LowStringPtr name;
      sd(pceId);
      sd(name);

      FuncEmitter* fe;
      if (pceId < 0) {
        fe = newFuncEmitter(name);
      } else {
        auto funcPce = pce(pceId);
        fe = newMethodEmitter(name, funcPce);
        auto const added UNUSED = funcPce->addMethod(fe);
        assertx(added);
      }
      assertx(fe->sn() == i);
      fe->serde(sd, lazy);
      fe->setEHTabIsSorted();
      fe->finish();
    }
  } else {
    auto total = m_fes.size();
    for (auto const pce : m_pceVec) total += pce->methods().size();
    sd(total);

    auto const write = [&] (FuncEmitter* fe, Id pceId) {
      sd(pceId);
      sd(fe->name);
      fe->serde(sd, false);
    };
    for (auto const& fe : m_fes) write(fe.get(), -1);
    for (auto const pce : m_pceVec) {
      for (auto const fe : pce->methods()) write(fe, pce->id());
    }
  }
}

template void UnitEmitter::serde<>(BlobDecoder&, bool);
template void UnitEmitter::serde<>(BlobEncoder&, bool);

std::unique_ptr<UnitEmitter>
createFatalUnit(const StringData* filename, const SHA1& sha1, FatalOp op,
                std::string err, Location::Range loc) {
  auto ue = std::make_unique<UnitEmitter>(sha1, SHA1{}, Native::s_noNativeFuncs,
                                          false);
  ue->m_filepath = filename;

  ue->m_fatalUnit = true;
  ue->m_fatalLoc = loc;
  ue->m_fatalOp = op;
  ue->m_fatalMsg = err;

  return ue;
}

///////////////////////////////////////////////////////////////////////////////
}
