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

#ifndef incl_HPHP_VM_FUNC_H_
#define incl_HPHP_VM_FUNC_H_

#include "hphp/runtime/base/atomic-countable.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/tracing.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/user-attributes.h"

#include "hphp/runtime/vm/indexed-string-map.h"
#include "hphp/runtime/vm/iter.h"
#include "hphp/runtime/vm/reified-generics-info.h"
#include "hphp/runtime/vm/rx.h"
#include "hphp/runtime/vm/type-constraint.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/util/fixed-vector.h"
#include "hphp/util/low-ptr.h"

#include <atomic>
#include <utility>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const StaticString s___call;

struct ActRec;
struct Class;
struct NamedEntity;
struct PreClass;
struct StringData;
struct StructuredLogEntry;
template <typename T> struct AtomicVector;

/*
 * Signature for native functions called by the hhvm using the hhvm
 * calling convention that provides raw access to the ActRec.
 */
using ArFunction = TypedValue* (*)(ActRec* ar);

/*
 * Signature for native functions expecting the platform ABI calling
 * convention. This must always be casted to a proper signature before
 * calling, so make something up to prevent accidental mixing with other
 * function pointer types.
 */
struct NativeArgs; // never defined
using NativeFunction = void(*)(NativeArgs*);

/*
 * Vector of pairs (param index, offset of corresponding DV funclet).
 */
using DVFuncletsVec = std::vector<std::pair<int, Offset>>;

///////////////////////////////////////////////////////////////////////////////
// EH table.

/*
 * Exception handler table entry.
 */
struct EHEnt {
  Offset m_base;
  Offset m_past;
  int m_iterId;
  int m_parentIndex;
  Offset m_handler;
  Offset m_end;

  EHEnt()
    : m_base()
    , m_past()
    , m_iterId()
    , m_parentIndex()
    , m_handler()
    , m_end()
  {}

  template<class SerDe> void serde(SerDe& sd);
};

///////////////////////////////////////////////////////////////////////////////
/*
 * Metadata about a PHP function or method.
 *
 * The Func class cannot be safely extended, because variable amounts of memory
 * associated with the Func are allocated before and after the actual object.
 *
 * All Funcs are also followed by a variable number of function prologue
 * pointers.  Six are statically allocated as part of the Func object, but more
 * may follow, depending on the value of getMaxNumPrologues().
 *
 *              +--------------------------------+ Func* address
 *              |  Func object                   |
 *              |                                |
 *              |  prologues at end of Func      |
 *              +--------------------------------+ Func* address
 *              |  [additional prologues]        |
 *              +--------------------------------+ high address
 *
 */
struct Func final {
  friend struct FuncEmitter;

  /////////////////////////////////////////////////////////////////////////////
  // Types.

  /*
   * Parameter default value info.
   */
  struct ParamInfo {
    enum class Flags {
      InOut,      // Is this an `inout' parameter?
      Variadic,   // Is this a `...' parameter?
      NativeArg,  // Does this use a NativeArg?
    };

    ParamInfo();

    bool hasDefaultValue() const;
    bool hasScalarDefaultValue() const;
    bool isInOut() const;
    bool isVariadic() const;
    bool isNativeArg() const;
    void setFlag(Flags flag);

    template<class SerDe> void serde(SerDe& sd);

    // Typehint for builtins.
    MaybeDataType builtinType{folly::none};
    // Flags as defined by the Flags enum.
    uint8_t flags{0};
    // DV initializer funclet offset.
    Offset funcletOff{kInvalidOffset};
    // Set to Uninit if there is no DV, or if there's a nonscalar DV.
    TypedValue defaultValue;
    // Eval-able PHP code.
    LowStringPtr phpCode{nullptr};
    // User-annotated type.
    LowStringPtr userType{nullptr};
    // offset of dvi funclet from cti section base.
    Offset ctiFunclet{kInvalidOffset};
    TypeConstraint typeConstraint;
    UserAttributeMap userAttributes;
  };

  /*
   * Static variable info.
   */
  struct SVInfo {
    template<class SerDe> void serde(SerDe& sd) { sd(name); }

    LowStringPtr name;
  };

  using ParamInfoVec = VMFixedVector<ParamInfo>;
  using SVInfoVec = VMFixedVector<SVInfo>;
  using EHEntVec = VMFixedVector<EHEnt>;
  using UpperBoundVec = VMCompactVector<TypeConstraint>;
  using ParamUBMap = vm_flat_map<uint32_t, UpperBoundVec>;

  /////////////////////////////////////////////////////////////////////////////
  // Creation and destruction.

  Func(Unit& unit, const StringData* name, Attr attrs);
  Func(Unit& unit, const StringData* name, Attr attrs,
    const StringData *methCallerCls, const StringData *methCallerMeth);
  ~Func();

  /*
   * Allocate memory for a function, including the variable number of prologues
   * that follow.
   */
  static void* allocFuncMem(int numParams);

  /*
   * Destruct and free a Func*.
   */
  static void destroy(Func* func);

  /*
   * Address of the end of the Func's variable-length memory allocation.
   */
  const void* mallocEnd() const;

