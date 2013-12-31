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

#ifndef incl_HPHP_VM_CLASS_H_
#define incl_HPHP_VM_CLASS_H_

#include <boost/range/iterator_range.hpp>

#include "hphp/util/fixed-vector.h"
#include "hphp/util/range.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/hphp-value.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/fixed-string-map.h"
#include "hphp/runtime/vm/instance-bits.h"
#include "hphp/runtime/vm/indexed-string-map.h"
#include "hphp/runtime/base/runtime-option.h"

namespace HPHP {

class ClassInfo;
class ClassInfoVM;
class HphpArray;
class ObjectData;
struct HhbcExtClassInfo;
class Func;
class FuncEmitter;
class Unit;
class UnitEmitter;
class Class;
class NamedEntity;
class PreClass;

typedef hphp_hash_set<const StringData*, string_data_hash,
                      string_data_isame> TraitNameSet;
typedef hphp_hash_set<const Class*, pointer_hash<Class> > ClassSet;

/*
 * User attributes on various runtime structures are stored in this
 * map, currently.
 */
typedef hphp_hash_map<
  const StringData*,
  TypedValue,
  string_data_hash,
  string_data_isame
> UserAttributeMap;

using BuiltinCtorFunction = ObjectData* (*)(Class*);
using BuiltinDtorFunction = void (*)(ObjectData*, const Class*);

/*
 * A PreClass represents the source-level definition of a php class,
 * interface, or trait.  Includes things like the names of the parent
 * class (if any), and the names of any interfaces implemented or
 * traits used.  Also contains metadata about properties and methods
 * of the class.
 *
 * This is separate from an actual Class because in different requests
 * (depending on include order), the actual instantiation of a
 * PreClass may differ since names may have different meanings.  For
 * example, if the PreClass "extends Foo", and Foo is defined
 * differently in different requests, we will make a different Class.
 *
 * Hoistability:
 *
 *    When a Unit is loaded at run time, each PreClass in the Unit
 *    which is determined to be `hoistable' will be loaded by the
 *    runtime (in the order they appear in the source) before the
 *    Unit's pseudo-main is executed.  The hoistability rules ensure
 *    that loading a PreClass which is determined to be hoistable will
 *    never cause the autoload facility to be invoked.
 *
 *    A class is considered `hoistable' iff the following conditions
 *    apply:
 *
 *      - It's defined at the top level.
 *
 *      - There is no other definition for a class of the same name in
 *        its Unit.
 *
 *      - It uses no traits.  (It may however *be* a trait.)
 *
 *      - It implements no interfaces.  (It may however *be* an
 *        interface.)
 *
 *      - It has no parent OR
 *           The parent is hoistable and defined earlier in the unit OR
 *           The parent is already defined when the unit is required
 *
 *    The very last condition here (parent already defined when the
 *    unit is required) is not known at parse time.  This leads to the
 *    Maybe/Always split below.
 *
 */
class PreClass : public AtomicCountable {
  friend class PreClassEmitter;
  friend class Peephole;

 public:
  enum Hoistable {
    NotHoistable,
    Mergeable,
    MaybeHoistable,
    AlwaysHoistable,
    ClosureHoistable
  };

  struct Prop {
    Prop() {}
    Prop(PreClass* preClass,
         const StringData* n,
         Attr attrs,
         const StringData* typeConstraint,
         const StringData* docComment,
         const TypedValue& val,
         DataType hphpcType);

    void prettyPrint(std::ostream& out) const;

    PreClass* preClass() const { return m_preClass; }
    const StringData* name() const { return m_name; }
    const String& nameRef() const { return *(String*)&m_name; }
    const StringData* mangledName() const { return m_mangledName; }
    const String& mangledNameRef() const { return *(String*)(&m_mangledName); }
    Attr attrs() const { return m_attrs; }
    const StringData* typeConstraint() const { return m_typeConstraint; }
    DataType hphpcType() const { return m_hphpcType; }
    const StringData* docComment() const { return m_docComment; }
    const TypedValue& val() const { return m_val; }

  private:
    PreClass* m_preClass;
    const StringData* m_name;
    const StringData* m_mangledName;
    Attr m_attrs;
    const StringData* m_typeConstraint;
    const StringData* m_docComment;
    TypedValue m_val;
    DataType m_hphpcType;
  };

