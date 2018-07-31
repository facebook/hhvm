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

#ifndef incl_HPHP_VM_CLASS_H_
#define incl_HPHP_VM_CLASS_H_

#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/rds-util.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/atomic-countable.h"
#include "hphp/runtime/vm/fixed-string-map.h"
#include "hphp/runtime/vm/indexed-string-map.h"
#include "hphp/runtime/vm/instance-bits.h"
#include "hphp/runtime/vm/preclass.h"
#include "hphp/runtime/vm/trait-method-import-data.h"

#include "hphp/util/compact-vector.h"
#include "hphp/util/default-ptr.h"
#include "hphp/util/hash-map.h"

#include <folly/Hash.h>
#include <folly/Range.h>

#include <boost/container/flat_map.hpp>

#include <list>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const StaticString s___MockClass;

struct Class;
struct ClassInfo;
struct EnumValues;
struct Func;
struct StringData;
struct c_Awaitable;

namespace collections {
struct CollectionsExtension;
}

namespace Native {
struct NativeDataInfo;
struct NativePropHandler;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Utility wrapper for static properties. Allows distinguishing them
 * via type_scan::Index.
 */
struct StaticPropData {
  TypedValue val;
};

using ClassPtr = AtomicSharedLowPtr<Class>;

/*
 * Class represents the full definition of a user class in a given request
 * context.
 *
 * See PreClass for more on the distinction.
 *
 * The method table is allocated at negative offset from the start of the Class
 * object, and the method slot is used as the negative offset from the object
 * to index into the method table.
 *
 *                                 +------------+
 * Func Slot n (offset -(n+1)) --> |            |
 *                                      ....
 *                                 |            |
 *                                 +------------+
 * Func Slot 1 (offset -2) ------> |            |
 *                                 +------------+
 * Func Slot 0 (offset -1) ------> |            |
 * Class* -----------------------> +------------+
 *                                 |            |
 *                                      ....
 *                                 |            |
 *                                 +------------+
 */
struct Class : AtomicCountable {

  /////////////////////////////////////////////////////////////////////////////
  // Types.

  /*
   * Class availability.
   *
   * @see: Class::avail()
   */
  enum class Avail {
    False,
    True,
    Fail
  };

  /*
   * Attributes computed at runtime class init time, used to short
   * circuit more expensive checks. Not to be confused with enum Attr,
   * which are a-priori attributes computed by the compiler.
   */
  enum RuntimeAttribute : uint8_t {
    CallToImpl           = 0x01, // call to{Boolean,Int64,Double}Impl
    HasSleep             = 0x02, // __sleep()
    HasClone             = 0x04, // defines __clone PHP method; only valid
                                 // when !isCppBuiltin()
    HasNativePropHandler = 0x08, // class has native magic props handler
    UseSet               = 0x10, // __set()
    UseGet               = 0x20, // __get()
    UseIsset             = 0x40, // __isset()
    UseUnset             = 0x80, // __unset()
  };

  /*
   * Instance property information.
   */
  struct Prop {
    /*
     * name is "" for inaccessible properties (i.e. private properties declared
     * by parents).
     */
    LowStringPtr name;
    LowStringPtr mangledName;
    LowStringPtr originalMangledName;

    /* First parent class that declares this property. */
    LowPtr<Class> cls;

    Attr attrs;
    LowStringPtr userType;
    TypeConstraint typeConstraint;

    /*
     * When built in RepoAuthoritative mode, this is a control-flow insensitive,
     * always-true type assertion for this property.  (It may be Gen if there
     * was nothing interesting known.)
     */
    RepoAuthType repoAuthType;
    LowStringPtr docComment;
    int idx;
  };

  /*
   * Static property information.
   */
  struct SProp {
    LowStringPtr name;
    Attr attrs;
    LowStringPtr userType;
    LowStringPtr docComment;
    TypeConstraint typeConstraint;

    /* Most derived class that declared this property. */
    LowPtr<Class> cls;
    int idx;

    /* Used if (cls == this). */
    TypedValue val;

    RepoAuthType repoAuthType;
  };

  /*
   * Class constant information.
   */
  struct Const {
    /* Most derived class that declared this constant. */
    LowPtr<const Class> cls;
    LowStringPtr name;
    TypedValueAux val;

    bool isAbstract() const { return val.constModifiers().isAbstract; }
    bool isType()     const { return val.constModifiers().isType; }
  };

  /*
   * Initialization vector for declared properties.
   *
   * This is a vector which contains default values for all of a Class's
   * declared instance properties.  It is used when instantiating new objects
   * from a Class.
   */
  struct PropInitVec {
    PropInitVec();
    ~PropInitVec();

    const PropInitVec& operator=(const PropInitVec&);

    using iterator = TypedValueAux*;

    iterator begin();
    iterator end();
    size_t size() const;

    TypedValueAux& operator[](size_t i);
    const TypedValueAux& operator[](size_t i) const;

    void push_back(const TypedValue& v);

    /*
     * Make a request-allocated copy of `src'.
     */
    static PropInitVec* allocWithReqAllocator(const PropInitVec& src);

    static constexpr size_t dataOff() {
      return offsetof(PropInitVec, m_data);
    }

  private:
    PropInitVec(const PropInitVec&);

    bool reqAllocated() const;

    TypedValueAux* m_data;
    uint32_t m_size;
    // m_capacity > 0, allocated on global huge heap
    // m_capacity = 0, not request allocated, m_data is nullptr
    // m_capacity < 0, request allocated, with '~m_capacity' slots
    int32_t m_capacity;
  };
  static_assert(sizeof(PropInitVec) <= 16, "");

  /*
   * A slot in a Class vtable vector, pointing to the vtable for an interface
   * and the interface itself. Used for efficient interface method dispatch and
   * instance checks.
   */
  struct VtableVecSlot {
    LowPtr<LowPtr<Func>> vtable;
    LowPtr<Class> iface;
  };

  /*
   * Container types.
   */
  using MethodMap         = FixedStringMap<Slot, false, Slot>;
  using MethodMapBuilder  = FixedStringMapBuilder<Func*, Slot, false, Slot>;
  using InterfaceMap      = IndexedStringMap<LowPtr<Class>, true, int>;
  using RequirementMap    = IndexedStringMap<
                              const PreClass::ClassRequirement*, true, int>;

