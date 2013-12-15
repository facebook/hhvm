/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_COMPILER_EMITTER_H_
#define incl_HPHP_COMPILER_EMITTER_H_

#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/statement/statement.h"
#include "hphp/compiler/statement/use_trait_statement.h"
#include "hphp/compiler/statement/trait_prec_statement.h"
#include "hphp/compiler/statement/trait_alias_statement.h"
#include "hphp/compiler/statement/typedef_statement.h"

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/util/hash.h"

#include <deque>
#include <utility>

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
class HhbcExtFuncInfo;
class HhbcExtClassInfo;

namespace Compiler {
///////////////////////////////////////////////////////////////////////////////

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

  struct IterPair {
    IterPair(IterKind k, Id i) : kind(k), id(i) {}
    IterKind kind;
    Id id;
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
#define ILA std::vector<IterPair>&
#define IVA int32_t
#define LA int32_t
#define IA int32_t
#define I64A int64_t
#define DA double
#define SA const StringData*
#define AA ArrayData*
#define BA Label&
#define OA unsigned char
#define VSA std::vector<std::string>&
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
#undef ILA
#undef IVA
#undef LA
#undef IA
#undef I64A
#undef DA
#undef SA
#undef AA
#undef BA
#undef OA
#undef VSA
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
    int64_t intval; // used for L and I symbolic flavors

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
  void setInt(int64_t v);
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
  int64_t getInt(int index) const;
  Offset getUnnamedLocStart(int index) const;

  void pushFDesc();
  void popFDesc();
};

class Label {
public:
  Label() : m_off(InvalidAbsoluteOffset) {}
  explicit Label(Emitter& e) : m_off(InvalidAbsoluteOffset) {
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
  explicit Funclet(Thunklet* body)
    : m_body(body) {
  }
  Thunklet* m_body;
  Label m_entry;
};


class FinallyRouter;

DECLARE_BOOST_TYPES(FinallyRouterEntry);
/*
 * FinallyRouterEntry represents a single level of the unified stack
 * of constructs that are meaningful from the point of view of finally
 * implementation. The levels are used to keep track of the information
 * such as the actions that can be taken inside a block.
 */
class FinallyRouterEntry {
public:
  enum EntryKind {
    // Top-level (global) context.
    GlobalEntry,
    // Function body / method body entry.
    FuncBodyEntry,
    // Entry for finally fault funclets emitted after the body of
    // a function
    FuncFaultEntry,
    // Try block entry (begins with try ends after catches).
    TryFinallyEntry,
    // Finally block entry (begins after catches ends after finally)
    FinallyEntry,
    // Loop OR a break statement.
    LoopEntry,
  };

  typedef Emitter::IterPair IterPair;
  typedef std::vector<IterPair> IterVec;

  DECLARE_BOOST_TYPES(Action);
  /*
   * The structure represents a code path that potentially requires
   * running finally blocks. A code path has an assigned state ID that
   * is used inside switch statements emitted at the end of finally
   * blocks. It also has an optional label (the destination to jump
   * to after all the required finally blocks are run).
   */
  struct Action {
    static const int k_unsetState;

    explicit Action(FinallyRouter* router);
    ~Action();

    // Manage state ID reuse.
    bool isAllocated();
    void allocate();
    void release();

    FinallyRouter* m_router;
    // The target to jump to once all the necessary finally blocks
    // are run.
    Label m_label;
    // The state ID that identifies this action inside finally switch
    // statements. This is the id assigned to the "state" unnamed
    // local variable.
    int m_state;
  };

  FinallyRouterEntry(FinallyRouter* router,
                     EntryKind kind,
                     FinallyRouterEntryPtr parent);
  ~FinallyRouterEntry();

  // Helpers used for freeing iterators when appropriate actions
  // require that.
  void emitIterBreak(Emitter& e,
                     IterVec& iters,
                     Label& target);
  void emitIterFree(Emitter& e, IterVec& iters);
  void emitIterFree(Emitter& e);

