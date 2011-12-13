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

class Emitter {
public:
  Emitter(ConstructPtr node, Unit &u, EmitterVisitor &ev)
    : m_node(node), m_u(u), m_ev(ev) {}
  Unit &getUnit() { return m_u; }
  ConstructPtr getNode() { return m_node; }
  EmitterVisitor &getEmitterVisitor() { return m_ev; }

#define O(name, imm, pop, push, flags) \
  void name(imm);
#define NA
#define ONE(typ) \
  typ a1
#define TWO(typ1, typ2) \
  typ1 a1, typ2 a2
#define THREE(typ1, typ2, typ3) \
  typ1 a1, typ2 a2, typ3 a3
#define LA std::vector<uchar>
#define IVA int32
#define I64A int64
#define DA double
#define SA StringData*
#define AA ArrayData*
#define BA Label&
#define OA unsigned char
  OPCODES
#undef O
#undef NA
#undef ONE
#undef TWO
#undef THREE
#undef LA
#undef IVA
#undef I64A
#undef DA
#undef SA
#undef AA
#undef BA
#undef OA
private:
  ConstructPtr m_node;
  Unit &m_u;
  EmitterVisitor &m_ev;
};

class SymbolicStack {
public:
  /**
   * Symbolic stack (m_symStack)
   *
   * The symbolic stack is used to keep track of the protoflavors of
   * values along with other contextual information. Each position in
   * the symbolic stack can encode a protoflavor and a "marker". Markers
   * provide contextual information and are used by the emitter in various
   * situations to determine the appropriate bytecode instruction to use.
   *
   * Note that not all positions on the symbolic stack correspond to a
   * value on the actual evaluation stack as described in the HHBC spec.
   * Such positions have no protoflavor associated with them.
   */
  std::vector<char> m_symStack;

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

  int *m_actualStackHighWaterPtr;
  int *m_fdescHighWaterPtr;

  SymbolicStack() : m_fdescCount(0) {}

  void push(char sym);
  void pop();
  char top() const;
  char get(int index) const;
  void set(int index, char sym);
  unsigned size() const;
  bool empty() const;
  void clear();

  char getActual(int index) const;
  void setActual(int index, char sym);
  int sizeActual() const;

  void pushFDesc();
  void popFDesc();
  int fdescCount() const { return m_fdescCount; }
};

class Label {
public:
  Label() : m_off(InvalidAbsoluteOffset) {}
  Label(Emitter &e) : m_off(InvalidAbsoluteOffset) {
    set(e);
  }
  Offset getAbsoluteOffset() const { return m_off; }
  // Sets the Label to the bytecode offset of given by e,
  // fixes up any instructions that have already been
  // emitted that reference this Label, and fixes up the
  // EmitterVisitor's jump target info
  void set(Emitter &e);
  // If a Label is has not been set, it is the Emitter's
  // resposibility to call bind on the Label each time it
  // prepares to emit an instruction that uses the Label
  void bind(EmitterVisitor & ev, Offset instrAddr, Offset offAddr);
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
  virtual void emit(Emitter &e) = 0;
};

class Funclet {
public:
  Funclet(Thunklet *body, Label *entry) : m_body(body), m_entry(entry) {}
  Thunklet *m_body;
  Label *m_entry;
};

class EmitterVisitor {
public:
  EmitterVisitor(Unit &u);
  ~EmitterVisitor();

  bool visit(ConstructPtr c);
  void visitKids(ConstructPtr c);
  void visit(FileScopePtr file);
  void assignLocalVariableIds(FunctionScopePtr fs);
  typedef std::vector<int> IndexChain;
  void visitListAssignmentLHS(Emitter& e, ExpressionPtr exp,
                              IndexChain& indexChain,
                              std::vector<IndexChain*>& chainList);
  void visitIfCondition(ExpressionPtr cond, Emitter& e, Label& tru, Label& fals,
                        bool truFallthrough);
  const SymbolicStack& getEvalStack() { return m_evalStack; }
  void setEvalStack(const SymbolicStack& es) {
    m_evalStack = es;
    m_evalStackIsUnknown = false;
  }
  bool evalStackIsUnknown() { return m_evalStackIsUnknown; }
  void popEvalStack(char protoflavor);
  void popEvalStackLMany();
  void popEvalStackFMany(int len);
  void pushEvalStack(char protoflavor);
  void peekEvalStack(char protoflavor, int depthActual);
  void pokeEvalStack(char protoflavor, int depthActual);
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
    return (m_unit.bcPos() == m_curFunc->m_base
            || isJumpTarget(m_unit.bcPos())
            || (instrFlags(getPrevOpcode()) & UF) == 0);
  }
  StringData* mangleStaticName(const std::string& varName);

  class IncludeTimeFatalException : public Exception {
  public:
    ConstructPtr m_node;
    IncludeTimeFatalException(ConstructPtr node, const char* fmt, ...)
        : Exception(), m_node(node) {
      va_list ap; va_start(ap, fmt); format(fmt, ap); va_end(ap);
    }
    virtual ~IncludeTimeFatalException() throw() {}
    virtual IncludeTimeFatalException* clone() {
      return new IncludeTimeFatalException(*this);
    }
    virtual void throwException() { throw *this; }
  };

