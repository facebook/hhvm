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

#ifndef __COMPILER_EMITTER_H__
#define __COMPILER_EMITTER_H__

#include <compiler/expression/expression.h>
#include <compiler/statement/statement.h>
#include <compiler/statement/use_trait_statement.h>
#include <compiler/statement/trait_prec_statement.h>
#include <compiler/statement/trait_alias_statement.h>

#include <runtime/vm/func.h>
#include <runtime/vm/unit.h>
#include <util/hash.h>

namespace HPHP {

DECLARE_BOOST_TYPES(ClosureExpression);
DECLARE_BOOST_TYPES(MethodStatement);
DECLARE_BOOST_TYPES(InterfaceStatement);
DECLARE_BOOST_TYPES(ListAssignment);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(FileScope);
DECLARE_BOOST_TYPES(FunctionCall);
DECLARE_BOOST_TYPES(SimpleFunctionCall);
DECLARE_BOOST_TYPES(SwitchStatement);
DECLARE_BOOST_TYPES(ForEachStatement);
class StaticClassName;

namespace Compiler {
///////////////////////////////////////////////////////////////////////////////

using VM::Offset;
using VM::Func;
using VM::Class;
using VM::Unit;
using VM::InvalidAbsoluteOffset;
using VM::Opcode;
using VM::Id;

using namespace VM;

// Forward declarations.
class Label;
class EmitterVisitor;

// Helper for creating unit MetaInfo.
struct MetaInfoBuilder {
  void add(int pos, Unit::MetaInfo::Kind kind,
           bool mVector, int arg, Id data);
  void addKnownDataType(DataType dt,
                        bool     dtPredicted,
                        int      pos,
                        bool     mVector,
                        int      arg);
  void deleteInfo(Offset bcOffset);
  void setForUnit(UnitEmitter&) const;

private:
  typedef std::vector<Unit::MetaInfo> Vec;
  typedef std::map<Offset,Vec> Map;
  Map m_metaMap;
};

class Emitter {
public:
  Emitter(ConstructPtr node, UnitEmitter& ue, EmitterVisitor& ev)
    : m_node(node), m_ue(ue), m_ev(ev) {}
  UnitEmitter& getUnitEmitter() { return m_ue; }
  ConstructPtr getNode() { return m_node; }
  EmitterVisitor& getEmitterVisitor() { return m_ev; }
  void setTempLocation(LocationPtr loc) { m_tempLoc = loc; }
  LocationPtr getTempLocation() { return m_tempLoc; }
  void incStat(int counter, int value) {
    if (RuntimeOption::EnableEmitterStats) {
      IncStat(counter, value);
    }
  }

  struct StrOff {
    StrOff(Id s, Label* d) : str(s), dest(d) {}
    Id str;
    Label* dest;
  };
#define O(name, imm, pop, push, flags) \
  void name(imm);
#define NA
#define ONE(typ) \
  typ a1
#define TWO(typ1, typ2) \
  typ1 a1, typ2 a2
#define THREE(typ1, typ2, typ3) \
  typ1 a1, typ2 a2, typ3 a3
#define FOUR(typ1, typ2, typ3, typ4) \
  typ1 a1, typ2 a2, typ3 a3, typ4 a4
#define MA std::vector<uchar>
#define BLA std::vector<Label*>&
#define SLA std::vector<StrOff>&
#define IVA int32
#define HA int32
#define IA int32
#define I64A int64
#define DA double
#define SA const StringData*
#define AA ArrayData*
#define BA Label&
#define OA unsigned char
  OPCODES
#undef O
#undef NA
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef MA
#undef BLA
#undef SLA
#undef IVA
#undef HA
#undef IA
#undef I64A
#undef DA
#undef SA
#undef AA
#undef BA
#undef OA
private:
  ConstructPtr m_node;
  UnitEmitter& m_ue;
  EmitterVisitor& m_ev;
  LocationPtr m_tempLoc;
};

struct SymbolicStack {
  enum ClassBaseType {
    CLS_INVALID,
    CLS_LATE_BOUND,
    CLS_UNNAMED_LOCAL, // loc is an unnamed local
    CLS_NAMED_LOCAL,   // loc is a normal program local
    CLS_STRING_NAME,   // name is the string to use
    CLS_SELF,
    CLS_PARENT
  };

