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

#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/tv-helpers.h"
#include "hphp/runtime/base/type-string.h"

#include "hphp/runtime/vm/indexed-string-map.h"
#include "hphp/runtime/vm/repo-helpers.h"
#include "hphp/runtime/vm/type-constraint.h"
#include "hphp/runtime/vm/unit.h"

#include <utility>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const int kNumFixedPrologues = 6;

struct ActRec;

/*
 * C++ builtin function type.
 */
using BuiltinFunction = TypedValue* (*)(ActRec* ar);

/*
 * Vector of pairs (param index, offset of corresponding DV funclet).
 */
using DVFuncletsVec = std::vector<std::pair<int, Offset>>;

///////////////////////////////////////////////////////////////////////////////

/*
 * Metadata about a PHP function or method.
 *
 * The Func class cannot be safely extended, because variable amounts of memory
 * associated with the Func are allocated before and after the actual object.
 *
 * If the function is a closure, the Func is preceded by a Func* which points
 * to the next cloned closures (closures are cloned in order to transplant them
 * into different implementation contexts).
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

  Func(Unit& unit, Id id, PreClass* preClass, int line1, int line2,
       Offset base, Offset past, const StringData* name, Attr attrs,
       bool top, const StringData* docComment, int numParams);
  ~Func();

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
   *
   * FIXME: Currently, we have a small handful of setters which set properties
   * after clone occurs.  This is kind of gross and we should fix it somehow.
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
   * The Unit, PreClass, and Classes of the function.
   *
   * The `baseCls' is the Class which first declares the method; the `cls' is
   * the Class which implements it.
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
   * Array of named locals.
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
   * FIXME(4497824): This naming is pretty bad.
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


  /////////////////////////////////////////////////////////////////////////////
  // Closures.                                                          [const]

  /*
   * Is this function the body of a Closure object?
   *
   * (All PHP anonymous functions are Closure objects.)
   */
  bool isClosureBody() const;

  /*
   * Is this function cloned from another closure function in order to
   * transplant it into a different context?
   */
  bool isClonedClosure() const;


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
  // Magic methods.                                                     [const]

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
   * Return true if Offset o is inside the protected region of a fault
   * funclet for iterId, otherwise false. itRef will be set to true if
   * the iterator was initialized with MIterInit*, false if the iterator
   * was initialized with IterInit*.
   */
  bool checkIterScope(Offset o, Id iterId, bool& itRef) const;

  // imeth: an abstract / interface method
  void checkDeclarationCompat(const PreClass* preClass,
                              const Func* imeth) const;

  // This can be thought of as "if I look up this Func's name while in fromUnit,
  // will I always get this Func back?" This is important for the translator: if
  // this condition holds, it allows for some translation-time optimizations by
  // making assumptions about where function calls will go.
  bool isNameBindingImmutable(const Unit* fromUnit) const;

  void getFuncInfo(ClassInfo::MethodInfo* mi) const;

  /**
   * Was this generated specially by the compiler to aide the runtime?
   */
  bool isGenerated() const { return shared()->m_isGenerated; }
  const ClassInfo::MethodInfo* methInfo() const { return shared()->m_info; }

  void setBaseCls(Class* baseCls) { m_baseCls = baseCls; }
  void setAttrs(Attr attrs) { m_attrs = attrs; }

  bool hasPrivateAncestor() const { return m_hasPrivateAncestor; }
  void setHasPrivateAncestor(bool b) { m_hasPrivateAncestor = b; }
  // Assembly linkage.
  static size_t fullNameOffset() {
    return offsetof(Func, m_fullName);
  }
  static size_t sharedOffset() {
    return offsetof(Func, m_shared);
  }
  static size_t sharedBaseOffset() {
    return offsetof(SharedData, m_base);
  }
  char &maybeIntercepted() const { return m_maybeIntercepted; }

  bool shouldPGO() const;
  void incProfCounter();

  static void* allocFuncMem(
    const StringData* name, int numParams,
    bool needsNextClonedClosure,
    bool lowMem);


  const NamedEntity* getNamedEntity() const {
    assert(!m_shared->m_preClass);
    return m_namedEntity;
  }
  Slot methodSlot() const {
    assert(m_cls);
    return m_methodSlot;
  }
  void setMethodSlot(Slot s) {
    assert(m_cls);
    m_methodSlot = s;
  }
  RDS::Handle funcHandle() const { return m_cachedFunc.handle(); }
  void setFuncHandle(RDS::Link<Func*> l) {
    // TODO(#2950356): this assertion fails for create_function with
    // an existing declared function named __lambda_func.
    // assert(!m_cachedFunc.valid());
    m_cachedFunc = l;
  }

  bool anyBlockEndsAt(Offset off) const;


  /////////////////////////////////////////////////////////////////////////////
  // Offset accessors.                                                 [static]