protected:
  typedef std::pair<StringData*, bool> ClosureUseVar;  // (name, byRef)
  typedef std::vector<ClosureUseVar> ClosureUseVarVec;
  class PostponedMeth {
  public:
    PostponedMeth(MethodStatementPtr m, Func *f, bool top,
                  ClosureUseVarVec* useVars)
        : m_meth(m), m_func(f), m_top(top), m_closureUseVars(useVars) {}
    MethodStatementPtr m_meth;
    Func *m_func;
    bool m_top;
    ClosureUseVarVec* m_closureUseVars;
  };
  class PostponedCtor {
  public:
    PostponedCtor(InterfaceStatementPtr is, Func *f)
      : m_is(is), m_func(f) {}
    InterfaceStatementPtr m_is;
    Func *m_func;
  };
  typedef std::pair<StringData*, ExpressionPtr> NonScalarPair;
  typedef std::vector<NonScalarPair> NonScalarVec;
  class PostponedNonScalars {
  public:
    PostponedNonScalars(InterfaceStatementPtr is, Func *f, NonScalarVec* v)
      : m_is(is), m_func(f), m_vec(v) {}
    void release() {
      delete m_vec;
    }
    InterfaceStatementPtr m_is;
    Func *m_func;
    NonScalarVec* m_vec;
  };
  class PostponedClosureCtor {
  public:
    PostponedClosureCtor(ClosureUseVarVec& v, ClosureExpressionPtr e, Func* f)
        : m_useVars(v), m_expr(e), m_func(f) {}
    ClosureUseVarVec m_useVars;
    ClosureExpressionPtr m_expr;
    Func* m_func;
  };
  class ControlTargets {
  public:
    ControlTargets(Id itId, Label &brkTarg, Label &cntTarg, Label &brkHand,
        Label &cntHand) : m_itId(itId), m_brkTarg(brkTarg), m_cntTarg(cntTarg),
    m_brkHand(brkHand), m_cntHand(cntHand) {}
    Id m_itId;
    Label &m_brkTarg;  // Jump here for "break;" (after doing IterFree)
    Label &m_cntTarg;  // Jump here for "continue;"
    Label &m_brkHand;  // Push N and jump here for "break N;"
    Label &m_cntHand;  // Push N and jump here for "continue N;"
  };
  class ControlTargetPusher {
  public:
    ControlTargetPusher(EmitterVisitor *e, Id itId, Label &brkTarg,
        Label &cntTarg, Label &brkHand, Label &cntHand) : m_e(e) {
      e->m_contTargets.push_front(ControlTargets(itId, brkTarg, cntTarg,
            brkHand, cntHand));
    }
    ~ControlTargetPusher() {
      m_e->m_contTargets.pop_front();
    }
  private:
    EmitterVisitor *m_e;
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
    FaultRegion(Offset start, Offset end)
     : m_start(start), m_end(end) {}
    Offset m_start;
    Offset m_end;
    Label m_func;
  };
  class ForEachRegion {
  public:
    ForEachRegion(Offset start, Offset end, Id iterId)
      : m_start(start), m_end(end), m_iterId(iterId) {}
    Offset m_start;
    Offset m_end;
    Id m_iterId;
  };
  class FPIRegion {
    public:
      FPIRegion(Offset start, Offset end, Offset fpOff)
        : m_start(start), m_end(end), m_fpOff(fpOff) {}
      Offset m_start;
      Offset m_end;
      Offset m_fpOff;
  };
  Unit &m_unit;
  Func *m_curFunc;

  Opcode m_prevOpcode;

  std::deque<PostponedMeth> m_postponedMeths;
  std::deque<PostponedCtor> m_postponedCtors;
  std::deque<PostponedNonScalars> m_postponedPinits;
  std::deque<PostponedNonScalars> m_postponedSinits;
  std::deque<PostponedNonScalars> m_postponedCinits;
  std::deque<PostponedClosureCtor> m_postponedClosureCtors;
  std::map<StringData*, Label*, string_data_lt> m_methLabels;
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
  std::deque<ForEachRegion*> m_feRegions;
  std::deque<FPIRegion*> m_fpiRegions;
  std::vector<HphpArray*> m_staticArrays;

  std::map<StringData*, Label, string_data_lt> m_gotoLabels;