  enum MetaType {
    META_NONE,
    META_LITSTR,
    META_DATA_TYPE
  };

private:
  /**
   * Symbolic stack (m_symStack)
   *
   * The symbolic stack is used to keep track of the flavor descriptors
   * of values along with other contextual information. Each position in
   * the symbolic stack can encode a "symbolic flavor" and a "marker".
   * Most symbolic flavors correspond with flavor descriptors in the HHBC
   * spec, but some symbolic flavors used in the symbolic stack (ex. "L")
   * do not correspond with a flavor descriptor from the spec. Markers
   * provide contextual information and are used by the emitter in various
   * situations to determine the appropriate bytecode instruction to use.
   *
   * Note that not all positions on the symbolic stack correspond to a
   * value on the actual evaluation stack as described in the HHBC spec.
   */
  struct SymEntry {
    explicit SymEntry(char s = 0)
      : sym(s)
      , metaType(META_NONE)
      , notRef(false)
      , notNull(false)
      , dtPredicted(false)
      , className(nullptr)
      , intval(-1)
      , unnamedLocalStart(InvalidAbsoluteOffset)
      , clsBaseType(CLS_INVALID)
    {}
    char sym;
    MetaType metaType;
    bool notRef:1;
    bool notNull:1;
    bool dtPredicted:1;
    union {
      const StringData* name;   // META_LITSTR
      DataType dt;              // META_DATA_TYPE
    }   metaData;
    const StringData* className;
    int64 intval; // used for L and I symbolic flavors

    // If intval is an unnamed local temporary, this offset is the start
    // of the region we are using it (which we will need to have a
    // fault funclet for).
    Offset unnamedLocalStart;

    // When class bases are emitted, we need to delay class lookup for
    // evaluation order reasons, but may have to do some evaluation
    // early.  How this works depends on the type of class base---see
    // emitResolveClsBase for details.
    ClassBaseType clsBaseType;
  };
  std::vector<SymEntry> m_symStack;

  /**
   * Actual stack (m_actualStack)
   *
   * The actual stack represents the evaluation stack as described in the
   * HHBC spec. Each position in m_actualStack contains the index of the
   * corresponding symbolic value in m_symStack.
   */
  std::vector<int> m_actualStack;

  // The number of Func descriptors (in HHVM terms, ActRecs) currently on the
  // stack.
  int m_fdescCount;

public:
  int* m_actualStackHighWaterPtr;
  int* m_fdescHighWaterPtr;

  SymbolicStack() : m_fdescCount(0) {}

  std::string pretty() const;

  void push(char sym);
  void setInt(int64 v);
  void setString(const StringData* s);
  void setKnownCls(const StringData* s, bool nonNull);
  void setNotRef();
  bool getNotRef() const;
  void setKnownType(DataType dt, bool predicted = false);
  void cleanTopMeta();
  DataType getKnownType(int index = -1, bool noRef = true) const;
  void setClsBaseType(ClassBaseType);
  void setUnnamedLocal(int index, int localId, Offset startOffset);
  void pop();
  char top() const;
  char get(int index) const;
  const StringData* getName(int index) const;
  const StringData* getClsName(int index) const;
  bool isCls(int index) const;
  bool isTypePredicted(int index = -1 /* stack top */) const;
  void set(int index, char sym);
  unsigned size() const;
  bool empty() const;
  void clear();

  /*
   * Erase a stack element depth below the top.  This is used for some
   * instructions that pull elements out of the middle, and for our
   * ClassBaseType virtual elements.
   */
  void consumeBelowTop(int depth);

  int getActualPos(int vpos) const;
  char getActual(int index) const;
  void setActual(int index, char sym);
  void insertAt(int depth, char sym);
  int sizeActual() const;

  ClassBaseType getClsBaseType(int index) const;
  int getLoc(int index) const;
  int64 getInt(int index) const;
  Offset getUnnamedLocStart(int index) const;

