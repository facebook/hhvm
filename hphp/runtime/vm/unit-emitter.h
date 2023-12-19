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

#pragma once

#include <list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "hphp/runtime/base/location.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/constant.h"
#include "hphp/runtime/vm/decl-dep.h"
#include "hphp/runtime/vm/module.h"
#include "hphp/runtime/vm/preclass.h"
#include "hphp/runtime/vm/repo-file.h"
#include "hphp/runtime/vm/type-alias.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/util/functional.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/hash-set.h"
#include "hphp/util/sha1.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct FuncEmitter;
struct PreClassEmitter;
struct StringData;
struct TypeAliasEmitter;
struct Extension;

namespace Native {
struct FuncTable;
}

/*
 * Whether we need to keep the extended line table (for debugging, or
 * dumping to hhas).
 */
bool needs_extended_line_table();

///////////////////////////////////////////////////////////////////////////////

/*
 * Pre-runtime representation of Unit used to emit bytecode and instantiate
 * runtime Units.
 */
struct UnitEmitter {
  /////////////////////////////////////////////////////////////////////////////
  // Initialization and execution.

  explicit UnitEmitter(const SHA1& sha1,
                       const SHA1& bcSha1,
                       const PackageInfo&);
  UnitEmitter(UnitEmitter&&) = delete;
  ~UnitEmitter();

  void setSha1(const SHA1& sha1) { m_sha1 = sha1; }

  /*
   * Instatiate a runtime Unit*.
   */
  std::unique_ptr<Unit> create() const;

  template<typename SerDe> void serde(SerDe&, bool lazy);

  /*
   * Run the verifier on this unit.
   */
  bool check(bool verbose) const;

  /////////////////////////////////////////////////////////////////////////////
  // Basic data.

  /*
   * The SHA1 hash of the source for Unit.
   */
  const SHA1& sha1() const;

  /*
   * The SHA1 hash of the bytecode for Unit.
   */
  const SHA1& bcSha1() const;


  /////////////////////////////////////////////////////////////////////////////
  // Litstrs and Arrays.

  /*
   * Look up a static string or array/arraytype by ID. This might load
   * the data from the repo if lazy loading is enabled.
   */
  const StringData* lookupLitstr(Id id) const;
  const ArrayData* lookupArray(Id id) const;
  const RepoAuthType::Array* lookupRATArray(Id id) const;

  /*
   * Like the above lookup functions, but create ref-counted copies,
   * and don't cache the result. This is meant for things like the
   * verifier, where we want to lookup the values, but not keep them
   * around.
   */
  String lookupLitstrCopy(Id id) const;
  Array lookupArrayCopy(Id id) const;

  Id numArrays() const { return m_arrays.size(); }
  Id numLitstrs() const { return m_litstrs.size(); }

  /*
   * Merge a literal string into the Unit.
   */
  Id mergeLitstr(const StringData*);

  /*
   * Merge a scalar array into the Unit.
   */
  Id mergeArray(const ArrayData*);

  /*
   * Merge a RAT array into the Unit.
   */
  Id mergeRATArray(const RepoAuthType::Array*);

  /*
   * Load literal array or strings from the repo. The data is loaded
   * from the unit given by the SN, at the location given by the
   * token. The token should be what was calculated when the
   * unit-emitter was created.
   */
  static const ArrayData* loadLitarrayFromRepo(int64_t unitSn,
                                               RepoFile::Token token,
                                               bool makeStatic);
  static const StringData* loadLitstrFromRepo(int64_t unitSn,
                                              RepoFile::Token token,
                                              bool makeStatic);
  static const RepoAuthType::Array* loadRATArrayFromRepo(int64_t unitSn,
                                                         RepoFile::Token token);

  /////////////////////////////////////////////////////////////////////////////
  // FuncEmitters.

  /*
   * Const reference to all of the Unit's FuncEmitters.
   */
  auto const& fevec() const;

  /*
   * Create a new FuncEmitter and add it to the FE vector.
   */
  FuncEmitter* newFuncEmitter(const StringData* name, int64_t sn = -1);

  /*
   * Create a new FuncEmitter for the method given by `name' and `pce'.
   *
   * Does /not/ add it to the FE vector.
   */
  FuncEmitter* newMethodEmitter(const StringData* name, PreClassEmitter* pce, int64_t sn = -1);

  /*
   * Create a new function for `fe'.
   *
   * This should only be called from fe->create(), and just constructs a new
   * Func* and adds it to unit.m_funcTable if required.
   */
  Func* newFunc(const FuncEmitter* fe, Unit& unit, const StringData* name,
                Attr attrs, int numParams);


  /////////////////////////////////////////////////////////////////////////////
  // PreClassEmitters.

  /*
   * Number of PreClassEmitters in the Unit.
   */
  size_t numPreClasses() const;

  /*
   * All PreClassEmitters in the Unit.
   */
  folly::Range<PreClassEmitter* const*> preclasses() const;

  /*
   * Create a new PreClassEmitter and add it to all the PCE data structures.
   */
  PreClassEmitter* newPreClassEmitter(const std::string& name);

  /////////////////////////////////////////////////////////////////////////////
  // Type aliases.

  /*
   * Const reference to all of the Unit's type aliases.
   */
  auto const& typeAliases() const;

  /*
   * Add a new type alias to the Unit.
   */
  TypeAliasEmitter* newTypeAliasEmitter(const std::string& name);

