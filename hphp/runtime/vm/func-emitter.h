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

#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/user-attributes.h"

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/type-constraint.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/unit-emitter.h"

#include <utility>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct PreClass;
struct StringData;

struct PreClassEmitter;

struct BlobDecoder;

namespace Native {
struct NativeFunctionInfo;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Bag of Func's fields used to emit Funcs.
 */
struct FuncEmitter {
  /////////////////////////////////////////////////////////////////////////////
  // Types.

  using UpperBoundVec = TypeIntersectionConstraint;
  using UpperBoundMap = std::unordered_map<const StringData*, UpperBoundVec>;
  struct ParamInfo : public Func::ParamInfo {
    ParamInfo() {}

    template<class SerDe>
    void serde(SerDe& sd) {
      Func::ParamInfo* parent = this;
      parent->serde(sd);
      sd(upperBounds);
    }

    UpperBoundVec upperBounds;
  };

  using ParamInfoVec = std::vector<ParamInfo>;
  using EHEntVec = std::vector<EHEnt>;

  using CoeffectRuleVec = std::vector<CoeffectRule>;
  using StaticCoeffectsVec = std::vector<LowStringPtr>;

  /////////////////////////////////////////////////////////////////////////////
  // Initialization and execution.

  FuncEmitter(UnitEmitter& ue, int sn, Id id, const StringData* n);
  FuncEmitter(UnitEmitter& ue, int sn, const StringData* n,
              PreClassEmitter* pce);
  ~FuncEmitter();

  /*
   * Just set some fields when we start and stop emitting.
   */
  void init(int l1, int l2, Attr attrs_,
            const StringData* docComment_);
  void finish();

  /*
   * Instantiate a runtime Func*.
   */
  Func* create(Unit& unit, PreClass* preClass = nullptr) const;

  /////////////////////////////////////////////////////////////////////////////
  // Serialization.

  /////////////////////////////////////////////////////////////////////////////
  // Serialization.

  template<class SerDe> void serdeMetaData(SerDe&);
  template<class SerDe> void serde(SerDe&, bool lazy);

  // Deserializing just a LineTable, previously encoded by serde (the
  // BlobDecoder must be at the correct offset).
  static void deserializeLineTable(BlobDecoder&, LineTable&);

  // Load a line table out of the unit given by the SN at the given
  // token. The token is the offset of the line table within the unit.
  static LineTable loadLineTableFromRepo(int64_t, RepoFile::Token);

  /////////////////////////////////////////////////////////////////////////////
  // Metadata.

  /*
   * Get the associated Unit and PreClass emitters.
   */
  UnitEmitter& ue() const;
  PreClassEmitter* pce() const;

  /*
   * XXX: What are these for?
   */
  int sn() const;
  Id id() const;

  /////////////////////////////////////////////////////////////////////////////
  // Locals, iterators, and parameters.

  /*
   * Count things.
   */
  Id numLocals() const;
  Id numNamedLocals() const;
  Id numIterators() const;
  Id numLiveIterators() const;

  /*
   * Set things.
   */
  void setNumIterators(Id numIterators);
  void setNumLiveIterators(Id id);

  /*
   * Check existence of, look up, and allocate named locals.
   */
  bool hasVar(const StringData* name) const;
  Id lookupVarId(const StringData* name) const;
  void allocVarId(const StringData* name, bool slotless = false);

  /*
   * Allocate unnamed locals.
   */
  Id allocUnnamedLocal();

  /*
   * Allocate and free iterators.
   */
  Id allocIterator();
  void freeIterator(Id id);

  /*
   * Add a parameter and corresponding named local.
   */
  void appendParam(const StringData* name, const ParamInfo& info);

  /*
   * Get the local variable name -> id map.
   */
  const Func::NamedLocalsMap::Builder& localNameMap() const;


  /////////////////////////////////////////////////////////////////////////////
  // Unit tables.

  /*
   * Add entries to the EH table, and return them by reference.
   */
  EHEnt& addEHEnt();

private:
  /*
   * Private table sort routines; called at finish()-time.
   */
  void sortEHTab();

public:
  /*
   * Declare that the EH table was created in sort-order and doesn't need to be
   * resorted at finish() time.
   */
  void setEHTabIsSorted();

  /////////////////////////////////////////////////////////////////////////////
  // Helper accessors.                                                  [const]

  /*
   * Is the function a method, variadic (i.e., takes a `...'
   * parameter), or an HNI function with a native implementation?
   */
  bool isMethod() const;
  bool isVariadic() const;

  /*
   * @returns: std::make_pair(line1, line2)
   */
  std::pair<int,int> getLocation() const;

  Native::NativeFunctionInfo getNativeInfo() const;
  String nativeFullname() const;

  /////////////////////////////////////////////////////////////////////////////
  // Complex setters.
  //

  /*
   * Shorthand for setting `line1' and `line2' because typing is hard.
   */
  void setLocation(int l1, int l2);

  /*
   * Pulls native and system attributes out of the user attributes map.
   *
   * System attributes are returned by reference through `attrs_', and native
   * attributes are returned as an integer.
   */
  int parseNativeAttributes(Attr& attrs_) const;