  void pushFDesc();
  void popFDesc();
};

class Label {
public:
  Label() : m_off(InvalidAbsoluteOffset) {}
  Label(Emitter& e) : m_off(InvalidAbsoluteOffset) {
    set(e);
  }
  Offset getAbsoluteOffset() const { return m_off; }
  // Sets the Label to the bytecode offset of given by e,
  // fixes up any instructions that have already been
  // emitted that reference this Label, and fixes up the
  // EmitterVisitor's jump target info
  void set(Emitter& e);
  // If a Label is has not been set, it is the Emitter's
  // resposibility to call bind on the Label each time it
  // prepares to emit an instruction that uses the Label
  void bind(EmitterVisitor& ev, Offset instrAddr, Offset offAddr);
  bool isSet() { return m_off != InvalidAbsoluteOffset; }
  bool isUsed();
private:
  Offset m_off;
  std::vector<std::pair<Offset, Offset> > m_emittedOffs;
  // m_evalStack is used to store the eval stack of the
  // first forward jump we see that references this label
  SymbolicStack m_evalStack;
};

class Thunklet {
public:
  virtual ~Thunklet();
  virtual void emit(Emitter& e) = 0;
};

class Funclet {
public:
  Funclet(Thunklet* body, Label* entry) : m_body(body), m_entry(entry) {}
  Thunklet* m_body;
  Label* m_entry;
};

class EmitterVisitor {
  friend class UnsetUnnamedLocalThunklet;
public:
  EmitterVisitor(UnitEmitter& ue);
  ~EmitterVisitor();

  bool visit(ConstructPtr c);
  bool visitImpl(ConstructPtr c);
  void visitKids(ConstructPtr c);
  void visit(FileScopePtr file);
  void assignLocalVariableIds(FunctionScopePtr fs);
  void fixReturnType(Emitter& e, FunctionCallPtr fn,
                     bool isBuiltinCall = false);
  typedef std::vector<int> IndexChain;
  void visitListAssignmentLHS(Emitter& e, ExpressionPtr exp,
                              IndexChain& indexChain,
                              std::vector<IndexChain*>& chainList);
  void visitIfCondition(ExpressionPtr cond, Emitter& e, Label& tru, Label& fals,
                        bool truFallthrough);
  const SymbolicStack& getEvalStack() const { return m_evalStack; }
  SymbolicStack& getEvalStack() { return m_evalStack; }
  void setEvalStack(const SymbolicStack& es) {
    m_evalStack = es;
    m_evalStackIsUnknown = false;
  }
  bool evalStackIsUnknown() { return m_evalStackIsUnknown; }
  void popEvalStack(char symFlavor, int arg = -1, int pos = -1);
  void popSymbolicLocal(Opcode opcode, int arg = -1, int pos = -1);
  void popEvalStackLMany();
  void popEvalStackMany(int len, char symFlavor);
  void pushEvalStack(char symFlavor);
  void peekEvalStack(char symFlavor, int depthActual);
  void pokeEvalStack(char symFlavor, int depthActual);
  void prepareEvalStack();
  void recordJumpTarget(Offset target, const SymbolicStack& evalStack);
  void recordJumpTarget(Offset target) {
    recordJumpTarget(target, m_evalStack);
  }
  void restoreJumpTargetEvalStack();
  bool isJumpTarget(Offset target);
  void setPrevOpcode(Opcode op) { m_prevOpcode = op; }
  Opcode getPrevOpcode() const { return m_prevOpcode; }
  bool currentPositionIsReachable() {
    return (m_ue.bcPos() == m_curFunc->base()
            || isJumpTarget(m_ue.bcPos())
            || (instrFlags(getPrevOpcode()) & TF) == 0);
  }

  class IncludeTimeFatalException : public Exception {
  public:
    ConstructPtr m_node;
    IncludeTimeFatalException(ConstructPtr node, const char* fmt, ...)
        : Exception(), m_node(node) {
      va_list ap; va_start(ap, fmt); format(fmt, ap); va_end(ap);
    }
    virtual ~IncludeTimeFatalException() throw() {}
    EXCEPTION_COMMON_IMPL(IncludeTimeFatalException);
  };