public: // Offset accessors for the translator.
#define X(f) static constexpr ptrdiff_t f##Off() {      \
    return offsetof(Func, m_##f);                       \
  }
  X(attrs);
  X(unit);
  X(cls);
  X(paramCounts);
  X(refBitVal);
  X(fullName);
  X(prologueTable);
  X(maybeIntercepted);
  X(maxStackCells);
  X(funcBody);
  X(shared);
#undef X


  /////////////////////////////////////////////////////////////////////////////
  // SharedData.

private:
  typedef IndexedStringMap<LowStringPtr, true, Id> NamedLocalsMap;

  struct SharedData : public AtomicCountable {
    PreClass* m_preClass;
    Id m_id;
    Offset m_base;
    Id m_numLocals;
    Id m_numIterators;
    Offset m_past;
    int m_line1;
    int m_line2;
    DataType m_returnType;
    const ClassInfo::MethodInfo* m_info; // For builtins.
    // bits 64 and up of the reffiness guards (first 64 bits
    // are in Func::m_refBitVal)
    uint64_t* m_refBitPtr;
    BuiltinFunction m_builtinFuncPtr;
    BuiltinFunction m_nativeFuncPtr;
    ParamInfoVec m_params; // m_params[i] corresponds to parameter i.
    NamedLocalsMap m_localNames; // includes parameter names
    SVInfoVec m_staticVars;
    EHEntVec m_ehtab;
    FPIEntVec m_fpitab;
    hphp_hash_set<Offset> m_blockEnds;
    LowStringPtr m_docComment;
    bool m_top : 1; // Defined at top level.
    bool m_isClosureBody : 1;
    bool m_isAsync : 1;
    bool m_isGenerator : 1;
    bool m_isPairGenerator : 1;
    bool m_isGenerated : 1;
    UserAttributeMap m_userAttributes;
    TypeConstraint m_retTypeConstraint;
    LowStringPtr m_retUserType;
    // per-func filepath for traits flattened during repo construction
    LowStringPtr m_originalFilename;
    SharedData(PreClass* preClass, Id id, Offset base,
        Offset past, int line1, int line2, bool top,
        const StringData* docComment);
    ~SharedData();
    void atomicRelease();
  };
  typedef AtomicSmartPtr<SharedData> SharedDataPtr;

  static const int kBitsPerQword = 64;
  static const StringData* s___call;
  static const StringData* s___callStatic;
  static const int kMagic = 0xba5eba11;

private:
  void setFullName(int numParams);
  void init(int numParams);
  void initPrologues(int numParams);
  void appendParam(bool ref, const ParamInfo& info,
                   std::vector<ParamInfo>& pBuilder);
  void finishedEmittingParams(std::vector<ParamInfo>& pBuilder);
  void allocVarId(const StringData* name);
  const SharedData* shared() const { return m_shared.get(); }
  SharedData* shared() { return m_shared.get(); }
  Func* findCachedClone(Class* cls) const;

  /*
   * Closures' __invoke() methods have an extra pointer used to keep cloned
   * versions of themselves with different contexts.
   *
   * const here is the equivalent of "mutable" since this is just a cache.
   */
  Func*& nextClonedClosure() const;

private:
  /*
   * Fields are organized in reverse order of frequency of use
   * Do not re-order without checking perf
   */
#ifdef DEBUG
  int m_magic; // For asserts only.
#endif
  LowStringPtr m_fullName;
  uint32_t m_profCounter{0};     // profile counter used to detect hot functions
  unsigned char* volatile m_funcBody;  // Accessed from assembly.
  mutable RDS::Link<Func*> m_cachedFunc{RDS::kInvalidHandle};
  FuncId m_funcId{InvalidFuncId};
  LowStringPtr m_name;
  // The first Class in the inheritance hierarchy that declared this method;
  // note that this may be an abstract class that did not provide an
  // implementation.
  LowClassPtr m_baseCls{nullptr};
  // The Class that provided this method implementation.
  LowClassPtr m_cls{nullptr};
  union {
    const NamedEntity* m_namedEntity{nullptr};
    Slot m_methodSlot;
  };
  // TODO(#1114385) intercept should work via invalidation.
  mutable char m_maybeIntercepted; // -1, 0, or 1.  Accessed atomically.
  bool m_hasPrivateAncestor : 1; // This flag indicates if any of this
                                 // Class's ancestors provide a
                                 // "private" implementation for this
                                 // method
  int m_maxStackCells{0};
  uint64_t m_refBitVal{0};
  Unit* m_unit;
  SharedDataPtr m_shared;
  // Initialized by Func::finishedEmittingParams, the least significant bit
  // is 1 if the last param is not variadic; the 31 most significant bits
  // are the total number of params (including the variadic param)
  uint32_t m_paramCounts{0};
  Attr m_attrs;
  // This must be the last field declared in this structure
  // and the Func class should not be inherited from.
  unsigned char* volatile m_prologueTable[kNumFixedPrologues];
};

