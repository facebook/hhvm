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

#ifndef incl_HPHP_VM_CLASS_H_
#define incl_HPHP_VM_CLASS_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/runtime/vm/fixed-string-map.h"
#include "hphp/runtime/vm/indexed-string-map.h"
#include "hphp/runtime/vm/instance-bits.h"
#include "hphp/runtime/vm/preclass.h"

#include <boost/range/iterator_range.hpp>

#include <list>
#include <memory>
#include <utility>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Class;
struct ClassInfo;
struct ClassInfoVM;
struct Func;
struct HhbcExtClassInfo;
struct StringData;

namespace Native { struct NativeDataInfo; }

///////////////////////////////////////////////////////////////////////////////

typedef AtomicSmartPtr<Class> ClassPtr;

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
   * Instance property information.
   */
  struct Prop {
    // m_name is "" for inaccessible properties (i.e. private properties
    // declared by parents).
    LowStringPtr m_name;
    LowStringPtr m_mangledName;
    LowStringPtr m_originalMangledName;
    // First parent class that declares this property.
    LowClassPtr m_class;
    Attr m_attrs;
    LowStringPtr m_typeConstraint;
    // When built in RepoAuthoritative mode, this is a control-flow
    // insensitive, always-true type assertion for this property.  (It may be
    // Gen if there was nothing interesting known.)
    RepoAuthType m_repoAuthType;
    LowStringPtr m_docComment;
    int m_idx;
  };

  /*
   * Static property information.
   */
  struct SProp {
    LowStringPtr m_name;
    Attr m_attrs;
    LowStringPtr m_typeConstraint;
    LowStringPtr m_docComment;
    // Most derived class that declared this property.
    LowClassPtr m_class;
    // Used if (m_class == this).
    TypedValue m_val;
    RepoAuthType m_repoAuthType;
    int m_idx;
  };

  /*
   * Class constant information.
   */
  struct Const {
    // Most derived class that declared this constant.
    LowClassPtr m_class;
    LowStringPtr m_name;
    TypedValue m_val;
    LowStringPtr m_phpCode;
    LowStringPtr m_typeConstraint;
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

    typedef TypedValueAux* iterator;

    iterator begin();
    iterator end();
    size_t size() const;

    TypedValueAux& operator[](size_t i);
    const TypedValueAux& operator[](size_t i) const;

    void push_back(const TypedValue& v);

    /*
     * Make a smart-allocated copy of `src'.
     */
    static PropInitVec* allocWithSmartAllocator(const PropInitVec& src);

    static size_t dataOff() {
      return offsetof(PropInitVec, m_data);
    }

  private:
    PropInitVec(const PropInitVec&);

    TypedValueAux* m_data;
    unsigned m_size;
    bool m_smart;
  };

  /*
   * Container typedefs.
   */
  typedef std::vector<std::pair<LowStringPtr,LowStringPtr>> TraitAliasVec;

  typedef FixedStringMap<Slot, false, Slot> MethodMap;
  typedef FixedStringMapBuilder<Func*, Slot, false, Slot> MethodMapBuilder;
  typedef IndexedStringMap<LowClassPtr, true, int> InterfaceMap;
  typedef IndexedStringMap<
    const PreClass::ClassRequirement*, true, int> RequirementMap;


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
   * The start of malloc'd memory for `this' (i.e., including the method
   * table).
   */
  Func** mallocPtrFromThis() const;

  /*
   * Pointer to the array of Class pointers, allocated immediately after
   * `this', which contain this class's inheritance hierarchy (including `this'
   * as the last element).
   */
  const LowClassPtr* classVec() const;

  /*
   * The size of the classVec.
   */
  unsigned classVecLen() const;


  /////////////////////////////////////////////////////////////////////////////
  // Ancestry.                                                          [const]

  /*
   * Determine if this represents a non-strict subtype of `cls'.
   *
   * Returns uint64_t instead of bool because it's called directly from the TC.
   */
  uint64_t classof(const Class* cls) const;

  /*
   * Whether this class implements an interface called `name'.
   */
  bool ifaceofDirect(const StringData* name) const;

  /*
   * Assuming this and cls are both regular classes (not interfaces or traits),
   * return their lowest common ancestor, or nullptr if they're unrelated.
   */
  const Class* commonAncestor(const Class* cls) const;


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
   * Whether we can load this class once and persist it across requests.
   *
   * Persistence is possible when a Class is uniquely named and is defined in a
   * psuedomain that has no side-effects (except other persistent definitions).
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
   * opposed to the default 86ctor() method generated by the compiler.
   */
  const Func* getCtor() const;
  const Func* getDeclaredCtor() const;
  const Func* getDtor() const;
  const Func* getToString() const;


  /////////////////////////////////////////////////////////////////////////////
  // Builtin classes.                                                   [const]

  /*
   * Is the class a builtin, whether PHP or C++?
   */
  bool isBuiltin() const;

  /*
   * Return the ClassInfo for a C++ extension class.
   */
  const ClassInfo* clsInfo() const;

  /*
   * Custom initialization and destruction routines for C++ extension classes.
   *
   * instanceCtor() returns true iff the class is a C++ extension class.
   */
  BuiltinCtorFunction instanceCtor() const;
  BuiltinDtorFunction instanceDtor() const;

  /*
   * This value is the pointer adjustment from an ObjectData* to get to the
   * property vector, for a builtin class.  In other words, it's the number of
   * bytes of subclass data following the ObjectData subobject.
   */
  int32_t builtinODTailSize() const;

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
   * Find the first class in the inheritance hierarchy which defines the method
   * called `methName'.  Return null if no such method is defined anywhere in
   * the hierarchy.
   *
   * For trait methods, the base class is the one that uses/imports the trait.
   */
  Class* findMethodBaseClass(const StringData* methName);

  /*
   * Whether this class (as opposed to some parent class) declared the given
   * method.
   *
   * For trait methods, the class declaring the method is the one that
   * uses/imports the trait.
   */
  bool declaredMethod(const Func* method);


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
  const Prop* declProperties() const;
  const SProp* staticProperties() const;

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

  /*
   * Whether this class has any properties that require deep initialization.
   *
   * Deep initialization means that the property cannot simply be memcpy'd when
   * creating new objects.
   */
  bool hasDeepInitProps() const;


  /////////////////////////////////////////////////////////////////////////////
  // Property initialization.                                           [const]

  /*
   * Whether this Class requires initialization, either because of nonscalar
   * instance property initializers, or simply due to having static properties.
   */
  bool needInitialization() const;

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
  const std::vector<const Func*>& pinitVec() const;


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
  RDS::Handle propHandle() const;

  /*
   * RDS handle for the static properties' is-initialized flag.
   */
  RDS::Handle sPropInitHandle() const;

  /*
   * RDS handle for the static property at `index'.
   */
  RDS::Handle sPropHandle(Slot index) const;

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

  /*
   * Get the slot and accessibility of the declared instance property `key' on
   * this class from the context `ctx'.
   *
   * Accessibility refers to the public/protected/private attribute of the
   * property.  The value of `accessible' is output by reference.
   *
   * Return kInvalidInd iff the property was not declared on this class or any
   * ancestor.  Note that if `accessible' is true, then the property must
   * exist.
   */
  Slot getDeclPropIndex(Class* ctx, const StringData* key,
                        bool& accessible) const;

  /*
   * Get the slot, visibility, and accessibility of the static property
   * `sPropName on this class from the context `ctx'.
   *
   * Visibility refers to whether or not the property exists at all.
   * Accessibility refers to the public/protected/private attribute of the
   * property.
   *
   * Both `visible' and `accessible' are output by reference.
   *
   * Return kInvalidInd (and set `visible' to false) iff the property does not
   * exist.  Note also that if `accessible' is true, then the property must
   * exist.
   */
  Slot findSProp(Class* ctx, const StringData* sPropName,
                 bool& visible, bool& accessible) const;

  /*
   * Get the request-local value of the static property `sPropName', as well as
   * its visibility and accessibility, from the context `ctx'.
   *
   * The behavior is identical to that of findSProp(), except substituting
   * nullptr for kInvalidInd.
   *
   * May perform initialization.
   */
  TypedValue* getSProp(Class* ctx, const StringData* sPropName,
                       bool& visible, bool& accessible) const;

  /*
   * Identical to getSProp(), but the output is boxed.
   *
   * Used by the ext_zend_compat layer.
   */
  RefData* zGetSProp(Class* ctx, const StringData* sPropName,
                     bool& visible, bool& accessible) const;

  /*
   * Return whether or not the declared instance property described by `prop'
   * is accessible from the context `ctx'.
   */
  static bool IsPropAccessible(const Prop& prop, Class* ctx);


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
   * Look up the actual value of a class constant.  Perform dynamic
   * initialization if necessary.
   *
   * Return a Cell containing KindOfUninit if this class has no such constant.
   *
   * The returned Cell is guaranteed not to hold a reference counted object (it
   * may, however, be KindOfString for a static string).
   */
  Cell clsCnsGet(const StringData* clsCnsName) const;

  /*
   * Look up a class constant's TypedValue if it doesn't require dynamic
   * initialization.  The index of the constant is output via `clsCnsInd'.
   *
   * Return nullptr if this class has no constant of the given name.
   *
   * The TypedValue represents the constant's value iff it is a scalar,
   * otherwise it has m_type set to KindOfUninit.  Non-scalar class constants
   * need to run 86cinit code to determine their value at runtime.
   */
  const Cell* cnsNameToTV(const StringData* clsCnsName, Slot& clsCnsInd) const;

  /*
   * Provide the current runtime type of this class constant.
   *
   * This has predictive value for the translator.
   */
  DataType clsCnsType(const StringData* clsCnsName) const;


  /////////////////////////////////////////////////////////////////////////////
  // Interfaces and traits.

  /*
   * Interfaces this class declared in its "implements" clause.
   */
  boost::iterator_range<const ClassPtr*> declInterfaces() const;

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
  const std::vector<ClassPtr>& usedTraitClasses() const;

  /*
   * Vector of (new name, original name) pairs representing trait aliases.
   *
   * Not const due to memoization; this is only used for reflection.
   */
  const TraitAliasVec& traitAliases();

  /*
   * All trait and interface requirements imposed on this class, including
   * those imposed by traits.
   */
  const RequirementMap& allRequirements() const;


  /////////////////////////////////////////////////////////////////////////////
  // Objects.                                                           [const]

  /*
   * Offset of the declared instance property at `index' on an ObjectData
   * instantiated from this class.
   */
  size_t declPropOffset(Slot index) const;

  /*
   * Whether instances of this class need to call a custom __init__ when
   * created.
   */
  bool callsCustomInstanceInit() const;


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
   * Populate the ClassInfoVM for this class in `ci'.
   */
  void getClassInfo(ClassInfoVM* ci);

  /*
   * Get and set the RDS handle for the class with this class's name.
   *
   * We can burn these into the TC even when classes are not persistent, since
   * only a single name-to-class mapping will exist per request.
   */
  RDS::Handle classHandle() const;
  void setClassHandle(RDS::Link<Class*> link) const;

  /*
   * Get and set the RDS-cached class with this class's name.
   */
  Class* getCached() const;
  void setCached();

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

  /*
   * NativeData type declared in <<__NativeData("Type")>>.
   */
  const Native::NativeDataInfo* getNativeDataInfo() const;

  /*
   * Get the underlying enum base type if this is an enum.
   */
  DataType enumBaseTy() const;


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
#undef OFF


  /////////////////////////////////////////////////////////////////////////////
  // Internal types.

private:
  typedef IndexedStringMap<Const,true,Slot> ConstMap;
  typedef IndexedStringMap<Prop,true,Slot> PropMap;
  typedef IndexedStringMap<SProp,true,Slot> SPropMap;

  struct TraitMethod {
    TraitMethod(Class* trait, Func* method, Attr modifiers)
      : m_trait(trait)
      , m_method(method)
      , m_modifiers(modifiers)
    {}

    LowClassPtr m_trait;
    Func* m_method;
    Attr m_modifiers;
  };

  typedef std::list<TraitMethod> TraitMethodList;
  typedef hphp_hash_map<LowStringPtr,
                        TraitMethodList,
                        string_data_hash,
                        string_data_isame> MethodToTraitListMap;


  /////////////////////////////////////////////////////////////////////////////
  // Private methods.

private:
  Class(PreClass* preClass,
        Class* parent,
        std::vector<ClassPtr>&& usedTraits,
        unsigned classVecLen,
        unsigned funcVecLen);
  ~Class();

  bool needsInitSProps() const;

  void importTraitMethod(const TraitMethod&  traitMethod,
                         const StringData*   methName,
                         MethodMapBuilder& curMethodMap);
  Class* findSingleTraitWithMethod(const StringData* methName);
  void setImportTraitMethodModifiers(TraitMethodList& methList,
                                     Class*           traitCls,
                                     Attr             modifiers);
  void importTraitMethods(MethodMapBuilder& curMethodMap);
  void addTraitPropInitializers(bool staticProps);
  void applyTraitRules(MethodToTraitListMap& importMethToTraitMap);
  void applyTraitPrecRule(const PreClass::TraitPrecRule& rule,
                          MethodToTraitListMap& importMethToTraitMap);
  void applyTraitAliasRule(const PreClass::TraitAliasRule& rule,
                           MethodToTraitListMap& importMethToTraitMap);
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
  void addTraitAlias(const StringData* traitName,
                     const StringData* origMethName,
                     const StringData* newMethName);

  void checkInterfaceMethods();
  void methodOverrideCheck(const Func* parentMethod, const Func* method);

  static bool compatibleTraitPropInit(TypedValue& tv1, TypedValue& tv2);
  void removeSpareTraitAbstractMethods(
    MethodToTraitListMap& importMethToTraitMap);

  void setParent();
  void setSpecial();
  void setMethods();
  void setODAttributes();
  void setConstants();
  void setProperties();
  void setInitializers();
  void setInterfaces();
  void setClassVec();
  void setFuncVec(MethodMapBuilder& builder);
  void setRequirements();
  void setEnumType();
  void checkRequirementConstraints() const;
  void raiseUnsatisfiedRequirement(const PreClass::ClassRequirement*) const;
  void setNativeDataInfo();

  template<bool setParents> void setInstanceBitsImpl();
  void addInterfacesFromUsedTraits(InterfaceMap::Builder& builder) const;


  /////////////////////////////////////////////////////////////////////////////
  // Static data members.

public:
  /*
   * A hashtable that maps class names to structures containing C++ function
   * pointers for the class's methods and constructors.
   */
  static hphp_hash_map<const StringData*,
                       const HhbcExtClassInfo*,
                       string_data_hash,
                       string_data_isame> s_extClassHash;

  /*
   * Callback which, if set, runs during setMethods().
   */
  static void (*MethodCreateHook)(Class* cls, MethodMapBuilder& builder);


  /////////////////////////////////////////////////////////////////////////////
  // Data members.
  //
  // Ordered by usage frequency.  Do not re-order for cosmetic reasons.
  //
  // The ordering is reverse order of hotness because m_classVec is relatively
  // hot, and must be the last member.

public:
  Class* m_nextClass{nullptr}; // used by NamedEntity

private:
  DataType m_enumBaseTy;

  // Objects with the <<__NativeData("T")>> UA are allocated with extra space
  // prior to the ObjectData structure itself.
  const Native::NativeDataInfo *m_nativeDataInfo{nullptr};
  std::unique_ptr<ClassPtr[]> m_declInterfaces;

  TraitAliasVec m_traitAliases;

  Slot m_traitsBeginIdx{0};
  Slot m_traitsEndIdx{0};
  mutable RDS::Link<Array> m_nonScalarConstantCache{RDS::kInvalidHandle};
  size_t m_numDeclInterfaces{0};
  Func* m_toString;

  // In RepoAuthoritative mode, we rely on trait flattening in the compile
  // phase to import the contents of traits.  As a result, m_usedTraits is
  // always empty.
  std::vector<ClassPtr> m_usedTraits;
  ConstMap m_constants;
  ClassPtr m_parent;
  int32_t m_declPropNumAccessible;
  mutable RDS::Link<Class*> m_cachedClass{RDS::kInvalidHandle};

  // Vector of 86pinit() methods that need to be called to complete instance
  // property initialization, and a pointer to a 86sinit() method that needs to
  // be called to complete static property initialization (or NULL).  Such
  // initialization is triggered only once, the first time one of the following
  // happens:
  //    - An instance of this class is created.
  //    - A static property of this class is accessed.
  std::vector<const Func*> m_sinitVec;
  const ClassInfo* m_clsInfo{nullptr};
  Func* m_invoke; // __invoke, iff non-static (or closure)
  Func* m_ctor;
  PropInitVec m_declPropInit;
  std::vector<const Func*> m_pinitVec;
  SPropMap m_staticProperties;
  BuiltinCtorFunction m_instanceCtor{nullptr};
  mutable RDS::Link<PropInitVec*> m_propDataCache{RDS::kInvalidHandle};
  uint32_t m_builtinODTailSize{0};
  PreClassPtr m_preClass;
  InterfaceMap m_interfaces;
  RequirementMap m_requirements;
  // Bitmap of parent classes and implemented interfaces.  Each bit corresponds
  // to a commonly used class name, determined during the profiling warmup
  // requests.
  InstanceBits::BitSet m_instanceBits;
  BuiltinDtorFunction m_instanceDtor{nullptr};
  Func* m_dtor;
  MethodMap m_methods;

  // Static properties are stored in RDS.  There are three phases of sprop
  // initialization:
  // 1. The array of links is itself allocated on Class creation.
  // 2. The links are bound either when codegen needs the handle value, or when
  //    initSProps() is called in any request.  Afterwards, m_sPropCacheInit is
  //    bound, defaulting to false.
  // 3. The RDS value at m_sPropCacheInit is set to true when initSProps() is
  //    called, and the values are actually initialized.
  mutable RDS::Link<TypedValue>* m_sPropCache{nullptr};
  mutable RDS::Link<bool> m_sPropCacheInit{RDS::kInvalidHandle};

  unsigned m_classVecLen;
  unsigned m_funcVecLen;

  // Each ObjectData is created with enough trailing space to directly store
  // the vector of declared properties. To look up a property by name and
  // determine whether it is declared, use m_declPropMap. If the declared
  // property index is already known (as may be the case when executing via the
  // TC), property metadata in m_declPropInfo can be directly accessed.
  //
  // m_declPropInit is indexed by the Slot values from m_declProperties, and
  // contains initialization information.
  PropMap m_declProperties;

  int32_t m_ODAttrs;
  unsigned m_needInitialization : 1;      // requires initialization,
                                          // due to [ps]init or simply
                                          // having static members
  unsigned m_completelyUnused : 1;        // keep things in the same place
  unsigned m_callsCustomInstanceInit : 1; // should we always call __init__
                                          // on new instances?
  unsigned m_hasDeepInitProps : 1;
  unsigned m_attrCopy : 28;               // cache of m_preClass->attrs().

  // Vector of Class pointers that encodes the inheritance hierarchy,
  // including this Class as the last element.
  LowClassPtr m_classVec[1]; // Dynamically sized; must come last.
};

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
 * Return the class that "owns" f.  This will normally be f->cls(), but for
 * Funcs with static locals, f may have been cloned into a derived class.
 *
 * @requires: RuntimeOption::EvalPerfDataMap
 */
const Class* getOwningClassForFunc(const Func* f);

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_VM_CLASS_INL_H_
#include "hphp/runtime/vm/class-inl.h"
#undef incl_HPHP_VM_CLASS_INL_H_

#endif // incl_HPHP_VM_CLASS_H_