  using TraitAliasVec = std::vector<PreClass::TraitAliasRule::NamePair>;

  /*
   * Scope context for a Closure subclass.
   */
  struct CloneScope {
    LowPtr<Class> ctx;
    Attr attrs;

    bool operator==(CloneScope o) const { return ctx == o.ctx &&
                                                 attrs == o.attrs; }
    bool operator!=(CloneScope o) const { return !(*this == o); }

    struct hash {
      size_t operator()(CloneScope cs) const {
        return folly::hash::hash_combine(
          cs.ctx.get(),
          static_cast<uint32_t>(cs.attrs)
        );
      }
    };
  };

  /*
   * Map from a Closure subclass C's scope context to the appropriately scoped
   * clone of C.
   *
   * @see: Class::ExtraData::m_scopedClones
   */
  using ScopedClonesMap = hphp_hash_map<CloneScope,ClassPtr,CloneScope::hash>;

  /*
   * A reference to a scoped clone of a Closure subclass.  We omit the Class
   * ctx, since we only use this struct when the ctx is `this'.
   */
  struct ScopedCloneBackref {
    ClassPtr template_cls;
    /* LowPtr<Class> ctx_cls = this; */
    Attr ctx_attrs;
  };

  /*
   * We store the length of vectors of methods, parent classes and interfaces.
   *
   * In lowptr builds, we limit all of these quantities to 2^16-1 to save
   * memory.
   */
  using veclen_t = std::conditional<use_lowptr, uint16_t, uint32_t>::type;


  /////////////////////////////////////////////////////////////////////////////
  // Creation and destruction.

  /*
   * Allocate a new Class object.
   *
   * Eventually deallocated using atomicRelease(), but can go through some
   * phase changes before that (see destroy()).
   */
  static Class* newClass(PreClass* preClass, Class* parent);

  /*
   * Make a clone of this Closure subclass, with `ctx' as the closure scope.
   *
   * Passing a value of `attrs' that is not AttrNone indicates that the scoping
   * is dynamic---i.e., via Closure::bind(), as opposed to a CreateCl opcode.
   * If the specified `attrs' do not match those of the __invoke method, we
   * update them in the clone along with the scope.  All closure __invoke
   * methods have AttrPublic, so using AttrNone as a sentinel here is
   * unambiguous.
   *
   * If the scoping already exists in m_extra->m_scopedClones, or if this class
   * is already scoped correctly, just return it.  Otherwise, we scope our own
   * m_invoke if it's not already scoped, or clone ourselves and scope the
   * clone's m_invoke, then add the mapping to m_scopedClones.  It is required
   * for correctness that all clones be added to the cache, because the cache
   * participates in synchronization with instance bits initialization.
   *
   * Note that all scoping events via CreateCl opcodes clone from the
   * "template" Closure subclass that is generated by the emitter, whereas
   * scoping events via Closure::bind() clone from another scoped clone (which
   * may or may not be the first clone, which aliases the template class).
   * Thus, when rescoping dynamically, we need to find the template class
   * first, since it owns the clone cache.
   *
   * Additionally, for dynamic rescopings, we always produce a clone.  Many
   * situations may arise in Closure::bind() that never do in CreateCl---e.g.,
   * a closure object whose class is Closure rather than an emitter-generated
   * subclass of Closure, a closure scoped to its own class, etc.  Requiring a
   * clone in the dynamic case keeps us from mucking up the template __invoke's
   * attrs, and gives us the invariant that the template class is being used as
   * a scoped clone iff its __invoke has a different cls().
   *
   * @requires: parent() == SystemLib::s_ClosureClass
   */
  Class* rescope(Class* ctx, Attr attrs = AttrNone);

  /*
   * Called when a Class becomes unreachable.
   *
   * This may happen before its refcount hits zero if it is still referred to
   * by any of:
   *    - its NamedEntity;
   *    - any derived Class;
   *    - any Class that implements it (for interfaces); or
   *    - any Class that uses it (for traits)
   *
   * Such referring classes must also be logically dead at the time destroy()
   * is called.  However, since we don't have back pointers to find them,
   * instead we leave the Class in a zombie state.  When we try to instantiate
   * one of its referrers, we will notice that it depends on a zombie and
   * destroy *that*, releasing its reference to this Class.
   */
  void destroy();

  /*
   * Called when the (atomic) refcount hits zero.
   *
   * The Class is completely dead at this point, and its memory is freed
   * immediately.
   */
  void atomicRelease();

private:
  /*
   * Free any references to child classes, interfaces, and traits.
   *
   * releaseRefs() is called when a Class is put into the zombie state.  It's
   * safe to call multiple times, so it is also called from the destructor (in
   * case we bypassed the zombie state).
   */
  void releaseRefs();

public:
  /*
   * Whether this class has been logically destroyed, but needed to be
   * preserved due to outstanding references.
   */
  bool isZombie() const;

  /*
   * Check whether a Class from a previous request is available to be defined.
   * The caller should check that it has the same PreClass that is being
   * defined.  Being available means that the parent, the interfaces, and the
   * traits are already defined (or become defined via autoload, if tryAutoload
   * is true).
   *
   * @returns: Avail::True:  if it's available
   *           Avail::Fail:  if at least one of the parent, interfaces, and
   *                         traits is not defined at all at this point
   *           Avail::False: if at least one of the parent, interfaces, and
   *                         traits is defined but does not correspond to this
   *                         particular Class*
   *
   * The parent parameter is used for two purposes: first, it lets us avoid
   * looking up the active parent class for each potential Class*; and second,
   * it is used on Fail to return the problem class so the caller can report
   * the error correctly.
   */
  Avail avail(Class*& parent, bool tryAutoload = false) const;


  /////////////////////////////////////////////////////////////////////////////
  // Pre- and post-allocations.                                         [const]

  /*
   * Pointer to this Class's FuncVec, which is allocated before this.
   */
  LowPtr<Func>* funcVec() const;

  /*
   * The start of malloc'd memory for `this' (i.e., including anything
   * allocated before the object itself.).
   */
  void* mallocPtr() const;