  /*
   * Duplicate this function.
   *
   * Funcs are cloned for a number of reasons---most notably, methods on
   * Classes are cloned from the methods defined on their respective
   * PreClasses.
   *
   * We also clone methods from traits when we transclude the trait in its user
   * Classes in repo mode.
   */
  Func* clone(Class* cls, const StringData* name = nullptr) const;

  /*
   * Reset this function's cls and attrs.
   *
   * Used to change the Class scope of a closure method.
   */
  void rescope(Class* ctx);

  /*
   * Free up a PreFunc for re-use as a cloned Func.
   *
   * @requires: isPreFunc()
   */
  void freeClone();

  /*
   * Verify that a Func's data is coherent.
   *
   * FIXME: Currently this method does almost nothing.
   */
  bool validate() const;

  /////////////////////////////////////////////////////////////////////////////
  // FuncId manipulation.

  /*
   * Get this function's ID.
   *
   * We allocate a unique 32-bit ID to almost all Funcs.  The Func* can be
   * retrieved by using this ID as an index into a global vector.  This lets
   * the JIT store references to Funcs more compactly.
   *
   * Funcs which do not represent actual runtime functions (namely, Funcs on
   * PreClasses) are not assigned an ID.
   */
  FuncId getFuncId() const;

  /*
   * Reserve the next available FuncId for `this', and add `this' to the
   * function table.
   */
  void setNewFuncId();

  /*
   * The next available FuncId.  For observation only; does not reserve.
   */
  static FuncId nextFuncId();

  /*
   * Lookup a Func* by its ID.
   */
  static const Func* fromFuncId(FuncId id);

  /*
   * Whether `id' actually keys a Func*.
   */
  static bool isFuncIdValid(FuncId id);

  /////////////////////////////////////////////////////////////////////////////
  // Basic info.                                                        [const]

  /*
   * Whether this function was defined in a pseudomain.
   */
  bool top() const;

  /*
   * The Unit the function is defined in.
   */
  Unit* unit() const;

  /*
   * The various Class contexts of a method.
   *
   * cls():       The Class context of the method.  This is usually the Class
   *              which implements the method, but for closure methods (i.e.,
   *              the __invoke() method on a closure object), it is instead the
   *              Class that the Closure object is scoped to.
   *
   * preClass():  The PreClass of the method's cls().  For closures, this still
   *              corresponds to the Closure subclass, rather than to the
   *              scoped Class.
   *
   *              When isFromTrait() is true, preClass() refers to different
   *              entities in repo vs. non-repo mode.  In repo mode, traits are
   *              flattened ahead of time, and preClass() refers to the class
   *              which imported the trait.  In non-repo mode, trait methods
   *              are cloned into trait users, but preClass() will still refer
   *              to the trait which defined the method.
   *
   * baseCls():   The first Class in the inheritance hierarchy which declares
   *              this method.
   *
   * implCls():   The Class which implements the method.  Just like cls(), but
   *              ignores closure scope (so it returns baseCls() for closures).
   *
   * It is possible for cls() to be nullptr on a method---this occurs when a
   * closure method is scoped to a null class context (e.g., if the closure is
   * created in a non-method function scope).  In this case, only the `cls' is
   * changed; the `preClass' and `baseCls' will continue to refer to the
   * PreClass and Class of the closure object.
   *
   * The converse also occurs---a function can have a `cls' (and `baseCls')
   * without being a method.  This happens when a pseudomain is included from a
   * class context.
   *
   * Consequently, none of these methods should be used to test whether the
   * function is a method; for that purpose, see isMethod().
   */
  Class* cls() const;
  PreClass* preClass() const;
  bool hasBaseCls() const;
  Class* baseCls() const;
  Class* implCls() const;

  /*
   * The function's short name (e.g., foo).
   */
  const StringData* name() const;
  StrNR nameStr() const;

  /*
   * The function's fully class-qualified, name (e.g., C::foo).
   */
  const StringData* fullName() const;
  StrNR fullNameStr() const;

  /*
   * The function's named entity.  Only valid for non-methods.
   *
   * @requires: shared()->m_preClass == nullptr
   */
  NamedEntity* getNamedEntity();
  const NamedEntity* getNamedEntity() const;

  /**
   * meth_caller
   */
  const StringData* methCallerClsName() const;
  const StringData* methCallerMethName() const;

  /////////////////////////////////////////////////////////////////////////////
  // File info.                                                         [const]

  /*
   * The filename where the function was originally defined.
   *
   * In repo mode, we flatten traits into the classes they're used in, so we
   * need this to track the original file for backtraces and errors.
   */
  const StringData* originalFilename() const;

  /*
   * The original filename if it is defined, the unit's filename otherwise.
   */
  const StringData* filename() const;

  /*
   * Start and end line of the function.
   *
   * It'd be nice if these were called lineStart and lineEnd or something, but
   * we're not allowed to have nice things.
   */
  int line1() const;
  int line2() const;

  /*
   * The system- or user-defined doc comment accompanying the function.
   */
  const StringData* docComment() const;

  /////////////////////////////////////////////////////////////////////////////
  // Bytecode.                                                          [const]

  /*
   * Get the function's main entrypoint.
   */
  PC getEntry() const;

  /*
   * Get the offsets of the start (base) and end (past) of the function's
   * bytecode, relative to the start of the unit.
   */
  Offset base() const;
  Offset past() const;