  struct Const {
    Const() {}
    Const(PreClass* preClass, const StringData* n,
          const StringData* typeConstraint, const TypedValue& val,
          const StringData* phpCode);

    void prettyPrint(std::ostream& out) const;

    PreClass* preClass() const { return m_preClass; }
    const StringData* name() const { return m_name; }
    const String& nameRef() const { return *(String*)&m_name; }
    const StringData* typeConstraint() const { return m_typeConstraint; }
    const TypedValue& val() const { return m_val; }
    const StringData* phpCode() const { return m_phpCode; }

  private:
    PreClass* m_preClass;
    const StringData* m_name;
    const StringData* m_typeConstraint;
    TypedValue m_val;
    const StringData* m_phpCode;
  };

  class TraitPrecRule {
   public:
    TraitPrecRule()
      : m_methodName(0)
      , m_selectedTraitName(0)
    {}

    TraitPrecRule(const StringData* selectedTraitName,
                  const StringData* methodName) :
        m_methodName(methodName), m_selectedTraitName(selectedTraitName),
        m_otherTraitNames() {}

    void addOtherTraitName(const StringData* traitName) {
      m_otherTraitNames.insert(traitName);
    }
    const StringData* getMethodName() const { return m_methodName; }
    const StringData* getSelectedTraitName() const {
      return m_selectedTraitName;
    }
    TraitNameSet getOtherTraitNames() const {
      return m_otherTraitNames;
    }

    template<class SerDe> void serde(SerDe& sd) {
      sd(m_methodName)(m_selectedTraitName)(m_otherTraitNames);
    }

   private:
    const StringData*  m_methodName;
    const StringData*  m_selectedTraitName;
    TraitNameSet       m_otherTraitNames;
  };

  class TraitAliasRule {
   public:
    TraitAliasRule()
      : m_traitName(0)
      , m_origMethodName(0)
      , m_newMethodName(0)
      , m_modifiers(AttrNone)
    {}

    TraitAliasRule(const StringData* traitName,
                   const StringData* origMethodName,
                   const StringData* newMethodName,
                   Attr              modifiers) :
        m_traitName(traitName),
        m_origMethodName(origMethodName),
        m_newMethodName(newMethodName),
        m_modifiers(modifiers) {}

    const StringData* getTraitName() const      { return m_traitName; }
    const StringData* getOrigMethodName() const { return m_origMethodName; }
    const StringData* getNewMethodName() const  { return m_newMethodName; }
    Attr              getModifiers() const      { return m_modifiers; }

    template<class SerDe> void serde(SerDe& sd) {
      sd(m_traitName)(m_origMethodName)(m_newMethodName)(m_modifiers);
    }

   private:
    const StringData* m_traitName;
    const StringData* m_origMethodName;
    const StringData* m_newMethodName;
    Attr              m_modifiers;
  };

  struct TraitRequirement {
   public:
    // Solely for SerDe
    TraitRequirement(): m_word(0) {}

    explicit TraitRequirement(const StringData* req, bool isExtends) {
      m_word = pack(req, isExtends);
    }

    const StringData* name() const {
      return reinterpret_cast<const StringData*>(m_word & ~0x1);
    }
    bool is_extends() const { return m_word & 0x1; }
    bool is_implements() const { return !is_extends(); }

    // Deserialization version.
    template<class SerDe>
    typename boost::enable_if_c<SerDe::deserializing>::type
    serde(SerDe& sd) {
      const StringData* sd_name;
      bool sd_is_extends;
      sd(sd_name)(sd_is_extends);
      m_word = pack(sd_name, sd_is_extends);
    }

    // Serialization version.
    template<class SerDe>
    typename boost::disable_if_c<SerDe::deserializing>::type
    serde(SerDe& sd) {
      const StringData* sd_name = name();
      bool sd_is_extends = is_extends();
      sd(sd_name)(sd_is_extends);
    }

   private:
    static uintptr_t pack(const StringData* req, bool isExtends) {
      auto req_ptr = reinterpret_cast<uintptr_t>(req);
      return isExtends ? (req_ptr | 0x1) : req_ptr;
    }

    uintptr_t m_word;
  };