public:
  Label &topBreakHandler() { return m_contTargets.front().m_brkHand; }
  Label &topContHandler() { return m_contTargets.front().m_cntHand; }

  int scanStackForLocation(int iLast);
  void buildVectorImm(std::vector<uchar>& vectorImm,
                      int iFirst, int iLast, bool allowW, Emitter& e);
  void emitCGet(Emitter &e);
  void emitVGet(Emitter &e);
  void emitIsset(Emitter &e);
  void emitEmpty(Emitter &e);
  void emitUnset(Emitter &e);
  void emitSet(Emitter &e);
  void emitSetOp(Emitter &e, int op);
  void emitBind(Emitter &e);
  void emitIncDec(Emitter &e, unsigned char cop);
  void emitPop(Emitter &e);
  void emitConvertToCell(Emitter &e);
  void emitConvertToCellIfVar(Emitter &e);
  void emitConvertToCellOrHome(Emitter &e);
  void emitConvertSecondToCell(Emitter &e);
  void emitConvertToVar(Emitter &e);
  void emitCls(Emitter &e, int depthActual);
  void emitClsIfSPropBase(Emitter &e);

  void markElem(Emitter &e);
  void markNewElem(Emitter &e);
  void markProp(Emitter &e);
  void markSProp(Emitter &e);
  void markName(Emitter &e);
  void markNameSecond(Emitter &e);
  void markGlobalName(Emitter &e);

  void emitNameString(Emitter &e, ExpressionPtr n);
  void emitAssignment(Emitter &e, ExpressionPtr c, int op, bool bind);
  void emitListAssignment(Emitter &e, ListAssignmentPtr lst);
  void postponeMeth(MethodStatementPtr m, Func *f, bool top,
                    ClosureUseVarVec* useVars = NULL);
  void postponeCtor(InterfaceStatementPtr m, Func *f);
  void postponePinit(InterfaceStatementPtr m, Func *f, NonScalarVec* v);
  void postponeSinit(InterfaceStatementPtr m, Func *f, NonScalarVec* v);
  void postponeCinit(InterfaceStatementPtr m, Func *f, NonScalarVec* v);
  void emitPostponedMeths();
  void emitPostponedCtors();
  void emitPostponedPSinit(PostponedNonScalars& p, bool pinit);
  void emitPostponedPinits();
  void emitPostponedSinits();
  void emitPostponedCinits();
  void emitPostponedClosureCtors();
  void emitFuncCall(Emitter &e, FunctionCallPtr node);
  void emitFuncCallArg(Emitter& e, ExpressionPtr exp, int paramId);
  void emitClass(Emitter& e, ClassScopePtr cNode, bool hoistable);
  void emitBreakHandler(Emitter &e, Label &brkTarg, Label &cntTarg,
      Label &brkHand, Label &cntHand, Id iter = -1);
  void emitForeach(Emitter &e, ExpressionPtr val, ExpressionPtr key,
      StatementPtr body, bool strong);
  void emitRestoreErrorReporting(Emitter& e, Id oldLevelLoc);
  void emitMakeUnitFatal(Emitter& e, const std::string& message);

  void addFunclet(Thunklet *body, Label *entry);
  void emitFunclets(Emitter &e);
  void newFaultRegion(Offset start, Offset end, Thunklet *t);
  void newForEachRegion(Offset start, Offset end, Id iterId);
  void newFPIRegion(Offset start, Offset end, Offset fpOff);
  void copyOverExnHandlers(Func *f);
  void copyOverFERegions(Func *f);
  void copyOverFPIRegions(Func *f);
  void saveMaxStackCells(Func *f);
  void finishFunc(Emitter& e, Func* f, Offset funclets=InvalidAbsoluteOffset);
  StringData* newClosureName();

  void initScalar(TypedValue& tvVal, ExpressionPtr val);

  void emitClassTraitPrecRule(PreClass* preClass, TraitPrecStatementPtr rule);
  void emitClassTraitAliasRule(PreClass* preClass, TraitAliasStatementPtr rule);
  void emitClassUseTrait(PreClass* preClass, UseTraitStatementPtr useStmt);
};

void emitHHBCVisitor(AnalysisResultPtr ar, StatementPtr sp, void* data);
extern "C" {
  Unit *hphp_compiler_parse(const char* code, int codeLen,
                            const char* filename);
}

///////////////////////////////////////////////////////////////////////////////
}
}

#endif // __COMPILER_EMITTER_H__
