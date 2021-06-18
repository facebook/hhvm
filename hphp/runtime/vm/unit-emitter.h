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

#include "hphp/parser/location.h"

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/vm/constant.h"
#include "hphp/runtime/vm/preclass.h"
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
struct RecordEmitter;
struct StringData;
struct TypeAliasEmitter;

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
                       const Native::FuncTable&,
                       bool useGlobalIds);
  UnitEmitter(UnitEmitter&&) = delete;
  ~UnitEmitter();

  void setSha1(const SHA1& sha1) { m_sha1 = sha1; }

  /*
   * Instatiate a runtime Unit*.
   */
  std::unique_ptr<Unit> create() const;

  template<typename SerDe> void serdeMetaData(SerDe&);
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
   * Look up a static string or array/arraytype by ID.
   */
  const StringData* lookupLitstr(Id id) const;
  const ArrayData* lookupArray(Id id) const;
  const RepoAuthType::Array* lookupArrayType(Id id) const;

  Id numArrays() const { return m_arrays.size(); }
  Id numLitstrs() const { return m_litstrs.size(); }

  bool useGlobalIds() const { return m_useGlobalIds; }
  /*
   * Merge a literal string into either the global LitstrTable or the table for
   * the Unit.
   */
  Id mergeLitstr(const StringData* litstr);

  /*
   * Merge a literal string into the table for the Unit.
   */
  Id mergeUnitLitstr(const StringData* litstr);

  /*
   * Merge a scalar array into the Unit.
   */
  Id mergeArray(const ArrayData* a);

  /*
   * Merge a scalar array into the table for the Unit.
   */
  Id mergeUnitArray(const ArrayData* a);

  /*
   * Clear and rebuild the array type table from the builder.
   */
  void repopulateArrayTypeTable(const ArrayTypeTable::Builder&);

  /////////////////////////////////////////////////////////////////////////////
  // FuncEmitters.

  /*
   * Const reference to all of the Unit's FuncEmitters.
   */
  auto const& fevec() const;

  /*
   * Create a new FuncEmitter and add it to the FE vector.
   */
  FuncEmitter* newFuncEmitter(const StringData* name);

  /*
   * Create a new FuncEmitter for the method given by `name' and `pce'.
   *
   * Does /not/ add it to the FE vector.
   */
  FuncEmitter* newMethodEmitter(const StringData* name, PreClassEmitter* pce);

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
   * The PreClassEmitter for `preClassId'.
   */
  const PreClassEmitter* pce(Id preClassId) const;
  PreClassEmitter* pce(Id preClassId);

  /*
   * The id for the pre-class named clsName, or -1 if
   * there is no such pre-class
   */
  Id pceId(folly::StringPiece clsName);

  /*
   * Create a new PreClassEmitter and add it to all the PCE data structures.
   */
  PreClassEmitter* newPreClassEmitter(const std::string& name);

  RecordEmitter* newRecordEmitter(const std::string& name);

  /////////////////////////////////////////////////////////////////////////////
  // RecordEmitters.

  /*
   * Number of RecordEmitters in the Unit.
   */
  size_t numRecords() const;

  /*
   * The RecordEmitter for `recordId'.
   */
  const RecordEmitter* re(Id recordId) const;
  RecordEmitter* re(Id recordId);

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
  // Other methods.

  /*
   * Is this a Unit for a systemlib?
   */
  bool isASystemLib() const;

  /////////////////////////////////////////////////////////////////////////////
  // Data members.

public:
  int64_t m_sn{-1};
  const StringData* m_filepath{nullptr};

  bool m_ICE{false}; // internal compiler error
  bool m_useGlobalIds{0};
  bool m_fatalUnit{false}; // parse/runtime error
  UserAttributeMap m_metaData;
  UserAttributeMap m_fileAttributes;
  SymbolRefs m_symbol_refs;
  /*
   * name=>NativeFuncInfo for native funcs in this unit
   */
  const Native::FuncTable& m_nativeFuncs;

  Location::Range m_fatalLoc;
  FatalOp m_fatalOp;
  std::string m_fatalMsg;

private:
  SHA1 m_sha1;
  SHA1 m_bcSha1;

  int m_nextFuncSn;

  /*
   * Litstr tables.
   */
  hphp_hash_map<const StringData*, Id,
                string_data_hash, string_data_same> m_litstr2id;
  std::vector<const StringData*> m_litstrs;

  /*
   * Scalar array tables.
   */
  hphp_hash_map<const ArrayData*, Id> m_array2id;
  std::vector<const ArrayData*> m_arrays;

  /*
   * Unit local array type table.
   */
  ArrayTypeTable m_arrayTypeTable;

  /*
   * Type alias table.
   */
  std::vector<std::unique_ptr<TypeAliasEmitter>> m_typeAliases;

  /*
   * Constants table.
   */
  std::vector<Constant> m_constants;

  /*
   * FuncEmitter tables.
   */
  std::vector<std::unique_ptr<FuncEmitter> > m_fes;

  /*
   * PreClassEmitter table.
   */
  std::vector<PreClassEmitter*> m_pceVec;

  /*
   * RecordEmitter table.
   */
  std::vector<RecordEmitter*> m_reVec;

  mutable std::mutex m_verifyLock;
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