  void pushIterId(Id id) { m_pendingIters.push_back(id); }
  void popIterId() { m_pendingIters.pop_back(); }
private:
  typedef std::pair<StringData*, bool> ClosureUseVar;  // (name, byRef)
  typedef std::vector<ClosureUseVar> ClosureUseVarVec;
  typedef std::vector<Id> IdVec;
  class PostponedMeth {
  public:
    PostponedMeth(MethodStatementPtr m, FuncEmitter* fe, bool top,
                  ClosureUseVarVec* useVars)
        : m_meth(m), m_fe(fe), m_top(top), m_closureUseVars(useVars) {}
    MethodStatementPtr m_meth;
    FuncEmitter* m_fe;
    bool m_top;
    ClosureUseVarVec* m_closureUseVars;
  };
  class PostponedCtor {
  public:
    PostponedCtor(InterfaceStatementPtr is, FuncEmitter* fe)
      : m_is(is), m_fe(fe) {}
    InterfaceStatementPtr m_is;
    FuncEmitter* m_fe;
  };
  typedef std::pair<StringData*, ExpressionPtr> NonScalarPair;
  typedef std::vector<NonScalarPair> NonScalarVec;
  class PostponedNonScalars {
  public:
    PostponedNonScalars(InterfaceStatementPtr is, FuncEmitter* fe,
                        NonScalarVec* v)
      : m_is(is), m_fe(fe), m_vec(v) {}
    void release() {
      delete m_vec;
    }
    InterfaceStatementPtr m_is;
    FuncEmitter* m_fe;
    NonScalarVec* m_vec;
  };
  class PostponedClosureCtor {
  public:
    PostponedClosureCtor(ClosureUseVarVec& v, ClosureExpressionPtr e,
                         FuncEmitter* fe)
        : m_useVars(v), m_expr(e), m_fe(fe) {}
    ClosureUseVarVec m_useVars;
    ClosureExpressionPtr m_expr;
    FuncEmitter* m_fe;
  };
  class ControlTargets {
  public:
    ControlTargets(Id itId, Label& brkTarg, Label& cntTarg, Label& brkHand,
        Label& cntHand) : m_itId(itId), m_brkTarg(brkTarg), m_cntTarg(cntTarg),
    m_brkHand(brkHand), m_cntHand(cntHand) {}
    Id m_itId;
    Label& m_brkTarg;  // Jump here for "break;" (after doing IterFree)
    Label& m_cntTarg;  // Jump here for "continue;"
    Label& m_brkHand;  // Push N and jump here for "break N;"
    Label& m_cntHand;  // Push N and jump here for "continue N;"
  };
  class ControlTargetPusher {
  public:
    ControlTargetPusher(EmitterVisitor* e, Id itId, Label& brkTarg,
        Label& cntTarg, Label& brkHand, Label& cntHand) : m_e(e) {
      e->m_contTargets.push_front(ControlTargets(itId, brkTarg, cntTarg,
            brkHand, cntHand));
    }
    ~ControlTargetPusher() {
      m_e->m_contTargets.pop_front();
    }
  private:
    EmitterVisitor* m_e;
  };
  class ExnHandlerRegion {
  public:
    ExnHandlerRegion(Offset start, Offset end) : m_start(start),
      m_end(end) {}
    ~ExnHandlerRegion() {
      for (std::vector<std::pair<StringData*, Label*> >::const_iterator it =
             m_catchLabels.begin(); it != m_catchLabels.end(); it++) {
        delete it->second;
      }
    }
    Offset m_start;
    Offset m_end;
    std::set<StringData*, string_data_lt> m_names;
    std::vector<std::pair<StringData*, Label*> > m_catchLabels;
  };
  class FaultRegion {
  public:
    FaultRegion(Offset start, Offset end, Id iterId)
     : m_start(start), m_end(end), m_iterId(iterId) {}
    Offset m_start;
    Offset m_end;
    Id m_iterId;
    Label m_func;
  };
  class FPIRegion {
    public:
      FPIRegion(Offset start, Offset end, Offset fpOff)
        : m_start(start), m_end(end), m_fpOff(fpOff) {}
      Offset m_start;
      Offset m_end;
      Offset m_fpOff;
  };
  typedef std::pair<Id, int> StrCase;
  struct SwitchState : private boost::noncopyable {
    SwitchState() : nonZeroI(-1), defI(-1) {}
    std::map<int64, int> cases; // a map from int (or litstr id) to case index
    std::vector<StrCase> caseOrder; // for string switches, a list of the
                                    // <litstr id, case index> in the order
                                    // they appear in the source
    int nonZeroI;
    int defI;
  };

  static const size_t kMinStringSwitchCases = 8;
  UnitEmitter& m_ue;
  FuncEmitter* m_curFunc;
  FileScopePtr m_file;