  /*
   * Address of the end of the Class's variable-length memory allocation.
   */
  const void* mallocEnd() const;

  /*
   * Pointer to the array of Class pointers, allocated immediately after
   * `this', which contain this class's inheritance hierarchy (including `this'
   * as the last element).
   */
  const LowPtr<Class>* classVec() const;

  /*
   * The size of the classVec.
   */
  veclen_t classVecLen() const;


  /////////////////////////////////////////////////////////////////////////////
  // Ancestry.                                                          [const]

  /*
   * Determine if this represents a non-strict subtype of `cls'.  The nonIFace
   * variant is faster, but has the additional precondition that `cls' is not
   * an interface.
   */
  bool classof(const Class*) const;
  bool classofNonIFace(const Class*) const;

  /*
   * Whether this class implements an interface called `name'.
   */
  bool ifaceofDirect(const StringData* name) const;

  /*
   * Assuming this and cls are both regular classes (not interfaces or traits),
   * return their lowest common ancestor, or nullptr if they're unrelated.
   */
  const Class* commonAncestor(const Class* cls) const;

  /*
   * Given that this class exists, return a class named "name" that is
   * also guaranteed to exist, or nullptr if there is none.
   */
  const Class* getClassDependency(const StringData* name) const;

  /////////////////////////////////////////////////////////////////////////////
  // Basic info.                                                        [const]

  /*
   * The name, PreClass, and parent class of this class.
   */
  const StringData* name() const;
  const PreClass* preClass() const;
  Class* parent() const;

  /*
   * Uncounted String names of this class and of its parent.
   */
  StrNR nameStr() const;
  StrNR parentStr() const;

  /*
   * The attributes on this class.
   */
  Attr attrs() const;

  /*
   * ObjectData attributes, to be set during instance initialization.
   */
  int getODAttrs() const;

  /*
   * Runtime class attributes, computed during class initialization.
   */
  bool rtAttribute(RuntimeAttribute) const;
  void initRTAttributes(uint8_t);

  /*
   * Whether we can load this class once and persist it across requests.
   *
   * Persistence is possible when a Class is uniquely named and is defined in a
   * pseudomain that has no side-effects (except other persistent definitions).
   *
   * A class which satisfies isPersistent() may not actually /be/ persistent,
   * if we had to allocate its RDS handle before we loaded the class.
   *
   * @see: classHasPersistentRDS()
   */
  bool isPersistent() const;


  /////////////////////////////////////////////////////////////////////////////
  // Magic methods.                                                     [const]

  /*
   * Get the constructor, destructor, or __toString() method on this class, or
   * nullptr if no such method exists.
   *
   * DeclaredCtor refers to a user-declared __construct() or ClassName(), as
   * opposed to the (shared) empty method generated by the compiler.
   */
  const Func* getCtor() const;
  const Func* getDeclaredCtor() const;
  const Func* getDtor() const;
  const Func* getToString() const;
  const Func* get86pinit() const;
  const Func* get86sinit() const;
  const Func* get86linit() const;

  /*
   * Look up a class' cached __invoke function.  We only cache __invoke methods
   * if they are instance methods or if the class is a static closure.
   */
  const Func* getCachedInvoke() const;

  /*
   * Does this class have a __call method?
   */
  bool hasCall() const;


  /////////////////////////////////////////////////////////////////////////////
  // Builtin classes.                                                   [const]

  /*
   * Is the class a builtin, whether PHP or C++?
   */
  bool isBuiltin() const;

  /*
   * Custom initialization and destruction routines for C++ extension classes.
   *
   * instanceCtor() returns true iff the class is a C++ extension class.
   */
  BuiltinCtorFunction instanceCtor() const;
  BuiltinDtorFunction instanceDtor() const;

  /*
   * Whether this C++ extension class has opted into serialization.
   *
   * @requires: instanceCtor()
   */
  bool isCppSerializable() const;

  /*
   * Whether this is a class for a Hack collection.
   */
  bool isCollectionClass() const;


  /////////////////////////////////////////////////////////////////////////////
  // Methods.

  /*
   * Number of methods on this class.
   *
   * Note that this may differ from m_funcVecLen, since numMethods() is the
   * exact number of methods, and m_funcVecLen is only required to be an upper
   * bound.
   *
   * In particular, outside of RepoAuth mode, trait methods are not transcluded
   * into the Classes which use them, and we are conservative when initially
   * counting methods since we do not resolve trait precedence first.
   */
  size_t numMethods() const;

  /*
   * Get or set a method by its index in the funcVec, which is allocated
   * contiguously before `this' in memory.
   */
  Func* getMethod(Slot idx) const;
  void setMethod(Slot idx, Func* func);

  /*
   * Look up a method by name.
   *
   * Return null if no such method exists.
   */
  Func* lookupMethod(const StringData* methName) const;

  /*
   * Return an Array (via `out') of all the methods of `cls' visible in the
   * context of `ctx' (which may be nullptr).
   *
   * The Array has the form [lowercase name => declared name], ordered with
   * methods implemented by `cls' first, followed by its parents' methods, and
   * so on, in declaration order for each Class in the hierarchy.  Any
   * unimplemented interface methods come last.
   */
  static void getMethodNames(const Class* cls, const Class* ctx, Array& out);


  /////////////////////////////////////////////////////////////////////////////
  // Property metadata.                                                 [const]
  //
  // Unless otherwise specified, the terms "declared instance properties" and
  // "static properties" both refer to properties declared on this class as
  // well as those declared on its ancestors.  Note that this includes private
  // properties in both cases.

  /*
   * Number of declared instance properties or static properties.
   */
  size_t numDeclProperties() const;
  size_t numStaticProperties() const;

  /*
   * Number of declared instance properties that are actually accessible from
   * this class's context.
    *
   * Only really used when iterating over an object's properties.
   */
  uint32_t declPropNumAccessible() const;

  /*
   * The info vector for declared instance properties or static properties.
   */
  folly::Range<const Prop*> declProperties() const;
  folly::Range<const SProp*> staticProperties() const;

