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

#ifndef incl_HPHP_VM_FUNC_H_
#define incl_HPHP_VM_FUNC_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/atomic-countable.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/user-attributes.h"

#include "hphp/runtime/vm/indexed-string-map.h"
#include "hphp/runtime/vm/type-constraint.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/util/fixed-vector.h"
#include "hphp/util/hash-map-typedefs.h"

#include <atomic>
#include <utility>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const int kNumFixedPrologues = 6;

struct ActRec;
struct Class;
struct NamedEntity;
struct PreClass;
struct StringData;
template <typename T> struct AtomicVector;

/*
 * C++ builtin function type.
 */
using BuiltinFunction = TypedValue* (*)(ActRec* ar);

/*
 * Vector of pairs (param index, offset of corresponding DV funclet).
 */
using DVFuncletsVec = std::vector<std::pair<int, Offset>>;

///////////////////////////////////////////////////////////////////////////////
// EH and FPI tables.

/*
 * Exception handler table entry.
 */
struct EHEnt {
  enum class Type : uint8_t {
    Catch,
    Fault
  };

  Type m_type;
  bool m_itRef;
  Offset m_base;
  Offset m_past;
  int m_iterId;
  int m_parentIndex;
  Offset m_fault;
  FixedVector<std::pair<Id,Offset>> m_catches;
};

/*
 * Function parameter info region table entry.
 */
struct FPIEnt {
  Offset m_fpushOff;
  Offset m_fcallOff;
  Offset m_fpOff; // evaluation stack depth to current frame pointer
  int m_parentIndex;
  int m_fpiDepth;

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
struct Func {
  friend class FuncEmitter;
  template <typename F> friend void scan(const Func&, F&);

  /////////////////////////////////////////////////////////////////////////////
  // Types.

  /*
   * Parameter default value info.
   */
  struct ParamInfo {
    ParamInfo();

    bool hasDefaultValue() const;
    bool hasScalarDefaultValue() const;
    bool isVariadic() const;

    template<class SerDe> void serde(SerDe& sd);

    // Typehint for builtins.
    MaybeDataType builtinType{folly::none};
    // True if this is a `...' parameter.
    bool variadic{false};
    // DV initializer funclet offset.
    Offset funcletOff{InvalidAbsoluteOffset};
    // Set to Uninit if there is no DV, or if there's a nonscalar DV.
    TypedValue defaultValue;
    // Eval-able PHP code.
    LowStringPtr phpCode{nullptr};
    // User-annotated type.
    LowStringPtr userType{nullptr};

    TypeConstraint typeConstraint;
    UserAttributeMap userAttributes;
  };

  /*
   * Static variable info.
   */
  struct SVInfo {
    template<class SerDe> void serde(SerDe& sd) { sd(name)(phpCode); }

    LowStringPtr name;
    LowStringPtr phpCode; // Eval'able PHP code.
  };

  typedef FixedVector<ParamInfo> ParamInfoVec;
  typedef FixedVector<SVInfo> SVInfoVec;
  typedef FixedVector<EHEnt> EHEntVec;
  typedef FixedVector<FPIEnt> FPIEntVec;


  /////////////////////////////////////////////////////////////////////////////
  // Creation and destruction.

  Func(Unit& unit, const StringData* name, Attr attrs);
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
   * Duplicate this function.
   *
   * Funcs are cloned for a number of reasons---most notably, methods on
   * Classes are cloned from the methods defined on their respective
   * PreClasses.
   *
   * We also clone methods from traits when we transclude the trait in its user
   * Classes in repo mode.  Finally, we clone inherited methods that define
   * static locals in order to instantiate new static locals for the child
   * class's copy of the method.
   */
  Func* clone(Class* cls, const StringData* name = nullptr) const;

  /*
   * Reset this function's cls and attrs.
   *
   * Used to change the Class scope of a closure method.
   */
  void rescope(Class* ctx, Attr attrs);

  /*
   * Free up a PreFunc for re-use as a cloned Func.
   *
   * @requires: isPreFunc()
   */
  void freeClone() const;

