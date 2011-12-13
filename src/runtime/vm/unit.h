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

#ifndef incl_VM_UNIT_H_
#define incl_VM_UNIT_H_

// Expects that runtime/vm/core_types.h is already included.
#include "runtime/vm/hhbc.h"
#include "runtime/vm/class.h"
#include "runtime/base/array/hphp_array.h"
#include "util/parser/location.h"
#include "runtime/base/md5.h"

namespace HPHP {
namespace VM {

typedef const uchar* PC;

// Forward declarations.
class Func;
struct ActRec;

// Exception handler table entry.
class EHEnt {
public:
  enum EHType {
    EHType_Catch,
    EHType_Fault
  };
  EHType m_ehtype;
  Offset m_base;
  Offset m_past;
  int m_parentIndex;
  Offset m_fault;
  typedef std::vector<std::pair<Id, Offset> > CatchVec;
  CatchVec m_catches;
};

class EHEntComp {
public:
  bool operator() (const EHEnt &eh1, const EHEnt &eh2) {
    if (eh1.m_base == eh2.m_base) {
      // for identical address ranges, Catch funclet is "outer" to Fault funclet
      if (eh1.m_past == eh2.m_past) {
        return eh1.m_ehtype == EHEnt::EHType_Catch;
      }
      return eh1.m_past > eh2.m_past;
    }
    return eh1.m_base < eh2.m_base;
  }
};

// Function paramater info region table entry.
class FPIEnt {
public:
  Offset m_base;
  Offset m_past;
  Offset m_fpOff;
  int m_parentIndex;
  int m_fpiDepth;
};

class FPIEntComp {
public:
  bool operator() (const FPIEnt &fpi1, const FPIEnt &fpi2) {
    return fpi1.m_base < fpi2.m_base;
  }
};

// Foreach region.
class FEEnt {
public:
  Id m_iterId;
  Offset m_base;
  Offset m_past;
  int m_parentIndex;
};

class FEEntComp {
public:
  bool operator() (const FEEnt &fe1, const FEEnt &fe2) {
    return fe1.m_base < fe2.m_base;
  }
};

class SourceLoc {
public:
  SourceLoc() : line0(1), char0(1), line1(1), char1(1) {}

  int line0;
  int char0;
  int line1;
  int char1;

  void setLoc(const Location *l) {
    line0 = l->line0;
    char0 = l->char0;
    line1 = l->line1;
    char1 = l->char1;
  }

  bool same(const SourceLoc *l) const {
    return (this == l) ||
           (line0 == l->line0 && char0 == l->char0 &&
            line1 == l->line1 && char1 == l->char1);
  }

  bool operator==(const SourceLoc &l) const {
    return same(&l);
  }
};

template<typename T>
struct IntervalMapEntry {
  Offset startOffset;
  Offset endOffset;
  T val;
};

typedef IntervalMapEntry<SourceLoc> SourceLocEntry;
typedef IntervalMapEntry<Func*> FuncEntry;

// Variable environment.
class VarEnv {
private:
  // A variable environment consists of the locals for the current function
  // (either pseudo-main or a normal function), plus any variables that are
  // dynamically defined.  A normal (not pseudo-main) function starts off with
  // a variable environment that contains only its locals, but a pseudo-main is
  // handed its caller's existing variable environment.  We want local variable
  // access to be fast for pseudo-mains, but we must maintain consistency with
  // the associated variable environment.
  //
  // We achieve a consistent variable environment without sacrificing locals
  // access performance by overlaying each pseudo-main's locals onto the
  // current variable environment, so that at any given time, a variable in the
  // variable environment has a canonical location (either a local variable on
  // the stack, or a dynamically defined variable), which can only change at
  // entry/exit of a pseudo-main.
  ActRec* m_cfp;
  unsigned m_depth;
  HphpArray* m_name2info;
  std::vector<TypedValue**> m_restoreLocations;

  TypedValue* m_extraArgs;
  unsigned m_numExtraArgs;
  bool m_isGlobalScope;

public:
  VarEnv(bool isGlobalScope = false, bool skipInsert = false);
  ~VarEnv();

  void attach(ActRec* fp);
  void lazyAttach(ActRec* fp);
  void detach(ActRec* fp);

  void set(const StringData* name, TypedValue* tv);
  void bind(const StringData* name, TypedValue* tv);
  TypedValue* lookup(const StringData* name);
  bool unset(const StringData* name);

  void setExtraArgs(TypedValue* args, unsigned nargs);
  unsigned numExtraArgs() const;
  TypedValue* getExtraArg(unsigned argInd) const;

  Array getDefinedVariables() const;

  // Used for save/store m_cfp for debugger
  void setCfp(ActRec* fp) { m_cfp = fp; }
  ActRec* getCfp() const { return m_cfp; }
};

//==============================================================================
// (const StringData*) versus (StringData*)
//
// The HHBC VM uses StringData objects to implement litstr's.  These immutable
// literal strings may be used repeatedly, for example via the PushString
// operation.  The Unit machinery maintains a map of all litstr's for the
// lifetime of the program in s_Litstrs, which "owns" the strings.  Note that
// s_Litstrs contains (StringData*), whereas every other data structure in Func
// and Unit contains (const StringData*).  These const fields are able to avoid
// owning references, because m_litstrs will survive long enough that the const
// fields will always refer to valid strings.  The use of (const StringData*)
// has a ripple effect on the methods provided by Func and Unit, though this is
// mainly an implementation detail.
//
//==============================================================================

// Compilation unit.
class Unit {
  static const size_t BCMaxInit = 4096; // Initial bytecode size.
  int    m_writeDepth;
  void protect() const;
  void unprotect() const;
public:
  uchar* m_bc;
  size_t m_bclen;
  size_t m_bcmax;
  MD5 m_md5;
  const StringData* m_filepath;
  const StringData* m_dirpath;
  static int bcOff() { return offsetof(Unit, m_bc); }