  /*
   * Look up the index of a declared instance property or static property.
   *
   * Return kInvalidSlot if no such property exists.
   */
  Slot lookupDeclProp(const StringData* propName) const;
  Slot lookupSProp(const StringData* sPropName) const;

  /*
   * The RepoAuthType of the declared instance property or static property at
   * `index' in the corresponding table.
   */
  RepoAuthType declPropRepoAuthType(Slot index) const;
  RepoAuthType staticPropRepoAuthType(Slot index) const;

  const TypeConstraint& declPropTypeConstraint(Slot index) const;
  const TypeConstraint& staticPropTypeConstraint(Slot index) const;

  /*
   * Whether this class has any properties that require deep initialization.
   *
   * Deep initialization means that the property cannot simply be memcpy'd when
   * creating new objects.
   */
  bool hasDeepInitProps() const;

  /*
   * Whether this class forbids the use of dynamic (non-declared) properties.
   */
  bool forbidsDynamicProps() const;

  /*
   * Whether this class has any immutable properties.
   */
  bool hasImmutableProps() const;

  /*
   * Return true, and set the m_serialized flag, iff this Class hasn't
   * been serialized yet (see prof-data-serialize.cpp).
   *
   * Not thread safe - caller is responsible for any necessary locking.
   */
  bool serialize() const;

  /*
   * Return true if this class was already serialized.
   */
  bool wasSerialized() const;

  /////////////////////////////////////////////////////////////////////////////
  // Property initialization.                                           [const]

  /*
   * Whether this Class requires initialization, either because of nonscalar
   * instance property initializers, simply due to having static properties, or
   * possible property type invariance violations.
   */
  bool needInitialization() const;

  /*
   * Whether this Class potentially has properties which redefine properties in
   * a parent class, and the properties might have inequivalent type-hints. If
   * so, a runtime check is needed during class initialization to possibly raise
   * an error.
   */
  bool maybeRedefinesPropTypes() const;

  /*
   * Whether this Class has properties that require a runtime initial value
   * check.
   */
  bool needsPropInitialValueCheck() const;

  /*
   * Perform request-local initialization.
   *
   * For declared instance properties, this means creating a request-local copy
   * of this Class's PropInitVec.  This is necessary in order to accommodate
   * non-scalar defaults (e.g., class constants), which not be consistent
   * across requests.
   *
   * For static properties, this means setting up request-local memory for the
   * actual static properties, if necessary, and initializing them to their
   * default values.
   */
  void initialize() const;
  void initProps() const;
  void initSProps() const;

  /*
   * Perform a property type-hint redefinition check for the property at a
   * particular slot.
   */
  void checkPropTypeRedefinition(Slot) const;

  /*
   * Check if class has been initialized.
   */
  bool initialized() const;

  /*
   * PropInitVec for this class's declared properties, with default values for
   * scalars only.
   *
   * This is the base from which the request-local copy is made.
   */
  const PropInitVec& declPropInit() const;

  /*
   * Vector of 86pinit non-scalar instance property initializer functions.
   *
   * These are invoked during initProps() to populate the copied PropInitVec.
   */
  const FixedVector<const Func*>& pinitVec() const;

  /*
   * RDS handle which marks whether a property type-hint redefinition check has
   * been performed for this class in this request yet.
   */
  rds::Handle checkedPropTypeRedefinesHandle() const;

  /*
   * RDS handle which marks whether the initial value check has been performed
   * for this class in this request yet.
   */
  rds::Handle checkedPropInitialValuesHandle() const;

  /////////////////////////////////////////////////////////////////////////////
  // Property storage.                                                  [const]

  /*
   * Initialize the RDS handles for the request-local PropInitVec and for the
   * static properties.
   */
  void initPropHandle() const;
  void initSPropHandles() const;

  /*
   * RDS handle of the request-local PropInitVec.
   */
  rds::Handle propHandle() const;

  /*
   * RDS handle for the static properties' is-initialized flag.
   */
  rds::Handle sPropInitHandle() const;

  /*
   * RDS handle for the static property at `index'.
   */
  rds::Handle sPropHandle(Slot index) const;
  rds::Link<StaticPropData, rds::Mode::NonNormal> sPropLink(Slot index) const;
  rds::Link<bool, rds::Mode::NonLocal> sPropInitLink() const;

  /*
   * Get the PropInitVec for the current request.
   */
  PropInitVec* getPropData() const;

  /*
   * Get the value of the static variable at `index' for the current request.
   */
  TypedValue* getSPropData(Slot index) const;


  /////////////////////////////////////////////////////////////////////////////
  // Property lookup and accessibility.                                 [const]

  struct PropValLookup {
    TypedValue* val;
    Slot slot;
    bool accessible;
  };

  struct PropSlotLookup {
    Slot slot;
    bool accessible;
  };

  /*
   * Get the slot and accessibility of a declared instance property on a class
   * from the given context.
   *
   * Accessibility refers to the public/protected/private attribute of the
   * property.
   *
   * Return kInvalidInd for the property iff the property was not declared on
   * this class or any ancestor.  Note that if the return is marked as
   * accessible, then the property must exist.
   */
  PropSlotLookup getDeclPropIndex(const Class*, const StringData*) const;

  /*
   * The equivalent of getDeclPropIndex(), but for static properties.
   */
  PropSlotLookup findSProp(const Class*, const StringData*) const;

  /*
   * Get the request-local value of the static property `sPropName', as well as
   * its accessibility, from the given context.
   *
   * The behavior is identical to that of findSProp(), except substituting
   * nullptr for kInvalidInd.
   *
   * May perform initialization.
   */
  PropValLookup getSProp(const Class*, const StringData*) const;

  /*
   * Return whether or not a declared instance property is accessible from the
   * given context.
   */
  static bool IsPropAccessible(const Prop&, Class*);


  /////////////////////////////////////////////////////////////////////////////
  // Constants.                                                         [const]

  /*
   * Number of class constants.
   */
  size_t numConstants() const;

  /*
   * The info vector for this class's constants.
   */
  const Const* constants() const;

  /*
   * Whether this class has a constant named `clsCnsName'.
   */
  bool hasConstant(const StringData* clsCnsName) const;

  /*
   * Whether this class has a type constant named `typeCnsName'.
   */
  bool hasTypeConstant(const StringData* typeCnsName,
                       bool includeAbs = false) const;