  /*
   * Whether a given PC or Offset (from the beginning of the unit) is within
   * the function's bytecode stream.
   */
  bool contains(PC pc) const;
  bool contains(Offset offset) const;

  /*
   * Return a vector of pairs of (param index, corresponding DV funclet
   * offset).
   */
  DVFuncletsVec getDVFunclets() const;

  /*
   * Is there a main or default value entrypoint at the given offset?
   */
  bool isEntry(Offset offset) const;
  bool isDVEntry(Offset offset) const;

  /*
   * Number of params required when entering at the given offset.
   *
   * Return -1 if an invalid offset is provided.
   */
  int getEntryNumParams(Offset offset) const;
  int getDVEntryNumParams(Offset offset) const;

  /*
   * Get the correct entrypoint (whether the main entry or a DV funclet) when
   * `numArgsPassed' arguments are passed to the function.
   *
   * This is the DV funclet offset of the numArgsPassed-th parameter, or the
   * next parameter that has a DV funclet.
   */
  Offset getEntryForNumArgs(int numArgsPassed) const;

  // CTI entry points
  Offset ctiEntry() const;
  void setCtiFunclet(int i, Offset);
  void setCtiEntry(Offset entry, uint32_t size);

  /////////////////////////////////////////////////////////////////////////////
  // Return type.                                                       [const]

  /*
   * CPP builtin's return type. Returns folly::none if function is not a CPP
   * builtin.
   *
   * There are a number of caveats regarding this value:
   *
   *    - If the return type is folly::none, the return is a Variant.
   *
   *    - If the return type is a string, array-like, object, ref, or resource
   *      type, null may also be returned.
   *
   *    - Likewise, if the function is marked AttrParamCoerceModeNull, null
   *      might also be returned.
   *
   *    - This list of caveats may be incorrect and/or incomplete.
   */
  MaybeDataType hniReturnType() const;

  /*
   * Return type inferred by HHBBC's static analysis. TGen if no data is
   * available.
   */
  RepoAuthType repoReturnType() const;

  /*
   * For async functions, the statically inferred inner type of the returned
   * WH based on HHBBC's analysis.
   */
  RepoAuthType repoAwaitedReturnType() const;

  /*
   * For builtins, whether the return value is returned in registers (as
   * opposed to indirect return, via tvBuiltinReturn).
   *
   * Not well-defined if this function is not a builtin.
   */
  bool isReturnByValue() const;

  /*
   * The TypeConstraint of the return.
   */
  const TypeConstraint& returnTypeConstraint() const;

  /*
   * The user-annotated Hack return type.
   */
  const StringData* returnUserType() const;

  bool hasReturnWithMultiUBs() const;
  const UpperBoundVec& returnUBs() const;

  /////////////////////////////////////////////////////////////////////////////
  // Parameters.                                                        [const]

  /*
   * Const reference to the parameter info table.
   *
   * ParamInfo objects pulled from the table will also be const.
   */
  const ParamInfoVec& params() const;

  /*
   * Number of parameters (including `...') accepted by the function.
   */
  uint32_t numParams() const;

  /*
   * Number of parameters, not including `...', accepted by the function.
   */
  uint32_t numNonVariadicParams() const;

  /*
   * Number of required parameters, i.e. all arguments starting from
   * the returned position have default value.
   */
  uint32_t numRequiredParams() const;

  /*
   * Whether the function is declared with a `...' parameter.
   */
  bool hasVariadicCaptureParam() const;

  /*
   * Whether the arg-th parameter was declared inout.
   */
  bool isInOut(int32_t arg) const;

  /*
   * Whether any of the parameters to this function are inout parameters.
   */
  bool takesInOutParams() const;

  /*
   * Returns the number of inout parameters taken by func.
   */
  uint32_t numInOutParams() const;

  bool hasParamsWithMultiUBs() const;

  const ParamUBMap& paramUBs() const;

  /////////////////////////////////////////////////////////////////////////////
  // Locals, iterators, and stack.                                      [const]

  /*
   * Number of locals, iterators, or named locals.
   */
  int numLocals() const;
  int numIterators() const;
  Id numNamedLocals() const;

  /*
   * Find the integral ID assigned to a named local.
   */
  Id lookupVarId(const StringData* name) const;

  /*
   * Find the name of the local with the given ID.
   */
  const StringData* localVarName(Id id) const;

  /*
   * Array of named locals.  Includes parameter names.
   * May contain nullptrs for unammed locals that mixed in with named ones.
   *
   * Should not be indexed past numNamedLocals() - 1.
   */
  LowStringPtr const* localNames() const;

  /*
   * Number of stack slots used by locals and iterator cells.
   */
  int numSlotsInFrame() const;

  /*
   * Access to the maximum stack cells this function can use.  This is
   * used for stack overflow checks.
   *
   * The maximum cells for a function includes all its locals, all cells
   * for its iterators, and all temporary eval stack slots. It does not
   * include its own ActRec, because whoever called it must have(+) included
   * the stack slot space reserved for this ActRec.  The reason it must still
   * count its parameter locals is that the caller may or may not pass any of
   * the parameters, regardless of how many are declared.
   *
   *   + Except in a re-entry situation.  That must be handled
   *     specially in bytecode.cpp.
   */
  int maxStackCells() const;