  // Emission of code snippets commencing actions. Method
  // emitXYZ(e, ...) should be used instead of e.XYZ(...) in
  // EmitterVisitor.
  void emitReturn(Emitter& e, char sym);
  void emitReturnImpl(Emitter& e,
                      char sym,
                      IterVec& iters);
  void emitGoto(Emitter& e, StringData* name);
  void emitGotoImpl(Emitter& e,
                    StringData* name,
                    IterVec& iters);
  void emitBreak(Emitter& e, int depth);
  void emitBreakImpl(Emitter& e,
                     int depth,
                     IterVec& iters);
  void emitContinue(Emitter& e, int depth);
  void emitContinueImpl(Emitter& e,
                        int depth,
                        IterVec& iters);

  // Emit the switch statement in the finally epilogue. Optimizes
  // cases where there is either just a fall-through case, or where
  // there is a single case other than fall-through.
  void emitFinallySwitch(Emitter& e);

  // Helper for emitting beginning of single case from the finally
  // epilogue.
  void emitCase(Emitter& e, std::vector<Label*>& cases, ActionPtr action);
  void emitReturnCase(Emitter& e, std::vector<Label*>& cases, char sym);
  void emitReturnCaseImpl(Emitter& e, char sym, IterVec& iters);
  void emitBreakCase(Emitter& e, std::vector<Label*>& cases, int depth);
  void emitBreakCaseImpl(Emitter& e, int depth, IterVec& iters);
  void emitContinueCase(Emitter& e, std::vector<Label*>& cases, int depth);
  void emitContinueCaseImpl(Emitter& e, int depth, IterVec& iters);

  // Helper for establishing the maximal depth of break / continue
  // actions that are allocated.
  int getBreakContinueDepth();

  // Returns the maximal break / continue depth admissable (aka the
  // number of nested loops).
  int getMaxBreakContinueDepth();

  // Methods used for emitting different cases for the finally
  // epilogue switch.
  void emitGotoCase(Emitter& e, std::vector<Label*>& cases, StringData* name);
  void emitGotoCaseImpl(Emitter& e, StringData* name, IterVec& iters);
  void emitReturnCases(Emitter& e, std::vector<Label*>& cases);
  void emitGotoCases(Emitter& e, std::vector<Label*>& cases);
  void emitBreakContinueCases(Emitter& e, std::vector<Label*>& cases);
  void emitAllCases(Emitter& e, std::vector<Label*>& cases);

  // Methods used for collecting labels corresponding to the cases
  // from the finally epilogue. All the labels are accumulated
  // inside the cases array. The array is eventually passed to
  // e.Switch as the immediate argument.
  void collectReturnCases(std::vector<Label*>& cases);
  void collectGotoCases(std::vector<Label*>& cases);
  void collectBreakContinueCases(std::vector<Label*>& cases);
  void collectCase(std::vector<Label*>& cases,
                   ActionPtr action);
  void collectAllCases(std::vector<Label*>& cases);

  // The number of cases to be emitted. This is a helper used in
  // establishing whether one of the optimized cases can be used.
  int getCaseCount();

  // Methods used for allocating new finally-aware code-paths.
  // When alloc is set to false, the action is merely allocated in
  // memory and shared among different entries. When alloc is set to
  // true, the action is additionally assigned a state ID.
  ActionPtr registerReturn(StatementPtr s, char sym);
  ActionPtr registerGoto(StatementPtr s, StringData* name, bool alloc);
  ActionPtr registerBreak(StatementPtr s, int depth, bool alloc);
  ActionPtr registerContinue(StatementPtr s, int depth, bool alloc);
  // Used to indicate that a particular label is present inside
  // an entry. Labels don't have corresponding actions, however
  // they need to be tracked in order to handle gotos appropriately.
  void registerLabel(StatementPtr s, StringData* name);
  // Yield / await is not supported inside a finally block for now.
  // This method throws a parse time fatal whenever appropriate.
  void registerYieldAwait(ExpressionPtr e);