  /*
   * Look up the actual value of a class constant.  Perform dynamic
   * initialization if necessary.
   *
   * Return a Cell containing KindOfUninit if this class has no such constant.
   *
   * The returned Cell is guaranteed not to hold a reference counted object (it
   * may, however, be KindOfString for a static string).
   */
  Cell clsCnsGet(const StringData* clsCnsName,
                 bool includeTypeCns = false) const;

  /*
   * Look up a class constant's TypedValue if it doesn't require dynamic
   * initialization.  The index of the constant is output via `clsCnsInd'.
   *
   * Return nullptr if this class has no constant of the given name.
   *
   * Return nullptr if the constant is abstract.
   *
   * The TypedValue represents the constant's value iff it is a scalar,
   * otherwise it has m_type set to KindOfUninit.  Non-scalar class constants
   * need to run 86cinit code to determine their value at runtime.
   */
  const Cell* cnsNameToTV(const StringData* clsCnsName,
                          Slot& clsCnsInd,
                          bool includeTypeCns = false) const;

  /*
   * Provide the current runtime type of this class constant.
   *
   * This has predictive value for the translator.
   */
  DataType clsCnsType(const StringData* clsCnsName) const;

  /*
   * Get the slot for a constant with name, which can optionally be abstract and
   * either must be or must not be a type constant.
   */
  Slot clsCnsSlot(
    const StringData* name, bool wantTypeCns, bool allowAbstract) const;


  /////////////////////////////////////////////////////////////////////////////
  // Interfaces and traits.

  /*
   * Interfaces this class declared in its "implements" clause.
   */
  folly::Range<const ClassPtr*> declInterfaces() const;

  /*
   * All interfaces implemented by this class, including those declared in
   * traits.
   */
  const InterfaceMap& allInterfaces() const;

  /*
   * Start and end offsets in m_methods of methods that come from used traits.
   *
   * The trait methods are precisely in [m_traitsBeginIdx, m_traitsEndIdx).
   */
  Slot traitsBeginIdx() const;
  Slot traitsEndIdx() const;

  /*
   * Traits used by this class.
   *
   * In RepoAuthoritative mode, we flatten all traits into their users in the
   * compile phase, which leaves m_usedTraits empty as a result.
   */
  const CompactVector<ClassPtr>& usedTraitClasses() const;

  /*
   * Trait alias rules.
   *
   * This is only used by reflection.
   */
  const TraitAliasVec& traitAliases() const;

  /*
   * All trait and interface requirements imposed on this class, including
   * those imposed by traits.
   */
  const RequirementMap& allRequirements() const;

  /*
   * Store a cache of enum values. Takes ownership of 'values'.
   */
  EnumValues* setEnumValues(EnumValues* values);
  EnumValues* getEnumValues() const;

  /////////////////////////////////////////////////////////////////////////////
  // Objects.                                                           [const]

  /*
   * Offset of the declared instance property at `index' on an ObjectData
   * instantiated from this class.
   */
  size_t declPropOffset(Slot index) const;

  /*
   * Whether instances of this class implement Throwable interface, which
   * requires additional initialization on construction.
   */
  bool needsInitThrowable() const;


  /////////////////////////////////////////////////////////////////////////////
  // JIT data.

  /*
   * Get and set the RDS handle for the class with this class's name.
   *
   * We can burn these into the TC even when classes are not persistent, since
   * only a single name-to-class mapping will exist per request.
   */
  rds::Handle classHandle() const;
  void setClassHandle(rds::Link<LowPtr<Class>, rds::Mode::NonLocal> link) const;

  /*
   * Get and set the RDS-cached class with this class's name.
   */
  Class* getCached() const;
  void setCached();


  /////////////////////////////////////////////////////////////////////////////
  // Native data.

  /*
   * NativeData type declared in <<__NativeData("Type")>>.
   */
  const Native::NativeDataInfo* getNativeDataInfo() const;

  /*
   * Whether the class registered native handler of magic props.
   */
  bool hasNativePropHandler() const;

  /*
   * Return the actual native handler of magic props.
   *
   * @requires hasNativePropHandler()
   */
  const Native::NativePropHandler* getNativePropHandler() const;


  /////////////////////////////////////////////////////////////////////////////
  // Closure subclasses.

  /*
   * Is this a scoped subclass of Closure?
   */
  bool isScopedClosure() const;

  /*
   * Return all the scoped clones of this closure class, or an empty map when
   * this is not a closure class.
   *
   * NOTE: Accessing this table is only permitted when synchronized with
   * instance bits initialization.
   *
   * @see: ExtraData::m_scopedClones
   */
  const ScopedClonesMap& scopedClones() const;

  /////////////////////////////////////////////////////////////////////////////
  // Memoization
  //

  /*
   * Whether an object of this class will have memo slots.
   */
  bool hasMemoSlots() const;

  /*
   * Return the number of memo slots an object of this class will require.
   */
  size_t numMemoSlots() const;

  /*
   * Given a function (belonging to this class or a parent), return the slot it
   * should use for memoization, and whether that slot is shared. The function
   * must be a memoize wrapper.
   */
  std::pair<Slot, bool> memoSlotForFunc(FuncId func) const;

  /////////////////////////////////////////////////////////////////////////////
  // LSB Memoize methods
  //
  /*
   * Retrieve the slot corresponding to the value/cache for a function
   * when bound to this class or a subclass.
   * These are for use by the JIT.
   */
  Slot lsbMemoSlot(const Func* func, bool forValue) const;

  /*
   * Get the offset into the Class of the extra structure
   * Used by the JIT to load m_extra
   */
  static constexpr size_t extraOffset() {
    static_assert(
      sizeof(m_extra) == sizeof(ExtraData*),
      "The JIT loads m_extra as a bare pointer");
    return offsetof(Class, m_extra);
  }

  /*
   * Get the offset into the extra structure of m_handles.
   * Used by the JIT.
   */
  static constexpr size_t lsbMemoExtraHandlesOffset() {
    return offsetof(ExtraData, m_lsbMemoExtra) +
           offsetof(LSBMemoExtra, m_handles);
  }