  typedef FixedVector<const StringData*> InterfaceVec;
  typedef FixedVector<const StringData*> UsedTraitVec;
  typedef FixedVector<TraitRequirement> TraitRequirementsVec;
  typedef FixedVector<TraitPrecRule> TraitPrecRuleVec;
  typedef FixedVector<TraitAliasRule> TraitAliasRuleVec;

  PreClass(Unit* unit, int line1, int line2, Offset o, const StringData* n,
           Attr attrs, const StringData* parent, const StringData* docComment,
           Id id, Hoistable hoistable);
  ~PreClass();
  void atomicRelease();

  Unit* unit() const { return m_unit; }
  int line1() const { return m_line1; }
  int line2() const { return m_line2; }
  Offset getOffset() const { return m_offset; }
  const StringData* name() const { return m_name; }
  const String& nameRef() const { return *(String*)(&m_name); }
  Attr attrs() const { return m_attrs; }
  static Offset attrsOffset() { return offsetof(PreClass, m_attrs); }
  const StringData* parent() const { return m_parent; }
  const String& parentRef() const { return *(String*)(&m_parent); }
  const StringData* docComment() const { return m_docComment; }
  Id id() const { return m_id; }
  const InterfaceVec& interfaces() const { return m_interfaces; }
  const UsedTraitVec& usedTraits() const { return m_usedTraits; }
  const TraitRequirementsVec& traitRequirements() const {
    return m_traitRequirements;
  }
  const TraitPrecRuleVec& traitPrecRules() const { return m_traitPrecRules; }
  const TraitAliasRuleVec& traitAliasRules() const { return m_traitAliasRules; }
  const UserAttributeMap& userAttributes() const { return m_userAttributes; }
  bool isPersistent() const { return m_attrs & AttrPersistent; }
  bool isBuiltin() const { return attrs() & AttrBuiltin; }

  /*
   * Funcs, Consts, and Props all behave similarly. Define raw accessors
   * foo() and numFoos() for people munging by hand, and ranges.
   *
   *   methods();   numMethods();    FuncRange allMethods();
   *   constants(); numConstants();  ConstRange allConstants();
   *   properties;  numProperties(); PropRange allProperties();
   */

#define DEF_ACCESSORS(Type, TypeName, fields, Fields)                   \
  Type const* fields() const { return m_##fields.accessList(); }        \
  Type*       mutable##Fields() { return m_##fields.mutableAccessList(); } \
  size_t num##Fields()  const { return m_##fields.size(); }             \
  typedef IterRange<Type const*> TypeName##Range;                       \
  TypeName##Range all##Fields() const {                                 \
    return TypeName##Range(fields(), fields() + m_##fields.size());     \
  }

  DEF_ACCESSORS(Func*, Func, methods, Methods)
  DEF_ACCESSORS(Const, Const, constants, Constants)
  DEF_ACCESSORS(Prop, Prop, properties, Properties)

#undef DEF_ACCESSORS

  bool hasMethod(const StringData* methName) const {
    return m_methods.contains(methName);
  }

  bool hasProp(const StringData* propName) const {
    return m_properties.contains(propName);
  }

  Func* lookupMethod(const StringData* methName) const {
    Func* f = m_methods.lookupDefault(methName, 0);
    assert(f != nullptr);
    return f;
  }

  const Prop* lookupProp(const StringData* propName) const {
    Slot s = m_properties.findIndex(propName);
    assert(s != kInvalidSlot);
    return &m_properties[s];
  }

  /*
   * Extension builtin classes have custom creation and destruction
   * routines.
   */
  BuiltinCtorFunction instanceCtor() const { return m_instanceCtor; }
  BuiltinDtorFunction instanceDtor() const { return m_instanceDtor; }

  /*
   * For a builtin class c_Foo, the builtinObjSize is the size of the
   * object excluding ObjectData (sizeof(c_Foo) - sizeof(ObjectData)),
   * and builtinODOffset is the offset of the ObjectData subobject in
   * c_Foo.
   */
  uint32_t builtinObjSize() const { return m_builtinObjSize; }
  int32_t builtinODOffset() const { return m_builtinODOffset; }

  void prettyPrint(std::ostream& out) const;

  NamedEntity* namedEntity() const { return m_namedEntity; }