  /*
   * Rename a function and reload it.
   */
  void rename(const StringData* name);

  /*
   * Verify that a Func's data is coherent.
   *
   * FIXME: Currently this method does almost nothing.
   */
  void validate() const;


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
   * preClass():  The PreClass of the method's cls().  For closures, this still
   *              corresponds to the Closure subclass, rather than to the
   *              scoped Class.
   * baseCls():   The first Class in the inheritance hierarchy which declares
   *              this method.
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
  const NamedEntity* getNamedEntity() const;


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


  /////////////////////////////////////////////////////////////////////////////
  // Return type.                                                       [const]

  /*
   * The function's return type.
   *
   * There are a number of caveats regarding this value:
   *
   *    - If the returnType() is folly::none, the return is a Variant.
   *
   *    - If the returnType() is KindOfString, KindOfArray, or KindOfObject,
   *      null may also be returned.
   *
   *    - If the function is marked with AttrParamCoerceModeFalse, then the
   *      function can also return bool in addition to this type.
   *
   *    - Likewise, if the function is marked AttrParamCoerceModeNull, null
   *      might also be returned.
   *
   *    - This list of caveats may be incorrect and/or incomplete.
   */
  MaybeDataType returnType() const;

  /*
   * Whether this function returns by reference.
   */
  bool isReturnRef() const;

  /*
   * The TypeConstraint of the return.
   */
  const TypeConstraint& returnTypeConstraint() const;

  /*
   * The user-annotated Hack return type.
   */
  const StringData* returnUserType() const;


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
   * Whether the arg-th parameter /may/ be taken by reference.
   */
  bool byRef(int32_t arg) const;

  /*
   * Whether any parameters /may/ be taken by reference.
   */
  bool anyByRef() const;

  /*
   * Whether the arg-th parameter /must/ be taken by reference.
   *
   * Some builtins take positional or variadic arguments only optionally by
   * ref, hence the distinction.
   */
  bool mustBeRef(int32_t arg) const;

  /*
   * Whether the function is declared with a `...' parameter.
   */
  bool hasVariadicCaptureParam() const;

  /*
   * Whether extra arguments passed at call time can be ignored because they
   * are never used.
   */
  bool discardExtraArgs() const;


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
   * The maximum cells for a function includes all its locals, all
   * cells for its iterators, all temporary eval stack slots, and all
   * cells it pushes for ActRecs in FPI regions.  It does not include
   * its own ActRec, because whoever called it must have(+) included
   * the size of the ActRec in an FPI region for itself.  The reason
   * it must still count its parameter locals is that the caller may
   * or may not pass any of the parameters, regardless of how many are
   * declared.
   *
   *   + Except in a re-entry situation.  That must be handled
   *     specially in bytecode.cpp.
   */
  int maxStackCells() const;

  /*
   * Checks if $this belong to a class that is not a subclass of cls().
   */
  bool hasForeignThis() const;


  /////////////////////////////////////////////////////////////////////////////
  // Static locals.                                                     [const]

  /*
   * Const reference to the static variable info table.
   *
   * SVInfo objects pulled from the table will also be const.
   */
  const SVInfoVec& staticVars() const;

  /*
   * Whether the function has any static locals.
   */
  bool hasStaticLocals() const;

  /*
   * Number of static locals declared in the function.
   */
  int numStaticLocals() const;


  /////////////////////////////////////////////////////////////////////////////
  // Definition context.                                                [const]

  /*
   * Is the function a pseudomain (i.e., the function implicitly defined by the
   * text after <?php in a file)?
   */
  bool isPseudoMain() const;

  /*
   * Is this function a method defined on a class?
   */
  bool isMethod() const;

  /*
   * Is this function a method defined on a trait?
   *
   * Note that trait methods may not satisfy isMethod().
   */
  bool isTraitMethod() const;

  /*
   * Is this function declared with `public', `static', or `abstract'?
   */
  bool isPublic() const;
  bool isStatic() const;
  bool isAbstract() const;