  /*
   * Checks if $this belong to a class that is not a subclass of cls().
   */
  bool hasForeignThis() const;

  void setHasForeignThis(bool);

  /////////////////////////////////////////////////////////////////////////////
  // Definition context.                                                [const]

  /*
   * Is the function a pseudomain (i.e., the function implicitly defined by the
   * text after <?php in a file)?
   */
  bool isPseudoMain() const;

  /*
   * Is this function a method defined on a class?
   *
   * Note that trait methods may not satisfy isMethod().
   */
  bool isMethod() const;

  /*
   * Was this function imported from a trait?
   *
   * Note that this returns false for a trait method in the trait it was
   * originally declared.
   */
  bool isFromTrait() const;

  /*
   * Is this function declared with `public', `static', or `abstract'?
   */
  bool isPublic() const;
  bool isStatic() const;
  bool isAbstract() const;

  /*
   * Whether a function is called non-statically. Generally this means
   * isStatic(), but eg static closures are still called with a valid
   * this pointer.
   */
  bool isStaticInPrologue() const;

  /*
   * Whether a method is guaranteed to have a valid this in the body.
   * A method which is !isStatic() || isClosureBody() is guaranteed to
   * be called with a valid this, but closures swap out the closure
   * object for the closure context in the prologue, so may not have
   * a this in the body.
   */
  bool hasThisInBody() const;

  /*
   * Is this Func owned by a PreClass?
   *
   * A PreFunc may be "adopted" by a Class when clone() is called, but only the
   * owning PreClass is allowed to free it.
   */
  bool isPreFunc() const;

  /*
   * Is this func a memoization wrapper?
   */
  bool isMemoizeWrapper() const;

  /*
   * Is this func a memoization wrapper with LSB parameter set?
   */
  bool isMemoizeWrapperLSB() const;

  /*
   * Is this string the name of a memoize implementation.
   */
  static bool isMemoizeImplName(const StringData*);

  /*
   * Is this function a memoization implementation.
   */
  bool isMemoizeImpl() const;

  /*
   * Assuming this func is a memoization wrapper, the name of the function it is
   * wrapping.
   *
   * Pre: isMemoizeWrapper()
   */
  const StringData* memoizeImplName() const;

  /*
   * Given the name of a memoization wrapper function, return the generated name
   * of the function it wraps. This is static so it can be used in contexts
   * where the actual Func* is not available.
   */
  static const StringData* genMemoizeImplName(const StringData*);

  /*
   * Given a meth_caller, return the class name or method name
   */
   static std::pair<const StringData*, const StringData*> getMethCallerNames(
     const StringData* name);

  /////////////////////////////////////////////////////////////////////////////
  // Builtins.                                                          [const]

  /*
   * Is the function a builtin, whether PHP or C++?
   */
  bool isBuiltin() const;

  /*
   * Is this function a C++ builtin (ie HNI function)?.
   *
   * @implies: isBuiltin()
   */
  bool isCPPBuiltin() const;

  /*
   * The function returned by arFuncPtr() takes an ActRec*, unpacks it,
   * and usually dispatches to a nativeFuncPtr() with a specific signature.
   *
   * All C++ builtins have an ArFunction, with no exceptions.
   *
   * Most HNI functions share a single ArFunction, which performs
   * unpacking and dispatch. The exception is HNI functions declared
   * with NeedsActRec, which do not have NativeFunctions, but have unique
   * ArFunctions which do all their work.
   */
  ArFunction arFuncPtr() const;

  /*
   * The nativeFuncPtr is a type-punned function pointer to the unerlying
   * function which takes the actual argument types, and does the actual work.
   *
   * These are the functions with names prefixed by f_ or t_.
   *
   * All C++ builtins have NativeFunctions, with the ironic exception of HNI
   * functions declared with NeedsActRec.
   */
  NativeFunction nativeFuncPtr() const;

  /////////////////////////////////////////////////////////////////////////////
  // Closures.                                                          [const]

  /*
   * Is this function the body (i.e., __invoke() method) of a Closure object?
   *
   * (All PHP anonymous functions are Closure objects.)
   */
  bool isClosureBody() const;

  /////////////////////////////////////////////////////////////////////////////
  // Resumables.                                                        [const]

  /*
   * Is this function asynchronous?  (May also be a generator.)
   */
  bool isAsync() const;

  /*
   * Is this function a generator?  (May also be async.)
   */
  bool isGenerator() const;

  /*
   * Is this function a generator which yields both key and value?
   *
   * @implies: isGenerator()
   */
  bool isPairGenerator() const;

  /*
   * @returns: !isGenerator() && isAsync()
   */
  bool isAsyncFunction() const;

  /*
   * @returns: isGenerator() && !isAsync()
   */
  bool isNonAsyncGenerator() const;

  /*
   * @returns: isGenerator() && isAsync()
   */
  bool isAsyncGenerator() const;

  /*
   * Is this a resumable function?
   *
   * @returns: isGenerator() || isAsync()
   */
  bool isResumable() const;

  /////////////////////////////////////////////////////////////////////////////
  // Reactivity.                                                        [const]

  /*
   * What is the level of reactivity of this function?
   */
  RxLevel rxLevel() const;

  /*
   * Is this the version of the function body with reactivity disabled via
   * if (Rx\IS_ENABLED) ?
   */
  bool isRxDisabled() const;