  static const StringData* manglePropName(const StringData* className,
                                          const StringData* propName,
                                          Attr              attrs);

private:
  typedef IndexedStringMap<Func*,false,Slot> MethodMap;
  typedef IndexedStringMap<Prop,true,Slot> PropMap;
  typedef IndexedStringMap<Const,true,Slot> ConstMap;

private:
  Unit* m_unit;
  NamedEntity* m_namedEntity;
  int m_line1;
  int m_line2;
  Offset m_offset;
  Id m_id;
  uint32_t m_builtinObjSize{0};
  int32_t m_builtinODOffset{0};
  Attr m_attrs;
  Hoistable m_hoistable;
  const StringData* m_name;
  const StringData* m_parent;
  const StringData* m_docComment;
  BuiltinCtorFunction m_instanceCtor = nullptr;
  InterfaceVec m_interfaces;
  UsedTraitVec m_usedTraits;
  TraitRequirementsVec m_traitRequirements;
  TraitPrecRuleVec m_traitPrecRules;
  TraitAliasRuleVec m_traitAliasRules;
  UserAttributeMap m_userAttributes;
  MethodMap m_methods;
  PropMap m_properties;
  ConstMap m_constants;
  BuiltinDtorFunction m_instanceDtor = nullptr;
};

typedef AtomicSmartPtr<PreClass> PreClassPtr;
typedef AtomicSmartPtr<Class> ClassPtr;

/*
 * Class represents the full definition of a user class in a given
 * request context.
 *
 * See PreClass for more on the distinction.
 */
struct Class : AtomicCountable {
  enum class Avail {
    False,
    True,
    Fail
  };

  struct Prop {
    // m_name is "" for inaccessible properties (i.e. private properties
    // declared by parents).
    const StringData* m_name;
    const StringData* m_mangledName;
    const StringData* m_originalMangledName;
    Class* m_class; // First parent class that declares this property.
    Attr m_attrs;
    const StringData* m_typeConstraint;

    /*
     * Set when the frontend can infer a particular type for a
     * declared property.  When this is not KindOfInvalid, the type
     * here is actually m_hphpcType OR KindOfNull, but we know
     * KindOfUninit is not possible.
     */
    DataType m_hphpcType;

    const StringData* m_docComment;
  };

  struct SProp {
    const StringData* m_name;
    Attr m_attrs;
    const StringData* m_typeConstraint;
    const StringData* m_docComment;
    Class* m_class; // Most derived class that declared this property.
    TypedValue m_val; // Used if (m_class == this).
  };

  struct Const {
    Class* m_class; // Most derived class that declared this constant.
    const StringData* m_name;
    TypedValue m_val;
    const StringData* m_phpCode;
    const StringData* m_typeConstraint;
    const String& nameRef() const { return *(String*)&m_name; }
  };

  class PropInitVec {
   public:
    PropInitVec();
    const PropInitVec& operator=(const PropInitVec&);
    ~PropInitVec();
    static PropInitVec* allocInRequestArena(const PropInitVec& src);
    static size_t dataOff() { return offsetof(PropInitVec, m_data); }

    typedef TypedValueAux* iterator;
    iterator begin() { return m_data; }
    iterator end() { return m_data + m_size; }
    TypedValueAux& operator[](size_t i) {
      assert(i < m_size); return m_data[i];
    }
    const TypedValueAux& operator[](size_t i) const {
      assert(i < m_size); return m_data[i];
    }
    void push_back(const TypedValue& v);
    size_t size() const { return m_size; }

   private:
    PropInitVec(const PropInitVec&);
    TypedValueAux* m_data;
    unsigned m_size;
    bool m_smart;
  };

  typedef std::vector<const Func*> InitVec;
  typedef std::vector<std::pair<const StringData*, const StringData*> >
          TraitAliasVec;
  typedef IndexedStringMap<Class*,true,int> InterfaceMap;
  typedef IndexedStringMap<Func*,false,Slot> MethodMap;

  /* If set, runs during setMethods() */
  static void (*MethodCreateHook)(Class* cls, MethodMap::Builder& builder);

  /*
   * Allocate a new Class object.
   *
   * Eventually deallocated using atomicRelease(), but can go through
   * some phase changes before that (see destroy().)
   */
  static Class* newClass(PreClass* preClass, Class* parent);