  /////////////////////////////////////////////////////////////////////////////
  // Bytecode.

  Offset offsetOf(const unsigned char* pc) const;

  /*
   * Bytecode pointer and current emit position.
   */
  const unsigned char* bc() const;
  Offset bcPos() const;

  /*
   * Set the bytecode pointer by allocating a copy of `bc' with size `bclen'.
   *
   * Not safe to call with m_bc as the argument because we free our current
   * bytecode stream before allocating a copy of `bc'.
   */
  void setBc(const unsigned char* bc, size_t bclen);
  void setBcToken(Func::BCPtr::Token token, size_t bclen);
  Optional<Func::BCPtr::Token> loadBc();

  /////////////////////////////////////////////////////////////////////////////
  // Bytecode emit.
  //
  // These methods emit values to bc() at bcPos() (or pos, if given) and then
  // update bcPos(), realloc-ing the bytecode region if necessary.

  void emitOp(Op op);
  void emitByte(unsigned char n, int64_t pos = -1);

  void emitInt16(uint16_t n, int64_t pos = -1);
  void emitInt32(int n, int64_t pos = -1);
  void emitInt64(int64_t n, int64_t pos = -1);
  void emitDouble(double n, int64_t pos = -1);

  void emitIVA(bool) = delete;
  template<typename T> void emitIVA(T n);

  void emitNamedLocal(NamedLocal loc);

 private:
  /*
   * Bytecode emit implementation.
   */
  template<class T>
  void emitImpl(T n, int64_t pos);

  /////////////////////////////////////////////////////////////////////////////
  // Source locations.

 public:
  /*
   * Return a copy of the SrcLocTable for the Func, if it has one; otherwise,
   * return an empty table.
   */
  SourceLocTable createSourceLocTable() const;

  /*
   * Does this Func contain full source location information?
   *
   * Generally, FuncEmitters loaded from a production repo will have a
   * LineTable only instead of a full SourceLocTable.
   */
  bool hasSourceLocInfo() const;

  /*
   * Const reference to the Func's LineTable.
   */
  const LineTable& lineTable() const;

  /*
   * Record source location information for the last chunk of bytecode added to
   * this FuncEmitter.
   *
   * Adjacent regions associated with the same source line will be collapsed as
   * this is created.
   */
  void recordSourceLocation(const Location::Range& sLoc, Offset start);

  /////////////////////////////////////////////////////////////////////////////
  // Data members.

private:
  // Initial bytecode size.
  static const size_t BCMaxInit = 64;

  /*
   * Metadata.
   */
  UnitEmitter& m_ue;
  PreClassEmitter* m_pce;

  int m_sn;
  Id m_id;

  Func::BCPtr m_bc;
  size_t m_bclen;
  size_t m_bcmax;

public:
  /*
   * Func fields.
   */
  int line1;
  int line2;
  LowStringPtr name;
  Attr attrs;

  ParamInfoVec params;
  int16_t maxStackCells{0};

  TypeConstraint retTypeConstraint;
  LowStringPtr retUserType;
  UpperBoundVec retUpperBounds;
  StaticCoeffectsVec staticCoeffects;
  CoeffectRuleVec coeffectRules;

  EHEntVec ehtab;

  union {
    uint16_t m_repoBoolBitset{0};
    struct {
      bool isMemoizeWrapper    : 1;
      bool isMemoizeWrapperLSB : 1;
      bool isClosureBody       : 1;
      bool isAsync             : 1;
      bool containsCalls       : 1;
      bool isNative            : 1;
      bool isGenerator         : 1;
      bool isPairGenerator     : 1;
      bool hasParamsWithMultiUBs : 1;
      bool hasReturnWithMultiUBs : 1;
      bool requiresFromOriginalModule : 1;
    };
  };

  LowStringPtr docComment;
  LowStringPtr originalFilename;
  LowStringPtr originalModuleName;

  UserAttributeMap userAttributes;

  StringData* memoizePropName;
  StringData* memoizeGuardPropName;
  int memoizeSharedPropIndex;
  RepoAuthType repoReturnType;
  RepoAuthType repoAwaitedReturnType;

private:
  /*
   * FuncEmitter-managed state.
   */
  Func::NamedLocalsMap::Builder m_localNames;
  Id m_numLocals;
  int m_numUnnamedLocals;
  Id m_numIterators;
  Id m_nextFreeIterator;
  bool m_ehTabSorted : 1;

  /*
   * Source location tables.
   *
   * Each entry encodes an open-closed range of bytecode offsets.
   *
   * The m_sourceLocTab is keyed by the start of each half-open range.  This is
   * to allow appending new bytecode offsets that are part of the same range to
   * coalesce.
   *
   * The m_lineTable is keyed by the past-the-end offset.  This is the
   * format we'll want it in when we go to create a Unit.
   */

  void setLineTable(LineTable);

  std::vector<std::pair<Offset,SourceLoc>> m_sourceLocTab;
  Func::LineTablePtr m_lineTable;
};

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_VM_FUNC_EMITTER_INL_H_
#include "hphp/runtime/vm/func-emitter-inl.h"
#undef incl_HPHP_VM_FUNC_EMITTER_INL_H_