  Opcode m_prevOpcode;

  std::deque<PostponedMeth> m_postponedMeths;
  std::deque<PostponedCtor> m_postponedCtors;
  std::deque<PostponedNonScalars> m_postponedPinits;
  std::deque<PostponedNonScalars> m_postponedSinits;
  std::deque<PostponedNonScalars> m_postponedCinits;
  std::deque<PostponedClosureCtor> m_postponedClosureCtors;
  IdVec m_pendingIters;
  typedef std::map<const StringData*, Label*, string_data_lt> LabelMap;
  LabelMap m_methLabels;
  SymbolicStack m_evalStack;
  bool m_evalStackIsUnknown;
  hphp_hash_map<Offset, SymbolicStack> m_jumpTargetEvalStacks;
  int m_actualStackHighWater;
  int m_fdescHighWater;
  int m_closureCounter;  // used to uniquify closures' mangled names
  std::deque<ControlTargets> m_contTargets;
  std::deque<Funclet> m_funclets;
  std::deque<ExnHandlerRegion*> m_exnHandlers;
  std::deque<FaultRegion*> m_faultRegions;
  std::deque<FPIRegion*> m_fpiRegions;
  std::vector<HphpArray*> m_staticArrays;
  std::set<std::string,stdltistr> m_hoistables;
  LocationPtr m_tempLoc;
  std::map<StringData*, Label, string_data_lt> m_gotoLabels;
  MetaInfoBuilder m_metaInfo;
public:
  Label& topBreakHandler() { return m_contTargets.front().m_brkHand; }
  Label& topContHandler() { return m_contTargets.front().m_cntHand; }

  bool checkIfStackEmpty(const char* forInstruction) const;
  void unexpectedStackSym(char sym, const char* where) const;

  int scanStackForLocation(int iLast);
  void buildVectorImm(std::vector<uchar>& vectorImm,
                      int iFirst, int iLast, bool allowW,
                      Emitter& e);
  void emitAGet(Emitter& e);
  void emitCGetL2(Emitter& e);
  void emitCGetL3(Emitter& e);
  void emitCGet(Emitter& e);
  void emitVGet(Emitter& e);
  void emitIsset(Emitter& e);
  void emitIsNull(Emitter& e);
  void emitIsArray(Emitter& e);
  void emitIsObject(Emitter& e);
  void emitIsString(Emitter& e);
  void emitIsInt(Emitter& e);
  void emitIsDouble(Emitter& e);
  void emitIsBool(Emitter& e);
  void emitEmpty(Emitter& e);
  void emitUnset(Emitter& e);
  void emitSet(Emitter& e);
  void emitSetOp(Emitter& e, int op);
  void emitBind(Emitter& e);
  void emitIncDec(Emitter& e, unsigned char cop);
  void emitPop(Emitter& e);
  void emitConvertToCell(Emitter& e);
  void emitFreePendingIters(Emitter& e);
  void emitConvertToCellIfVar(Emitter& e);
  void emitConvertToCellOrLoc(Emitter& e);
  void emitConvertSecondToCell(Emitter& e);
  void emitConvertToVar(Emitter& e);
  void emitVirtualLocal(int localId, DataType dt = KindOfUnknown);
  template<class Expr> void emitVirtualClassBase(Emitter&, Expr* node);
  void emitResolveClsBase(Emitter& e, int pos);
  void emitClsIfSPropBase(Emitter& e);
  Label* getContinuationGotoLabel(StatementPtr s);
  void emitContinuationSwitch(Emitter& e, SwitchStatementPtr s);
  DataType analyzeSwitch(SwitchStatementPtr s, SwitchState& state);
  void emitIntegerSwitch(Emitter& e, SwitchStatementPtr s,
                         std::vector<Label>& caseLabels, Label& done,
                         const SwitchState& state);
  void emitStringSwitch(Emitter& e, SwitchStatementPtr s,
                        std::vector<Label>& caseLabels, Label& done,
                        const SwitchState& state);

  void markElem(Emitter& e);
  void markNewElem(Emitter& e);
  void markProp(Emitter& e);
  void markSProp(Emitter& e);
  void markName(Emitter& e);
  void markNameSecond(Emitter& e);
  void markGlobalName(Emitter& e);