  FinallyRouter* m_router;
  EntryKind m_kind;
  // Only used for loop / break kind of entries.
  Id m_iterId;
  bool m_iterRef;
  // Because of a bug in code emission, functions sometimes have
  // inconsistent return flavours. Therefore instead of a single
  // return action, there need to be one return action per flavor
  // used. Once the bug is removed, this code can be simplified.
  std::map<char, ActionPtr> m_returnActions;
  // A map of goto actions. Each goto action is identified by the
  // name of the destination label.
  std::map<StringData*, ActionPtr, string_data_lt> m_gotoActions;
  // A map of goto labels occurrning inside the statement represented
  // by this entry. This value is used for establishing whether
  // a finally block needs to be executed when performing gotos.
  std::set<StringData*, string_data_lt> m_gotoLabels;
  // Continue actions identified by their depth.
  std::map<int, ActionPtr> m_continueActions;
  // Break actions identified by their depth.
  std::map<int, ActionPtr> m_breakActions;
  // The label denoting the beginning of a finally block inside the
  // current try. Only used when the entry kind is a try statement.
  Label m_finallyLabel;
  // The cases that need to be included in the switch statement. Only
  // used when kind is set to try statement.
  // These are the actions that are invoked inside the protected region
  // protected by the try represented by this entry.
  std::set<ActionPtr> m_finallyCases;
  // The parent entry.
  FinallyRouterEntryPtr m_parent;
};

class FinallyRouter {
public:
  typedef FinallyRouterEntry::EntryKind EntryKind;

  FinallyRouter();
  ~FinallyRouter();

  // The top entry on the unified stack.
  FinallyRouterEntryPtr top();
  // Create an entry corresponding to the passed in statement s.
  // If the statement is insignificant (e.g. is an if statement),
  // nullptr is returned.
  FinallyRouterEntryPtr createForStatement(StatementPtr s);
  // Create a global (top-level) entry.
  FinallyRouterEntryPtr createGlobal(StatementPtr s);
  // Create an entry for a functiob body.
  FinallyRouterEntryPtr createFuncBody(StatementPtr s);
  // Create an entry for a fault funclet inside a function body
  FinallyRouterEntryPtr createFuncFault(StatementPtr s);
  // Helper function for creating entries.
  FinallyRouterEntryPtr create(StatementPtr s, EntryKind kind);
  // Enter/leave the passed in entry. Note that entries sometimes need be
  // to be constructed before they are entered, or need to be accessed
  // after they are left. This especially applies to constructs such
  // as loops and try blocks.
  void enter(FinallyRouterEntryPtr);
  void leave(FinallyRouterEntryPtr);

  // Functions used for handling state IDs allocation.
  // FIXME (#3275259): This should be moved into global / func
  // body / fault funclet entries in order to optimize state
  // allocation. See the task description for more details.
  int allocateState();
  void releaseState(int state);

  // The stack of all the entered entries.
  std::vector<FinallyRouterEntryPtr> m_entries;
  // The state IDs currently allocated. See FIXME above.
  std::set<int> m_states;
};

class EmitterVisitor {
  friend class UnsetUnnamedLocalThunklet;
  friend class FuncFinisher;
public:
  explicit EmitterVisitor(UnitEmitter& ue);
  ~EmitterVisitor();

  bool visit(ConstructPtr c);
  bool visitImpl(ConstructPtr c);
  void visitKids(ConstructPtr c);
  void visit(FileScopePtr file);
  void assignLocalVariableIds(FunctionScopePtr fs);
  void assignFinallyVariableIds();
  void fixReturnType(Emitter& e, FunctionCallPtr fn,
                     Func* builtinFunc = nullptr);
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
  void popSymbolicLocal(Op opcode, int arg = -1, int pos = -1);
  void popEvalStackMMany();
  void popEvalStackMany(int len, char symFlavor);
  void popEvalStackCVMany(int len);
  void pushEvalStack(char symFlavor);
  void peekEvalStack(char symFlavor, int depthActual);
  void pokeEvalStack(char symFlavor, int depthActual);
  void prepareEvalStack();
  void recordJumpTarget(Offset target, const SymbolicStack& evalStack);
  void recordJumpTarget(Offset target) {
    recordJumpTarget(target, m_evalStack);
  }
  void restoreJumpTargetEvalStack();
  void recordCall();
  bool isJumpTarget(Offset target);
  void setPrevOpcode(Op op) { m_prevOpcode = op; }
  Op getPrevOpcode() const { return m_prevOpcode; }
  bool currentPositionIsReachable() {
    return (m_ue.bcPos() == m_curFunc->base()
            || isJumpTarget(m_ue.bcPos())
            || (instrFlags(getPrevOpcode()) & TF) == 0);
  }
  FuncEmitter* getFuncEmitter() { return m_curFunc; }
  FinallyRouter& getFinallyRouter() { return m_finallyRouter; }
  Id getStateLocal() {
    assert(m_stateLocal >= 0);
    return m_stateLocal;
  }
  Id getRetLocal() {
    assert(m_retLocal >= 0);
    return m_retLocal;
  }

