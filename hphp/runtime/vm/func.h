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
  enum class Type {
    Catch,
    Fault
  };
  typedef std::vector<std::pair<Id, Offset>> CatchVec;

  Type m_type;
  Offset m_base;
  Offset m_past;
  int m_iterId;
  bool m_itRef;
  int m_parentIndex;
  Offset m_fault;
  CatchVec m_catches;

  template<class SerDe> void serde(SerDe& sd);
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
 * If the function is a closure, the Func is preceded by a Func* which points
 * to the next cloned closures (closures are cloned in order to transplant them
 * into different implementation contexts).  This pointer is considered
 * mutable, even on const Funcs.
 *
 * All Funcs are also followed by a variable number of function prologue
 * pointers.  Six are statically allocated as part of the Func object, but more
 * may follow, depending on the value of getMaxNumPrologues().
 *
 *              +--------------------------------+ low address
 *              |  [Func** to next closure]      |
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

  /////////////////////////////////////////////////////////////////////////////
  // Types.

  /*
   * Parameter default value info.
   */
  struct ParamInfo {
    ParamInfo()
      : builtinType(KindOfInvalid)
      , funcletOff(InvalidAbsoluteOffset)
      , defaultValue(make_tv<KindOfUninit>())
      , phpCode(nullptr)
      , userType(nullptr)
      , variadic(false)
    {}

    template<class SerDe>
    void serde(SerDe& sd) {
      sd(builtinType)
        (funcletOff)
        (defaultValue)
        (phpCode)
        (typeConstraint)
        (variadic)
        (userAttributes)
        (userType)
        ;
    }

    bool hasDefaultValue() const {
      return funcletOff != InvalidAbsoluteOffset;
    }
    bool hasScalarDefaultValue() const {
      return hasDefaultValue() && defaultValue.m_type != KindOfUninit;
    }
    bool isVariadic() const { return variadic; }

  public:
    DataType builtinType;     // Typehint for builtins.
    Offset funcletOff;
    TypedValue defaultValue;  // Set to Uninit if there is no DV,
                              // or if there's a nonscalar DV.
    LowStringPtr phpCode;     // Eval'able PHP code.
    LowStringPtr userType;    // User-annotated type.
    TypeConstraint typeConstraint;
    bool variadic;
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

  Func(Unit& unit, PreClass* preClass, int line1, int line2,
       Offset base, Offset past, const StringData* name, Attr attrs,
       bool top, const StringData* docComment, int numParams);
  ~Func();

  /*
   * Allocate memory for a function, including the extra preceding and
   * succeeding data.
   */
  static void* allocFuncMem(const StringData* name, int numParams,
                            bool needsNextClonedClosure, bool lowMem);

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
  Func* cloneAndSetClass(Class* cls) const;

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
   * Whether this function was defined in a psuedomain.
   */
  bool top() const;

  /*
   * The Unit, PreClass, Classes of the function.
   *
   * The `baseCls' is the first Class in the inheritance hierarchy which
   * declares the method; the `cls' is the Class which implements it.
   */
  Unit* unit() const;
  PreClass* preClass() const;
  Class* baseCls() const;
  Class* cls() const;

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
  DataType returnType() const;

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
   */
  bool mayHaveThis() const;


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

  /*
   * Is this function cloned from another closure function in order to
   * transplant it into a different context?
   */
  bool isClonedClosure() const;

private:
  /*
   * Closures are allocated with an extra pointer before the Func object
   * itself.  These are used to chain clones of these closures with different
   * Class contexts.
   *
   * We consider this extra pointer to be a mutable member of Func, hence the
   * `const' specifier here.
   *
   * @requires: isClosureBody()
   */
  Func*& nextClonedClosure() const;

  /*
   * Find the clone of this closure with `cls' as its context.
   *
   * Return nullptr if this is not a closure or if no such clone exists.
   */
  Func* findCachedClone(Class* cls) const;

public:

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
  RDS::Handle funcHandle() const;

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
   * Populate the MethodInfo for this function in `mi'.
   *
   * If methInfo() is non-null, this just performs a deep copy.
   */
  void getFuncInfo(ClassInfo::MethodInfo* mi) const;

  static const AtomicVector<const Func*>& getFuncVec();

  /*
   * Profile-guided optimization linkage.
   */
  bool shouldPGO() const;
  void incProfCounter();
  uint32_t profCounter() const { return m_profCounter; }
  void setHot() { m_attrs = (Attr)(m_attrs | AttrHot); }

  /*
   * Does any HHBC block end at `off'?
   */
  bool anyBlockEndsAt(Offset off) const;


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
  void setFuncHandle(RDS::Link<Func*> l);
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

  struct SharedData : public AtomicCountable {
    SharedData(PreClass* preClass, Offset base, Offset past,
               int line1, int line2, bool top, const StringData* docComment);
    ~SharedData();

    /*
     * Interface for AtomicCountable.
     */
    void atomicRelease();

    /*
     * Properties shared by all clones of a Func.
     */
    PreClass* m_preClass;
    Offset m_base;
    Offset m_past;
    Id m_numLocals;
    Id m_numIterators;
    int m_line1;
    int m_line2;
    DataType m_returnType;
    const ClassInfo::MethodInfo* m_info;
    // Bits 64 and up of the reffiness guards (the first 64 bits are in
    // Func::m_refBitVal for faster access).
    uint64_t* m_refBitPtr;
    BuiltinFunction m_builtinFuncPtr;
    BuiltinFunction m_nativeFuncPtr;
    ParamInfoVec m_params;
    NamedLocalsMap m_localNames;
    SVInfoVec m_staticVars;
    EHEntVec m_ehtab;
    FPIEntVec m_fpitab;
    // Cache for the anyBlockEndsAt() method.
    hphp_hash_set<Offset> m_blockEnds;
    LowStringPtr m_docComment;
    bool m_top : 1;
    bool m_isClosureBody : 1;
    bool m_isAsync : 1;
    bool m_isGenerator : 1;
    bool m_isPairGenerator : 1;
    bool m_isGenerated : 1;
    UserAttributeMap m_userAttributes;
    TypeConstraint m_retTypeConstraint;
    LowStringPtr m_retUserType;
    LowStringPtr m_originalFilename;
  };

  typedef AtomicSmartPtr<SharedData> SharedDataPtr;

  /*
   * SharedData accessors for internal use.
   */
  const SharedData* shared() const { return m_shared.get(); }
        SharedData* shared()       { return m_shared.get(); }


  /////////////////////////////////////////////////////////////////////////////
  // Internal methods.
  //
  // These are all used at emit-time, and should be outsourced to FuncEmitter.

private:
  void init(int numParams);
  void initPrologues(int numParams);
  void setFullName(int numParams);
  void appendParam(bool ref, const ParamInfo& info,
                   std::vector<ParamInfo>& pBuilder);
  void finishedEmittingParams(std::vector<ParamInfo>& pBuilder);


  /////////////////////////////////////////////////////////////////////////////
  // Constants.

private:
  static constexpr int kBitsPerQword = 64;
  static const StringData* s___call;
  static const StringData* s___callStatic;
  static constexpr int kMagic = 0xba5eba11;

public:
  static std::atomic<bool> s_treadmill;

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
  LowStringPtr m_fullName;
  // Profile counter used to detect hot functions.
  uint32_t m_profCounter{0};
  unsigned char* volatile m_funcBody;
  mutable RDS::Link<Func*> m_cachedFunc{RDS::kInvalidHandle};
  FuncId m_funcId{InvalidFuncId};
  LowStringPtr m_name;
  // The first Class in the inheritance hierarchy that declared this method.
  // Note that this may be an abstract class that did not provide an
  // implementation.
  LowClassPtr m_baseCls{nullptr};
  // The Class that provided this method implementation.
  LowClassPtr m_cls{nullptr};
  union {
    const NamedEntity* m_namedEntity{nullptr};
    Slot m_methodSlot;
  };
  // Atomically-accessed intercept flag.  -1, 0, or 1.
  // TODO(#1114385) intercept should work via invalidation.
  mutable char m_maybeIntercepted;
  bool m_hasPrivateAncestor : 1;
  int m_maxStackCells{0};
  uint64_t m_refBitVal{0};
  Unit* m_unit;
  SharedDataPtr m_shared;
  // Initialized by Func::finishedEmittingParams.  The least significant bit is
  // 1 if the last param is not variadic; the 31 most significant bits are the
  // total number of params (including the variadic param).
  uint32_t m_paramCounts{0};
  Attr m_attrs;
  // This must be the last field declared in this structure, and the Func class
  // should not be inherited from.
  unsigned char* volatile m_prologueTable[kNumFixedPrologues];
};

///////////////////////////////////////////////////////////////////////////////

template<class EHEntVec>
const EHEnt* findEH(const EHEntVec& ehtab, Offset o) {
  uint32_t i;
  uint32_t sz = ehtab.size();

  const EHEnt* eh = nullptr;
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