  /*
   * Is this function conditionally reactive?
   */
  bool isRxConditional() const;

  /////////////////////////////////////////////////////////////////////////////
  // Methods.                                                           [const]

  /*
   * Index of this function in the method table of its Class.
   */
  Slot methodSlot() const;

  /*
   * Whether this function has a private implementation on a parent class.
   */
  bool hasPrivateAncestor() const;

  /////////////////////////////////////////////////////////////////////////////
  // Magic methods.                                                     [const]

  /*
   * Is this a compiler-generated function?
   *
   * This includes special methods like 86pinit and 86sinit as well
   * as all closures.
   */
  bool isGenerated() const;

  /*
   * Is `name' the name of a special initializer function?
   */
  static bool isSpecial(const StringData* name);

  /////////////////////////////////////////////////////////////////////////////
  // Persistence.                                                       [const]

  /*
   * Whether this function is uniquely named across the codebase.
   *
   * It's legal in PHP to define multiple functions in different pseudomains
   * with the same name, so long as both are not required in the same request.
   *
   * Note that if EvalJitEnableRenameFunction is set, no Func is unique.
   */
  bool isUnique() const;

  /*
   * Whether we can load this function once and persist it across requests.
   *
   * Persistence is possible when a Func is defined in a pseudomain that has no
   * side-effects (except other persistent definitions).
   *
   * @implies: isUnique()
   */
  bool isPersistent() const;

  bool isInterceptable() const;

  /*
   * Given that func would be called when func->name() is invoked on cls,
   * determine if it would also be called when invoked on any descendant
   * of cls.
   */
  bool isImmutableFrom(const Class* cls) const;

  /////////////////////////////////////////////////////////////////////////////
  // Other attributes.                                                  [const]

  /*
   * Get the system attributes of the function.
   */
  Attr attrs() const;

  /*
   * Get the user-declared attributes of the function.
   */
  const UserAttributeMap& userAttributes() const;

  /*
   * Whether to ignore this function's frame in backtraces.
   */
  bool isNoInjection() const;

  /*
   * Whether this function's frame should be skipped when searching for context
   * (e.g., array_map evaluates its callback in the context of its caller).
   */
  bool isSkipFrame() const;

  /*
   * Whether this function's frame should be skipped with searching for a
   * context for array provenance
   */
  bool isProvenanceSkipFrame() const;

  /*
   * Whether the function can be constant-folded at callsites where it is
   * passed constant arguments.
   */
  bool isFoldable() const;

  /*
   * Supports async eager return optimization?
   */
  bool supportsAsyncEagerReturn() const;

  /*
   * Is this func allowed to be called dynamically?
   */
  bool isDynamicallyCallable() const;

  /*
   * Is this a meth_caller func?
   */
  bool isMethCaller() const;

  /*
   * Indicates that a function does not make any explicit calls to other PHP
   * functions.  It may still call other user-level functions via re-entry
   * (e.g., for autoload), and it may make calls to builtins using FCallBuiltin.
   */
  bool isPhpLeafFn() const;

  /*
   * Does this function has reified generics?
   */
  bool hasReifiedGenerics() const;

  /*
   * Returns a ReifiedGenericsInfo containing how many generics this func has,
   * indices of its reified generics, and which ones are soft reified
   */
  const ReifiedGenericsInfo& getReifiedGenericsInfo() const;

  /////////////////////////////////////////////////////////////////////////////
  // Unit table entries.                                                [const]

  const EHEntVec& ehtab() const;

  /*
   * Find the first EHEnt that covers a given offset, or return null.
   */
  const EHEnt* findEH(Offset o) const;

  /*
   * Same as non-static findEH(), but takes as an operand any ehtab-like
   * container.
   */
  template<class Container>
  static const typename Container::value_type*
  findEH(const Container& ehtab, Offset o);

  bool shouldSampleJit() const { return m_shouldSampleJit; }

  /////////////////////////////////////////////////////////////////////////////
  // JIT data.

  /*
   * Get the RDS handle for the function with this function's name.
   *
   * We can burn these into the TC even when functions are not persistent,
   * since only a single name-to-function mapping will exist per request.
   */
  rds::Handle funcHandle() const;

  /*
   * Get, set and reset the function body code pointer.
   */
  unsigned char* getFuncBody() const;
  void setFuncBody(unsigned char* fb);
  void resetFuncBody();

  /*
   * Get and set the `index'-th function prologue.
   */
  uint8_t* getPrologue(int index) const;
  void setPrologue(int index, unsigned char* tca);

  /*
   * Number of prologues allocated for the function.
   */
  int numPrologues() const;

  /*
   * Reset a specific prologue, or all prologues.
   */
  void resetPrologue(int numParams);

  /////////////////////////////////////////////////////////////////////////////
  // Pretty printer.                                                    [const]

  struct PrintOpts {
    PrintOpts()
      : metadata(true)
    {}

    PrintOpts& noMetadata() {
      metadata = false;
      return *this;
    }

    bool metadata;
  };

  void prettyPrint(std::ostream& out, const PrintOpts& = PrintOpts()) const;

  /*
   * Print function attributes to out.
   */
  static void print_attrs(std::ostream& out, Attr attrs);