  class IncludeTimeFatalException : public Exception {
  public:
    ConstructPtr m_node;
    bool m_parseFatal;
    IncludeTimeFatalException(ConstructPtr node, const char* fmt, ...)
        : Exception(), m_node(node), m_parseFatal(false) {
      va_list ap; va_start(ap, fmt); format(fmt, ap); va_end(ap);
    }
    virtual ~IncludeTimeFatalException() throw() {}
    EXCEPTION_COMMON_IMPL(IncludeTimeFatalException);
    void setParseFatal(bool b = true) { m_parseFatal = b; }
  };

  void pushIterScope(Id id, IterKind kind) {
    m_pendingIters.emplace_back(id, kind);
  }
  void popIterScope() { m_pendingIters.pop_back(); }

private:
  typedef std::pair<StringData*, bool> ClosureUseVar;  // (name, byRef)
  typedef std::vector<ClosureUseVar> ClosureUseVarVec;
  typedef std::vector<std::pair<Id,IterKind> > PendingIterVec;
  typedef std::pair<StringData*, ExpressionPtr> NonScalarPair;
  typedef std::vector<NonScalarPair> NonScalarVec;
  typedef std::pair<Id, int> StrCase;

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
    FaultRegion(Offset start,
                Offset end,
                Label* func,
                Id iterId,
                IterKind kind)
      : m_start(start)
      , m_end(end)
      , m_func(func)
      , m_iterId(iterId)
      , m_iterKind(kind) {}

    Offset m_start;
    Offset m_end;
    Label* m_func;
    Id m_iterId;
    IterKind m_iterKind;
  };

  class FPIRegion {
    public:
      FPIRegion(Offset start, Offset end, Offset fpOff)
        : m_start(start), m_end(end), m_fpOff(fpOff) {}
      Offset m_start;
      Offset m_end;
      Offset m_fpOff;
  };

  struct SwitchState : private boost::noncopyable {
    SwitchState() : nonZeroI(-1), defI(-1) {}
    std::map<int64_t, int> cases; // a map from int (or litstr id) to case index
    std::vector<StrCase> caseOrder; // for string switches, a list of the
                                    // <litstr id, case index> in the order
                                    // they appear in the source
    int nonZeroI;
    int defI;
  };

private:
  static const size_t kMinStringSwitchCases = 8;
  UnitEmitter& m_ue;
  FuncEmitter* m_curFunc;
  FileScopePtr m_file;

  Op m_prevOpcode;

  std::deque<PostponedMeth> m_postponedMeths;
  std::deque<PostponedCtor> m_postponedCtors;
  std::deque<PostponedNonScalars> m_postponedPinits;
  std::deque<PostponedNonScalars> m_postponedSinits;
  std::deque<PostponedNonScalars> m_postponedCinits;
  std::deque<PostponedClosureCtor> m_postponedClosureCtors;
  PendingIterVec m_pendingIters;
  hphp_hash_map<std::string,FuncEmitter*> m_generatorEmitted;
  hphp_hash_set<std::string> m_nonTopGeneratorEmitted;
  hphp_hash_set<std::string> m_topMethodEmitted;
  SymbolicStack m_evalStack;
  bool m_evalStackIsUnknown;
  hphp_hash_map<Offset, SymbolicStack> m_jumpTargetEvalStacks;
  int m_actualStackHighWater;
  int m_fdescHighWater;
  typedef tbb::concurrent_hash_map<const StringData*, int,
                                   StringDataHashCompare> EmittedClosures;
  static EmittedClosures s_emittedClosures;
  std::deque<Funclet*> m_funclets;
  std::map<StatementPtr, Funclet*> m_memoizedFunclets;
  std::deque<ExnHandlerRegion*> m_exnHandlers;
  std::deque<FaultRegion*> m_faultRegions;
  std::deque<FPIRegion*> m_fpiRegions;
  std::vector<Array> m_staticArrays;
  std::set<std::string,stdltistr> m_hoistables;
  LocationPtr m_tempLoc;
  std::vector<Label> m_yieldLabels;
  FinallyRouter m_finallyRouter;
  Id m_stateLocal;
  Id m_retLocal;

