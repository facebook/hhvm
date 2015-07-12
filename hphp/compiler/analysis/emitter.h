/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/compiler/statement/class_require_statement.h"
#include "hphp/compiler/statement/trait_prec_statement.h"
#include "hphp/compiler/statement/trait_alias_statement.h"
#include "hphp/compiler/statement/typedef_statement.h"
#include "hphp/compiler/expression/binary_op_expression.h"
#include "hphp/compiler/expression/object_property_expression.h"
#include "hphp/parser/parser.h"

#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/util/hash.h"

#include <folly/Optional.h>

#include <deque>
#include <utility>
#include <map>
#include <set>
#include <vector>

namespace HPHP {

DECLARE_BOOST_TYPES(AwaitExpression);
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

using OptLocation = folly::Optional<Location::Range>;

class Emitter {
public:
  Emitter(ConstructPtr node, UnitEmitter& ue, EmitterVisitor& ev)
      : m_node(node), m_ue(ue), m_ev(ev) {}
  UnitEmitter& getUnitEmitter() { return m_ue; }
  ConstructPtr getNode() { return m_node; }
  EmitterVisitor& getEmitterVisitor() { return m_ev; }
  void setTempLocation(const OptLocation& r) {
    m_tempLoc = r;
  }
  const OptLocation& getTempLocation() { return m_tempLoc; }
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

#define NA
#define ONE(typ) \
  IMM_##typ
#define TWO(typ1, typ2) \
  IMM_##typ1, IMM_##typ2
#define THREE(typ1, typ2, typ3) \
  IMM_##typ1, IMM_##typ2, IMM_##typ3
#define FOUR(typ1, typ2, typ3, typ4) \
  IMM_##typ1, IMM_##typ2, IMM_##typ3, IMM_##typ4
#define IMM_MA std::vector<unsigned char>
#define IMM_BLA std::vector<Label*>&
#define IMM_SLA std::vector<StrOff>&
#define IMM_ILA std::vector<IterPair>&
#define IMM_IVA int32_t
#define IMM_LA int32_t
#define IMM_IA int32_t
#define IMM_I64A int64_t
#define IMM_DA double
#define IMM_SA const StringData*
#define IMM_RATA RepoAuthType
#define IMM_AA ArrayData*
#define IMM_BA Label&
#define IMM_OA(type) type
#define IMM_VSA std::vector<std::string>&
#define O(name, imm, pop, push, flags) void name(imm);
  OPCODES
#undef O
#undef NA
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef IMM_MA
#undef IMM_BLA
#undef IMM_SLA
#undef IMM_ILA
#undef IMM_IVA
#undef IMM_LA
#undef IMM_IA
#undef IMM_I64A
#undef IMM_DA
#undef IMM_SA
#undef IMM_RATA
#undef IMM_AA
#undef IMM_BA
#undef IMM_OA
#undef IMM_VSA

private:
  ConstructPtr m_node;
  UnitEmitter& m_ue;
  EmitterVisitor& m_ev;
  OptLocation m_tempLoc;
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
      , className(nullptr)
      , intval(-1)
      , unnamedLocalStart(InvalidAbsoluteOffset)
      , clsBaseType(CLS_INVALID)
    {}
    char sym;
    MetaType metaType;
    union {
      const StringData* name;   // META_LITSTR
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
  void cleanTopMeta();
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

DECLARE_BOOST_TYPES(ControlTarget);
/*
 * The structure represents a code path that potentially requires
 * running finally blocks. A code path has an assigned state ID that
 * is used inside switch statements emitted at the end of finally
 * blocks. It also has an optional label (the destination to jump
 * to after all the required finally blocks are run).
 */
struct ControlTarget {
  static const int k_unsetState;
  explicit ControlTarget(EmitterVisitor* router);
  ~ControlTarget();
  // Manage state ID reuse.
  bool isRegistered();
  EmitterVisitor* m_visitor;
  // The target to jump to once all the necessary finally blocks are run.
  Label m_label;
  // The state ID that identifies this control target inside finally
  // epilogues. This ID assigned to the "state" unnamed local variable.
  int m_state;
};

struct ControlTargetInfo {
  ControlTargetInfo() : used(false) {}
  ControlTargetInfo(ControlTargetPtr t, bool b) : target(t), used(b) {}
  ControlTargetPtr target;
  bool used;
};

DECLARE_BOOST_TYPES(Region);
/*
 * Region represents a single level of the unified stack
 * of constructs that are meaningful from the point of view of finally
 * implementation. The levels are used to keep track of the information
 * such as the control targets that can be taken inside a block.
 */
class Region {
public:
  enum Kind {
    // Top-level (global) context.
    Global,
    // Function body / method body entry.
    FuncBody,
    // Entry for finally fault funclets emitted after the body of
    // a function
    FaultFunclet,
    // Region by a finally clause
    TryFinally,
    // Finally block entry (begins after catches ends after finally)
    Finally,
    // Loop or switch statement.
    LoopOrSwitch,
  };