  class BCWriteToken {
    Unit& m_u;
    BCWriteToken(Unit& u);
    ~BCWriteToken();
  };
  Unit(const Location* sLoc);
  ~Unit();

  Id mergeLitstr(StringData* litstr, bool isAnon=false);
  StringData* lookupLitstrStr(StringData* litstr);
  StringData* lookupLitstrStr(const std::string& str);
  StringData* lookupLitstrId(Id id) const;

  static ArrayData* mergeAnonArray(ArrayData* a, const StringData* key=NULL);
  Id mergeArray(ArrayData* a);
  ArrayData* lookupArrayId(Id id) const;

  std::string toString() const;

  void recordSourceLocation(const Location *sLoc, Offset start, Offset end);
  void recordFunction(Func *func);
  int getLineNumber(Offset pc) const;
  const SourceLoc* getSourceLoc(Offset pc);
  Func* getFunc(Offset pc) const;

  void emitOp(Op op, int64 pos = -1) {
    emitByte((uchar)op, pos);
  }
  void emitByte(uchar n, int64 pos = -1) {
    emitImpl(n, pos);
  }
  void emitInt32(int n, int64 pos = -1) {
    emitImpl(n, pos);
  }
  void emitInt64(int64 n, int64 pos = -1) {
    emitImpl(n, pos);
  }
  void emitDouble(double n, int64 pos = -1) {
    emitImpl(n, pos);
  }

  Offset bcPos() const { return (Offset)m_bclen; }

  void close();
  uchar* allocNewBytecode() const {
    return (uchar*) malloc(m_bcmax);
  }
  void replaceBc(uchar* bc, size_t bclen);

  static void dumpUnit(Unit* unit);
  void prettyPrint(std::ostream &out) const;
  PreClass* newPreClass(StringData* n, Attr attrs, StringData* parent,
                        StringData* docComment, const Location* sLoc, Offset o,
                        bool hoistable);
  Func* newFunc(StringData* n, bool top);
  Func *getMain()                     { return m_main; }
  const PC entry() const              { return m_bc; }
  const PC at(const Offset off) const {
    ASSERT(off < Offset(m_bclen));
    return m_bc + off;
  }
  const Offset offsetOf(const Opcode* op) const {
    ASSERT(op >= m_bc && op < (m_bc + m_bclen));
    return op - m_bc;
  }
  void setMd5(CStrRef md5) { m_md5 = MD5(md5); }
  MD5 md5() const { return m_md5; }

  Class* defClass(PreClass* preClass, bool failIsFatal);

  typedef hphp_hash_set<StringData*, string_data_hash,
                        string_data_isame> HoistedPreClassSet;
  HoistedPreClassSet m_hoistablePreClassSet;
  std::vector<PreClass*> m_hoistablePreClassVec;
  std::vector<PreClassPtr> m_preClasses;
  std::vector<Func*> m_funcs;
  typedef hphp_hash_map<StringData*, Func*, string_data_hash,
                        string_data_isame> FuncMap;
  FuncMap m_funcMap;
  hphp_hash_map<const StringData*, Id,
                string_data_hash, string_data_same> m_litstr2id;
  std::vector<const StringData*> m_litstrs;
  hphp_hash_map<const StringData*, Id,
                string_data_hash, string_data_same> m_array2id;
  std::vector<ArrayData*> m_arrays;

  // m_lineRuns[i] is the first bytecode address *after* the end of the i'th
  // line run. A line run is a range of consecutive bytecode that comes from the
  // same source line. So if m_lineRuns[0] = 20, the bytecode from 0-19
  // (inclusive) is from the same source line. The source line number is in
  // m_lines[0].
  //
  // m_lineRuns is in nondecreasing order, so it can be binary-searched.
  std::vector<Offset> m_lineRuns;
  std::vector<int> m_lines;

  std::map<Offset, SourceLocEntry> m_sourceLocTable;
  std::map<Offset, FuncEntry> m_funcTable;
  Func *m_main;
  typedef hphp_hash_map<const StringData*, std::vector<ClassPtr>,
                        string_data_hash, string_data_isame> ClassMap;
  ClassMap m_classes;
  Mutex m_classesMutex;
public:

  template<class T>
  void emitImpl(T n, int64 pos) {
    uchar *c = (uchar*)&n;
    if (pos == - 1) {
      // Make sure m_bc is large enough.
      while (m_bclen + sizeof(T) > m_bcmax) {
        m_bc = (uchar*)realloc(m_bc, m_bcmax << 1);
        m_bcmax <<= 1;
      }
      memcpy(&m_bc[m_bclen], c, sizeof(T));
      m_bclen += sizeof(T);
    } else {
      ASSERT(pos + sizeof(T) <= m_bcmax);
      for (uint i = 0; i < sizeof(T); ++i) {
        m_bc[pos + i] = c[i];
      }
    }
  }
};

// hphp_compiler_parse() is defined in the compiler, but we must use
// dlsym() to get at it. CompileStringFn matches its signature.
typedef Unit*(*CompileStringFn)(const char*, int, const char*);

} } // HPHP::VM
#endif