  MetaInfoBuilder m_metaInfo;
public:
  bool checkIfStackEmpty(const char* forInstruction) const;
  void unexpectedStackSym(char sym, const char* where) const;

  int scanStackForLocation(int iLast);
  void buildVectorImm(std::vector<uchar>& vectorImm,
                      int iFirst, int iLast, bool allowW,
                      Emitter& e);
  enum class PassByRefKind {
    AllowCell,
    WarnOnCell,
    ErrorOnCell,
  };
  PassByRefKind getPassByRefKind(ExpressionPtr exp);
  void emitAGet(Emitter& e);
  void emitCGetL2(Emitter& e);
  void emitCGetL3(Emitter& e);
  void emitPushL(Emitter& e);
  void emitCGet(Emitter& e);
  void emitVGet(Emitter& e);
  void emitIsset(Emitter& e);
  void emitIsType(Emitter& e, IsTypeOp op);
  void emitEmpty(Emitter& e);
  void emitUnset(Emitter& e, ExpressionPtr exp = ExpressionPtr());
  void emitVisitAndUnset(Emitter& e, ExpressionPtr exp);
  void emitSet(Emitter& e);
  void emitSetOp(Emitter& e, int op);
  void emitBind(Emitter& e);
  void emitIncDec(Emitter& e, unsigned char cop);
  void emitPop(Emitter& e);
  void emitConvertToCell(Emitter& e);
  void emitConvertToCellIfVar(Emitter& e);
  void emitConvertToCellOrLoc(Emitter& e);
  void emitConvertSecondToCell(Emitter& e);
  void emitConvertToVar(Emitter& e);
  void emitFPass(Emitter& e, int paramID, PassByRefKind passByRefKind);
  void emitVirtualLocal(int localId, DataType dt = KindOfUnknown);
  template<class Expr> void emitVirtualClassBase(Emitter&, Expr* node);
  void emitResolveClsBase(Emitter& e, int pos);
  void emitClsIfSPropBase(Emitter& e);
  Id emitVisitAndSetUnnamedL(Emitter& e, ExpressionPtr exp);
  Id emitSetUnnamedL(Emitter& e);
  void emitPushAndFreeUnnamedL(Emitter& e, Id tempLocal, Offset start);
  void emitContinuationSwitch(Emitter& e, int ncase);
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
                    ClosureUseVarVec* useVars = nullptr);
  void postponeCtor(InterfaceStatementPtr m, FuncEmitter* fe);
  void postponePinit(InterfaceStatementPtr m, FuncEmitter* fe, NonScalarVec* v);
  void postponeSinit(InterfaceStatementPtr m, FuncEmitter* fe, NonScalarVec* v);
  void postponeCinit(InterfaceStatementPtr m, FuncEmitter* fe, NonScalarVec* v);
  void emitPostponedMeths();
  void bindUserAttributes(MethodStatementPtr meth,
                          FuncEmitter *fe,
                          bool &allowOverride);
  void bindNativeFunc(MethodStatementPtr meth, FuncEmitter *fe);
  void emitMethodMetadata(MethodStatementPtr meth,
                          ClosureUseVarVec* useVars,
                          bool top);
  void fillFuncEmitterParams(FuncEmitter* fe,
                             ExpressionListPtr params,
                             bool builtin = false);
  void emitMethodPrologue(Emitter& e, MethodStatementPtr meth);
  void emitMethod(MethodStatementPtr meth);
  std::pair<FuncEmitter*,bool> createFuncEmitterForGeneratorBody(
                 MethodStatementPtr meth,
                 FuncEmitter* fe,
                 vector<FuncEmitter*>& top_fes);
  void emitAsyncMethod(MethodStatementPtr meth);
  void emitGeneratorCreate(MethodStatementPtr meth);
  void emitGeneratorBody(MethodStatementPtr meth);
  void emitConstMethodCallNoParams(Emitter& e, string name);
  void emitCreateStaticWaitHandle(Emitter& e, std::string cls,
                                  std::function<void()> emitParam);
  void emitSetFuncGetArgs(Emitter& e);
  void emitMethodDVInitializers(Emitter& e,
                                MethodStatementPtr& meth,
                                Label& topOfBody);
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
  Func* canEmitBuiltinCall(const std::string& name, int numParams);
  void emitFuncCall(Emitter& e, FunctionCallPtr node,
                    const char* nameOverride = nullptr,
                    ExpressionListPtr paramsOverride = nullptr);
  void emitFuncCallArg(Emitter& e, ExpressionPtr exp, int paramId);
  void emitBuiltinCallArg(Emitter& e, ExpressionPtr exp, int paramId,
                         bool byRef);
  void emitBuiltinDefaultArg(Emitter& e, Variant& v, DataType t, int paramId);
  void emitClass(Emitter& e, ClassScopePtr cNode, bool topLevel);
  void emitTypedef(Emitter& e, TypedefStatementPtr);
  void emitForeachListAssignment(Emitter& e,
                                 ListAssignmentPtr la,
                                 int vLocalId);
  void emitForeach(Emitter& e,
                   ForEachStatementPtr fe,
                   FinallyRouterEntryPtr entry);
  void emitRestoreErrorReporting(Emitter& e, Id oldLevelLoc);
  void emitMakeUnitFatal(Emitter& e,
                         const char* msg,
                         FatalOp k = FatalOp::Runtime);