  /////////////////////////////////////////////////////////////////////////////
  // Other methods.
  //
  // Avoiding adding methods to this section.

  /*
   * Whether this class can be made persistent---i.e., if AttrPersistent is set
   * and all parents, interfaces, and traits for this class are persistent.
   */
  bool verifyPersistent() const;

  /*
   * Set the instance bits on this class.
   *
   * The instance bits are a bitfield cache for instanceof checks.  During
   * warmup, we profile the classes and interfaces most commonly checked
   * against in instanceof checks.  Then we cache whether or not this Class
   * satisifes the check in the corresponding bit.
   */
  void setInstanceBits();
  void setInstanceBitsAndParents();
  bool checkInstanceBit(unsigned int bit) const;

  /*
   * Get the underlying enum base type if this is an enum.
   *
   * A return of folly::none represents the `mixed' type.
   */
  MaybeDataType enumBaseTy() const;


  bool needsInitSProps() const;

  static bool compatibleTraitPropInit(const TypedValue& tv1,
                                      const TypedValue& tv2);

  // For assertions:
  bool validate() const;

  /////////////////////////////////////////////////////////////////////////////
  // Offset accessors.                                                 [static]

#define OFF(f)                          \
  static constexpr ptrdiff_t f##Off() { \
    return offsetof(Class, m_##f);      \
  }
  OFF(classVec)
  OFF(classVecLen)
  OFF(instanceBits)
  OFF(invoke)
  OFF(preClass)
  OFF(propDataCache)
  OFF(vtableVecLen)
  OFF(vtableVec)
  OFF(funcVecLen)
  OFF(RTAttrs)
#undef OFF

  static constexpr ptrdiff_t constantsVecOff() {
    return offsetof(Class, m_constants) + ConstMap::vecOff();
  }

  static constexpr ptrdiff_t constantsVecLenOff() {
    return offsetof(Class, m_constants) + ConstMap::sizeOff();
  }

  static constexpr size_t constantsVecLenSize() {
    return ConstMap::sizeSize();
  }

  /////////////////////////////////////////////////////////////////////////////
  // ExtraData.

private:
  struct LSBMemoExtra {
    /*
     * Mapping of methods (declared by this class only) to their assigned slots
     * for LSB memoization. This is populated in the Class ctor when LSB
     * memoized methods are present.
     *
     */
    boost::container::flat_map<FuncId, Slot> m_slots;

    /*
     * The total number of memo slots, and also the next LSB memo slot to
     * assign. This must be larger than our parent's m_numSlots, since slots
     * are inherited.
     */
    Slot m_numSlots{0};

    /*
     * Cached handles to LSB memoization for this class.
     * This array is initialized in the Class ctor, when LSB memoized
     * methods are present.
     */
    rds::Handle* m_handles{nullptr};
  };

  struct ExtraData {
    ~ExtraData();

    /*
     * Vector of (new name, original name) pairs, representing trait aliases.
     */
    TraitAliasVec m_traitAliases;

    /*
     * In RepoAuthoritative mode, we rely on trait flattening in the compile
     * phase to import the contents of traits.  As a result, m_usedTraits is
     * always empty.
     */
    CompactVector<ClassPtr> m_usedTraits;

    /*
     * Only used by reflection for method ordering.  Whenever we have no traits
     * (e.g., in repo mode, where traits are flattened), these will both be 0.
     */
    Slot m_traitsBeginIdx{0};
    Slot m_traitsEndIdx{0};

    /*
     * Builtin-specific data.
     */
    BuiltinCtorFunction m_instanceCtor{nullptr};
    BuiltinDtorFunction m_instanceDtor{nullptr};

    /*
     * Cache for Closure subclass scopings.
     *
     * Only meaningful when `this' is the "template" for a family of Closure
     * subclasses.  When we need to create a closure in the scope of a Class C
     * (and with attrs A), we clone `this', rescope its __invoke()
     * appropriately, and then cache the (C,A) => clone binding here.
     *
     * @see: rescope()
     */
    ScopedClonesMap m_scopedClones;

    /*
     * List of references to Closure subclasses whose scoped Class context is
     * `this'.
     */
    CompactVector<ScopedCloneBackref> m_clonesWithThisScope;

    /*
     * Objects with the <<__NativeData("T")>> UA are allocated with extra space
     * prior to the ObjectData structure itself.
     */
    const Native::NativeDataInfo *m_nativeDataInfo{nullptr};

    /*
     * Cache of persistent enum values, managed by EnumCache.
     */
    std::atomic<EnumValues*> m_enumValues{nullptr};

    /*
     * Mapping of functions (in this class only) to their assigned slots and
     * whether the slot is shared. This is not inherited from the parent.
     */
    boost::container::flat_map<FuncId, std::pair<Slot, bool>> m_memoMappings;
    /*
     * Maps a parameter count to an assigned slot for that count. This is
     * inherited from the parent.
     */
    boost::container::flat_map<size_t, Slot> m_sharedMemoSlots;
    /*
     * The next memo slot to assign. This is inherited from the parent.
     */
    Slot m_nextMemoSlot{0};

    /*
     * MemoizeLSB extra data
     */
    mutable LSBMemoExtra m_lsbMemoExtra;

    /*
     * If initialized, then this class has already performed a property
     * type-hint redefinition check.
     */
    mutable rds::Link<bool, rds::Mode::Normal> m_checkedPropTypeRedefs;

    /*
     * If initialized, then this class has already performed an initial value
     * check.
     */
    mutable rds::Link<bool, rds::Mode::Normal> m_checkedPropInitialValues;
  };

  /*
   * Allocate the ExtraData; done only when necessary.
   */
  void allocExtraData();

  /////////////////////////////////////////////////////////////////////////////
  // Internal types.

private:
  using ConstMap = IndexedStringMap<Const,true,Slot>;
  using PropMap  = IndexedStringMap<Prop,true,Slot>;
  using SPropMap = IndexedStringMap<SProp,true,Slot>;

  struct TraitMethod {
    TraitMethod(const Class* trait_, const Func* method_, Attr modifiers_)
      : trait(trait_)
      , method(method_)
      , modifiers(modifiers_)
    {}