  typedef Emitter::IterPair IterPair;
  typedef std::vector<IterPair> IterVec;

  Region(Region::Kind kind, RegionPtr parent);

  // Helper for establishing the maximal depth of break / continue
  // control targets that are allocated.
  int getBreakContinueDepth();

  // Returns the maximal break / continue depth admissable (aka the
  // number of nested loops).
  int getMaxBreakContinueDepth();

  int getMaxState();

  // The number of cases to be emitted. This is a helper used in
  // establishing whether one of the optimized cases can be used.
  int getCaseCount();

  bool isForeach() { return m_iterId != -1; }
  bool isTryFinally() { return m_kind == Region::Kind::TryFinally; }
  bool isFinally() { return m_kind == Region::Kind::Finally; }

  bool isBreakUsed(int i) {
    auto it = m_breakTargets.find(i);
    if (it == m_breakTargets.end()) return false;
    return it->second.used;
  }

  bool isContinueUsed(int i) {
    auto it = m_continueTargets.find(i);
    if (it == m_continueTargets.end()) return false;
    return it->second.used;
  }

  Region::Kind m_kind;
  // Only used for loop / break kind of entries.
  Id m_iterId;
  IterKind m_iterKind;
  // Because of a bug in code emission, functions sometimes have
  // inconsistent return flavors. Therefore instead of a single
  // return control target, there need to be one return control
  // target per flavor used. Once the bug is removed, this code
  // can be simplified.
  std::map<char, ControlTargetInfo> m_returnTargets;
  // Break and continue control targets identified by their depth.
  std::map<int, ControlTargetInfo> m_breakTargets;
  std::map<int, ControlTargetInfo> m_continueTargets;
  // Goto control targets. Each goto control target is identified
  // by the name of the destination label.
  std::map<StringData*, ControlTargetInfo, string_data_lt> m_gotoTargets;
  // A set of goto labels occurrning inside the statement represented
  // by this entry. This value is used for establishing whether
  // a finally block needs to be executed when performing gotos.
  std::set<StringData*, string_data_lt> m_gotoLabels;
  // The label denoting the beginning of a finally block inside the
  // current try. Only used when the entry kind is a try statement.
  Label m_finallyLabel;
  // The parent entry.
  RegionPtr m_parent;
};

class EmitterVisitor {
  friend class UnsetUnnamedLocalThunklet;
  friend class FuncFinisher;
public:
  typedef std::vector<int> IndexChain;
  typedef Emitter::IterPair IterPair;
  typedef std::vector<IterPair> IterVec;

  explicit EmitterVisitor(UnitEmitter& ue);
  ~EmitterVisitor();

  bool visit(ConstructPtr c);
  void visitKids(ConstructPtr c);
  void visit(FileScopePtr file);
  void assignLocalVariableIds(FunctionScopePtr fs);
  void assignFinallyVariableIds();
  void fixReturnType(Emitter& e, FunctionCallPtr fn,
                     Func* builtinFunc = nullptr);