  /////////////////////////////////////////////////////////////////////////////
  // Other methods.
  //
  // You should avoid adding methods to this section.  If the logic you're
  // implementing is specific to a particular subsystem, define it as a helper
  // there instead.
  //
  // If you absolutely must add more methods to Func here, just follow these
  // simple guidelines:
  //
  //    (1) Don't add more methods to Func here.

  /*
   * Intercept hook flag.
   */
  int8_t& maybeIntercepted() const;

  /*
   * Access to the global vector of funcs.  This maps FuncID's back to Func*'s.
   */
  static const AtomicVector<const Func*>& getFuncVec();


  /////////////////////////////////////////////////////////////////////////////
  // Public setters.
  //
  // TODO(#4504609): These setters are only used by Class at Class creation
  // time.  We should refactor the creation path into a separate friend module
  // to avoid this garbage.
  //
  // Having public setters here should be avoided, so try not to add any.

  void setAttrs(Attr attrs);
  void setBaseCls(Class* baseCls);
  void setFuncHandle(rds::Link<LowPtr<Func>, rds::Mode::NonLocal> l);
  void setHasPrivateAncestor(bool b);
  void setMethodSlot(Slot s);
  void setGenerated(bool b);

  // Return true, and set the m_serialized flag, iff this Func hasn't
  // been serialized yet (see prof-data-serialize.cpp).
  bool serialize() const;

  /////////////////////////////////////////////////////////////////////////////
  // Offset accessors.                                                 [static]

#define OFF(f)                          \
  static constexpr ptrdiff_t f##Off() { \
    return offsetof(Func, m_##f);       \
  }
  OFF(attrs)
  OFF(name)
  OFF(funcBody)
  OFF(maxStackCells)
  OFF(maybeIntercepted)
  OFF(paramCounts)
  OFF(prologueTable)
  OFF(inoutBitVal)
  OFF(shared)
  OFF(unit)
  OFF(methCallerMethName)
#undef OFF

  static constexpr ptrdiff_t clsOff() {
    return offsetof(Func, m_u);
  }

  static constexpr ptrdiff_t methCallerClsNameOff() {
    return offsetof(Func, m_u);
  }

  static constexpr ptrdiff_t sharedBaseOff() {
    return offsetof(SharedData, m_base);
  }

  static constexpr ptrdiff_t sharedInOutBitPtrOff() {
    return offsetof(SharedData, m_inoutBitPtr);
  }


  /////////////////////////////////////////////////////////////////////////////
  // SharedData.

private:
  using NamedLocalsMap = IndexedStringMap<LowStringPtr, true, Id>;

  // Some 16-bit values in SharedData are stored as small deltas if they fit
  // under this limit.  If not, they're set to the limit value and an
  // ExtendedSharedData will be allocated for the full-width field.
  static constexpr auto kSmallDeltaLimit = uint16_t(-1);

  /*
   * Properties shared by all clones of a Func.
   */
  struct SharedData : AtomicCountable {
    SharedData(PreClass* preClass, Offset base, Offset past,
               int line1, int line2, bool top, bool isPhpLeafFn,
               const StringData* docComment);
    ~SharedData();

    /*
     * Interface for AtomicCountable.
     */
    void atomicRelease();

    /*
     * Data fields are packed to minimize size.  Try not to add anything new
     * here or reorder anything.
     */
    // (There's a 32-bit integer in the AtomicCountable base class here.)
    Offset m_base;
    PreClass* m_preClass;
    Id m_numLocals;
    Id m_numIterators;
    int m_line1;
    LowStringPtr m_docComment;
    // Bits 64 and up of the inout-ness guards (the first 64 bits are in
    // Func::m_inoutBitVal for faster access).
    uint64_t* m_inoutBitPtr;
    ParamInfoVec m_params;
    NamedLocalsMap m_localNames;
    EHEntVec m_ehtab;

    /*
     * Up to 32 bits.
     */
    bool m_top : 1;
    bool m_isClosureBody : 1;
    bool m_isAsync : 1;
    bool m_isGenerator : 1;
    bool m_isPairGenerator : 1;
    bool m_isGenerated : 1;
    bool m_hasExtendedSharedData : 1;
    bool m_returnByValue : 1; // only for builtins
    bool m_isMemoizeWrapper : 1;
    bool m_isMemoizeWrapperLSB : 1;
    bool m_isPhpLeafFn : 1;
    bool m_hasReifiedGenerics : 1;
    bool m_isRxDisabled : 1;
    bool m_hasParamsWithMultiUBs : 1;
    bool m_hasReturnWithMultiUBs : 1;

    // 16 bits of padding here in LOWPTR builds

    LowStringPtr m_retUserType;
    UserAttributeMap m_userAttributes;
    TypeConstraint m_retTypeConstraint; // NB: sizeof(TypeConstraint) == 12
    LowStringPtr m_originalFilename;
    RepoAuthType m_repoReturnType;
    RepoAuthType m_repoAwaitedReturnType;

    /*
     * The `past' offset and `line2' are likely to be small, particularly
     * relative to m_base and m_line1, so we encode each as a 16-bit
     * difference.
     *
     * If the delta doesn't fit, we need to have an ExtendedSharedData to hold
     * the real values---in that case, the field here that overflowed is set to
     * kSmallDeltaLimit and the corresponding field in ExtendedSharedData will
     * be valid.
     */
    uint16_t m_line2Delta;
    uint16_t m_pastDelta;