  /*
   * Could this function have a valid $this?
   *
   * Instance methods certainly have $this, but pseudomains may as well, if
   * they were included in the context of an instance method definition.
   *
   * Note that closure __invoke() methods that are scoped outside the context
   * of a class (e.g., in a toplevel non-method function) may /not/ have $this.
   */
  bool mayHaveThis() const;

  /*
   * Is this Func owned by a PreClass?
   *
   * A PreFunc may be "adopted" by a Class when clone() is called, but only the
   * owning PreClass is allowed to free it.
   */
  bool isPreFunc() const;


  /////////////////////////////////////////////////////////////////////////////
  // Builtins.                                                          [const]

  /*
   * Is the function a builtin, whether PHP or C++?
   */
  bool isBuiltin() const;

  /*
   * Is this function a C++ builtin?  Maybe IDL- or HNI-defined.
   *
   * @implies: isBuiltin()
   */
  bool isCPPBuiltin() const;

  /*
   * Is this an HNI function?
   *
   * Note that "Native" here refers to a different concept than nativeFuncPtr.
   * In fact, the only functions that may not have nativeFuncPtr's are Native
   * (i.e., HNI) functions declared with NeedsActRec.
   *
   * FIXME(#4497824): This naming is pretty bad.
   */
  bool isNative() const;

  /*
   * The builtinFuncPtr takes an ActRec*, unpacks it, and usually dispatches to
   * a nativeFuncPtr.
   *
   * All C++ builtins have a builtinFuncPtr, with no exceptions.
   *
   * IDL builtins all have distinct builtinFuncPtr's.  Most HNI functions share
   * a single builtinFuncPtr, which performs unpacking and dispatch.  The
   * exception is HNI functions declared with NeedsActRec, which do not have
   * nativeFuncPtr's and have unique builtinFuncPtr's which do all their work.
   */
  BuiltinFunction builtinFuncPtr() const;

  /*
   * The nativeFuncPtr is a type-punned function pointer which takes the actual
   * argument types of a builtin and does the actual work.
   *
   * These are the functions with names prefixed by f_ or t_.
   *
   * All C++ builtins have nativeFuncPtr's, with the ironic exception of HNI
   * (i.e., "Native") functions declared with NeedsActRec.
   */
  BuiltinFunction nativeFuncPtr() const;

  /*
   * Get the MethodInfo object of a builtin.
   *
   * Return null if the function is not a builtin.
   */
  const ClassInfo::MethodInfo* methInfo() const;


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
   * This includes special methods like 86pinit, 86sinit, and 86ctor, as well
   * as all closures.
   */
  bool isGenerated() const;

  /*
   * Is this function __destruct()?
   */
  bool isDestructor() const;

  /*
   * Is this function __call()?
   */
  bool isMagicCallMethod() const;

  /*
   * Is this function __callStatic()?
   */
  bool isMagicCallStaticMethod() const;

  /*
   * Is this function any __call*()?
   */
  bool isMagic() const;

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

  /*
   * Is this always the function that's returned when we look up its name from
   * the context of `fromUnit'?
   *
   * A weaker condition than persistence, since a function is always name
   * binding immutable from the context of the unit in which it is defined.
   * Used to make some translation-time optimizations which make assumptions
   * about where function calls will go.
   */
  bool isNameBindingImmutable(const Unit* fromUnit) const;


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
   * Whether this builtin may be replaced by user-defined functions.
   */
  bool isAllowOverride() const;

  /*
   * Whether this function's frame should be skipped when searching for context
   * (e.g., array_map evaluates its callback in the context of its caller).
   */
  bool isSkipFrame() const;

  /*
   * Whether the function can be constant-folded at callsites where it is
   * passed constant arguments.
   */
  bool isFoldable() const;

  /*
   * Whether the function's return is coerced to the correct type.  If so, it
   * may also return null or bool if coercion fails, depending on the coercion
   * kind.
   */
  bool isParamCoerceMode() const;


  /////////////////////////////////////////////////////////////////////////////
  // Unit table entries.                                                [const]

  const EHEntVec& ehtab() const;
  const FPIEntVec& fpitab() const;