  void emitNameString(Emitter& e, ExpressionPtr n, bool allowLiteral = false);
  void emitAssignment(Emitter& e, ExpressionPtr c, int op, bool bind);
  void emitListAssignment(Emitter& e, ListAssignmentPtr lst);
  void postponeMeth(MethodStatementPtr m, FuncEmitter* fe, bool top,
                    ClosureUseVarVec* useVars = NULL);
  void postponeCtor(InterfaceStatementPtr m, FuncEmitter* fe);
  void postponePinit(InterfaceStatementPtr m, FuncEmitter* fe, NonScalarVec* v);
  void postponeSinit(InterfaceStatementPtr m, FuncEmitter* fe, NonScalarVec* v);
  void postponeCinit(InterfaceStatementPtr m, FuncEmitter* fe, NonScalarVec* v);
  void emitPostponedMeths();
  void emitPostponedCtors();
  void emitPostponedPSinit(PostponedNonScalars& p, bool pinit);
  void emitPostponedPinits();
  void emitPostponedSinits();
  void emitPostponedCinits();
  void emitPostponedClosureCtors();
  enum CallUserFuncFlags {
    CallUserFuncNone = -1,
    CallUserFuncPlain = 0,
    CallUserFuncArray = 1,
    CallUserFuncSafe = 2,
    CallUserFuncReturn = 4,
    CallUserFuncForward = 8,
    CallUserFuncSafeArray = CallUserFuncSafe | CallUserFuncArray,
    CallUserFuncSafeReturn = CallUserFuncSafe | CallUserFuncReturn,
    CallUserFuncForwardArray = CallUserFuncForward | CallUserFuncArray
  };

  bool emitCallUserFunc(Emitter& e, SimpleFunctionCallPtr node);
  bool canEmitBuiltinCall(FunctionCallPtr fn, const std::string& name,
                          int numParams);
  void emitFuncCall(Emitter& e, FunctionCallPtr node);
  void emitFuncCallArg(Emitter& e, ExpressionPtr exp, int paramId);
  void emitBuiltinCallArg(Emitter& e, ExpressionPtr exp, int paramId,
                         bool byRef);
  void emitBuiltinDefaultArg(Emitter& e, Variant& v, DataType t, int paramId);
  PreClass::Hoistable emitClass(Emitter& e, ClassScopePtr cNode,
                                bool topLevel);
  void emitBreakHandler(Emitter& e, Label& brkTarg, Label& cntTarg,
      Label& brkHand, Label& cntHand, Id iter = -1);
  void emitForeach(Emitter& e, ForEachStatementPtr fe);
  void emitRestoreErrorReporting(Emitter& e, Id oldLevelLoc);
  void emitMakeUnitFatal(Emitter& e, const std::string& message);

  void addFunclet(Thunklet* body, Label* entry);
  void emitFunclets(Emitter& e);
  void newFaultRegion(Offset start, Offset end, Thunklet* t, Id iter = -1);
  void newFPIRegion(Offset start, Offset end, Offset fpOff);
  void copyOverExnHandlers(FuncEmitter* fe);
  void copyOverFPIRegions(FuncEmitter* fe);
  void saveMaxStackCells(FuncEmitter* fe);
  void finishFunc(Emitter& e, FuncEmitter* fe);
  StringData* newClosureName();

  void initScalar(TypedValue& tvVal, ExpressionPtr val);

  void emitClassTraitPrecRule(PreClassEmitter* pce, TraitPrecStatementPtr rule);
  void emitClassTraitAliasRule(PreClassEmitter* pce,
                               TraitAliasStatementPtr rule);
  void emitClassUseTrait(PreClassEmitter* pce, UseTraitStatementPtr useStmt);
};

void emitAllHHBC(AnalysisResultPtr ar);

extern "C" {
  Unit* hphp_compiler_parse(const char* code, int codeLen, const MD5& md5,
                            const char* filename);
  Unit* hphp_build_native_func_unit(const HhbcExtFuncInfo* builtinFuncs,
                                    ssize_t numBuiltinFuncs);
  Unit* hphp_build_native_class_unit(const HhbcExtClassInfo* builtinClasses,
                                     ssize_t numBuiltinClasses);
}

///////////////////////////////////////////////////////////////////////////////
}
}

#endif // __COMPILER_EMITTER_H__