  void listAssignmentVisitLHS(Emitter& e, ExpressionPtr exp,
                              IndexChain& indexChain,
                              std::vector<IndexChain*>& chainList);
  void listAssignmentAssignElements(Emitter& e,
                                    std::vector<IndexChain*>& indexChains,
                                    std::function<void()> emitSrc);

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
    return (m_ue.bcPos() == m_curFunc->base
            || isJumpTarget(m_ue.bcPos())
            || (instrFlags(getPrevOpcode()) & TF) == 0);
  }
  FuncEmitter* getFuncEmitter() { return m_curFunc; }
  Id getStateLocal() {
    assert(m_stateLocal >= 0);
    return m_stateLocal;
  }
  Id getRetLocal() {
    assert(m_retLocal >= 0);
    return m_retLocal;
  }

  struct IncludeTimeFatalException : Exception {
    ConstructPtr m_node;
    bool m_parseFatal;
    IncludeTimeFatalException(ConstructPtr node, const char* fmt, ...)
        : Exception(), m_node(node), m_parseFatal(false) {
      va_list ap; va_start(ap, fmt); format(fmt, ap); va_end(ap);
    }
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

  class CatchRegion {
  public:
    CatchRegion(Offset start, Offset end) : m_start(start),
      m_end(end) {}
    ~CatchRegion() {
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
  static constexpr size_t kMinStringSwitchCases = 8;

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
  hphp_hash_map<std::string, FuncEmitter*> m_topMethodEmitted;
  SymbolicStack m_evalStack;
  bool m_evalStackIsUnknown;
  hphp_hash_map<Offset, SymbolicStack> m_jumpTargetEvalStacks;
  int m_actualStackHighWater;
  int m_fdescHighWater;
  typedef tbb::concurrent_hash_map<const StringData*, int,
                                   StringDataHashICompare> EmittedClosures;
  static EmittedClosures s_emittedClosures;
  std::deque<Funclet*> m_funclets;
  std::map<StatementPtr, Funclet*> m_memoizedFunclets;
  std::deque<CatchRegion*> m_catchRegions;
  std::deque<FaultRegion*> m_faultRegions;
  std::deque<FPIRegion*> m_fpiRegions;
  std::vector<Array> m_staticArrays;
  std::vector<folly::Optional<CollectionType>> m_staticColType;
  std::set<std::string,stdltistr> m_hoistables;
  OptLocation m_tempLoc;
  std::unordered_set<std::string> m_staticEmitted;

  // The stack of all Regions that this EmitterVisitor is currently inside
  std::vector<RegionPtr> m_regions;
  // The state IDs currently allocated for the "finally router" logic.
  // See FIXME above the registerControlTarget() method.
  std::set<int> m_states;
  // Unnamed local variables used by the "finally router" logic
  Id m_stateLocal;
  Id m_retLocal;

public:
  bool checkIfStackEmpty(const char* forInstruction) const;
  void unexpectedStackSym(char sym, const char* where) const;

  int scanStackForLocation(int iLast);
  void buildVectorImm(std::vector<unsigned char>& vectorImm,
                      int iFirst, int iLast, bool allowW,
                      Emitter& e);
  enum class PassByRefKind {
    AllowCell,
    WarnOnCell,
    ErrorOnCell,
  };
  PassByRefKind getPassByRefKind(ExpressionPtr exp);
  void emitCall(Emitter& e, FunctionCallPtr func,
                ExpressionListPtr params, Offset fpiStart);
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
  void emitIncDec(Emitter& e, IncDecOp cop);
  void emitPop(Emitter& e);
  void emitConvertToCell(Emitter& e);
  void emitConvertToCellIfVar(Emitter& e);
  void emitConvertToCellOrLoc(Emitter& e);
  void emitConvertSecondToCell(Emitter& e);
  void emitConvertToVar(Emitter& e);
  void emitFPass(Emitter& e, int paramID, PassByRefKind passByRefKind);
  void emitVirtualLocal(int localId);
  template<class Expr> void emitVirtualClassBase(Emitter&, Expr* node);
  void emitResolveClsBase(Emitter& e, int pos);
  void emitClsIfSPropBase(Emitter& e);
  Id emitVisitAndSetUnnamedL(Emitter& e, ExpressionPtr exp);
  Id emitSetUnnamedL(Emitter& e);
  void emitPushAndFreeUnnamedL(Emitter& e, Id tempLocal, Offset start);
  MaybeDataType analyzeSwitch(SwitchStatementPtr s, SwitchState& state);
  void emitIntegerSwitch(Emitter& e, SwitchStatementPtr s,
                         std::vector<Label>& caseLabels, Label& done,
                         const SwitchState& state);
  void emitStringSwitch(Emitter& e, SwitchStatementPtr s,
                        std::vector<Label>& caseLabels, Label& done,
                        const SwitchState& state);
  void emitArrayInit(Emitter& e, ExpressionListPtr el,
                     folly::Optional<CollectionType> ct = folly::none);
  void emitPairInit(Emitter&e, ExpressionListPtr el);
  void emitVectorInit(Emitter&e, CollectionType ct, ExpressionListPtr el);
  void emitMapInit(Emitter&e, CollectionType ct, ExpressionListPtr el);
  void emitSetInit(Emitter&e, CollectionType ct, ExpressionListPtr el);
  void emitCollectionInit(Emitter& e, BinaryOpExpressionPtr exp);
  void markElem(Emitter& e);
  void markNewElem(Emitter& e);
  void markProp(Emitter& e, PropAccessType propAccessType);
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
  int32_t emitNativeOpCodeImpl(MethodStatementPtr meth,
                               const char* funcName,
                               const char* className,
                               FuncEmitter* fe);
  void emitMethodMetadata(MethodStatementPtr meth,
                          ClosureUseVarVec* useVars,
                          bool top);
  void fillFuncEmitterParams(FuncEmitter* fe,
                             ExpressionListPtr params,
                             bool coerce_params = false);
  void emitMethodPrologue(Emitter& e, MethodStatementPtr meth);
  void emitDeprecationWarning(Emitter& e, MethodStatementPtr meth);
  void emitMethod(MethodStatementPtr meth);
  void emitMemoizeProp(Emitter& e, MethodStatementPtr meth, Id localID,
                       const std::vector<Id>& paramIDs, uint32_t numParams);
  void addMemoizeProp(MethodStatementPtr meth);
  void emitMemoizeMethod(MethodStatementPtr meth, const StringData* methName);
  void emitConstMethodCallNoParams(Emitter& e, string name);
  bool emitHHInvariant(Emitter& e, SimpleFunctionCallPtr);
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

  bool emitSystemLibVarEnvFunc(Emitter& e, SimpleFunctionCallPtr node);
  bool emitCallUserFunc(Emitter& e, SimpleFunctionCallPtr node);
  Func* canEmitBuiltinCall(const std::string& name, int numParams);
  void emitFuncCall(Emitter& e, FunctionCallPtr node,
                    const char* nameOverride = nullptr,
                    ExpressionListPtr paramsOverride = nullptr);
  void emitFuncCallArg(Emitter& e, ExpressionPtr exp, int paramId,
                       bool isUnpack);
  void emitBuiltinCallArg(Emitter& e, ExpressionPtr exp, int paramId,
                         bool byRef);
  void emitLambdaCaptureArg(Emitter& e, ExpressionPtr exp);
  void emitBuiltinDefaultArg(Emitter& e, Variant& v,
                             MaybeDataType t, int paramId);
  void emitClass(Emitter& e, ClassScopePtr cNode, bool topLevel);
  void emitTypedef(Emitter& e, TypedefStatementPtr);
  void emitForeachListAssignment(Emitter& e,
                                 ListAssignmentPtr la,
                                 std::function<void()> emitSrc);
  void emitForeach(Emitter& e, ForEachStatementPtr fe);
  void emitForeachAwaitAs(Emitter& e, ForEachStatementPtr fe);
  void emitRestoreErrorReporting(Emitter& e, Id oldLevelLoc);
  void emitMakeUnitFatal(Emitter& e,
                         const char* msg,
                         FatalOp k = FatalOp::Runtime);

  // Emits a Jmp or IterBreak instruction to the specified target, freeing
  // the specified iterator variables. emitJump() cannot be used to leave a
  // try region, except if it jumps to the m_finallyLabel of the try region.
  void emitJump(Emitter& e, IterVec& iters, Label& target);

  // These methods handle the return, break, continue, and goto operations.
  // These methods are aware of try/finally blocks and foreach blocks and
  // will free iterators and jump to finally epilogues as appropriate.
  void emitReturn(Emitter& e, char sym, StatementPtr s);
  void emitBreak(Emitter& e, int depth, StatementPtr s);
  void emitContinue(Emitter& e, int depth, StatementPtr s);
  void emitGoto(Emitter& e, StringData* name, StatementPtr s);

  // Helper methods for emitting IterFree instructions
  void emitIterFree(Emitter& e, IterVec& iters);
  void emitIterFreeForReturn(Emitter& e);

  // A "finally epilogue" is a blob of bytecode that comes after an inline
  // copy of a "finally" clause body. Finally epilogues are used to ensure
  // that that the bodies of finally clauses are executed whenever a return,
  // break, continue, or goto operation jumps out of their corresponding
  // "try" blocks.
  void emitFinallyEpilogue(Emitter& e, Region* entry);
  void emitReturnTrampoline(Emitter& e, Region* entry,
                            std::vector<Label*>& cases, char sym);
  void emitBreakTrampoline(Emitter& e, Region* entry,
                           std::vector<Label*>& cases, int depth);
  void emitContinueTrampoline(Emitter& e, Region* entry,
                              std::vector<Label*>& cases, int depth);
  void emitGotoTrampoline(Emitter& e, Region* entry,
                          std::vector<Label*>& cases, StringData* name);

  // Returns true if VerifyRetType should be emitted before Ret for
  // the current function.
  bool shouldEmitVerifyRetType();

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
  void
  newFaultRegionAndFunclet(Offset start,
                           Offset end,
                           Thunklet* t,
                           FaultIterInfo = FaultIterInfo { -1, KindOfIter });
  void
  newFaultRegionAndFunclet(StatementPtr stmt,
                           Offset start,
                           Offset end,
                           Thunklet* t,
                           FaultIterInfo = FaultIterInfo { -1, KindOfIter });

  void newFPIRegion(Offset start, Offset end, Offset fpOff);
  void copyOverCatchAndFaultRegions(FuncEmitter* fe);
  void copyOverFPIRegions(FuncEmitter* fe);
  void saveMaxStackCells(FuncEmitter* fe, int32_t stackPad);
  void finishFunc(Emitter& e, FuncEmitter* fe, int32_t stackPad);
  void initScalar(TypedValue& tvVal, ExpressionPtr val,
                  folly::Optional<CollectionType> ct = folly::none);
  bool requiresDeepInit(ExpressionPtr initExpr) const;

  void emitClassTraitPrecRule(PreClassEmitter* pce, TraitPrecStatementPtr rule);
  void emitClassTraitAliasRule(PreClassEmitter* pce,
                               TraitAliasStatementPtr rule);
  void emitClassUseTrait(PreClassEmitter* pce, UseTraitStatementPtr useStmt);

  // Helper function for creating entries.
  RegionPtr createRegion(StatementPtr s, Region::Kind kind);
  // Enter/leave the passed in entry. Note that entries sometimes need be
  // to be constructed before they are entered, or need to be accessed
  // after they are left. This especially applies to constructs such
  // as loops and try blocks.
  void enterRegion(RegionPtr);
  void leaveRegion(RegionPtr);

  // Functions used for handling state IDs allocation.
  // FIXME (#3275259): This should be moved into global / func
  // body / fault funclet entries in order to optimize state
  // allocation. See the task description for more details.
  void registerControlTarget(ControlTarget* t);
  void unregisterControlTarget(ControlTarget* t);

  void registerReturn(StatementPtr s, Region* entry, char sym);
  void registerYieldAwait(ExpressionPtr e);
  ControlTargetPtr registerBreak(StatementPtr s, Region* entry, int depth,
                                 bool alloc);
  ControlTargetPtr registerContinue(StatementPtr s, Region* entry, int depth,
                                    bool alloc);
  ControlTargetPtr registerGoto(StatementPtr s, Region* entry,
                                StringData* name, bool alloc);
};

void emitAllHHBC(AnalysisResultPtr&& ar);


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

#endif // incl_HPHP_COMPILER_EMITTER_H_