  /*
   * destroy() is called when a Class becomes unreachable. This may happen
   * before its refCount hits zero, because it is referred to by
   * its NamedEntity, any derived classes, and (for interfaces) and Class
   * that implents it, and (for traits) any class that uses it.
   * Such referring classes must also be logically dead at the time
   * destroy is called, but we dont have back pointers to find them,
   * so instead we leave the Class in a zombie state. When we try to
   * instantiate one of its referrers, we will notice that it depends
   * on a zombie, and destroy *that*, releasing its refernce to this
   * Class.
   */
  void destroy();

  /*
   * atomicRelease() is called when the (atomic) refCount hits zero.
   * The class is completely dead at this point, and its memory is
   * freed immediately.
   */
  void atomicRelease();

  /*
   * releaseRefs() is called when a Class is put into the zombie state,
   * to free any references to child classes, interfaces and traits
   * Its safe to call multiple times, so is also called from the destructor
   * (in case we bypassed the zombie state).
   */
  void releaseRefs();

  /*
   * isZombie() returns true if this class has been logically destroyed,
   * but needed to be preserved due to outstanding references.
   */
  bool isZombie() const { return !m_cachedClass.bound(); }

  Avail avail(Class *&parent, bool tryAutoload = false) const;

  /*
   * classof() determines if this represents a non-strict subtype of
   * cls. Returns uint64_t instead of bool because it's called directly from
   * the TC.
   */
  uint64_t classof(const Class* cls) const {
    /*
      If cls is an interface, we can simply check to see if cls is in
      this->m_interfaces.  Otherwise, if this is not an interface, the classVec
      check will determine whether its an instance of cls (including the case
      where this and cls are the same trait).  Otherwise, this is an interface,
      and cls is not, so we need to return false. But the classVec check can
      never return true in that case (cls's classVec contains only
      non-interfaces, while this->classVec is either empty, or contains
      interfaces).
    */
    if (UNLIKELY(cls->attrs() & AttrInterface)) {
      return this == cls ||
        m_interfaces.lookupDefault(cls->m_preClass->name(), nullptr) == cls;
    }
    if (m_classVecLen >= cls->m_classVecLen) {
      return (m_classVec[cls->m_classVecLen-1] == cls);
    }
    return false;
  }
  bool ifaceofDirect(const StringData* name) const {
    return m_interfaces.lookupDefault(name, nullptr) != nullptr;
  }

  /*
   * Assuming this and cls are both regular classes (not interfaces or traits),
   * return their lowest common ancestor, or nullptr if they're unrelated.
   */
  const Class* commonAncestor(const Class* cls) const;

  const StringData* name() const {
    return m_preClass->name();
  }
  Class* parent() const {
    return m_parent.get();
  }
  const String& nameRef() const {
    return m_preClass->nameRef();
  }
  const String& parentRef() const {
    return m_preClass->parentRef();
  }

  Func* const* methods() const { return m_methods.accessList(); }
  size_t numMethods() const { return m_methods.size(); }
  typedef IterRange<Func*const*> MethodRange;
  MethodRange methodRange() const {
    return MethodRange(methods(), methods() + numMethods());
  }
  typedef IterRange<Func**> MutableMethodRange;
  MutableMethodRange mutableMethodRange() {
    return MutableMethodRange(m_methods.mutableAccessList(),
                              m_methods.mutableAccessList() + m_methods.size());
  }
  const SProp* staticProperties() const
    { return m_staticProperties.accessList(); }
  size_t numStaticProperties() const { return m_staticProperties.size(); }
  const Prop* declProperties() const { return m_declProperties.accessList(); }
  size_t numDeclProperties() const { return m_declProperties.size(); }
  uint32_t declPropNumAccessible() const { return m_declPropNumAccessible; }
  const Const* constants() const { return m_constants.accessList(); }
  size_t numConstants() const { return m_constants.size(); }
  Attr attrs() const {
    assert(Attr(m_attrCopy) == m_preClass->attrs());
    return Attr(m_attrCopy);
  }
  bool verifyPersistent() const;
  const Func* getCtor() const { return m_ctor; }
  const Func* getDeclaredCtor() const;
  const Func* getDtor() const { return m_dtor; }
  const Func* getToString() const { return m_toString; }
  const PreClass* preClass() const { return m_preClass.get(); }
  const ClassInfo* clsInfo() const { return m_clsInfo; }