///////////////////////////////////////////////////////////////////////////////

class PreClassEmitter;

class FuncEmitter {
public:
  typedef std::vector<Func::SVInfo> SVInfoVec;
  typedef std::vector<EHEnt> EHEntVec;
  typedef std::vector<FPIEnt> FPIEntVec;

  struct ParamInfo: public Func::ParamInfo {
    ParamInfo() : m_ref(false) {}

    void setRef(bool ref) { m_ref = ref; }
    bool ref() const { return m_ref; }

    template<class SerDe> void serde(SerDe& sd) {
      Func::ParamInfo* parent = this;
      parent->serde(sd);
      sd(m_ref);
    }

  private:
    bool m_ref; // True if parameter is passed by reference.
  };

  typedef std::vector<ParamInfo> ParamInfoVec;

  FuncEmitter(UnitEmitter& ue, int sn, Id id, const StringData* n);
  FuncEmitter(UnitEmitter& ue, int sn, const StringData* n,
      PreClassEmitter* pce);
  ~FuncEmitter();

  void init(int line1, int line2, Offset base, Attr attrs, bool top,
      const StringData* docComment);
  void finish(Offset past, bool load);

  template<class SerDe> void serdeMetaData(SerDe&);

  EHEnt& addEHEnt();
  FPIEnt& addFPIEnt();
  void setEhTabIsSorted();

  Id newLocal();
  void appendParam(const StringData* name, const ParamInfo& info);
  void setParamFuncletOff(Id id, Offset off) {
    m_params[id].funcletOff = off;
  }
  void allocVarId(const StringData* name);
  Id lookupVarId(const StringData* name) const;
  bool hasVar(const StringData* name) const;
  Id numParams() const { return m_params.size(); }

  /*
   * Return type constraints will eventually be runtime-enforced
   * return types, but for now are unused.
   */
  void setReturnTypeConstraint(const TypeConstraint retTypeConstraint) {
    m_retTypeConstraint = retTypeConstraint;
  }
  const TypeConstraint& returnTypeConstraint() const {
    return m_retTypeConstraint;
  }

  /*
   * Return "user types" are string-format specifications of return
   * types only used for reflection purposes.
   */
  void setReturnUserType(const StringData* retUserType) {
    m_retUserType = retUserType;
  }
  const StringData* returnUserType() const {
    return m_retUserType;
  }

  /*
   * Returns whether this FuncEmitter represents an HNI function with
   * a native implementation.
   */
  bool isHNINative() const { return getReturnType() != KindOfInvalid; }

  Id numIterators() const { return m_numIterators; }
  void setNumIterators(Id numIterators);
  Id allocIterator();
  void freeIterator(Id id);
  Id numLiveIterators() { return m_nextFreeIterator; }
  void setNumLiveIterators(Id id) { m_nextFreeIterator = id; }

  Id allocUnnamedLocal();
  void freeUnnamedLocal(Id id);
  Id numLocals() const { return m_numLocals; }
  void setNumLocals(Id numLocals);
  const Func::NamedLocalsMap::Builder& localNameMap() const {
    return m_localNames;
  }

  void setMaxStackCells(int cells) { m_maxStackCells = cells; }
  void addStaticVar(Func::SVInfo svInfo);
  const SVInfoVec& svInfo() const { return m_staticVars; }

  UnitEmitter& ue() const { return m_ue; }
  PreClassEmitter* pce() const { return m_pce; }
  void setIds(int sn, Id id) {
    m_sn = sn;
    m_id = id;
  }
  int sn() const { return m_sn; }
  Id id() const {
    assert(m_pce == nullptr);
    return m_id;
  }
  Offset base() const { return m_base; }
  Offset past() const { return m_past; }
  const StringData* name() const { return m_name; }
  const ParamInfoVec& params() const { return m_params; }
  const EHEntVec& ehtab() const { return m_ehtab; }
  EHEntVec& ehtab() { return m_ehtab; }
  const FPIEntVec& fpitab() const { return m_fpitab; }

  void setAttrs(Attr attrs) { m_attrs = attrs; }
  Attr attrs() const { return m_attrs; }
  bool isVariadic() const {
    return m_params.size() && m_params[(m_params.size() - 1)].isVariadic();
  }

  void setTop(bool top) { m_top = top; }
  bool top() const { return m_top; }

  bool isPseudoMain() const { return m_name->empty(); }

  void setIsClosureBody(bool isClosureBody) { m_isClosureBody = isClosureBody; }
  bool isClosureBody() const { return m_isClosureBody; }