    using class_type = const Class*;
    using method_type = const Func*;
    using modifiers_type = Attr;

    const Class* trait;
    const Func* method;
    Attr modifiers;
  };

  struct TMIOps {
    using prec_type  = const PreClass::TraitPrecRule&;
    using alias_type = const PreClass::TraitAliasRule&;

    // Whether `str' is empty.
    static bool strEmpty(const StringData* str);

    // Return the name for the trait class.
    static const StringData* clsName(const Class* traitCls);

    // Is-a methods.
    static bool isTrait(const Class* traitCls);
    static bool isAbstract(Attr modifiers);

    // Whether to exclude methods with name `methName' when adding.
    static bool exclude(const StringData* methName);

    // TraitMethod constructor.
    static TraitMethod traitMethod(const Class* traitCls,
                                   const Func* traitMeth,
                                   alias_type rule);

    // Accessors for the precedence rule type.
    static const StringData* precMethodName(prec_type rule);
    static const StringData* precSelectedTraitName(prec_type rule);
    static TraitNameSet      precOtherTraitNames(prec_type rule);

    // Accessors for the alias rule type.
    static const StringData* aliasTraitName(alias_type rule);
    static const StringData* aliasOrigMethodName(alias_type rule);
    static const StringData* aliasNewMethodName(alias_type rule);
    static Attr aliasModifiers(alias_type rule);

    // Register a trait alias once the trait class is found.
    static void addTraitAlias(Class* cls, alias_type rule,
                              const Class* traitCls);

    // Trait class/method finders.
    static const Class* findSingleTraitWithMethod(const Class* cls,
                                       const StringData* origMethName);
    static const Class* findTraitClass(const Class* cls,
                                       const StringData* traitName);
    static const Func* findTraitMethod(const Class* cls,
                                       const Class* traitCls,
                                       const StringData* origMethName);

    // Errors.
    static void errorUnknownMethod(prec_type rule);
    static void errorUnknownMethod(alias_type rule,
                                   const StringData* methName);
    template <class Rule>
    static void errorUnknownTrait(const Rule& rule,
                                  const StringData* traitName);
    static void errorDuplicateMethod(const Class* cls,
                                     const StringData* methName);
    static void errorInconsistentInsteadOf(const Class* cls,
                                           const StringData* methName);
    template <class Rule>
    static void errorMultiplyExcluded(const Rule& rule,
                                      const StringData* traitName,
                                      const StringData* methName);
  };

  friend struct TMIOps;

  using TMIData = TraitMethodImportData<TraitMethod,
                                        TMIOps,
                                        const StringData*,
                                        string_data_hash,
                                        string_data_isame>;


  /////////////////////////////////////////////////////////////////////////////
  // Private methods.

private:
  Class(PreClass* preClass,
        Class* parent,
        CompactVector<ClassPtr>&& usedTraits,
        unsigned classVecLen,
        unsigned funcVecLen);
  ~Class();

  /*
   * Trait method import routines.
   */
  void importTraitMethod(const TMIData::MethodData& mdata,
                         MethodMapBuilder& curMethodMap);
  void importTraitMethods(MethodMapBuilder& curMethodMap);
  void applyTraitRules(TMIData& tmid);

  void importTraitProps(int idxOffset,
                        PropMap::Builder& curPropMap,
                        SPropMap::Builder& curSPropMap);
  void importTraitInstanceProp(Class*      trait,
                               Prop&       traitProp,
                               TypedValue& traitPropVal,
                               const int idxOffset,
                               PropMap::Builder& curPropMap);
  void importTraitStaticProp(Class*   trait,
                             SProp&   traitProp,
                             const int idxOffset,
                             PropMap::Builder& curPropMap,
                             SPropMap::Builder& curSPropMap);
  void addTraitPropInitializers(std::vector<const Func*>&, Attr which);

  void checkInterfaceMethods();
  void checkInterfaceConstraints();
  void methodOverrideCheck(const Func* parentMethod, const Func* method);

  void setParent();
  void setSpecial();
  void setMethods();
  void setRTAttributes();
  void setConstants();
  void setProperties();
  void setInitializers();
  void setInterfaces();
  void setInterfaceVtables();
  void setClassVec();
  void setFuncVec(MethodMapBuilder& builder);
  void setRequirements();
  void setEnumType();
  void checkRequirementConstraints() const;
  void raiseUnsatisfiedRequirement(const PreClass::ClassRequirement*) const;
  void setNativeDataInfo();
  void setInstanceMemoCacheInfo();
  void setLSBMemoCacheInfo();

  template<bool setParents> void setInstanceBitsImpl();
  void addInterfacesFromUsedTraits(InterfaceMap::Builder& builder) const;

  void initLSBMemoHandles();
  void checkPropTypeRedefinitions() const;
  void checkPropInitialValues() const;

  /////////////////////////////////////////////////////////////////////////////
  // Static data members.

public:
  /////////////////////////////////////////////////////////////////////////////
  // Data members.
  //
  // Ordered by usage frequency.  Do not re-order for cosmetic reasons.
  //
  // The ordering is reverse order of hotness because m_classVec is relatively
  // hot, and must be the last member.

  LowPtr<Class> m_nextClass{nullptr}; // used by NamedEntity

private:
  static constexpr uint32_t kMagic = 0xce7adb33;

#ifndef NDEBUG
  // For asserts only.
  uint32_t m_magic;
#endif

  default_ptr<ExtraData> m_extra;
  template<class T> friend typename
    std::enable_if<std::is_base_of<c_Awaitable, T>::value, void>::type
  finish_class();

  friend struct collections::CollectionsExtension;

  friend struct StandardExtension;

  RequirementMap m_requirements;
  std::unique_ptr<ClassPtr[]> m_declInterfaces;
  uint32_t m_numDeclInterfaces{0};
  mutable rds::Link<Array, rds::Mode::Normal> m_nonScalarConstantCache;

  LowPtr<Func> m_toString;
  LowPtr<Func> m_invoke; // __invoke, iff non-static (or closure)

  ConstMap m_constants;

  ClassPtr m_parent;
  int32_t m_declPropNumAccessible;
  mutable rds::Link<LowPtr<Class>, rds::Mode::NonLocal> m_cachedClass;