  // We use the TypedValue::_count field to indicate whether a property
  // requires "deep" initialization (0 = no, 1 = yes)
  const PropInitVec* getPropData() const;

  bool hasDeepInitProps() const { return m_hasDeepInitProps; }
  bool needInitialization() const { return m_needInitialization; }
  bool hasInitMethods() const { return m_hasInitMethods; }
  bool callsCustomInstanceInit() const { return m_callsCustomInstanceInit; }
  const InterfaceMap& allInterfaces() const { return m_interfaces; }
  const std::vector<ClassPtr>& usedTraits() const {
    return m_usedTraits;
  }
  const TraitAliasVec& traitAliases() const { return m_traitAliases; }
  const InitVec& pinitVec() const { return m_pinitVec; }
  const PropInitVec& declPropInit() const { return m_declPropInit; }

  // ObjectData attributes, to be set during instance initialization.
  int getODAttrs() const { return m_ODAttrs; }

  /*
   * Custom initialization and destruction routines for builtin
   * classes.
   */
  BuiltinCtorFunction instanceCtor() const { return m_instanceCtor; }
  BuiltinDtorFunction instanceDtor() const { return m_instanceDtor; }

  /*
   * This value is the pointer adjustment from an ObjectData* to get
   * to the property vector, for a builtin class.  I.e., it's the
   * number of bytes of subclass data following the ObjectData
   * subobject.
   */
  int32_t builtinODTailSize() const { return m_builtinODTailSize; }

  bool isCppSerializable() const;
  bool isCollectionClass() const;

  // Interfaces this class declares in its "implements" clause.
  boost::iterator_range<const ClassPtr*> declInterfaces() const {
    return boost::make_iterator_range(
      m_declInterfaces.get(),
      m_declInterfaces.get() + m_numDeclInterfaces
    );
  }

  // These two fields are used by getClassInfo to locate all trait
  // methods present in methods(): indices [traitsBeginIdx, traitsEndIdx)
  Slot traitsBeginIdx() const { return m_traitsBeginIdx; }
  Slot traitsEndIdx() const   { return m_traitsEndIdx; }

  Func* lookupMethod(const StringData* methName) const {
    return m_methods.lookupDefault(methName, 0);
  }

  bool isPersistent() const { return m_attrCopy & AttrPersistent; }
  bool isBuiltin() const { return attrs() & AttrBuiltin; }

  // Finds the base class defining the given method (NULL if none).
  // Note: for methods imported via traits, the base class is the one that
  // uses/imports the trait.
  Class* findMethodBaseClass(const StringData* methName);

  // Returns true iff this class declared the given method.
  // For trait methods, the class declaring them is the one that uses/imports
  // the trait.
  bool declaredMethod(const Func* method);

  bool hasConstant(const StringData* clsCnsName) const {
    return m_constants.contains(clsCnsName);
  }

  /*
   * Look up the actual value of a class constant.  Performs dynamic
   * initialization if necessary.
   *
   * The returned Cell is guaranteed not to hold a reference counted
   * object (may however be KindOfString for a static string).
   * Returns a Cell containing KindOfUninit if this class has no
   * constant with the given name.
   */
  Cell clsCnsGet(const StringData* clsCnsName) const;

  /*
   * Look up a class constant's TypedValue if it doesn't require
   * dynamic initialization.
   *
   * The TypedValue represents the constant's value iff it is a
   * scalar, otherwise it has m_type set to KindOfUninit.  (Non-scalar
   * class constants need to run 86cinit code to determine their value
   * at runtime.)
   *
   * Returns nullptr if this class has no constant of the given name.
   */
  Cell* cnsNameToTV(const StringData* name, Slot& slot) const;

  /*
   * Provide the current runtime type of this class constant.  This
   * has predictive value for the translator.
   */
  DataType clsCnsType(const StringData* clsCnsName) const;

  /*
   * Tracing interface.  (Returns the set of static properties for the
   * Tracer.)
   */
  void getChildren(std::vector<TypedValue*>& out);