  /*
   * Find the first EHEnt that covers a given offset, or return null.
   */
  const EHEnt* findEH(Offset o) const;

  /*
   * Locate FPI regions by offset.
   */
  const FPIEnt* findFPI(Offset o) const;
  const FPIEnt* findPrecedingFPI(Offset o) const;


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
   * Get and set the function body code pointer.
   */
  unsigned char* getFuncBody() const;
  void setFuncBody(unsigned char* fb);

  /*
   * Get and set the `index'-th function prologue.
   */
  unsigned char* getPrologue(int index) const;
  void setPrologue(int index, unsigned char* tca);

  /*
   * Actual number of prologues allocated for the function.
   *
   * A minimum of kNumFixedPrologues is always allocated.  The result of
   * numPrologues() will always be either that minimum, or the result of
   * getMaxNumPrologues().
   */
  int numPrologues() const;

  /*
   * Maximum number of prologues needed by the function.
   */
  static int getMaxNumPrologues(int numParams);

  /*
   * Reset a specific prologue, or all prologues.
   */
  void resetPrologue(int numParams);
  void resetPrologues();

  /*
   * Smash prologue guards to prevent function from being called.
   */
  void smashPrologues();


  /////////////////////////////////////////////////////////////////////////////
  // Pretty printer.                                                    [const]

  struct PrintOpts {
    PrintOpts()
      : fpi(true)
      , metadata(true)
    {}

    PrintOpts& noFpi() {
      fpi = false;
      return *this;
    }

    PrintOpts& noMetadata() {
      metadata = false;
      return *this;
    }

    bool fpi;
    bool metadata;
  };

  void prettyPrint(std::ostream& out, const PrintOpts& = PrintOpts()) const;


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
  char& maybeIntercepted() const;

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
  void setFuncHandle(rds::Link<Func*> l);
  void setHasPrivateAncestor(bool b);
  void setMethodSlot(Slot s);


  /////////////////////////////////////////////////////////////////////////////
  // Offset accessors.                                                 [static]

#define OFF(f)                          \
  static constexpr ptrdiff_t f##Off() { \
    return offsetof(Func, m_##f);       \
  }
  OFF(attrs)
  OFF(cls)
  OFF(fullName)
  OFF(funcBody)
  OFF(maxStackCells)
  OFF(maybeIntercepted)
  OFF(paramCounts)
  OFF(prologueTable)
  OFF(refBitVal)
  OFF(shared)
  OFF(unit)
#undef OFF

  static constexpr ptrdiff_t sharedBaseOff() {
    return offsetof(SharedData, m_base);
  }


  /////////////////////////////////////////////////////////////////////////////
  // SharedData.

private:
  typedef IndexedStringMap<LowStringPtr, true, Id> NamedLocalsMap;

  // Some 16-bit values in SharedData are stored as small deltas if they fit
  // under this limit.  If not, they're set to the limit value and an
  // ExtendedSharedData will be allocated for the full-width field.
  static constexpr auto kSmallDeltaLimit = uint16_t(-1);

  /*
   * Properties shared by all clones of a Func.
   */
  struct SharedData : AtomicCountable {
    SharedData(PreClass* preClass, Offset base, Offset past,
               int line1, int line2, bool top, const StringData* docComment);
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
    // Bits 64 and up of the reffiness guards (the first 64 bits are in
    // Func::m_refBitVal for faster access).
    uint64_t* m_refBitPtr;
    ParamInfoVec m_params;
    NamedLocalsMap m_localNames;
    SVInfoVec m_staticVars;
    EHEntVec m_ehtab;
    FPIEntVec m_fpitab;

    // One byte worth of bools right now.  Check what it does to
    // sizeof(SharedData) if you are trying to add more than one more ...
    bool m_top : 1;
    bool m_isClosureBody : 1;
    bool m_isAsync : 1;
    bool m_isGenerator : 1;
    bool m_isPairGenerator : 1;
    bool m_isGenerated : 1;
    bool m_hasExtendedSharedData : 1;

    MaybeDataType m_returnType;
    LowStringPtr m_retUserType;
    UserAttributeMap m_userAttributes;
    TypeConstraint m_retTypeConstraint;
    LowStringPtr m_originalFilename;