  /*
   * Whether this is a subclass of Closure whose m_invoke->m_cls has been set
   * to the closure's context class.
   */
  std::atomic<bool> m_scoped{false};

  /* This class, or one of its transitive parents, has a property which maybe
   * redefines an existing property in an incompatible way. */
  bool m_maybeRedefsPropTy       : 1;
  /* This class (and not any of its transitive parents) has a property which
   * maybe redefines an existing property in an incompatible way. */
  bool m_selfMaybeRedefsPropTy   : 1;
  /* This class has a property with an initial value which might not satisfy its
   * type-hint (and therefore requires a check when initialized). */
  bool m_needsPropInitialCheck   : 1;
  // NB: 21 bits available here (in USE_LOWPTR builds).

  /*
   * Vector of 86pinit() methods that need to be called to complete instance
   * property initialization, and a pointer to a 86sinit() method that needs to
   * be called to complete static property initialization (or NULL).  Such
   * initialization is triggered only once, the first time one of the following
   * happens:
   *    - An instance of this class is created.
   *    - A static property of this class is accessed.
   */
  FixedVector<const Func*> m_sinitVec;
  FixedVector<const Func*> m_linitVec;
  LowPtr<Func> m_ctor;
  LowPtr<Func> m_dtor;
  PropInitVec m_declPropInit;
  FixedVector<const Func*> m_pinitVec;
  SPropMap m_staticProperties;
  PreClassPtr m_preClass;
  InterfaceMap m_interfaces;

  /*
   * Bitmap of parent classes and implemented interfaces.  Each bit corresponds
   * to a commonly used class name, determined during the profiling warmup
   * requests.
   */
  InstanceBits::BitSet m_instanceBits;
  MethodMap m_methods;

  /*
   * Static properties are stored in RDS.  There are three phases of sprop
   * initialization:
   * 1. The array of links is itself allocated on Class creation.
   * 2. The links are bound either when codegen needs the handle value, or when
   *    initSProps() is called in any request.  Afterwards, m_sPropCacheInit is
   *    bound, defaulting to false.
   * 3. The RDS value at m_sPropCacheInit is set to true when initSProps() is
   *    called, and the values are actually initialized.
   *
   * For non-persistent classes, we put sPropCache in rds::Local, but use the
   * m_sPropCacheInit flag to indicate whether sPropCache needs to be
   * reinitialized.
   */
  mutable rds::Link<StaticPropData, rds::Mode::NonNormal>*
    m_sPropCache{nullptr};
  mutable rds::Link<bool, rds::Mode::NonLocal> m_sPropCacheInit;

  veclen_t m_classVecLen;
  veclen_t m_funcVecLen;
  veclen_t m_vtableVecLen{0};
  LowPtr<VtableVecSlot> m_vtableVec{nullptr};

  /*
   * Each ObjectData is created with enough trailing space to directly store
   * the vector of declared properties. To look up a property by name and
   * determine whether it is declared, use m_declPropMap. If the declared
   * property index is already known (as may be the case when executing via the
   * TC), property metadata in m_declPropInfo can be directly accessed.
   *
   * m_declPropInit is indexed by the Slot values from m_declProperties, and
   * contains initialization information.
   */
  PropMap m_declProperties;

  MaybeDataType m_enumBaseTy;

  /*
   * runtime attributes computed at runtime init time. Not to be confused with
   * m_attrCopy which are compile-time and stored in the repo.
   */
  uint8_t m_RTAttrs;

  /*
   * Default ObjectData::Attribute bits for new instances
   */
  uint8_t m_ODAttrs;

  mutable rds::Link<PropInitVec*, rds::Mode::Normal> m_propDataCache;

  /*
   * Whether the Class requires initialization, because it has either
   * {p,s}init() methods or static members, or possibly has prop type invariance
   * violations.
   */
  bool m_needInitialization : 1;

  bool m_needsInitThrowable : 1;
  bool m_hasDeepInitProps : 1;
  /*
   * Whether this class has been serialized yet.
   */
  mutable bool m_serialized : 1;
  /*
   * Cache of m_preClass->attrs().
   */
  unsigned m_attrCopy : 28;

  /*
   * Vector of Class pointers that encodes the inheritance hierarchy, including
   * this Class as the last element.
   */
  LowPtr<Class> m_classVec[1]; // Dynamically sized; must come last.
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Global lock used during class loading.
 */
extern Mutex g_classesMutex;

///////////////////////////////////////////////////////////////////////////////

/*
 * Class kinds---classes, interfaces, traits, and enums.
 *
 * "Normal class" refers to any classes that are not interfaces, traits, enums.
 */
enum class ClassKind {
  Class = AttrNone,
  Interface = AttrInterface,
  Trait = AttrTrait,
  Enum = AttrEnum
};

Attr classKindAsAttr(ClassKind kind);

bool isTrait(const Class* cls);
bool isInterface(const Class* cls);
bool isEnum(const Class* cls);
bool isAbstract(const Class* cls);
bool isNormalClass(const Class* cls);

/*
 * Whether a class is persistent /and/ has a persistent RDS handle.  You
 * probably mean this instead of cls->isPersistent(), which only checks the
 * attributes.
 *
 * A persistent class can end up with a non-persistent RDS handle if we had to
 * allocate the handle before we loaded the class.
 */
bool classHasPersistentRDS(const Class* cls);

/*
 * Returns whether cls or any of its children may have magic property methods.
 */
bool classMayHaveMagicPropMethods(const Class* cls);

/*
 * Return the class that "owns" f.  This will normally be f->cls(), but for
 * Funcs with static locals, f may have been cloned into a derived class.
 *
 * @requires: RuntimeOption::EvalPerfDataMap
 */
const Class* getOwningClassForFunc(const Func* f);

/*
 * Convert a class pointer where a string is needed in some context. A warning
 * will be raised when compiler option Eval.RaiseClassConversionWarning is true.
 */
const StringData* classToStringHelper(const Class* cls);

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_VM_CLASS_INL_H_
#include "hphp/runtime/vm/class-inl.h"
#undef incl_HPHP_VM_CLASS_INL_H_

#endif // incl_HPHP_VM_CLASS_H_