  void setIsGenerator(bool isGenerator) { m_isGenerator = isGenerator; }
  bool isGenerator() const { return m_isGenerator; }

  bool isMethod() const {
    return !isPseudoMain() && (bool)pce();
  }

  void setIsPairGenerator(bool b) { m_isPairGenerator = b; }
  bool isPairGenerator() const { return m_isPairGenerator; }

  void setContainsCalls() { m_containsCalls = true; }

  void setIsAsync(bool isAsync) { m_isAsync = isAsync; }
  bool isAsync() const { return m_isAsync; }

  void addUserAttribute(const StringData* name, TypedValue tv);
  void setUserAttributes(UserAttributeMap map) {
    m_userAttributes = std::move(map);
  }
  const UserAttributeMap& getUserAttributes() const {
    return m_userAttributes;
  }
  bool hasUserAttribute(const StringData* name) const {
    auto it = m_userAttributes.find(name);
    return it != m_userAttributes.end();
  }
  int parseNativeAttributes(Attr &attrs) const;

  void commit(RepoTxn& txn) const;
  Func* create(Unit& unit, PreClass* preClass = nullptr) const;

  void setBuiltinFunc(const ClassInfo::MethodInfo* info,
      BuiltinFunction bif, BuiltinFunction nif, Offset base);
  void setBuiltinFunc(BuiltinFunction bif, BuiltinFunction nif,
                      Attr attrs, Offset base);

  void setOriginalFilename(const StringData* name) {
    m_originalFilename = name;
  }
  const StringData* originalFilename() const { return m_originalFilename; }

  /*
   * Return types used for HNI functions with a native C++
   * implementation.
   */
  void setReturnType(DataType dt) { m_returnType = dt; }
  DataType getReturnType() const { return m_returnType; }

  void setDocComment(const char *dc) {
    m_docComment = makeStaticString(dc);
  }
  const StringData* getDocComment() const {
    return m_docComment;
  }

  void setLocation(int l1, int l2) {
    m_line1 = l1;
    m_line2 = l2;
  }

  std::pair<int,int> getLocation() const {
    return std::make_pair(m_line1, m_line2);
  }

private:
  void sortEHTab();
  void sortFPITab(bool load);

  UnitEmitter& m_ue;
  PreClassEmitter* m_pce;
  int m_sn;
  Id m_id;
  Offset m_base;
  Offset m_past;
  int m_line1;
  int m_line2;
  LowStringPtr m_name;

  ParamInfoVec m_params;
  Func::NamedLocalsMap::Builder m_localNames;
  Id m_numLocals;
  int m_numUnnamedLocals;
  int m_activeUnnamedLocals;
  Id m_numIterators;
  Id m_nextFreeIterator;
  int m_maxStackCells;
  SVInfoVec m_staticVars;

  TypeConstraint m_retTypeConstraint;
  LowStringPtr m_retUserType;

  EHEntVec m_ehtab;
  bool m_ehTabSorted;
  FPIEntVec m_fpitab;

  Attr m_attrs;
  DataType m_returnType;
  bool m_top;
  LowStringPtr m_docComment;
  bool m_isClosureBody;
  bool m_isAsync;
  bool m_isGenerator;
  bool m_isPairGenerator;
  bool m_containsCalls;

  UserAttributeMap m_userAttributes;

  const ClassInfo::MethodInfo* m_info;
  BuiltinFunction m_builtinFuncPtr;
  BuiltinFunction m_nativeFuncPtr;

  LowStringPtr m_originalFilename;
};

class FuncRepoProxy : public RepoProxy {
  friend class Func;
  friend class FuncEmitter;
public:
  explicit FuncRepoProxy(Repo& repo);
  ~FuncRepoProxy();
  void createSchema(int repoId, RepoTxn& txn);

#define FRP_IOP(o) FRP_OP(Insert##o, insert##o)
#define FRP_GOP(o) FRP_OP(Get##o, get##o)
#define FRP_OPS \
  FRP_IOP(Func) \
  FRP_GOP(Funcs)
  class InsertFuncStmt : public RepoProxy::Stmt {
  public:
    InsertFuncStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(const FuncEmitter& fe,
                RepoTxn& txn, int64_t unitSn, int funcSn, Id preClassId,
                const StringData* name, bool top);
  };
  class GetFuncsStmt : public RepoProxy::Stmt {
  public:
    GetFuncsStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue);
  };
#define FRP_OP(c, o) \
 public: \
  c##Stmt& o(int repoId) { return *m_##o[repoId]; } \
 private: \
  c##Stmt m_##o##Local; \
  c##Stmt m_##o##Central; \
  c##Stmt* m_##o[RepoIdCount];
FRP_OPS
#undef FRP_OP
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