    std::atomic<Offset> m_cti_base; // relative to CodeCache cti section
    uint32_t m_cti_size; // size of cti code
    // 32 bits free here.
  };

  /*
   * If this Func represents a native function or is exceptionally large
   * (line count or bytecode size), it requires extra information that most
   * Funcs don't need, so it's SharedData is actually one of these extended
   * SharedDatas.
   */
  struct ExtendedSharedData : SharedData {
    template<class... Args>
    explicit ExtendedSharedData(Args&&... args)
      : SharedData(std::forward<Args>(args)...)
    {
      m_hasExtendedSharedData = true;
    }
    ExtendedSharedData(const ExtendedSharedData&) = delete;
    ExtendedSharedData(ExtendedSharedData&&) = delete;

    MaybeDataType m_hniReturnType;
    ArFunction m_arFuncPtr;
    NativeFunction m_nativeFuncPtr;
    ReifiedGenericsInfo m_reifiedGenericsInfo;
    ParamUBMap m_paramUBs;
    UpperBoundVec m_returnUBs;
    Offset m_past;  // Only read if SharedData::m_pastDelta is kSmallDeltaLimit
    int m_line2;    // Only read if SharedData::m_line2 is kSmallDeltaLimit
  };

  /*
   * SharedData accessors for internal use.
   */
  const SharedData* shared() const { return m_shared.get(); }
        SharedData* shared()       { return m_shared.get(); }

  /*
   * Returns ExtendedSharedData if we have one, or else a nullptr.
   */
  const ExtendedSharedData* extShared() const;
        ExtendedSharedData* extShared();


  /////////////////////////////////////////////////////////////////////////////
  // Internal methods.
  //
  // These are all used at emit-time, and should be outsourced to FuncEmitter.

private:
  Func(const Func&) = default;  // used for clone()
  Func& operator=(const Func&) = delete;
  void init(int numParams);
  void initPrologues(int numParams);
  void setFullName(int numParams);
  void appendParam(bool ref, const ParamInfo& info,
                   std::vector<ParamInfo>& pBuilder);
  void finishedEmittingParams(std::vector<ParamInfo>& pBuilder);
  void setNamedEntity(const NamedEntity*);

  /////////////////////////////////////////////////////////////////////////////
  // Internal types.

  struct ClonedFlag {
    ClonedFlag() {}
    ClonedFlag(const ClonedFlag&) {}
    ClonedFlag& operator=(const ClonedFlag&) = delete;

    std::atomic_flag flag = ATOMIC_FLAG_INIT;
  };

  /*
   * Wrapper around std::atomic<Attr> that pretends like it's not atomic.
   *
   * Func::m_attrs is only accessed by multiple threads in the closure scoping
   * process for Closure classes, which is synchronized in Class::rescope().
   * This wrapper is just to make m_attrs copy-constructible, and there should
   * never be a race when copying.
   */
  struct AtomicAttr {
    AtomicAttr() {}
    explicit AtomicAttr(Attr attrs) : m_attrs{attrs} {}

    AtomicAttr(const AtomicAttr& o)
      : m_attrs{o.m_attrs.load(std::memory_order_relaxed)}
    {}

    AtomicAttr& operator=(Attr attrs) {
      m_attrs.store(attrs, std::memory_order_relaxed);
      return *this;
    }

    /* implicit */ operator Attr() const {
      return m_attrs.load(std::memory_order_relaxed);
    }

  private:
    std::atomic<Attr> m_attrs;
  };

public:
#ifdef USE_LOWPTR
  using low_storage_t = uint32_t;
#else
  using low_storage_t = uintptr_t;
#endif

private:
  /*
   * Lowptr wrapper around std::atomic<Union> for Class* or StringData*
   */
  struct UnionWrapper {
    union U {
     low_storage_t m_cls;
     low_storage_t m_methCallerClsName;
    };
    std::atomic<U> m_u;

    // constructors
    explicit UnionWrapper(Class *cls)
      : m_u([](Class *cls){
        U u;
        u.m_cls = to_low(cls);
        return u; }(cls)) {}
    explicit UnionWrapper(const StringData *name)
      : m_u([](const StringData *n){
        U u;
        u.m_methCallerClsName = to_low(n, kMethCallerBit);
        return u; }(name)) {}
    /* implicit */ UnionWrapper(std::nullptr_t /*px*/)
      : m_u([](){
        U u;
        u.m_cls = 0;
        return u; }()) {}
    UnionWrapper(const UnionWrapper& r) :
      m_u(r.m_u.load()) {
    }

    // Assignments
    UnionWrapper& operator=(UnionWrapper r) {
      m_u.store(r.m_u, std::memory_order_relaxed);
      return *this;
    }

    // setter & getter
    void setCls(Class *cls) {
      U u;
      u.m_cls = to_low(cls);
      m_u.store(u, std::memory_order_relaxed);
    }
    Class* cls() const {
      auto cls = m_u.load(std::memory_order_relaxed).m_cls;
      assertx(!(cls & kMethCallerBit));
      return reinterpret_cast<Class*>(cls);
    }
    StringData* name() const {
     auto n = m_u.load(std::memory_order_relaxed).m_methCallerClsName;
     assertx(n & kMethCallerBit);
     return reinterpret_cast<StringData*>(n - kMethCallerBit);
    }
  };