    /*
     * The `past' offset and `line2' are likely to be small, particularly
     * relative to m_base and m_line1.  So we encode each as a 16-bit
     * difference.  If the delta doesn't fit, we need to have an
     * ExtendedSharedData to hold the real values---in that case, the field
     * here that overflowed is set to kSmallDeltaLimit and the corresponding
     * field in ExtendedSharedData will be valid.
     */
    uint16_t m_line2Delta;
    uint16_t m_pastDelta;
  };

  /*
   * If a Func represents a C++ builtin, or is exceptionally large (either in
   * line count or bytecode size), it requires extra information that most
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

    const ClassInfo::MethodInfo* m_info;
    BuiltinFunction m_builtinFuncPtr;
    BuiltinFunction m_nativeFuncPtr;
    Offset m_past;  // Only read if SharedData::m_pastDelta is kSmallDeltaLimit
    int m_line2;    // Only read if SharedData::m_line2 is kSmallDeltaLimit
  };

  typedef AtomicSharedPtr<SharedData> SharedDataPtr;

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


  /////////////////////////////////////////////////////////////////////////////
  // Constants.

private:
  static constexpr int kBitsPerQword = 64;
  static const StringData* s___call;
  static const StringData* s___callStatic;
  static constexpr int kMagic = 0xba5eba11;

public:
  static std::atomic<bool>     s_treadmill;
  static std::atomic<uint32_t> s_totalClonedClosures;


  /////////////////////////////////////////////////////////////////////////////
  // Data members.
  //
  // The fields of Func are organized in reverse order of frequency of use.
  // Do not re-order without checking perf!

private:
#ifdef DEBUG
  // For asserts only.
  int m_magic;
#endif
  unsigned char* volatile m_funcBody;
  mutable rds::Link<Func*> m_cachedFunc{rds::kInvalidHandle};
  FuncId m_funcId{InvalidFuncId};
  LowStringPtr m_fullName;
  LowStringPtr m_name;
  // The first Class in the inheritance hierarchy that declared this method.
  // Note that this may be an abstract class that did not provide an
  // implementation.
  LowPtr<Class> m_baseCls{nullptr};
  // The Class that provided this method implementation.
  AtomicLowPtr<Class> m_cls{nullptr};
  union {
    const NamedEntity* m_namedEntity{nullptr};
    Slot m_methodSlot;
  };
  // Atomically-accessed intercept flag.  -1, 0, or 1.
  // TODO(#1114385) intercept should work via invalidation.
  mutable char m_maybeIntercepted;
  mutable ClonedFlag m_cloned;
  bool m_isPreFunc : 1;
  bool m_hasPrivateAncestor : 1;
  int m_maxStackCells{0};
  uint64_t m_refBitVal{0};
  Unit* m_unit;
  SharedDataPtr m_shared;
  // Initialized by Func::finishedEmittingParams.  The least significant bit is
  // 1 if the last param is not variadic; the 31 most significant bits are the
  // total number of params (including the variadic param).
  uint32_t m_paramCounts{0};
  AtomicAttr m_attrs;
  // This must be the last field declared in this structure, and the Func class
  // should not be inherited from.
  unsigned char* volatile m_prologueTable[kNumFixedPrologues];
};

///////////////////////////////////////////////////////////////////////////////

template<class Container>
const typename Container::value_type* findEH(const Container& ehtab, Offset o) {
  uint32_t i;
  uint32_t sz = ehtab.size();

  const typename Container::value_type* eh = nullptr;
  for (i = 0; i < sz; i++) {
    if (ehtab[i].m_base <= o && o < ehtab[i].m_past) {
      eh = &ehtab[i];
    }
  }
  return eh;
}

///////////////////////////////////////////////////////////////////////////////

}

#define incl_HPHP_VM_FUNC_INL_H_
#include "hphp/runtime/vm/func-inl.h"
#undef incl_HPHP_VM_FUNC_INL_H_

#endif // incl_HPHP_VM_FUNC_H_