  void initialize() const;
  void initPropHandle() const;
  RDS::Handle classHandle() const { return m_cachedClass.handle(); }
  void setClassHandle(RDS::Link<Class*> link) const {
    assert(!m_cachedClass.bound());
    m_cachedClass = link;
  }
  RDS::Handle propHandle() const { return m_propDataCache.handle(); }
  void initSPropHandle() const;
  RDS::Handle sPropHandle() const { return m_propSDataCache.handle(); }
  void initProps() const;
  TypedValue* initSProps() const;
  Class* getCached() const;
  void setCached();

  void setInstanceBits();
  void setInstanceBitsAndParents();

  // Returns kInvalidSlot if we can't find this property.
  Slot lookupDeclProp(const StringData* propName) const {
    return m_declProperties.findIndex(propName);
  }

  Slot getDeclPropIndex(Class* ctx, const StringData* key,
                        bool& accessible) const;

  TypedValue* getSProp(Class* ctx, const StringData* sPropName,
                       bool& visible, bool& accessible) const;

  static bool IsPropAccessible(const Prop& prop, Class* ctx);

  // Returns kInvalidSlot if we can't find this static property.
  Slot lookupSProp(const StringData* sPropName) const {
    return m_staticProperties.findIndex(sPropName);
  }

  TypedValue getStaticPropInitVal(const SProp& prop);

  void getClassInfo(ClassInfoVM* ci);

  size_t declPropOffset(Slot index) const;

  DataType declPropHphpcType(Slot index) const {
    auto& prop = m_declProperties[index];
    return prop.m_hphpcType;
  }

  unsigned classVecLen() const {
    return m_classVecLen;
  }
  const Class* const* classVec() const { return m_classVec; }
  static size_t preClassOff() { return offsetof(Class, m_preClass); }
  static size_t classVecOff() { return offsetof(Class, m_classVec); }
  static size_t classVecLenOff() { return offsetof(Class, m_classVecLen); }
  static Offset getMethodsOffset() { return offsetof(Class, m_methods); }
  static ptrdiff_t invokeFuncOff() { return offsetof(Class, m_invoke); }
  static size_t instanceBitsOff() { return offsetof(Class, m_instanceBits); }

  static hphp_hash_map<const StringData*, const HhbcExtClassInfo*,
                       string_data_hash, string_data_isame> s_extClassHash;

  /**
   * Used by the ext_zend_compat layer.
   * Identical to getSProp but the output is boxed.
   */
  RefData* zGetSProp(Class* ctx, const StringData* sPropName,
                     bool& visible, bool& accessible) const;

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

    Class* m_trait;
    Func*  m_method;
    Attr   m_modifiers;
  };
  typedef std::list<TraitMethod> TraitMethodList;
  typedef hphp_hash_map<const StringData*, TraitMethodList, string_data_hash,
                        string_data_isame> MethodToTraitListMap;

private:
  Class(PreClass* preClass, Class* parent, unsigned classVecLen);
  ~Class();

private:
  void initialize(TypedValue*& sPropData) const;
  PropInitVec* initPropsImpl() const;
  TypedValue* initSPropsImpl() const;
  void setPropData(PropInitVec* propData) const;
  void setSPropData(TypedValue* sPropData) const;
  TypedValue* getSPropData() const;

