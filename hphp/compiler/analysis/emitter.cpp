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

#include "hphp/compiler/analysis/emitter.h"

#include <algorithm>
#include <atomic>
#include <deque>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include <boost/algorithm/string/predicate.hpp>

#include <folly/MapUtil.h>
#include <folly/Memory.h>
#include <folly/ScopeGuard.h>
#ifndef _MSC_VER
#include <folly/Subprocess.h>
#endif
#include <folly/String.h>

#include "hphp/compiler/builtin_symbols.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/expression/array_element_expression.h"
#include "hphp/compiler/expression/array_pair_expression.h"
#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/expression/binary_op_expression.h"
#include "hphp/compiler/expression/class_constant_expression.h"
#include "hphp/compiler/expression/class_expression.h"
#include "hphp/compiler/expression/closure_expression.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/expression/dynamic_variable.h"
#include "hphp/compiler/expression/encaps_list_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/include_expression.h"
#include "hphp/compiler/expression/list_assignment.h"
#include "hphp/compiler/expression/modifier_expression.h"
#include "hphp/compiler/expression/new_object_expression.h"
#include "hphp/compiler/expression/object_method_expression.h"
#include "hphp/compiler/expression/parameter_expression.h"
#include "hphp/compiler/expression/qop_expression.h"
#include "hphp/compiler/expression/null_coalesce_expression.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/expression/static_member_expression.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/compiler/expression/yield_expression.h"
#include "hphp/compiler/expression/yield_from_expression.h"
#include "hphp/compiler/expression/await_expression.h"
#include "hphp/compiler/statement/block_statement.h"
#include "hphp/compiler/statement/break_statement.h"
#include "hphp/compiler/statement/case_statement.h"
#include "hphp/compiler/statement/catch_statement.h"
#include "hphp/compiler/statement/class_constant.h"
#include "hphp/compiler/statement/class_variable.h"
#include "hphp/compiler/statement/do_statement.h"
#include "hphp/compiler/statement/echo_statement.h"
#include "hphp/compiler/statement/exp_statement.h"
#include "hphp/compiler/statement/for_statement.h"
#include "hphp/compiler/statement/foreach_statement.h"
#include "hphp/compiler/statement/finally_statement.h"
#include "hphp/compiler/statement/function_statement.h"
#include "hphp/compiler/statement/global_statement.h"
#include "hphp/compiler/statement/goto_statement.h"
#include "hphp/compiler/statement/if_branch_statement.h"
#include "hphp/compiler/statement/if_statement.h"
#include "hphp/compiler/statement/label_statement.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/return_statement.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/statement/static_statement.h"
#include "hphp/compiler/statement/switch_statement.h"
#include "hphp/compiler/statement/try_statement.h"
#include "hphp/compiler/statement/unset_statement.h"
#include "hphp/compiler/statement/while_statement.h"
#include "hphp/compiler/statement/use_trait_statement.h"
#include "hphp/compiler/statement/class_require_statement.h"
#include "hphp/compiler/statement/trait_prec_statement.h"
#include "hphp/compiler/statement/trait_alias_statement.h"
#include "hphp/compiler/statement/typedef_statement.h"
#include "hphp/compiler/statement/declare_statement.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/hhbbc/hhbbc.h"
#include "hphp/hhbbc/parallel.h"

#include "hphp/util/trace.h"
#include "hphp/util/safe-cast.h"
#include "hphp/util/logger.h"
#include "hphp/util/job-queue.h"
#include "hphp/util/match.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/extern-compiler.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/user-attributes.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/runtime.h"

#include "hphp/system/systemlib.h"

namespace HPHP {

DECLARE_BOOST_TYPES(AwaitExpression);
DECLARE_BOOST_TYPES(ClosureExpression);
DECLARE_BOOST_TYPES(FileScope);
DECLARE_BOOST_TYPES(ForEachStatement);
DECLARE_BOOST_TYPES(FunctionCall);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(InterfaceStatement);
DECLARE_BOOST_TYPES(ListAssignment);
DECLARE_BOOST_TYPES(MethodStatement);
DECLARE_BOOST_TYPES(SimpleFunctionCall);
DECLARE_BOOST_TYPES(SwitchStatement);

namespace Compiler {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(emitter);

const StaticString
  s_ini_get("ini_get"),
  s_is_deprecated("deprecated function"),
  s_trigger_error("trigger_error"),
  s_trigger_sampled_error("trigger_sampled_error"),
  s_zend_assertions("zend.assertions");

struct Label;
struct EmitterVisitor;

using OptLocation = folly::Optional<Location::Range>;

namespace StackSym {
  static const char None = 0x00;

  /*
   * We don't actually track the U flavor (we treat it as a C),
   * because there's nothing important to do with it for emission.
   * The verifier will check they are only created at the appropriate
   * times.
   */
  static const char C = 0x01; // Cell symbolic flavor
  static const char V = 0x02; // Ref symbolic flavor
  static const char A = 0x03; // Classref symbolic flavor
  static const char R = 0x04; // Return value symbolic flavor
  static const char F = 0x05; // Function argument symbolic flavor
  static const char L = 0x06; // Local symbolic flavor
  static const char T = 0x07; // String literal symbolic flavor
  static const char I = 0x08; // int literal symbolic flavor
  static const char H = 0x09; // $this symbolic flavor

  static const char N = 0x10; // Name marker
  static const char G = 0x20; // Global name marker
  static const char E = 0x30; // Element marker
  static const char W = 0x40; // New element marker
  static const char P = 0x50; // Property marker
  static const char S = 0x60; // Static property marker
  static const char M = 0x70; // Non elem/prop/W part of M-vector
  static const char K = (char)0x80u; // Class base marker
  static const char Q = (char)0x90u; // NullSafe Property marker

  static const char CN = C | N;
  static const char CG = C | G;
  static const char CS = C | S;
  static const char LN = L | N;
  static const char LG = L | G;
  static const char LS = L | S;
  static const char AM = A | M;

  char GetSymFlavor(char sym) { return (sym & 0x0F); }
  char GetMarker(char sym) { return (sym & 0xF0); }

  /*
   * Return whether or not sym represents a symbolic stack element, rather than
   * an actual stack element. Symbolic stack elements do not have corresponding
   * values on the real eval stack at runtime, and represent things like local
   * variable ids or literal ints and strings.
   */
  bool IsSymbolic(char sym) {
    auto const flavor = GetSymFlavor(sym);
    if (flavor == L || flavor == T || flavor == I || flavor == H ||
        flavor == A) {
      return true;
    }

    auto const marker = GetMarker(sym);
    if (marker == W || marker == K) return true;

    return false;
  }

  std::string ToString(char sym) {
    char symFlavor = StackSym::GetSymFlavor(sym);
    std::string res;
    switch (symFlavor) {
      case StackSym::C: res = "C"; break;
      case StackSym::V: res = "V"; break;
      case StackSym::A: res = "A"; break;
      case StackSym::R: res = "R"; break;
      case StackSym::F: res = "F"; break;
      case StackSym::L: res = "L"; break;
      case StackSym::T: res = "T"; break;
      case StackSym::I: res = "I"; break;
      case StackSym::H: res = "H"; break;
      default: break;
    }
    char marker = StackSym::GetMarker(sym);
    switch (marker) {
      case StackSym::N: res += "N"; break;
      case StackSym::G: res += "G"; break;
      case StackSym::E: res += "E"; break;
      case StackSym::W: res += "W"; break;
      case StackSym::P: res += "P"; break;
      case StackSym::Q: res += "Q"; break;
      case StackSym::S: res += "S"; break;
      case StackSym::K: res += "K"; break;
      case StackSym::M: res += "M"; break;
      default: break;
    }
    if (res == "") {
      if (sym == StackSym::None) {
        res = "None";
      } else {
        res = "?";
      }
    }
    return res;
  }
}

namespace {

// Placeholder for class-ref slot immediates when emitting a bytecode op. The
// emitter takes care of assigning slots automatically so you do not need to
// specify one.
struct ClsRefSlotPlaceholder {};
constexpr const ClsRefSlotPlaceholder kClsRefSlotPlaceholder{};

}

struct Emitter {
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
#define IMM_BLA std::vector<Label*>&
#define IMM_SLA std::vector<StrOff>&
#define IMM_ILA std::vector<IterPair>&
#define IMM_IVA uint32_t
#define IMM_LA int32_t
#define IMM_IA int32_t
#define IMM_CAR ClsRefSlotPlaceholder
#define IMM_CAW ClsRefSlotPlaceholder
#define IMM_I64A int64_t
#define IMM_DA double
#define IMM_SA const StringData*
#define IMM_RATA RepoAuthType
#define IMM_AA ArrayData*
#define IMM_BA Label&
#define IMM_OA(type) type
#define IMM_VSA std::vector<std::string>&
#define IMM_KA MemberKey
#define IMM_LAR LocalRange
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
#undef IMM_CAR
#undef IMM_CAW
#undef IMM_I64A
#undef IMM_DA
#undef IMM_SA
#undef IMM_RATA
#undef IMM_AA
#undef IMM_BA
#undef IMM_OA
#undef IMM_VSA
#undef IMM_KA
#undef IMM_LAR

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
      , name(nullptr)
      , className(nullptr)
      , intval(-1)
      , unnamedLocalStart(InvalidAbsoluteOffset)
      , clsBaseType(CLS_INVALID)
    {}
    char sym;
    const StringData* name;
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

    std::string pretty() const;
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

  // The next class-ref slot to be allocated.
  int m_clsRefTop = 0;
  // The highest class-ref slot allocated plus 1 (which sizes the number of
  // slots needed).
  int m_maxClsRefCount = 0;
public:
  int* m_actualStackHighWaterPtr;

  SymbolicStack() : m_fdescCount(0) {}

  std::string pretty() const;

  void updateHighWater();
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
  size_t size() const;
  size_t actualSize() const;
  size_t fdescSize() const { return m_fdescCount; }
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
  void symbolicInsertAt(int depth, char sym);
  void pushSkipSymbolicTop(char sym);
  int sizeActual() const;

  ClassBaseType getClsBaseType(int index) const;
  int getLoc(int index) const;
  int64_t getInt(int index) const;
  Offset getUnnamedLocStart(int index) const;

  void pushFDesc();
  void popFDesc();

  // Class-ref slots are allocated in a stack-like manner. We don't need an
  // actual stack because the slots are always allocated in order.
  int pushClsRefSlot() {
    auto slot = m_clsRefTop++;
    m_maxClsRefCount = std::max(m_clsRefTop, m_maxClsRefCount);
    return slot;
  }
  int popClsRefSlot() { return --m_clsRefTop; }
  int peekClsRefSlot() const { return m_clsRefTop; }
  int clsRefSlotStackEmpty() const { return m_clsRefTop <= 0; }
  int numClsRefSlots() const { return m_maxClsRefCount; }
  void setNumClsRefSlots(int val) { m_maxClsRefCount = val; }
};

struct Label {
  enum class NoEntryNopFlag {};
  Label() : m_off(InvalidAbsoluteOffset) {}
  explicit Label(Emitter& e) : m_off(InvalidAbsoluteOffset) {
    set(e, true);
  }
  Label(Emitter& e, NoEntryNopFlag) : m_off(InvalidAbsoluteOffset) {
    set(e, false);
  }
  Offset getAbsoluteOffset() const { return m_off; }
  // Sets the Label to the bytecode offset of given by e,
  // fixes up any instructions that have already been
  // emitted that reference this Label, and fixes up the
  // EmitterVisitor's jump target info
  void set(Emitter& e, bool emitNopAtEntry = true);
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

struct Thunklet {
  virtual ~Thunklet();
  virtual void emit(Emitter& e) = 0;
};

struct Funclet {
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
struct Region {
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

struct OpEmitContext;

struct EmitterVisitor {
  friend struct UnsetUnnamedLocalThunklet;
  friend struct FuncFinisher;
public:
  typedef std::vector<int> IndexChain;
  typedef std::pair<ExpressionPtr, IndexChain> IndexPair;
  typedef Emitter::IterPair IterPair;
  typedef std::vector<IterPair> IterVec;

  explicit EmitterVisitor(UnitEmitter& ue);
  ~EmitterVisitor();

  bool visit(ConstructPtr c);
  void visitKids(ConstructPtr c);
  void visit(FileScopePtr file);
  void assignLocalVariableIds(FunctionScopePtr fs);
  void assignFinallyVariableIds();

  void listAssignmentVisitLHS(Emitter& e, ExpressionPtr exp,
                              IndexChain& indexChain,
                              std::vector<IndexPair>& chainList);
  void listAssignmentAssignElements(Emitter& e,
                                    std::vector<IndexPair>& indexChains,
                                    std::function<void()> emitSrc);

  void visitIfCondition(ExpressionPtr cond, Emitter& e, Label& tru, Label& fals,
                        bool truFallthrough);
  const SymbolicStack& getEvalStack() const { return m_evalStack; }
  SymbolicStack& getEvalStack() { return m_evalStack; }
  void setEvalStack(const SymbolicStack& es) {
    auto const s = m_evalStack.numClsRefSlots();
    m_evalStack = es;
    m_evalStack.setNumClsRefSlots(std::max(s, es.numClsRefSlots()));
    m_evalStackIsUnknown = false;
  }
  bool evalStackIsUnknown() { return m_evalStackIsUnknown; }
  void popEvalStack(char symFlavor);
  int popClsRefSlot();
  void popSymbolicLocal(OpEmitContext& ctx);
  void popSymbolicClassRef(OpEmitContext& ctx);
  void popEvalStackMMany();
  void popEvalStackMany(int len, char symFlavor);
  void popEvalStackCVMany(int len);
  void pushEvalStack(char symFlavor);
  void pushEvalStackFromOp(char symFlavor, OpEmitContext& ctx);
  void insertEvalStackFromOp(char symFlavor, int depth, OpEmitContext& ctx);
  void pushSymbolicClassRef();
  int pushClsRefSlot();
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
  void setPrevOpcode(Op op) {
    m_prevOpcode.emplace(op);
  }
  bool currentPositionIsReachable() {
    return m_ue.bcPos() == m_curFunc->base ||
           isJumpTarget(m_ue.bcPos()) ||
           (m_prevOpcode.hasValue() &&
            ((instrFlags(m_prevOpcode.value()) & TF) == 0 ||
             m_prevOpcode.value() == OpExit));
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

  struct PostponedMeth {
    PostponedMeth(MethodStatementPtr m, FuncEmitter* fe, bool top,
                  ClosureUseVarVec* useVars)
        : m_meth(m), m_fe(fe), m_top(top), m_closureUseVars(useVars) {}
    MethodStatementPtr m_meth;
    FuncEmitter* m_fe;
    bool m_top;
    ClosureUseVarVec* m_closureUseVars;
  };

  struct PostponedCtor {
    PostponedCtor(InterfaceStatementPtr is, FuncEmitter* fe)
      : m_is(is), m_fe(fe) {}
    InterfaceStatementPtr m_is;
    FuncEmitter* m_fe;
  };

  struct PostponedNonScalars {
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

  struct PostponedClosureCtor {
    PostponedClosureCtor(ClosureUseVarVec& v, ClosureExpressionPtr e,
                         FuncEmitter* fe)
        : m_useVars(v), m_expr(e), m_fe(fe) {}
    ClosureUseVarVec m_useVars;
    ClosureExpressionPtr m_expr;
    FuncEmitter* m_fe;
  };

  struct CatchRegion {
    CatchRegion(Offset start,
                Offset handler,
                Offset end,
                Id iterId,
                IterKind kind)
      : m_start(start)
      , m_handler(handler)
      , m_end(end)
      , m_iterId(iterId)
      , m_iterKind(kind) {}

    Offset m_start;
    Offset m_handler;
    Offset m_end;
    Id m_iterId;
    IterKind m_iterKind;
  };

  struct FaultRegion {
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

  struct FPIRegion {
      FPIRegion(Offset start, Offset end, Offset fpOff)
        : m_start(start), m_end(end), m_fpOff(fpOff) {}
      Offset m_start;
      Offset m_end;
      Offset m_fpOff;
  };

  void allocPipeLocal(Id pipeVar) { m_pipeVars.emplace(pipeVar); }
  void releasePipeLocal(Id pipeVar) {
    assert(!m_pipeVars.empty() && m_pipeVars.top() == pipeVar);
    m_pipeVars.pop();
  }
  folly::Optional<Id> getPipeLocal() {
    if (m_pipeVars.empty()) return folly::none;
    return m_pipeVars.top();
  }

private:
  UnitEmitter& m_ue;
  FuncEmitter* m_curFunc;
  FileScopePtr m_file;

  folly::Optional<Op> m_prevOpcode;

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
  std::deque<Funclet*> m_funclets;
  std::map<StatementPtr, Funclet*> m_memoizedFunclets;
  std::deque<CatchRegion*> m_catchRegions;
  std::deque<FaultRegion*> m_faultRegions;
  std::deque<FPIRegion*> m_fpiRegions;
  std::vector<Array> m_staticArrays;
  std::vector<folly::Optional<HeaderKind>> m_staticColType;
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
  // stack of nested unnamed pipe variables
  std::stack<Id> m_pipeVars;

public:
  bool checkIfStackEmpty(const char* forInstruction) const;
  void unexpectedStackSym(char sym, const char* where) const;

  int scanStackForLocation(int iLast);

  /*
   * Emit bytecodes for the base and intermediate dims, returning the number of
   * eval stack slots containing member keys that should be consumed by the
   * final operation.
   */
  struct MInstrOpts {
    explicit MInstrOpts(MOpMode mode)
      : allowW{mode == MOpMode::Define}
      , mode{mode}
    {}

    explicit MInstrOpts(int32_t paramId)
      : allowW{true}
      , fpass{true}
      , paramId{paramId}
    {}

    MInstrOpts& rhs() {
      rhsVal = true;
      return *this;
    }

    bool allowW{false};
    bool rhsVal{false};
    bool fpass{false};
    union {
      MOpMode mode;
      int32_t paramId;
    };
  };

  MemberKey symToMemberKey(Emitter& e, int i, bool allowW);
  size_t emitMOp(int iFirst, int& iLast, Emitter& e,
                 MInstrOpts opts, bool includeLast = false);
  void emitQueryMOp(int iFirst, int iLast, Emitter& e, QueryMOp op);

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
  void emitPushL(Emitter& e);
  void emitCGet(Emitter& e);
  void emitCGetQuiet(Emitter& e);
  bool emitVGet(Emitter& e, bool skipCells = false);
  void emitIsset(Emitter& e);
  void emitIsType(Emitter& e, IsTypeOp op);
  void emitEmpty(Emitter& e);
  void emitUnset(Emitter& e, ExpressionPtr exp = ExpressionPtr());
  void emitUnsetL(Emitter& e, Id local);
  void emitVisitAndUnset(Emitter& e, ExpressionPtr exp);
  void emitSet(Emitter& e);
  void emitSetL(Emitter& e, Id local);
  void emitSetOp(Emitter& e, int op);
  void emitBind(Emitter& e);
  void emitBindL(Emitter& e, Id local);
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
  void emitFreeUnnamedL(Emitter& e, Id tempLocal, Offset start);
  void emitPushAndFreeUnnamedL(Emitter& e, Id tempLocal, Offset start);
  void emitArrayInit(Emitter& e, ExpressionListPtr el,
                     folly::Optional<HeaderKind> ct = folly::none,
                     bool isDictForSetCollection = false);
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
                          FuncEmitter *fe);
  Attr bindNativeFunc(MethodStatementPtr meth,
                      FuncEmitter *fe,
                      bool dynCallWrapper);
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
  void addMemoizeProp(MethodStatementPtr meth);
  void emitMemoizeMethod(MethodStatementPtr meth, const StringData* methName);
  void emitConstMethodCallNoParams(Emitter& e, const std::string& name);
  bool emitInlineGen(Emitter& e, const ExpressionPtr&);
  bool emitInlineGena(Emitter& e, const SimpleFunctionCallPtr& call);
  bool emitInlineGenva(Emitter& e, const SimpleFunctionCallPtr& call);
  bool emitInlineHHAS(Emitter& e, SimpleFunctionCallPtr);
  bool emitHHInvariant(Emitter& e, SimpleFunctionCallPtr);
  void emitMethodDVInitializers(Emitter& e,
                                FuncEmitter* fe,
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
  void emitFuncCall(Emitter& e, FunctionCallPtr node,
                    const char* nameOverride = nullptr,
                    ExpressionListPtr paramsOverride = nullptr);
  bool emitConstantFuncCall(Emitter& e, SimpleFunctionCallPtr call);
  void emitFuncCallArg(Emitter& e, ExpressionPtr exp, int paramId,
                       bool isUnpack);
  void emitClosureUseVar(Emitter& e, ExpressionPtr exp, int paramId,
                         bool byRef);
  bool emitScalarValue(Emitter& e, const Variant& value);
  void emitLambdaCaptureArg(Emitter& e, ExpressionPtr exp);
  Id emitClass(Emitter& e, ClassScopePtr cNode, bool topLevel);
  Id emitTypedef(Emitter& e, TypedefStatementPtr);
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

  void emitYieldFrom(Emitter& e, ExpressionPtr exp);

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

  template<class EmitCatchBodyFun>
  void emitCatch(Emitter& e,
                 Offset start,
                 EmitCatchBodyFun emitCatchBody,
                 FaultIterInfo = FaultIterInfo { -1, KindOfIter });
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
                  folly::Optional<HeaderKind> ct = folly::none);
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

//=============================================================================
// Emitter.

#define InvariantViolation(...) do {                        \
  Logger::Warning(__VA_ARGS__);                             \
  Logger::Warning("Eval stack at the time of error: %s",    \
                  m_evalStack.pretty().c_str());            \
  assertx(false);                                            \
} while (0)

/*
 * RAII guard for function creation.
 *
 * This ensures that the eval stack's high water pointer is pointing
 * to the current function's maxStackCells (needed before we start
 * emitting bytecodes that manipulate the stack), and also that we
 * properly finish the function at the end of the scope.
 */
struct FuncFinisher {
  FuncFinisher(EmitterVisitor* ev, Emitter& e, FuncEmitter* fe,
               int32_t stackPad = 0)
    : m_ev(ev), m_e(e), m_fe(fe), m_stackPad(stackPad)
  {
    assert(!ev->m_evalStack.m_actualStackHighWaterPtr ||
           ev->m_evalStack.m_actualStackHighWaterPtr == &fe->maxStackCells);
    ev->m_evalStack.m_actualStackHighWaterPtr = &fe->maxStackCells;
  }

  ~FuncFinisher() {
    m_ev->finishFunc(m_e, m_fe, m_stackPad);
  }

  void setStackPad(int32_t stackPad) {
    m_stackPad = stackPad;
  }
private:
  EmitterVisitor* m_ev;
  Emitter&        m_e;
  FuncEmitter*    m_fe;
  int32_t         m_stackPad;
};

// RAII guard for temporarily overriding an Emitter's location
struct LocationGuard {
  LocationGuard(Emitter& e, const OptLocation& newLoc)
      : m_e(e), m_loc(e.getTempLocation()) {
    if (newLoc) m_e.setTempLocation(newLoc);
  }
  ~LocationGuard() {
    m_e.setTempLocation(m_loc);
  }

private:
  Emitter& m_e;
  OptLocation m_loc;
};

// Temporary bag of state for coordination between functions during op emission.
struct OpEmitContext {
  explicit OpEmitContext(Op op) : op{op} {}
  Op op;
  // If op is a CGetL*, did we skip over a symbolic class-ref slot when popping
  // the inputs?
  bool cgetlPopSkippedClsRef = false;
};

#define O(name, imm, pop, push, flags)                                  \
  void Emitter::name(imm) {                                             \
    auto const opcode = Op::name;                                       \
    ITRACE(2, "{}\n", #name);                                           \
    Trace::Indent indent;                                               \
    ITRACE(3, "before: {}\n", m_ev.getEvalStack().pretty());            \
    /* Process opcode's effects on the EvalStack and emit it */         \
    Offset curPos UNUSED = getUnitEmitter().bcPos();                    \
    {                                                                   \
      Trace::Indent indent2;                                            \
      getEmitterVisitor().prepareEvalStack();                           \
      OpEmitContext ctx{opcode};                                        \
      const int nIn UNUSED = COUNT_##pop;                               \
      POP_CAR_##imm;                                                    \
      POP_##pop;                                                        \
      POP_LA_##imm;                                                     \
      PUSH_##push;                                                      \
      PUSH_CAW_##imm;                                                   \
      getUnitEmitter().emitOp(Op##name);                                \
      IMPL_##imm;                                                       \
    }                                                                   \
    ITRACE(3, "after: {}\n", m_ev.getEvalStack().pretty());             \
    auto& loc = m_tempLoc ? *m_tempLoc : m_node->getRange();            \
    auto UNUSED pc = m_ue.bc() + curPos;                                \
    ITRACE(2, "lines [{},{}] chars [{},{}]\n",                          \
           loc.line0, loc.line1, loc.char0, loc.char1);                 \
    /* Update various other metadata */                                 \
    getUnitEmitter().recordSourceLocation(loc, curPos);                 \
    if (flags & TF) {                                                   \
      getEmitterVisitor().restoreJumpTargetEvalStack();                 \
      ITRACE(3, "   jmp: {}\n", m_ev.getEvalStack().pretty());          \
    }                                                                   \
    if (opcode == Op::FCall) getEmitterVisitor().recordCall();          \
    getEmitterVisitor().setPrevOpcode(opcode);                          \
  }

#define COUNT_NOV 0
#define COUNT_ONE(t) 1
#define COUNT_TWO(t1,t2) 2
#define COUNT_THREE(t1,t2,t3) 3
#define COUNT_FOUR(t1,t2,t3,t4) 4
#define COUNT_MFINAL 0
#define COUNT_F_MFINAL 0
#define COUNT_C_MFINAL 0
#define COUNT_V_MFINAL 0
#define COUNT_FMANY 0
#define COUNT_CVUMANY 0
#define COUNT_CMANY 0
#define COUNT_SMANY 0

#define ONE(t) \
  DEC_##t a1
#define TWO(t1, t2) \
  DEC_##t1 a1, DEC_##t2 a2
#define THREE(t1, t2, t3) \
  DEC_##t1 a1, DEC_##t2 a2, DEC_##t3 a3
#define FOUR(t1, t2, t3, t4) \
  DEC_##t1 a1, DEC_##t2 a2, DEC_##t3 a3, DEC_##t4 a4
#define NA
#define DEC_BLA std::vector<Label*>&
#define DEC_SLA std::vector<StrOff>&
#define DEC_ILA std::vector<IterPair>&
#define DEC_IVA uint32_t
#define DEC_LA int32_t
#define DEC_IA int32_t
#define DEC_CAR ClsRefSlotPlaceholder
#define DEC_CAW ClsRefSlotPlaceholder
#define DEC_I64A int64_t
#define DEC_DA double
#define DEC_SA const StringData*
#define DEC_RATA RepoAuthType
#define DEC_AA ArrayData*
#define DEC_BA Label&
#define DEC_OA(type) type
#define DEC_VSA std::vector<std::string>&
#define DEC_KA MemberKey
#define DEC_LAR LocalRange

#define POP_NOV
#define POP_ONE(t) \
  POP_##t(0)
#define POP_TWO(t1, t2) \
  POP_##t1(0);          \
  POP_##t2(1)
#define POP_THREE(t1, t2, t3) \
  POP_##t1(0);                \
  POP_##t2(1);                \
  POP_##t3(2)
#define POP_FOUR(t1, t2, t3, t4) \
  POP_##t1(0);                   \
  POP_##t2(1);                   \
  POP_##t3(2);                   \
  POP_##t4(3)
#define POP_MFINAL \
  getEmitterVisitor().popEvalStackMMany()
#define POP_F_MFINAL POP_MFINAL
#define POP_C_MFINAL \
  getEmitterVisitor().popEvalStack(StackSym::C); \
  getEmitterVisitor().popEvalStackMMany()
#define POP_V_MFINAL \
  getEmitterVisitor().popEvalStack(StackSym::V); \
  getEmitterVisitor().popEvalStackMMany()
#define POP_FMANY \
  getEmitterVisitor().popEvalStackMany(a1, StackSym::F)
#define POP_CVUMANY \
  getEmitterVisitor().popEvalStackCVMany(a1)
#define POP_CMANY \
  getEmitterVisitor().popEvalStackMany(a1, StackSym::C)
#define POP_SMANY \
  getEmitterVisitor().popEvalStackMany(a1.size(), StackSym::C)

#define POP_CV(i) getEmitterVisitor().popEvalStack(StackSym::C)
#define POP_VV(i) getEmitterVisitor().popEvalStack(StackSym::V)
#define POP_RV(i) getEmitterVisitor().popEvalStack(StackSym::R)
#define POP_FV(i) getEmitterVisitor().popEvalStack(StackSym::F)
#define POP_UV(i) POP_CV(i)
#define POP_CUV(i) POP_CV(i)

// Pop of virtual "locs" on the stack that turn into immediates.
#define POP_LA_ONE(t) \
  POP_LA_##t(nIn)
#define POP_LA_TWO(t1, t2) \
  POP_LA_##t1(nIn);    \
  POP_LA_##t2(nIn)
#define POP_LA_THREE(t1, t2, t3) \
  POP_LA_##t1(nIn);          \
  POP_LA_##t2(nIn);          \
  POP_LA_##t3(nIn)
#define POP_LA_FOUR(t1, t2, t3, t4) \
  POP_LA_##t1(nIn);             \
  POP_LA_##t2(nIn);             \
  POP_LA_##t3(nIn);             \
  POP_LA_##t4(nIn)

#define POP_LA_NA
#define POP_LA_BLA(i)
#define POP_LA_SLA(i)
#define POP_LA_ILA(i)
#define POP_LA_IVA(i)
#define POP_LA_IA(i)
#define POP_LA_CAR(i)
#define POP_LA_CAW(i)
#define POP_LA_I64A(i)
#define POP_LA_DA(i)
#define POP_LA_SA(i)
#define POP_LA_RATA(i)
#define POP_LA_AA(i)
#define POP_LA_BA(i)
#define POP_LA_IMPL(x)
#define POP_LA_OA(i) POP_LA_IMPL
#define POP_LA_VSA(i)
#define POP_LA_KA(i)
#define POP_LA_LAR(i)

#define POP_LA_LA(i) \
  getEmitterVisitor().popSymbolicLocal(ctx)

// Pop of virtual class-refs on the stack
#define POP_CAR_ONE(t) \
  POP_CAR_##t(nIn)
#define POP_CAR_TWO(t1, t2) \
  POP_CAR_##t1(nIn);    \
  POP_CAR_##t2(nIn)
#define POP_CAR_THREE(t1, t2, t3) \
  POP_CAR_##t1(nIn);          \
  POP_CAR_##t2(nIn);          \
  POP_CAR_##t3(nIn)
#define POP_CAR_FOUR(t1, t2, t3, t4) \
  POP_CAR_##t1(nIn);             \
  POP_CAR_##t2(nIn);             \
  POP_CAR_##t3(nIn);             \
  POP_CAR_##t4(nIn)

#define POP_CAR_NA
#define POP_CAR_BLA(i)
#define POP_CAR_SLA(i)
#define POP_CAR_ILA(i)
#define POP_CAR_IVA(i)
#define POP_CAR_LA(i)
#define POP_CAR_IA(i)
#define POP_CAR_CAR(i) getEmitterVisitor().popSymbolicClassRef(ctx)
#define POP_CAR_CAW(i)
#define POP_CAR_I64A(i)
#define POP_CAR_DA(i)
#define POP_CAR_SA(i)
#define POP_CAR_RATA(i)
#define POP_CAR_AA(i)
#define POP_CAR_BA(i)
#define POP_CAR_IMPL(x)
#define POP_CAR_OA(i) POP_CAR_IMPL
#define POP_CAR_VSA(i)
#define POP_CAR_KA(i)
#define POP_CAR_LAR(i)

// Push of virtual class-refs on the stack
#define PUSH_CAW_ONE(t) \
  PUSH_CAW_##t(nIn)
#define PUSH_CAW_TWO(t1, t2) \
  PUSH_CAW_##t1(nIn);    \
  PUSH_CAW_##t2(nIn)
#define PUSH_CAW_THREE(t1, t2, t3) \
  PUSH_CAW_##t1(nIn);          \
  PUSH_CAW_##t2(nIn);          \
  PUSH_CAW_##t3(nIn)
#define PUSH_CAW_FOUR(t1, t2, t3, t4) \
  PUSH_CAW_##t1(nIn);             \
  PUSH_CAW_##t2(nIn);             \
  PUSH_CAW_##t3(nIn);             \
  PUSH_CAW_##t4(nIn)

#define PUSH_CAW_NA
#define PUSH_CAW_BLA(i)
#define PUSH_CAW_SLA(i)
#define PUSH_CAW_ILA(i)
#define PUSH_CAW_IVA(i)
#define PUSH_CAW_LA(i)
#define PUSH_CAW_IA(i)
#define PUSH_CAW_CAR(i)
#define PUSH_CAW_CAW(i) getEmitterVisitor().pushSymbolicClassRef()
#define PUSH_CAW_I64A(i)
#define PUSH_CAW_DA(i)
#define PUSH_CAW_SA(i)
#define PUSH_CAW_RATA(i)
#define PUSH_CAW_AA(i)
#define PUSH_CAW_BA(i)
#define PUSH_CAW_IMPL(x)
#define PUSH_CAW_OA(i) PUSH_CAW_IMPL
#define PUSH_CAW_VSA(i)
#define PUSH_CAW_KA(i)
#define PUSH_CAW_LAR(i)

#define PUSH_NOV
#define PUSH_ONE(t) \
  PUSH_##t
#define PUSH_TWO(t1, t2) \
  PUSH_##t2; \
  PUSH_##t1
#define PUSH_THREE(t1, t2, t3) \
  PUSH_##t3; \
  PUSH_##t2; \
  PUSH_##t1
#define PUSH_FOUR(t1, t2, t3, t4) \
  PUSH_##t4; \
  PUSH_##t3; \
  PUSH_##t2; \
  PUSH_##t1
#define PUSH_INS_1(t) PUSH_INS_1_##t

#define PUSH_CV getEmitterVisitor().pushEvalStackFromOp(StackSym::C, ctx)
#define PUSH_UV PUSH_CV
#define PUSH_CUV PUSH_CV
#define PUSH_VV getEmitterVisitor().pushEvalStackFromOp(StackSym::V, ctx)
#define PUSH_RV getEmitterVisitor().pushEvalStackFromOp(StackSym::R, ctx)
#define PUSH_FV getEmitterVisitor().pushEvalStackFromOp(StackSym::F, ctx)

#define PUSH_INS_1_CV \
  getEmitterVisitor().insertEvalStackFromOp(StackSym::C, 1, ctx);

#define IMPL_NA
#define IMPL_ONE(t) \
  IMPL1_##t
#define IMPL_TWO(t1, t2) \
  IMPL1_##t1; \
  IMPL2_##t2
#define IMPL_THREE(t1, t2, t3) \
  IMPL1_##t1; \
  IMPL2_##t2; \
  IMPL3_##t3
#define IMPL_FOUR(t1, t2, t3, t4) \
  IMPL1_##t1; \
  IMPL2_##t2; \
  IMPL3_##t3; \
  IMPL4_##t4

#define IMPL_BLA(var) do {                            \
  getUnitEmitter().emitInt32(var.size());             \
  for (unsigned int i = 0; i < var.size(); ++i) {     \
    IMPL_BA(*var[i]);                                 \
  }                                                   \
} while (0)
#define IMPL1_BLA IMPL_BLA(a1)
#define IMPL2_BLA IMPL_BLA(a2)
#define IMPL3_BLA IMPL_BLA(a3)
#define IMPL4_BLA IMPL_BLA(a4)

#define IMPL_ILA(var) do {     \
  auto& ue = getUnitEmitter(); \
  ue.emitInt32(var.size());    \
  for (auto& i : var) {        \
    ue.emitInt32(i.kind);      \
    ue.emitInt32(i.id);        \
  }                            \
} while(0)
#define IMPL1_ILA IMPL_ILA(a1)
#define IMPL2_ILA IMPL_ILA(a2)
#define IMPL3_ILA IMPL_ILA(a3)
#define IMPL4_ILA IMPL_ILA(a4)

#define IMPL_SLA(var) do {                      \
  auto& ue = getUnitEmitter();                  \
  ue.emitInt32(var.size());                     \
  for (auto& i : var) {                         \
    ue.emitInt32(i.str);                        \
    IMPL_BA(*i.dest);                           \
  }                                             \
} while (0)
#define IMPL1_SLA IMPL_SLA(a1)
#define IMPL2_SLA IMPL_SLA(a2)
#define IMPL3_SLA IMPL_SLA(a3)

#define IMPL_VSA(var) do {                          \
  auto n = var.size();                              \
  getUnitEmitter().emitInt32(n);                    \
  for (size_t i = 0; i < n; ++i) {                  \
    IMPL_SA((HPHP::String(var[i])).get());          \
  }                                                 \
} while (0)
#define IMPL1_VSA IMPL_VSA(a1)
#define IMPL2_VSA IMPL_VSA(a2)
#define IMPL3_VSA IMPL_VSA(a3)
#define IMPL4_VSA IMPL_VSA(a4)

#define IMPL_IVA(var) do { \
  getUnitEmitter().emitIVA(var); \
} while (0)
#define IMPL1_IVA IMPL_IVA(a1)
#define IMPL2_IVA IMPL_IVA(a2)
#define IMPL3_IVA IMPL_IVA(a3)
#define IMPL4_IVA IMPL_IVA(a4)

#define IMPL1_LA IMPL_IVA(a1)
#define IMPL2_LA IMPL_IVA(a2)
#define IMPL3_LA IMPL_IVA(a3)
#define IMPL4_LA IMPL_IVA(a4)

#define IMPL1_IA IMPL_IVA(a1)
#define IMPL2_IA IMPL_IVA(a2)
#define IMPL3_IA IMPL_IVA(a3)
#define IMPL4_IA IMPL_IVA(a4)

#define IMPL_CAR do {                         \
  getUnitEmitter().emitIVA(                   \
    getEmitterVisitor().popClsRefSlot()       \
  );                                          \
} while (0)
#define IMPL1_CAR IMPL_CAR
#define IMPL2_CAR IMPL_CAR
#define IMPL3_CAR IMPL_CAR
#define IMPL4_CAR IMPL_CAR

#define IMPL_CAW do {                         \
  getUnitEmitter().emitIVA(                   \
    getEmitterVisitor().pushClsRefSlot()      \
  );                                          \
} while (0)
#define IMPL1_CAW IMPL_CAW
#define IMPL2_CAW IMPL_CAW
#define IMPL3_CAW IMPL_CAW
#define IMPL4_CAW IMPL_CAW

#define IMPL_I64A(var) getUnitEmitter().emitInt64(var)
#define IMPL1_I64A IMPL_I64A(a1)
#define IMPL2_I64A IMPL_I64A(a2)
#define IMPL3_I64A IMPL_I64A(a3)
#define IMPL4_I64A IMPL_I64A(a4)

#define IMPL_SA(var) \
  getUnitEmitter().emitInt32(getUnitEmitter().mergeLitstr(var))
#define IMPL1_SA IMPL_SA(a1)
#define IMPL2_SA IMPL_SA(a2)
#define IMPL3_SA IMPL_SA(a3)
#define IMPL4_SA IMPL_SA(a4)

// Emitting RATAs isn't supported here right now.  (They're only
// created in hhbbc.)
#define IMPL_RATA(var) not_reached()
#define IMPL1_RATA IMPL_RATA(a1)
#define IMPL2_RATA IMPL_RATA(a2)
#define IMPL3_RATA IMPL_RATA(a3)
#define IMPL4_RATA IMPL_RATA(a4)

#define IMPL_AA(var) \
  getUnitEmitter().emitInt32(getUnitEmitter().mergeArray(var))
#define IMPL1_AA IMPL_AA(a1)
#define IMPL2_AA IMPL_AA(a2)
#define IMPL3_AA IMPL_AA(a3)
#define IMPL4_AA IMPL_AA(a4)

#define IMPL_DA(var) getUnitEmitter().emitDouble(var)
#define IMPL1_DA IMPL_DA(a1)
#define IMPL2_DA IMPL_DA(a2)
#define IMPL3_DA IMPL_DA(a3)
#define IMPL4_DA IMPL_DA(a4)

#define IMPL_BA(var) \
  if ((var).getAbsoluteOffset() == InvalidAbsoluteOffset) { \
    /* For forward jumps, we store information about the */ \
    /* current instruction in the Label. When the Label is */ \
    /* set, it will fix up any instructions that reference */ \
    /* it, and then it will call recordJumpTarget */ \
    (var).bind(getEmitterVisitor(), curPos, getUnitEmitter().bcPos()); \
  } else { \
    /* For backward jumps, we simply call recordJumpTarget */ \
    getEmitterVisitor().recordJumpTarget((var).getAbsoluteOffset()); \
  } \
  getUnitEmitter().emitInt32((var).getAbsoluteOffset() - curPos);
#define IMPL1_BA IMPL_BA(a1)
#define IMPL2_BA IMPL_BA(a2)
#define IMPL3_BA IMPL_BA(a3)
#define IMPL4_BA IMPL_BA(a4)

#define IMPL_OA(var) getUnitEmitter().emitByte(static_cast<uint8_t>(var))
#define IMPL1_OA(type) IMPL_OA(a1)
#define IMPL2_OA(type) IMPL_OA(a2)
#define IMPL3_OA(type) IMPL_OA(a3)
#define IMPL4_OA(type) IMPL_OA(a4)

#define IMPL_KA(var) encode_member_key(var, getUnitEmitter())
#define IMPL1_KA IMPL_KA(a1)
#define IMPL2_KA IMPL_KA(a2)
#define IMPL3_KA IMPL_KA(a3)
#define IMPL4_KA IMPL_KA(a4)

#define IMPL_LAR(var) encodeLocalRange(getUnitEmitter(), var)
#define IMPL1_LAR IMPL_LAR(a1)
#define IMPL2_LAR IMPL_LAR(a2)
#define IMPL3_LAR IMPL_LAR(a3)
#define IMPL4_LAR IMPL_LAR(a4)

 OPCODES

#undef O
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef NA
#undef DEC_IVA
#undef DEC_LA
#undef DEC_IA
#undef DEC_CAR
#undef DEC_CAW
#undef DEC_I64A
#undef DEC_DA
#undef DEC_SA
#undef DEC_RATA
#undef DEC_AA
#undef DEC_BA
#undef DEC_OA
#undef DEC_KA
#undef DEC_LAR
#undef POP_NOV
#undef POP_ONE
#undef POP_TWO
#undef POP_THREE
#undef POP_FOUR
#undef POP_MFINAL
#undef POP_F_MFINAL
#undef POP_C_MFINAL
#undef POP_V_MFINAL
#undef POP_CV
#undef POP_VV
#undef POP_HV
#undef POP_RV
#undef POP_FV
#undef POP_LREST
#undef POP_FMANY
#undef POP_CVUMANY
#undef POP_CMANY
#undef POP_SMANY
#undef POP_LA_ONE
#undef POP_LA_TWO
#undef POP_LA_THREE
#undef POP_LA_FOUR
#undef POP_LA_NA
#undef POP_LA_IVA
#undef POP_LA_IA
#undef POP_LA_CAR
#undef POP_LA_CAW
#undef POP_LA_I64A
#undef POP_LA_DA
#undef POP_LA_SA
#undef POP_LA_RATA
#undef POP_LA_AA
#undef POP_LA_BA
#undef POP_LA_IMPL
#undef POP_LA_OA
#undef POP_LA_LA
#undef POP_LA_KA
#undef POP_LA_LAR
#undef POP_CAR_ONE
#undef POP_CAR_TWO
#undef POP_CAR_THREE
#undef POP_CAR_FOUR
#undef POP_CAR_NA
#undef POP_CAR_IVA
#undef POP_CAR_IA
#undef POP_CAR_CAR
#undef POP_CAR_CAW
#undef POP_CAR_I64A
#undef POP_CAR_DA
#undef POP_CAR_SA
#undef POP_CAR_RATA
#undef POP_CAR_AA
#undef POP_CAR_BA
#undef POP_CAR_IMPL
#undef POP_CAR_OA
#undef POP_CAR_LA
#undef POP_CAR_KA
#undef POP_CAR_LAR
#undef PUSH_CAW_ONE
#undef PUSH_CAW_TWO
#undef PUSH_CAW_THREE
#undef PUSH_CAW_FOUR
#undef PUSH_CAW_NA
#undef PUSH_CAW_IVA
#undef PUSH_CAW_IA
#undef PUSH_CAW_CAR
#undef PUSH_CAW_CAW
#undef PUSH_CAW_I64A
#undef PUSH_CAW_DA
#undef PUSH_CAW_SA
#undef PUSH_CAW_RATA
#undef PUSH_CAW_AA
#undef PUSH_CAW_BA
#undef PUSH_CAW_IMPL
#undef PUSH_CAW_OA
#undef PUSH_CAW_LA
#undef PUSH_CAW_KA
#undef PUSH_CAW_LAR
#undef PUSH_NOV
#undef PUSH_ONE
#undef PUSH_TWO
#undef PUSH_THREE
#undef PUSH_FOUR
#undef PUSH_CV
#undef PUSH_UV
#undef PUSH_CUV
#undef PUSH_VV
#undef PUSH_HV
#undef PUSH_RV
#undef PUSH_FV
#undef IMPL_ONE
#undef IMPL_TWO
#undef IMPL_THREE
#undef IMPL_FOUR
#undef IMPL_NA
#undef IMPL_BLA
#undef IMPL1_BLA
#undef IMPL2_BLA
#undef IMPL3_BLA
#undef IMPL4_BLA
#undef IMPL_SLA
#undef IMPL1_SLA
#undef IMPL2_SLA
#undef IMPL3_SLA
#undef IMPL4_SLA
#undef IMPL_ILA
#undef IMPL1_ILA
#undef IMPL2_ILA
#undef IMPL3_ILA
#undef IMPL4_ILA
#undef IMPL_IVA
#undef IMPL1_IVA
#undef IMPL2_IVA
#undef IMPL3_IVA
#undef IMPL4_IVA
#undef IMPL1_LA
#undef IMPL2_LA
#undef IMPL3_LA
#undef IMPL4_LA
#undef IMPL1_IA
#undef IMPL2_IA
#undef IMPL3_IA
#undef IMPL4_IA
#undef IMPL1_CAR
#undef IMPL2_CAR
#undef IMPL3_CAR
#undef IMPL4_CAR
#undef IMPL1_CAW
#undef IMPL2_CAW
#undef IMPL3_CAW
#undef IMPL4_CAW
#undef IMPL_I64A
#undef IMPL1_I64A
#undef IMPL2_I64A
#undef IMPL3_I64A
#undef IMPL4_I64A
#undef IMPL_DA
#undef IMPL1_DA
#undef IMPL2_DA
#undef IMPL3_DA
#undef IMPL4_DA
#undef IMPL_SA
#undef IMPL1_SA
#undef IMPL2_SA
#undef IMPL3_SA
#undef IMPL4_SA
#undef IMPL_RATA
#undef IMPL1_RATA
#undef IMPL2_RATA
#undef IMPL3_RATA
#undef IMPL4_RATA
#undef IMPL_AA
#undef IMPL1_AA
#undef IMPL2_AA
#undef IMPL3_AA
#undef IMPL4_AA
#undef IMPL_BA
#undef IMPL1_BA
#undef IMPL2_BA
#undef IMPL3_BA
#undef IMPL4_BA
#undef IMPL_OA
#undef IMPL1_OA
#undef IMPL2_OA
#undef IMPL3_OA
#undef IMPL4_OA
#undef IMPL_KA
#undef IMPL1_KA
#undef IMPL2_KA
#undef IMPL3_KA
#undef IMPL4_KA
#undef IMPL_LAR
#undef IMPL1_LAR
#undef IMPL2_LAR
#undef IMPL3_LAR
#undef IMPL4_LAR

static void checkJmpTargetEvalStack(const SymbolicStack& source,
                                    const SymbolicStack& dest) {
  if (source.size() != dest.size()) {
    Logger::FWarning("Emitter detected a point in the bytecode where the "
                     "depth of the stack is not the same for all possible "
                     "control flow paths. source size: {}. dest size: {}",
                     source.size(),
                     dest.size());
    Logger::Warning("src stack : %s", source.pretty().c_str());
    Logger::Warning("dest stack: %s", dest.pretty().c_str());
    assertx(false);
    return;
  }

  for (unsigned int i = 0; i < source.size(); ++i) {
    char flavor = StackSym::GetSymFlavor(source.get(i));
    bool matches = source.get(i) == dest.get(i) &&
      (flavor != StackSym::L || source.getLoc(i) == dest.getLoc(i)) &&
      (flavor != StackSym::T || source.getName(i) == dest.getName(i)) &&
      (flavor != StackSym::I || source.getInt(i) == dest.getInt(i));
    if (!matches) {
      Logger::Warning("Emitter detected a point in the bytecode where the "
                      "symbolic flavor of a slot on the stack is not the same "
                      "for all possible control flow paths");
      Logger::Warning("src stack : %s", source.pretty().c_str());
      Logger::Warning("dest stack: %s", dest.pretty().c_str());
      assert(false);
      return;
    }
  }

  if (source.peekClsRefSlot() != dest.peekClsRefSlot()) {
    Logger::Warning("Emitter detected a point in the bytecode where the "
                    "class-ref slot assignments are not the same "
                    "for all possible control flow paths");
    Logger::Warning("src stack : %s", source.pretty().c_str());
    Logger::Warning("dest stack: %s", dest.pretty().c_str());
    assert(false);
    return;
  }
}

std::string SymbolicStack::SymEntry::pretty() const {
  std::string ret;
  ret += StackSym::ToString(sym);
  char flavor = StackSym::GetSymFlavor(sym);
  if (flavor == StackSym::L || flavor == StackSym::I) {
    folly::toAppend(':', intval, &ret);
  } else if (flavor == StackSym::T && name) {
    folly::toAppend(':', name->data(), &ret);
  }
  return ret;
}

std::string SymbolicStack::pretty() const {
  std::ostringstream out;
  out << "[" << std::hex;
  size_t j = 0;
  auto sep = "";
  for (size_t i = 0; i < m_symStack.size(); ++i) {
    out << sep;
    sep = " ";
    while (j < m_actualStack.size() && m_actualStack[j] < int(i)) {
      ++j;
    }
    if (j < m_actualStack.size() && m_actualStack[j] == int(i)) {
      out << "*";
    }
    out << m_symStack[i].pretty();
  }
  out << "] (Class-ref top: " << std::dec << m_clsRefTop << ')';
  return out.str();
}

void SymbolicStack::updateHighWater() {
  *m_actualStackHighWaterPtr =
    std::max(*m_actualStackHighWaterPtr,
             static_cast<int>(m_actualStack.size() + m_fdescCount));
}

void SymbolicStack::push(char sym) {
  if (!StackSym::IsSymbolic(sym)) {
    m_actualStack.push_back(m_symStack.size());
    updateHighWater();
  }
  m_symStack.push_back(SymEntry(sym));
  ITRACE(4, "push: {}\n", m_symStack.back().pretty());
}

void SymbolicStack::pop() {
  // TODO(drew): assert eval stack unknown is false?
  assert(!m_symStack.empty());
  char sym = m_symStack.back().sym;
  char flavor = StackSym::GetSymFlavor(sym);
  if (StackSym::GetMarker(sym) != StackSym::W &&
      flavor != StackSym::L && flavor != StackSym::T && flavor != StackSym::I &&
      flavor != StackSym::H && flavor != StackSym::A) {
    assert(!m_actualStack.empty());
    m_actualStack.pop_back();
  }

  ITRACE(4, "pop: {}\n", m_symStack.back().pretty());
  m_symStack.pop_back();
}

char SymbolicStack::top() const {
  assert(!m_symStack.empty());
  return m_symStack.back().sym;
}

char SymbolicStack::get(int index) const {
  assert(index >= 0 && index < (int)m_symStack.size());
  return m_symStack[index].sym;
}

const StringData* SymbolicStack::getName(int index) const {
  assert(index >= 0 && index < (int)m_symStack.size());
  return m_symStack[index].name;
}

const StringData* SymbolicStack::getClsName(int index) const {
  assert(index >= 0 && index < (int)m_symStack.size());
  return m_symStack[index].className;
}

bool SymbolicStack::isCls(int index) const {
  assert(index >= 0 && index < (int)m_symStack.size());
  return m_symStack[index].className != nullptr;
}

void SymbolicStack::setString(const StringData* s) {
  assert(m_symStack.size());
  SymEntry& se = m_symStack.back();
  assert(!se.name || se.name == s);
  se.name = s;
}

void SymbolicStack::setKnownCls(const StringData* s, bool /*nonNull*/) {
  assert(m_symStack.size());
  SymEntry& se = m_symStack.back();
  assert(!se.className || se.className == s);
  se.className = s;
}

void SymbolicStack::setInt(int64_t v) {
  assert(m_symStack.size());
  m_symStack.back().intval = v;
}

void SymbolicStack::cleanTopMeta() {
  SymEntry& se = m_symStack.back();
  se.clsBaseType = CLS_INVALID;
  se.name = nullptr;
}

void SymbolicStack::setClsBaseType(ClassBaseType type) {
  assert(!m_symStack.empty());
  m_symStack.back().clsBaseType = type;
}

void SymbolicStack::setUnnamedLocal(int index,
                                    int localId,
                                    Offset startOff) {
  assert(size_t(index) < m_symStack.size());
  assert(m_symStack[index].sym == StackSym::K);
  assert(m_symStack[index].clsBaseType == CLS_UNNAMED_LOCAL);
  m_symStack[index].intval = localId;
  m_symStack[index].unnamedLocalStart = startOff;
}

void SymbolicStack::set(int index, char sym) {
  assert(index >= 0 && index < (int)m_symStack.size());
  // XXX Add assert in debug build to make sure W is not getting
  // written or overwritten by something else
  m_symStack[index].sym = sym;
  ITRACE(4, "   set: {} -> {}\n", index, m_symStack[index].pretty());
}

size_t SymbolicStack::size() const {
  return m_symStack.size();
}

size_t SymbolicStack::actualSize() const {
  return m_actualStack.size();
}

bool SymbolicStack::empty() const {
  return m_symStack.empty();
}

void SymbolicStack::clear() {
  m_symStack.clear();
  m_actualStack.clear();
  m_fdescCount = 0;
  m_clsRefTop = 0;
}

void SymbolicStack::consumeBelowTop(int depth) {
  if (int(m_symStack.size()) < depth + 1) {
    Logger::Warning(
      "Emitter tried to consumeBelowTop() when the symbolic "
      "stack did not have enough elements in it.");
    assert(false);
    return;
  }
  assert(int(m_symStack.size()) >= depth + 1);
  int index = m_symStack.size() - depth - 1;
  m_symStack.erase(m_symStack.begin() + index);

  /*
   * Update any indexes into the actual stack that pointed to or past
   * this element.
   *
   * (In practice they should all be past---we don't currently ever
   * remove below the top for actual stack elements.)
   */
  for (size_t i = 0; i < m_actualStack.size(); ++i) {
    if (m_actualStack[i] >= index) {
      --m_actualStack[i];
    }
  }
}

int SymbolicStack::getActualPos(int vpos) const {
  assert(vpos >= 0 && vpos < int(m_symStack.size()));
  assert(!m_actualStack.empty());
  for (int j = int(m_actualStack.size()) - 1; j >= 0; --j) {
    if (m_actualStack[j] == vpos) {
      return j;
    }
  }
  not_reached();
}

char SymbolicStack::getActual(int index) const {
  assert(index >= 0 && index < (int)m_actualStack.size());
  return get(m_actualStack[index]);
}

void SymbolicStack::setActual(int index, char sym) {
  assert(index >= 0 && index < (int)m_actualStack.size());
  set(m_actualStack[index], sym);
}

SymbolicStack::ClassBaseType
SymbolicStack::getClsBaseType(int index) const {
  assert(m_symStack.size() > size_t(index));
  assert(m_symStack[index].sym == StackSym::K);
  assert(m_symStack[index].clsBaseType != CLS_INVALID);
  return m_symStack[index].clsBaseType;
}

int SymbolicStack::getLoc(int index) const {
  assert(m_symStack.size() > size_t(index));
  assert(StackSym::GetSymFlavor(m_symStack[index].sym) == StackSym::L ||
         m_symStack[index].clsBaseType == CLS_NAMED_LOCAL ||
         m_symStack[index].clsBaseType == CLS_UNNAMED_LOCAL);
  assert(m_symStack[index].intval != -1);
  return m_symStack[index].intval;
}

int64_t SymbolicStack::getInt(int index) const {
  assert(m_symStack.size() > size_t(index));
  assert(StackSym::GetSymFlavor(m_symStack[index].sym) == StackSym::I);
  return m_symStack[index].intval;
}

Offset SymbolicStack::getUnnamedLocStart(int index) const {
  assert(m_symStack.size() > size_t(index));
  assert(m_symStack[index].sym == StackSym::K);
  assert(m_symStack[index].clsBaseType == CLS_UNNAMED_LOCAL);
  return m_symStack[index].unnamedLocalStart;
}

// Insert an element in the actual stack at the specified depth of the
// actual stack.
void SymbolicStack::insertAt(int depth, char sym) {
  assert(!StackSym::IsSymbolic(sym));
  assert(depth <= sizeActual() && depth > 0);
  int virtIdx = m_actualStack[sizeActual() - depth];

  m_symStack.insert(m_symStack.begin() + virtIdx, SymEntry(sym));
  m_actualStack.insert(m_actualStack.end() - depth, virtIdx);

  for (size_t i = sizeActual() - depth + 1; i < m_actualStack.size(); ++i) {
    ++m_actualStack[i];
  }
}

// Push a non-symbolic element onto the top of the stack. If there's a symbolic
// element at the top, push it behind it.
void SymbolicStack::pushSkipSymbolicTop(char sym) {
  assert(!StackSym::IsSymbolic(sym));
  if (m_symStack.empty() ||
      !StackSym::IsSymbolic(m_symStack[m_symStack.size() - 1].sym)) {
    return push(sym);
  }
  auto const virtIdx = m_symStack.size() - 1;
  m_symStack.insert(m_symStack.begin() + virtIdx, SymEntry(sym));
  m_actualStack.push_back(virtIdx);
}

// Insert a non-symbolic element at the specified depth, including symbolic
// elements.
void SymbolicStack::symbolicInsertAt(int depth, char sym) {
  assert(!StackSym::IsSymbolic(sym));
  assert(depth > 0);
  assert(m_symStack.size() >= depth);

  auto const virtIdx = m_symStack.size() - depth;
  size_t actualIdx = 0;
  while (actualIdx < m_actualStack.size()) {
    auto const idx = m_actualStack[actualIdx];
    if (idx >= virtIdx) break;
    ++actualIdx;
  }

  m_actualStack.insert(m_actualStack.begin() + actualIdx, virtIdx);
  m_symStack.insert(m_symStack.begin() + virtIdx, SymEntry(sym));

  for (size_t i = actualIdx + 1; i < m_actualStack.size(); ++i) {
    ++m_actualStack[i];
  }
}

int SymbolicStack::sizeActual() const {
  return m_actualStack.size();
}

void SymbolicStack::pushFDesc() {
  m_fdescCount += kNumActRecCells;
  updateHighWater();
}

void SymbolicStack::popFDesc() {
  m_fdescCount -= kNumActRecCells;
}

void Label::set(Emitter& e, bool emitNopAtEntry /* = true */) {
  auto const off = e.getUnitEmitter().bcPos();
  if (isSet()) {
    InvariantViolation(
      "Label::set was called more than once on the same "
      "Label; originally set to %d; now %d",
      m_off,
      off);
    return;
  }
  EmitterVisitor& ev = e.getEmitterVisitor();
  if (emitNopAtEntry && ev.getFuncEmitter()->base == off) {
    // Make sure there are no jumps to the base of functions.
    // The JIT relies on this in some analysis.
    e.getUnitEmitter().emitOp(OpEntryNop);
  }
  m_off = e.getUnitEmitter().bcPos();
  // Fix up any forward jumps that reference to this Label
  for (std::vector<std::pair<Offset, Offset> >::const_iterator it =
      m_emittedOffs.begin(); it != m_emittedOffs.end(); ++it) {
    e.getUnitEmitter().emitInt32(m_off - it->first, it->second);
  }
  if (!m_emittedOffs.empty()) {
    // If there were forward jumps that referenced this Label,
    // compare the the eval stack from the first foward jump we
    // saw with the current eval stack
    if (!ev.evalStackIsUnknown()) {
      checkJmpTargetEvalStack(m_evalStack, ev.getEvalStack());
    } else {
      // Assume the current eval stack matches that of the forward branch
      ITRACE(3, "bind: {}\n", m_evalStack.pretty());
      ev.setEvalStack(m_evalStack);
    }
    // Fix up the EmitterVisitor's table of jump targets
    ev.recordJumpTarget(m_off, ev.getEvalStack());
  } else {
    // There were no forward jumps that referenced this label
    ev.prepareEvalStack();
    // Fix up the EmitterVisitor's table of jump targets
    ev.recordJumpTarget(m_off, ev.getEvalStack());
  }
}

bool Label::isUsed() {
  return (m_off != InvalidAbsoluteOffset || !m_emittedOffs.empty());
}

void Label::bind(EmitterVisitor& ev, Offset instrAddr, Offset offAddr) {
  if (m_off != InvalidAbsoluteOffset) {
    InvariantViolation("Label::bind was called on a Label that has already "
                       "been set to %d",
                       m_off);
    return;
  }
  bool labelHasEvalStack = !m_emittedOffs.empty();
  m_emittedOffs.push_back(std::pair<Offset, Offset>(instrAddr, offAddr));
  if (labelHasEvalStack) {
    checkJmpTargetEvalStack(m_evalStack, ev.getEvalStack());
  } else {
    m_evalStack = ev.getEvalStack();
  }
}

struct FPIRegionRecorder {
  FPIRegionRecorder(EmitterVisitor* ev, UnitEmitter& ue, SymbolicStack& stack,
                    Offset start)
      : m_ev(ev), m_ue(ue), m_stack(stack), m_fpOff(m_stack.sizeActual()),
        m_start(start) {
    m_stack.pushFDesc();
  }
  ~FPIRegionRecorder() {
    m_stack.popFDesc();
    m_ev->newFPIRegion(m_start, m_ue.bcPos(), m_fpOff);
  }
private:
  EmitterVisitor* m_ev;
  UnitEmitter& m_ue;
  SymbolicStack& m_stack;
  int m_fpOff;
  Offset m_start;
};

//=============================================================================
// ControlTarget.

const int ControlTarget::k_unsetState = -1;

ControlTarget::ControlTarget(EmitterVisitor* router)
  : m_visitor(router), m_label(), m_state(k_unsetState) {
  assert(m_visitor != nullptr);
}

ControlTarget::~ControlTarget() {
  // The scope of states used in finally router is controled
  // using shared pointer refcounting. State numbers can be reused once
  // all the references are released.
  if (isRegistered()) {
    m_visitor->unregisterControlTarget(this);
    m_state = k_unsetState;
  }
}

bool ControlTarget::isRegistered() {
  return m_state != k_unsetState;
}

//=============================================================================
// Region.

Region::Region(Region::Kind kind, RegionPtr parent)
  : m_kind(kind),
    m_iterId(-1),
    m_iterKind(KindOfIter),
    m_parent(parent) {
}

void
EmitterVisitor::registerReturn(StatementPtr s, Region* region, char sym) {
  ControlTargetPtr t;
  Region* r;
  for (r = region; true; r = r->m_parent.get()) {
    assert(r);
    if (r->isFinally()) {
      throw EmitterVisitor::IncludeTimeFatalException(s,
              "Return inside a finally block is not supported");
    }
    if (r->m_returnTargets.count(sym)) {
      // We registered the control target before. Just return the existing one.
      t = r->m_returnTargets[sym].target;
      break;
    }
    // Haven't registered the control target with region r yet.
    if (r->m_parent.get() == nullptr) {
      // Top of the region hierarchy - allocate a fresh control target.
      t = std::make_shared<ControlTarget>(this);
      r = r->m_parent.get();
      break;
    }
  }
  assert(t != nullptr);
  if (!t->isRegistered()) {
    registerControlTarget(t.get());
  }
  // For all entries we visited that did not have this control target in
  // m_returnTargets, add this control target to these entries' m_returnTargets
  // fields as appropriate.
  Region* end = r;
  for (r = region; r != end; r = r->m_parent.get()) {
    r->m_returnTargets[sym] = ControlTargetInfo(t, r->isTryFinally());
  }
}

ControlTargetPtr
EmitterVisitor::registerGoto(StatementPtr /*s*/, Region* region,
                             StringData* name, bool alloc) {
  ControlTargetPtr t;
  Region* r;
  for (r = region; true; r = r->m_parent.get()) {
    assert(r);
    if (r->m_gotoTargets.count(name)) {
      // We registered the control target before. Just return the existing one.
      t = r->m_gotoTargets[name].target;
      if (alloc && r->isTryFinally()) {
        r->m_gotoTargets[name].used = true;
      }
      break;
    }
    // Haven't registered the control target in this region yet.
    if (r->m_parent.get() == nullptr) {
      // Top of the region hierarchy - allocate a fresh control target.
      t = std::make_shared<ControlTarget>(this);
      r = r->m_parent.get();
      break;
    }
  }
  assert(t != nullptr);
  if (alloc && !t->isRegistered()) {
    registerControlTarget(t.get());
  }
  // For all entries we visited that did not have this control target in
  // m_gotoTargets, add this control target to these entries' m_gotoTargets
  // fields as appropriate.
  Region* end = r;
  for (r = region; r != end; r = r->m_parent.get()) {
    r->m_gotoTargets[name] = ControlTargetInfo(t, alloc && r->isTryFinally());
  }
  return t;
}

void EmitterVisitor::registerYieldAwait(ExpressionPtr e) {
  Region* region = m_regions.back().get();
  for (; region; region = region->m_parent.get()) {
    if (region->isFinally()) {
      throw EmitterVisitor::IncludeTimeFatalException(e,
              "Yield expression inside a finally block is not supported");
    }
  }
}

ControlTargetPtr
EmitterVisitor::registerBreak(StatementPtr s, Region* region, int depth,
                              bool alloc) {
  ControlTargetPtr t;
  assert(depth >= 1);
  int d = depth;
  Region* r;
  for (r = region; true; r = r->m_parent.get()) {
    assert(r);
    if (r->isFinally()) {
      throw EmitterVisitor::IncludeTimeFatalException(s,
              "Break jump is not allowed to leave a finally block");
    }
    if (r->m_breakTargets.count(d)) {
      // We registered the control target before. Just return the existing one.
      t = r->m_breakTargets[d].target;
      if (alloc && r->isTryFinally()) {
        r->m_breakTargets[d].used = true;
      }
      break;
    }
    if (r->m_kind != Region::Kind::LoopOrSwitch) {
      continue;
    }
    if (d == 1) {
      // We should never reach this case if alloc == true, since the loop or
      // switch should have registered its break target in advance
      assert(!alloc);
      // If this is a loop, and depth is one, just allocate a fresh
      // control target, since there are no more entries to delegate to.
      t = std::make_shared<ControlTarget>(this);
      r = r->m_parent.get();
      break;
    }
    // Otherwise, delegate to the parent. One break level has been
    // taken care of by this region.
    --d;
  }
  assert(t != nullptr);
  if (alloc) {
    if (!t->isRegistered()) {
      registerControlTarget(t.get());
    }
  }
  // For all of the entries that did not have this control target in
  // m_breakTargets, add this control target to these entries' m_breakTargets
  // fields as appropriate.
  Region* end = r;
  for (r = region; r != end; r = r->m_parent.get()) {
    r->m_breakTargets[depth] =
      ControlTargetInfo(t, alloc && r->isTryFinally());
    if (r->m_kind == Region::Kind::LoopOrSwitch) {
      --depth;
    }
  }
  return t;
}

ControlTargetPtr
EmitterVisitor::registerContinue(StatementPtr s, Region* region, int depth,
                                 bool alloc) {
  ControlTargetPtr t;
  assert(depth >= 1);
  int d = depth;
  Region* r;
  for (r = region; true; r = r->m_parent.get()) {
    assert(r);
    if (r->isFinally()) {
      throw EmitterVisitor::IncludeTimeFatalException(s,
              "Continue jump is not allowed to leave a finally block");
    }
    if (r->m_continueTargets.count(d)) {
      // We registered the control target before. Just return the existing one.
      t = r->m_continueTargets[d].target;
      if (alloc && r->isTryFinally()) {
        r->m_continueTargets[d].used = true;
      }
      break;
    }
    if (r->m_kind != Region::Kind::LoopOrSwitch) {
      continue;
    }
    if (d == 1) {
      // We should never reach this case if alloc == true, since the loop or
      // switch should have registered its continue target in advance
      assert(!alloc);
      t = std::make_shared<ControlTarget>(this);
      r = r->m_parent.get();
      break;
    }
    // Otherwise, delegate to the parent. One continue level has been
    // taken care of by this region.
    --d;
  }
  assert(t != nullptr);
  if (alloc && !t->isRegistered()) {
    registerControlTarget(t.get());
  }
  // For all of the entries that did not have this control target in
  // m_continueTargets, add this control target to these entries'
  // m_continueTargets fields as appropriate.
  Region* end = r;
  for (r = region; r != end; r = r->m_parent.get()) {
    r->m_continueTargets[depth] =
      ControlTargetInfo(t, alloc && r->isTryFinally());
    if (r->m_kind == Region::Kind::LoopOrSwitch) {
      --depth;
    }
  }
  return t;
}

int Region::getCaseCount() {
  int count = 1; // The fall-through case.
  for (auto& t : m_returnTargets) {
    if (t.second.target->isRegistered()) ++count;
  }
  for (auto& t : m_breakTargets) {
    if (t.second.target->isRegistered()) ++count;
  }
  for (auto& t : m_continueTargets) {
    if (t.second.target->isRegistered()) ++count;
  }
  for (auto& t : m_gotoTargets) {
    if (t.second.target->isRegistered()) ++count;
  }
  return count;
}

void EmitterVisitor::emitIterFree(Emitter& e, IterVec& iters) {
  for (auto& iter : iters) {
    assert(iter.id != -1);
    if (iter.kind == KindOfMIter) {
      e.MIterFree(iter.id);
    } else {
      assert(iter.kind == KindOfIter);
      e.IterFree(iter.id);
    }
  }
}

void EmitterVisitor::emitJump(Emitter& e, IterVec& iters, Label& target) {
  if (!iters.empty()) {
    e.IterBreak(target, iters);
    iters.clear();
  } else {
    e.Jmp(target);
  }
}

void EmitterVisitor::emitReturn(Emitter& e, char sym, StatementPtr s) {
  Region* region = m_regions.back().get();
  registerReturn(s, region, sym);
  assert(getEvalStack().size() == 1);
  assert(region->m_returnTargets.count(sym));
  IterVec iters;
  for (Region* r = region; true; r = r->m_parent.get()) {
    auto& t = r->m_returnTargets[sym].target;
    if (r->m_parent == nullptr) {
      // At the top of the hierarchy, no more finally blocks to run.
      // Check return type, free pending iterators and actually return.
      if (sym == StackSym::C) {
        if (shouldEmitVerifyRetType()) {
          e.VerifyRetTypeC();
        }
        // IterFree must come after VerifyRetType, because VerifyRetType may
        // throw, in which case any Iters will be freed by the fault funclet.
        emitIterFree(e, iters);
        e.RetC();
      } else {
        assert(sym == StackSym::V);
        if (shouldEmitVerifyRetType()) {
          e.VerifyRetTypeV();
        }
        emitIterFree(e, iters);
        e.RetV();
      }
      return;
    }

    if (r->isTryFinally()) {
      // We encountered a try block - a finally needs to be run
      // before returning.
      assert(t->isRegistered());
      e.Int(t->m_state);
      emitSetL(e, getStateLocal());
      e.PopC();
      // Emit code stashing the current return value in the "ret" unnamed
      // local
      if (sym == StackSym::C) {
        emitSetL(e, getRetLocal());
        e.PopC();
      } else {
        assert(sym == StackSym::V);
        emitBindL(e, getRetLocal());
        e.PopV();
      }
      emitJump(e, iters, r->m_finallyLabel);
      return;
    }
    if (r->isForeach()) {
      iters.push_back(IterPair(r->m_iterKind, r->m_iterId));
    }
  }
}

void EmitterVisitor::emitGoto(Emitter& e, StringData* name, StatementPtr s) {
  Region* region = m_regions.back().get();
  registerGoto(s, region, name, true);
  assert(region->m_gotoTargets.count(name));
  IterVec iters;
  for (Region* r = region; true; r = r->m_parent.get()) {
    auto t = r->m_gotoTargets[name].target;
    if (r->m_gotoLabels.count(name)) {
      // If only the destination label is within the statement
      // associated with the current statement, just perform a
      // direct jump. Free the pending iterators on the way.
      emitJump(e, iters, t->m_label);
      return;
    }
    if (r->isFinally()) {
      throw EmitterVisitor::IncludeTimeFatalException(s,
        "Goto to a label outside a finally block is not supported");
    }
    if (r->isTryFinally()) {
      // We came across a try region, need to run a finally block.
      // Store appropriate value inside the state local.
      assert(t->isRegistered());
      e.Int(t->m_state);
      emitSetL(e, getStateLocal());
      e.PopC();
      // Jump to the finally block and free any pending iterators on the
      // way.
      emitJump(e, iters, r->m_finallyLabel);
      return;
    }
    if (r->isForeach()) {
      iters.push_back(IterPair(r->m_iterKind, r->m_iterId));
    }
  }
}

void EmitterVisitor::emitBreak(Emitter& e, int depth, StatementPtr s) {
  Region* region = m_regions.back().get();
  registerBreak(s, region, depth, true);
  assert(depth >= 1);
  assert(!region->isFinally());
  assert(region->m_parent != nullptr);
  assert(region->m_breakTargets.count(depth));
  IterVec iters;

  for (Region* r = region; true; r = r->m_parent.get()) {
    auto t = r->m_breakTargets[depth].target;
    if (r->isTryFinally()) {
      // Encountered a try block, need to run finally.
      assert(r->m_breakTargets.count(depth));
      assert(t->isRegistered());
      e.Int(t->m_state);
      emitSetL(e, getStateLocal());
      e.PopC();
      emitJump(e, iters, r->m_finallyLabel);
      return;
    }
    if (r->m_kind != Region::Kind::LoopOrSwitch) {
      continue;
    }
    // Free iterator for the current loop whether or not
    // this is the last loop that we jump out of.
    if (r->isForeach()) {
      iters.push_back(IterPair(r->m_iterKind, r->m_iterId));
    }
    if (depth == 1) {
      // Last loop to jumpt out of. Performa direct jump to the
      // break lable and free any pending iterators left.
      emitJump(e, iters, t->m_label);
      return;
    }
    --depth;
  }
}

void EmitterVisitor::emitContinue(Emitter& e, int depth, StatementPtr s) {
  Region* region = m_regions.back().get();
  registerContinue(s, region, depth, true);
  assert(depth >= 1);
  assert(!region->isFinally());
  assert(region->m_parent != nullptr);
  assert(region->m_continueTargets.count(depth));
  IterVec iters;

  for (Region* r = region; true; r = r->m_parent.get()) {
    auto t = r->m_continueTargets[depth].target;
    if (r->isTryFinally()) {
      // Encountered a try block, need to run finally.
      assert(r->m_continueTargets.count(depth));
      assert(t->isRegistered());
      e.Int(t->m_state);
      emitSetL(e, getStateLocal());
      e.PopC();
      emitJump(e, iters, r->m_finallyLabel);
      return;
    }
    if (r->m_kind != Region::Kind::LoopOrSwitch) {
      continue;
    }
    if (depth == 1) {
      // Last level. Don't free the iterator for the current loop
      // however free any earlier pending iterators.
      emitJump(e, iters, t->m_label);
      return;
    }
    // Only free the iterator for the current loop if this is
    // NOT the last level to continue out of.
    if (r->isForeach()) {
      iters.push_back(IterPair(r->m_iterKind, r->m_iterId));
    }
    --depth;
  }
}

void EmitterVisitor::emitFinallyEpilogue(Emitter& e, Region* region) {
  assert(region != nullptr);
  assert(region->isTryFinally());
  assert(region->m_finallyLabel.isSet());
  int count = region->getCaseCount();
  assert(count >= 1);
  Label after;
  if (count == 1) {
    // If there is only one case (the fall-through case) then we're done
    after.set(e);
    return;
  }
  // Otherwise, we need to emit some conditional jumps/switches to handle
  // the different cases. We start by builing up a vector of Label* that
  // we'll use for the Switch instruction and/or for conditional branches.
  int maxState = region->getMaxState();
  std::vector<Label*> cases;
  while (cases.size() <= maxState) {
    cases.push_back(new Label());
  }
  // Now that we have our vector of Label*'s ready, we can emit a
  // Switch instruction and/or conditional branches, and we can
  // emit the body of each case.
  emitVirtualLocal(getStateLocal());
  emitIsset(e);
  e.JmpZ(after);
  if (count >= 3) {
    // A switch is needed since there are more than two cases.
    emitVirtualLocal(getStateLocal());
    emitCGet(e);
    e.Switch(SwitchKind::Unbounded, 0, cases);
  }
  for (auto& p : region->m_returnTargets) {
    if (p.second.used) emitReturnTrampoline(e, region, cases, p.first);
  }
  assert(region->isTryFinally());
  int max_depth = region->getBreakContinueDepth();
  for (int i = 1; i <= max_depth; ++i) {
    if (region->isBreakUsed(i)) emitBreakTrampoline(e, region, cases, i);
    if (region->isContinueUsed(i)) emitContinueTrampoline(e, region, cases, i);
  }
  for (auto& p : region->m_gotoTargets) {
    if (p.second.used) emitGotoTrampoline(e, region, cases, p.first);
  }
  for (auto c : cases) {
    // Some cases might get assigned state numbers but not actually
    // occur in the try block. We need to set /some/ target for them,
    // so point them here.
    if (!c->isSet()) c->set(e);
    delete c;
  }
  after.set(e);
}

void EmitterVisitor::emitReturnTrampoline(Emitter& e,
                                          Region* region,
                                          std::vector<Label*>& cases,
                                          char sym) {
  assert(region->isTryFinally());
  assert(region->m_parent != nullptr);
  assert(region->m_returnTargets.count(sym));
  auto& t = region->m_returnTargets[sym].target;
  cases[t->m_state]->set(e);

  IterVec iters;
  // We are emitting a case in a finally epilogue, therefore skip
  // the current try region and start from its parent
  for (region = region->m_parent.get(); true; region = region->m_parent.get()) {
    assert(region->m_returnTargets.count(sym));
    assert(region->m_returnTargets[sym].target->isRegistered());
    // Add pending iterator if applicable
    if (region->isForeach()) {
      iters.push_back(IterPair(region->m_iterKind, region->m_iterId));
    }
    if (region->m_parent == nullptr) {
      // At the bottom of the hierarchy. Restore the return value
      // and perform the actual return.
      emitVirtualLocal(getRetLocal());
      if (sym == StackSym::C) {
        emitCGet(e);
        if (shouldEmitVerifyRetType()) {
          e.VerifyRetTypeC();
        }
        e.RetC();
      } else {
        assert(sym == StackSym::V);
        emitVGet(e);
        if (shouldEmitVerifyRetType()) {
          e.VerifyRetTypeV();
        }
        e.RetV();
      }
      return;
    }
    if (region->isTryFinally()) {
      // Encountered another try block, jump to its finally and free
      // iterators on the way.
      emitJump(e, iters, region->m_finallyLabel);
      return;
    }
  }
}

void EmitterVisitor::emitGotoTrampoline(Emitter& e,
                                        Region* region,
                                        std::vector<Label*>& cases,
                                        StringData* name) {
  assert(region->m_gotoTargets.count(name));
  auto t = region->m_gotoTargets[name].target;
  cases[t->m_state]->set(e);
  assert(region->m_parent != nullptr);
  IterVec iters;
  for (region = region->m_parent.get(); true; region = region->m_parent.get()) {
    assert(region->m_gotoTargets.count(name));
    auto t = region->m_gotoTargets[name].target;
    if (region->m_gotoLabels.count(name)) {
      // If only there is the appropriate label inside the current region
      // perform a jump.
      // We need to unset the state unnamed local in order to correctly
      // fall through any future finally blocks.
      emitUnsetL(e, getStateLocal());
      // Jump to the label and free any pending iterators.
      emitJump(e, iters, t->m_label);
      return;
    }
    if (region->isTryFinally()) {
      // Encountered a finally block, jump and free any pending iterators
      emitJump(e, iters, region->m_finallyLabel);
      return;
    }
    // Otherwise we will be jumping out of the current context,
    // therefore if we are in a loop, we need to free the iterator.
    if (region->isForeach()) {
      iters.push_back(IterPair(region->m_iterKind, region->m_iterId));
    }
    // Error, because the label is crossing a finally
    if (region->isFinally()) {
        throw EmitterVisitor::IncludeTimeFatalException(e.getNode(),
          "jump out of a finally block is disallowed");
    }
    // We should never break out of a function, therefore there
    // should always be a parent
    assert(region->m_parent != nullptr);
  }
}

void EmitterVisitor::emitBreakTrampoline(Emitter& e, Region* region,
                                         std::vector<Label*>& cases,
                                         int depth) {
  assert(depth >= 1);
  assert(region->isTryFinally());
  assert(region->m_breakTargets.count(depth));
  auto t = region->m_breakTargets[depth].target;
  cases[t->m_state]->set(e);
  assert(region->m_parent != nullptr);
  IterVec iters;
  for (region = region->m_parent.get(); true; region = region->m_parent.get()) {
    assert(depth >= 1);
    assert(!region->isFinally());
    assert(region->m_parent != nullptr);
    assert(region->m_breakTargets.count(depth));
    auto t = region->m_breakTargets[depth].target;
    if (region->isTryFinally()) {
      // We encountered another try block, jump to the corresponding
      // finally, freeing any iterators on the way.
      emitJump(e, iters, region->m_finallyLabel);
      return;
    }
    if (region->m_kind != Region::Kind::LoopOrSwitch) {
      continue;
    }
    // Whether or not this is the last loop to break out of, we
    // will be freeing the current iterator
    if (region->isForeach()) {
      iters.push_back(IterPair(region->m_iterKind, region->m_iterId));
    }
    if (depth == 1) {
      // This is the last loop to break out of. Unset the state local in
      // order to correctly fall through any future finally blocks
      emitUnsetL(e, getStateLocal());
      // Jump to the break label and free any pending iterators on the
      // way.
      emitJump(e, iters, t->m_label);
      return;
    }
    // Otherwise just delegate to the parent. One loop level has been
    // taken care of.
    --depth;
  }
}

void EmitterVisitor::emitContinueTrampoline(Emitter& e, Region* region,
                                            std::vector<Label*>& cases,
                                            int depth) {
  assert(depth >= 1);
  assert(region->isTryFinally());
  assert(region->m_continueTargets.count(depth));
  auto t = region->m_continueTargets[depth].target;
  cases[t->m_state]->set(e);
  assert(region->m_parent != nullptr);
  IterVec iters;
  for (region = region->m_parent.get(); true; region = region->m_parent.get()) {
    assert(depth >= 1);
    assert(region->m_parent != nullptr);
    assert(!region->isFinally());
    auto t = region->m_continueTargets[depth].target;
    if (region->isTryFinally()) {
      emitJump(e, iters, region->m_finallyLabel);
      return;
    }
    if (region->m_kind != Region::Kind::LoopOrSwitch) {
      continue;
    }
    if (depth == 1) {
      // This is the last loop level to continue out of. Don't free the
      // iterator for the current loop. We need to free the state unnamed
      // local in order to fall through any future finallies correctly
      emitUnsetL(e, getStateLocal());
      // Jump to the continue label and free any pending iterators
      emitJump(e, iters, t->m_label);
      return;
    }
    // This is not the last loop level, therefore the current
    // iterator should be freed.
    if (region->isForeach()) {
      iters.push_back(IterPair(region->m_iterKind, region->m_iterId));
    }
    --depth;
  }
}

bool EmitterVisitor::shouldEmitVerifyRetType() {
  return (m_curFunc->retTypeConstraint.hasConstraint() &&
          !m_curFunc->isGenerator);
}

int Region::getMaxBreakContinueDepth() {
  if (m_parent == nullptr || isFinally()) {
    return 0;
  } else if (m_kind == Region::Kind::LoopOrSwitch) {
    return m_parent->getMaxBreakContinueDepth() + 1;
  } else {
    return m_parent->getMaxBreakContinueDepth();
  }
}

int Region::getBreakContinueDepth() {
  int depth = 0;
  for (auto& p : m_breakTargets) {
    depth = std::max(depth, p.first);
  }
  for (auto& p : m_continueTargets) {
    depth = std::max(depth, p.first);
  }
  return depth;
}

int Region::getMaxState() {
  int maxState = -1;
  for (auto& p : m_returnTargets) {
    if (p.second.used) {
      maxState = std::max(maxState, p.second.target->m_state);
    }
  }
  int max_depth = getBreakContinueDepth();
  for (int i = 1; i <= max_depth; ++i) {
    if (isBreakUsed(i)) {
      maxState = std::max(maxState, m_breakTargets[i].target->m_state);
    }
    if (isContinueUsed(i)) {
      maxState = std::max(maxState, m_continueTargets[i].target->m_state);
    }
  }
  for (auto& p : m_gotoTargets) {
    if (p.second.used) {
      maxState = std::max(maxState, p.second.target->m_state);
    }
  }
  return maxState;
}

RegionPtr
EmitterVisitor::createRegion(StatementPtr s, Region::Kind kind) {
  RegionPtr parent = nullptr;
  if (kind != Region::Kind::FuncBody && kind != Region::Kind::FaultFunclet &&
      kind != Region::Kind::Global && !m_regions.empty()) {
    parent = m_regions.back();
  }
  auto region = std::make_shared<Region>(kind, parent);
  // We preregister all the labels occurring in the provided statement
  // ahead of the time. Therefore at the time of emitting the actual
  // goto instructions we can reliably tell which finally blocks to
  // run.
  for (auto& label : s->getLabelScope()->getLabels()) {
    StringData* nName = makeStaticString(label.getName().c_str());
    if (!region->m_gotoLabels.count(nName)) {
      region->m_gotoLabels.insert(nName);
    }
  }
  return region;
}

void EmitterVisitor::enterRegion(RegionPtr region) {
  assert(region != nullptr);
  m_regions.push_back(region);
}

void EmitterVisitor::leaveRegion(RegionPtr region) {
  assert(region != nullptr);
  assert(m_regions.size() > 0);
  assert(m_regions.back() == region);
  m_regions.pop_back();
}

void EmitterVisitor::registerControlTarget(ControlTarget* t) {
  assert(!t->isRegistered());
  int state = 0;
  while (m_states.count(state)) {
    ++state;
  }
  m_states.insert(state);
  t->m_state = state;
}

void EmitterVisitor::unregisterControlTarget(ControlTarget* t) {
  assert(t->isRegistered());
  int state = t->m_state;
  assert(m_states.count(state));
  m_states.erase(state);
  t->m_state = ControlTarget::k_unsetState;
}

//=============================================================================
// EmitterVisitor.

EmitterVisitor::EmitterVisitor(UnitEmitter& ue)
  : m_ue(ue),
    m_curFunc(ue.getMain()),
    m_evalStackIsUnknown(false),
    m_stateLocal(-1),
    m_retLocal(-1) {
  m_evalStack.m_actualStackHighWaterPtr = &m_curFunc->maxStackCells;
}

EmitterVisitor::~EmitterVisitor() {
  // If a fatal occurs during emission, some extra cleanup is necessary.
  for (std::deque<CatchRegion*>::const_iterator it = m_catchRegions.begin();
       it != m_catchRegions.end(); ++it) {
    delete *it;
  }
}

bool EmitterVisitor::checkIfStackEmpty(const char* forInstruction) const {
  if (m_evalStack.empty()) {
    InvariantViolation("Emitter tried to emit a %s instruction when the "
                       "evaluation stack is empty (at offset %d)",
                       forInstruction,
                       m_ue.bcPos());
    return true;
  }
  return false;
}

void EmitterVisitor::unexpectedStackSym(char sym, const char* where) const {
  InvariantViolation("Emitter encountered an unexpected StackSym \"%s\""
                     " in %s() (at offset %d)",
                     StackSym::ToString(sym).c_str(),
                     where,
                     m_ue.bcPos());
}

void EmitterVisitor::popEvalStack(char expected) {
  // Pop a value off of the evaluation stack, and verify that it
  // matches the specified symbolic flavor
  if (m_evalStack.size() == 0) {
    InvariantViolation("Emitter emitted an instruction that tries to consume "
                       "a value from the stack when the stack is empty "
                       "(expected symbolic flavor \"%s\" at offset %d)",
                       StackSym::ToString(expected).c_str(),
                       m_ue.bcPos());
    return;
  }

  char sym = m_evalStack.top();
  char actual = StackSym::GetSymFlavor(sym);
  m_evalStack.pop();
  if (actual != expected) {
    InvariantViolation(
      "Emitter emitted an instruction that tries to consume a "
      "value from the stack when the top of the stack does not "
      "match the symbolic flavor that the instruction expects "
      "(expected symbolic flavor \"%s\", actual symbolic flavor \"%s\" "
      "at offset %d)",
      StackSym::ToString(expected).c_str(),
      StackSym::ToString(actual).c_str(),
      m_ue.bcPos());
  }
}

int EmitterVisitor::popClsRefSlot() {
  if (m_evalStack.clsRefSlotStackEmpty()) {
    InvariantViolation("Emitter emitted an instruction that tries to consume "
                       "a class-ref slot when the class-ref slot stack is "
                       "empty (at offset %d)",
                       m_ue.bcPos());
    return 0;
  }
  return m_evalStack.popClsRefSlot();
}

void EmitterVisitor::popSymbolicLocal(OpEmitContext& ctx) {
  // A number of member instructions read locals without consuming an L from
  // the symbolic stack through the normal path.
  if (isMemberBaseOp(ctx.op) ||
      isMemberDimOp(ctx.op) ||
      isMemberFinalOp(ctx.op)) {
    return;
  }

  int belowTop = (ctx.op == OpCGetL2) ? 2 : 1;

  if (ctx.op == OpCGetL || ctx.op == OpCGetL2) {
    // If there's a symbolic class-ref where we would expect the symbolic local
    // to be, skip over it. We need to keep the class-ref for now as it will be
    // popped later by popSymbolicClassRef().
    char symFlavor = StackSym::GetSymFlavor(
      m_evalStack.get(m_evalStack.size() - belowTop)
    );
    if (symFlavor == StackSym::A) {
      ++belowTop;
      ctx.cgetlPopSkippedClsRef = true;
    }
  }

  if (belowTop > 1) {
    char symFlavor = StackSym::GetSymFlavor(
      m_evalStack.get(m_evalStack.size() - belowTop)
    );
    if (symFlavor != StackSym::L) {
      InvariantViolation("Operation tried to remove a local below the top of"
                         " the symbolic stack but instead found \"%s\"",
                         StackSym::ToString(symFlavor).c_str());
    }
    m_evalStack.consumeBelowTop(belowTop - 1);
  } else {
    popEvalStack(StackSym::L);
  }
}

void EmitterVisitor::pushSymbolicClassRef() {
  ITRACE(3, "pushSymbolicClassRef()\n");
  Trace::Indent i;
  pushEvalStack(StackSym::A);
}

void EmitterVisitor::popSymbolicClassRef(OpEmitContext& ctx) {
  ITRACE(3, "popSymbolicClassRef()\n");
  Trace::Indent i;

  if (ctx.op == OpSetS || ctx.op == OpSetOpS || ctx.op == OpBindS) {
    // These ops should always have the symbolic class-ref as the second
    // element.
    char symFlavor = StackSym::GetSymFlavor(
      m_evalStack.get(m_evalStack.size() - 2)
    );
    if (symFlavor != StackSym::A) {
      InvariantViolation("Operation tried to remove a symbolic class-ref below "
                         "the top of the symbolic stock but instead found "
                         "\"%s\"", StackSym::ToString(symFlavor).c_str());
    }
    m_evalStack.consumeBelowTop(1);
  } else if (ctx.op == OpBaseSC || ctx.op == OpBaseSL) {
    // BaseSC and BaseSL can have an optional value on the stack, so check for
    // the symbolic class-ref on the top two elements.
    if (m_evalStack.size() > 1) {
      char symFlavor = StackSym::GetSymFlavor(
        m_evalStack.get(m_evalStack.size() - 2)
      );
      if (symFlavor == StackSym::A) {
        m_evalStack.consumeBelowTop(1);
        return;
      }
    }
    popEvalStack(StackSym::A);
  } else {
    popEvalStack(StackSym::A);
  }
}

void EmitterVisitor::popEvalStackMMany() {
  ITRACE(3, "popEvalStackMMany()\n");
  Trace::Indent i;

  ITRACE(3, "popping member codes\n");
  while (!m_evalStack.empty()) {
    char sym = m_evalStack.top();
    char symFlavor = StackSym::GetSymFlavor(sym);
    char marker = StackSym::GetMarker(sym);
    if (marker == StackSym::E || marker == StackSym::P ||
        marker == StackSym::Q) {
      if (symFlavor != StackSym::C && symFlavor != StackSym::L &&
          symFlavor != StackSym::T && symFlavor != StackSym::I) {
        InvariantViolation(
          "Emitter emitted an instruction that tries to consume "
          "a value from the stack when the top of the stack "
          "does not match the symbolic flavor that the instruction "
          "expects (expected symbolic flavor \"C\", \"L\", \"T\", or \"I\", "
          "actual symbolic flavor \"%s\" at offset %d)",
          StackSym::ToString(symFlavor).c_str(),
          m_ue.bcPos());
      }
    } else if (marker == StackSym::W) {
      if (symFlavor != StackSym::None) {
        InvariantViolation(
          "Emitter emitted an instruction that tries to consume "
          "a value from the stack when the top of the stack "
          "does not match the symbolic flavor that the instruction "
          "expects (expected symbolic flavor \"None\", actual "
          "symbolic flavor \"%s\" at offset %d)",
          StackSym::ToString(symFlavor).c_str(),
          m_ue.bcPos());
      }
    } else if (marker == StackSym::M) {
      assert(symFlavor == StackSym::A);
    } else {
      break;
    }
    m_evalStack.pop();
  }

  if (m_evalStack.empty()) {
    InvariantViolation("Emitter emitted an instruction that tries to consume "
                       "a value from the stack when the stack is empty "
                       "(at offset %d)",
                       m_ue.bcPos());
    return;
  }

  ITRACE(3, "popping location\n");
  char sym = m_evalStack.top();
  char symFlavor = StackSym::GetSymFlavor(sym);
  m_evalStack.pop();
  if (symFlavor != StackSym::C && symFlavor != StackSym::L &&
      symFlavor != StackSym::R && symFlavor != StackSym::H) {
    InvariantViolation(
      "Emitter emitted an instruction that tries to consume a "
      "value from the stack when the top of the stack does not "
      "match the symbolic flavor that the instruction expects "
      "(expected symbolic flavor \"C\", \"L\", \"R\", or \"H\", actual "
      "symbolic flavor \"%s\" at offset %d)",
      StackSym::ToString(symFlavor).c_str(),
      m_ue.bcPos());
  }
}

void EmitterVisitor::popEvalStackMany(int len, char symFlavor) {
  for (int i = 0; i < len; ++i) {
    popEvalStack(symFlavor);
  }
}

void EmitterVisitor::popEvalStackCVMany(int len) {
  for (int i = 0; i < len; i++) {
    if (m_evalStack.size() == 0) {
      InvariantViolation("Emitter emitted an instruction that tries to consume "
                         "a value from the stack when the stack is empty "
                         "(expected symbolic flavor C or V at offset %d)",
                         m_ue.bcPos());
      return;
    }

    char sym = m_evalStack.top();
    char actual = StackSym::GetSymFlavor(sym);
    m_evalStack.pop();
    if (actual != StackSym::C && actual != StackSym::V) {
      InvariantViolation(
        "Emitter emitted an instruction that tries to consume a "
        "value from the stack when the top of the stack does not "
        "match the symbolic flavor that the instruction expects "
        "(expected symbolic flavor C or V, actual symbolic flavor \"%s\" "
        "at offset %d)",
        StackSym::ToString(actual).c_str(),
        m_ue.bcPos());
    }
  }
}

void EmitterVisitor::pushEvalStack(char symFlavor) {
  // Push a value from the evaluation stack with the specified
  // symbolic flavor
  m_evalStack.push(symFlavor);
}

// Push an element onto the eval stack in the context of emitting an op.
void EmitterVisitor::pushEvalStackFromOp(char symFlavor, OpEmitContext& ctx) {
  if (ctx.op == OpCGetL && ctx.cgetlPopSkippedClsRef) {
    // CGetL is somewhat special. Before popping the symbolic local from the
    // stack, we could have either [A L] or [L A], which mean different
    // things. After popping the symbolic local they both just become [A]
    // here. So, we record whether we skipped over a symbolic class-ref element
    // when popping the symbolic local. This lets us disambiguate the two
    // cases. For the [L A] case we want to push this new cell past the symbolic
    // class-ref. Otherwise we push it on front.
    assert(symFlavor == StackSym::C);
    assert(m_evalStack.size() > 0);
    assert(StackSym::GetSymFlavor(m_evalStack.get(m_evalStack.size() - 1))
           == StackSym::A);
    return m_evalStack.pushSkipSymbolicTop(symFlavor);
  }
  pushEvalStack(symFlavor);
}

// Insert an element in the eval stack in the context of emitting an op.
void EmitterVisitor::insertEvalStackFromOp(char symFlavor,
                                           int depth,
                                           OpEmitContext& ctx) {
  assert(depth > 0);
  if (ctx.op == OpCGetL2 && ctx.cgetlPopSkippedClsRef) {
    // Same deal as above, but with CGetL2.
    assert(symFlavor == StackSym::C);
    assert(m_evalStack.size() > depth);
    assert(StackSym::GetSymFlavor(
             m_evalStack.get(m_evalStack.size() - depth - 1)
           ) == StackSym::A);
    return m_evalStack.symbolicInsertAt(depth + 1, symFlavor);
  }
  m_evalStack.insertAt(depth, symFlavor);
}

int EmitterVisitor::pushClsRefSlot() {
  return m_evalStack.pushClsRefSlot();
}

void EmitterVisitor::peekEvalStack(char expected, int depthActual) {
  int posActual = (m_evalStack.sizeActual() - depthActual - 1);
  if (posActual >= 0 && posActual < (int)m_evalStack.sizeActual()) {
    char sym = m_evalStack.getActual(posActual);
    char actual = StackSym::GetSymFlavor(sym);
    if (actual != expected) {
      InvariantViolation(
        "Emitter emitted an instruction that tries to consume a "
        "value from the stack whose symbolic flavor does not match "
        "the symbolic flavor that the instruction expects (expected "
        "symbolic flavor \"%s\", actual symbolic flavor \"%s\" at "
        "offset %d)",
        StackSym::ToString(expected).c_str(),
        StackSym::ToString(actual).c_str(),
        m_ue.bcPos());
    }
  }
}

void EmitterVisitor::pokeEvalStack(char symFlavor, int depthActual) {
  int sizeActual = m_evalStack.sizeActual();
  int posActual = sizeActual - depthActual - 1;
  if (posActual >= 0 && posActual < sizeActual) {
    m_evalStack.setActual(posActual, symFlavor);
  }
}

/*
 * Prior to making any changes to the evaluation stack in between
 * instructions, this function should be called.
 *
 * What this handles is recording the evaluation stack state at
 * instruction boundaries so that jumps know what their stack state
 * will be at the destination.
 *
 * When m_evalStackIsUnknown, it means we have hit a place in the
 * bytecode where the offset cannot be reached via fallthrough, but no
 * forward jumps targeted it either.  In this case, the stack must be
 * empty, and we need to record that this is the case so that later
 * backward jumps can check this is the case at their jump site.
 */
void EmitterVisitor::prepareEvalStack() {
  if (m_evalStackIsUnknown) {
    if (!m_evalStack.empty()) {
      InvariantViolation("Emitter expected to have an empty evaluation "
                         "stack because the eval stack was unknown, but "
                         "it was non-empty.");
      return;
    }
    if (!m_evalStack.clsRefSlotStackEmpty()) {
      InvariantViolation("Emitter expected to not have any class-ref slots "
                         "in use because the eval stack was unknown, but "
                         "there are some in use.");
      return;
    }
    // Record that we are assuming that the eval stack is empty
    recordJumpTarget(m_ue.bcPos(), m_evalStack);
    m_evalStackIsUnknown = false;
  }
}

void EmitterVisitor::recordJumpTarget(Offset target,
                                      const SymbolicStack& evalStack) {
  if (target == InvalidAbsoluteOffset) {
    InvariantViolation(
      "Offset passed to EmitterVisitor::recordJumpTarget was invalid");
  }
  auto it = m_jumpTargetEvalStacks.find(target);
  if (it == m_jumpTargetEvalStacks.end()) {
    m_jumpTargetEvalStacks[target] = evalStack;
    return;
  }
  checkJmpTargetEvalStack(evalStack, it->second);
}

void EmitterVisitor::restoreJumpTargetEvalStack() {
  m_evalStack.clear();
  auto it = m_jumpTargetEvalStacks.find(m_ue.bcPos());
  if (it == m_jumpTargetEvalStacks.end()) {
    m_evalStackIsUnknown = true;
    return;
  }
  m_evalStack = it->second;
}

void EmitterVisitor::recordCall() {
  m_curFunc->containsCalls = true;
}

bool EmitterVisitor::isJumpTarget(Offset target) {
  // Returns true iff one of the following conditions is true:
  //   1) We have seen an instruction that jumps to the specified offset
  //   2) We know of a Label that has been set to the specified offset
  //   3) We have seen a try region that ends at the specified offset
  auto it = m_jumpTargetEvalStacks.find(target);
  return (it != m_jumpTargetEvalStacks.end());
}

struct IterFreeThunklet final : Thunklet {
  IterFreeThunklet(Id iterId, bool itRef)
    : m_id(iterId), m_itRef(itRef) {}
  void emit(Emitter& e) override {
    if (m_itRef) {
      e.MIterFree(m_id);
    } else {
      e.IterFree(m_id);
    }
    e.Unwind();
  }
private:
  Id m_id;
  bool m_itRef;
};

/**
 * A thunklet for the fault region protecting a silenced (@) expression.
 */
struct RestoreErrorReportingThunklet final : Thunklet {
  explicit RestoreErrorReportingThunklet(Id loc)
    : m_oldLevelLoc(loc) {}
  void emit(Emitter& e) override {
    e.getEmitterVisitor().emitRestoreErrorReporting(e, m_oldLevelLoc);
    e.Unwind();
  }
private:
  Id m_oldLevelLoc;
};

struct UnsetUnnamedLocalThunklet final : Thunklet {
  explicit UnsetUnnamedLocalThunklet(Id loc)
    : m_loc(loc) {}
  void emit(Emitter& e) override {
    e.getEmitterVisitor().emitUnsetL(e, m_loc);
    e.Unwind();
  }
private:
  Id m_loc;
};

struct UnsetUnnamedLocalsThunklet final : Thunklet {
  explicit UnsetUnnamedLocalsThunklet(std::vector<Id>&& locs)
    : m_locs(std::move(locs)) {}
  void emit(Emitter& e) override {
    auto& visitor = e.getEmitterVisitor();
    for (auto loc : m_locs) {
      visitor.emitUnsetL(e, loc);
    }
    e.Unwind();
  }
private:
  const std::vector<Id> m_locs;
};

struct UnsetGeneratorDelegateThunklet final : Thunklet {
  explicit UnsetGeneratorDelegateThunklet(Id iterId)
    : m_id(iterId) {}
  void emit(Emitter& e) override {
    e.ContUnsetDelegate(CudOp::FreeIter, m_id);
    e.Unwind();
  }
private:
  Id m_id;
};

struct FinallyThunklet final : Thunklet {
  explicit FinallyThunklet(FinallyStatementPtr finallyStatement,
                           int numLiveIters)
      : m_finallyStatement(finallyStatement), m_numLiveIters(numLiveIters) {}
  void emit(Emitter& e) override {
    auto& visitor = e.getEmitterVisitor();
    auto region =
      visitor.createRegion(m_finallyStatement, Region::Kind::FaultFunclet);
    visitor.enterRegion(region);
    SCOPE_EXIT { visitor.leaveRegion(region); };
    visitor.emitUnsetL(e, visitor.getStateLocal());
    visitor.emitUnsetL(e, visitor.getRetLocal());
    auto* func = visitor.getFuncEmitter();
    int oldNumLiveIters = func->numLiveIterators();
    func->setNumLiveIterators(m_numLiveIters);
    SCOPE_EXIT { func->setNumLiveIterators(oldNumLiveIters); };
    visitor.visit(m_finallyStatement);
    e.Unwind();
  }
private:
  FinallyStatementPtr m_finallyStatement;
  int m_numLiveIters;
};

/**
 * Helper to deal with emitting list assignment and keeping track of some
 * associated info.  A list assignment can be thought of as a list of "index
 * chains"; that is, sequences of indices that should be accessed for each
 * bottom-level expression in the list assignment.  We recursively walk down the
 * LHS, building up index chains and copying them into the top-level list as we
 * reach the leaves of the tree.
 */
void EmitterVisitor::listAssignmentVisitLHS(Emitter& e, ExpressionPtr exp,
                                            IndexChain& indexChain,
                                            std::vector<IndexPair>& all) {
  if (!exp) {
    // Empty slot
    return;
  }

  if (exp->is(Expression::KindOfListAssignment)) {
    // Nested assignment
    auto la = static_pointer_cast<ListAssignment>(exp);
    auto lhs = la->getVariables();
    int n = lhs->getCount();
    for (int i = 0; i < n; ++i) {
      indexChain.push_back(i);
      listAssignmentVisitLHS(e, (*lhs)[i], indexChain, all);
      indexChain.pop_back();
    }
  } else {
    // Reached a "leaf".  Lock in this index chain and deal with this exp.
    assert(!indexChain.empty());
    all.emplace_back(exp, IndexChain(indexChain));

    // First: the order we visit the LHS elements matters, as does whether we
    // do the RHS or LHS first, for things like:
    // list($a[$n++], $b[$n++]) = $c[$n++]
    //
    // In PHP5 mode, we visit the LHS elements of the list() now. This does
    // two things: it causes their side effects to happen in LTR order, but
    // since they are pushed onto the m_evalStack they are actually asigned to
    // in LIFO order, e.g., RTL, in listAssignmentAssignElements below.
    //
    // In PHP7 mode, we need to visit the elements in LTR order so their side
    // effects take place in that order, but we *also* need to assign them in
    // LTR order, so we can't push them onto the m_evalStack right now. Since
    // visit() does both of these things, we need to delay calling visit()
    // until listAssignmentAssignElements below. This also has the side effect
    // of making isRhsFirst() effectively always on in PHP7 mode when doing
    // list() assignment (since we delay the visit() until after the check
    // anyways), which turns out to be the right behavior.
    if (!RuntimeOption::PHP7_LTR_assign) {
      visit(exp);
      emitClsIfSPropBase(e);
    }
  }
}

void EmitterVisitor::listAssignmentAssignElements(
  Emitter& e,
  std::vector<IndexPair>& indexPairs,
  std::function<void()> emitSrc
) {

  // PHP5 does list() assignments RTL, PHP7 does them LTR, so this loop can go
  // either way and looks a little ugly. The assignment order normally isn't
  // visible, but it is if you do something like:
  // list($a[], $a[]) = $foo
  auto const ltr = RuntimeOption::PHP7_LTR_assign;
  for (int i = ltr ? 0 : (int)indexPairs.size() - 1;
       i >= 0 && i < (int)indexPairs.size();
       ltr ? i++ : i--) {
    if (ltr) {
      // Visit now, so we can both eval LTR and assign LTR. See comment in
      // listAssignmentVisitLHS.
      visit(indexPairs[i].first);
      emitClsIfSPropBase(e);
    }

    IndexChain& currIndexChain = indexPairs[i].second;
    if (currIndexChain.empty()) {
      continue;
    }

    if (emitSrc == nullptr) {
      e.Null();
    } else {
      emitSrc();
      for (int j = 0; j < (int)currIndexChain.size(); ++j) {
        m_evalStack.push(StackSym::I);
        m_evalStack.setInt(currIndexChain[j]);
        markElem(e);
      }
      emitCGet(e);
    }

    emitSet(e);
    emitPop(e);
  }
}

/**
 * visitIfCondition() serves as a helper method for visiting the condition
 * of an "if" statement. This function recursively visits each node in an
 * expression, keeping track of where to jump if an "and" or "or" expression
 * short circuits. If an expression other than "and", "or", or "not" is
 * encountered, this method calls EmitterVisitor::visit() to handle the
 * expression.
 */
void EmitterVisitor::visitIfCondition(
  ExpressionPtr cond, Emitter& e, Label& tru, Label& fals,
  bool truFallthrough) {

  auto binOpNode = dynamic_pointer_cast<BinaryOpExpression>(cond);

  if (binOpNode) {
    int op = binOpNode->getOp();
    // Short circuit && and ||
    if (op == T_LOGICAL_OR || op == T_LOGICAL_AND ||
        op == T_BOOLEAN_OR || op == T_BOOLEAN_AND) {
      bool isOr = (op == T_LOGICAL_OR || op == T_BOOLEAN_OR);
      Label localLabel;
      ExpressionPtr lhs = binOpNode->getExp1();
      if (isOr) {
        Emitter lhsEmitter(lhs, m_ue, *this);
        visitIfCondition(lhs, lhsEmitter, tru, localLabel, false);
        // falls through if that condition was false
      } else {
        Emitter lhsEmitter(lhs, m_ue, *this);
        visitIfCondition(lhs, lhsEmitter, localLabel, fals, true);
        // falls through if that condition was true
      }
      if (localLabel.isUsed()) {
        localLabel.set(e);
      }
      if (currentPositionIsReachable()) {
        ExpressionPtr rhs = binOpNode->getExp2();
        Emitter rhsEmitter(rhs, m_ue, *this);
        visitIfCondition(rhs, rhsEmitter, tru, fals, truFallthrough);
      }
      return;
    }
  }

  auto unOpNode = dynamic_pointer_cast<UnaryOpExpression>(cond);
  if (unOpNode) {
    int op = unOpNode->getOp();
    // Logical not
    if (op == '!') {
      ExpressionPtr val = unOpNode->getExpression();
      Emitter valEmitter(val, m_ue, *this);
      visitIfCondition(val, valEmitter, fals, tru, !truFallthrough);
      return;
    }
  }

  Variant val;
  if (cond->getScalarValue(val)) {
    if (truFallthrough) {
      if (!val.toBoolean()) e.Jmp(fals);
    } else {
      if (val.toBoolean()) e.Jmp(tru);
    }
    return;
  }

  visit(cond);
  emitConvertToCell(e);
  if (truFallthrough) {
    e.JmpZ(fals);
  } else {
    e.JmpNZ(tru);
  }
}

// Assigns ids to all of the local variables eagerly. This gives us the
// nice property that all named local variables will be assigned ids
// 0 through k-1, while any unnamed local variable will have an id >= k.
void EmitterVisitor::assignLocalVariableIds(FunctionScopePtr fs) {
  VariableTablePtr variables = fs->getVariables();
  std::vector<std::string> localNames;
  variables->getLocalVariableNames(localNames);
  for (int i = 0; i < (int)localNames.size(); ++i) {
    StringData* nLiteral = makeStaticString(localNames[i].c_str());
    m_curFunc->allocVarId(nLiteral);
  }
}

void EmitterVisitor::assignFinallyVariableIds() {
  assert(m_stateLocal < 0);
  m_stateLocal = m_curFunc->allocUnnamedLocal();
  assert(m_retLocal < 0);
  m_retLocal = m_curFunc->allocUnnamedLocal();
}

void EmitterVisitor::visit(FileScopePtr file) {
  const std::string& filename = file->getName();
  m_ue.m_filepath = makeStaticString(filename);
  m_ue.m_isHHFile = file->isHHFile();
  m_ue.m_useStrictTypes = file->useStrictTypes();
  m_ue.m_useStrictTypesForBuiltins = file->useStrictTypesForBuiltins();

  FunctionScopePtr func(file->getPseudoMain());
  if (!func) return;

  SCOPE_ASSERT_DETAIL("visit FileScope") { return m_evalStack.pretty(); };
  ITRACE(1, "Emitting file {}\n", file->getName());
  Trace::Indent indent;

  m_file = file;
  assignLocalVariableIds(func);

  AnalysisResultPtr ar(file->getContainingProgram());
  assert(ar);
  auto m = dynamic_pointer_cast<MethodStatement>(func->getStmt());
  if (!m) return;
  StatementListPtr stmts(m->getStmts());
  if (!stmts) return;

  Emitter e(m, m_ue, *this);

  int i, nk = stmts->getCount();
  for (i = 0; i < nk; i++) {
    StatementPtr s = (*stmts)[i];
    if (auto meth = dynamic_pointer_cast<MethodStatement>(s)) {
      if (auto msg = s->getFunctionScope()->getFatalMessage()) {
        throw IncludeTimeFatalException(s, msg->data());
      }
      // Emit afterwards
      postponeMeth(meth, nullptr, true);
    }
  }
  {
    FunctionScopePtr fsp = m->getFunctionScope();
    if (fsp->needsLocalThis()) {
      static const StringData* thisStr = makeStaticString("this");
      Id thisId = m_curFunc->lookupVarId(thisStr);
      emitVirtualLocal(thisId);
      e.InitThisLoc(thisId);
    }
    if (fsp->needsFinallyLocals()) {
      assignFinallyVariableIds();
    }
    FuncFinisher ff(this, e, m_curFunc);
    TypedValue mainReturn;
    mainReturn.m_type = kInvalidDataType;
    bool notMergeOnly = false;

    if (Option::UseHHBBC && SystemLib::s_inited) notMergeOnly = true;

    auto region = createRegion(stmts, Region::Kind::Global);
    enterRegion(region);
    SCOPE_EXIT { leaveRegion(region); };

    for (i = 0; i < nk; i++) {
      StatementPtr s = (*stmts)[i];
      e.setTempLocation(s->getRange());
      switch (s->getKindOf()) {
        case Statement::KindOfMethodStatement:
        case Statement::KindOfFunctionStatement:
          break;
        case Statement::KindOfInterfaceStatement:
        case Statement::KindOfClassStatement: {
          // Handle classes directly here, since only top-level classes are
          // hoistable.
          ClassScopePtr cNode = s->getClassScope();
          emitClass(e, cNode, true);
          if (cNode->getFatalMessage()) {
            notMergeOnly = true;
          }
          break;
        }
        case Construct::KindOfDeclareStatement: {
          auto ds = static_pointer_cast<DeclareStatement>(s);
          for (auto& decl : ds->getDeclareMap()) {
            if (decl.first == "strict_types") {
              if (ds->getBlock()->getStmts()->getCount()) {
                emitMakeUnitFatal(e, "strict_types declaration must not use "
                                  "block mode");
                break;
              }
              if (!RuntimeOption::PHP7_ScalarTypes) {
                emitMakeUnitFatal(e, "strict_types can only be used when "
                                  "hhvm.php7.scalar_types = true");
                break;
              }
            }
          }

          visit(ds->getBlock());
          break;
        }
        case Statement::KindOfTypedefStatement: {
          auto const id =
            emitTypedef(e, static_pointer_cast<TypedefStatement>(s));
          m_ue.pushMergeableTypeAlias(Unit::MergeKind::TypeAlias, id);
          break;
        }
        case Statement::KindOfReturnStatement:
          if (mainReturn.m_type != kInvalidDataType) break;

          visit(s);
          if (notMergeOnly) {
            tvWriteUninit(&mainReturn);
            m_ue.m_returnSeen = true;
            continue;
          }

          {
            auto r = static_pointer_cast<ReturnStatement>(s);
            Variant v((Variant::NullInit()));
            if (r->getRetExp() &&
                !r->getRetExp()->getScalarValue(v)) {
              tvWriteUninit(&mainReturn);
              notMergeOnly = true;
              continue;
            }
            if (v.isString()) {
              v = String(makeStaticString(v.asCStrRef().get()));
            } else if (v.isArray()) {
              v = Array(ArrayData::GetScalarArray(v.asCArrRef().get()));
            } else {
              assert(v.isInitialized());
              assert(!isRefcountedType(v.getType()));
            }
            mainReturn = *v.asCell();
            m_ue.m_returnSeen = true;
          }
          break;
        case Statement::KindOfExpStatement:
          if (mainReturn.m_type == kInvalidDataType) {
            auto e = static_pointer_cast<ExpStatement>(s)->getExpression();
            switch (e->getKindOf()) {
              case Expression::KindOfSimpleFunctionCall: {
                auto func = static_pointer_cast<SimpleFunctionCall>(e);
                StringData *name;
                TypedValue tv;
                if (func->isSimpleDefine(&name, &tv)) {
                  auto k = func->isDefineWithoutImpl(ar)
                    ? Unit::MergeKind::PersistentDefine
                    : Unit::MergeKind::Define;
                  if (tv.m_type == KindOfUninit) {
                    tv.m_type = KindOfNull;
                  }
                  m_ue.pushMergeableDef(k, name, tv);
                  visit(s);
                  continue;
                }
                break;
              }
              case Expression::KindOfAssignmentExpression: {
                auto ae = static_pointer_cast<AssignmentExpression>(e);
                StringData *name;
                TypedValue tv;
                if (ae->isSimpleGlobalAssign(&name, &tv)) {
                  m_ue.pushMergeableDef(Unit::MergeKind::Global, name, tv);
                  visit(s);
                  continue;
                }
                break;
              }
              case Expression::KindOfIncludeExpression: {
                auto inc = static_pointer_cast<IncludeExpression>(e);
                if (inc->isReqLit()) {
                  if (FileScopeRawPtr f = inc->getIncludedFile(ar)) {
                    if (StatementListPtr sl = f->getStmt()) {
                      FunctionScopeRawPtr ps DEBUG_ONLY =
                        sl->getFunctionScope();
                      assert(ps && ps->inPseudoMain());
                      m_ue.pushMergeableInclude(
                        Unit::MergeKind::ReqDoc,
                        makeStaticString(inc->includePath()));
                      visit(s);
                      continue;
                    }
                  }
                }
                break;
              }
              default:
                break;
            }
          } // fall through
        default:
          if (mainReturn.m_type != kInvalidDataType) break;
          notMergeOnly = true;
          visit(s);
      }
    }

    if (!notMergeOnly) {
      m_ue.m_mergeOnly = true;
      if (mainReturn.m_type == kInvalidDataType) {
        tvWriteUninit(&mainReturn);
        if (boost::algorithm::ends_with(filename, EVAL_FILENAME_SUFFIX)) {
          tvAsVariant(&mainReturn) = init_null();
        } else {
          tvAsVariant(&mainReturn) = 1;
        }
      }
      m_ue.m_mainReturn = mainReturn;
    }

    // Pseudo-main returns the integer value 1 by default. If the
    // current position in the bytecode is reachable, emit code to
    // return 1.
    if (currentPositionIsReachable()) {
      Location::Range loc;
      if (m_ue.bcPos() > 0) loc.line0 = -1;
      e.setTempLocation(loc);
      if (boost::algorithm::ends_with(filename, EVAL_FILENAME_SUFFIX)) {
        e.Null();
      } else {
        e.Int(1);
      }
      e.RetC();
      e.setTempLocation(OptLocation());
    }
  }

  if (!m_evalStack.empty()) {
    InvariantViolation("Eval stack was not empty as expected before "
                       "emitPostponed* phase");
  }
  if (!m_evalStack.clsRefSlotStackEmpty()) {
    InvariantViolation("Class-ref slots aren't not in use as expected before "
                       "emitPostponed* phase");
  }

  // Method bodies
  emitPostponedMeths();
  emitPostponedCtors();
  emitPostponedPinits();
  emitPostponedSinits();
  emitPostponedCinits();
}

static StringData* getClassName(ExpressionPtr e) {
  ClassScopeRawPtr cls;
  if (e->isThis()) {
    cls = e->getClassScope();
  }
  if (cls && !cls->isTrait()) {
    return makeStaticString(cls->getScopeName());
  }
  return nullptr;
}

void EmitterVisitor::visitKids(ConstructPtr c) {
  for (int i = 0, nk = c->getKidCount(); i < nk; i++) {
    ConstructPtr kid(c->getNthKid(i));
    visit(kid);
  }
}

template<uint32_t MaxMakeSize, class Fun>
bool checkKeys(ExpressionPtr init_expr, bool check_size, Fun fun) {
  if (init_expr->getKindOf() != Expression::KindOfExpressionList) {
    return false;
  }

  auto el = static_pointer_cast<ExpressionList>(init_expr);
  int n = el->getCount();
  if (n < 1 || (check_size && n > MaxMakeSize)) {
    return false;
  }

  for (int i = 0; i < n; ++i) {
    ExpressionPtr ex = (*el)[i];
    if (ex->getKindOf() != Expression::KindOfArrayPairExpression) {
      return false;
    }
    auto ap = static_pointer_cast<ArrayPairExpression>(ex);
    if (ap->isRef()) return false;
    if (!fun(ap)) return false;
  }
  return true;
}

/*
 * isPackedInit() returns true if this expression list looks like an
 * array with no keys and no ref values; e.g. array(x,y,z).
 *
 * In this case we can NewPackedArray to create the array. The elements are
 * pushed on the stack, so we arbitrarily limit this to a small multiple of
 * MixedArray::SmallSize (12).
 */
bool isPackedInit(ExpressionPtr init_expr, int* size,
                  bool check_size = true, bool hack_arr_compat = true) {
  *size = 0;
  return checkKeys<MixedArray::MaxMakeSize>(init_expr, check_size,
    [&](ArrayPairExpressionPtr ap) {
      Variant key;

      // If we have a key...
      if (ap->getName() != nullptr) {
        // ...and it has no scalar value, bail.
        if (!ap->getName()->getScalarValue(key)) return false;

        if (key.isInteger()) {
          // If it's an integer key, check if it's the next packed index.
          if (key.asInt64Val() != *size) return false;
        } else if (key.isBoolean()) {
          // Bool to Int conversion
          if (hack_arr_compat &&
              RuntimeOption::EvalHackArrCompatNotices) return false;
          if (static_cast<int>(key.asBooleanVal()) != *size) return false;
        } else {
          // Give up if it's not a string.
          if (!key.isString()) return false;

          if (hack_arr_compat &&
              RuntimeOption::EvalHackArrCompatNotices) return false;

          int64_t i; double d;
          auto numtype = key.getStringData()->isNumericWithVal(i, d, false);

          // If it's a string of the next packed index,
          if (numtype != KindOfInt64 || i != *size) return false;
        }
      }

      (*size)++;
      return true;
    });
}

/*
 * isStructInit() is like isPackedInit(), but returns true if the keys are
 * all static strings with no duplicates.
 */
bool isStructInit(ExpressionPtr init_expr, std::vector<std::string>& keys) {
  return checkKeys<MixedArray::MaxStructMakeSize>(init_expr, true,
    [&](ArrayPairExpressionPtr ap) {
      auto key = ap->getName();
      if (key == nullptr || !key->isLiteralString()) return false;
      auto name = key->getLiteralString();
      int64_t ival;
      double dval;
      auto kind = is_numeric_string(name.data(), name.size(), &ival, &dval, 0);
      if (kind != KindOfNull) return false; // don't allow numeric keys
      if (std::find(keys.begin(), keys.end(), name) != keys.end()) return false;
      keys.push_back(name);
      return true;
    });
}

void EmitterVisitor::emitCall(Emitter& e,
                              FunctionCallPtr func,
                              ExpressionListPtr params,
                              Offset fpiStart) {
  auto const numParams = params ? params->getCount() : 0;
  auto const unpack = func->hasUnpack();
  if (!func->checkUnpackParams()) {
    throw IncludeTimeFatalException(
      func, "Only the last parameter in a function call is allowed to use ...");
  }
  {
    FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
    for (int i = 0; i < numParams; i++) {
      auto param = (*params)[i];
      emitFuncCallArg(e, param, i, param->isUnpack());
    }
  }

  if (unpack) {
    e.FCallUnpack(numParams);
  } else {
    e.FCall(numParams);
  }
}

bool EmitterVisitor::visit(ConstructPtr node) {
  if (!node) return false;

  SCOPE_ASSERT_DETAIL("visit Construct") { return node->getText(); };

  Emitter e(node, m_ue, *this);

  switch (node->getKindOf()) {
  case Construct::KindOfBlockStatement:
  case Construct::KindOfStatementList:
    visitKids(node);
    return false;

  case Construct::KindOfTypedefStatement: {
    emitMakeUnitFatal(e, "Type statements are currently only allowed at "
                         "the top-level");
    return false;
  }

  case Construct::KindOfDeclareStatement: {
    auto ds = static_pointer_cast<DeclareStatement>(node);
    for (auto& decl : ds->getDeclareMap()) {
      if (decl.first == "strict_types") {
        emitMakeUnitFatal(e, "strict_types declaration must not use "
                          "block mode");
      }
    }

    visit(ds->getBlock());
    return false;
  }

  case Construct::KindOfContinueStatement:
  case Construct::KindOfBreakStatement: {
    auto s = static_pointer_cast<Statement>(node);
    auto bs = static_pointer_cast<BreakStatement>(s);
    uint64_t destLevel = bs->getDepth();

    if (destLevel > m_regions.back()->getMaxBreakContinueDepth()) {
      std::ostringstream msg;
      msg << "Cannot break/continue " << destLevel << " level";
      if (destLevel > 1) {
        msg << "s";
      }
      emitMakeUnitFatal(e, msg.str().c_str());
      return false;
    }

    if (bs->is(Construct::KindOfBreakStatement)) {
      emitBreak(e, destLevel, bs);
    } else {
      emitContinue(e, destLevel, bs);
    }

    return false;
  }

  case Construct::KindOfDoStatement: {
    auto s = static_pointer_cast<Statement>(node);
    auto region = createRegion(s, Region::Kind::LoopOrSwitch);
    auto ds = static_pointer_cast<DoStatement>(s);

    Label top(e);
    Label& condition =
      registerContinue(ds, region.get(), 1, false)->m_label;
    Label& exit =
      registerBreak(ds, region.get(), 1, false)->m_label;
    {
      enterRegion(region);
      SCOPE_EXIT { leaveRegion(region); };
      visit(ds->getBody());
    }
    condition.set(e);
    {
      ExpressionPtr c = ds->getCondExp();
      Emitter condEmitter(c, m_ue, *this);
      visitIfCondition(c, condEmitter, top, exit, false);
    }

    if (exit.isUsed()) exit.set(e);
    return false;
  }

  case Construct::KindOfCaseStatement: {
    // Should never be called. Handled in visitSwitch.
    not_reached();
  }

  case Construct::KindOfCatchStatement: {
    auto cs = static_pointer_cast<CatchStatement>(node);
    auto vName = makeStaticString(cs->getVariable()->getName());
    auto vId = m_curFunc->lookupVarId(vName);

    // Store the exception object on the stack in the appropriate local.
    emitSetL(e, vId);
    emitPop(e);

    // Emit the catch body.
    visit(cs->getStmt());
    return false;
  }

  case Construct::KindOfEchoStatement: {
    auto es = static_pointer_cast<EchoStatement>(node);
    auto exps = es->getExpressionList();
    int count = exps->getCount();
    for (int i = 0; i < count; i++) {
      visit((*exps)[i]);
      emitConvertToCell(e);
      e.Print();
      e.PopC();
    }
    return false;
  }

  case Construct::KindOfExpStatement: {
    auto s = static_pointer_cast<Statement>(node);
    auto es = static_pointer_cast<ExpStatement>(s);
    if (visit(es->getExpression())) {
      // reachability tracking isn't very sophisticated; emitting a pop
      // when we're unreachable will make the emitter think the next
      // position is reachable.
      // In that case, it will spit out Null;RetC at the end of the
      // function, which can cause issues if an asm expression has
      // already output fault funclets.
      if (currentPositionIsReachable()) {
        emitPop(e);
      } else {
        popEvalStack(StackSym::C);
      }
    }
    return false;
  }

  case Construct::KindOfForStatement: {
    auto s = static_pointer_cast<Statement>(node);
    auto region = createRegion(s, Region::Kind::LoopOrSwitch);
    auto fs = static_pointer_cast<ForStatement>(s);

    if (visit(fs->getInitExp())) {
      emitPop(e);
    }
    Label preCond(e);
    Label& preInc = registerContinue(fs, region.get(), 1, false)->m_label;
    Label& fail = registerBreak(fs, region.get(), 1, false)->m_label;
    ExpressionPtr condExp = fs->getCondExp();
    auto emit_cond = [&] (Label& tru, bool truFallthrough) {
      if (!condExp) return;
      Emitter condEmitter(condExp, m_ue, *this);
      visitIfCondition(condExp, condEmitter, tru, fail, truFallthrough);
    };
    Label top;
    emit_cond(top, true);
    top.set(e);
    {
      enterRegion(region);
      SCOPE_EXIT { leaveRegion(region); };
      visit(fs->getBody());
    }
    preInc.set(e);
    if (visit(fs->getIncExp())) {
      emitPop(e);
    }
    if (!condExp) {
      e.Jmp(top);
    } else {
      emit_cond(top, false);
    }
    if (fail.isUsed()) fail.set(e);
    return false;
  }

  case Construct::KindOfForEachStatement: {
    auto fe = static_pointer_cast<ForEachStatement>(node);
    if (fe->isAwaitAs()) {
      emitForeachAwaitAs(e, fe);
    } else {
      emitForeach(e, fe);
    }
    return false;
  }

  case Construct::KindOfGlobalStatement: {
    auto vars = static_pointer_cast<GlobalStatement>(node)->getVars();
    for (int i = 0, n = vars->getCount(); i < n; i++) {
      ExpressionPtr var((*vars)[i]);
      if (var->is(Construct::KindOfSimpleVariable)) {
        auto sv = static_pointer_cast<SimpleVariable>(var);
        if (sv->isSuperGlobal()) {
          continue;
        }
        StringData* nLiteral = makeStaticString(sv->getName());
        Id id = m_curFunc->lookupVarId(nLiteral);
        emitVirtualLocal(id);
        e.String(nLiteral);
        markGlobalName(e);
        e.VGetG();
        emitBind(e);
        e.PopV();
      } else if (var->is(Construct::KindOfDynamicVariable)) {
        // global $<exp> =& $GLOBALS[<exp>]
        auto dv = static_pointer_cast<DynamicVariable>(var);
        // Get the variable name as a cell, for the LHS
        visit(dv->getSubExpression());
        emitConvertToCell(e);
        // Copy the variable name, for indexing into $GLOBALS
        e.Dup();
        markNameSecond(e);
        markGlobalName(e);
        e.VGetG();
        e.BindN();
        e.PopV();
      } else {
        not_implemented();
      }
    }
    return false;
  }

  case Construct::KindOfIfStatement: {
    auto ifp = static_pointer_cast<IfStatement>(node);
    StatementListPtr branches(ifp->getIfBranches());
    int nb = branches->getCount();
    Label done;
    for (int i = 0; i < nb; i++) {
      auto branch = static_pointer_cast<IfBranchStatement>((*branches)[i]);
      Label fals;
      if (branch->getCondition()) {
        Label tru;
        Emitter condEmitter(branch->getCondition(), m_ue, *this);
        visitIfCondition(branch->getCondition(), condEmitter,
                         tru, fals, true);
        if (tru.isUsed()) {
          tru.set(e);
        }
      }
      visit(branch->getStmt());
      if (currentPositionIsReachable() && i + 1 < nb) {
        e.Jmp(done);
      }
      if (fals.isUsed()) {
        fals.set(e);
      }
    }
    if (done.isUsed()) {
      done.set(e);
    }
    return false;
  }

  case Construct::KindOfIfBranchStatement:
    not_reached(); // handled by KindOfIfStatement

  case Construct::KindOfReturnStatement: {
    auto r = static_pointer_cast<ReturnStatement>(node);

    char retSym = StackSym::C;
    if (visit(r->getRetExp())) {
      if (r->getRetExp()->getContext() & Expression::RefValue &&
          // Generators don't support returning by references
          !m_curFunc->isGenerator) {
        emitConvertToVar(e);
        retSym = StackSym::V;
      } else {
        emitConvertToCell(e);
      }
    } else {
      e.Null();
    }
    assert(m_evalStack.size() == 1);
    assert(IMPLIES(m_curFunc->isAsync || m_curFunc->isGenerator,
                   retSym == StackSym::C));
    emitReturn(e, retSym, r);
    return false;
  }

  case Construct::KindOfStaticStatement: {
    auto vars = static_pointer_cast<StaticStatement>(node)->getVars();
    for (int i = 0, n = vars->getCount(); i < n; i++) {
      ExpressionPtr se((*vars)[i]);
      assert(se->is(Construct::KindOfAssignmentExpression));
      auto ae = static_pointer_cast<AssignmentExpression>(se);
      ExpressionPtr var(ae->getVariable());
      ExpressionPtr value(ae->getValue());
      assert(var->is(Construct::KindOfSimpleVariable));
      auto sv = static_pointer_cast<SimpleVariable>(var);
      StringData* name = makeStaticString(sv->getName());
      Id local = m_curFunc->lookupVarId(name);

      if (m_staticEmitted.insert(sv->getName()).second) {
        Func::SVInfo svInfo;
        svInfo.name = name;
        m_curFunc->staticVars.push_back(svInfo);
      }

      if (value->isScalar()) {
        emitVirtualLocal(local);
        visit(value);
        emitConvertToCell(e);
        e.StaticLocInit(local, name);
      } else {
        Label done;
        emitVirtualLocal(local);
        e.StaticLocCheck(local, name);
        e.JmpNZ(done);

        emitVirtualLocal(local);
        visit(value);
        emitConvertToCell(e);
        e.StaticLocDef(local, name);

        done.set(e);
      }
    }
    return false;
  }

  case Construct::KindOfSwitchStatement: {
    auto s = static_pointer_cast<Statement>(node);
    auto region = createRegion(s, Region::Kind::LoopOrSwitch);
    auto sw = static_pointer_cast<SwitchStatement>(node);

    auto cases = sw->getCases();
    if (!cases) {
      visit(sw->getExp());
      emitPop(e);
      return false;
    }
    uint32_t ncase = cases->getCount();
    std::vector<Label> caseLabels(ncase);
    Label& brkTarget = registerBreak(sw, region.get(), 1, false)->m_label;
    Label& contTarget =
      registerContinue(sw, region.get(), 1, false)->m_label;
    // There are two different ways this can go.  If the subject is a simple
    // variable, then we have to evaluate it every time we compare against a
    // case condition.  Otherwise, we evaluate it once and store it in an
    // unnamed local.  This is because (a) switch statements are equivalent
    // to a series of if-elses, and (b) Zend has some weird evaluation order
    // rules.  For example, "$a == ++$a" is true but "$a[0] == ++$a[0]" is
    // false.  In particular, if a case condition modifies the switch
    // subject, things behave differently depending on whether the subject
    // is a simple variable.
    auto subject = sw->getExp();
    bool simpleSubject = subject->is(Construct::KindOfSimpleVariable)
      && !static_pointer_cast<SimpleVariable>(subject)->getAlwaysStash();
    Id tempLocal = -1;
    Offset start = InvalidAbsoluteOffset;

    auto call = dynamic_pointer_cast<SimpleFunctionCall>(subject);

    if (!simpleSubject) {
      // Evaluate the subject once and stash it in a local
      tempLocal = m_curFunc->allocUnnamedLocal();
      emitVirtualLocal(tempLocal);
      visit(subject);
      emitConvertToCell(e);
      emitSet(e);
      emitPop(e);
      start = m_ue.bcPos();
    }

    int defI = -1;
    for (uint32_t i = 0; i < ncase; i++) {
      auto c = static_pointer_cast<CaseStatement>((*cases)[i]);
      ExpressionPtr condition = c->getCondition();
      if (condition) {
        if (simpleSubject) {
          // Evaluate the subject every time.
          visit(subject);
          emitConvertToCellOrLoc(e);
          visit(condition);
          emitConvertToCell(e);
          emitConvertSecondToCell(e);
        } else {
          emitVirtualLocal(tempLocal);
          emitCGet(e);
          visit(condition);
          emitConvertToCell(e);
        }
        e.Eq();
        e.JmpNZ(caseLabels[i]);
      } else if (LIKELY(defI == -1)) {
        // Default clause.
        defI = i;
      } else {
        throw IncludeTimeFatalException(
          c, "Switch statements may only contain one default: clause");
      }
    }
    if (defI != -1) {
      e.Jmp(caseLabels[defI]);
    } else {
      e.Jmp(brkTarget);
    }

    for (uint32_t i = 0; i < ncase; i++) {
      caseLabels[i].set(e);
      auto c = static_pointer_cast<CaseStatement>((*cases)[i]);
      enterRegion(region);
      SCOPE_EXIT { leaveRegion(region); };
      visit(c->getStatement());
    }
    if (brkTarget.isUsed()) brkTarget.set(e);
    if (contTarget.isUsed()) contTarget.set(e);
    if (!simpleSubject) {
      // Null out temp local, to invoke any needed refcounting
      assert(tempLocal >= 0);
      assert(start != InvalidAbsoluteOffset);
      newFaultRegionAndFunclet(start, m_ue.bcPos(),
                               new UnsetUnnamedLocalThunklet(tempLocal));
      emitUnsetL(e, tempLocal);
      m_curFunc->freeUnnamedLocal(tempLocal);
    }
    return false;
  }

  case Construct::KindOfThrowStatement: {
    visitKids(node);
    emitConvertToCell(e);
    e.Throw();
    return false;
  }

  case Construct::KindOfFinallyStatement: {
    auto s = static_pointer_cast<Statement>(node);
    auto region = createRegion(s, Region::Kind::Finally);
    enterRegion(region);
    SCOPE_EXIT { leaveRegion(region); };

    auto fs = static_pointer_cast<FinallyStatement>(node);
    visit(fs->getBody());
    return false;
  }

  case Construct::KindOfTryStatement: {
    auto s = static_pointer_cast<Statement>(node);
    auto region = createRegion(s, Region::Kind::TryFinally);
    if (!m_evalStack.empty()) {
      InvariantViolation(
        "Emitter detected that the evaluation stack is not empty "
        "at the beginning of a try region: %d", m_ue.bcPos());
    }
    if (!m_evalStack.clsRefSlotStackEmpty()) {
      InvariantViolation(
        "Emitter detected that class-ref slots are in use "
        "at the beginning of a try region: %d", m_ue.bcPos());
    }

    auto ts = static_pointer_cast<TryStatement>(node);
    auto f = static_pointer_cast<FinallyStatement>(ts->getFinally());

    Offset start = m_ue.bcPos();
    Offset end;
    Label after;

    {
      if (f) {
        enterRegion(region);
      }
      SCOPE_EXIT {
        if (f) {
          leaveRegion(region);
        }
      };

      visit(ts->getBody());
      end = m_ue.bcPos();

      if (!m_evalStack.empty()) {
        InvariantViolation("Emitter detected that the evaluation stack "
                           "is not empty at the end of a try region: %d",
                           end);
      }
      if (!m_evalStack.clsRefSlotStackEmpty()) {
        InvariantViolation("Emitter detected that class-ref slots are in use "
                           "at the end of a try region: %d",
                           end);
      }
      if (m_evalStack.fdescSize()) {
        InvariantViolation("Emitter detected an open function call at the end "
                           "of a try region: %d",
                           end);
      }

      StatementListPtr catches = ts->getCatches();
      int catch_count = catches->getCount();
      if (catch_count > 0 && start != end) {
        auto const emitCatchBody = [&] {
          std::set<StringData*, string_data_lt> seen;

          for (int i = 0; i < catch_count; i++) {
            auto c = static_pointer_cast<CatchStatement>((*catches)[i]);
            StringData* eName = makeStaticString(c->getOriginalClassName());

            if (!seen.insert(eName).second) {
              // Already seen a catch of this class, skip.
              continue;
            }

            Label nextCatch;
            e.Dup();
            e.InstanceOfD(eName);
            e.JmpZ(nextCatch);
            visit(c);
            // Safe to jump out of the catch block, as eval stack was empty at
            // the end of try region.
            e.Jmp(after);
            nextCatch.set(e);
          }
        };

        emitCatch(e, start, emitCatchBody);
      }
    }

    Offset end_catches = m_ue.bcPos();
    if (after.isUsed()) after.set(e);

    if (f) {
      region->m_finallyLabel.set(e);
      visit(f);
      emitFinallyEpilogue(e, region.get());
      if (start != end_catches) {
        auto func = getFunclet(f);
        if (func == nullptr) {
          auto thunklet =
            new FinallyThunklet(f, m_curFunc->numLiveIterators());
          func = addFunclet(f, thunklet);
        }
        newFaultRegion(start, end_catches, &func->m_entry);
      }
    }

    return false;
  }

  case Construct::KindOfUnsetStatement: {
    auto exps = static_pointer_cast<UnsetStatement>(node)->getExps();
    for (int i = 0, n = exps->getCount(); i < n; i++) {
      emitVisitAndUnset(e, (*exps)[i]);
    }
    return false;
  }

  case Construct::KindOfWhileStatement: {
    auto s = static_pointer_cast<Statement>(node);
    auto region = createRegion(s, Region::Kind::LoopOrSwitch);
    auto ws = static_pointer_cast<WhileStatement>(s);
    ExpressionPtr condExp(ws->getCondExp());
    Label& lcontinue = registerContinue(ws, region.get(), 1,
      false)->m_label;
    Label& fail = registerBreak(ws, region.get(), 1, false)->m_label;
    Label top;
    auto emit_cond = [&] (Label& tru, bool truFallthrough) {
      Emitter condEmitter(condExp, m_ue, *this);
      visitIfCondition(condExp, condEmitter, tru, fail, truFallthrough);
    };
    emit_cond(top, true);
    top.set(e);
    {
      enterRegion(region);
      SCOPE_EXIT { leaveRegion(region); };
      visit(ws->getBody());
    }
    if (lcontinue.isUsed()) lcontinue.set(e);
    emit_cond(top, false);
    if (fail.isUsed()) fail.set(e);
    return false;
  }

  case Construct::KindOfInterfaceStatement:
  case Construct::KindOfClassStatement: {
    emitClass(e, node->getClassScope(), false);
    return false;
  }

  case Construct::KindOfClassVariable:
  case Construct::KindOfClassConstant:
  case Construct::KindOfMethodStatement:
    // handled by emitClass
    not_reached();

  case Construct::KindOfFunctionStatement: {
    auto m = static_pointer_cast<MethodStatement>(node);
    // Only called for fn defs not on the top level
    assert(!node->getClassScope()); // Handled directly by emitClass().
    if (auto msg = node->getFunctionScope()->getFatalMessage()) {
      emitMakeUnitFatal(e, msg->data());
      return false;
    }
    StringData* nName = makeStaticString(m->getOriginalName());
    FuncEmitter* fe = m_ue.newFuncEmitter(nName);
    e.DefFunc(fe->id());
    postponeMeth(m, fe, false);
    return false;
  }

  case Construct::KindOfGotoStatement: {
    auto g = static_pointer_cast<GotoStatement>(node);
    StringData* nName = makeStaticString(g->label());
    emitGoto(e, nName, g);
    return false;
  }

  case Construct::KindOfLabelStatement: {
    auto l = static_pointer_cast<LabelStatement>(node);
    StringData* nName = makeStaticString(l->label());
    registerGoto(l, m_regions.back().get(), nName, false)
      ->m_label.set(e);
    return false;
  }
  case Construct::KindOfStatement:
  case Construct::KindOfUseTraitStatement:
  case Construct::KindOfClassRequireStatement:
  case Construct::KindOfTraitPrecStatement:
  case Construct::KindOfTraitAliasStatement: {
    not_implemented();
  }
  case Construct::KindOfUnaryOpExpression: {
    auto u = static_pointer_cast<UnaryOpExpression>(node);
    int op = u->getOp();

    if (op == T_UNSET) {
      // php doesnt have an unset expression, but hphp's optimizations
      // sometimes introduce them
      auto exp = u->getExpression();
      if (exp->is(Construct::KindOfExpressionList)) {
        auto exps = static_pointer_cast<ExpressionList>(exp);
        if (exps->getListKind() == ExpressionList::ListKindParam) {
          for (int i = 0, n = exps->getCount(); i < n; i++) {
            emitVisitAndUnset(e, (*exps)[i]);
          }
          e.Null();
          return true;
        }
      }
      emitVisitAndUnset(e, exp);
      e.Null();
      return true;
    }

    if (op == T_ARRAY) {
      auto el = static_pointer_cast<ExpressionList>(u->getExpression());
      emitArrayInit(e, el);
      return true;
    }

    if (op == T_DICT) {
      auto el = static_pointer_cast<ExpressionList>(u->getExpression());
      emitArrayInit(e, el, HeaderKind::Dict);
      return true;
    }

    if (op == T_VEC) {
      auto el = static_pointer_cast<ExpressionList>(u->getExpression());
      emitArrayInit(e, el, HeaderKind::VecArray);
      return true;
    }

    if (op == T_KEYSET) {
      auto el = static_pointer_cast<ExpressionList>(u->getExpression());
      emitArrayInit(e, el, HeaderKind::Keyset);
      return true;
    }

    if (op == T_ISSET) {
      auto list = dynamic_pointer_cast<ExpressionList>(u->getExpression());
      if (list) {
        // isset($a, $b, ...)  ==>  isset($a) && isset($b) && ...
        Label done;
        int n = list->getCount();
        for (int i = 0; i < n - 1; ++i) {
          visit((*list)[i]);
          emitIsset(e);
          e.Dup();
          e.JmpZ(done);
          emitPop(e);
        }
        // Treat the last one specially; let it fall through
        visit((*list)[n - 1]);
        emitIsset(e);
        done.set(e);
      } else {
        // Simple case
        visit(u->getExpression());
        emitIsset(e);
      }
      return true;
    } else if (op == '+' || op == '-') {
      e.Int(0);
    }

    Id oldErrorLevelLoc = -1;
    Offset start = InvalidAbsoluteOffset;
    if (op == '@') {
      oldErrorLevelLoc = m_curFunc->allocUnnamedLocal();
      emitVirtualLocal(oldErrorLevelLoc);
      auto idx = m_evalStack.size() - 1;
      e.Silence(m_evalStack.getLoc(idx), SilenceOp::Start);
      start = m_ue.bcPos();
    }

    ExpressionPtr exp = u->getExpression();
    if (exp && visit(exp)) {
      if (op != T_EMPTY && op != T_INC && op != T_DEC) {
        emitConvertToCell(e);
      }
    } else if (op == T_EXIT) {
      // exit without an expression is treated as exit(0)
      e.Int(0);
    } else {
      // __FILE__ and __DIR__ are special unary ops that don't
      // have expressions
      assert(op == T_FILE || op == T_DIR || op == T_METHOD_C);
    }
    switch (op) {
      case T_INC:
      case T_DEC: {
        // $this++ is a no-op
        if (auto var = dynamic_pointer_cast<SimpleVariable>(exp)) {
          if (var->isThis()) break;
        }

        auto const cop = [&] {
          if (op == T_INC) {
            if (RuntimeOption::IntsOverflowToInts) {
              return u->getFront() ? IncDecOp::PreInc : IncDecOp::PostInc;
            }
            return u->getFront() ? IncDecOp::PreIncO : IncDecOp::PostIncO;
          }
          if (RuntimeOption::IntsOverflowToInts) {
            return u->getFront() ? IncDecOp::PreDec : IncDecOp::PostDec;
          }
          return u->getFront() ? IncDecOp::PreDecO : IncDecOp::PostDecO;
        }();
        emitIncDec(e, cop);
        break;
      }
      case T_EMPTY: emitEmpty(e); break;
      case T_CLONE: e.Clone(); break;
      case '+':
        RuntimeOption::IntsOverflowToInts ? e.Add() : e.AddO();
        break;
      case '-':
        RuntimeOption::IntsOverflowToInts ? e.Sub() : e.SubO();
        break;
      case '!': e.Not(); break;
      case '~': e.BitNot(); break;
      case '(': break;
      case T_INT_CAST: e.CastInt(); break;
      case T_DOUBLE_CAST: e.CastDouble(); break;
      case T_STRING_CAST: e.CastString(); break;
      case T_ARRAY_CAST: e.CastArray(); break;
      case T_OBJECT_CAST: e.CastObject(); break;
      case T_BOOL_CAST: e.CastBool(); break;
      case T_UNSET_CAST: emitPop(e); e.Null(); break;
      case T_EXIT: {
        auto save = m_evalStack;
        e.Exit();
        m_evalStackIsUnknown = false;
        m_evalStack = save;
        break;
      }
      case '@': {
        assert(oldErrorLevelLoc >= 0);
        assert(start != InvalidAbsoluteOffset);
        emitRestoreErrorReporting(e, oldErrorLevelLoc);
        newFaultRegionAndFunclet(start, m_ue.bcPos(),
          new RestoreErrorReportingThunklet(oldErrorLevelLoc));
        m_curFunc->freeUnnamedLocal(oldErrorLevelLoc);
        break;
      }
      case T_PRINT: e.Print(); break;
      case T_EVAL: e.Eval(); break;
      case T_FILE: {
        e.File();
        break;
      }
      case T_DIR: {
        e.Dir();
        break;
      }
      case T_METHOD_C: {
        e.Method();
        break;
      }
      default:
        assert(false);
    }
    return true;
  }

  case Construct::KindOfAssignmentExpression: {
    auto ae = static_pointer_cast<AssignmentExpression>(node);
    ExpressionPtr rhs = ae->getValue();
    Id tempLocal = -1;
    Offset start = InvalidAbsoluteOffset;

    if (ae->isRhsFirst()) {
      assert(!rhs->hasContext(Expression::RefValue));
      tempLocal = emitVisitAndSetUnnamedL(e, rhs);
      start = m_ue.bcPos();
    }

    visit(ae->getVariable());
    emitClsIfSPropBase(e);

    if (ae->isRhsFirst()) {
      emitPushAndFreeUnnamedL(e, tempLocal, start);
    } else {
      visit(rhs);
    }

    if (rhs->hasContext(Expression::RefValue)) {
      emitConvertToVar(e);
      emitBind(e);
      if (ae->hasAnyContext(Expression::AccessContext|
                            Expression::ObjectContext|
                            Expression::ExistContext)) {
        /*
         * hphpc optimizations can result in
         * ($x =& $y)->foo or ($x =& $y)['foo'] or empty($x =& $y)
         */
        emitConvertToCellIfVar(e);
      }
    } else {
      emitConvertToCell(e);
      emitSet(e);
    }
    return true;
  }

  case Construct::KindOfBinaryOpExpression: {
    auto b = static_pointer_cast<BinaryOpExpression>(node);
    int op = b->getOp();
    if (b->isAssignmentOp()) {
      visit(b->getExp1());
      emitClsIfSPropBase(e);
      visit(b->getExp2());
      emitConvertToCell(e);
      emitSetOp(e, op);
      return true;
    }

    if (b->isShortCircuitOperator()) {
      Label tru, fls, done;
      visitIfCondition(b, e, tru, fls, false);
      if (fls.isUsed()) fls.set(e);
      if (currentPositionIsReachable()) {
        e.False();
        e.Jmp(done);
      }
      if (tru.isUsed()) tru.set(e);
      if (currentPositionIsReachable()) {
        e.True();
      }
      done.set(e);
      return true;
    }

    if (op == T_INSTANCEOF) {
      visit(b->getExp1());
      emitConvertToCell(e);
      ExpressionPtr second = b->getExp2();
      if (second->isScalar()) {
        auto scalar = dynamic_pointer_cast<ScalarExpression>(second);
        bool notQuoted = scalar && !scalar->isQuoted();
        std::string s = second->getLiteralString();

        const auto isame =
          [](const std::string& a, const std::string& b) {
            return (a.size() == b.size()) &&
                   !strncasecmp(a.c_str(), b.c_str(), a.size());
          };

        if (notQuoted && isame(s, "static")) {
          // Can't resolve this to a literal name at emission time
          static const StringData* fname
            = makeStaticString("get_called_class");
          e.FCallBuiltin(0, 0, fname);
          e.UnboxRNop();
          e.InstanceOf();
        } else if (s != "") {
          ClassScopeRawPtr cls = second->getClassScope();
          bool isTrait = cls && cls->isTrait();
          bool isSelf = notQuoted && isame(s, "self");
          bool isParent = notQuoted && isame(s, "parent");

          if (isTrait && (isSelf || isParent)) {
            emitConvertToCell(e);

            if (isSelf) {
              e.Self(kClsRefSlotPlaceholder);
            } else if (isParent) {
              e.Parent(kClsRefSlotPlaceholder);
            }
            e.ClsRefName(kClsRefSlotPlaceholder);
            e.InstanceOf();
          } else {
            if (cls) {
              if (isSelf) {
                s = cls->getScopeName();
              } else if (isParent) {
                s = cls->getOriginalParent();
              }
            }

            StringData* nLiteral = makeStaticString(s);
            e.InstanceOfD(nLiteral);
          }
        } else {
          visit(b->getExp2());
          emitConvertToCell(e);
          e.InstanceOf();
        }
      } else {
        visit(b->getExp2());
        emitConvertToCell(e);
        e.InstanceOf();
      }
      return true;
    }

    if (op == T_COLLECTION) {
      emitCollectionInit(e, b);
      return true;
    }

    if (op == T_PIPE) {
      Id pipeVar = emitVisitAndSetUnnamedL(e, b->getExp1());
      allocPipeLocal(pipeVar);
      auto const start = m_ue.bcPos();
      visit(b->getExp2());
      emitConvertToCell(e);
      releasePipeLocal(pipeVar);
      emitFreeUnnamedL(e, pipeVar, start);
      return true;
    }

    visit(b->getExp1());
    emitConvertToCellOrLoc(e);
    visit(b->getExp2());
    emitConvertToCell(e);
    emitConvertSecondToCell(e);
    switch (op) {
      case T_LOGICAL_XOR: e.Xor(); break;
      case '|': e.BitOr(); break;
      case '&': e.BitAnd(); break;
      case '^': e.BitXor(); break;
      case '.': e.Concat(); break;
      case '+':
        RuntimeOption::IntsOverflowToInts ? e.Add() : e.AddO();
        break;
      case '-':
        RuntimeOption::IntsOverflowToInts ? e.Sub() : e.SubO();
        break;
      case '*':
        RuntimeOption::IntsOverflowToInts ? e.Mul() : e.MulO();
        break;
      case '/': e.Div(); break;
      case '%': e.Mod(); break;
      case T_SL: e.Shl(); break;
      case T_SR: e.Shr(); break;
      case T_IS_IDENTICAL: e.Same(); break;
      case T_IS_NOT_IDENTICAL: e.NSame(); break;
      case T_IS_EQUAL: e.Eq(); break;
      case T_IS_NOT_EQUAL: e.Neq(); break;
      case '<': e.Lt(); break;
      case T_IS_SMALLER_OR_EQUAL: e.Lte(); break;
      case '>': e.Gt(); break;
      case T_IS_GREATER_OR_EQUAL: e.Gte(); break;
      case T_SPACESHIP: e.Cmp(); break;
      case T_POW: e.Pow(); break;
      default: assert(false);
    }
    return true;
  }

  case Construct::KindOfClassConstantExpression: {
    auto cc = static_pointer_cast<ClassConstantExpression>(node);
    auto const nName = makeStaticString(cc->getConName());
    auto const getOriginalClassName = [&] {
      const std::string& clsName = cc->getOriginalClassName();
      return makeStaticString(clsName);
    };

    // We treat ::class as a class constant in the AST and the
    // parser, but at the bytecode and runtime level it isn't
    // one.
    auto const emitClsCns = [&] {
      if (cc->isColonColonClass()) {
        e.ClsRefName(kClsRefSlotPlaceholder);
        return;
      }
      e.ClsCns(nName, kClsRefSlotPlaceholder);
    };
    auto const noClassAllowed = [&] {
      auto const nCls = getOriginalClassName();
      std::ostringstream s;
      s << "Cannot access " << nCls->data() << "::" << nName->data() <<
           " when no class scope is active";
      throw IncludeTimeFatalException(cc, s.str().c_str());
    };

    if (cc->isStatic()) {
      // static::Constant
      e.LateBoundCls(kClsRefSlotPlaceholder);
      emitClsCns();
    } else if (cc->getClass()) {
      // $x::Constant
      ExpressionPtr cls(cc->getClass());
      visit(cls);
      emitAGet(e);
      emitClsCns();
    } else if (cc->getOriginalClassScope() &&
               !cc->getOriginalClassScope()->isTrait()) {
      // C::Constant inside a class
      auto nCls = getOriginalClassName();
      if (cc->isColonColonClass()) {
        e.String(nCls);
      } else {
        e.ClsCnsD(nName, nCls);
      }
    } else if (cc->isSelf()) {
      // self::Constant inside trait or pseudomain
      e.Self(kClsRefSlotPlaceholder);
      if (cc->isColonColonClass() &&
          cc->getFunctionScope()->inPseudoMain()) {
        noClassAllowed();
      }
      emitClsCns();
    } else if (cc->isParent()) {
      // parent::Constant inside trait or pseudomain
      e.Parent(kClsRefSlotPlaceholder);
      if (cc->isColonColonClass() &&
          cc->getFunctionScope()->inPseudoMain()) {
        noClassAllowed();
      }
      emitClsCns();
    } else {
      // C::Constant inside a trait or pseudomain
      // Be careful to keep this case here after the isSelf and
      // isParent cases because StaticClassName::resolveClass()
      // will set cc->originalClassName to the trait's name for
      // the isSelf and isParent cases, but self and parent must
      // be resolved dynamically when used inside of traits.
      auto nCls = getOriginalClassName();
      if (cc->isColonColonClass()) noClassAllowed();
      e.ClsCnsD(nName, nCls);
    }
    return true;
  }

  case Construct::KindOfConstantExpression: {
    auto c = static_pointer_cast<ConstantExpression>(node);
    if (c->isNull()) {
      e.Null();
    } else if (c->isBoolean()) {
      if (c->getBooleanValue()) {
        e.True();
      } else {
        e.False();
      }
      return true;
    } else {
      std::string nameStr = c->getOriginalName();
      StringData* nName = makeStaticString(nameStr);
      if (c->hadBackslash()) {
        e.CnsE(nName);
      } else {
        const std::string& nonNSName = c->getNonNSOriginalName();
        if (nonNSName != nameStr) {
          StringData* nsName = nName;
          nName = makeStaticString(nonNSName);
          e.CnsU(nsName, nName);
        } else {
          e.Cns(makeStaticString(c->getName()));
        }
      }
    }
    return true;
  }

  case Construct::KindOfEncapsListExpression: {
    auto el = static_pointer_cast<EncapsListExpression>(node);
    auto args = el->getExpressions();
    int n = args ? args->getCount() : 0;
    int i = 0;
    FPIRegionRecorder* fpi = nullptr;
    if (el->getType() == '`') {
      const static StringData* s_shell_exec =
        makeStaticString("shell_exec");
      Offset fpiStart = m_ue.bcPos();
      e.FPushFuncD(1, s_shell_exec);
      fpi = new FPIRegionRecorder(this, m_ue, m_evalStack, fpiStart);
    }

    if (n) {
      visit((*args)[i++]);
      emitConvertToCellOrLoc(e);
      if (i == n) {
        emitConvertToCell(e);
        e.CastString();
      } else {
        while (i < n) {
          visit((*args)[i++]);
          emitConvertToCell(e);
          emitConvertSecondToCell(e);
          e.Concat();
        }
      }
    } else {
      e.String(staticEmptyString());
    }

    if (el->getType() == '`') {
      emitConvertToCell(e);
      e.FPassC(0);
      delete fpi;
      e.FCall(1);
    }
    return true;
  }

  case Construct::KindOfArrayElementExpression: {
    auto ae = static_pointer_cast<ArrayElementExpression>(node);
    if (!ae->isSuperGlobal() || !ae->getOffset()) {
      visit(ae->getVariable());
      // XHP syntax allows for expressions like "($a =& $b)[0]". We
      // handle this by unboxing the var produced by "($a =& $b)".
      emitConvertToCellIfVar(e);
    }

    ExpressionPtr offset = ae->getOffset();
    Variant v;
    if (!ae->isSuperGlobal() && offset &&
        offset->getScalarValue(v) && (v.isInteger() || v.isString())) {
      if (v.isString()) {
        m_evalStack.push(StackSym::T);
        m_evalStack.setString(
          makeStaticString(v.toCStrRef().get()));
      } else {
        m_evalStack.push(StackSym::I);
        m_evalStack.setInt(v.asInt64Val());
      }
      markElem(e);
    } else if (visit(offset)) {
      emitConvertToCellOrLoc(e);
      if (ae->isSuperGlobal()) {
        markGlobalName(e);
      } else {
        markElem(e);
      }
    } else {
      markNewElem(e);
    }
    if (!ae->hasAnyContext(Expression::AccessContext|
                           Expression::ObjectContext)) {
      m_tempLoc = ae->getRange();
    }
    return true;
  }

  case Construct::KindOfSimpleFunctionCall: {
    auto call = static_pointer_cast<SimpleFunctionCall>(node);
    auto params = call->getParams();

    if (call->isFatalFunction()) {
      if (params && params->getCount() == 1) {
        ExpressionPtr p = (*params)[0];
        Variant v;
        if (p->getScalarValue(v)) {
          assert(v.isString());
          StringData* msg = makeStaticString(v.toString());
          auto exn = IncludeTimeFatalException(call, "%s", msg->data());
          exn.setParseFatal(call->isParseFatalFunction());
          throw exn;
        }
        not_reached();
      }
    } else if (emitCallUserFunc(e, call)) {
      return true;
    } else if (call->isCallToFunction("array_key_exists")) {
      if (params && params->getCount() == 2) {
        visit((*params)[0]);
        emitConvertToCell(e);
        visit((*params)[1]);
        emitConvertToCell(e);
        call->changeToBytecode();
        e.AKExists();
        return true;
      }
    } else if (call->isCallToFunction("hh\\asm")) {
      if (emitInlineHHAS(e, call)) return true;
    } else if (call->isCallToFunction("hh\\invariant")) {
      if (emitHHInvariant(e, call)) return true;
    } else if (call->isCallToFunction("hh\\idx") &&
               !RuntimeOption::EvalJitEnableRenameFunction) {
      if (params && (params->getCount() == 2 || params->getCount() == 3)) {
        visit((*params)[0]);
        emitConvertToCell(e);
        visit((*params)[1]);
        emitConvertToCell(e);
        if (params->getCount() == 2) {
          e.Null();
        } else {
          visit((*params)[2]);
          emitConvertToCell(e);
        }
        call->changeToBytecode();
        e.Idx();
        return true;
      }
    } else if (call->isCallToFunction("hphp_array_idx")) {
      if (params && params->getCount() == 3) {
        visit((*params)[0]);
        emitConvertToCell(e);
        visit((*params)[1]);
        emitConvertToCell(e);
        visit((*params)[2]);
        emitConvertToCell(e);
        call->changeToBytecode();
        e.ArrayIdx();
        return true;
      }
    } else if (call->isCallToFunction("max")) {
      if (params && params->getCount() == 2) {
        emitFuncCall(e, call, "__SystemLib\\max2", params);
        return true;
      }
    } else if (call->isCallToFunction("min")) {
      if (params && params->getCount() == 2) {
        emitFuncCall(e, call, "__SystemLib\\min2", params);
        return true;
      }
    } else if (call->isCallToFunction("define")) {
      if (params && params->getCount() == 2) {
        ExpressionPtr p0 = (*params)[0];
        Variant v0;
        if (p0->getScalarValue(v0) && v0.isString()) {
          const StringData* cname =
            makeStaticString(v0.toString());
          visit((*params)[1]);
          emitConvertToCell(e);
          e.DefCns(cname);
          return true;
        }
      }
    } else if (call->isCallToFunction("class_alias")) {
      if (params &&
          (params->getCount() == 2 ||
           params->getCount() == 3)) {
        ExpressionPtr p0 = (*params)[0];
        ExpressionPtr p1 = (*params)[1];
        Variant v0, v1;
        if (p0->getScalarValue(v0) && v0.isString() &&
            p1->getScalarValue(v1) && v1.isString()) {
          const StringData* orig_name =
            makeStaticString(v0.toString());
          const StringData* alias_name =
            makeStaticString(v1.toString());
          if (params->getCount() == 3) {
            visit((*params)[1]);
            emitConvertToCell(e);
          } else {
            e.True();
          }
          e.AliasCls(orig_name, alias_name);
          return true;
        }
      }
    } else if (call->isCallToFunction("assert")) {
      // Special-case some logic around emitting assert(), or jumping around
      // it. This all applies only for direct calls to assert() -- dynamic
      // calls don't get this special logic, and don't in PHP7 either.

      if (!RuntimeOption::AssertEmitted) {
        e.True();
        return true;
      }

      // We need to emit an ini_get around all asserts to check if the
      // zend.assertions option is enabled -- you can switch between 0 and 1
      // at runtime, and having it set to 0 disables the assert from running,
      // including side effects of function arguments, so we need to jump
      // around it if so. (The -1 value of zend.assertions corresponds to
      // AssertEmitted being set to 0 above, and is not changeable at
      // runtime.)
      Label disabled, after;
      e.String(s_zend_assertions.get());
      e.FCallBuiltin(1, 1, s_ini_get.get());
      e.UnboxRNop();
      e.Int(0);
      e.Gt();
      e.JmpZ(disabled);

      emitFuncCall(e, call, "assert", call->getParams());
      emitConvertToCell(e);
      e.Jmp(after);

      disabled.set(e);
      e.True();

      after.set(e);
      return true;
    } else if (call->isCallToFunction("array_slice") &&
               params && params->getCount() == 2 &&
               !RuntimeOption::EvalJitEnableRenameFunction) {
      ExpressionPtr p0 = (*params)[0];
      ExpressionPtr p1 = (*params)[1];
      Variant v1;
      if (p0->getKindOf() == Construct::KindOfSimpleFunctionCall &&
          p1->getScalarValue(v1) && v1.isInteger()) {
        auto innerCall = static_pointer_cast<SimpleFunctionCall>(p0);
        auto innerParams = innerCall->getParams();
        if (innerCall->isCallToFunction("func_get_args") &&
            (!innerParams || innerParams->getCount() == 0)) {
          params->removeElement(0);
          emitFuncCall(e, innerCall,
                       "__SystemLib\\func_slice_args", params);
          return true;
        }
      }
      // fall through
    } else if ((call->isCallToFunction("class_exists") ||
                call->isCallToFunction("interface_exists") ||
                call->isCallToFunction("trait_exists"))
               && params
               && (params->getCount() == 1 || params->getCount() == 2)) {
      // Push name
      emitNameString(e, (*params)[0]);
      emitConvertToCell(e);
      e.CastString();

      // Push autoload, defaulting to true
      if (params->getCount() == 1) {
        e.True();
      } else {
        visit((*params)[1]);
        emitConvertToCell(e);
        e.CastBool();
      }
      if (call->isCallToFunction("class_exists")) {
        e.OODeclExists(OODeclExistsOp::Class);
      } else if (call->isCallToFunction("interface_exists")) {
        e.OODeclExists(OODeclExistsOp::Interface);
      } else {
        assert(call->isCallToFunction("trait_exists"));
        e.OODeclExists(OODeclExistsOp::Trait);
      }
      return true;
    } else if (call->isCallToFunction("get_class") &&
               !params &&
               call->getClassScope() &&
               !call->getClassScope()->isTrait()) {
      StringData* name =
        makeStaticString(call->getClassScope()->getScopeName());
      e.String(name);
      return true;
    } else if (((call->isCallToFunction("dict") &&
                 (m_ue.m_isHHFile || RuntimeOption::EnableHipHopSyntax)) ||
                call->isCallToFunction("HH\\dict")) &&
               params && params->getCount() == 1) {
      visit((*params)[0]);
      emitConvertToCell(e);
      e.CastDict();
      return true;
    } else if (((call->isCallToFunction("vec") &&
                 (m_ue.m_isHHFile || RuntimeOption::EnableHipHopSyntax)) ||
                call->isCallToFunction("HH\\vec")) &&
               params && params->getCount() == 1) {
      visit((*params)[0]);
      emitConvertToCell(e);
      e.CastVec();
      return true;
    } else if (((call->isCallToFunction("keyset") &&
                 (m_ue.m_isHHFile || RuntimeOption::EnableHipHopSyntax)) ||
                call->isCallToFunction("HH\\keyset")) &&
               params && params->getCount() == 1) {
      visit((*params)[0]);
      emitConvertToCell(e);
      e.CastKeyset();
      return true;
    } else if (((call->isCallToFunction("varray") &&
                 (m_ue.m_isHHFile || RuntimeOption::EnableHipHopSyntax)) ||
                call->isCallToFunction("HH\\varray")) &&
               params && params->getCount() == 1) {
      visit((*params)[0]);
      emitConvertToCell(e);
      e.CastVArray();
      return true;
    } else if (((call->isCallToFunction("darray") &&
                 (m_ue.m_isHHFile || RuntimeOption::EnableHipHopSyntax)) ||
                call->isCallToFunction("HH\\darray")) &&
               params && params->getCount() == 1) {
      visit((*params)[0]);
      emitConvertToCell(e);
      e.CastDArray();
      return true;
    } else if (((call->isCallToFunction("is_vec") &&
                 (m_ue.m_isHHFile || RuntimeOption::EnableHipHopSyntax)) ||
                call->isCallToFunction("HH\\is_vec")) &&
               params && params->getCount() == 1) {
      visit((*call->getParams())[0]);
      emitIsType(e, IsTypeOp::Vec);
      return true;
    } else if (((call->isCallToFunction("is_dict") &&
                 (m_ue.m_isHHFile || RuntimeOption::EnableHipHopSyntax)) ||
                call->isCallToFunction("HH\\is_dict")) &&
               params && params->getCount() == 1) {
      visit((*call->getParams())[0]);
      emitIsType(e, IsTypeOp::Dict);
      return true;
    } else if (((call->isCallToFunction("is_keyset") &&
                 (m_ue.m_isHHFile || RuntimeOption::EnableHipHopSyntax)) ||
                call->isCallToFunction("HH\\is_keyset")) &&
               params && params->getCount() == 1) {
      visit((*call->getParams())[0]);
      emitIsType(e, IsTypeOp::Keyset);
      return true;
    } else if (((call->isCallToFunction("is_varray_or_darray") &&
                 (m_ue.m_isHHFile || RuntimeOption::EnableHipHopSyntax)) ||
                call->isCallToFunction("HH\\is_varray_or_darray")) &&
               params && params->getCount() == 1) {
      visit((*call->getParams())[0]);
      emitIsType(e, IsTypeOp::Arr);
      return true;
    }
  #define TYPE_CONVERT_INSTR(what, What)                             \
    else if (call->isCallToFunction(#what"val") &&                 \
             params && params->getCount() == 1) {                  \
      visit((*params)[0]);                                         \
      emitConvertToCell(e);                                        \
      e.Cast ## What();                                            \
      return true;                                                 \
    }
  TYPE_CONVERT_INSTR(bool, Bool)
  TYPE_CONVERT_INSTR(int, Int)
  TYPE_CONVERT_INSTR(double, Double)
  TYPE_CONVERT_INSTR(float, Double)
  TYPE_CONVERT_INSTR(str, String)
#undef TYPE_CONVERT_INSTR

#define TYPE_CHECK_INSTR(what, What)                \
    else if (call->isCallToFunction("is_"#what) &&  \
             params && params->getCount() == 1) {   \
      visit((*call->getParams())[0]);               \
      emitIsType(e, IsTypeOp::What);                \
      return true;                                  \
    }

  TYPE_CHECK_INSTR(null, Null)
  TYPE_CHECK_INSTR(object, Obj)
  TYPE_CHECK_INSTR(array, Arr)
  TYPE_CHECK_INSTR(string, Str)
  TYPE_CHECK_INSTR(int, Int)
  TYPE_CHECK_INSTR(integer, Int)
  TYPE_CHECK_INSTR(long, Int)
  TYPE_CHECK_INSTR(bool, Bool)
  TYPE_CHECK_INSTR(double, Dbl)
  TYPE_CHECK_INSTR(real, Dbl)
  TYPE_CHECK_INSTR(float, Dbl)
  TYPE_CHECK_INSTR(scalar, Scalar)
#undef TYPE_CHECK_INSTR
    else if (emitConstantFuncCall(e, call)) {
      return true;
    }
  }
  // fall through
  case Construct::KindOfDynamicFunctionCall: {
    emitFuncCall(e, static_pointer_cast<FunctionCall>(node));
    return true;
  }

  case Construct::KindOfIncludeExpression: {
    auto ie = static_pointer_cast<IncludeExpression>(node);
    if (ie->isReqLit()) {
      StringData* nValue = makeStaticString(ie->includePath());
      e.String(nValue);
    } else {
      visit(ie->getExpression());
      emitConvertToCell(e);
    }
    switch (ie->getOp()) {
      case T_INCLUDE:
        e.Incl();
        break;
      case T_INCLUDE_ONCE:
        e.InclOnce();
        break;
      case T_REQUIRE:
        e.Req();
        break;
      case T_REQUIRE_ONCE:
        if (ie->isDocumentRoot()) {
          e.ReqDoc();
        } else {
          e.ReqOnce();
        }
        break;
    }
    return true;
  }

  case Construct::KindOfListAssignment: {
    auto la = static_pointer_cast<ListAssignment>(node);
    auto rhs = la->getArray();

    // listAssignmentVisitLHS should have handled this
    assert(rhs);

    bool nullRHS = la->getRHSKind() == ListAssignment::Null;
    // If the RHS is not a simple variable, we need to evaluate it and assign
    // it to a temp local. If it is, whether or not we directly use it or copy
    // it into a temp local is visible in perverse statements like:
    // list($a, $b) = $a
    // The behavior of that changed between PHP5 and PHP7; in PHP5 we directly
    // use the temp local, in PHP7 we need to copy it.
    bool simpleRHS = rhs->is(Construct::KindOfSimpleVariable)
      && !static_pointer_cast<SimpleVariable>(rhs)->getAlwaysStash()
      && !RuntimeOption::PHP7_LTR_assign;
    Id tempLocal = -1;
    Offset start = InvalidAbsoluteOffset;

    if (!simpleRHS && la->isRhsFirst()) {
      tempLocal = emitVisitAndSetUnnamedL(e, rhs);
      start = m_ue.bcPos();
    }

    // We use "index chains" to deal with nested list assignment.  We will
    // end up with one index chain per expression we need to assign to.
    // The helper function will populate indexChains.
    //
    // In PHP5 mode, this will also evaluate the LHS; in PHP7 mode, that is
    // always delayed until listAssignmentAssignElements below. This means
    // that isRhsFirst() has no effect in PHP7 mode. See comments in
    // listAssignmentVisitLHS and listAssignmentAssignElements for more
    // explanation.
    std::vector<IndexPair> indexPairs;
    IndexChain workingChain;
    listAssignmentVisitLHS(e, la, workingChain, indexPairs);

    if (!simpleRHS && !la->isRhsFirst()) {
      assert(tempLocal == -1);
      assert(start == InvalidAbsoluteOffset);
      tempLocal = emitVisitAndSetUnnamedL(e, rhs);
      start = m_ue.bcPos();
    }

    // Assign elements.
    if (nullRHS) {
      listAssignmentAssignElements(e, indexPairs, nullptr);
    } else if (simpleRHS) {
      listAssignmentAssignElements(e, indexPairs, [&] { visit(rhs); });
    } else {
      listAssignmentAssignElements(
        e, indexPairs,
        [&] { emitVirtualLocal(tempLocal); }
      );
    }

    // Leave the RHS on the stack
    if (simpleRHS) {
      visit(rhs);
    } else {
      emitPushAndFreeUnnamedL(e, tempLocal, start);
    }

    return true;
  }

  case Construct::KindOfNewObjectExpression: {
    auto ne = static_pointer_cast<NewObjectExpression>(node);
    auto params = ne->getParams();
    int numParams = params ? params->getCount() : 0;
    ClassScopeRawPtr cls = ne->getClassScope();

    Offset fpiStart;
    if (ne->isStatic()) {
      // new static()
      e.LateBoundCls(kClsRefSlotPlaceholder);
      fpiStart = m_ue.bcPos();
      e.FPushCtor(numParams, kClsRefSlotPlaceholder);
    } else if (ne->getOriginalName().empty()) {
      // new $x()
      visit(ne->getNameExp());
      emitAGet(e);
      fpiStart = m_ue.bcPos();
      e.FPushCtor(numParams, kClsRefSlotPlaceholder);
    } else if ((ne->isSelf() || ne->isParent()) &&
               (!cls || cls->isTrait() ||
                (ne->isParent() && cls->getOriginalParent().empty()))) {
      if (ne->isSelf()) {
        // new self() inside a trait or code statically not inside any class
        e.Self(kClsRefSlotPlaceholder);
      } else {
        // new parent() inside a trait, code statically not inside any
        // class, or a class with no parent
        e.Parent(kClsRefSlotPlaceholder);
      }
      fpiStart = m_ue.bcPos();
      e.FPushCtor(numParams, kClsRefSlotPlaceholder);
    } else {
      // new C() inside trait or pseudomain
      fpiStart = m_ue.bcPos();
      e.FPushCtorD(numParams,
                   makeStaticString(ne->getOriginalClassName()));
    }

    emitCall(e, ne, params, fpiStart);
    e.PopR();
    return true;
  }

  case Construct::KindOfObjectMethodExpression: {
    auto om = static_pointer_cast<ObjectMethodExpression>(node);
    // $obj->name(...)
    // ^^^^
    visit(om->getObject());
    m_tempLoc = om->getRange();
    emitConvertToCell(e);
    ExpressionListPtr params(om->getParams());
    int numParams = params ? params->getCount() : 0;

    Offset fpiStart = 0;
    ExpressionPtr methName = om->getNameExp();
    bool useDirectForm = false;
    if (methName->is(Construct::KindOfScalarExpression)) {
      auto sval = static_pointer_cast<ScalarExpression>(methName);
      const std::string& methStr = sval->getOriginalLiteralString();
      if (!methStr.empty()) {
        // $obj->name(...)
        //       ^^^^
        // Use getOriginalLiteralString(), which hasn't been
        // case-normalized, since __call() needs to preserve
        // the case.
        StringData* nameLiteral = makeStaticString(methStr);
        fpiStart = m_ue.bcPos();
        e.FPushObjMethodD(
          numParams,
          nameLiteral,
          om->isNullSafe() ? ObjMethodOp::NullSafe : ObjMethodOp::NullThrows
        );
        useDirectForm = true;
      }
    }
    if (!useDirectForm) {
      // $obj->{...}(...)
      //       ^^^^^
      visit(methName);
      emitConvertToCell(e);
      fpiStart = m_ue.bcPos();
      e.FPushObjMethod(
        numParams,
        om->isNullSafe() ? ObjMethodOp::NullSafe : ObjMethodOp::NullThrows
      );
    }
    // $obj->name(...)
    //           ^^^^^
    emitCall(e, om, params, fpiStart);
    return true;
  }

  case Construct::KindOfObjectPropertyExpression: {
    auto op = static_pointer_cast<ObjectPropertyExpression>(node);
    if (op->isNullSafe() &&
        op->hasAnyContext(
            Expression::RefValue
          | Expression::LValue
          | Expression::DeepReference
        ) && !op->hasContext(Expression::InvokeArgument)
    ) {
      throw IncludeTimeFatalException(op,
        Strings::NULLSAFE_PROP_WRITE_ERROR);
    }
    ExpressionPtr obj = op->getObject();
    auto sv = dynamic_pointer_cast<SimpleVariable>(obj);
    if (sv && sv->isThis() && sv->hasContext(Expression::ObjectContext)) {
      e.CheckThis();
      m_evalStack.push(StackSym::H);
    } else {
      visit(obj);
    }
    StringData* clsName = getClassName(op->getObject());
    if (clsName) {
      m_evalStack.setKnownCls(clsName, false);
    }
    emitNameString(e, op->getProperty(), true);
    if (!op->hasAnyContext(Expression::AccessContext|
                           Expression::ObjectContext)) {
      m_tempLoc = op->getRange();
    }
    markProp(
      e,
      op->isNullSafe()
        ? PropAccessType::NullSafe
        : PropAccessType::Normal
    );
    return true;
  }

  case Construct::KindOfQOpExpression: {
    auto q = static_pointer_cast<QOpExpression>(node);
    if (q->getYes()) {
      // <expr> ? <expr> : <expr>
      Label tru, fals, done;
      {
        Emitter condEmitter(q->getCondition(), m_ue, *this);
        visitIfCondition(q->getCondition(), condEmitter,
                         tru, fals, true);
      }
      if (tru.isUsed()) {
        tru.set(e);
      }
      if (currentPositionIsReachable()) {
        visit(q->getYes());
        emitConvertToCell(e);
        e.Jmp(done);
      }
      if (fals.isUsed()) fals.set(e);
      if (currentPositionIsReachable()) {
        visit(q->getNo());
        emitConvertToCell(e);
      }
      if (done.isUsed()) {
        done.set(e);
        m_evalStack.cleanTopMeta();
      }
    } else {
      // <expr> ?: <expr>
      Label done;
      visit(q->getCondition());
      emitConvertToCell(e);
      e.Dup();
      e.JmpNZ(done);
      e.PopC();
      visit(q->getNo());
      emitConvertToCell(e);
      done.set(e);
      m_evalStack.cleanTopMeta();
    }
    return true;
  }

  case Construct::KindOfNullCoalesceExpression: {
    auto q = static_pointer_cast<NullCoalesceExpression>(node);

    Label done;
    visit(q->getFirst());
    emitCGetQuiet(e);
    e.Dup();
    emitIsset(e);
    e.JmpNZ(done);
    e.PopC();
    visit(q->getSecond());
    emitConvertToCell(e);
    done.set(e);
    m_evalStack.cleanTopMeta();

    return true;
  }

  case Construct::KindOfScalarExpression: {
    auto ex = static_pointer_cast<Expression>(node);
    Variant v;
    ex->getScalarValue(v);
    auto const emitted = emitScalarValue(e, v);
    always_assert(emitted);
    return true;
  }

  case Construct::KindOfPipeVariable: {
    if (auto pipeVar = getPipeLocal()) {
      emitVirtualLocal(*pipeVar);
      return true;
    }

    throw IncludeTimeFatalException(
      node, "Pipe variables must occur only in the RHS of pipe expressions");
  }

  case Construct::KindOfSimpleVariable: {
    auto sv = static_pointer_cast<SimpleVariable>(node);
    if (sv->isThis()) {
      if (sv->hasContext(Expression::ObjectContext)) {
        e.This();
      } else if (sv->getFunctionScope()->needsLocalThis()) {
        static const StringData* thisStr = makeStaticString("this");
        Id thisId = m_curFunc->lookupVarId(thisStr);
        emitVirtualLocal(thisId);
      } else {
        auto const subop = sv->hasContext(Expression::ExistContext)
          ? BareThisOp::NoNotice
          : BareThisOp::Notice;
        e.BareThis(subop);
      }
    } else {
      StringData* nLiteral = makeStaticString(sv->getName());
      if (sv->isSuperGlobal()) {
        e.String(nLiteral);
        markGlobalName(e);
        return true;
      }
      Id i = m_curFunc->lookupVarId(nLiteral);
      emitVirtualLocal(i);
      if (sv->getAlwaysStash() &&
          !sv->hasAnyContext(Expression::ExistContext |
                             Expression::RefValue |
                             Expression::LValue |
                             Expression::RefParameter)) {
        emitConvertToCell(e);
      }
    }

    return true;
  }

  case Construct::KindOfDynamicVariable: {
    auto dv = static_pointer_cast<DynamicVariable>(node);
    visit(dv->getSubExpression());
    emitConvertToCellOrLoc(e);
    markName(e);
    return true;
  }

  case Construct::KindOfStaticMemberExpression: {
    auto sm = static_pointer_cast<StaticMemberExpression>(node);
    emitVirtualClassBase(e, sm.get());
    emitNameString(e, sm->getExp());
    markSProp(e);
    return true;
  }

  case Construct::KindOfArrayPairExpression: {
    auto ap = static_pointer_cast<ArrayPairExpression>(node);

    auto key = ap->getName();
    if (!m_staticArrays.empty()) {
      auto val = ap->getValue();

      TypedValue tvVal;
      initScalar(tvVal, val);

      if (key != nullptr) {
        assert(key->isScalar());
        TypedValue tvKey = make_tv<KindOfNull>();
        if (!key->getScalarValue(tvAsVariant(&tvKey))) {
          InvariantViolation("Expected scalar value for array key\n");
          always_assert(0);
        }
        m_staticArrays.back().set(tvAsCVarRef(&tvKey),
                                  tvAsVariant(&tvVal));
      } else {
        // If we're building a static dict for the contents of a Set/ImmSet
        // we only have a val but need to store it in both the key and the val
        if (m_staticColType.back() == HeaderKind::Dict) {
          m_staticArrays.back().set(tvAsVariant(&tvVal),
                                    tvAsVariant(&tvVal));
        } else {
          m_staticArrays.back().append(tvAsCVarRef(&tvVal));
        }
      }
    } else {
      // Assume new array is on top of stack
      bool hasKey = (bool)key;
      if (hasKey) {
        visit(key);
        emitConvertToCellOrLoc(e);
      }
      visit(ap->getValue());
      if (ap->isRef()) {
        emitConvertToVar(e);
        if (hasKey) {
          emitConvertSecondToCell(e);
          e.AddElemV();
        } else {
          e.AddNewElemV();
        }
      } else {
        emitConvertToCell(e);
        if (hasKey) {
          emitConvertSecondToCell(e);
          e.AddElemC();
        } else {
          e.AddNewElemC();
        }
      }
    }
    return true;
  }
  case Construct::KindOfExpressionList: {
    auto el = static_pointer_cast<ExpressionList>(node);
    if (!m_staticArrays.empty() &&
        (m_staticColType.back() == HeaderKind::VecArray ||
         m_staticColType.back() == HeaderKind::Keyset)) {
      auto const nelem = el->getCount();
      for (int i = 0; i < nelem; ++i) {
        auto const expr = (*el)[i];
        assert(expr->isScalar());
        TypedValue tvVal;
        initScalar(tvVal, expr);
        m_staticArrays.back().append(tvAsCVarRef(&tvVal));
      }
      return true;
    } else {
      int nelem = el->getCount(), i;
      bool pop = el->getListKind() != ExpressionList::ListKindParam;
      int keep = el->getListKind() == ExpressionList::ListKindLeft ?
        0 : nelem - 1;
      int cnt = 0;
      for (i = 0; i < nelem; i++) {
        ExpressionPtr p((*el)[i]);
        if (visit(p)) {
          if (pop && i != keep) {
            emitPop(e);
          } else {
            cnt++;
          }
        }
      }
      return cnt != 0;
    }
  }
  case Construct::KindOfParameterExpression: {
    not_implemented();
  }
  case Construct::KindOfModifierExpression: {
    not_implemented();
  }
  case Construct::KindOfUserAttribute: {
    not_implemented();
  }
  case Construct::KindOfClosureExpression: {
    // Closures are implemented by anonymous classes that extend Closure.
    // There is one anonymous class per closure body.
    auto ce = static_pointer_cast<ClosureExpression>(node);

    // Build a convenient list of use-variables. Each one corresponds to:
    // (a) an instance variable, to store the value until call time
    // (b) a parameter of the generated constructor
    // (c) an argument to the constructor at the definition site
    // (d) a line of code in the generated constructor;
    // (e) a line of code in the generated prologue to the closure body
    auto useList = ce->getClosureVariables();
    ClosureUseVarVec useVars;
    int useCount = (useList ? useList->getCount() : 0);
    if (useList) {
      for (int i = 0; i < useCount; ++i) {
        auto var = static_pointer_cast<ParameterExpression>((*useList)[i]);
        StringData* varName = makeStaticString(var->getName());
        useVars.push_back(ClosureUseVar(varName, var->isRef()));
      }
    }

    // We're still at the closure definition site. Emit code to instantiate
    // the new anonymous class, with the use variables as arguments.
    ExpressionListPtr valuesList(ce->getClosureValues());
    for (int i = 0; i < useCount; ++i) {
      ce->type() == ClosureType::Short
        ? emitLambdaCaptureArg(e, (*valuesList)[i])
        : emitClosureUseVar(e, (*valuesList)[i], i, useVars[i].second);
    }

    // Create a new class to hold the closure.
    PreClassEmitter* pce = m_ue.newPreClassEmitter(
      ce->getClosureFunction()->getOriginalName(), PreClass::NotHoistable);

    auto const attrs = AttrNoOverride | AttrUnique | AttrPersistent;

    static auto const parentName = makeStaticString("Closure");
    pce->init(ce->line0(), ce->line1(), m_ue.bcPos(),
              attrs, parentName, nullptr);

    // Instance properties---one for each use var, and one for
    // each static local.
    TypedValue uninit;
    tvWriteUninit(&uninit);
    for (auto& useVar : useVars) {
      pce->addProperty(useVar.first, AttrPrivate, staticEmptyString(), nullptr,
                       &uninit, RepoAuthType{});
    }

    // The __invoke method. This is the body of the closure, preceded by
    // code that pulls the object's instance variables into locals.
    static auto const invokeName = makeStaticString("__invoke");
    auto const invoke = m_ue.newMethodEmitter(invokeName, pce);
    invoke->isClosureBody = true;
    pce->addMethod(invoke);
    auto const body =
      static_pointer_cast<MethodStatement>(ce->getClosureFunction());
    postponeMeth(body, invoke, false, new ClosureUseVarVec(useVars));

    e.CreateCl(useCount, pce->id());

    return true;
  }
  case Construct::KindOfClassExpression: {
    auto ce = static_pointer_cast<ClassExpression>(node);

    auto save = m_evalStack;
    auto const id = emitClass(e, ce->getClass()->getClassScope(), false);
    if (id < 0) {
      m_evalStackIsUnknown = false;
      m_evalStack = save;
      e.Null();
    } else {
      auto fpiStart = m_ue.bcPos();
      auto params = ce->getParams();
      int numParams = params ? params->getCount() : 0;
      e.FPushCtorI(numParams, id);
      emitCall(e, ce, ce->getParams(), fpiStart);
      e.PopR();
    }
    return true;
  }
  case Construct::KindOfYieldExpression: {
    auto y = static_pointer_cast<YieldExpression>(node);

    registerYieldAwait(y);
    assert(m_evalStack.size() == 0);
    assert(m_evalStack.clsRefSlotStackEmpty());

    // evaluate key passed to yield, if applicable
    ExpressionPtr keyExp = y->getKeyExpression();
    if (keyExp) {
      m_curFunc->isPairGenerator = true;
      visit(keyExp);
      emitConvertToCell(e);
    }

    // evaluate value expression passed to yield
    visit(y->getValueExpression());
    emitConvertToCell(e);

    // suspend generator
    if (keyExp) {
      assert(m_evalStack.size() == 2);
      e.YieldK();
    } else {
      assert(m_evalStack.size() == 1);
      e.Yield();
    }

    // continue with the received result on the stack
    assert(m_evalStack.size() == 1);
    return true;
  }
  case Construct::KindOfYieldFromExpression: {
    auto yf = static_pointer_cast<YieldFromExpression>(node);

    registerYieldAwait(yf);
    assert(m_evalStack.size() == 0);
    assert(m_evalStack.clsRefSlotStackEmpty());

    emitYieldFrom(e, yf->getExpression());

    return true;
  }
  case Construct::KindOfAwaitExpression: {
    auto await = static_pointer_cast<AwaitExpression>(node);

    registerYieldAwait(await);
    assert(m_evalStack.size() == 0);
    assert(m_evalStack.clsRefSlotStackEmpty());

    auto expression = await->getExpression();
    if (emitInlineGen(e, expression)) return true;

    Label resume;

    // evaluate expression passed to await
    visit(expression);
    emitConvertToCell(e);

    // if expr is null, just continue
    e.Dup();
    emitIsType(e, IsTypeOp::Null);
    e.JmpNZ(resume);

    assert(m_evalStack.size() == 1);

    e.Await();

    resume.set(e);
    return true;
  }
  case Construct::KindOfUseDeclarationStatementFragment:
  case Construct::KindOfExpression: {
    not_reached();
  }
  }

  not_reached();
}

bool EmitterVisitor::emitScalarValue(Emitter& e, const Variant& v) {
  switch (v.getRawType()) {
    case KindOfUninit:
      e.NullUninit();
      return true;

    case KindOfNull:
      e.Null();
      return true;

    case KindOfBoolean:
      v.asBooleanVal() ? e.True() : e.False();
      return true;

    case KindOfInt64:
      e.Int(v.getInt64());
      return true;

    case KindOfDouble:
      e.Double(v.getDouble());
      return true;

    case KindOfPersistentString:
    case KindOfString:
      e.String(makeStaticString(v.getStringData()));
      return true;

    case KindOfPersistentVec:
    case KindOfVec:
      assert(v.isVecArray());
      e.Vec(ArrayData::GetScalarArray(v.getArrayData()));
      return true;

    case KindOfPersistentDict:
    case KindOfDict:
      assert(v.isDict());
      e.Dict(ArrayData::GetScalarArray(v.getArrayData()));
      return true;

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      assert(v.isKeyset());
      e.Keyset(ArrayData::GetScalarArray(v.getArrayData()));
      return true;

    case KindOfPersistentArray:
    case KindOfArray:
      assert(v.isPHPArray());
      e.Array(ArrayData::GetScalarArray(v.getArrayData()));
      return true;

    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
      return false;
  }
  not_reached();
}

void EmitterVisitor::emitConstMethodCallNoParams(Emitter& e,
                                                 const std::string& name) {
  auto const nameLit = makeStaticString(name);
  auto const fpiStart = m_ue.bcPos();
  e.FPushObjMethodD(0, nameLit, ObjMethodOp::NullThrows);
  {
    FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
  }
  e.FCall(0);
  emitConvertToCell(e);
}

namespace {
const StaticString
  s_hh_invariant_violation("hh\\invariant_violation"),
  s_invariant_violation("invariant_violation"),
  s_gennull("HH\\Asio\\null"),
  s_fromArray("fromArray"),
  s_AwaitAllWaitHandle("HH\\AwaitAllWaitHandle"),
  s_WaitHandle("HH\\WaitHandle")
  ;
}

bool EmitterVisitor::emitInlineGenva(
  Emitter& e,
  const SimpleFunctionCallPtr& call
) {
  assert(call->isCallToFunction("genva"));
  const auto params = call->getParams();
  if (!params) {
    e.Array(staticEmptyArray());
    return true;
  }
  if (params->containsUnpack()) {
    throw IncludeTimeFatalException(params, "do not use ...$args with genva()");
  }
  const auto num_params = params->getCount();
  assertx(num_params > 0);

  for (auto i = int{0}; i < num_params; i++) {
    Label gwh, have_wh;

    visit((*params)[i]);
    emitConvertToCell(e);

    // if we already have a WaitHandle, we can skip the getWaitHandle call
    e.Dup();
    e.InstanceOfD(s_WaitHandle.get());
    e.JmpNZ(have_wh);

    // if $_ is null, create a HH\Asio\null() instead of calling getWaitHandle
    e.Dup();
    emitIsType(e, IsTypeOp::Null);
    e.JmpZ(gwh);
    emitPop(e);

    Offset fpiStart = m_ue.bcPos();
    e.FPushFuncD(0, s_gennull.get());
    {
      FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
    }
    e.FCall(0);
    e.UnboxR();
    e.Jmp(have_wh);

    gwh.set(e);
    emitConstMethodCallNoParams(e, "getWaitHandle");
    have_wh.set(e);
  }

  std::vector<Id> waithandles(num_params);
  for (auto i = int{num_params - 1}; i >= 0; --i) {
    waithandles[i] = emitSetUnnamedL(e);
  }
  assertx(waithandles.size() == num_params);

  // AwaitAllWaitHandle::fromArray() always returns a WaitHandle.
  Offset fpiStart = m_ue.bcPos();
  e.FPushClsMethodD(1, s_fromArray.get(), s_AwaitAllWaitHandle.get());
  {
    FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
    // create a packed array of the waithandles
    for (const auto wh : waithandles) {
      emitVirtualLocal(wh);
      emitCGet(e);
    }
    e.NewPackedArray(num_params);
    emitFPass(e, 0, PassByRefKind::ErrorOnCell);
  }
  e.FCall(1);
  e.UnboxR();

  e.Await();
  // result of AwaitAllWaitHandle does not matter
  emitPop(e);

  for (const auto wh : waithandles) {
    emitVirtualLocal(wh);
    emitPushL(e);
    e.WHResult();
  }
  e.NewPackedArray(num_params);

  for (auto wh : waithandles) {
    m_curFunc->freeUnnamedLocal(wh);
  }

  newFaultRegionAndFunclet(
    fpiStart, m_ue.bcPos(),
    new UnsetUnnamedLocalsThunklet(std::move(waithandles)));

  return true;
}

bool EmitterVisitor::emitInlineGena(
  Emitter& e,
  const SimpleFunctionCallPtr& call
) {
  assert(call->isCallToFunction("gena"));
  const auto params = call->getParams();

  if (params->getCount() != 1) return false;

  //
  // Convert input into an array of WH (inline this?)
  // Two elements is the most common size.
  //
  e.NewArray(2);
  const auto array = emitSetUnnamedL(e);
  const auto arrayStart = m_ue.bcPos();

  //
  // Iterate over input and store wait handles for all elements in
  // a new array.
  //
  const auto key = m_curFunc->allocUnnamedLocal();
  const auto item = m_curFunc->allocUnnamedLocal();
  {
    emitVirtualLocal(key);
    emitVirtualLocal(item);

    visit((*params)[0]);
    emitConvertToCell(e);

    Label endloop;
    const auto initItId = m_curFunc->allocIterator();
    e.IterInitK(initItId, endloop, item, key);
    const auto iterStart = m_ue.bcPos();
    {
      Label loop(e);
      Label have_wh;

      emitVirtualLocal(array); // for Set below
      emitVirtualLocal(key); // for Set below
      markElem(e);

      emitVirtualLocal(item);
      emitCGet(e);

      // call getWaitHandle if we don't already have a WaitHandle
      e.Dup();
      e.InstanceOfD(s_WaitHandle.get());
      e.JmpNZ(have_wh);
      emitConstMethodCallNoParams(e, "getWaitHandle");
      have_wh.set(e);

      emitSet(e);   // array[$key] = $item->getWaitHandle();
      emitPop(e);

      emitVirtualLocal(key);
      emitVirtualLocal(item);
      e.IterNextK(initItId, loop, item, key);
      endloop.set(e);
    }
    // Clear item and key.  Free iterator.
    emitUnsetL(e, item);
    emitUnsetL(e, key);
    m_curFunc->freeIterator(initItId);

    newFaultRegionAndFunclet(iterStart, m_ue.bcPos(),
                             new UnsetUnnamedLocalsThunklet({item, key}));
    newFaultRegionAndFunclet(iterStart, m_ue.bcPos(),
                             new IterFreeThunklet(initItId, false),
                             { initItId, KindOfIter });
  }

  //
  // Construct an AAWH from the array.
  //
  const auto fromArrayStart = m_ue.bcPos();
  e.FPushClsMethodD(1, s_fromArray.get(), s_AwaitAllWaitHandle.get());
  {
    FPIRegionRecorder fpi(this, m_ue, m_evalStack, fromArrayStart);
    emitVirtualLocal(array);
    e.FPassL(0, array);
  }
  e.FCall(1);
  e.UnboxR();

  //
  // Await on the AAWH.  Note: the result of Await does not matter.
  //
  e.Await();
  emitPop(e);

  //
  // Iterate over results and store in array.  Reuse same temporary array.
  //
  {
    emitVirtualLocal(key);
    emitVirtualLocal(item);

    emitVirtualLocal(array);
    emitCGet(e);

    Label endloop2;
    const auto resultItId = m_curFunc->allocIterator();
    e.IterInitK(resultItId, endloop2, item, key);
    const auto iterStart2 = m_ue.bcPos();
    {
      Label loop2(e);

      emitVirtualLocal(array); // for Set below
      emitVirtualLocal(key); // for Set below
      markElem(e);

      emitVirtualLocal(item);
      emitCGet(e);
      e.WHResult();

      emitSet(e);   // array[$key] = WHResult($item);
      emitPop(e);

      emitVirtualLocal(key);
      emitVirtualLocal(item);
      e.IterNextK(resultItId, loop2, item, key);
      endloop2.set(e);
    }
    // Clear item and key.  Free iterator.
    emitUnsetL(e, item);
    emitUnsetL(e, key);
    m_curFunc->freeIterator(resultItId);

    newFaultRegionAndFunclet(iterStart2, m_ue.bcPos(),
                             new UnsetUnnamedLocalsThunklet({item, key}));

    newFaultRegionAndFunclet(iterStart2, m_ue.bcPos(),
                             new IterFreeThunklet(resultItId, false),
                             { resultItId, KindOfIter });
  }

  // clean up locals
  m_curFunc->freeUnnamedLocal(item);
  m_curFunc->freeUnnamedLocal(key);

  // Leave result array on the stack.
  emitPushAndFreeUnnamedL(e, array, arrayStart);

  return true;
}

bool EmitterVisitor::emitInlineGen(
  Emitter& e,
  const ExpressionPtr& expression
) {
  if (!m_ue.m_isHHFile || !RuntimeOption::EnableHipHopSyntax ||
      !expression->is(Expression::KindOfSimpleFunctionCall) ||
      RuntimeOption::EvalJitEnableRenameFunction ||
      RuntimeOption::EvalDisableHphpcOpts) {
    return false;
  }

  const auto call = static_pointer_cast<SimpleFunctionCall>(expression);
  if (call->isCallToFunction("genva")) {
    return emitInlineGenva(e, call);
  } else if (call->isCallToFunction("gena")) {
    return emitInlineGena(e, call);
  }
  return false;
}

// Compile a static string as HHAS
//
// The hhas bytecodes should either leave the stack untouched, in
// which case the result of the hh\asm() expression will be null; or
// they should push exactly one cell, which will be the result of the
// hh\asm() expression.
bool EmitterVisitor::emitInlineHHAS(Emitter& e, SimpleFunctionCallPtr func) {
  if (SystemLib::s_inited &&
      !func->getFunctionScope()->isSystem() &&
      !RuntimeOption::EvalAllowHhas) {
    throw IncludeTimeFatalException(func,
      "Inline hhas only allowed in systemlib");
  }
  auto const params = func->getParams();
  if (!params || params->getCount() != 1) {
    throw IncludeTimeFatalException(func,
      "Inline hhas expects exactly one argument");
  }
  Variant v;
  if (!((*params)[0]->getScalarValue(v)) || !v.isString()) {
    throw IncludeTimeFatalException(func,
      "Inline hhas must be string literal");
  }

  try {
    auto result =
      assemble_expression(m_ue, m_curFunc,
                          m_evalStack.size() + m_evalStack.fdescSize(),
                          v.toString().toCppString());
    switch (result) {
      case AsmResult::NoResult:
        e.Null();
        break;
      case AsmResult::ValuePushed:
        pushEvalStack(StackSym::C);
        break;
      case AsmResult::Unreachable:
        // PrevOpcode is only used to determine whether the current position
        // is reachable. Arbitrarily set it to Jmp to ensure that the emitter
        // knows the current position is not reachable
        setPrevOpcode(Op::Jmp);
        pushEvalStack(StackSym::C);
        break;
    }
  } catch (const std::exception& ex) {
    throw IncludeTimeFatalException(func, ex.what());
  }

  return true;
}

bool EmitterVisitor::emitHHInvariant(Emitter& e, SimpleFunctionCallPtr call) {
  if (!m_ue.m_isHHFile && !RuntimeOption::EnableHipHopSyntax) return false;

  auto const params = call->getParams();
  if (!params || params->getCount() < 1) return false;

  Label ok;

  visit((*params)[0]);
  emitCGet(e);
  e.JmpNZ(ok);

  auto const fpiStart = m_ue.bcPos();
  e.FPushFuncD(params->getCount() - 1, s_hh_invariant_violation.get());
  {
    FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
    for (auto i = uint32_t{1}; i < params->getCount(); ++i) {
      emitFuncCallArg(e, (*params)[i], i - 1, false);
    }
  }
  e.FCall(params->getCount() - 1);
  emitPop(e);
  // The invariant_violation can't return; but bytecode invariants mandate an
  // opcode that can't fall through:
  e.String(s_invariant_violation.get());
  e.Fatal(FatalOp::Runtime);

  ok.set(e);
  e.Null(); // invariant returns null if used in an expression, void according
            // to the typechecker.
  return true;
}

int EmitterVisitor::scanStackForLocation(int iLast) {
  assertx(iLast >= 0);
  assertx(iLast < (int)m_evalStack.size());
  for (int i = iLast; i >= 0; --i) {
    char marker = StackSym::GetMarker(m_evalStack.get(i));
    if (marker != StackSym::E && marker != StackSym::W &&
        marker != StackSym::P && marker != StackSym::M &&
        marker != StackSym::Q) {
      return i;
    }
  }
  InvariantViolation("Emitter expected a location on the stack but none "
                     "was found (at offset %d)",
                     m_ue.bcPos());
  return 0;
}

size_t EmitterVisitor::emitMOp(
  int iFirst,
  int& iLast,
  Emitter& e,
  MInstrOpts opts,
  bool includeLast
) {
  auto stackIdx = [&](int i) {
    return m_evalStack.actualSize() - 1 - m_evalStack.getActualPos(i);
  };

  auto const baseMode = opts.fpass ? MOpMode::None : opts.mode;

  // Emit the base location operation.
  auto sym = m_evalStack.get(iFirst);
  auto flavor = StackSym::GetSymFlavor(sym);
  switch (StackSym::GetMarker(sym)) {
    case StackSym::N:
      switch (flavor) {
        case StackSym::C:
          if (opts.fpass) {
            e.FPassBaseNC(opts.paramId, stackIdx(iFirst));
          } else {
            e.BaseNC(stackIdx(iFirst), baseMode);
          }
          break;
        case StackSym::L:
          if (opts.fpass) {
            e.FPassBaseNL(opts.paramId, m_evalStack.getLoc(iFirst));
          } else {
            e.BaseNL(m_evalStack.getLoc(iFirst), baseMode);
          }
          break;
        default:
          always_assert(false);
      }
      break;

    case StackSym::G:
      switch (flavor) {
        case StackSym::C:
          if (opts.fpass) {
            e.FPassBaseGC(opts.paramId, stackIdx(iFirst));
          } else {
            e.BaseGC(stackIdx(iFirst), baseMode);
          }
          break;
        case StackSym::L:
          if (opts.fpass) {
            e.FPassBaseGL(opts.paramId, m_evalStack.getLoc(iFirst));
          } else {
            e.BaseGL(m_evalStack.getLoc(iFirst), baseMode);
          }
          break;
        default:
          always_assert(false);
      }
      break;

    case StackSym::S: {
      if (m_evalStack.get(iLast) != StackSym::AM) {
        unexpectedStackSym(sym, "S-vector base, class ref");
      }

      switch (flavor) {
        case StackSym::C:
          e.BaseSC(stackIdx(iFirst), kClsRefSlotPlaceholder);
          break;
        case StackSym::L:
          e.BaseSL(m_evalStack.getLoc(iFirst), kClsRefSlotPlaceholder);
          break;
        default:
          unexpectedStackSym(sym, "S-vector base, prop name");
          break;
      }
      // The BaseS* bytecodes consume a symbolic class-ref from the symbolic
      // stack. Adjust iLast accordingly.
      --iLast;
      break;
    }

    case StackSym::None:
      switch (flavor) {
        case StackSym::L:
          if (opts.fpass) {
            e.FPassBaseL(opts.paramId, m_evalStack.getLoc(iFirst));
          } else {
            e.BaseL(m_evalStack.getLoc(iFirst), baseMode);
          }
          break;
        case StackSym::C:
          e.BaseC(stackIdx(iFirst));
          break;
        case StackSym::R:
          e.BaseR(stackIdx(iFirst));
          break;
        case StackSym::H:
          e.BaseH();
          break;
        default:
          always_assert(false);
      }
      break;

    default:
      always_assert(false && "Bad base marker");
  }

  assert(StackSym::GetMarker(m_evalStack.get(iLast)) != StackSym::M);

  // Emit all intermediate operations, optionally leaving the final operation up
  // to our caller.
  for (auto i = iFirst + 1; i < (includeLast ? iLast+1 : iLast); ++i) {
    if (opts.fpass) {
      e.FPassDim(opts.paramId, symToMemberKey(e, i, opts.allowW));
    } else {
      e.Dim(opts.mode, symToMemberKey(e, i, opts.allowW));
    }
  }

  size_t stackCount = 0;
  for (int i = iFirst; i <= iLast; ++i) {
    if (!StackSym::IsSymbolic(m_evalStack.get(i))) ++stackCount;
  }
  return stackCount;
}

MemberKey EmitterVisitor::symToMemberKey(Emitter& e, int i, bool allowW) {
  auto const sym = m_evalStack.get(i);
  auto const marker = StackSym::GetMarker(sym);
  if (marker == StackSym::W) {
    if (allowW) return MemberKey{};

    throw EmitterVisitor::IncludeTimeFatalException(
      e.getNode(), "Cannot use [] for reading"
    );
  }

  switch (StackSym::GetSymFlavor(sym)) {
    case StackSym::L: {
      auto const local = m_evalStack.getLoc(i);
      switch (marker) {
        case StackSym::E: return MemberKey{MEL, local};
        case StackSym::P: return MemberKey{MPL, local};
        default:          always_assert(false);
      }
    }
    case StackSym::C: {
      auto const idx =
        int32_t(m_evalStack.actualSize() - 1 - m_evalStack.getActualPos(i));
      switch (marker) {
        case StackSym::E: return MemberKey{MEC, idx};
        case StackSym::P: return MemberKey{MPC, idx};
        default:          always_assert(false);
      }
    }
    case StackSym::I: {
      auto const int64 = m_evalStack.getInt(i);
      switch (marker) {
        case StackSym::E: return MemberKey{MEI, int64};
        default:          always_assert(false);
      }
    }
    case StackSym::T: {
      auto const str = m_evalStack.getName(i);
      switch (marker) {
        case StackSym::E: return MemberKey{MET, str};
        case StackSym::P: return MemberKey{MPT, str};
        case StackSym::Q: return MemberKey{MQT, str};
        default:          always_assert(false);
      }
    }
    default:
      always_assert(false);
  }
}

void EmitterVisitor::emitPop(Emitter& e) {
  if (checkIfStackEmpty("Pop*")) return;
  LocationGuard loc(e, m_tempLoc);
  m_tempLoc.clear();

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.CGetL(m_evalStack.getLoc(i)); // fall through
      case StackSym::C:  e.PopC(); break;
      case StackSym::LN: e.CGetL(m_evalStack.getLoc(i)); // fall through
      case StackSym::CN: e.CGetN(); e.PopC(); break;
      case StackSym::LG: e.CGetL(m_evalStack.getLoc(i)); // fall through
      case StackSym::CG: e.CGetG(); e.PopC(); break;
      case StackSym::LS: e.CGetL(m_evalStack.getLoc(i)); // fall through
      case StackSym::CS:
        e.CGetS(kClsRefSlotPlaceholder);
        e.PopC();
        break;
      case StackSym::V:  e.PopV(); break;
      case StackSym::R:  e.PopR(); break;
      default: {
        unexpectedStackSym(sym, "emitPop");
        break;
      }
    }
  } else {
    emitQueryMOp(i, iLast, e, QueryMOp::CGet);
    e.PopC();
  }
}

void EmitterVisitor::emitCGetL2(Emitter& e) {
  assert(m_evalStack.size() >= 2);
  assert(m_evalStack.sizeActual() >= 1);

  auto const symFlavor = StackSym::GetSymFlavor(
    m_evalStack.get(m_evalStack.size() - 2)
  );
  auto const stackIdx = (symFlavor == StackSym::A) ? 3 : 2;

  assert(m_evalStack.size() >= stackIdx);
  assert(StackSym::GetSymFlavor(m_evalStack.get(m_evalStack.size() - stackIdx))
         == StackSym::L);
  int localIdx = m_evalStack.getLoc(m_evalStack.size() - stackIdx);
  e.CGetL2(localIdx);
}

void EmitterVisitor::emitPushL(Emitter& e) {
  assert(m_evalStack.size() >= 1);
  auto const idx = m_evalStack.size() - 1;
  assert(StackSym::GetSymFlavor(m_evalStack.get(idx)) == StackSym::L);
  e.PushL(m_evalStack.getLoc(idx));
}

void EmitterVisitor::emitAGet(Emitter& e) {
  if (checkIfStackEmpty("AGet*")) return;

  emitConvertToCellOrLoc(e);
  switch (char sym = m_evalStack.top()) {
  case StackSym::L:
    e.ClsRefGetL(
      m_evalStack.getLoc(m_evalStack.size() - 1),
      kClsRefSlotPlaceholder
    );
    break;
  case StackSym::C:
    e.ClsRefGetC(kClsRefSlotPlaceholder);
    break;
  default:
    unexpectedStackSym(sym, "emitAGet");
  }
}

void EmitterVisitor::emitQueryMOp(int iFirst, int iLast, Emitter& e,
                                  QueryMOp op) {
  auto const flags = getQueryMOpMode(op);
  auto const stackCount = emitMOp(iFirst, iLast, e, MInstrOpts{flags});
  e.QueryM(stackCount, op, symToMemberKey(e, iLast, false /* allowW */));
}

void EmitterVisitor::emitCGet(Emitter& e) {
  if (checkIfStackEmpty("CGet*")) return;
  LocationGuard loc(e, m_tempLoc);
  m_tempLoc.clear();

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.CGetL(m_evalStack.getLoc(i));  break;
      case StackSym::C:  /* nop */   break;
      case StackSym::LN: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CN: e.CGetN();  break;
      case StackSym::LG: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CG: e.CGetG();  break;
      case StackSym::LS: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CS: e.CGetS(kClsRefSlotPlaceholder); break;
      case StackSym::V:  e.Unbox();  break;
      case StackSym::R:  e.UnboxR(); break;
      default: {
        unexpectedStackSym(sym, "emitCGet");
        break;
      }
    }
  } else {
    emitQueryMOp(i, iLast, e, QueryMOp::CGet);
  }
}

void EmitterVisitor::emitCGetQuiet(Emitter& e) {
  if (checkIfStackEmpty("CGetQuiet*")) return;
  LocationGuard loc(e, m_tempLoc);
  m_tempLoc.clear();

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.CGetQuietL(m_evalStack.getLoc(i));  break;
      case StackSym::C:  /* nop */   break;
      case StackSym::LN: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CN: e.CGetQuietN();  break;
      case StackSym::LG: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CG: e.CGetQuietG();  break;
      case StackSym::LS: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CS: e.CGetS(kClsRefSlotPlaceholder);  break;
      case StackSym::V:  e.Unbox();  break;
      case StackSym::R:  e.UnboxR(); break;
      default: {
        unexpectedStackSym(sym, "emitCGetQuiet");
        break;
      }
    }

  } else {
    emitQueryMOp(i, iLast, e, QueryMOp::CGetQuiet);
  }
}

bool EmitterVisitor::emitVGet(Emitter& e, bool skipCells) {
  if (checkIfStackEmpty("VGet*")) return false;
  LocationGuard loc(e, m_tempLoc);
  m_tempLoc.clear();

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.VGetL(m_evalStack.getLoc(i)); break;
      case StackSym::C:  if (skipCells) return true; e.Box(); break;
      case StackSym::LN: e.CGetL(m_evalStack.getLoc(i)); // fall through
      case StackSym::CN: e.VGetN(); break;
      case StackSym::LG: e.CGetL(m_evalStack.getLoc(i)); // fall through
      case StackSym::CG: e.VGetG(); break;
      case StackSym::LS: e.CGetL(m_evalStack.getLoc(i)); // fall through
      case StackSym::CS: e.VGetS(kClsRefSlotPlaceholder); break;
      case StackSym::V:  /* nop */  break;
      case StackSym::R:  e.BoxR();  break;
      default: {
        unexpectedStackSym(sym, "emitVGet");
        break;
      }
    }
  } else {
    auto const stackCount =
      emitMOp(i, iLast, e, MInstrOpts{MOpMode::Define});
    e.VGetM(stackCount, symToMemberKey(e, iLast, true /* allowW */));
  }
  return false;
}

Id EmitterVisitor::emitVisitAndSetUnnamedL(Emitter& e, ExpressionPtr exp) {
  visit(exp);
  emitConvertToCell(e);

  return emitSetUnnamedL(e);
}

Id EmitterVisitor::emitSetUnnamedL(Emitter& e) {
  Id tempLocal = m_curFunc->allocUnnamedLocal();
  emitSetL(e, tempLocal);
  emitPop(e);
  return tempLocal;
}

void EmitterVisitor::emitFreeUnnamedL(Emitter& e, Id tempLocal, Offset start) {
  assert(tempLocal >= 0);
  assert(start != InvalidAbsoluteOffset);
  newFaultRegionAndFunclet(start, m_ue.bcPos(),
                           new UnsetUnnamedLocalThunklet(tempLocal));
  emitUnsetL(e, tempLocal);
  m_curFunc->freeUnnamedLocal(tempLocal);
}

void EmitterVisitor::emitPushAndFreeUnnamedL(Emitter& e, Id tempLocal,
                                             Offset start) {
  assert(tempLocal >= 0);
  assert(start != InvalidAbsoluteOffset);
  newFaultRegionAndFunclet(start, m_ue.bcPos(),
                           new UnsetUnnamedLocalThunklet(tempLocal));
  emitVirtualLocal(tempLocal);
  emitPushL(e);
  m_curFunc->freeUnnamedLocal(tempLocal);
}

EmitterVisitor::PassByRefKind
EmitterVisitor::getPassByRefKind(ExpressionPtr exp) {
  auto permissiveKind = PassByRefKind::AllowCell;

  // The PassByRefKind of a list assignment expression is determined
  // by the PassByRefKind of the RHS. This loop will repeatedly recurse
  // on the RHS until it encounters an expression other than a list
  // assignment expression.
  while (exp->is(Expression::KindOfListAssignment)) {
    exp = static_pointer_cast<ListAssignment>(exp)->getArray();
    permissiveKind = PassByRefKind::WarnOnCell;
  }

  switch (exp->getKindOf()) {
    case Expression::KindOfSimpleFunctionCall: {
      auto sfc = static_pointer_cast<SimpleFunctionCall>(exp);
      // this only happens for calls that have been morphed into bytecode
      // e.g. idx(), abs(), strlen(), etc..
      // It is to allow the following code to work
      // function f(&$arg) {...}
      // f(idx($array, 'key')); <- this fails otherwise
      if (sfc->hasBeenChangedToBytecode()) {
        return PassByRefKind::AllowCell;
      }
    } break;
    case Expression::KindOfNewObjectExpression:
    case Expression::KindOfIncludeExpression:
    case Expression::KindOfSimpleVariable:
      // New and include/require
      return PassByRefKind::AllowCell;
    case Expression::KindOfArrayElementExpression:
      // Allow if bare; warn if inside list assignment
      return permissiveKind;
    case Expression::KindOfAssignmentExpression:
      // Assignment (=) and binding assignment (=&)
      return PassByRefKind::WarnOnCell;
    case Expression::KindOfBinaryOpExpression: {
      auto b = static_pointer_cast<BinaryOpExpression>(exp);
      // Assignment op (+=, -=, *=, etc)
      if (b->isAssignmentOp()) return PassByRefKind::WarnOnCell;
    } break;
    case Expression::KindOfUnaryOpExpression: {
      auto u = static_pointer_cast<UnaryOpExpression>(exp);
      int op = u->getOp();
      if (op == T_CLONE) {
        // clone
        return PassByRefKind::AllowCell;
      } else if (op == '@' || op == T_EVAL ||
                 ((op == T_INC || op == T_DEC) && u->getFront())) {
        // Silence operator, eval, preincrement, and predecrement
        return PassByRefKind::WarnOnCell;
      }
    } break;
    case Expression::KindOfExpressionList: {
      auto el = static_pointer_cast<ExpressionList>(exp);
      if (el->getListKind() != ExpressionList::ListKindParam) {
        return el->getListKind() == ExpressionList::ListKindWrappedNoWarn ?
          PassByRefKind::AllowCell : PassByRefKind::WarnOnCell;
      }
    } break;
    default:
      break;
  }
  // All other cases
  return PassByRefKind::ErrorOnCell;
}

void EmitterVisitor::emitClosureUseVar(Emitter& e, ExpressionPtr exp,
                                       int /*paramId*/, bool byRef) {
  visit(exp);
  if (checkIfStackEmpty("Closure Use Var*")) return;
  if (byRef) {
    emitVGet(e, true);
  } else {
    emitCGet(e);
  }
}

static bool isNormalLocalVariable(const ExpressionPtr& expr) {
  SimpleVariable* sv = static_cast<SimpleVariable*>(expr.get());
  return (expr->is(Expression::KindOfSimpleVariable) &&
          !sv->isSuperGlobal() &&
          !sv->isThis());
}

void EmitterVisitor::emitLambdaCaptureArg(Emitter& e, ExpressionPtr exp) {
  // Constant folding may lead this to be not a var anymore,
  // so we should not be emitting *GetL in this case.
  if (!isNormalLocalVariable(exp)) {
    visit(exp);
    return;
  }
  auto const sv = static_cast<SimpleVariable*>(exp.get());
  Id locId = m_curFunc->lookupVarId(makeStaticString(sv->getName()));
  emitVirtualLocal(locId);
  e.CUGetL(locId);
}

void EmitterVisitor::emitFuncCallArg(Emitter& e,
                                     ExpressionPtr exp,
                                     int paramId,
                                     bool isUnpack) {
  visit(exp);
  if (checkIfStackEmpty("FPass*")) return;

  // TODO(4599379): if dealing with an unpack, here is where we'd want to
  // emit a bytecode to traverse any containers;

  auto kind = getPassByRefKind(exp);
  if (isUnpack) {
    // This deals with the case where the called function has a
    // by ref param at the index of the unpack (because we don't
    // want to box the unpack itself).
    // But note that unless the user created the array manually,
    // and added reference params at the correct places, we'll
    // still get warnings, and the array elements will not be
    // passed by reference.
    emitConvertToCell(e);
    kind = PassByRefKind::AllowCell;
  }
  emitFPass(e, paramId, kind);
}

void EmitterVisitor::emitFPass(Emitter& e, int paramId,
                               PassByRefKind passByRefKind) {
  if (checkIfStackEmpty("FPass*")) return;
  LocationGuard locGuard(e, m_tempLoc);
  m_tempLoc.clear();

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.FPassL(paramId, m_evalStack.getLoc(i)); break;
      case StackSym::C:
        switch (passByRefKind) {
          case PassByRefKind::AllowCell:   e.FPassC(paramId); break;
          case PassByRefKind::WarnOnCell:  e.FPassCW(paramId); break;
          case PassByRefKind::ErrorOnCell: e.FPassCE(paramId); break;
          default: assert(false);
        }
        break;
      case StackSym::LN: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CN: e.FPassN(paramId); break;
      case StackSym::LG: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CG: e.FPassG(paramId); break;
      case StackSym::LS: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CS:
        e.FPassS(paramId, kClsRefSlotPlaceholder);
        break;
      case StackSym::V:  e.FPassV(paramId); break;
      case StackSym::R:  e.FPassR(paramId); break;
      default: {
        unexpectedStackSym(sym, "emitFPass");
        break;
      }
    }
  } else {
    auto const stackCount = emitMOp(i, iLast, e, MInstrOpts{paramId});
    e.FPassM(paramId, stackCount, symToMemberKey(e, iLast, true /* allowW */));
  }
}

void EmitterVisitor::emitIsset(Emitter& e) {
  if (checkIfStackEmpty("Isset*")) return;

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.IssetL(m_evalStack.getLoc(i)); break;
      case StackSym::LN: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CN: e.IssetN(); break;
      case StackSym::LG: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CG: e.IssetG(); break;
      case StackSym::LS: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CS: e.IssetS(kClsRefSlotPlaceholder); break;
      //XXX: Zend does not allow isset() on the result
      // of a function call. We allow it here so that emitted
      // code is valid. Once the parser handles this correctly,
      // the R and C cases can go.
      case StackSym::R:  e.UnboxR(); // fall through
      case StackSym::C:
        e.IsTypeC(IsTypeOp::Null);
        e.Not();
        break;
      default: {
        unexpectedStackSym(sym, "emitIsset");
        break;
      }
    }
  } else {
    emitQueryMOp(i, iLast, e, QueryMOp::Isset);
  }
}

void EmitterVisitor::emitIsType(Emitter& e, IsTypeOp op) {
  if (checkIfStackEmpty("IsType")) return;

  emitConvertToCellOrLoc(e);
  switch (char sym = m_evalStack.top()) {
  case StackSym::L:
    e.IsTypeL(m_evalStack.getLoc(m_evalStack.size() - 1), op);
    break;
  case StackSym::C:
    e.IsTypeC(op);
    break;
  default:
    unexpectedStackSym(sym, "emitIsType");
  }
}

void EmitterVisitor::emitEmpty(Emitter& e) {
  if (checkIfStackEmpty("Empty*")) return;

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.EmptyL(m_evalStack.getLoc(i)); break;
      case StackSym::LN: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CN: e.EmptyN(); break;
      case StackSym::LG: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CG: e.EmptyG(); break;
      case StackSym::LS: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CS: e.EmptyS(kClsRefSlotPlaceholder); break;
      case StackSym::R:  e.UnboxR(); // fall through
      case StackSym::C:  e.Not(); break;
      default: {
        unexpectedStackSym(sym, "emitEmpty");
        break;
      }
    }
  } else {
    emitQueryMOp(i, iLast, e, QueryMOp::Empty);
  }
}

void EmitterVisitor::emitUnset(Emitter& e,
                               ExpressionPtr exp /* = ExpressionPtr() */) {
  if (checkIfStackEmpty("Unset*")) return;

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.UnsetL(m_evalStack.getLoc(i)); break;
      case StackSym::LN: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CN: e.UnsetN(); break;
      case StackSym::LG: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CG: e.UnsetG(); break;
      case StackSym::LS: // fall through
      case StackSym::CS: {
        assert(exp);

        std::ostringstream s;
        s << "Attempt to unset static property " << exp->getText();
        emitMakeUnitFatal(e, s.str().c_str());
        break;
      }
      default: {
        unexpectedStackSym(sym, "emitUnset");
        break;
      }
    }
  } else {
    auto const stackCount = emitMOp(i, iLast, e, MInstrOpts{MOpMode::Unset});
    e.UnsetM(stackCount, symToMemberKey(e, iLast, false /* allowW */));
  }
}

void EmitterVisitor::emitUnsetL(Emitter&e, Id local) {
  emitVirtualLocal(local);
  e.UnsetL(local);
}

void EmitterVisitor::emitVisitAndUnset(Emitter& e, ExpressionPtr exp) {
  visit(exp);
  emitUnset(e, exp);
}

void EmitterVisitor::emitSet(Emitter& e) {
  if (checkIfStackEmpty("Set*")) return;

  int iLast = m_evalStack.size()-2;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.SetL(m_evalStack.getLoc(i)); break;
      case StackSym::LN: emitCGetL2(e); // fall through
      case StackSym::CN: e.SetN();   break;
      case StackSym::LG: emitCGetL2(e); // fall through
      case StackSym::CG: e.SetG();   break;
      case StackSym::LS: emitCGetL2(e); // fall through
      case StackSym::CS: e.SetS(kClsRefSlotPlaceholder);   break;
      default: {
        unexpectedStackSym(sym, "emitSet");
        break;
      }
    }
  } else {
    auto const stackCount =
      emitMOp(i, iLast, e, MInstrOpts{MOpMode::Define}.rhs());
    return e.SetM(stackCount, symToMemberKey(e, iLast, true /* allowW */));
  }
}

/*
 * emitSet() requires the destination to be placed on the stack before the
 * source cell. If the destination is a local, this restriction imposed by
 * the symbolic stack abstraction goes beyond the actual requirements of HHBC,
 * where the local is represented by an immediate argument passed to SetL.
 * This function allows you to bypass this requirement.
 */
void EmitterVisitor::emitSetL(Emitter& e, Id local) {
  popEvalStack(StackSym::C);
  emitVirtualLocal(local);
  pushEvalStack(StackSym::C);
  e.SetL(local);
}

void EmitterVisitor::emitSetOp(Emitter& e, int tokenOp) {
  if (checkIfStackEmpty("SetOp*")) return;

  auto ifIntOverflow = [](SetOpOp trueVal, SetOpOp falseVal) {
    return RuntimeOption::IntsOverflowToInts ? trueVal : falseVal;
  };

  auto const op = [&] {
    switch (tokenOp) {
    case T_PLUS_EQUAL:
      return ifIntOverflow(SetOpOp::PlusEqual, SetOpOp::PlusEqualO);
    case T_MINUS_EQUAL:
      return ifIntOverflow(SetOpOp::MinusEqual, SetOpOp::MinusEqualO);
    case T_MUL_EQUAL:
      return ifIntOverflow(SetOpOp::MulEqual, SetOpOp::MulEqualO);
    case T_POW_EQUAL:    return SetOpOp::PowEqual;
    case T_DIV_EQUAL:    return SetOpOp::DivEqual;
    case T_CONCAT_EQUAL: return SetOpOp::ConcatEqual;
    case T_MOD_EQUAL:    return SetOpOp::ModEqual;
    case T_AND_EQUAL:    return SetOpOp::AndEqual;
    case T_OR_EQUAL:     return SetOpOp::OrEqual;
    case T_XOR_EQUAL:    return SetOpOp::XorEqual;
    case T_SL_EQUAL:     return SetOpOp::SlEqual;
    case T_SR_EQUAL:     return SetOpOp::SrEqual;
    default:             break;
    }
    not_reached();
  }();

  int iLast = m_evalStack.size()-2;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.SetOpL(m_evalStack.getLoc(i), op); break;
      case StackSym::LN: emitCGetL2(e); // fall through
      case StackSym::CN: e.SetOpN(op); break;
      case StackSym::LG: emitCGetL2(e); // fall through
      case StackSym::CG: e.SetOpG(op); break;
      case StackSym::LS: emitCGetL2(e); // fall through
      case StackSym::CS: e.SetOpS(op, kClsRefSlotPlaceholder); break;
      default: {
        unexpectedStackSym(sym, "emitSetOp");
        break;
      }
    }
  } else {
    auto const stackCount =
      emitMOp(i, iLast, e, MInstrOpts{MOpMode::Define}.rhs());
    e.SetOpM(stackCount, op, symToMemberKey(e, iLast, true /* allowW */));
  }
}

void EmitterVisitor::emitBind(Emitter& e) {
  if (checkIfStackEmpty("Bind*")) return;

  int iLast = m_evalStack.size()-2;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.BindL(m_evalStack.getLoc(i)); break;
      case StackSym::LN: emitCGetL2(e); // fall through
      case StackSym::CN: e.BindN();  break;
      case StackSym::LG: emitCGetL2(e); // fall through
      case StackSym::CG: e.BindG();  break;
      case StackSym::LS: emitCGetL2(e); // fall through
      case StackSym::CS: e.BindS(kClsRefSlotPlaceholder);  break;
      default: {
        unexpectedStackSym(sym, "emitBind");
        break;
      }
    }
  } else {
    auto const stackCount =
      emitMOp(i, iLast, e, MInstrOpts{MOpMode::Define}.rhs());
    e.BindM(stackCount, symToMemberKey(e, iLast, true /* allowW */));
  }
}

// See EmitterVisitor::emitSetL().
void EmitterVisitor::emitBindL(Emitter& e, Id local) {
  popEvalStack(StackSym::V);
  emitVirtualLocal(local);
  pushEvalStack(StackSym::V);
  e.BindL(local);
}

void EmitterVisitor::emitIncDec(Emitter& e, IncDecOp op) {
  if (checkIfStackEmpty("IncDec*")) return;

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L: e.IncDecL(m_evalStack.getLoc(i), op); break;
      case StackSym::LN: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CN: e.IncDecN(op); break;
      case StackSym::LG: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CG: e.IncDecG(op); break;
      case StackSym::LS: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CS: e.IncDecS(op, kClsRefSlotPlaceholder); break;
      default: {
        unexpectedStackSym(sym, "emitIncDec");
        break;
      }
    }
  } else {
    auto const stackCount =
      emitMOp(i, iLast, e, MInstrOpts{MOpMode::Define});
    e.IncDecM(stackCount, op, symToMemberKey(e, iLast, true /* allowW */));
  }
}

void EmitterVisitor::emitConvertToCell(Emitter& e) {
  emitCGet(e);
}

void EmitterVisitor::emitConvertSecondToCell(Emitter& e) {
  if (m_evalStack.size() <= 1) {
    InvariantViolation(
      "Emitter encounted an empty evaluation stack when inside "
      "the emitConvertSecondToCell() function (at offset %d)",
      m_ue.bcPos());
    return;
  }
  char sym = m_evalStack.get(m_evalStack.size() - 2);
  char symFlavor = StackSym::GetSymFlavor(sym);
  if (symFlavor == StackSym::C) {
    // do nothing
  } else if (symFlavor == StackSym::L) {
    emitCGetL2(e);
  } else {
    // emitConvertSecondToCell() should never be used for symbolic flavors
    // other than C or L
    InvariantViolation(
      "Emitter encountered an unsupported StackSym \"%s\" on "
      "the evaluation stack inside the emitConvertSecondToCell()"
      " function (at offset %d)",
      StackSym::ToString(sym).c_str(),
      m_ue.bcPos());
  }
}

void EmitterVisitor::emitConvertToCellIfVar(Emitter& e) {
  if (!m_evalStack.empty()) {
    char sym = m_evalStack.top();
    if (sym == StackSym::V) {
      emitConvertToCell(e);
    }
  }
}

void EmitterVisitor::emitConvertToCellOrLoc(Emitter& e) {
  if (m_evalStack.empty()) {
    InvariantViolation(
      "Emitter encounted an empty evaluation stack when inside "
      "the emitConvertToCellOrLoc() function (at offset %d)",
      m_ue.bcPos());
    return;
  }
  char sym = m_evalStack.top();
  if (sym == StackSym::L) {
    // If the top of stack is a loc that is not marked, do nothing
  } else {
    // Otherwise, call emitCGet to convert the top of stack to cell
    emitCGet(e);
  }
}

void EmitterVisitor::emitConvertToVar(Emitter& e) {
  emitVGet(e);
}

/*
 * Class bases are stored on the symbolic stack in a "virtual" way so
 * we can resolve them later (here) in order to properly handle php
 * evaluation order.
 *
 * For example, in:
 *
 *      $cls = 'cls';
 *      $cls::$x[0][f()] = g();
 *
 * We need to evaluate f(), then resolve $cls to an A (possibly
 * invoking an autoload handler), then evaluate g(), then do the set.
 *
 * Complex cases involve unnamed local temporaries.  For example, in:
 *
 *     ${func()}::${f()} = g();
 *
 * We'll emit code which calls func() and stashes the result in a
 * unnamed local.  Then we call f(), then we turn the unnamed local
 * into an 'A' so that autoload handlers will run after f().  Then g()
 * is evaluated and then the set happens.
 */
void EmitterVisitor::emitResolveClsBase(Emitter& e, int pos) {
  switch (m_evalStack.getClsBaseType(pos)) {
  case SymbolicStack::CLS_STRING_NAME:
    e.String(m_evalStack.getName(pos));
    emitAGet(e);
    break;
  case SymbolicStack::CLS_LATE_BOUND:
    e.LateBoundCls(kClsRefSlotPlaceholder);
    break;
  case SymbolicStack::CLS_SELF:
    e.Self(kClsRefSlotPlaceholder);
    break;
  case SymbolicStack::CLS_PARENT:
    e.Parent(kClsRefSlotPlaceholder);
    break;
  case SymbolicStack::CLS_NAMED_LOCAL: {
    int loc = m_evalStack.getLoc(pos);
    emitVirtualLocal(loc);
    emitAGet(e);
    break;
  }
  case SymbolicStack::CLS_UNNAMED_LOCAL: {
    int loc = m_evalStack.getLoc(pos);
    emitVirtualLocal(loc);
    emitAGet(e);
    emitUnsetL(e, loc);
    newFaultRegionAndFunclet(m_evalStack.getUnnamedLocStart(pos),
                             m_ue.bcPos(),
                             new UnsetUnnamedLocalThunklet(loc));
    m_curFunc->freeUnnamedLocal(loc);
    break;
  }
  case SymbolicStack::CLS_INVALID:
  default:
    assert(false);
  }

  m_evalStack.consumeBelowTop(m_evalStack.size() - pos - 1);
}

void EmitterVisitor::emitClsIfSPropBase(Emitter& e) {
  // If the eval stack is empty, then there is no work to do
  if (m_evalStack.empty()) return;

  // Scan past any values marked with the Elem, NewElem, or Prop markers
  int pos = m_evalStack.size() - 1;
  for (;;) {
    char marker = StackSym::GetMarker(m_evalStack.get(pos));
    if (marker != StackSym::E && marker != StackSym::W &&
        marker != StackSym::P && marker != StackSym::Q) {
      break;
    }
    --pos;
    if (pos < 0) {
      InvariantViolation("Emitter expected a location on the stack but none "
                         "was found (at offset %d)",
                         m_ue.bcPos());
      return;
    }
  }
  // After scanning, if we did not find a value marked with the SProp
  // marker then there is no work to do
  if (StackSym::GetMarker(m_evalStack.get(pos)) != StackSym::S) {
    return;
  }

  --pos;
  if (pos < 0) {
    InvariantViolation(
      "Emitter emitted an instruction that tries to consume "
      "a value from the stack when the stack is empty "
      "(expected symbolic flavor \"C\" or \"L\" at offset %d)",
      m_ue.bcPos());
  }

  emitResolveClsBase(e, pos);
  m_evalStack.set(m_evalStack.size() - 1,
                  m_evalStack.get(m_evalStack.size() - 1) | StackSym::M);
}

void EmitterVisitor::markElem(Emitter& /*e*/) {
  if (m_evalStack.empty()) {
    InvariantViolation("Emitter encountered an empty evaluation stack inside"
                       " the markElem function (at offset %d)",
                       m_ue.bcPos());
    return;
  }
  char sym = m_evalStack.top();
  if (sym == StackSym::C || sym == StackSym::L || sym == StackSym::T ||
      sym == StackSym::I) {
    m_evalStack.set(m_evalStack.size()-1, (sym | StackSym::E));
  } else {
    InvariantViolation(
      "Emitter encountered an unsupported StackSym \"%s\" on "
      "the evaluation stack inside the markElem function (at "
      "offset %d)",
      StackSym::ToString(sym).c_str(),
      m_ue.bcPos());
  }
}

void EmitterVisitor::markNewElem(Emitter& /*e*/) {
  m_evalStack.push(StackSym::W);
}

void EmitterVisitor::markProp(Emitter& /*e*/, PropAccessType propAccessType) {
  if (m_evalStack.empty()) {
    InvariantViolation(
      "Emitter encountered an empty evaluation stack inside "
      "the markProp function (at offset %d)",
      m_ue.bcPos());
    return;
  }
  char sym = m_evalStack.top();
  if (sym == StackSym::C || sym == StackSym::L || sym == StackSym::T) {
    m_evalStack.set(
      m_evalStack.size()-1,
      (sym | (propAccessType == PropAccessType::NullSafe
        ? StackSym::Q
        : StackSym::P
      ))
    );
  } else {
    InvariantViolation(
      "Emitter encountered an unsupported StackSym \"%s\" on "
      "the evaluation stack inside the markProp function (at "
      "offset %d)",
      StackSym::ToString(sym).c_str(),
      m_ue.bcPos());
  }
}

void EmitterVisitor::markSProp(Emitter& /*e*/) {
  if (m_evalStack.empty()) {
    InvariantViolation(
      "Emitter encountered an empty evaluation stack inside "
      "the markSProp function (at offset %d)",
      m_ue.bcPos());
    return;
  }
  char sym = m_evalStack.top();
  if (sym == StackSym::C || sym == StackSym::L) {
    m_evalStack.set(m_evalStack.size()-1, (sym | StackSym::S));
  } else {
    InvariantViolation(
      "Emitter encountered an unsupported StackSym \"%s\" on "
      "the evaluation stack inside the markSProp function "
      "(at offset %d)",
      StackSym::ToString(sym).c_str(),
      m_ue.bcPos());
  }
}

#define MARK_NAME_BODY(index, requiredStackSize)            \
  if (m_evalStack.size() < requiredStackSize) {             \
    InvariantViolation(                                     \
      "Emitter encountered an evaluation stack with %lu"    \
      " elements inside the %s function (at offset %d)",    \
      (unsigned long)m_evalStack.size(),                    \
      __FUNCTION__, m_ue.bcPos());                          \
    return;                                                 \
  }                                                         \
  char sym = m_evalStack.get(index);                        \
  if (sym == StackSym::C || sym == StackSym::L) {           \
    m_evalStack.set(index, (sym | StackSym::N));            \
  } else {                                                  \
    InvariantViolation(                                     \
      "Emitter encountered an unsupported StackSym \"%s\" " \
      "on the evaluation stack inside the %s function (at " \
      "offset %d)",                                         \
      StackSym::ToString(sym).c_str(), __FUNCTION__,        \
      m_ue.bcPos());                                        \
}

void EmitterVisitor::markName(Emitter& /*e*/) {
  int index = m_evalStack.size() - 1;
  MARK_NAME_BODY(index, 1);
}

void EmitterVisitor::markNameSecond(Emitter& /*e*/) {
  int index = m_evalStack.size() - 2;
  MARK_NAME_BODY(index, 2);
}

#undef MARK_NAME_BODY

void EmitterVisitor::markGlobalName(Emitter& /*e*/) {
  if (m_evalStack.empty()) {
    InvariantViolation(
      "Emitter encountered an empty evaluation stack inside "
      "the markGlobalName function (at offset %d)",
      m_ue.bcPos());
    return;
  }
  char sym = m_evalStack.top();
  if (sym == StackSym::C || sym == StackSym::L) {
    m_evalStack.set(m_evalStack.size()-1, (sym | StackSym::G));
  } else {
    InvariantViolation(
      "Emitter encountered an unsupported StackSym \"%s\" on "
      "the evaluation stack inside the markGlobalName function "
      "(at offset %d)",
      StackSym::ToString(sym).c_str(),
      m_ue.bcPos());
  }
}

void EmitterVisitor::emitNameString(Emitter& e, ExpressionPtr n,
                                    bool allowLiteral) {
  Variant v;
  if (n->getScalarValue(v) && v.isString()) {
    StringData* nLiteral = makeStaticString(v.toCStrRef().get());
    if (allowLiteral) {
      m_evalStack.push(StackSym::T);
    } else {
      e.String(nLiteral);
    }
    m_evalStack.setString(nLiteral);
  } else {
    visit(n);
    emitConvertToCellOrLoc(e);
  }
}

void EmitterVisitor::postponeMeth(MethodStatementPtr m, FuncEmitter* fe,
                                  bool top,
                                  ClosureUseVarVec* useVars /* = NULL */) {
  m_postponedMeths.push_back(PostponedMeth(m, fe, top, useVars));
}

void EmitterVisitor::postponeCtor(InterfaceStatementPtr is, FuncEmitter* fe) {
  m_postponedCtors.push_back(PostponedCtor(is, fe));
}

void EmitterVisitor::postponePinit(InterfaceStatementPtr is, FuncEmitter* fe,
                                   NonScalarVec* v) {
  m_postponedPinits.push_back(PostponedNonScalars(is, fe, v));
}

void EmitterVisitor::postponeSinit(InterfaceStatementPtr is, FuncEmitter* fe,
                                   NonScalarVec* v) {
  m_postponedSinits.push_back(PostponedNonScalars(is, fe, v));
}

void EmitterVisitor::postponeCinit(InterfaceStatementPtr is, FuncEmitter* fe,
                                   NonScalarVec* v) {
  m_postponedCinits.push_back(PostponedNonScalars(is, fe, v));
}

static Attr buildAttrs(ModifierExpressionPtr mod, bool isRef = false) {
  int attrs = AttrNone;
  if (isRef) {
    attrs |= AttrReference;
  }
  if (mod) {
    attrs |= mod->isPublic() ? AttrPublic :
      mod->isPrivate() ? AttrPrivate :
      mod->isProtected() ? AttrProtected : AttrNone;
    if (mod->isStatic()) {
      attrs |= AttrStatic;
    }
    if (mod->isAbstract()) {
      attrs |= AttrAbstract;
    }
    if (mod->isFinal()) {
      attrs |= AttrFinal;
    }
  }
  return Attr(attrs);
}

/*
 * <<__HipHopSpecific>> user attribute marks funcs/methods as HipHop specific
 * for reflection.
 * <<__IsFoldable>> Function has no side-effects and may be called at
 * compile time with constant input to get deterministic output.
 */
const StaticString
  s_IsFoldable("__IsFoldable"),
  s_AllowStatic("__AllowStatic"),
  s_ParamCoerceModeNull("__ParamCoerceModeNull"),
  s_ParamCoerceModeFalse("__ParamCoerceModeFalse");

static void parseUserAttributes(FuncEmitter* fe, Attr& attrs) {
  if (fe->userAttributes.count(s_IsFoldable.get())) {
    attrs = attrs | AttrIsFoldable;
  }
  if ((attrs & AttrBuiltin) &&
      fe->pce() &&
      !(attrs & AttrStatic) &&
      !fe->userAttributes.count(s_AllowStatic.get())) {
    attrs |= AttrRequiresThis;
  }
  if (fe->userAttributes.count(s_ParamCoerceModeNull.get())) {
    attrs = attrs | AttrParamCoerceModeNull;
  } else if (fe->userAttributes.count(s_ParamCoerceModeFalse.get())) {
    attrs = attrs | AttrParamCoerceModeFalse;
  }
}

static Attr
buildMethodAttrs(MethodStatementPtr meth, FuncEmitter* fe, bool /*top*/) {
  FunctionScopePtr funcScope = meth->getFunctionScope();
  ModifierExpressionPtr mod(meth->getModifiers());
  Attr attrs = buildAttrs(mod, meth->isRef());

  // Be conservative by default. HHBBC can clear it where appropriate.
  attrs |= AttrMayUseVV;

  if (funcScope->hasRefVariadicParam()) {
    attrs |= AttrVariadicByRef;
  }
  if (funcScope->isDynamicInvoke() ||
      RuntimeOption::EvalJitEnableRenameFunction) {
    attrs |= AttrInterceptable;
  }

  auto fullName = meth->getOriginalFullName();
  auto it = Option::FunctionSections.find(fullName);
  if ((it != Option::FunctionSections.end() && it->second == "hot") ||
      (RuntimeOption::EvalRandomHotFuncs &&
       (hash_string_i_unsafe(fullName.c_str(), fullName.size()) & 8))) {
    attrs = attrs | AttrHot;
  }

  if (!SystemLib::s_inited || funcScope->isSystem()) {
    // we're building systemlib. everything is unique
    if (!meth->getClassScope()) attrs |= AttrUnique | AttrPersistent;
    attrs |= AttrBuiltin;
  }

  if (Option::WholeProgram) {
    if (funcScope->isFromTrait()) {
      attrs = attrs | AttrTrait;
    }
  }

  // For closures, the MethodStatement didn't have real attributes; enforce
  // that the __invoke method is public here
  if (fe->isClosureBody) {
    assert(!(attrs & (AttrProtected | AttrPrivate)));
    attrs = attrs | AttrPublic;
  }

  // Coerce memoized methods to private. This is needed for code that uses
  // parent:: to call through to the correct underlying function
  if (meth->is(Statement::KindOfMethodStatement) && fe->isMemoizeImpl) {
    attrs = static_cast<Attr>(attrs & ~(AttrPublic | AttrProtected));
    attrs = attrs | AttrPrivate;
  }

  parseUserAttributes(fe, attrs);
  // Not supported except in __Native functions
  attrs = static_cast<Attr>(
    attrs & ~(AttrParamCoerceModeNull | AttrParamCoerceModeFalse));

  return attrs;
}

/**
 * The code below is used for both, function/method parameter type as well as
 * for function/method return type.
 */
static TypeConstraint
determine_type_constraint_from_annot(const TypeAnnotationPtr annot,
                                     bool is_return) {
  if (annot) {
    auto flags = TypeConstraint::ExtendedHint | TypeConstraint::HHType;

    // We only care about a subset of extended type constaints:
    // typevar, nullable, soft, return types.
    //
    // For everything else, we return {}. We also return {} for annotations
    // we don't know how to handle.
    if (annot->isFunction() || annot->isMixed()) {
      return {};
    }
    if (annot->isTypeAccess()) {
      flags = flags | TypeConstraint::TypeConstant;
    }
    if (annot->isTypeVar()) {
      flags = flags | TypeConstraint::TypeVar;
    }
    if (annot->isNullable()) {
      flags = flags | TypeConstraint::Nullable;
    }
    if (annot->isSoft()) {
      flags = flags | TypeConstraint::Soft;
    }
    if (!is_return &&
        (flags == (TypeConstraint::ExtendedHint | TypeConstraint::HHType))) {
      return {};
    }

    auto strippedName = annot->stripNullable().stripSoft().vanillaName();

    return TypeConstraint{
      makeStaticString(strippedName),
      flags
    };
  }

  return {};
}

static TypeConstraint
determine_type_constraint(const ParameterExpressionPtr& par) {
  if (par->hasTypeHint()) {
    auto ce = dynamic_pointer_cast<ConstantExpression>(par->defaultValue());
    auto flags = TypeConstraint::NoFlags;
    if (ce && ce->isNull()) {
      flags = flags|TypeConstraint::Nullable;
    }
    if (par->hhType()) {
      flags = flags|TypeConstraint::HHType;
    }
    return TypeConstraint{
      makeStaticString(par->getOriginalTypeHint()),
      flags
    };
  }

  return determine_type_constraint_from_annot(par->annotation(), false);
}

void EmitterVisitor::emitPostponedMeths() {
  std::vector<FuncEmitter*> top_fes;
  while (!m_postponedMeths.empty()) {
    assert(m_evalStack.m_actualStackHighWaterPtr == nullptr);
    PostponedMeth& p = m_postponedMeths.front();
    MethodStatementPtr meth = p.m_meth;
    FuncEmitter* fe = p.m_fe;

    ITRACE(1, "Emitting postponed method {}\n", meth->getOriginalFullName());
    Trace::Indent indent;

    if (!fe) {
      assert(p.m_top);
      const StringData* methName = makeStaticString(meth->getOriginalName());
      fe = new FuncEmitter(m_ue, -1, -1, methName);
      auto oldFunc = m_topMethodEmitted.find(meth->getOriginalName());
      if (oldFunc != m_topMethodEmitted.end()) {
        throw IncludeTimeFatalException(
          meth,
          "Cannot redeclare %s() (previously declared in %s:%d)",
          meth->getOriginalName().c_str(),
          oldFunc->second->ue().m_filepath->data(),
          oldFunc->second->getLocation().second);
      }
      m_topMethodEmitted.emplace(meth->getOriginalName(), fe);

      p.m_fe = fe;
      top_fes.push_back(fe);
    }

    auto funcScope = meth->getFunctionScope();
    m_curFunc = fe;
    fe->isAsync = funcScope->isAsync();
    fe->isGenerator = funcScope->isGenerator();

    if (fe->isAsync && !fe->isGenerator && meth->retTypeAnnotation()) {
      auto rta = meth->retTypeAnnotation();
      auto nTypeArgs = rta->numTypeArgs();
      if (!rta->isAwaitable() && !rta->isWaitHandle()) {
        if (fe->isClosureBody) {
          throw IncludeTimeFatalException(
            meth,
            "Return type hint for async closure must be awaitable"
          );
        } else {
          throw IncludeTimeFatalException(
            meth,
            "Return type hint for async %s %s() must be awaitable",
            meth->getClassScope() ? "method" : "function",
            meth->getOriginalFullName().c_str()
          );
        }
      }
      if (nTypeArgs >= 2) {
        throw IncludeTimeFatalException(
          meth,
          "Awaitable interface expects 1 type argument, %d given",
          nTypeArgs);
      }
    }

    if (funcScope->userAttributes().count("__Memoize") &&
        !funcScope->isAbstract()) {
      auto const originalName = fe->name;
      auto const rewrittenName = Func::genMemoizeImplName(originalName);

      FuncEmitter* memoizeFe = nullptr;
      if (meth->is(Statement::KindOfFunctionStatement)) {
        if (!p.m_top) {
          throw IncludeTimeFatalException(meth,
            "<<__Memoize>> cannot be applied to closures and inline functions");
        }

        memoizeFe = new FuncEmitter(m_ue, -1, -1, originalName);
        fe->name = rewrittenName;
        top_fes.push_back(memoizeFe);
      } else {
        // Rename the method and create a new method with the original name
        fe->pce()->renameMethod(originalName, rewrittenName);
        memoizeFe = m_ue.newMethodEmitter(originalName, fe->pce());
        bool added UNUSED = fe->pce()->addMethod(memoizeFe);
        assert(added);
      }

      // Emit the new method that handles the memoization
      m_curFunc = memoizeFe;
      m_curFunc->isMemoizeWrapper = true;
      addMemoizeProp(meth);
      emitMethodMetadata(meth, p.m_closureUseVars, p.m_top);
      emitMemoizeMethod(meth, rewrittenName);

      // Switch back to the original method and mark it as a memoize
      // implementation
      m_curFunc = fe;
      m_curFunc->isMemoizeImpl = true;
    }

    if (funcScope->isNative()) {
      auto const attr = bindNativeFunc(meth, fe, false);
      if (attr & (AttrReadsCallerFrame | AttrWritesCallerFrame)) {
        // If this is a builtin which may access the caller's frame, generate a
        // dynamic call wrapper function. Dynamic calls to the builtin will be
        // routed to this wrapper instead.  This function is identical to the
        // normal builtin function, except it includes the VarEnvDynCall opcode.
        if (!meth->is(Statement::KindOfFunctionStatement) ||
            !p.m_top || fe->pce()) {
          throw IncludeTimeFatalException(
            meth,
            "ReadsCallerFrame or WritesCallerFrame can only "
            "be applied to top-level functions"
          );
        }
        auto const rewrittenName = makeStaticString(
          folly::sformat("{}$dyncall_wrapper", fe->name->data()));
        auto stub = m_ue.newFuncEmitter(rewrittenName);
        m_curFunc = stub;
        SCOPE_EXIT { m_curFunc = fe; };
        bindNativeFunc(meth, stub, true);
        assert(stub->id() != kInvalidId);
        fe->dynCallWrapperId = stub->id();
      }
    } else {
      emitMethodMetadata(meth, p.m_closureUseVars, p.m_top);
      emitMethod(meth);
    }

    if (fe->isClosureBody) {
      TypedValue uninit;
      tvWriteUninit(&uninit);
      for (auto& sv : m_curFunc->staticVars) {
        auto const str = makeStaticString(
          folly::format("86static_{}", sv.name->data()).str());
        fe->pce()->addProperty(str, AttrPrivate, staticEmptyString(), nullptr,
                               &uninit, RepoAuthType{});
      }
    }

    delete p.m_closureUseVars;
    m_postponedMeths.pop_front();
  }

  for (size_t i = 0; i < top_fes.size(); i++) {
    m_ue.appendTopEmitter(top_fes[i]);
  }
}

void EmitterVisitor::bindUserAttributes(MethodStatementPtr meth,
                                        FuncEmitter *fe) {
  auto const& userAttrs = meth->getFunctionScope()->userAttributes();
  for (auto& attr : userAttrs) {
    const StringData* uaName = makeStaticString(attr.first);
    ExpressionPtr uaValue = attr.second;
    assert(uaValue);
    assert(uaValue->isScalar());
    TypedValue tv;
    initScalar(tv, uaValue);
    fe->userAttributes[uaName] = tv;
  }
}

const StaticString s_Void("HH\\void");
const char* attr_Deprecated = "__Deprecated";
const StaticString s_attr_Deprecated(attr_Deprecated);

Attr EmitterVisitor::bindNativeFunc(MethodStatementPtr meth,
                                    FuncEmitter *fe,
                                    bool dynCallWrapper) {
  if (SystemLib::s_inited &&
      !(Option::WholeProgram && meth->isSystem())) {
    throw IncludeTimeFatalException(meth,
          "Native functions/methods may only be defined in systemlib");
  }

  auto modifiers = meth->getModifiers();
  bindUserAttributes(meth, fe);

  Attr attributes = AttrBuiltin | AttrUnique | AttrPersistent;
  if (meth->isRef()) {
    attributes = attributes | AttrReference;
  }
  auto pce = fe->pce();
  if (pce) {
    if (modifiers->isStatic()) {
      attributes = attributes | AttrStatic;
    }
    if (modifiers->isFinal()) {
      attributes = attributes | AttrFinal;
    }
    if (modifiers->isAbstract()) {
      attributes = attributes | AttrAbstract;
    }
    if (modifiers->isPrivate()) {
      attributes = attributes | AttrPrivate;
    } else {
      attributes = attributes | (modifiers->isProtected()
                              ? AttrProtected : AttrPublic);
    }
  }
  parseUserAttributes(fe, attributes);
  if (!(attributes & (AttrParamCoerceModeFalse | AttrParamCoerceModeNull))) {
    attributes = attributes | AttrParamCoerceModeNull;
  }

  fe->setLocation(meth->line0(), meth->line1());
  fe->docComment = makeStaticString(
    Option::GenerateDocComments ? meth->getDocComment().c_str() : ""
  );
  auto retType = meth->retTypeAnnotation();
  assert(retType ||
         meth->isNamed("__construct") ||
         meth->isNamed("__destruct"));
  fe->hniReturnType = retType ? retType->dataType() : KindOfNull;
  fe->retUserType = makeStaticString(meth->getReturnTypeConstraint());

  FunctionScopePtr funcScope = meth->getFunctionScope();
  const char *funcname  = funcScope->getScopeName().c_str();
  const char *classname = pce ? pce->name()->data() : nullptr;
  auto const& info = Native::GetBuiltinFunction(funcname, classname,
                                                modifiers->isStatic());

  if (!classname && (
        !strcasecmp(funcname, "fb_call_user_func_safe") ||
        !strcasecmp(funcname, "fb_call_user_func_safe_return") ||
        !strcasecmp(funcname, "fb_call_user_func_array_safe"))) {
    // Legacy optimization functions
    funcScope->setOptFunction(hphp_opt_fb_call_user_func);
  }

  int nativeAttrs = fe->parseNativeAttributes(attributes);
  BuiltinFunction bif = nullptr, nif = nullptr;
  Native::getFunctionPointers(info, nativeAttrs, bif, nif);
  if (nif && !(nativeAttrs & Native::AttrZendCompat)) {
    if (retType) {
      fe->retTypeConstraint =
        determine_type_constraint_from_annot(retType, true);
    } else {
      fe->retTypeConstraint = TypeConstraint {
        s_Void.get(),
        TypeConstraint::ExtendedHint | TypeConstraint::HHType
      };
    }
  }

  Emitter e(meth, m_ue, *this);
  FuncFinisher ff(this, e, fe, 0);
  Label topOfBody(e, Label::NoEntryNopFlag{});

  Offset base = m_ue.bcPos();

  if (meth->getFunctionScope()->userAttributes().count(attr_Deprecated)) {
    emitDeprecationWarning(e, meth);
  }

  if (!(attributes & (AttrReadsCallerFrame | AttrWritesCallerFrame))) {
    if (meth->getFunctionScope()->isDynamicInvoke() ||
        RuntimeOption::EvalJitEnableRenameFunction) {
      attributes |= AttrInterceptable;
    }
  }

  fe->setBuiltinFunc(attributes, base);
  fillFuncEmitterParams(fe, meth->getParams(), true);

  if (dynCallWrapper) e.VarEnvDynCall();

  if (nativeAttrs & Native::AttrOpCodeImpl) {
    ff.setStackPad(emitNativeOpCodeImpl(meth, funcname, classname, fe));
  } else {
    e.NativeImpl();
  }
  emitMethodDVInitializers(e, fe, meth, topOfBody);
  return attributes;
}

void EmitterVisitor::emitMethodMetadata(MethodStatementPtr meth,
                                        ClosureUseVarVec* useVars,
                                        bool top) {
  FuncEmitter* fe = m_curFunc;
  bindUserAttributes(meth, fe);

  // assign ids to parameters (all methods)
  int numParam = meth->getParams() ? meth->getParams()->getCount() : 0;
  for (int i = 0; i < numParam; i++) {
    auto par =
      static_pointer_cast<ParameterExpression>((*meth->getParams())[i]);
    fe->allocVarId(makeStaticString(par->getName()));
  }

  // assign ids to 0Closure and use parameters (closures)
  if (fe->isClosureBody) {
    fe->allocVarId(makeStaticString("0Closure"));

    for (auto& useVar : *useVars) {
      fe->allocVarId(useVar.first);
    }
  }

  // assign id to 86metadata local representing frame metadata
  if (meth->mayCallSetFrameMetadata()) {
    fe->allocVarId(makeStaticString("86metadata"));
  }

  // assign ids to local variables
  if (!fe->isMemoizeWrapper) {
    assignLocalVariableIds(meth->getFunctionScope());
  }

  // add parameter info
  fillFuncEmitterParams(fe, meth->getParams(),
                        meth->getFunctionScope()->isParamCoerceMode());

  // copy declared return type (hack)
  fe->retUserType = makeStaticString(meth->getReturnTypeConstraint());

  auto annot = meth->retTypeAnnotation();
  // For a non-generator async function with a return annotation of the form
  // "Awaitable<T>", we set m_retTypeConstraint to T. For all other async
  // functions, we leave m_retTypeConstraint empty.
  if (annot && fe->isAsync && !fe->isGenerator) {
    // Semantic checks ensure that the return annotation is "Awaitable" or
    // "WaitHandle" and that it has at most one type parameter
    assert(annot->isAwaitable() || annot->isWaitHandle());
    assert(annot->numTypeArgs() <= 1);
    bool isSoft = annot->isSoft();
    // If annot was "Awaitable" with no type args, getTypeArg() will return an
    // empty annotation
    annot = annot->getTypeArg(0);
    // If the original annotation was soft, make sure we preserve the softness
    if (annot && isSoft) annot->setSoft();
  }
  // Ideally we should handle the void case in TypeConstraint::check. This
  // should however get done in a different diff, since it could impact
  // perf in a negative way (#3145038)
  if (annot && !annot->isVoid()) {
    fe->retTypeConstraint = determine_type_constraint_from_annot(annot, true);
  }

  // add the original filename for flattened traits
  auto const originalFilename = meth->getOriginalFilename();
  if (!originalFilename.empty()) {
    fe->originalFilename = makeStaticString(originalFilename);
  }

  StringData* methDoc = Option::GenerateDocComments ?
    makeStaticString(meth->getDocComment()) : staticEmptyString();

  fe->init(meth->line0(),
           meth->line1(),
           m_ue.bcPos(),
           buildMethodAttrs(meth, fe, top),
           top,
           methDoc);

  if (meth->getFunctionScope()->needsFinallyLocals()) {
    assignFinallyVariableIds();
  }
}

void EmitterVisitor::fillFuncEmitterParams(FuncEmitter* fe,
                                           ExpressionListPtr params,
                                           bool coerce_params /*= false */) {
  int numParam = params ? params->getCount() : 0;
  for (int i = 0; i < numParam; i++) {
    auto par = static_pointer_cast<ParameterExpression>((*params)[i]);
    StringData* parName = makeStaticString(par->getName());

    FuncEmitter::ParamInfo pi;
    auto const typeConstraint = determine_type_constraint(par);
    if (typeConstraint.hasConstraint()) {
      pi.typeConstraint = typeConstraint;
    }
    if (coerce_params) {
      if (auto const typeAnnotation = par->annotation()) {
        pi.builtinType = typeAnnotation->dataType();
      }
    }

    if (par->hasUserType()) {
      pi.userType = makeStaticString(par->getUserTypeHint());
    }

    // Store info about the default value if there is one.
    if (par->isOptional()) {
      const StringData* phpCode;
      ExpressionPtr vNode = par->defaultValue();
      if (vNode->isScalar()) {
        TypedValue dv;
        initScalar(dv, vNode);
        pi.defaultValue = dv;

        std::string orig = vNode->getComment();
        if (orig.empty()) {
          // Simple case: it's a scalar value so we just serialize it
          VariableSerializer vs(VariableSerializer::Type::PHPOutput);
          String result = vs.serialize(tvAsCVarRef(&dv), true);
          phpCode = makeStaticString(result.get());
        } else {
          // This was optimized from a Constant, or ClassConstant
          // use the original string
          phpCode = makeStaticString(orig);
        }
      } else {
        // Non-scalar, so we have to output PHP from the AST node
        std::ostringstream os;
        CodeGenerator cg(&os, CodeGenerator::PickledPHP);
        auto ar = std::make_shared<AnalysisResult>();
        vNode->outputPHP(cg, ar);
        phpCode = makeStaticString(os.str());
      }
      pi.phpCode = phpCode;
    }

    auto paramUserAttrs =
      dynamic_pointer_cast<ExpressionList>(par->userAttributeList());
    if (paramUserAttrs) {
      for (int j = 0; j < paramUserAttrs->getCount(); ++j) {
        auto a = dynamic_pointer_cast<UserAttribute>((*paramUserAttrs)[j]);
        StringData* uaName = makeStaticString(a->getName());
        ExpressionPtr uaValue = a->getExp();
        assert(uaValue);
        assert(uaValue->isScalar());
        TypedValue tv;
        initScalar(tv, uaValue);
        pi.userAttributes[uaName] = tv;
      }
    }

    pi.byRef = par->isRef();
    pi.variadic = par->isVariadic();
    fe->appendParam(parName, pi);
  }
}

void EmitterVisitor::emitMethodPrologue(Emitter& e, MethodStatementPtr meth) {
  FunctionScopePtr funcScope = meth->getFunctionScope();

  if (!m_curFunc->isMemoizeWrapper &&
      funcScope->needsLocalThis() && !funcScope->isStatic()) {
    assert(!m_curFunc->top);
    static const StringData* thisStr = makeStaticString("this");
    Id thisId = m_curFunc->lookupVarId(thisStr);
    emitVirtualLocal(thisId);
    e.InitThisLoc(thisId);
  }

  for (uint32_t i = 0; i < m_curFunc->params.size(); i++) {
    auto const &param = m_curFunc->params[i];
    if (param.isVariadic()) continue;
    auto const& tc = param.typeConstraint;
    if (!tc.hasConstraint()) continue;
    emitVirtualLocal(i);
    e.VerifyParamType(i);
  }

  if (funcScope->isAbstract()) {
    std::ostringstream s;
    s << "Cannot call abstract method " << meth->getOriginalFullName() << "()";
    emitMakeUnitFatal(e, s.str().c_str(), FatalOp::RuntimeOmitFrame);
  }
}

void EmitterVisitor::emitDeprecationWarning(Emitter& e,
                                            MethodStatementPtr meth) {
  auto funcScope = meth->getFunctionScope();

  auto userAttributes DEBUG_ONLY = funcScope->userAttributes();
  assert(userAttributes.find(attr_Deprecated) != userAttributes.end());

  // Include the message from <<__Deprecated('<message>')>> in the warning
  auto deprArgs = funcScope->getUserAttributeParams(attr_Deprecated);
  auto deprMessage = deprArgs.empty()
    ? s_is_deprecated.data()
    : deprArgs.front()->getString();

  // how often to display the warning (1 / rate)
  auto rate = deprArgs.size() > 1 ? deprArgs[1]->getLiteralInteger() : 1;
  if (rate <= 0) {
    // deprecation warnings disabled
    return;
  }

  { // preface the message with the name of the offending function
    auto funcName = funcScope->getScopeName();
    BlockScopeRawPtr b = funcScope->getOuterScope();
    if (b && b->is(BlockScope::ClassScope)) {
      auto clsScope = dynamic_pointer_cast<ClassScope>(b);
      if (clsScope->isTrait()) {
        e.Self(kClsRefSlotPlaceholder);
        e.ClsRefName(kClsRefSlotPlaceholder);
        e.String(makeStaticString("::" + funcName + ": " + deprMessage));
        e.Concat();
      } else {
        e.String(makeStaticString(
                   clsScope->getScopeName() + "::" + funcName
                   + ": " + deprMessage));
      }
    } else {
      e.String(makeStaticString(funcName + ": " + deprMessage));
    }
  }

  e.Int(rate);
  e.Int((funcScope->isSystem() || funcScope->isNative())
        ? (int)ErrorMode::PHP_DEPRECATED : (int)ErrorMode::USER_DEPRECATED);
  e.FCallBuiltin(3, 3, s_trigger_sampled_error.get());
  emitPop(e);
}

void EmitterVisitor::emitMethod(MethodStatementPtr meth) {
  auto region = createRegion(meth, Region::Kind::FuncBody);
  enterRegion(region);
  SCOPE_EXIT { leaveRegion(region); };

  Emitter e(meth, m_ue, *this);
  FuncFinisher ff(this, e, m_curFunc);
  Label topOfBody(e, Label::NoEntryNopFlag{});
  emitMethodPrologue(e, meth);

  if (!m_curFunc->isMemoizeImpl &&
      meth->getFunctionScope()->userAttributes().count(attr_Deprecated)) {
    emitDeprecationWarning(e, meth);
  }

  // emit code to create generator object
  if (m_curFunc->isGenerator) {
    e.CreateCont();
    e.PopC();
  }

  // emit method body
  visit(meth->getStmts());
  assert(m_evalStack.size() == 0);
  assert(m_evalStack.clsRefSlotStackEmpty());

  // if the current position is reachable, emit code to return null
  if (currentPositionIsReachable()) {
    auto r = meth->getRange();
    r.line0 = r.line1;
    r.char0 = r.char1 - 1;
    e.setTempLocation(r);
    e.Null();
    if (shouldEmitVerifyRetType()) {
      e.VerifyRetTypeC();
    }
    e.RetC();
    e.setTempLocation(OptLocation());
  }

  if (!m_curFunc->isMemoizeImpl) {
    emitMethodDVInitializers(e, m_curFunc, meth, topOfBody);
  }
}

void EmitterVisitor::emitMethodDVInitializers(Emitter& e,
                                              FuncEmitter* fe,
                                              MethodStatementPtr& meth,
                                              Label& topOfBody) {
  bool hasOptional = false;
  ExpressionListPtr params = meth->getParams();
  int numParam = params ? params->getCount() : 0;
  Offset skippedDefault = kInvalidOffset;

  auto hasRuntimeTypeCheck = [] (const FuncEmitter::ParamInfo& pi) {
    if (pi.byRef) return false;
    if (pi.builtinType) return true;
    return pi.typeConstraint.isNullable() &&
      pi.typeConstraint.underlyingDataType();
  };

  for (int i = 0; i < numParam; i++) {
    auto par = static_pointer_cast<ParameterExpression>((*params)[i]);
    if (par->isOptional()) {
      hasOptional = true;
      Label entryPoint(e);
      if (fe->isNative) {
        auto const& pi = fe->params[i];
        if (pi.defaultValue.m_type == KindOfNull &&
            !hasRuntimeTypeCheck(pi)) {

          // builtins with untyped, default null params expect to
          // get uninits, rather than nulls.
          if (skippedDefault == kInvalidOffset) {
            e.Nop();
            skippedDefault = entryPoint.getAbsoluteOffset();
          }
          m_curFunc->params[i].funcletOff = skippedDefault;
          continue;
        }
      }
      emitVirtualLocal(i);
      visit(par->defaultValue());
      emitCGet(e);
      emitSet(e);
      e.PopC();
      m_curFunc->params[i].funcletOff = entryPoint.getAbsoluteOffset();
      skippedDefault = kInvalidOffset;
    }
  }
  if (hasOptional) e.JmpNS(topOfBody);
}

void EmitterVisitor::addMemoizeProp(MethodStatementPtr meth) {
  assert(m_curFunc->isMemoizeWrapper);

  /*
   * A memoize cache can take a variety of forms, depending on the function's
   * parameter count, and what type of function it is. If the function does not
   * take any parameters (and thus can only cache a single value), the cache is
   * simply storage for a single value. If the function takes parameters, then
   * the cache is a set of nested dicts, N levels depth for the N parameters. At
   * level M, the key is the function's Mth parameter, converted into a memo
   * key. At the end of the dict chain, the actual value is stored.
   *
   * If the cache stores a single value, Null is used to represent the lack of a
   * value. This poses a problem for functions which actually can return
   * Null. In that case, such a cache is called "guarded", and has an additional
   * boolean associated with it. If the value is set to Null, and the guard is
   * true, the cached value is actually Null, otherwise its not set. If we can't
   * determine whether will return Null or not, we'll assume it can, and let
   * HHBBC attempt to infer the return type and optimize away the guard.
   *
   * If the function is a free function, the cache is stored as a static local
   * scoped to that function. If the function is a static method, the cache is
   * stored as a static property on the function's class. One static property
   * per cache. If the function is a non-static method, the cache is stored as a
   * per-instance property on the object. However, to avoid adding an excessive
   * number of individual properties to every instance, all memoize caches for
   * that instance share the same property. This means that even if the
   * functions do not take any parameters, they will use the above described
   * dict storage. A synthetic integer index is used as the first key to address
   * the cache for a specific function. An exception is if an object only has
   * one memoized function. In that case, it will use a single property (without
   * synthetic index) for the cache storage.
   */

  auto pce = m_curFunc->pce();
  auto classScope = meth->getClassScope();
  auto funcScope = meth->getFunctionScope();

  // Statically determine if this function can potentially return Null. Async
  // functions cannot return null, nor do ones with type-constraints that don't
  // admit Null (and return type-constraint enforcement is enabled).
  auto const cantReturnNull =
    funcScope->isAsync() ||
    (m_curFunc->retTypeConstraint.hasConstraint() &&
     !m_curFunc->retTypeConstraint.isSoft() &&
     !m_curFunc->retTypeConstraint.isNullable() &&
     RuntimeOption::EvalCheckReturnTypeHints >= 3);

  if (meth->is(Statement::KindOfFunctionStatement)) {
    // Functions use statics within themselves. So all we need to do here is set
    // the name and the guard name if necessary.
    m_curFunc->memoizePropName = makeStaticString("static$memoize_cache");
    if ((!meth->getParams() || meth->getParams()->getCount() == 0)
        && !cantReturnNull) {
      m_curFunc->memoizeGuardPropName =
        makeStaticString("static$memoize_cache$guard");
    }
    return;
  }

  // Determine if we need to use a shared cache. We do if this function isn't
  // static and the class it belongs to has more than one non-static memoized
  // function.
  bool useSharedProp = false;
  if (!funcScope->isStatic()) {
    // If we're already allocated a memoize cache key, we know immediately we
    // need to use a shared cache. Otherwise we could be the first, so iterate
    // over all of the class' functions looking for a non-static memoized
    // function that isn't the current one.
    if (!pce->areMemoizeCacheKeysAllocated()) {
      for (auto const& f : classScope->allFunctions()) {
        if (!f->isStatic() && f->userAttributes().count("__Memoize") &&
            !f->isAbstract() && f != funcScope) {
          useSharedProp = true;
          break;
        }
      }
    } else {
      useSharedProp = true;
    }
  }

  if (useSharedProp) {
    m_curFunc->hasMemoizeSharedProp = true;
    m_curFunc->memoizeSharedPropIndex = pce->getNextMemoizeCacheKey();
  }

  // Even if the cache isn't shared but if the function isn't static, use the
  // $shared prefix for its name because there's PHP code which assumes that.
  auto const propNameBase = funcScope->isStatic()
    ? toLower(funcScope->getScopeName())
    : "$shared";

  // The prop definition in traits conflicts with the definition in a class
  // so make a different prop for each trait
  std::string traitNamePart;
  if (classScope && classScope->isTrait()) {
    traitNamePart = toLower(classScope->getScopeName());
    // the backslash comes from namespaces. @jan thought that would cause
    // issues, so use $ instead
    for (char &c: traitNamePart) {
      c = (c == '\\' ? '$' : c);
    }
    traitNamePart += "$";
  }

  auto const makeProp = [&](const std::string& baseName, TypedValue val) {
    auto const name = makeStaticString(
      folly::sformat("{}${}{}", propNameBase, traitNamePart, baseName)
    );
    Attr attrs = AttrPrivate | AttrNoSerialize;
    attrs = attrs | (funcScope->isStatic() ? AttrStatic : AttrNone);
    pce->addProperty(
      name, attrs, staticEmptyString(), nullptr, &val, RepoAuthType{}
    );
    return name;
  };

  // As described above: If the cache is shared, or if the function has
  // parameters, use the nested dict scheme, otherwise use a single value.
  m_curFunc->memoizePropName = [&]{
    // NB: If you change the naming format, make sure the reflection code is
    // updated to properly recognize memo cache properties.
    if (useSharedProp ||
        (meth->getParams() && meth->getParams()->getCount() > 0)) {
      return makeProp(
        "multi$memoize_cache",
        make_tv<KindOfPersistentDict>(staticEmptyDictArray())
      );
    }

    // Use a single value for functions which cannot possibly return null, and a
    // guarded single value otherwise.
    if (cantReturnNull) {
      return makeProp("single$memoize_cache", make_tv<KindOfNull>());
    }

    m_curFunc->memoizeGuardPropName =
      makeProp(
        "guarded_single$memoize_cache$guard",
        make_tv<KindOfBoolean>(false)
      );
    return makeProp("guarded_single$memoize_cache", make_tv<KindOfNull>());
  }();
}

void EmitterVisitor::emitMemoizeMethod(MethodStatementPtr meth,
                                       const StringData* methName) {
  assert(m_curFunc->isMemoizeWrapper);

  if (meth->getFunctionScope()->isRefReturn()) {
    throw IncludeTimeFatalException(meth,
      "<<__Memoize>> cannot be used on functions that return by reference");
  }
  if (meth->getFunctionScope()->allowsVariableArguments()) {
    throw IncludeTimeFatalException(meth,
      "<<__Memoize>> cannot be used on functions with variable arguments");
  }

  auto classScope = meth->getClassScope();
  if (classScope && classScope->isInterface()) {
    throw IncludeTimeFatalException(meth,
      "<<__Memoize>> cannot be used in interfaces");
  }

  bool isFunc = meth->is(Statement::KindOfFunctionStatement);
  int numParams = m_curFunc->params.size();

  auto region = createRegion(meth, Region::Kind::FuncBody);
  enterRegion(region);
  SCOPE_EXIT { leaveRegion(region); };

  Emitter e(meth, m_ue, *this);
  FuncFinisher ff(this, e, m_curFunc);

  Label topOfBody(e, Label::NoEntryNopFlag{});
  Label cacheMiss;
  Label cacheMissNoClean;

  emitMethodPrologue(e, meth);
  // We want to emit the depreciation warning on every call, even if its a cache
  // hit.
  if (meth->getFunctionScope()->userAttributes().count(attr_Deprecated)) {
    emitDeprecationWarning(e, meth);
  }

  // Function start
  int staticLocalID = 0;
  int staticGuardLocalID = 0;
  if (isFunc) {
    // Free function, so load the cache from its static local.
    staticLocalID = m_curFunc->allocUnnamedLocal();
    emitVirtualLocal(staticLocalID);
    if (numParams == 0) {
      if (m_curFunc->memoizeGuardPropName) {
        staticGuardLocalID = m_curFunc->allocUnnamedLocal();
        emitVirtualLocal(staticGuardLocalID);
        e.False();
        e.StaticLocInit(staticGuardLocalID, m_curFunc->memoizeGuardPropName);
      }
      e.Null();
    } else {
      e.Dict(staticEmptyDictArray());
    }
    e.StaticLocInit(staticLocalID, m_curFunc->memoizePropName);
  } else if (!meth->getFunctionScope()->isStatic()) {
    e.CheckThis();
  }

  // If needed, store all the memo keys into a set of contiguous locals which
  // the MemoGet/MemoSet opcodes can read from:

  uint32_t keysBegin = 0;
  uint32_t keyCount = 0;
  if (m_curFunc->hasMemoizeSharedProp) {
    // If this is a shared cache, the first key is always the index, which is
    // always statically known.
    keysBegin = m_curFunc->allocUnnamedLocal();
    ++keyCount;
    emitVirtualLocal(keysBegin);
    e.Int(m_curFunc->memoizeSharedPropIndex);
    emitSet(e);
    emitPop(e);
  }

  // Serialize the params into memo keys and store them in unnamed locals:
  for (int i = 0; i < numParams; i++) {
    if (m_curFunc->params[i].byRef) {
      throw IncludeTimeFatalException(
        meth,
        "<<__Memoize>> cannot be used on functions with args passed by "
        "reference"
      );
    }

    auto const local = m_curFunc->allocUnnamedLocal();
    if (!keyCount) keysBegin = local;
    always_assert(local == keysBegin + keyCount);
    ++keyCount;

    emitVirtualLocal(local);
    emitVirtualLocal(i);
    e.GetMemoKeyL(i);
    emitSet(e);
    emitPop(e);
  }

  auto const emitBase = [&] (bool guard) {
    if (meth->is(Statement::KindOfFunctionStatement)) {
      emitVirtualLocal(guard ? staticGuardLocalID : staticLocalID);
    } else if (meth->getFunctionScope()->isStatic()) {
      m_evalStack.push(StackSym::K);
      if (!classScope || classScope->isTrait() ||
          meth->getFunctionScope()->isClosure()) {
        m_evalStack.setClsBaseType(SymbolicStack::CLS_SELF);
      } else {
        m_evalStack.setClsBaseType(SymbolicStack::CLS_STRING_NAME);
        m_evalStack.setString(m_curFunc->pce()->name());
      }
      e.String(
        guard ? m_curFunc->memoizeGuardPropName : m_curFunc->memoizePropName
      );
      markSProp(e);
    } else {
      m_evalStack.push(StackSym::H);
      m_evalStack.setKnownCls(m_curFunc->pce()->name(), false);
      m_evalStack.push(StackSym::T);
      m_evalStack.setString(
        guard ? m_curFunc->memoizeGuardPropName : m_curFunc->memoizePropName
      );
      markProp(e, PropAccessType::Normal);
    }
  };

  auto const emitMemoAccess = [&](bool read){
    always_assert(keyCount > 0);
    always_assert(keysBegin + keyCount <= m_curFunc->numLocals());

    emitClsIfSPropBase(e);
    int iLast = m_evalStack.size() - (read ? 1 : 2);
    int i = scanStackForLocation(iLast);
    if (read) {
      auto const count = emitMOp(i, iLast, e, MInstrOpts{MOpMode::Warn}, true);
      always_assert(keysBegin + keyCount <= m_curFunc->numLocals());
      e.MemoGet(count, LocalRange{ keysBegin, keyCount - 1 });
    } else {
      auto const count =
        emitMOp(i, iLast, e, MInstrOpts{MOpMode::Define}.rhs(), true);
      always_assert(keysBegin + keyCount <= m_curFunc->numLocals());
      e.MemoSet(count, LocalRange{ keysBegin, keyCount - 1});
    }
  };

  // If we're using a nested dict cache, emit a MemoGet and check if the
  // returned value is Uninit or not. If we're using a single value cache,
  // access its stored value and check if its null or not. If we get a value,
  // return it. Note that even if the cache is guarded, we check if the value is
  // Null first. If the value is non-null, then the guard must be true, so if
  // Null isn't common, its a win to always check the value first.
  if (keyCount > 0) {
    emitBase(false);
    emitMemoAccess(true);
    e.IsUninit();
    e.JmpNZ(cacheMiss);
    e.CGetCUNop();
  } else {
    if (m_curFunc->memoizeGuardPropName) {
      // Emit a check for the memoize function's return-type being Null. If it
      // is, then the cached value will always be Null and we can skip the value
      // check. HHBBC will optimize this check away in either case.
      e.Null();
      e.IsMemoType();
      e.JmpNZ(cacheMissNoClean);
    }
    emitBase(false);
    emitCGet(e);
    e.Dup();
    emitIsType(e, IsTypeOp::Null);
    e.JmpNZ(cacheMiss);
  }
  e.RetC();

  // The value isn't there. First clean the Null or Uninit value off the stack.
  cacheMiss.set(e);
  if (keyCount > 0) {
    e.UGetCUNop();
    e.PopU();
  } else {
    e.PopC();
  }

  cacheMissNoClean.set(e);
  if (m_curFunc->memoizeGuardPropName) {
    // Either we optimized away the cached value check all together, or we did
    // the check and it contained Null. Either way, we need to check the
    // guard. If its true, the stored value actually is Null, otherwise we need
    // to call the function.
    //
    // However, if the memoized function cannot possibly return Null, there's no
    // need to check the guard. In that case, we know the value isn't
    // present. HHBBC will optimize away the MaybeMemoType check.
    Label guardNotSet;
    e.Null();
    e.MaybeMemoType();
    e.JmpZ(guardNotSet);
    emitBase(true);
    emitCGet(e);
    e.JmpZ(guardNotSet);
    e.Null();
    e.RetC();
    guardNotSet.set(e);
  }

  auto const emitImplCall = [&]{
    auto fpiStart = m_ue.bcPos();
    if (isFunc) {
      e.FPushFuncD(numParams, methName);
    } else if (meth->getFunctionScope()->isStatic()) {
      emitClsIfSPropBase(e);

      if (classScope && classScope->isTrait()) {
        e.String(methName);
        e.Self(kClsRefSlotPlaceholder);
        fpiStart = m_ue.bcPos();
        e.FPushClsMethodF(numParams, kClsRefSlotPlaceholder);
      } else {
        fpiStart = m_ue.bcPos();
        e.FPushClsMethodD(numParams, methName, m_curFunc->pce()->name());
      }
    } else {
      e.This();
      fpiStart = m_ue.bcPos();
      e.FPushObjMethodD(numParams, methName, ObjMethodOp::NullThrows);
    }
    {
      FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
      for (uint32_t i = 0; i < numParams; i++) {
        emitVirtualLocal(i);
        emitFPass(e, i, PassByRefKind::ErrorOnCell);
      }
    }
    e.FCall(numParams);
    emitConvertToCell(e);
  };

  /*
   * Make the call to the memoized function and store the value in the
   * cache. There's three distinct cases here:
   *
   * 1 - Full dict memo cache. Just store the function call result in it. We
   * have to emit the base before the function call to ensure all the values are
   * at the right stack locations.
   *
   * 2 - Single value memo cache with a guard. Store the function call result in
   * the cache and store a True into the guard. If HHBBC can determine either
   * the cache or the guard is redundant, elide the appropriate sets. This case
   * is more complicated because whether or not we're going to perform either
   * set determines whether we need to push a base before or after the call.
   *
   * 3 - Single value memo cache without a guard. Similar to #1, but store the
   * value in the single value cache.
   */
  if (keyCount > 0) {
    emitBase(false);
    emitImplCall();
    emitMemoAccess(false);
  } else if (m_curFunc->memoizeGuardPropName) {
    Label noCacheSet;
    Label join;
    Label done;

    // If HHBBC determines the memoize function always returns Null, then
    // there's no point in storing the value, so jump over the set. This has to
    // be done before calling the wrapped function because we need to know if
    // we're going to have to push the cache's base first.
    e.Null();
    e.IsMemoType();
    e.JmpNZ(noCacheSet);

    // Otherwise make the call and store the value in the cache like usual.
    emitBase(false);
    emitImplCall();
    emitSet(e);
    e.Jmp(join);

    // We skipped storing the value in the cache, so just call the function
    // (without pushing a base).
    noCacheSet.set(e);
    emitImplCall();

    // If HHBBC determines the memoize function cannot return Null, there's no
    // need to set the guard (its not necessary because Null unambiguously means
    // not present). In that case, skip over setting the guard to true.
    join.set(e);
    e.Null();
    e.MaybeMemoType();
    e.JmpZ(done);
    emitBase(true);
    emitClsIfSPropBase(e);
    e.True();
    emitSet(e);
    e.PopC();

    done.set(e);
  } else {
    emitBase(false);
    emitImplCall();
    emitSet(e);
  }
  e.RetC();

  assert(m_evalStack.size() == 0);
  assert(m_evalStack.clsRefSlotStackEmpty());

  emitMethodDVInitializers(e, m_curFunc, meth, topOfBody);
}

void EmitterVisitor::emitPostponedCtors() {
  while (!m_postponedCtors.empty()) {
    PostponedCtor& p = m_postponedCtors.front();

    Attr attrs = AttrPublic;
    if (!SystemLib::s_inited || p.m_is->getClassScope()->isSystem()) {
      attrs = attrs | AttrBuiltin;
    }
    StringData* methDoc = staticEmptyString();
    p.m_fe->init(p.m_is->line0(), p.m_is->line1(),
                 m_ue.bcPos(), attrs, false, methDoc);
    Emitter e(p.m_is, m_ue, *this);
    FuncFinisher ff(this, e, p.m_fe);
    e.Null();
    e.RetC();

    m_postponedCtors.pop_front();
  }
}

void EmitterVisitor::emitPostponedPSinit(PostponedNonScalars& p,
                                         bool /*pinit*/) {
  Attr attrs = (Attr)(AttrPrivate | AttrStatic);
  if (!SystemLib::s_inited || p.m_is->getClassScope()->isSystem()) {
    attrs = attrs | AttrBuiltin;
  }
  StringData* methDoc = staticEmptyString();
  p.m_fe->init(p.m_is->line0(), p.m_is->line1(),
               m_ue.bcPos(), attrs, false, methDoc);

  Emitter e(p.m_is, m_ue, *this);
  FuncFinisher ff(this, e, p.m_fe);

  // Private instance and static properties are initialized using
  // InitProp.
  size_t nProps = p.m_vec->size();
  assert(nProps > 0);
  for (size_t i = 0; i < nProps; ++i) {
    const StringData* propName =
      makeStaticString(((*p.m_vec)[i]).first);

    Label isset;
    InitPropOp op = InitPropOp::NonStatic;
    const PreClassEmitter::Prop& preProp =
      p.m_fe->pce()->lookupProp(propName);
    if ((preProp.attrs() & AttrStatic) == AttrStatic) {
      op = InitPropOp::Static;
    } else if ((preProp.attrs() & (AttrPrivate|AttrStatic)) != AttrPrivate) {
      e.CheckProp(const_cast<StringData*>(propName));
      e.JmpNZ(isset);
    }
    visit((*p.m_vec)[i].second);
    emitConvertToCell(e);
    e.InitProp(const_cast<StringData*>(propName), op);
    isset.set(e);
  }
  e.Null();
  e.RetC();
}

void EmitterVisitor::emitPostponedPinits() {
  while (!m_postponedPinits.empty()) {
    PostponedNonScalars& p = m_postponedPinits.front();
    emitPostponedPSinit(p, true);
    p.release(); // Manually trigger memory cleanup.
    m_postponedPinits.pop_front();
  }
}

void EmitterVisitor::emitPostponedSinits() {
  while (!m_postponedSinits.empty()) {
    PostponedNonScalars& p = m_postponedSinits.front();
    emitPostponedPSinit(p, false);
    p.release(); // Manually trigger memory cleanup.
    m_postponedSinits.pop_front();
  }
}

void EmitterVisitor::emitPostponedCinits() {
  while (!m_postponedCinits.empty()) {
    PostponedNonScalars& p = m_postponedCinits.front();

    Attr attrs = (Attr)(AttrPrivate | AttrStatic);
    if (!SystemLib::s_inited || p.m_is->getClassScope()->isSystem()) {
      attrs = attrs | AttrBuiltin;
    }
    StringData* methDoc = staticEmptyString();
    p.m_fe->init(p.m_is->line0(), p.m_is->line1(),
                 m_ue.bcPos(), attrs, false, methDoc);
    static const StringData* s_constName = makeStaticString("constName");
    p.m_fe->appendParam(s_constName, FuncEmitter::ParamInfo());

    Emitter e(p.m_is, m_ue, *this);
    FuncFinisher ff(this, e, p.m_fe);

    // Generate HHBC of the structure:
    //
    //   private static function 86cinit(constName) {
    //     if (constName == "FOO") {
    //       return <expr for FOO>;
    //     } else if (constName == "BAR") {
    //       return <expr for BAR>;
    //     } else { # (constName == "BAZ")
    //        return <expr for BAZ>;
    //     }
    //   }
    size_t nConsts = p.m_vec->size();
    assert(nConsts > 0);
    Label retC;
    for (size_t i = 0; i < nConsts - 1; ++i) {
      Label mismatch;

      emitVirtualLocal(0);
      emitCGet(e);
      e.String((StringData*)makeStaticString(((*p.m_vec)[i]).first));
      e.Eq();
      e.JmpZ(mismatch);

      visit((*p.m_vec)[i].second);

      e.Jmp(retC);
      mismatch.set(e);
    }
    visit((*p.m_vec)[nConsts-1].second);
    retC.set(e);
    e.RetC();

    p.release(); // Manually trigger memory cleanup.
    m_postponedCinits.pop_front();
  }
}

void EmitterVisitor::emitVirtualLocal(int localId) {
  prepareEvalStack();

  m_evalStack.push(StackSym::L);
  m_evalStack.setInt(localId);
}

template<class Expr>
void EmitterVisitor::emitVirtualClassBase(Emitter& e, Expr* node) {
  prepareEvalStack();

  m_evalStack.push(StackSym::K);
  auto const func = node->getFunctionScope();

  if (node->isStatic()) {
    m_evalStack.setClsBaseType(SymbolicStack::CLS_LATE_BOUND);
  } else if (node->getClass()) {
    const ExpressionPtr& expr = node->getClass();
    if (isNormalLocalVariable(expr)) {
      SimpleVariable* sv = static_cast<SimpleVariable*>(expr.get());
      StringData* name = makeStaticString(sv->getName());
      Id locId = m_curFunc->lookupVarId(name);
      m_evalStack.setClsBaseType(SymbolicStack::CLS_NAMED_LOCAL);
      m_evalStack.setInt(locId);
    } else {
      /*
       * More complex expressions get stashed into an unnamed local so
       * we can evaluate them at the proper time.
       *
       * See emitResolveClsBase() for examples.
       */
      int unnamedLoc = m_curFunc->allocUnnamedLocal();
      int clsBaseIdx = m_evalStack.size() - 1;
      m_evalStack.setClsBaseType(SymbolicStack::CLS_UNNAMED_LOCAL);
      emitVirtualLocal(unnamedLoc);
      visit(node->getClass());
      emitConvertToCell(e);
      emitSet(e);
      m_evalStack.setUnnamedLocal(clsBaseIdx, unnamedLoc, m_ue.bcPos());
      emitPop(e);
    }
  } else if (!node->getClassScope() ||
             node->getClassScope()->isTrait() ||
             (func && func->isClosure())) {
    // In a trait, a potentially rebound closure or psuedo-main, we can't
    // resolve self:: or parent:: yet, so we emit special instructions that do
    // those lookups.
    if (node->isParent()) {
      m_evalStack.setClsBaseType(SymbolicStack::CLS_PARENT);
    } else if (node->isSelf()) {
      m_evalStack.setClsBaseType(SymbolicStack::CLS_SELF);
    } else {
      m_evalStack.setClsBaseType(SymbolicStack::CLS_STRING_NAME);
      m_evalStack.setString(
        makeStaticString(node->getOriginalClassName()));
    }
  } else if (node->isParent() &&
             node->getClassScope()->getOriginalParent().empty()) {
    // parent:: in a class without a parent.  We'll emit a Parent
    // opcode because it can handle this error case.
    m_evalStack.setClsBaseType(SymbolicStack::CLS_PARENT);
  } else {
    m_evalStack.setClsBaseType(SymbolicStack::CLS_STRING_NAME);
    m_evalStack.setString(
      makeStaticString(node->getOriginalClassName()));
  }
}

bool EmitterVisitor::emitCallUserFunc(Emitter& e, SimpleFunctionCallPtr func) {
  static struct {
    const char* name;
    int minParams, maxParams;
    CallUserFuncFlags flags;
  } cufTab[] = {
    { "call_user_func", 1, INT_MAX, CallUserFuncPlain },
    { "call_user_func_array", 2, 2, CallUserFuncArray },
    { "forward_static_call", 1, INT_MAX, CallUserFuncForward },
    { "forward_static_call_array", 2, 2, CallUserFuncForwardArray },
    { "fb_call_user_func_safe", 1, INT_MAX, CallUserFuncSafe },
    { "fb_call_user_func_array_safe", 2, 2, CallUserFuncSafeArray },
    { "fb_call_user_func_safe_return", 2, INT_MAX, CallUserFuncSafeReturn },
  };

  if (RuntimeOption::EvalDisableHphpcOpts) return false;

  ExpressionListPtr params = func->getParams();
  if (!params) return false;
  int nParams = params->getCount();
  if (!nParams) return false;
  CallUserFuncFlags flags = CallUserFuncNone;
  for (unsigned i = 0; i < sizeof(cufTab) / sizeof(cufTab[0]); i++) {
    if (func->isCallToFunction(cufTab[i].name) &&
        nParams >= cufTab[i].minParams &&
        nParams <= cufTab[i].maxParams) {
      flags = cufTab[i].flags;
      break;
    }
  }
  if (flags == CallUserFuncNone) return false;
  if (func->hasUnpack()) {
    throw EmitterVisitor::IncludeTimeFatalException(
      func,
      "Using argument unpacking for a call_user_func is not supported"
    );
  }
  int param = 1;
  ExpressionPtr callable = (*params)[0];
  visit(callable);
  emitConvertToCell(e);
  Offset fpiStart = m_ue.bcPos();
  if (flags & CallUserFuncForward) {
    e.FPushCufF(nParams - param);
  } else if (flags & CallUserFuncSafe) {
    if (flags & CallUserFuncReturn) {
      assert(nParams >= 2);
      visit((*params)[param++]);
      emitConvertToCell(e);
    } else {
      e.Null();
    }
    fpiStart = m_ue.bcPos();
    e.FPushCufSafe(nParams - param);
  } else {
    e.FPushCuf(nParams - param);
  }

  {
    FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
    for (int i = param; i < nParams; i++) {
      visit((*params)[i]);
      emitConvertToCell(e);
      e.FPassC(i - param);
    }
  }

  if (flags & CallUserFuncArray) {
    e.FCallArray();
  } else {
    e.FCall(nParams - param);
  }
  if (flags & CallUserFuncSafe) {
    if (flags & CallUserFuncReturn) {
      e.CufSafeReturn();
    } else {
      e.CufSafeArray();
    }
  }
  return true;
}

void EmitterVisitor::emitFuncCall(Emitter& e, FunctionCallPtr node,
                                  const char* nameOverride,
                                  ExpressionListPtr paramsOverride) {
  ExpressionPtr nameExp = node->getNameExp();
  const std::string& nameStr = nameOverride ? nameOverride :
                                              node->getOriginalName();
  ExpressionListPtr params(paramsOverride ? paramsOverride :
                                            node->getParams());
  int numParams = params ? params->getCount() : 0;

  ExpressionListPtr funcParams;
  StringData* nLiteral = nullptr;
  Offset fpiStart = 0;
  if (node->getClass() || node->hasStaticClass()) {
    bool isSelfOrParent = node->isSelf() || node->isParent();
    if (!node->isStatic() && !isSelfOrParent &&
        !node->getOriginalClassName().empty() && !nameStr.empty()) {
      // cls::foo()
      StringData* cLiteral =
        makeStaticString(node->getOriginalClassName());
      nLiteral = makeStaticString(nameStr);
      fpiStart = m_ue.bcPos();
      e.FPushClsMethodD(numParams, nLiteral, cLiteral);
    } else {
      emitVirtualClassBase(e, node.get());
      if (!nameStr.empty()) {
        // ...::foo()
        nLiteral = makeStaticString(nameStr);
        e.String(nLiteral);
      } else {
        // ...::$foo()
        visit(nameExp);
        emitConvertToCell(e);
      }
      emitResolveClsBase(e, m_evalStack.size() - 2);
      fpiStart = m_ue.bcPos();
      if (isSelfOrParent) {
        // self and parent are "forwarding" calls, so we need to
        // use FPushClsMethodF instead
        e.FPushClsMethodF(numParams, kClsRefSlotPlaceholder);
      } else {
        e.FPushClsMethod(numParams, kClsRefSlotPlaceholder);
      }
    }
  } else if (!nameStr.empty()) {
    // foo()
    nLiteral = makeStaticString(nameStr);
    StringData* nsName = nullptr;
    if (!node->hadBackslash() && !nameOverride) {
      // nameOverride is to be used only when there's an exact function
      // to be called ... supporting a fallback doesn't make sense
      const std::string& nonNSName = node->getNonNSOriginalName();
      if (nonNSName != nameStr) {
        nsName = nLiteral;
        nLiteral = makeStaticString(nonNSName);
      }
    }

    fpiStart = m_ue.bcPos();
    if (nsName == nullptr) {
      e.FPushFuncD(numParams, nLiteral);
    } else {
      assert(!nameOverride);
      e.FPushFuncU(numParams, nsName, nLiteral);
    }
  } else {
    // $foo()
    visit(nameExp);
    emitConvertToCell(e);
    // FPushFunc consumes method name from stack
    fpiStart = m_ue.bcPos();
    e.FPushFunc(numParams);
  }
  emitCall(e, node, params, fpiStart);
}

bool EmitterVisitor::emitConstantFuncCall(Emitter& e,
                                          SimpleFunctionCallPtr call) {
  if (!Option::WholeProgram || Option::ConstantFunctions.empty()) return false;

  if (call->getClass()) {
    // The class expression was either non-scalar or static, neither of which
    // we want to optimize.
    return false;
  }

  auto const name = call->getFullName();
  auto const it = Option::ConstantFunctions.find(name);
  if (it == Option::ConstantFunctions.end()) return false;

  VariableUnserializer uns{
    it->second.data(), it->second.size(), VariableUnserializer::Type::Serialize,
    false, empty_array()
  };

  try {
    return emitScalarValue(e, uns.unserialize());
  } catch (const Exception& e) {
    throw IncludeTimeFatalException(call,
                                    "Bad ConstantValue for %s: '%s'",
                                    name.c_str(), it->second.c_str());
  }
}

void EmitterVisitor::emitClassTraitPrecRule(PreClassEmitter* pce,
                                            TraitPrecStatementPtr stmt) {
  StringData* traitName  = makeStaticString(stmt->getTraitName());
  StringData* methodName = makeStaticString(stmt->getMethodName());

  PreClass::TraitPrecRule rule(traitName, methodName);

  hphp_string_iset otherTraitNames;
  stmt->getOtherTraitNames(otherTraitNames);
  for (auto const& name : otherTraitNames) {
    rule.addOtherTraitName(makeStaticString(name));
  }

  pce->addTraitPrecRule(rule);
}

void EmitterVisitor::emitClassTraitAliasRule(PreClassEmitter* pce,
                                             TraitAliasStatementPtr stmt) {
  StringData* traitName    = makeStaticString(stmt->getTraitName());
  StringData* origMethName = makeStaticString(stmt->getMethodName());
  StringData* newMethName  = makeStaticString(stmt->getNewMethodName());
  // If there are no modifiers, buildAttrs() defaults to AttrPublic.
  // Here we don't want that. Instead, set AttrNone so that the modifiers of the
  // original method are preserved.
  Attr attr = (stmt->getModifiers()->getCount() == 0 ? AttrNone :
               buildAttrs(stmt->getModifiers()));

  PreClass::TraitAliasRule rule(traitName, origMethName, newMethName, attr);

  pce->addTraitAliasRule(rule);
}

void EmitterVisitor::emitClassUseTrait(PreClassEmitter* pce,
                                       UseTraitStatementPtr useStmt) {
  auto rules = useStmt->getStmts();
  for (int r = 0; r < rules->getCount(); r++) {
    auto rule = (*rules)[r];
    auto precStmt = dynamic_pointer_cast<TraitPrecStatement>(rule);
    if (precStmt) {
      emitClassTraitPrecRule(pce, precStmt);
    } else {
      auto aliasStmt = dynamic_pointer_cast<TraitAliasStatement>(rule);
      assert(aliasStmt);
      emitClassTraitAliasRule(pce, aliasStmt);
    }
  }
}

Id EmitterVisitor::emitTypedef(Emitter& e, TypedefStatementPtr td) {
  auto const nullable = td->annot->isNullable();
  auto const annot = td->annot->stripNullable();
  auto const valueStr = annot.vanillaName();

  // We have to merge the strings as litstrs to ensure namedentity
  // creation.
  auto const name = makeStaticString(td->name);
  auto const value = makeStaticString(valueStr);
  m_ue.mergeLitstr(name);
  m_ue.mergeLitstr(value);

  AnnotType type;
  if (annot.isFunction() || annot.isMixed()) {
    type = AnnotType::Mixed;
  } else {
    auto const at = nameToAnnotType(value);
    type = at ? *at : AnnotType::Object;
    // Type aliases are always defined at top-level scope, so
    // they're not allowed to reference "self" or "parent" (and
    // "static" is already disallowed by the parser, so we don't
    // need to worry about it here).
    if (UNLIKELY(type == AnnotType::Self || type == AnnotType::Parent)) {
      throw IncludeTimeFatalException(
        td,
        "Cannot access %s when no class scope is active",
        type == AnnotType::Self ? "self" : "parent");
    }
  }

  UserAttributeMap userAttrs;
  ExpressionListPtr attrList = td->attrList;
  if (attrList) {
    for (int i = 0; i < attrList->getCount(); ++i) {
      auto attr = dynamic_pointer_cast<UserAttribute>((*attrList)[i]);
      auto const uaName = makeStaticString(attr->getName());
      auto uaValue = attr->getExp();
      assert(uaValue);
      assert(uaValue->isScalar());
      TypedValue tv;
      initScalar(tv, uaValue);
      userAttrs[uaName] = tv;
    }
  }

  TypeAlias record;
  record.typeStructure = Array(td->annot->getScalarArrayRep());
  record.name = name;
  record.value = value;
  record.type = type;
  record.nullable = nullable;
  record.userAttrs = userAttrs;
  record.attrs = !SystemLib::s_inited ? AttrPersistent : AttrNone;
  Id id = m_ue.addTypeAlias(record);
  e.DefTypeAlias(id);

  return id;
}

Id EmitterVisitor::emitClass(Emitter& e,
                             ClassScopePtr cNode,
                             bool toplevel) {

  const StringData* fatal_msg = cNode->getFatalMessage();
  if (fatal_msg != nullptr) {
    e.String(fatal_msg);
    e.Fatal(FatalOp::Runtime);
    return -1;
  }

  auto is = static_pointer_cast<InterfaceStatement>(cNode->getStmt());
  StringData* parentName = makeStaticString(cNode->getOriginalParent());
  StringData* classDoc = Option::GenerateDocComments ?
    makeStaticString(cNode->getDocComment()) : staticEmptyString();
  Attr attr = cNode->isInterface() ? AttrInterface :
              cNode->isTrait()     ? AttrTrait     :
              cNode->isAbstract()  ? AttrAbstract  :
              cNode->isEnum()      ? (AttrEnum | AttrFinal) :
                                     AttrNone;
  if (cNode->isFinal()) {
    attr = attr | AttrFinal;
  }

  if (Option::WholeProgram) {
    if (cNode->getUsedTraitNames().size()) {
      attr = attr | AttrNoExpandTrait;
    }
  }

  if (!SystemLib::s_inited || cNode->isSystem()) {
    // we're building systemlib. everything is unique
    attr |= AttrBuiltin | AttrUnique | AttrPersistent;
  }

  const std::vector<std::string>& bases(cNode->getBases());
  int firstInterface = cNode->getOriginalParent().empty() ? 0 : 1;
  int nInterfaces = bases.size();
  PreClass::Hoistable hoistable = PreClass::NotHoistable;
  if (toplevel) {
    if (SystemLib::s_inited && !cNode->isSystem()) {
      if (nInterfaces > firstInterface
          || cNode->getUsedTraitNames().size()
          || cNode->getClassRequiredExtends().size()
          || cNode->getClassRequiredImplements().size()
          || cNode->isEnum()
         ) {
        hoistable = PreClass::Mergeable;
      } else if (firstInterface &&
                 !m_hoistables.count(cNode->getOriginalParent())) {
        hoistable = PreClass::MaybeHoistable;
      }
    }
    if (hoistable == PreClass::NotHoistable) {
      hoistable = attr & AttrUnique ?
        PreClass::AlwaysHoistable : PreClass::MaybeHoistable;
      m_hoistables.insert(cNode->getOriginalName());
    }
  }
  PreClassEmitter* pce = m_ue.newPreClassEmitter(cNode->getOriginalName(),
                                                 hoistable);
  pce->init(is->line0(), is->line1(), m_ue.bcPos(), attr, parentName,
            classDoc);
  auto r = is->getRange();
  r.line1 = r.line0;
  r.char1 = r.char0;
  e.setTempLocation(r);
  if (hoistable != PreClass::AlwaysHoistable) {
    e.DefCls(pce->id());
  } else {
    // To attach the line number to for error reporting.
    e.DefClsNop(pce->id());
  }
  e.setTempLocation(OptLocation());
  for (int i = firstInterface; i < nInterfaces; ++i) {
    pce->addInterface(makeStaticString(bases[i]));
  }

  const std::vector<std::string>& usedTraits = cNode->getUsedTraitNames();
  for (size_t i = 0; i < usedTraits.size(); i++) {
    pce->addUsedTrait(makeStaticString(usedTraits[i]));
  }
  pce->setNumDeclMethods(cNode->getNumDeclMethods());
  if (cNode->isTrait() || cNode->isInterface() || Option::WholeProgram) {
    for (auto& reqExtends : cNode->getClassRequiredExtends()) {
      pce->addClassRequirement(
        PreClass::ClassRequirement(makeStaticString(reqExtends), true));
    }
    for (auto& reqImplements : cNode->getClassRequiredImplements()) {
      pce->addClassRequirement(
        PreClass::ClassRequirement(makeStaticString(reqImplements), false));
    }
  }
  auto const& userAttrs = cNode->userAttributes();
  for (auto it = userAttrs.begin(); it != userAttrs.end(); ++it) {
    const StringData* uaName = makeStaticString(it->first);
    ExpressionPtr uaValue = it->second;
    assert(uaValue);
    assert(uaValue->isScalar());
    TypedValue tv;
    initScalar(tv, uaValue);
    pce->addUserAttribute(uaName, tv);
  }

  NonScalarVec* nonScalarPinitVec = nullptr;
  NonScalarVec* nonScalarSinitVec = nullptr;
  NonScalarVec* nonScalarConstVec = nullptr;
  if (StatementListPtr stmts = is->getStmts()) {
    int i, n = stmts->getCount();
    for (i = 0; i < n; i++) {
      if (auto meth = dynamic_pointer_cast<MethodStatement>((*stmts)[i])) {
        StringData* methName = makeStaticString(meth->getOriginalName());
        FuncEmitter* fe = m_ue.newMethodEmitter(methName, pce);
        bool added UNUSED = pce->addMethod(fe);
        assert(added);
        postponeMeth(meth, fe, false);
      } else if (auto cv = dynamic_pointer_cast<ClassVariable>((*stmts)[i])) {
        ModifierExpressionPtr mod(cv->getModifiers());
        ExpressionListPtr el(cv->getVarList());
        Attr declAttrs = buildAttrs(mod);
        StringData* typeConstraint = makeStaticString(
          cv->getTypeConstraint());
        int nVars = el->getCount();
        for (int ii = 0; ii < nVars; ii++) {
          ExpressionPtr exp((*el)[ii]);
          ExpressionPtr vNode;
          SimpleVariablePtr var;
          if (exp->is(Expression::KindOfAssignmentExpression)) {
            auto ae = static_pointer_cast<AssignmentExpression>(exp);
            var = static_pointer_cast<SimpleVariable>(ae->getVariable());
            vNode = ae->getValue();
          } else {
            var = static_pointer_cast<SimpleVariable>(exp);
          }

          auto const propName = makeStaticString(var->getName());
          auto const propDoc = Option::GenerateDocComments ?
            makeStaticString(var->getDocComment()) : staticEmptyString();
          TypedValue tvVal;
          // Some properties may need to be marked with the AttrDeepInit
          // attribute, while other properties should not be marked with
          // this attrbiute. We copy declAttrs into propAttrs for each loop
          // iteration so that we can safely add AttrDeepInit to propAttrs
          // without mutating the original declAttrs.
          Attr propAttrs = declAttrs;
          if (vNode) {
            if (vNode->isScalar()) {
              initScalar(tvVal, vNode);
            } else {
              tvWriteUninit(&tvVal);
              if (!(declAttrs & AttrStatic)) {
                if (requiresDeepInit(vNode)) {
                  propAttrs = propAttrs | AttrDeepInit;
                }
                if (nonScalarPinitVec == nullptr) {
                  nonScalarPinitVec = new NonScalarVec();
                }
                nonScalarPinitVec->push_back(NonScalarPair(propName, vNode));
              } else {
                if (nonScalarSinitVec == nullptr) {
                  nonScalarSinitVec = new NonScalarVec();
                }
                nonScalarSinitVec->push_back(NonScalarPair(propName, vNode));
              }
            }
          } else {
            tvWriteNull(&tvVal);
          }
          bool added UNUSED =
            pce->addProperty(propName, propAttrs, typeConstraint,
                             propDoc, &tvVal, RepoAuthType{});
          assert(added);
        }
      } else if (auto cc = dynamic_pointer_cast<ClassConstant>((*stmts)[i])) {

        ExpressionListPtr el(cc->getConList());
        StringData* typeConstraint =
          makeStaticString(cc->getTypeConstraint());
        int nCons = el->getCount();

        if (cc->isAbstract()) {
          for (int ii = 0; ii < nCons; ii++) {
            auto con = static_pointer_cast<ConstantExpression>((*el)[ii]);
            StringData* constName = makeStaticString(con->getName());
            bool added UNUSED =
              pce->addAbstractConstant(constName, typeConstraint,
                                       cc->isTypeconst());
            assert(added);
          }
        } else {
          for (int ii = 0; ii < nCons; ii++) {
            auto ae = static_pointer_cast<AssignmentExpression>((*el)[ii]);
            auto con =
              static_pointer_cast<ConstantExpression>(ae->getVariable());
            auto vNode = ae->getValue();
            StringData* constName = makeStaticString(con->getName());
            assert(vNode);
            TypedValue tvVal;
            if (vNode->isCollection()) {
              throw IncludeTimeFatalException(
                cc, "Collections are not allowed in class constants");
            } else if (vNode->isScalar()) {
              initScalar(tvVal, vNode);
            } else {
              tvWriteUninit(&tvVal);
              if (nonScalarConstVec == nullptr) {
                nonScalarConstVec = new NonScalarVec();
              }
              nonScalarConstVec->push_back(NonScalarPair(constName, vNode));
            }
            // Store PHP source code for constant initializer.
            std::ostringstream os;
            CodeGenerator cg(&os, CodeGenerator::PickledPHP);
            auto ar = std::make_shared<AnalysisResult>();
            vNode->outputPHP(cg, ar);
            bool added UNUSED = pce->addConstant(
              constName, typeConstraint, &tvVal,
              makeStaticString(os.str()),
              cc->isTypeconst(),
              cc->getTypeStructure());
            assert(added);
          }
        }
      } else if (auto useStmt =
                 dynamic_pointer_cast<UseTraitStatement>((*stmts)[i])) {
        emitClassUseTrait(pce, useStmt);
      }
    }
  }

  if (!cNode->getAttribute(ClassScope::HasConstructor) &&
      !cNode->getAttribute(ClassScope::ClassNameConstructor) &&
      !cNode->isStaticUtil()) {
    // cNode does not have a constructor; synthesize 86ctor() so that the class
    // will always have a method that can be called during construction.
    static const StringData* methName = makeStaticString("86ctor");
    FuncEmitter* fe = m_ue.newMethodEmitter(methName, pce);
    bool added UNUSED = pce->addMethod(fe);
    assert(added);
    postponeCtor(is, fe);
  }

  if (nonScalarPinitVec != nullptr) {
    // Non-scalar property initializers require 86pinit() for run-time
    // initialization support.
    static const StringData* methName = makeStaticString("86pinit");
    FuncEmitter* fe = m_ue.newMethodEmitter(methName, pce);
    pce->addMethod(fe);
    postponePinit(is, fe, nonScalarPinitVec);
  }

  if (nonScalarSinitVec != nullptr) {
    // Non-scalar property initializers require 86sinit() for run-time
    // initialization support.
    static const StringData* methName = makeStaticString("86sinit");
    FuncEmitter* fe = m_ue.newMethodEmitter(methName, pce);
    pce->addMethod(fe);
    postponeSinit(is, fe, nonScalarSinitVec);
  }

  if (nonScalarConstVec != nullptr) {
    // Non-scalar constant initializers require 86cinit() for run-time
    // initialization support.
    static const StringData* methName = makeStaticString("86cinit");
    FuncEmitter* fe = m_ue.newMethodEmitter(methName, pce);
    assert(!(attr & AttrTrait));
    bool added UNUSED = pce->addMethod(fe);
    assert(added);
    postponeCinit(is, fe, nonScalarConstVec);
  }

  // If this is an enum, get its type constraint.
  if (cNode->isEnum()) {
    auto cs = static_pointer_cast<ClassStatement>(is);
    auto const typeConstraint =
      determine_type_constraint_from_annot(cs->getEnumBaseTy(), true);
    pce->setEnumBaseTy(typeConstraint);
  }

  return pce->id();
}

namespace {

struct ForeachIterGuard {
  ForeachIterGuard(EmitterVisitor& ev,
                   Id iterId,
                   IterKind kind)
    : m_ev(ev)
  {
    m_ev.pushIterScope(iterId, kind);
  }
  ~ForeachIterGuard() {
    m_ev.popIterScope();
  }

private:
  EmitterVisitor& m_ev;
};

}

void EmitterVisitor::emitForeachListAssignment(Emitter& e,
                                               ListAssignmentPtr la,
                                               std::function<void()> emitSrc) {
  std::vector<IndexPair> indexPairs;
  IndexChain workingChain;
  listAssignmentVisitLHS(e, la, workingChain, indexPairs);

  if (indexPairs.size() == 0) {
    throw IncludeTimeFatalException(la, "Cannot use empty list");
  }

  listAssignmentAssignElements(e, indexPairs, emitSrc);
}

void EmitterVisitor::emitForeach(Emitter& e,
                                 ForEachStatementPtr fe) {
  auto region = createRegion(fe, Region::Kind::LoopOrSwitch);
  ExpressionPtr ae(fe->getArrayExp());
  ExpressionPtr val(fe->getValueExp());
  ExpressionPtr key(fe->getNameExp());
  StatementPtr body(fe->getBody());
  int keyTempLocal;
  int valTempLocal;
  bool strong = fe->isStrong();
  Label& exit = registerBreak(fe, region.get(), 1, false)->m_label;
  Label& next = registerContinue(fe, region.get(), 1, false)->m_label;
  Label start;
  Offset bIterStart;
  Id itId = m_curFunc->allocIterator();
  ForeachIterGuard fig(*this, itId, strong ? KindOfMIter : KindOfIter);
  bool simpleCase = (!key || isNormalLocalVariable(key)) &&
                    isNormalLocalVariable(val);
  bool listKey = key ? key->is(Expression::KindOfListAssignment) : false;
  bool listVal = val->is(Expression::KindOfListAssignment);

  if (simpleCase) {
    auto svVal = static_pointer_cast<SimpleVariable>(val);
    StringData* name = makeStaticString(svVal->getName());
    valTempLocal = m_curFunc->lookupVarId(name);
    if (key) {
      auto svKey = static_pointer_cast<SimpleVariable>(key);
      name = makeStaticString(svKey->getName());
      keyTempLocal = m_curFunc->lookupVarId(name);
      visit(key);
      // Meta info on the key local will confuse the translator (and
      // wouldn't be useful anyway)
      m_evalStack.cleanTopMeta();
    } else {
      // Make gcc happy
      keyTempLocal = -1;
    }
    visit(val);
    // Meta info on the value local will confuse the translator (and
    // wouldn't be useful anyway)
    m_evalStack.cleanTopMeta();
    visit(ae);
    if (strong) {
      emitConvertToVar(e);
      if (key) {
        e.MIterInitK(itId, exit, valTempLocal, keyTempLocal);
      } else {
        e.MIterInit(itId, exit, valTempLocal);
      }
    } else {
      emitConvertToCell(e);
      if (key) {
        e.IterInitK(itId, exit, valTempLocal, keyTempLocal);
      } else {
        e.IterInit(itId, exit, valTempLocal);
      }
    }

    start.set(e);
    bIterStart = m_ue.bcPos();
  } else {
    keyTempLocal = key ? m_curFunc->allocUnnamedLocal() : -1;
    valTempLocal = m_curFunc->allocUnnamedLocal();
    if (key) {
      emitVirtualLocal(keyTempLocal);
    }
    emitVirtualLocal(valTempLocal);

    visit(ae);
    if (strong) {
      emitConvertToVar(e);
    } else {
      emitConvertToCell(e);
    }

    if (strong) {
      if (key) {
        e.MIterInitK(itId, exit, valTempLocal, keyTempLocal);
      } else {
        e.MIterInit(itId, exit, valTempLocal);
      }
    } else {
      if (key) {
        e.IterInitK(itId, exit, valTempLocal, keyTempLocal);
      } else {
        e.IterInit(itId, exit, valTempLocal);
      }
    }

    // At this point, valTempLocal and keyTempLocal if applicable, contain the
    // key and value for the iterator.
    start.set(e);
    bIterStart = m_ue.bcPos();
    if (key && !listKey) {
      visit(key);
      emitClsIfSPropBase(e);
    }
    if (listVal) {
      emitForeachListAssignment(
        e,
        ListAssignmentPtr(static_pointer_cast<ListAssignment>(val)),
        [&] { emitVirtualLocal(valTempLocal); }
      );
    } else {
      visit(val);
      emitClsIfSPropBase(e);
      emitVirtualLocal(valTempLocal);
      if (strong) {
        emitVGet(e);
        emitBind(e);
      } else {
        emitCGet(e);
        emitSet(e);
      }
      emitPop(e);
    }
    emitUnsetL(e, valTempLocal);
    newFaultRegionAndFunclet(bIterStart, m_ue.bcPos(),
                             new UnsetUnnamedLocalThunklet(valTempLocal));
    if (key) {
      assert(keyTempLocal != -1);
      if (listKey) {
        emitForeachListAssignment(
          e,
          ListAssignmentPtr(static_pointer_cast<ListAssignment>(key)),
          [&] { emitVirtualLocal(keyTempLocal); }
        );
      } else {
        emitVirtualLocal(keyTempLocal);
        emitCGet(e);
        emitSet(e);
        emitPop(e);
      }
      emitUnsetL(e, keyTempLocal);
      newFaultRegionAndFunclet(bIterStart, m_ue.bcPos(),
                               new UnsetUnnamedLocalThunklet(keyTempLocal));
    }
  }

  {
    region->m_iterId = itId;
    region->m_iterKind = strong ? KindOfMIter : KindOfIter;
    enterRegion(region);
    SCOPE_EXIT { leaveRegion(region); };
    if (body) visit(body);
  }
  if (next.isUsed()) {
    next.set(e);
  }
  if (key) {
    emitVirtualLocal(keyTempLocal);
    // Meta info on the key local will confuse the translator (and
    // wouldn't be useful anyway)
    m_evalStack.cleanTopMeta();
  }
  emitVirtualLocal(valTempLocal);
  // Meta info on the value local will confuse the translator (and
  // wouldn't be useful anyway)
  m_evalStack.cleanTopMeta();
  if (strong) {
    if (key) {
      e.MIterNextK(itId, start, valTempLocal, keyTempLocal);
    } else {
      e.MIterNext(itId, start, valTempLocal);
    }
  } else {
    if (key) {
      e.IterNextK(itId, start, valTempLocal, keyTempLocal);
    } else {
      e.IterNext(itId, start, valTempLocal);
    }
  }
  newFaultRegionAndFunclet(bIterStart, m_ue.bcPos(),
                           new IterFreeThunklet(itId, strong),
                           { itId, strong ? KindOfMIter : KindOfIter });
  if (!simpleCase) {
    m_curFunc->freeUnnamedLocal(valTempLocal);
    if (key) {
      m_curFunc->freeUnnamedLocal(keyTempLocal);
    }
  }
  exit.set(e);
  m_curFunc->freeIterator(itId);
}

void EmitterVisitor::emitForeachAwaitAs(Emitter& e,
                                        ForEachStatementPtr fe) {
  assert(!fe->isStrong());
  auto region = createRegion(fe, Region::Kind::LoopOrSwitch);
  Label& exit = registerBreak(fe, region.get(), 1, false)->m_label;
  Label& next = registerContinue(fe, region.get(), 1, false)->m_label;

  // Evaluate the AsyncIterator object and store it into unnamed local.
  auto const iterTempLocal = m_curFunc->allocUnnamedLocal();
  emitVirtualLocal(iterTempLocal);
  visit(fe->getArrayExp());
  emitConvertToCell(e);
  emitSet(e);
  auto const iterTempStartUse = m_ue.bcPos();

  // Make sure it actually is an AsyncIterator.
  e.InstanceOfD(makeStaticString("HH\\AsyncIterator"));
  e.JmpNZ(next);
  e.String(makeStaticString(
    "Unable to iterate non-AsyncIterator asynchronously"));
  e.Fatal(FatalOp::Runtime);

  // Start of the next iteration.
  next.set(e);

  // Await the next value.
  emitVirtualLocal(iterTempLocal);
  emitCGet(e);
  emitConstMethodCallNoParams(e, "next");
  e.Await();
  auto const resultTempLocal = emitSetUnnamedL(e);

  // Did we finish yet?
  emitVirtualLocal(resultTempLocal);
  emitIsType(e, IsTypeOp::Null);
  e.JmpNZ(exit);

  auto const populate = [&](ExpressionPtr target, int index) {
    auto const emitSrc = [&] {
      emitVirtualLocal(resultTempLocal);
      m_evalStack.push(StackSym::I);
      m_evalStack.setInt(index);
      markElem(e);
    };

    if (target->is(Expression::KindOfListAssignment)) {
      emitForeachListAssignment(
        e,
        ListAssignmentPtr(static_pointer_cast<ListAssignment>(target)),
        emitSrc
      );
    } else {
      // Obtain target to be set.
      visit(target);

      // Put $result[index] on the stack.
      emitSrc();
      emitCGet(e);

      // Set target.
      emitSet(e);
      emitPop(e);
    }
  };

  auto const resultTempStartUse = m_ue.bcPos();

  // Set the key.
  if (fe->getNameExp()) {
    populate(fe->getNameExp(), 0);
  }

  // Set the value.
  populate(fe->getValueExp(), 1);

  newFaultRegionAndFunclet(resultTempStartUse, m_ue.bcPos(),
                           new UnsetUnnamedLocalThunklet(resultTempLocal));
  emitUnsetL(e, resultTempLocal);

  // Run body.
  {
    enterRegion(region);
    SCOPE_EXIT { leaveRegion(region); };
    if (fe->getBody()) visit(fe->getBody());
  }

  // Continue iteration.
  e.Jmp(next);

  // Exit cleanup.
  exit.set(e);

  emitUnsetL(e, resultTempLocal);
  m_curFunc->freeUnnamedLocal(resultTempLocal);

  newFaultRegionAndFunclet(iterTempStartUse, m_ue.bcPos(),
                           new UnsetUnnamedLocalThunklet(iterTempLocal));
  emitUnsetL(e, iterTempLocal);
  m_curFunc->freeUnnamedLocal(iterTempLocal);
}

void EmitterVisitor::emitYieldFrom(Emitter& e, ExpressionPtr exp) {
  Id itId = m_curFunc->allocIterator();

  // Set the delegate to the result of visiting our expression
  visit(exp);
  emitConvertToCell(e);
  e.ContAssignDelegate(itId);

  Offset bDelegateAssigned = m_ue.bcPos();

  // Pass null to ContEnterDelegate initially.
  e.Null();

  Label loopBeginning(e);
  e.ContEnterDelegate();
  e.YieldFromDelegate(itId, loopBeginning);
  newFaultRegionAndFunclet(bDelegateAssigned, m_ue.bcPos(),
                           new UnsetGeneratorDelegateThunklet(itId));

  // Now that we're done with it, remove the delegate. This lets us enforce
  // the invariant that if we have a delegate set, we should be using it.
  e.ContUnsetDelegate(CudOp::IgnoreIter, itId);
}

/**
 * Emits bytecode that restores the previous error reporting level after
 * evaluating a silenced (@) expression, or in the fault funclet protecting such
 * an expression.  Requires a local variable id containing the previous error
 * reporting level.  The whole silenced expression looks like this:
 *   oldvalue = error_reporting(0)
 *   ...evaluate silenced expression...
 *   oldvalue = error_reporting(oldvalue)
 *   if oldvalue != 0:
 *     error_reporting(oldvalue)
 */
void EmitterVisitor::emitRestoreErrorReporting(Emitter& e, Id oldLevelLoc) {
  emitVirtualLocal(oldLevelLoc);
  auto idx = m_evalStack.size() - 1;
  e.Silence(m_evalStack.getLoc(idx), SilenceOp::End);
}

void EmitterVisitor::emitMakeUnitFatal(Emitter& e,
                                       const char* msg,
                                       FatalOp k) {
  const StringData* sd = makeStaticString(msg);
  e.String(sd);
  e.Fatal(k);
}

Funclet* EmitterVisitor::addFunclet(StatementPtr stmt, Thunklet* body) {
  Funclet* f = addFunclet(body);
  m_memoizedFunclets.insert(std::make_pair(stmt, f));
  return f;
}

Funclet* EmitterVisitor::addFunclet(Thunklet* body) {
  m_funclets.push_back(new Funclet(body));
  return m_funclets.back();
}

Funclet* EmitterVisitor::getFunclet(StatementPtr stmt) {
  if (m_memoizedFunclets.count(stmt)) {
    return m_memoizedFunclets[stmt];
  } else {
    return nullptr;
  }
}

void EmitterVisitor::emitFunclets(Emitter& e) {
  // TODO (#3271358): New fault funclets might appear while emitting
  // finally fault funclets. This is because we currently don't memoize
  // fault funclets other than finally fault fuclets. See task
  // description for more details.
  for (int i = 0; i < m_funclets.size(); ++i) {
    Funclet* f = m_funclets[i];
    f->m_entry.set(e);
    f->m_body->emit(e);
    delete f->m_body;
    f->m_body = nullptr;
  }
}

template<class EmitCatchBodyFun>
void EmitterVisitor::emitCatch(Emitter& e,
                               Offset start,
                               EmitCatchBodyFun emitCatchBody,
                               FaultIterInfo iter) {
  Label afterCatch;
  e.Jmp(afterCatch);

  Offset handler = e.getUnitEmitter().bcPos();

  e.Catch();
  emitCatchBody();
  e.Throw();

  Offset end = e.getUnitEmitter().bcPos();
  afterCatch.set(e);

  m_catchRegions.push_back(
      new CatchRegion(start, handler, end, iter.iterId, iter.kind));
}

void EmitterVisitor::newFaultRegion(Offset start,
                                    Offset end,
                                    Label* entry,
                                    FaultIterInfo iter) {
  assert(start < end);
  auto r = new FaultRegion(start, end, entry, iter.iterId, iter.kind);
  m_faultRegions.push_back(r);
}

void EmitterVisitor::newFaultRegionAndFunclet(Offset start,
                                              Offset end,
                                              Thunklet* t,
                                              FaultIterInfo iter) {
  if (start == end) {
    delete t;
    return;
  }
  Funclet* f = addFunclet(t);
  newFaultRegion(start, end, &f->m_entry, iter);
}

void EmitterVisitor::newFaultRegionAndFunclet(StatementPtr stmt,
                                              Offset start,
                                              Offset end,
                                              Thunklet* t,
                                              FaultIterInfo iter) {
  if (start == end) {
    delete t;
    return;
  }
  Funclet* f = addFunclet(stmt, t);
  newFaultRegion(start, end, &f->m_entry, iter);
}

void EmitterVisitor::newFPIRegion(Offset start, Offset end, Offset fpOff) {
  FPIRegion* r = new FPIRegion(start, end, fpOff);
  m_fpiRegions.push_back(r);
}

void EmitterVisitor::copyOverCatchAndFaultRegions(FuncEmitter* fe) {
  for (auto& cr : m_catchRegions) {
    auto& e = fe->addEHEnt();
    e.m_type = EHEnt::Type::Catch;
    e.m_base = cr->m_start;
    e.m_past = cr->m_handler;
    e.m_iterId = cr->m_iterId;
    e.m_itRef = cr->m_iterKind == KindOfMIter;
    e.m_handler = cr->m_handler;
    e.m_end = cr->m_end;
    assert(e.m_base != kInvalidOffset);
    assert(e.m_past != kInvalidOffset);
    assert(e.m_handler != kInvalidOffset);
    delete cr;
  }
  m_catchRegions.clear();
  for (auto& fr : m_faultRegions) {
    auto& e = fe->addEHEnt();
    e.m_type = EHEnt::Type::Fault;
    e.m_base = fr->m_start;
    e.m_past = fr->m_end;
    assert(e.m_base != kInvalidOffset);
    assert(e.m_past != kInvalidOffset);
    e.m_iterId = fr->m_iterId;
    e.m_itRef = fr->m_iterKind == KindOfMIter;
    e.m_handler = fr->m_func->getAbsoluteOffset();
    e.m_end = kInvalidOffset;
    assert(e.m_handler != kInvalidOffset);
    delete fr;
  }
  m_faultRegions.clear();
  for (auto f : m_funclets) {
    delete f;
  }
  m_funclets.clear();
  m_memoizedFunclets.clear();
}

void EmitterVisitor::copyOverFPIRegions(FuncEmitter* fe) {
  for (std::deque<FPIRegion*>::iterator it = m_fpiRegions.begin();
       it != m_fpiRegions.end(); ++it) {
    FPIEnt& e = fe->addFPIEnt();
    e.m_fpushOff = (*it)->m_start;
    e.m_fpiEndOff = (*it)->m_end;
    e.m_fpOff = (*it)->m_fpOff;
    delete *it;
  }
  m_fpiRegions.clear();
}

void EmitterVisitor::saveMaxStackCells(FuncEmitter* fe, int32_t stackPad) {
  fe->maxStackCells +=
    fe->numIterators() * kNumIterCells +
    fe->numLocals() +
    clsRefCountToCells(fe->numClsRefSlots()) +
    stackPad;

  m_evalStack.m_actualStackHighWaterPtr = nullptr;
}

// Are you sure you mean to be calling this directly? Would FuncFinisher
// be more appropriate?
void EmitterVisitor::finishFunc(Emitter& e, FuncEmitter* fe, int32_t stackPad) {
  emitFunclets(e);
  fe->setNumClsRefSlots(m_evalStack.numClsRefSlots());
  m_evalStack.setNumClsRefSlots(0);
  saveMaxStackCells(fe, stackPad);
  copyOverCatchAndFaultRegions(fe);
  copyOverFPIRegions(fe);
  m_staticEmitted.clear();
  Offset past = e.getUnitEmitter().bcPos();
  fe->finish(past, false);
  e.getUnitEmitter().recordFunction(fe);
  if (m_stateLocal >= 0) {
    m_stateLocal = -1;
  }
  if (m_retLocal >= 0) {
    m_retLocal = -1;
  }
}

void EmitterVisitor::initScalar(TypedValue& tvVal, ExpressionPtr val,
                                folly::Optional<HeaderKind> kind) {
  assert(val->isScalar());
  tvVal.m_type = KindOfUninit;
  // static array initilization
  auto initArray = [&](ExpressionPtr el, folly::Optional<HeaderKind> k) {
    if (k == HeaderKind::Dict) {
      m_staticArrays.push_back(Array::attach(MixedArray::MakeReserveDict(0)));
    } else if (k == HeaderKind::VecArray) {
      m_staticArrays.push_back(Array::attach(PackedArray::MakeReserveVec(0)));
    } else if (k == HeaderKind::Keyset) {
      m_staticArrays.push_back(Array::attach(SetArray::MakeReserveSet(0)));
    } else {
      m_staticArrays.push_back(Array::attach(PackedArray::MakeReserve(0)));
    }
    m_staticColType.push_back(k);
    visit(el);
    tvVal = make_array_like_tv(
      ArrayData::GetScalarArray(m_staticArrays.back().get())
    );
    m_staticArrays.pop_back();
    m_staticColType.pop_back();
  };
  switch (val->getKindOf()) {
    case Expression::KindOfConstantExpression: {
      auto ce = static_pointer_cast<ConstantExpression>(val);
      if (ce->isNull()) {
        tvVal.m_data.num = 0;
        tvVal.m_type = KindOfNull;
      } else if (ce->isBoolean()) {
        tvVal = make_tv<KindOfBoolean>(ce->getBooleanValue());
      } else if (ce->isScalar()) {
        ce->getScalarValue(tvAsVariant(&tvVal));
      } else {
        not_implemented();
      }
      break;
    }
    case Expression::KindOfScalarExpression: {
      auto sval = static_pointer_cast<ScalarExpression>(val);
      const std::string* s;
      if (sval->getString(s)) {
        StringData* sd = makeStaticString(*s);
        tvVal = make_tv<KindOfString>(sd);
        break;
      }
      int64_t i;
      if (sval->getInt(i)) {
        tvVal = make_tv<KindOfInt64>(i);
        break;
      }
      double d;
      if (sval->getDouble(d)) {
        tvVal = make_tv<KindOfDouble>(d);
        break;
      }
      assert(false);
      break;
    }
    case Expression::KindOfExpressionList: {
      // Array, possibly for collection initialization.
      initArray(val, kind);
      break;
    }
    case Expression::KindOfUnaryOpExpression: {
      auto u = static_pointer_cast<UnaryOpExpression>(val);
      if (u->getOp() == T_ARRAY) {
        initArray(u->getExpression(), folly::none);
        break;
      }
      if (u->getOp() == T_DICT) {
        initArray(u->getExpression(), HeaderKind::Dict);
        break;
      }
      if (u->getOp() == T_VEC) {
        initArray(u->getExpression(), HeaderKind::VecArray);
        break;
      }
      if (u->getOp() == T_KEYSET) {
        initArray(u->getExpression(), HeaderKind::Keyset);
        break;
      }
      // Fall through
    }
    default: {
      if (val->getScalarValue(tvAsVariant(&tvVal))) {
        if (tvAsVariant(&tvVal).isArray()) {
          not_implemented();
        }
        break;
      }
      not_reached();
    }
  }
}

void EmitterVisitor::emitArrayInit(Emitter& e, ExpressionListPtr el,
                                   folly::Optional<HeaderKind> kind,
                                   bool isDictForSetCollection) {
  assert(m_staticArrays.empty());
  assertx(!kind || isHackArrayKind(*kind));
  assertx(!isDictForSetCollection || (kind && *kind == HeaderKind::Dict));
  auto const isDict = kind == HeaderKind::Dict;
  auto const isVec = kind == HeaderKind::VecArray;
  auto const isKeyset = kind == HeaderKind::Keyset;

  if (el == nullptr) {
    if (isDict) {
      e.Dict(staticEmptyDictArray());
      return;
    }
    if (isVec) {
      e.Vec(staticEmptyVecArray());
      return;
    }
    if (isKeyset) {
      e.Keyset(staticEmptyKeysetArray());
      return;
    }
    e.Array(staticEmptyArray());
    return;
  }

  auto const scalar = [&]{
    if (isDictForSetCollection) el->isSetCollectionScalar();
    if (isVec) return el->isScalar();
    if (isDict) return isDictScalar(el);
    if (isKeyset) return isKeysetScalar(el);
    return isArrayScalar(el);
  }();
  if (scalar) {
    TypedValue tv;
    tvWriteUninit(&tv);
    initScalar(tv, el, kind);
    if (isDict) {
      assert(tv.m_data.parr->isDict());
      e.Dict(tv.m_data.parr);
      return;
    }
    if (isVec) {
      assert(tv.m_data.parr->isVecArray());
      e.Vec(tv.m_data.parr);
      return;
    }
    if (isKeyset) {
      assert(tv.m_data.parr->isKeyset());
      e.Keyset(tv.m_data.parr);
      return;
    }
    assert(tv.m_data.parr->isPHPArray());
    e.Array(tv.m_data.parr);
    return;
  }

  if (isVec || isKeyset) {
    auto const count = el->getCount();
    for (int i = 0; i < count; i++) {
      auto expr = static_pointer_cast<Expression>((*el)[i]);
      visit(expr);
      emitConvertToCell(e);
    }
    if (isVec) {
      e.NewVecArray(count);
    } else {
      e.NewKeysetArray(count);
    }
    return;
  }

  auto capacityHint = MixedArray::SmallSize;
  auto const capacity = el->getCount();
  if (capacity > 0) capacityHint = capacity;

  if (isDictForSetCollection) {
    e.NewDictArray(capacityHint);
    auto const count = el->getCount();
    for (int i = 0; i < count; i++) {
      auto ap = static_pointer_cast<ArrayPairExpression>((*el)[i]);
      visit(ap->getValue());
      emitConvertToCell(e);
      e.Dup();
      e.AddElemC();
    }
    return;
  }

  if (isDict) {
    e.NewDictArray(capacityHint);
    visit(el);
    return;
  }

  // from here on, we're dealing with PHP arrays only
  assertx(!kind);

  int nElms;
  if (isPackedInit(el, &nElms)) {
    for (int i = 0; i < nElms; ++i) {
      auto ap = static_pointer_cast<ArrayPairExpression>((*el)[i]);
      visit(ap->getValue());
      emitConvertToCell(e);
    }
    e.NewPackedArray(nElms);
    return;
  }

  std::vector<std::string> keys;
  if (isStructInit(el, keys)) {
    for (int i = 0, n = keys.size(); i < n; i++) {
      auto ap = static_pointer_cast<ArrayPairExpression>((*el)[i]);
      visit(ap->getValue());
      emitConvertToCell(e);
    }
    e.NewStructArray(keys);
    return;
  }

  if (isPackedInit(el, &nElms,
                   false /* ignore size */,
                   false /* hack arr compat */)) {
    e.NewArray(capacityHint);
  } else {
    e.NewMixedArray(capacityHint);
  }
  visit(el);
}

void EmitterVisitor::emitPairInit(Emitter& e, ExpressionListPtr el) {
  if (el->getCount() != 2) {
    throw IncludeTimeFatalException(el,
      "Pair objects must have exactly 2 elements");
  }
  for (int i = 0; i < 2; i++) {
    auto ap = static_pointer_cast<ArrayPairExpression>((*el)[i]);
    if (ap->getName() != nullptr) {
      throw IncludeTimeFatalException(ap,
        "Keys may not be specified for Pair initialization");
    }
    visit(ap->getValue());
    emitConvertToCell(e);
  }
  e.NewPair();
}

void EmitterVisitor::emitVectorInit(Emitter&e, CollectionType ct,
                                    ExpressionListPtr el) {
  // Do not allow specification of keys even if the resulting array is packed.
  // It doesn't make sense to specify keys for Vectors. Also, unwrap the values
  // out of these ArrayPairExpressions so that we can use the vec init code.
  // We can't do the upwrap in place because we sometimes visit parts of the
  // AST twice.
  auto unwrapped = std::make_shared<ExpressionList>(
    el->getScope(), el->getRange());
  for (int i = 0; i < el->getCount(); i++) {
    auto expr = (*el)[i];
    assertx(expr->getKindOf() == Expression::KindOfArrayPairExpression);
    auto ap = static_pointer_cast<ArrayPairExpression>(expr);
    if (ap->getName() != nullptr) {
      throw IncludeTimeFatalException(ap,
        "Keys may not be specified for Vector initialization");
    }
    unwrapped->addElement(ap->getValue());
  }
  emitArrayInit(e, unwrapped, HeaderKind::VecArray);
  e.ColFromArray(ct);
  return;
}

void EmitterVisitor::emitSetInit(Emitter&e, CollectionType ct,
                                 ExpressionListPtr el) {
  // Do not allow keys; it doesn't make sense to specify keys for Sets.
  for (int i = 0; i < el->getCount(); i++) {
    auto const expr = (*el)[i];
    assertx(expr->getKindOf() == Expression::KindOfArrayPairExpression);
    auto const ap = static_pointer_cast<ArrayPairExpression>(expr);
    if (ap->getName() != nullptr) {
      throw EmitterVisitor::IncludeTimeFatalException(ap,
        "Keys may not be specified for Set initialization");
    }
  }
  emitArrayInit(e, el, HeaderKind::Dict, true);
  e.ColFromArray(ct);
}

void EmitterVisitor::emitMapInit(Emitter&e, CollectionType ct,
                                 ExpressionListPtr el) {
  // Make sure all the ArrayPairExpressions have keys.
  for (int i = 0; i < el->getCount(); i++) {
    auto const expr = (*el)[i];
    assertx(expr->getKindOf() == Expression::KindOfArrayPairExpression);
    auto const ap = static_pointer_cast<ArrayPairExpression>(expr);
    if (ap->getName() == nullptr) {
      throw EmitterVisitor::IncludeTimeFatalException(ap,
        "Keys must be specified for Map initialization");
    }
  }
  emitArrayInit(e, el, HeaderKind::Dict);
  e.ColFromArray(ct);
}

void EmitterVisitor::emitCollectionInit(Emitter& e, BinaryOpExpressionPtr b) {
  auto cls = static_pointer_cast<ScalarExpression>(b->getExp1());
  const std::string* clsName = nullptr;
  cls->getString(clsName);
  auto ct = collections::stringToType(*clsName);
  if (!ct) {
    throw IncludeTimeFatalException(b,
      "Cannot use collection initialization for non-collection class");
  }

  ExpressionListPtr el = static_pointer_cast<ExpressionList>(b->getExp2());
  if (!el || el->getCount() == 0) {
    if (ct == CollectionType::Pair) {
      throw IncludeTimeFatalException(b, "Initializer needed for Pair object");
    }
    e.NewCol(*ct);
    return;
  }

  if (ct == CollectionType::Pair) {
    return emitPairInit(e, el);
  }

  if (ct == CollectionType::Vector || ct == CollectionType::ImmVector) {
    return emitVectorInit(e, *ct, el);
  }

  if (ct == CollectionType::Map || ct == CollectionType::ImmMap) {
    return emitMapInit(e, *ct, el);
  }

  if (ct == CollectionType::Set || ct == CollectionType::ImmSet) {
    return emitSetInit(e, *ct, el);
  }

  not_reached();
}

bool EmitterVisitor::requiresDeepInit(ExpressionPtr initExpr) const {
  switch (initExpr->getKindOf()) {
    case Expression::KindOfScalarExpression:
      return false;
    case Expression::KindOfClassConstantExpression:
    case Expression::KindOfConstantExpression:
      return !initExpr->isScalar();
    case Expression::KindOfUnaryOpExpression: {
      auto u = static_pointer_cast<UnaryOpExpression>(initExpr);
      if (u->getOp() == T_ARRAY || u->getOp() == T_DICT) {
        auto el = static_pointer_cast<ExpressionList>(u->getExpression());
        if (el) {
          int n = el->getCount();
          for (int i = 0; i < n; i++) {
            auto ap = static_pointer_cast<ArrayPairExpression>((*el)[i]);
            ExpressionPtr key = ap->getName();
            if (requiresDeepInit(ap->getValue()) ||
                (key && requiresDeepInit(key))) {
              return true;
            }
          }
        }
        return false;
      } else if (u->getOp() == T_VEC || u->getOp() == T_KEYSET) {
        auto el = static_pointer_cast<ExpressionList>(u->getExpression());
        if (el) {
          int n = el->getCount();
          for (int i = 0; i < n; i++) {
            auto expr = static_pointer_cast<Expression>((*el)[i]);
            if (requiresDeepInit(expr)) return true;
          }
        }
        return false;
      } else if (u->getOp() == '+' || u->getOp() == '-') {
        return requiresDeepInit(u->getExpression());
      } else if (u->getOp() == T_FILE || u->getOp() == T_DIR) {
        return false;
      }
      return true;
    }
    case Expression::KindOfBinaryOpExpression: {
      auto b = static_pointer_cast<BinaryOpExpression>(initExpr);
      return requiresDeepInit(b->getExp1()) || requiresDeepInit(b->getExp2());
    }
    default:
      return true;
  }
}

Thunklet::~Thunklet() {}

static ConstructPtr doOptimize(ConstructPtr c, AnalysisResultConstPtr ar) {
  if (RuntimeOption::EvalDisableHphpcOpts) return ConstructPtr();

  for (int i = 0, n = c->getKidCount(); i < n; i++) {
    if (ConstructPtr k = c->getNthKid(i)) {
      if (ConstructPtr rep = doOptimize(k, ar)) {
        c->setNthKid(i, rep);
      }
    }
  }
  if (auto e = dynamic_pointer_cast<Expression>(c)) {
    switch (e->getKindOf()) {
      case Expression::KindOfBinaryOpExpression:
      case Expression::KindOfUnaryOpExpression:
      case Expression::KindOfIncludeExpression:
      case Expression::KindOfSimpleFunctionCall:
        return e->preOptimize(ar);
      case Expression::KindOfClosureExpression: {
        auto cl = static_pointer_cast<ClosureExpression>(e);
        auto UNUSED exp = doOptimize(cl->getClosureFunction(), ar);
        assert(!exp);
        break;
      }
      default: break;
    }
  }
  return ConstructPtr();
}

static UnitEmitter* emitHHBCUnitEmitter(AnalysisResultPtr ar, FileScopePtr fsp,
                                        const MD5& md5) {
  if (fsp->getPseudoMain() && !Option::WholeProgram) {
    ar->setPhase(AnalysisResult::FirstPreOptimize);
    doOptimize(fsp->getPseudoMain()->getStmt(), ar);
  }

  if (RuntimeOption::EvalDumpAst) {
    if (fsp->getPseudoMain()) {
      fsp->getPseudoMain()->getStmt()->dump(0, ar);
    }
  }

  auto msp =
    dynamic_pointer_cast<MethodStatement>(fsp->getPseudoMain()->getStmt());
  UnitEmitter* ue = new UnitEmitter(md5);
  ue->m_preloadPriority = fsp->preloadPriority();
  ue->initMain(msp->line0(), msp->line1());
  EmitterVisitor ev(*ue);
  try {
    ev.visit(fsp);
  } catch (EmitterVisitor::IncludeTimeFatalException& ex) {
    // Replace the unit with an empty one, but preserve its file path.
    UnitEmitter* nue = new UnitEmitter(md5);
    nue->initMain(msp->line0(), msp->line1());
    nue->m_filepath = ue->m_filepath;
    delete ue;
    ue = nue;

    EmitterVisitor fev(*ue);
    Emitter emitter(ex.m_node, *ue, fev);
    FuncFinisher ff(&fev, emitter, ue->getMain());
    auto kind = ex.m_parseFatal ? FatalOp::Parse : FatalOp::Runtime;
    fev.emitMakeUnitFatal(emitter, ex.getMessage().c_str(), kind);
  }
  return ue;
}

enum GeneratorMethod {
  METH_NEXT,
  METH_SEND,
  METH_RAISE,
  METH_VALID,
  METH_CURRENT,
  METH_KEY,
  METH_REWIND,
  METH_GETRETURN,
};

typedef hphp_hash_map<const StringData*, GeneratorMethod,
                      string_data_hash, string_data_same> ContMethMap;
typedef std::map<StaticString, GeneratorMethod> ContMethMapT;

namespace {
StaticString s_next("next");
StaticString s_send("send");
StaticString s_raise("raise");
StaticString s_valid("valid");
StaticString s_current("current");
StaticString s_key("key");
StaticString s_throw("throw");
StaticString s_rewind("rewind");
StaticString s_getReturn("getReturn");

StaticString genCls("Generator");
StaticString asyncGenCls("HH\\AsyncGenerator");

ContMethMapT s_asyncGenMethods = {
    {s_next, GeneratorMethod::METH_NEXT},
    {s_send, GeneratorMethod::METH_SEND},
    {s_raise, GeneratorMethod::METH_RAISE}
  };
ContMethMapT s_genMethods = {
    {s_next, GeneratorMethod::METH_NEXT},
    {s_send, GeneratorMethod::METH_SEND},
    {s_raise, GeneratorMethod::METH_RAISE},
    {s_valid, GeneratorMethod::METH_VALID},
    {s_current, GeneratorMethod::METH_CURRENT},
    {s_key, GeneratorMethod::METH_KEY},
    {s_throw, GeneratorMethod::METH_RAISE},
    {s_rewind, GeneratorMethod::METH_REWIND},
    {s_getReturn, GeneratorMethod::METH_GETRETURN}
  };
}

static int32_t emitGeneratorMethod(UnitEmitter& ue,
                                   FuncEmitter* fe,
                                   GeneratorMethod m,
                                   bool isAsync) {
  Attr attrs = (Attr)(AttrBuiltin | AttrPublic);
  fe->init(0, 0, ue.bcPos(), attrs, false, staticEmptyString());
  fe->isNative = false;
  if (!isAsync && RuntimeOption::AutoprimeGenerators) {
    // Create a dummy Emitter, so it's possible to emit jump instructions
    EmitterVisitor ev(ue);
    Emitter e(ConstructPtr(), ue, ev);
    Location::Range loc;
    if (ue.bcPos() > 0) loc.line0 = -1;
    e.setTempLocation(loc);

    // Check if the generator has started yet
    Label started;
    e.ContStarted();
    e.JmpNZ(started);

    // If it hasn't started, perform one "next" operation before
    // the actual operation (auto-priming)
    e.ContCheck(ContCheckOp::IgnoreStarted);
    e.Null();
    e.ContEnter();
    e.PopC();
    started.set(e);
  }

  if (!RuntimeOption::AutoprimeGenerators && m == METH_REWIND) {
    // In non-autopriming mode the rewind function will always call next, when
    // autopriming is enabled, rewind matches PHP behavior and will only advance
    // the generator when it has not yet been started.
    m = METH_NEXT;
  }

  switch (m) {
    case METH_SEND:
    case METH_RAISE:
    case METH_NEXT: {
      // We always want these methods to be cloned with new funcids in
      // subclasses so we can burn Class*s and Func*s into the
      // translations
      fe->attrs |= AttrClone;

      // check generator status; send()/raise() also checks started
      ue.emitOp(OpContCheck);
      ue.emitByte(static_cast<uint8_t>(
          m == METH_SEND || m == METH_RAISE ? ContCheckOp::CheckStarted :
          ContCheckOp::IgnoreStarted
      ));

      switch (m) {
        case METH_NEXT:
          ue.emitOp(OpNull);
          ue.emitOp(OpContEnter);
          break;
        case METH_SEND:
          ue.emitOp(OpPushL); ue.emitIVA(0);
          ue.emitOp(OpContEnter);
          break;
        case METH_RAISE:
          ue.emitOp(OpPushL); ue.emitIVA(0);
          ue.emitOp(OpContRaise);
          break;
        default:
          not_reached();
      }

      // Backtrace has off-by-one bug when determining whether we are
      // in returning opcode; add Nop to avoid it
      ue.emitOp(OpNop);
      ue.emitOp(OpRetC);
      break;
    }
    case METH_VALID: {
      ue.emitOp(OpContValid);
      ue.emitOp(OpRetC);
      break;
    }
    case METH_CURRENT: {
      ue.emitOp(OpContCurrent);
      ue.emitOp(OpRetC);
      break;
    }
    case METH_KEY: {
      ue.emitOp(OpContKey);
      ue.emitOp(OpRetC);
      break;
    }
    case METH_REWIND: {
      ue.emitOp(OpNull);
      ue.emitOp(OpRetC);
      break;
    }
    case METH_GETRETURN: {
      ue.emitOp(OpContGetReturn);
      ue.emitOp(OpRetC);
      break;
    }
    default:
      not_reached();
  }

  return 1;  // Above cases push at most one stack cell.
}

// Emit byte codes to implement methods. Return the maximum stack cell count.
int32_t EmitterVisitor::emitNativeOpCodeImpl(MethodStatementPtr meth,
                                             const char* funcName,
                                             const char* className,
                                             FuncEmitter* fe) {
  GeneratorMethod* cmeth;
  StaticString s_func(funcName);
  StaticString s_class(className);

  if (genCls.same(s_class) &&
      (cmeth = folly::get_ptr(s_genMethods, s_func))) {
    return emitGeneratorMethod(m_ue, fe, *cmeth, false);
  } else if (asyncGenCls.same(s_class) &&
      (cmeth = folly::get_ptr(s_asyncGenMethods, s_func))) {
    return emitGeneratorMethod(m_ue, fe, *cmeth, true);
  }

  throw IncludeTimeFatalException(meth,
    "OpCodeImpl attribute is not applicable to %s", funcName);
}

namespace {

std::atomic<uint64_t> lastHHBCUnitIndex;

}

static UnitEmitter* emitHHBCVisitor(AnalysisResultPtr ar, FileScopeRawPtr fsp) {
  // If we're in whole program mode, we can just assign each Unit an increasing
  // counter, guaranteeing uniqueness.
  auto md5 = Option::WholeProgram ? MD5{++lastHHBCUnitIndex} : fsp->getMd5();

  if (!Option::WholeProgram) {
    // The passed-in ar is only useful in whole-program mode, so create a
    // distinct ar to be used only for emission of this unit, and perform
    // unit-level (non-global) optimization.
    ar = std::make_shared<AnalysisResult>();
    fsp->setOuterScope(ar);

    ar->setPhase(AnalysisResult::AnalyzeAll);
    fsp->analyzeProgram(ar);
  }

  auto ue = emitHHBCUnitEmitter(ar, fsp, md5);
  assert(ue != nullptr);

  if (Option::GenerateTextHHBC) {
    // TODO(#2973538): Move HHBC text generation to after all the
    // units are created, and get rid of the LitstrTable locking,
    // since it won't be needed in that case.
    LitstrTable::get().mutex().lock();
    LitstrTable::get().setReading();
    std::unique_ptr<Unit> unit(ue->create());
    std::string fullPath = AnalysisResult::prepareFile(
      ar->getOutputPath().c_str(), Option::UserFilePrefix + fsp->getName(),
      true, false) + ".hhbc.txt";

    std::ofstream f(fullPath.c_str());
    if (!f) {
      Logger::Error("Unable to open %s for write", fullPath.c_str());
    } else {
      CodeGenerator cg(&f, CodeGenerator::TextHHBC);
      cg.printf("Hash: %" PRIx64 "%016" PRIx64 "\n", md5.q[0], md5.q[1]);
      cg.printRaw(unit->toString().c_str());
      f.close();
    }
    LitstrTable::get().setWriting();
    LitstrTable::get().mutex().unlock();
  }

  return ue;
}

struct UEQ : Synchronizable {
  void push(UnitEmitter* ue) {
    assert(ue != nullptr);
    Lock lock(this);
    m_ues.push_back(ue);
    notify();
  }
  UnitEmitter* tryPop(long sec, long long nsec) {
    Lock lock(this);
    if (m_ues.empty()) {
      // Check for empty() after wait(), in case of spurious wakeup.
      if (!wait(sec, nsec) || m_ues.empty()) {
        return nullptr;
      }
    }
    assert(m_ues.size() > 0);
    UnitEmitter* ue = m_ues.front();
    assert(ue != nullptr);
    m_ues.pop_front();
    return ue;
  }
 private:
  std::deque<UnitEmitter*> m_ues;
};
static UEQ s_ueq;

class EmitterWorker
  : public JobQueueWorker<FileScopeRawPtr, void*, true, true> {
 public:
  EmitterWorker() : m_ret(true) {}
  void doJob(JobType job) override {
    try {
      AnalysisResultPtr ar = ((AnalysisResult*)m_context)->shared_from_this();
      UnitEmitter* ue = emitHHBCVisitor(ar, job);
      if (Option::GenerateBinaryHHBC) {
        s_ueq.push(ue);
      } else {
        delete ue;
      }
    } catch (Exception &e) {
      Logger::Error("%s", e.getMessage().c_str());
      m_ret = false;
    } catch (...) {
      Logger::Error("Fatal: An unexpected exception was thrown");
      m_ret = false;
    }
  }
  void onThreadEnter() override {
    g_context.getCheck();
  }
  void onThreadExit() override {
    hphp_memory_cleanup();
  }
 private:
  bool m_ret;
};

static void
addEmitterWorker(AnalysisResultPtr /*ar*/, StatementPtr sp, void* data) {
  ((JobQueueDispatcher<EmitterWorker>*)data)->enqueue(sp->getFileScope());
}

static void
commitGlobalData(std::unique_ptr<ArrayTypeTable::Builder> arrTable) {
  auto gd                       = Repo::GlobalData{};
  gd.UsedHHBBC                  = Option::UseHHBBC;
  gd.EnableHipHopSyntax         = RuntimeOption::EnableHipHopSyntax;
  gd.HardTypeHints              = HHBBC::options.HardTypeHints;
  gd.HardReturnTypeHints        = HHBBC::options.HardReturnTypeHints;
  gd.HardPrivatePropInference   = true;
  gd.DisallowDynamicVarEnvFuncs = HHBBC::options.DisallowDynamicVarEnvFuncs;
  gd.ElideAutoloadInvokes       = HHBBC::options.ElideAutoloadInvokes;
  gd.PHP7_IntSemantics          = RuntimeOption::PHP7_IntSemantics;
  gd.PHP7_ScalarTypes           = RuntimeOption::PHP7_ScalarTypes;
  gd.PHP7_Substr                = RuntimeOption::PHP7_Substr;
  gd.PHP7_Builtins              = RuntimeOption::PHP7_Builtins;
  gd.AutoprimeGenerators        = RuntimeOption::AutoprimeGenerators;
  gd.PromoteEmptyObject         = RuntimeOption::EvalPromoteEmptyObject;
  gd.EnableRenameFunction       = RuntimeOption::EvalJitEnableRenameFunction;
  gd.CheckThisTypeHints         = RuntimeOption::EvalCheckThisTypeHints;
  gd.HackArrCompatNotices       = RuntimeOption::EvalHackArrCompatNotices;

  for (auto a : Option::APCProfile) {
    gd.APCProfile.emplace_back(StringData::MakeStatic(folly::StringPiece(a)));
  }
  if (arrTable) gd.arrayTypeTable.repopulate(*arrTable);
  Repo::get().saveGlobalData(gd);
}

/*
 * This is the entry point for offline bytecode generation.
 */
void emitAllHHBC(AnalysisResultPtr&& ar) {
  unsigned int threadCount = Option::ParserThreadCount;
  unsigned int nFiles = ar->getAllFilesVector().size();
  if (threadCount > nFiles) {
    threadCount = nFiles;
  }
  if (!threadCount) threadCount = 1;

  LitstrTable::get().setWriting();

  /* there is a race condition in the first call to
     makeStaticString. Make sure we dont hit it */
  {
    makeStaticString("");
    /* same for TypeConstraint */
    TypeConstraint tc;
  }

  Compiler::ClearErrors();

  JobQueueDispatcher<EmitterWorker>
    dispatcher(threadCount, 0, false, ar.get());

  auto setPreloadPriority = [&ar](const std::string& f, int p) {
    auto fs = ar->findFileScope(f);
    if (fs) fs->setPreloadPriority(p);
  };

  /*
   * Mark files that are referenced from the autoload map
   * so they get preloaded via preloadRepo.
   * Higher priorities are preloaded first.
   * Classes, then functions, then constants mimics
   * the order of our existing warmup scripts
   */
  for (const auto& ent : Option::AutoloadConstMap) {
    setPreloadPriority(ent.second, 1);
  }
  for (const auto& ent : Option::AutoloadFuncMap) {
    setPreloadPriority(ent.second, 2);
  }
  for (const auto& ent : Option::AutoloadClassMap) {
    setPreloadPriority(ent.second, 3);
  }

  dispatcher.start();
  ar->visitFiles(addEmitterWorker, &dispatcher);

  auto ues = ar->getHhasFiles();
  if (!Option::UseHHBBC && ues.size()) {
    batchCommit(std::move(ues));
  }

  if (Option::GenerateBinaryHHBC) {
    // kBatchSize needs to strike a balance between reducing transaction commit
    // overhead (bigger batches are better), and limiting the cost incurred by
    // failed commits due to identical units that require rollback and retry
    // (smaller batches have less to lose).  Empirical results indicate that a
    // value in the 2-10 range is reasonable.
    static const unsigned kBatchSize = 8;

    // Gather up units created by the worker threads and commit them in
    // batches.
    bool didPop;
    bool inShutdown = false;
    while (true) {
      // Poll, but with a 100ms timeout so that this thread doesn't spin wildly
      // if it gets ahead of the workers.
      UnitEmitter* ue = s_ueq.tryPop(0, 100 * 1000 * 1000);
      if ((didPop = (ue != nullptr))) {
        ues.push_back(std::unique_ptr<UnitEmitter>{ue});
      }
      if (!Option::UseHHBBC &&
          (ues.size() == kBatchSize ||
           (!didPop && inShutdown && ues.size() > 0))) {
        batchCommit(std::move(ues));
      }
      if (!inShutdown) {
        inShutdown = dispatcher.pollEmpty();
      } else if (!didPop) {
        assert(Option::UseHHBBC || ues.size() == 0);
        break;
      }
    }

    if (!Option::UseHHBBC) {
      commitGlobalData(std::unique_ptr<ArrayTypeTable::Builder>{});
    }
  } else {
    dispatcher.waitEmpty();
  }

  assert(Option::UseHHBBC || ues.empty());

  ar->finish();
  ar.reset();

  if (!Option::UseHHBBC) {
    batchCommit(std::move(ues));
    return;
  }

  RuntimeOption::EvalJit = false; // For HHBBC to invoke builtins.
  auto pair = [&]{
    Timer timer(Timer::WallTime, "running HHBBC");
    return HHBBC::whole_program(
      std::move(ues),
      Option::ParserThreadCount > 0 ? Option::ParserThreadCount : 0
    );
  }();
  batchCommit(std::move(pair.first));
  commitGlobalData(std::move(pair.second));
}

namespace {

bool startsWith(const char* big, const char* small) {
  return strncmp(big, small, strlen(small)) == 0;
}

bool isFileHack(const char* code, size_t codeLen) {
  // if the file starts with a shebang
  if (codeLen > 2 && strncmp(code, "#!", 2) == 0) {
    // reset code to the next char after the shebang line
    const char* loc = reinterpret_cast<const char*>(
        memchr(code, '\n', codeLen));
    if (!loc) {
      return false;
    }

    ptrdiff_t offset = loc - code;
    code = loc + 1;
    codeLen -= offset + 1;
  }

  return codeLen > strlen("<?hh") && startsWith(code, "<?hh");
}

////////////////////////////////////////////////////////////////////////////////
}

extern "C" {

/**
 * This is the entry point from the runtime; i.e. online bytecode generation.
 * The 'filename' parameter may be NULL if there is no file associated with
 * the source code.
 *
 * Before being actually used, hphp_compiler_parse must be called with
 * a NULL `code' parameter to do initialization.
 */

Unit* hphp_compiler_parse(const char* code, int codeLen, const MD5& md5,
                          const char* filename, Unit** releaseUnit) {
  if (UNLIKELY(!code)) {
    // Do initialization when code is null; see above.
    HHBBC::options.HardReturnTypeHints =
      RuntimeOption::EvalCheckReturnTypeHints >= 3;
    Option::RecordErrors = false;
    Option::ParseTimeOpts = false;
    Option::WholeProgram = false;
    BuiltinSymbols::LoadSuperGlobals();
    TypeConstraint tc;
    return nullptr;
  }

  // Do not count memory used during parsing/emitting towards OOM.
  MemoryManager::SuppressOOM so(MM());

  SCOPE_ASSERT_DETAIL("hphp_compiler_parse") { return filename; };
  std::unique_ptr<Unit> unit;
  SCOPE_EXIT {
    if (unit && releaseUnit) *releaseUnit = unit.release();
  };

  // We don't want to invoke the JIT when trying to run PHP code.
  auto const prevFolding = RID().getJitFolding();
  RID().setJitFolding(true);
  SCOPE_EXIT { RID().setJitFolding(prevFolding); };

  try {
    UnitOrigin unitOrigin = UnitOrigin::File;
    if (!filename) {
      filename = "";
      unitOrigin = UnitOrigin::Eval;
    }

    std::unique_ptr<UnitEmitter> ue;
    // Check if this file contains raw hip hop bytecode instead of
    // php.  This is dictated by a special file extension.
    if (RuntimeOption::EvalAllowHhas) {
      if (const char* dot = strrchr(filename, '.')) {
        const char hhbc_ext[] = "hhas";
        if (!strcmp(dot + 1, hhbc_ext)) {
          ue = assemble_string(code, codeLen, filename, md5);
          if (BuiltinSymbols::s_systemAr) {
            ue->m_filepath = makeStaticString(
              "/:" + ue->m_filepath->toCppString());
            BuiltinSymbols::s_systemAr->addHhasFile(std::move(ue));
            ue = assemble_string(code, codeLen, filename, md5);
          }
        }
      }
    }

    // maybe run external compilers, but not until we've done all of systemlib
    if (SystemLib::s_inited) {
      auto const hcMode = hackc_mode();

      // Use the PHP7 compiler if it is configured and the file is PHP
      if (RuntimeOption::EvalPHP7CompilerEnabled &&
          !RuntimeOption::EnableHipHopSyntax &&
          !isFileHack(code, codeLen)) {
        auto res = php7_compile(code, codeLen, filename, md5);
        match<void>(res,
          [&] (std::unique_ptr<Unit>& u) {
            unit = std::move(u);
          },
          [&] (std::string& err) {
            ue = createFatalUnit(
              makeStaticString(filename),
              md5,
              FatalOp::Runtime,
              makeStaticString(err)
            );
          }
        );
      // otherwise, use hackc if we're allowed to try
      } else if (hcMode != HackcMode::kNever) {
        auto res = hackc_compile(code, codeLen, filename, md5);
        match<void>(
          res,
          [&] (std::unique_ptr<Unit>& u) {
            unit = std::move(u);
          },
          [&] (std::string& err) {
            if (hcMode == HackcMode::kFallback) return;
            ue = createFatalUnit(
              makeStaticString(filename),
              md5,
              FatalOp::Runtime,
              makeStaticString(err)
            );
          }
        );
      }
    }

    if (!ue && !unit) {
      auto parseit = [=] (AnalysisResultPtr ar) {
        Scanner scanner(code, codeLen,
                        RuntimeOption::GetScannerType(), filename);
        Parser parser(scanner, filename, ar, codeLen);
        parser.parse();
        return parser.getFileScope();
      };

      if (BuiltinSymbols::s_systemAr) {
        parseit(BuiltinSymbols::s_systemAr)->setMd5(md5);
      }

      auto ar = std::make_shared<AnalysisResult>();
      FileScopePtr fsp = parseit(ar);
      fsp->setOuterScope(ar);

      ar->setPhase(AnalysisResult::AnalyzeAll);
      fsp->analyzeProgram(ar);

      ue.reset(emitHHBCUnitEmitter(ar, fsp, md5));
    }

    if (!unit) {
      // NOTE: Repo errors are ignored!
      Repo::get().commitUnit(ue.get(), unitOrigin);

      unit = ue->create();
      ue.reset();
    }

    if (unit->sn() == -1) {
      // the unit was not committed to the Repo, probably because
      // another thread did it first. Try to use the winner.
      auto u = Repo::get().loadUnit(filename ? filename : "", md5);
      if (u != nullptr) {
        return u.release();
      }
    }
    return unit.release();
  } catch (const std::exception&) {
    // extern "C" function should not be throwing exceptions...
    return nullptr;
  }
}

} // extern "C"

///////////////////////////////////////////////////////////////////////////////
}
}