  Funclet* addFunclet(Thunklet* body);
  Funclet* addFunclet(StatementPtr stmt,
                      Thunklet* body);
  Funclet* getFunclet(StatementPtr stmt);
  void emitFunclets(Emitter& e);

  struct FaultIterInfo {
    Id iterId;
    IterKind kind;
  };

  void newFaultRegion(Offset start,
                      Offset end,
                      Label* entry,
                      FaultIterInfo = FaultIterInfo { -1, KindOfIter });
  void newFaultRegion(StatementPtr stmt,
                      Offset start,
                      Offset end,
                      Label* entry,
                      FaultIterInfo = FaultIterInfo { -1, KindOfIter });
  void newFuncletAndRegion(Offset start,
                           Offset end,
                           Thunklet* t,
                           FaultIterInfo = FaultIterInfo { -1, KindOfIter });
  void newFuncletAndRegion(StatementPtr stmt,
                           Offset start,
                           Offset end,
                           Thunklet* t,
                           FaultIterInfo = FaultIterInfo { -1, KindOfIter });

  void newFPIRegion(Offset start, Offset end, Offset fpOff);
  void copyOverExnHandlers(FuncEmitter* fe);
  void copyOverFPIRegions(FuncEmitter* fe);
  void saveMaxStackCells(FuncEmitter* fe);
  void finishFunc(Emitter& e, FuncEmitter* fe);

  void initScalar(TypedValue& tvVal, ExpressionPtr val);
  bool requiresDeepInit(ExpressionPtr initExpr) const;

  void emitClassTraitPrecRule(PreClassEmitter* pce, TraitPrecStatementPtr rule);
  void emitClassTraitAliasRule(PreClassEmitter* pce,
                               TraitAliasStatementPtr rule);
  void emitClassUseTrait(PreClassEmitter* pce, UseTraitStatementPtr useStmt);
};

void emitAllHHBC(AnalysisResultPtr ar);

extern "C" {
  String hphp_compiler_serialize_code_model_for(String code, String prefix);
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

#endif // incl_HPHP_COMPILER_EMITTER_H_