  void importTraitMethod(const TraitMethod&  traitMethod,
                         const StringData*   methName,
                         MethodMap::Builder& curMethodMap);
  Class* findSingleTraitWithMethod(const StringData* methName);
  void setImportTraitMethodModifiers(TraitMethodList& methList,
                                     Class*           traitCls,
                                     Attr             modifiers);
  void importTraitMethods(MethodMap::Builder& curMethodMap);
  void addTraitPropInitializers(bool staticProps);
  void applyTraitRules(MethodToTraitListMap& importMethToTraitMap);
  void applyTraitPrecRule(const PreClass::TraitPrecRule& rule,
                          MethodToTraitListMap& importMethToTraitMap);
  void applyTraitAliasRule(const PreClass::TraitAliasRule& rule,
                           MethodToTraitListMap& importMethToTraitMap);
  void importTraitProps(PropMap::Builder& curPropMap,
                        SPropMap::Builder& curSPropMap);
  void importTraitInstanceProp(Class*      trait,
                               Prop&       traitProp,
                               TypedValue& traitPropVal,
                               PropMap::Builder& curPropMap);
  void importTraitStaticProp(Class*   trait,
                             SProp&   traitProp,
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
  void setUsedTraits();
  void checkTraitConstraints() const;
  void checkTraitConstraintsRec(const std::vector<ClassPtr>& usedTraits,
                                const StringData* recName) const;
  template<bool setParents> void setInstanceBitsImpl();
  void addInterfacesFromUsedTraits(InterfaceMap::Builder& builder) const;

private:
  PreClassPtr m_preClass;
  ClassPtr m_parent;
  std::unique_ptr<ClassPtr[]> m_declInterfaces;
  size_t m_numDeclInterfaces;
  InterfaceMap m_interfaces;

  std::vector<ClassPtr> m_usedTraits;
  TraitAliasVec m_traitAliases;

  MethodMap m_methods;

  Slot m_traitsBeginIdx;
  Slot m_traitsEndIdx;
  Func* m_ctor;
  Func* m_dtor;
  Func* m_toString;
  Func* m_invoke;    // __invoke, iff non-static (or closure)

  // Vector of 86pinit() methods that need to be called to complete instance
  // property initialization, and a pointer to a 86sinit() method that needs to
  // be called to complete static property initialization (or NULL).  Such
  // initialization is triggered only once, the first time one of the following
  // happens:
  //
  // + An instance of this class is created.
  // + A static property of this class is accessed.
  InitVec m_pinitVec;
  InitVec m_sinitVec;

  const ClassInfo* m_clsInfo;

  unsigned m_needInitialization : 1;      // requires initialization,
                                          // due to [ps]init or simply
                                          // having static members
  unsigned m_hasInitMethods : 1;          // any __[ps]init() methods?
  unsigned m_callsCustomInstanceInit : 1; // should we always call __init__
                                          // on new instances?
  unsigned m_hasDeepInitProps : 1;
  unsigned m_attrCopy : 28;               // cache of m_preClass->attrs().
  int32_t m_ODAttrs;

  uint32_t m_builtinODTailSize{0};
  int32_t m_declPropNumAccessible;
  unsigned m_classVecLen;
  mutable RDS::Link<Class*> m_cachedClass; // can this be const Class*?
  mutable RDS::Link<PropInitVec*> m_propDataCache;
  mutable RDS::Link<TypedValue*> m_propSDataCache;
  mutable RDS::Link<Array> m_nonScalarConstantCache;
  BuiltinCtorFunction m_instanceCtor{nullptr};
  BuiltinDtorFunction m_instanceDtor{nullptr};
  ConstMap m_constants;

  /*
   * Each ObjectData is created with enough trailing space to directly store
   * the vector of declared properties. To look up a property by name and
   * determine whether it is declared, use m_declPropMap. If the declared
   * property index is already known (as may be the case when executing via
   * the TC), property metadata in m_declPropInfo can be directly accessed.
   *
   * m_declPropInit is indexed by the Slot values from m_declProperties, and
   * contains initialization information.
   */
  PropMap m_declProperties;
  PropInitVec m_declPropInit;
  SPropMap m_staticProperties;

public:
  Class* m_nextClass; // used by Unit

private:
  // Bitmap of parent classes and implemented interfaces. Each bit
  // corresponds to a commonly used class name, determined during the
  // profiling warmup requests.
  InstanceBits::BitSet m_instanceBits;
  // Vector of Class pointers that encodes the inheritance hierarchy,
  // including this Class as the last element.
  Class* m_classVec[1]; // Dynamically sized; must come last.
};

inline bool isTrait(const Class* cls) {
  return cls->attrs() & AttrTrait;
}
inline bool isInterface(const Class* cls) {
  return cls->attrs() & AttrInterface;
}
inline bool isNormalClass(const Class* cls ) {
  return !(cls->attrs() & (AttrTrait | AttrInterface));
}

enum class ClassKind { Class, Interface, Trait };

/*
 * Returns whether a class is persistent *and* has a persistent RDS
 * handle.  You probably mean this instead of cls->isPersistent(),
 * which only checks the attributes.
 *
 * A persistent class can end up with a non-persistent RDS handle if
 * we had to allocate the handle before we loaded the class.
 */
inline bool classHasPersistentRDS(const Class* cls) {
  return (RuntimeOption::RepoAuthoritative &&
          cls &&
          RDS::isPersistentHandle(cls->classHandle()));
}

} // HPHP

#endif
