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

#ifndef incl_VM_FUNC_H_
#define incl_VM_FUNC_H_

#include "runtime/vm/bytecode.h"
#include "runtime/vm/type_constraint.h"

namespace HPHP {
namespace VM {

typedef TypedValue*(*BuiltinClassFunction)(ActRec* ar);

// Function.

class Func {
 public:
  struct ParamInfo { // Parameter default value info.
    Offset m_funcletOff; // If no default: InvalidAbsoluteOffset.
    const StringData* m_phpCode; // eval'able PHP code.
    TypeConstraint m_typeConstraint;
    bool hasDefaultValue() const {
      return m_funcletOff != InvalidAbsoluteOffset;
    }
  };
  struct SVInfo { // Static variable info.
    const StringData* name;
    const StringData* mangledName;
    const StringData* phpCode; // eval'able PHP or NULL if no default.
  };
  typedef TypedValue*(*BuiltinFunction)(ActRec* ar);

  Func(Unit& unit, Id id, const StringData* n);
  Func(const StringData* n, const ClassInfo::MethodInfo* info,
       BuiltinFunction funcPtr);
  Func(const StringData* n, const ClassInfo::MethodInfo* info,
       PreClass* preClass, BuiltinClassFunction funcPtr);
  Func(Unit& unit, const StringData* n, PreClass* preClass);
  ~Func();

  Id lookupVarId(const StringData* s) const;
  void allocVarId(const StringData* s);

  Id allocIterator() {
    ASSERT(m_numIterators >= m_nextFreeIterator);
    Id id = m_nextFreeIterator++;
    if (m_numIterators < m_nextFreeIterator) {
      m_numIterators = m_nextFreeIterator;
    }
    return id;
  }
  void freeIterator(Id id) {
    --m_nextFreeIterator;
    ASSERT(id == m_nextFreeIterator);
  }
  Id allocUnnamedLocal() {
    if (m_nextFreeUnnamedLocal < (int)m_unnamedLocals.size()) {
      return m_unnamedLocals[m_nextFreeUnnamedLocal++];
    } else {
      ASSERT(m_nextFreeUnnamedLocal == (int)m_unnamedLocals.size());
      m_unnamedLocals.push_back(newLocal());
      ++m_nextFreeUnnamedLocal;
      return m_unnamedLocals.back();
    }
  }
  void freeUnnamedLocal(Id id) {
    --m_nextFreeUnnamedLocal;
    ASSERT(id == m_unnamedLocals[m_nextFreeUnnamedLocal]);
  }

  int numSlotsInFrame() const {
    return m_numLocals + m_numIterators * kNumIterCells;
  }
  void init(const Location* sLoc, Offset base, Attr attrs, bool top,
            const StringData* docComment);
  void finish(Offset funclets, Offset past);

  void appendParam(const StringData* name, bool ref);
  EHEnt &addEHEnt();
  FEEnt &addFEEnt();
  FPIEnt &addFPIEnt();
  bool checkIterScope (Offset o, Id iterId) const;
  const FPIEnt* findFPI(Offset o) const;
  const EHEnt* findEH(Offset o) const;
  Offset findFaultPCFromEH(Offset o) const;

  // This can be thought of as "if I look up this Func's name while in fromUnit,
  // will I always get this Func back?" This is important for the translator: if
  // this condition holds, it allows for some translation-time optimizations by
  // making assumptions about where function calls will go.
  bool isNameBindingImmutable(const Unit* fromUnit) const;

  void setMaxStackCells(int cells) { m_maxStackCells = cells; }
  int  maxStackCells() const {
    // All non-builtins have to return something, which pushes at least 1 cell
    ASSERT(isBuiltin() || m_maxStackCells > 0);
    return m_maxStackCells;
  }

  bool byRef(int32 arg) const;
  bool mustBeRef(int32 arg) const;
  void prettyPrint(std::ostream &out) const;

  bool isPseudoMain() const { return m_name->empty(); }
  bool isBuiltin() const { return bool(m_info); }
  bool isDestructor() const {
    return !strcmp(m_name->data(), "__destruct");
  }

  void getFuncInfo(ClassInfo::MethodInfo* mi) const;

  Unit* m_unit;
  Id m_id;
  Offset m_base;
  Offset m_funclets;
  Offset m_past;
  int m_line1;
  int m_line2;
  const StringData* m_name;
  StringData* m_fullName;
  const ClassInfo::MethodInfo* m_info; // For builtins.
  PreClass* m_preClass;
  bool m_top;  // defined at top level

  std::vector<ParamInfo> m_params; // m_params[i] corresponds to parameter i.
  std::vector<EHEnt> m_ehtab;
  std::vector<FPIEnt> m_fpitab;
  std::vector<FEEnt> m_fetab;
  hphp_hash_map<const StringData*, Id,
    string_data_hash, string_data_same> m_name2pind;
  std::vector<const StringData*> m_pnames;
  std::vector<SVInfo> m_staticVars;
  Attr m_attrs;
  const StringData* m_docComment;
  Id m_numLocals;
  Id m_numIterators;
  Id m_nextFreeIterator;
  BuiltinFunction m_builtinFuncPtr;
  BuiltinClassFunction m_builtinClassFuncPtr;
  bool m_isClosureBody;
  bool m_isGenerator;
  bool m_needsStaticLocalCtx;
 public:
  int m_numParams; // Public for assembly access
  uint64_t* m_refBitVec; // Public for assembly access
#ifdef DEBUG
  static const int kMagic = 0xba5eba11;
  int m_magic; // For asserts only.
#endif
 private:
  void init();
  Id newLocal() { return m_numLocals++; }
  static const int kBitsPerQword = 64;

  std::vector<Id> m_unnamedLocals;
  int m_nextFreeUnnamedLocal;

 public:
  int m_maxStackCells;
  const inline Opcode *getEntry() const {
    return m_unit->entry() + m_base;
  }

  const char *name() const {
    if (m_name) {
      return m_name->data();
    }
    return NULL;
  }
};

} }

#endif