  template <class T>
  static Func::low_storage_t to_low(T* px, Func::low_storage_t bit = 0) {
    Func::low_storage_t ones = ~0;
    auto ptr = reinterpret_cast<uintptr_t>(px) | bit;
    always_assert((ptr & ones) == ptr);
    return (Func::low_storage_t)(ptr);
  }

  /////////////////////////////////////////////////////////////////////////////
  // Constants.

private:
  static constexpr int argToQword(int32_t arg) {
    return static_cast<uint32_t>(arg) / kBitsPerQword - 1;
  }
  static constexpr int kBitsPerQword = 64;
  static constexpr int kMagic = 0xba5eba11;
  static constexpr intptr_t kNeedsFullName = 0x1;

public:
  static std::atomic<bool>     s_treadmill;
  static std::atomic<uint32_t> s_totalClonedClosures;

  // To conserve space, we use unions for pairs of mutually exclusive fields
  static auto constexpr kMethCallerBit = 0x1;  // set for m_methCaller
  /////////////////////////////////////////////////////////////////////////////
  // Data members.
  //
  // The fields of Func are organized in reverse order of frequency of use.
  // Do not re-order without checking perf!

private:
#ifndef NDEBUG
  // For asserts only.
  int m_magic;
#endif
  AtomicLowPtr<uint8_t> m_funcBody;
  mutable rds::Link<LowPtr<Func>, rds::Mode::NonLocal> m_cachedFunc;
  FuncId m_funcId{InvalidFuncId};
  mutable AtomicLowPtr<const StringData> m_fullName{nullptr};
  LowStringPtr m_name{nullptr};

  union {
    // The first Class in the inheritance hierarchy that declared this method.
    // Note that this may be an abstract class that did not provide an
    // implementation.
    low_storage_t m_baseCls{0};
    // m_methCallerMethName can be accessed by meth_caller() only
    low_storage_t m_methCallerMethName;
  };

  // m_u is used to represent
  // the Class that provided this method implementation, or
  // the class name provided by meth_caller()
  UnionWrapper m_u{nullptr};
  union {
    Slot m_methodSlot{0};
    LowPtr<const NamedEntity>::storage_type m_namedEntity;
  };
  // Atomically-accessed intercept flag.  -1, 0, or 1.
  // TODO(#1114385) intercept should work via invalidation.
  mutable int8_t m_maybeIntercepted;
  mutable ClonedFlag m_cloned;
  bool m_isPreFunc : 1;
  bool m_hasPrivateAncestor : 1;
  bool m_shouldSampleJit : 1;
  bool m_serialized : 1;
  bool m_hasForeignThis : 1;
  int m_maxStackCells{0};
  uint64_t m_inoutBitVal{0};
  Unit* const m_unit;
  AtomicSharedPtr<SharedData> m_shared;
  // Initialized by Func::finishedEmittingParams.  The least significant bit is
  // 1 if the last param is not variadic; the 31 most significant bits are the
  // total number of params (including the variadic param).
  uint32_t m_paramCounts{0};
  AtomicAttr m_attrs;
  // This must be the last field declared in this structure, and the Func class
  // should not be inherited from.
  AtomicLowPtr<uint8_t> m_prologueTable[1];
};

///////////////////////////////////////////////////////////////////////////////

/*
 * A prologue is identified by the called function and the number of arguments
 * that the prologue handles.
 */
struct PrologueID {
  PrologueID(FuncId funcId, uint32_t nargs)
    : m_funcId(funcId)
    , m_nargs(nargs)
  { }

  PrologueID(const Func* func, uint32_t nargs)
    : m_funcId(func->getFuncId())
    , m_nargs(nargs)
  { }

  PrologueID()
  { }

  FuncId      funcId() const { return m_funcId; }
  uint32_t    nargs()  const { return m_nargs;  }
  const Func* func()   const { return Func::fromFuncId(m_funcId); }

  bool operator==(const PrologueID& other) const {
    return m_funcId == other.m_funcId && m_nargs == other.m_nargs;
  }

  struct Hasher {
    size_t operator()(PrologueID pid) const {
      return pid.funcId() + (size_t(pid.nargs()) << 32);
    }
  };

 private:
  FuncId   m_funcId{InvalidFuncId};
  uint32_t m_nargs{0xffffffff};
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Log meta-information about func. Records attributes, number of locals,
 * parameters, static locals, class ref slots, frame cells, high watermark,
 * and iterators. Does not record function name or class.
 */
void logFunc(const Func* func, StructuredLogEntry& ent);

inline tracing::Props traceProps(const Func* f) {
  return tracing::Props{}.add("func_name", f->fullName());
}

/*
 * Convert a function pointer where a string is needed in some context.
 */
const StringData* funcToStringHelper(const Func* func);

int64_t funcToInt64Helper(const Func* func);

///////////////////////////////////////////////////////////////////////////////

}

#define incl_HPHP_VM_FUNC_INL_H_
#include "hphp/runtime/vm/func-inl.h"
#undef incl_HPHP_VM_FUNC_INL_H_

#endif // incl_HPHP_VM_FUNC_H_