  /////////////////////////////////////////////////////////////////////////////
  // Constants.

  /*
   * Reference to all of the Unit's type aliases.
   */
  std::vector<Constant>& constants();
  const std::vector<Constant>& constants() const;

  /*
   * Add a new constant to the Unit.
   */
  Id addConstant(const Constant& c);

  /////////////////////////////////////////////////////////////////////////////
  // Modules.

  /*
   * Reference to all of the Unit's modules.
   */
  std::vector<Module>& modules();
  const std::vector<Module>& modules() const;

  /*
   * Add a new module to the Unit.
   */
  Id addModule(const Module&);

  /////////////////////////////////////////////////////////////////////////////
  // Other methods.

  /*
   * Is this a Unit for a systemlib?
   */
  bool isASystemLib() const;

  /*
   * Use StructuredLog to record decl related information about this unit.
   */
  void logDeclInfo() const;

  /////////////////////////////////////////////////////////////////////////////
  // EntryPoint.

  void finish();

  void setEntryPointIdCalculated();

  Id getEntryPointId() const;

  /////////////////////////////////////////////////////////////////////////////
  // Package info.

  const PackageInfo& getPackageInfo() const;

private:
  void calculateEntryPointId();

  static const ArrayData* loadLitarrayFromPtr(const char*, size_t);

  /////////////////////////////////////////////////////////////////////////////
  // Data members.

public:
  int64_t m_sn{-1};
  const StringData* m_filepath{nullptr};

  bool m_softDeployedRepoOnly{false}; // is it part of a soft package?
  bool m_ICE{false}; // internal compiler error
  bool m_fatalUnit{false}; // parse/runtime error
  UserAttributeMap m_metaData;
  UserAttributeMap m_fileAttributes;

  // A list of dependencies queried by the frontend while compiling
  // this unit.
  std::vector<DeclDep> m_deps;

  // A list of symbols which will be required by this unit at runtime.
  SymbolRefs m_symbol_refs;

  /*
   * Extension this unit is part of. Can be used to get the native func impl
   */
  const Extension* m_extension{nullptr};

  Location::Range m_fatalLoc;
  FatalOp m_fatalOp;
  std::string m_fatalMsg;
  const StringData* m_moduleName{makeStaticString(Module::DEFAULT)};
  PackageInfo m_packageInfo;

  /*
   * HackC may report a list of referenced symbols which either could not be
   * looked up or did not exist while compiling the unit.
   *
   * These are initialized when either StressShallowDeclDeps or
   * StressFoldedDeclDeps is true during compilation.
   */
  std::vector<const StringData*> m_missingSyms;
  std::vector<const StringData*> m_errorSyms;

private:
  SHA1 m_sha1;
  SHA1 m_bcSha1;

  int m_nextFuncSn;

  /*
   * Litstr tables.
   */
  hphp_fast_map<const StringData*, Id> m_litstr2id;
  mutable std::vector<UnsafeLockFreePtrWrapper<StringOrToken>> m_litstrs;

  /*
   * Scalar array tables.
   */
  hphp_fast_map<const ArrayData*, Id> m_array2id;
  mutable std::vector<UnsafeLockFreePtrWrapper<ArrayOrToken>> m_arrays;

  /*
   * Repo-auth-type arrays.
   */
  hphp_fast_map<const RepoAuthType::Array*, Id> m_rat2id;
  mutable std::vector<UnsafeLockFreePtrWrapper<RATArrayOrToken>> m_rats;

  /*
   * Type alias table.
   */
  std::vector<std::unique_ptr<TypeAliasEmitter>> m_typeAliases;

  /*
   * Constants table.
   */
  std::vector<Constant> m_constants;

  /*
   * Modules table.
   */
  std::vector<Module> m_modules;

  /*
   * FuncEmitter tables.
   */
  std::vector<std::unique_ptr<FuncEmitter> > m_fes;

  /*
   * PreClassEmitter table.
   */
  std::vector<PreClassEmitter*> m_pceVec;

  mutable std::mutex m_verifyLock;

  Id m_entryPointId{kInvalidId};

  bool m_entryPointIdCalculated{false};

  // When deserializing, used to provide the location of raw array
  // data to lookupArray.
  const char* m_litarrayBuffer{nullptr};
  size_t m_litarrayBufferSize{0};
};

///////////////////////////////////////////////////////////////////////////////

// UnitEmitter's serde implementation does not serialize all of the
// UnitEmitter's data (it is sometimes stored elsewhere). This is a
// wrapper around a UnitEmitter allowing for complete stand-alone
// serializating and deserializing.
struct UnitEmitterSerdeWrapper {
  UnitEmitterSerdeWrapper() = default;
  /* implicit */ UnitEmitterSerdeWrapper(std::unique_ptr<UnitEmitter> ue)
      : m_ue{std::move(ue)} {}
  std::unique_ptr<UnitEmitter> m_ue;
  template <typename SerDe> void serde(SerDe& sd);
};

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<UnitEmitter> createFatalUnit(
  const StringData* filename,
  const SHA1& sha1,
  FatalOp op,
  std::string err,
  Location::Range loc = {-1,-1,-1,-1}
);

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_VM_UNIT_EMITTER_INL_H_
#include "hphp/runtime/vm/unit-emitter-inl.h"
#undef incl_HPHP_VM_UNIT_EMITTER_INL_H_
