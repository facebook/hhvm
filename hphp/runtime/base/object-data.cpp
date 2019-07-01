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

#include "hphp/runtime/base/object-data.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/object-iterator.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/variable-serializer.h"

#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/ext/simplexml/ext_simplexml.h"
#include "hphp/runtime/ext/datetime/ext_datetime.h"
#include "hphp/runtime/ext/std/ext_std_closure.h"

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/memo-cache.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/native-prop-handler.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"

#include "hphp/system/systemlib.h"

#include <folly/Hash.h>
#include <folly/ScopeGuard.h>

#include <vector>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(runtime);

//////////////////////////////////////////////////////////////////////

namespace {

const StaticString
  s_call("__call"),
  s_clone("__clone");

ALWAYS_INLINE
void invoke_destructor(ObjectData* obj, const Func* dtor) {
  try {
    // Call the destructor method
    g_context->invokeMethodV(obj, dtor, InvokeArgs{}, false);
  } catch (...) {
    // Swallow any exceptions that escape the __destruct method
    handle_destructor_exception();
  }
}

ALWAYS_INLINE
void verifyTypeHint(const Class* thisCls,
                    const Class::Prop* prop,
                    tv_rval val) {
  assertx(cellIsPlausible(*val));
  assertx(type(val) != KindOfUninit);
  if (RuntimeOption::EvalCheckPropTypeHints <= 0) return;
  if (!prop || !prop->typeConstraint.isCheckable()) return;
  prop->typeConstraint.verifyProperty(val, thisCls, prop->cls, prop->name);
}

ALWAYS_INLINE
void unsetTypeHint(const Class::Prop* prop) {
  if (RuntimeOption::EvalCheckPropTypeHints <= 0) return;
  if (!prop || prop->typeConstraint.isMixedResolved()) return;
  raise_property_typehint_unset_error(
    prop->cls,
    prop->name,
    prop->typeConstraint.isSoft()
  );
}

}

//////////////////////////////////////////////////////////////////////

// Check that the given property's type matches its type-hint.
bool ObjectData::assertTypeHint(tv_rval prop, Slot propIdx) const {
  assertx(tvIsPlausible(*prop));
  assertx(propIdx < m_cls->numDeclProperties());
  auto const& propDecl = m_cls->declProperties()[propIdx];

  // AttrLateInitSoft properties can be potentially anything due to default
  // values, so don't assume anything.
  if (propDecl.attrs & AttrLateInitSoft) return true;

  if (debug && RuntimeOption::RepoAuthoritative) {
    // The fact that uninitialized LateInit props are uninint isn't
    // reflected in the repo-auth-type.
    if (prop.type() != KindOfUninit || !(propDecl.attrs & AttrLateInit)) {
      always_assert(tvMatchesRepoAuthType(*prop, propDecl.repoAuthType));
    }
  }

  // If we're not hard enforcing, then the prop might contain anything.
  if (RuntimeOption::EvalCheckPropTypeHints <= 2) return true;
  if (!propDecl.typeConstraint.isCheckable() ||
      propDecl.typeConstraint.isSoft()) return true;
  if (prop.type() == KindOfNull && !(propDecl.attrs & AttrNoImplicitNullable)) {
    return true;
  }
  if (prop.type() == KindOfUninit && (propDecl.attrs & AttrLateInit)) {
    return true;
  }
  if (prop.type() == KindOfRef || prop.type() == KindOfUninit) {
    return propDecl.typeConstraint.maybeMixed();
  }
  return propDecl.typeConstraint.assertCheck(prop);
}

//////////////////////////////////////////////////////////////////////

NEVER_INLINE
static void freeDynPropArray(ObjectData* inst) {
  auto& table = g_context->dynPropTable;
  auto it = table.find(inst);
  assertx(it != end(table));
  assertx(it->second.arr().isPHPArray());
  it->second.destroy();
  table.erase(it);
}

NEVER_INLINE
void ObjectData::slowDestroyCases() {
  assertx(slowDestroyCheck());

  if (getAttribute(UsedMemoCache)) {
    assertx(m_cls->hasMemoSlots());
    auto const nSlots = m_cls->numMemoSlots();
    for (Slot i = 0; i < nSlots; ++i) {
      auto slot = memoSlot(i);
      if (slot->isCache()) {
        if (auto cache = slot->getCache()) req::destroy_raw(cache);
      } else {
        tvDecRefGen(*slot->getValue());
      }
    }
  }

  if (UNLIKELY(getAttribute(HasDynPropArr))) freeDynPropArray(this);
  if (UNLIKELY(getAttribute(IsWeakRefed))) {
    WeakRefData::invalidateWeakRef((uintptr_t)this);
  }
}

// Single check for a couple different unlikely actions during destruction.
inline bool ObjectData::slowDestroyCheck() const {
  return m_aux16 & (HasDynPropArr | IsWeakRefed | UsedMemoCache);
}

NEVER_INLINE
void ObjectData::release(ObjectData* obj, const Class* cls) noexcept {
  assertx(obj->kindIsValid());
  assertx(!obj->hasInstanceDtor());
  assertx(!obj->hasNativeData());
  assertx(obj->getVMClass() == cls);
  assertx(cls->releaseFunc() == &ObjectData::release);

  // Note: cleanups done in this function are only run for classes without an
  // instanceDtor. Some of these cleanups are duplicated in ~ObjectData, and
  // your instanceDtor may call that to have them run; if you choose not to run
  // ~ObjectData from your instanceDtor you MUST do some of them manually
  // (e.g. invalidate WeakRefs). Some cleanups (e.g. clearing memo caches) are
  // not done from ~ObjectData because it is assumed they're not needed for
  // builtin classes (and in the case of memo caches, since the clearing needs
  // to be done differently when there is native data).
  // Finally, cleanups such as invalidating WeakRefs that have to be done for
  // correctness MUST also be done in Collector::sweep, since none of the code
  // in this function or the instanceDtor will be run when the object is
  // collected by GC.

  // `obj' is being torn down now---be careful about where/how you dereference
  // it from here on.

  auto const nProps = size_t{cls->numDeclProperties()};
  auto prop = reinterpret_cast<TypedValue*>(obj + 1);
  auto const stop = prop + nProps;
  for (; prop != stop; ++prop) {
    tvDecRefGen(prop);
  }

  if (UNLIKELY(obj->slowDestroyCheck())) obj->slowDestroyCases();

  auto const size =
    reinterpret_cast<char*>(stop) - reinterpret_cast<char*>(obj);
  assertx(size == sizeForNProps(nProps));

  if (cls->hasMemoSlots()) {
    auto const memoSize = objOffFromMemoNode(cls);
    assertx(
      reinterpret_cast<const MemoNode*>(
        reinterpret_cast<const char*>(obj) - memoSize
      )->objOff() == memoSize
    );
    tl_heap->objFree(reinterpret_cast<char*>(obj) - memoSize,
                     size + memoSize);
  } else {
    tl_heap->objFree(obj, size);
  }
  AARCH64_WALKABLE_FRAME();
}

///////////////////////////////////////////////////////////////////////////////
// class info

StrNR ObjectData::getClassName() const {
  return m_cls->preClass()->nameStr();
}

bool ObjectData::instanceof(const String& s) const {
  assertx(kindIsValid());
  auto const cls = Unit::lookupClass(s.get());
  return cls && instanceof(cls);
}

bool ObjectData::toBooleanImpl() const noexcept {
  // Note: if you add more cases here, hhbbc/class-util.cpp also needs
  // to be changed.
  if (isCollection()) {
    if (RuntimeOption::EvalNoticeOnCollectionToBool) {
      raise_notice(
        "%s to boolean cast",
        collections::typeToString((CollectionType)m_kind)->data()
      );
    }
    return collections::toBool(this);
  }

  if (instanceof(SimpleXMLElement_classof())) {
    // SimpleXMLElement is the only non-collection class that has custom bool
    // casting.
    if (RuntimeOption::EvalNoticeOnSimpleXMLBehavior) {
      raise_notice("SimpleXMLElement to boolean cast");
    }
    return SimpleXMLElement_objectCast(this, KindOfBoolean).toBoolean();
  }

  always_assert(false);
  return false;
}

int64_t ObjectData::toInt64Impl() const noexcept {
  // SimpleXMLElement is the only class that has proper custom int casting.
  assertx(instanceof(SimpleXMLElement_classof()));
  if (RuntimeOption::EvalNoticeOnSimpleXMLBehavior) {
    raise_notice("SimpleXMLElement to integer cast");
  }
  return SimpleXMLElement_objectCast(this, KindOfInt64).toInt64();
}

double ObjectData::toDoubleImpl() const noexcept {
  // SimpleXMLElement is the only class that has custom double casting.
  assertx(instanceof(SimpleXMLElement_classof()));
  if (RuntimeOption::EvalNoticeOnSimpleXMLBehavior) {
    raise_notice("SimpleXMLElement to double cast");
  }
  return SimpleXMLElement_objectCast(this, KindOfDouble).toDouble();
}

///////////////////////////////////////////////////////////////////////////////
// instance methods and properties

const StaticString s_getIterator("getIterator");

Object ObjectData::iterableObject(bool& isIterable,
                                  bool mayImplementIterator /* = true */) {
  assertx(mayImplementIterator || !isIterator());
  if (mayImplementIterator && isIterator()) {
    isIterable = true;
    return Object(this);
  }
  Object obj(this);
  while (obj->instanceof(SystemLib::s_IteratorAggregateClass)) {
    auto iterator = obj->o_invoke_few_args(s_getIterator, 0);
    if (!iterator.isObject()) break;
    auto o = iterator.getObjectData();
    if (o->isIterator()) {
      isIterable = true;
      return Object{o};
    }
    obj.reset(o);
  }
  if (!isIterator() && obj->instanceof(SimpleXMLElement_classof())) {
    if (RuntimeOption::EvalNoticeOnSimpleXMLBehavior) {
      raise_notice("SimpleXMLElement used as iterator");
    }
    isIterable = true;
    return create_object(
      s_SimpleXMLElementIterator,
      make_vec_array(obj)
    );
  }
  isIterable = false;
  return obj;
}

Array& ObjectData::dynPropArray() const {
  assertx(getAttribute(HasDynPropArr));
  assertx(g_context->dynPropTable.count(this));
  assertx(g_context->dynPropTable[this].arr().isPHPArray());
  return g_context->dynPropTable[this].arr();
}

void ObjectData::setDynProps(const Array& newArr) {
  // don't expose the ref returned by setDynPropArr
  (void)setDynPropArray(newArr);
}

void ObjectData::reserveDynProps(int numDynamic) {
  // don't expose the ref returned by reserveProperties()
  (void)reserveProperties(numDynamic);
}

Array& ObjectData::reserveProperties(int numDynamic /* = 2 */) {
  if (getAttribute(HasDynPropArr)) {
    return dynPropArray();
  }

  return setDynPropArray(
      Array::attach(MixedArray::MakeReserveMixed(numDynamic))
  );
}

Array& ObjectData::setDynPropArray(const Array& newArr) {
  assertx(!g_context->dynPropTable.count(this));
  assertx(!getAttribute(HasDynPropArr));
  assertx(newArr.isPHPArray());

  if (m_cls->forbidsDynamicProps()) {
    throw_object_forbids_dynamic_props(getClassName().data());
  }
  if (RuntimeOption::EvalNoticeOnCreateDynamicProp) {
    IterateKV(newArr.get(), [&] (Cell k, TypedValue v) {
      auto const key = tvCastToString(k);
      raiseCreateDynamicProp(key.get());
    });
  }

  // newArr can have refcount 2 or higher
  auto& arr = g_context->dynPropTable[this].arr();
  assertx(arr.isPHPArray());
  arr = newArr;
  setAttribute(HasDynPropArr);
  return arr;
}

tv_lval ObjectData::makeDynProp(const StringData* key) {
  if (RuntimeOption::EvalNoticeOnCreateDynamicProp) {
    raiseCreateDynamicProp(key);
  }
  SuppressHACFalseyPromoteNotices shacn;
  return reserveProperties().lvalAt(StrNR(key), AccessFlags::Key);
}

void ObjectData::setDynProp(const StringData* key, Cell val) {
  if (RuntimeOption::EvalNoticeOnCreateDynamicProp) {
    raiseCreateDynamicProp(key);
  }
  reserveProperties().set(StrNR(key), val, true);
}

Variant ObjectData::o_get(const String& propName, bool error /* = true */,
                          const String& context /*= null_string*/) {
  assertx(kindIsValid());

  // This is not (just) a check for empty string; property names that start
  // with null are intentionally being rejected here.
  if (UNLIKELY(!*propName.data())) {
    throw_invalid_property_name(propName);
  }

  Class* ctx = nullptr;
  if (!context.empty()) {
    ctx = Unit::lookupClass(context.get());
  }

  // Can't use propImpl here because if the property is not accessible and
  // there is no magic __get, propImpl will raise_error("Cannot access ...",
  // but o_get will only (maybe) raise_notice("Undefined property ..." :-(

  auto const lookup = getPropImpl<false, true, true>(ctx, propName.get());
  if (lookup.val && lookup.accessible) {
    if (lookup.val.type() != KindOfUninit) {
      return Variant::wrap(tvToCell(lookup.val.tv()));
    } else if (lookup.prop && (lookup.prop->attrs & AttrLateInit)) {
      if (lookup.prop->attrs & AttrLateInitSoft) {
        raise_soft_late_init_prop(lookup.prop->cls, propName.get(), false);
        tvDup(
          *g_context->getSoftLateInitDefault().asTypedValue(),
          lookup.val
        );
        return Variant::wrap(tvToCell(lookup.val.tv()));
      }
      if (error) throw_late_init_prop(lookup.prop->cls, propName.get(), false);
      return uninit_null();
    }
  }

  if (m_cls->rtAttribute(Class::UseGet)) {
    if (auto r = invokeGet(propName.get())) {
      return std::move(tvAsVariant(&r.val));
    }
  }

  if (error) {
    raise_notice("Undefined property: %s::$%s", getClassName().data(),
                 propName.data());
  }

  return uninit_null();
}

void ObjectData::o_set(const String& propName, const Variant& v,
                       const String& context /* = null_string */) {
  assertx(kindIsValid());

  // This is not (just) a check for empty string; property names that start
  // with null are intentionally being rejected here.
  if (UNLIKELY(!*propName.data())) {
    throw_invalid_property_name(propName);
  }

  Class* ctx = nullptr;
  if (!context.empty()) {
    ctx = Unit::lookupClass(context.get());
  }

  // Can't use setProp here because if the property is not accessible and
  // there is no magic __set, setProp will raise_error("Cannot access ...",
  // but o_set will skip writing and return normally. Also, if we try to
  // invoke __set and fail due to recursion, setProp will fall back to writing
  // the property normally, but o_set will just skip writing and return :-(

  auto const useSet = m_cls->rtAttribute(Class::UseSet);
  auto const lookup = getPropImpl<true, false, true>(ctx, propName.get());
  auto prop = lookup.val;
  if (prop && lookup.accessible) {
    if (!useSet || type(prop) != KindOfUninit ||
        (lookup.prop && (lookup.prop->attrs & AttrLateInit))) {
      if (UNLIKELY(lookup.isConst) && !isBeingConstructed()) {
        throwMutateConstProp(lookup.slot);
      }
      auto const val = tvToInitCell(*v.asTypedValue());
      verifyTypeHint(m_cls, lookup.prop, &val);
      tvSet(val, prop);
      return;
    }
  }

  if (useSet) {
    invokeSet(propName.get(), *v.toCell());
  } else if (!prop) {
    setDynProp(propName.get(), tvToInitCell(*v.asTypedValue()));
  }
}

void ObjectData::o_setArray(const Array& properties) {
  for (ArrayIter iter(properties); iter; ++iter) {
    String k = iter.first().toString();
    Class* ctx = nullptr;
    // If the key begins with a NUL, it's a private or protected property. Read
    // the class name from between the two NUL bytes.
    //
    // Note: if you change this, you need to change similar logic in
    // apc-object.
    if (!k.empty() && k[0] == '\0') {
      int subLen = k.find('\0', 1) + 1;
      String cls = k.substr(1, subLen - 2);
      if (cls.size() == 1 && cls[0] == '*') {
        // Protected.
        ctx = m_cls;
      } else {
        // Private.
        ctx = Unit::lookupClass(cls.get());
        if (!ctx) continue;
      }
      k = k.substr(subLen);
    }

    setProp(ctx, k.get(), tvAssertCell(iter.secondRval().tv()));
  }
}

void ObjectData::o_getArray(Array& props,
                            bool pubOnly /* = false */,
                            bool ignoreLateInit /* = false */) const {
  assertx(kindIsValid());

  // Fast path for classes with no declared properties
  if (!m_cls->numDeclProperties() && getAttribute(HasDynPropArr)) {
    props = dynPropArray();
    if (RuntimeOption::EvalNoticeOnReadDynamicProp) {
      IterateKV(props.get(), [&](Cell k, TypedValue) {
        auto const key = tvCastToString(k);
        raiseReadDynamicProp(key.get());
      });
    }
    return;
  }

  auto cls = m_cls;
  if (cls->hasReifiedGenerics()) {
    auto const slot = cls->lookupReifiedInitProp();
    assertx(slot != kInvalidSlot);
    auto const declProps = cls->declProperties();
    auto const prop = declProps[slot];
    auto val = this->propRvalAtOffset(slot);
    props.setWithRef(StrNR(prop.name).asString(), val.tv());
  }
  IteratePropToArrayOrderNoInc(
    this,
    [&](Slot slot, const Class::Prop& prop, tv_rval val) {
      assertx(assertTypeHint(val, slot));
      if (UNLIKELY(val.type() == KindOfUninit)) {
        if (!ignoreLateInit) {
          if (prop.attrs & AttrLateInitSoft) {
            raise_soft_late_init_prop(cls, prop.name, false);
            tvDup(
              *g_context->getSoftLateInitDefault().asTypedValue(),
              const_cast<ObjectData*>(this)->propLvalAtOffset(slot)
            );
            props.setWithRef(
              StrNR(prop.mangledName).asString(),
              val.tv()
            );
          } else if (prop.attrs & AttrLateInit) {
            throw_late_init_prop(cls, prop.name, false);
          }
        }
      } else if (!pubOnly || (prop.attrs & AttrPublic)) {
        // Skip all the reified properties since we already prepended the
        // current class' reified property to the list
        if (prop.name != s_86reified_prop.get()) {
          props.setWithRef(StrNR(prop.mangledName).asString(), val.tv());
        }
      }
    },
    [&](Cell key_tv, TypedValue val) {
      props.setWithRef(key_tv, val, true);
      if (RuntimeOption::EvalNoticeOnReadDynamicProp) {
        auto const key = tvCastToString(key_tv);
        raiseReadDynamicProp(key.get());
      }
    }
  );
}

// a constant for ArrayObjects AND ArrayIterators that changes the way the
// object is converted to an array
const int64_t ARRAY_OBJ_ITERATOR_STD_PROP_LIST = 1;

const StaticString s_flags("flags");

template <IntishCast IC /* = IntishCast::None */>
Array ObjectData::toArray(bool pubOnly /* = false */,
                          bool ignoreLateInit /* = false */) const {
  assertx(kindIsValid());

  // We can quickly tell if this object is a collection, which lets us avoid
  // checking for each class in turn if it's not one.
  if (isCollection()) {
    return collections::toArray<IC>(this);
  } else if (UNLIKELY(m_cls->rtAttribute(Class::CallToImpl))) {
    // If we end up with other classes that need special behavior, turn the
    // assert into an if and add cases.
    assertx(instanceof(SimpleXMLElement_classof()));
    if (RuntimeOption::EvalNoticeOnSimpleXMLBehavior) {
      raise_notice("SimpleXMLElement to array cast");
    }
    return SimpleXMLElement_objectCast(this, KindOfArray).toArray();
  } else if (UNLIKELY(instanceof(SystemLib::s_ArrayObjectClass)) ||
             UNLIKELY(instanceof(SystemLib::s_ArrayIteratorClass))) {
    auto const cls = [&]() {
      if (instanceof(SystemLib::s_ArrayObjectClass)) {
        return SystemLib::s_ArrayObjectClass;
      }
      assertx(instanceof(SystemLib::s_ArrayIteratorClass));
      return SystemLib::s_ArrayIteratorClass;
    }();

    auto const flags = getProp(cls, s_flags.get());
    assertx(flags.is_set());
    if (UNLIKELY(flags.type() == KindOfInt64 &&
                 flags.val().num == ARRAY_OBJ_ITERATOR_STD_PROP_LIST)) {
      auto ret = Array::Create();
      o_getArray(ret, true, ignoreLateInit);
      return ret;
    }

    check_recursion_throw();

    auto const storage = getProp(cls, s_storage.get());
    assertx(storage.is_set());
    return tvCastToArrayLike(storage.tv());
  } else if (UNLIKELY(instanceof(c_Closure::classof()))) {
    return make_packed_array(Object(const_cast<ObjectData*>(this)));
  } else if (UNLIKELY(instanceof(DateTimeData::getClass()))) {
    return Native::data<DateTimeData>(this)->getDebugInfo();
  } else {
    auto ret = Array::Create();
    o_getArray(ret, pubOnly, ignoreLateInit);
    return ret;
  }
}

template
Array ObjectData::toArray<IntishCast::None>(bool, bool) const;
template
Array ObjectData::toArray<IntishCast::Cast>(bool, bool) const;


namespace {

size_t getPropertyIfAccessible(ObjectData* obj,
                               const Class* ctx,
                               const StringData* key,
                               ObjectData::IterMode mode,
                               Array& properties,
                               size_t propLeft) {
  auto const prop = obj->getProp(ctx, key);
  if (prop && prop.type() != KindOfUninit) {
    --propLeft;
    if (mode == ObjectData::EraseRefs) {
      properties.set(StrNR(key), prop.tv(), true);
    } else {
      properties.setWithRef(StrNR(key), prop.tv(), true);
    }
  }
  return propLeft;
}

}

Array ObjectData::o_toIterArray(const String& context, IterMode mode) {
  if (mode == PreserveRefs && !m_cls->numDeclProperties()) {
    if (getAttribute(HasDynPropArr)) {
      auto const props = dynPropArray();
      if (RuntimeOption::EvalNoticeOnReadDynamicProp) {
        IterateKV(props.get(), [&](Cell k, TypedValue) {
          auto const key = tvCastToString(k);
          raiseReadDynamicProp(key.get());
        });
      }
      // not returning Array&; makes a copy
      return props;
    }
    return Array::Create();
  }

  size_t accessibleProps = m_cls->declPropNumAccessible();
  size_t size = accessibleProps;
  if (getAttribute(HasDynPropArr)) {
    size += dynPropArray().size();
  }
  Array retArray { Array::attach(MixedArray::MakeReserveMixed(size)) };

  Class* ctx = nullptr;
  if (!context.empty()) {
    ctx = Unit::lookupClass(context.get());
  }

  // Get all declared properties first, bottom-to-top in the inheritance
  // hierarchy, in declaration order.
  const Class* klass = m_cls;
  while (klass) {
    const PreClass::Prop* props = klass->preClass()->properties();
    const size_t numProps = klass->preClass()->numProperties();

    for (size_t i = 0; i < numProps; ++i) {
      auto key = const_cast<StringData*>(props[i].name());
      accessibleProps = getPropertyIfAccessible(
          this, ctx, key, mode, retArray, accessibleProps);
    }
    klass = klass->parent();
  }
  if (!(m_cls->attrs() & AttrNoExpandTrait) && accessibleProps > 0) {
    // we may have properties from traits
    for (auto const& prop : m_cls->declProperties()) {
      auto const key = prop.name.get();
      if (!retArray.get()->exists(key)) {
        accessibleProps = getPropertyIfAccessible(
          this, ctx, key, mode, retArray, accessibleProps);
        if (accessibleProps == 0) break;
      }
    }
  }

  // Now get dynamic properties.
  if (getAttribute(HasDynPropArr)) {
    auto& dynProps = dynPropArray();
    auto ad = dynProps.get();
    ssize_t iter = ad->iter_begin();
    auto pos_limit = ad->iter_end();
    while (iter != pos_limit) {
      ad = dynProps.get();
      auto const key = ad->nvGetKey(iter);
      iter = ad->iter_advance(iter);

      if (RuntimeOption::EvalNoticeOnReadDynamicProp) {
        auto const k = tvCastToString(key);
        raiseReadDynamicProp(k.get());
      }

      // You can get this if you cast an array to object. These
      // properties must be dynamic because you can't declare a
      // property with a non-string name.
      if (UNLIKELY(!isStringType(key.m_type))) {
        assertx(key.m_type == KindOfInt64);
        switch (mode) {
        case EraseRefs: {
          auto const val = dynProps.get()->at(key.m_data.num);
          retArray.set(key.m_data.num, val);
          break;
        }
        case PreserveRefs: {
          auto const val = dynProps.get()->at(key.m_data.num);
          retArray.setWithRef(key.m_data.num, val);
          break;
        }
        }
        continue;
      }

      auto const strKey = key.m_data.pstr;
      switch (mode) {
      case EraseRefs: {
        auto const val = dynProps.get()->at(strKey);
        retArray.set(StrNR(strKey), val, true /* isKey */);
        break;
      }
      case PreserveRefs: {
        auto const val = dynProps.get()->at(strKey);
        retArray.setWithRef(make_tv<KindOfString>(strKey),
                            val, true /* isKey */);
        break;
      }
      }
      decRefStr(strKey);
    }
  }

  return retArray;
}

static bool decode_invoke(const String& s, ObjectData* obj, bool fatal,
                          CallCtx& ctx) {
  ctx.this_ = obj;
  ctx.cls = obj->getVMClass();
  ctx.invName = nullptr;
  ctx.dynamic = true;

  ctx.func = ctx.cls->lookupMethod(s.get());
  if (ctx.func) {
    // Null out this_ for statically called methods
    if (ctx.func->isStaticInPrologue()) {
      ctx.this_ = nullptr;
    }
  } else {
    // If this_ is non-null AND we could not find a method, try
    // looking up __call in cls's method table
    ctx.func = ctx.cls->lookupMethod(s_call.get());

    if (!ctx.func) {
      // Bail if we couldn't find the method or __call
      o_invoke_failed(ctx.cls->name()->data(), s.data(), fatal);
      return false;
    }
    // We found __call! Stash the original name into invName.
    assertx(!(ctx.func->attrs() & AttrStatic));
    ctx.invName = s.get();
    ctx.invName->incRefCount();
    ctx.dynamic = false;
  }
  return true;
}

Variant ObjectData::o_invoke(const String& s, const Variant& params,
                             bool fatal /* = true */) {
  CallCtx ctx;
  if (!decode_invoke(s, this, fatal, ctx) ||
      (!isContainer(params) && !params.isNull())) {
    return Variant(Variant::NullInit());
  }
  return Variant::attach(
    g_context->invokeFunc(ctx, params)
  );
}

#define INVOKE_FEW_ARGS_IMPL3                        \
  const Variant& a0, const Variant& a1, const Variant& a2
#define INVOKE_FEW_ARGS_IMPL6                        \
  INVOKE_FEW_ARGS_IMPL3,                             \
  const Variant& a3, const Variant& a4, const Variant& a5
#define INVOKE_FEW_ARGS_IMPL10                       \
  INVOKE_FEW_ARGS_IMPL6,                             \
  const Variant& a6, const Variant& a7, const Variant& a8, const Variant& a9
#define INVOKE_FEW_ARGS_IMPL_ARGS INVOKE_FEW_ARGS(IMPL,INVOKE_FEW_ARGS_COUNT)

Variant ObjectData::o_invoke_few_args(const String& s, int count,
                                      INVOKE_FEW_ARGS_IMPL_ARGS) {

  CallCtx ctx;
  if (!decode_invoke(s, this, true, ctx)) {
    return Variant(Variant::NullInit());
  }

  TypedValue args[INVOKE_FEW_ARGS_COUNT];
  switch(count) {
    default: not_implemented();
#if INVOKE_FEW_ARGS_COUNT > 6
    case 10: tvCopy(*a9.asTypedValue(), args[9]);
    case  9: tvCopy(*a8.asTypedValue(), args[8]);
    case  8: tvCopy(*a7.asTypedValue(), args[7]);
    case  7: tvCopy(*a6.asTypedValue(), args[6]);
#endif
#if INVOKE_FEW_ARGS_COUNT > 3
    case  6: tvCopy(*a5.asTypedValue(), args[5]);
    case  5: tvCopy(*a4.asTypedValue(), args[4]);
    case  4: tvCopy(*a3.asTypedValue(), args[3]);
#endif
    case  3: tvCopy(*a2.asTypedValue(), args[2]);
    case  2: tvCopy(*a1.asTypedValue(), args[1]);
    case  1: tvCopy(*a0.asTypedValue(), args[0]);
    case  0: break;
  }

  return Variant::attach(
    g_context->invokeFuncFew(ctx, count, args)
  );
}

ObjectData* ObjectData::clone() {
  if (isCppBuiltin()) {
    assertx(!m_cls->hasMemoSlots());
    if (isCollection()) return collections::clone(this);
    if (instanceof(c_Closure::classof())) {
      return c_Closure::fromObject(this)->clone();
    }
    assertx(instanceof(c_Awaitable::classof()));
    // cloning WaitHandles is not allowed
    // invoke the instanceCtor to get the right sort of exception
    auto const ctor = m_cls->instanceCtor();
    ctor(m_cls);
    always_assert(false);
  }

  // clone prevents a leak if something throws before clone() returns
  Object clone;
  auto const nProps = m_cls->numDeclProperties();
  if (hasNativeData()) {
    assertx(m_cls->instanceCtor() == Native::nativeDataInstanceCtor);
    clone = Object::attach(
      Native::nativeDataInstanceCopyCtor(this, m_cls, nProps)
    );
    assertx(clone->hasExactlyOneRef());
    assertx(clone->hasInstanceDtor());
  } else if (m_cls->hasMemoSlots()) {
    auto const size = sizeForNProps(nProps);
    auto const objOff = objOffFromMemoNode(m_cls);
    auto mem = tl_heap->objMalloc(size + objOff);
    new (NotNull{}, mem) MemoNode(objOff);
    std::memset(
      reinterpret_cast<char*>(mem) + sizeof(MemoNode),
      0,
      objOff - sizeof(MemoNode)
    );
    auto const obj = new (NotNull{}, reinterpret_cast<char*>(mem) + objOff)
      ObjectData(m_cls, InitRaw{}, ObjectData::NoAttrs);
    clone = Object::attach(obj);
    assertx(clone->hasExactlyOneRef());
    assertx(!clone->hasInstanceDtor());
  } else {
    auto const size = sizeForNProps(nProps);
    auto const obj = new (NotNull{}, tl_heap->objMalloc(size))
      ObjectData(m_cls, InitRaw{}, ObjectData::NoAttrs);
    clone = Object::attach(obj);
    assertx(clone->hasExactlyOneRef());
    assertx(!clone->hasInstanceDtor());
  }

  auto const clonePropVec = clone->propVecForConstruct();
  for (auto i = Slot{0}; i < nProps; i++) {
    tvDupWithRef(propVec()[i], clonePropVec[i]);
    assertx(assertTypeHint(&clonePropVec[i], i));
  }

  if (UNLIKELY(getAttribute(HasDynPropArr))) {
    clone->setAttribute(HasDynPropArr);
    g_context->dynPropTable.emplace(clone.get(), dynPropArray().get());
  }
  if (m_cls->rtAttribute(Class::HasClone)) {
    assertx(!isCppBuiltin());
    auto const method = clone->m_cls->lookupMethod(s_clone.get());
    assertx(method);
    clone->unlockObject();
    SCOPE_EXIT { clone->lockObject(); };
    g_context->invokeMethodV(clone.get(), method, InvokeArgs{}, false);
  }
  return clone.detach();
}

bool ObjectData::equal(const ObjectData& other) const {
  if (this == &other) return true;
  if (isCollection()) {
    return collections::equals(this, &other);
  }
  if (UNLIKELY(instanceof(SystemLib::s_DateTimeInterfaceClass) &&
               other.instanceof(SystemLib::s_DateTimeInterfaceClass))) {
    return DateTimeData::compare(this, &other) == 0;
  }
  if (getVMClass() != other.getVMClass()) return false;
  if (UNLIKELY(instanceof(SimpleXMLElement_classof()))) {
    if (RuntimeOption::EvalNoticeOnSimpleXMLBehavior) {
      raise_notice("SimpleXMLElement equality comparison");
    }
    // Compare the whole object (including native data), not just props
    auto ar1 = SimpleXMLElement_objectCast(this, KindOfArray).toArray();
    auto ar2 = SimpleXMLElement_objectCast(&other, KindOfArray).toArray();
    return ArrayData::Equal(ar1.get(), ar2.get());
  }
  if (UNLIKELY(instanceof(c_Closure::classof()))) {
    // First comparison already proves they are different
    return false;
  }

  // check for dynamic props first because we need to short-circuit if there's
  // a different number of them
  auto thisSize = UNLIKELY(getAttribute(HasDynPropArr)) ?
    dynPropArray().size() : 0;
  size_t otherSize = 0;
  ArrayData* otherDynProps = nullptr;
  if (UNLIKELY(other.getAttribute(HasDynPropArr))) {
    otherDynProps = other.dynPropArray().get();
    otherSize = otherDynProps->size();
  }
  if (thisSize != otherSize) return false;

  // Prevent circular referenced objects/arrays or deep ones.
  check_recursion_error();

  bool result = true;
  auto cls = m_cls;
  IteratePropMemOrderNoInc(
    this,
    [&](Slot slot, const Class::Prop& prop, tv_rval thisVal) {
      auto otherVal = other.propRvalAtOffset(slot);
      if ((UNLIKELY(thisVal.type() == KindOfUninit) ||
           UNLIKELY(otherVal.type() == KindOfUninit)) &&
          (prop.attrs & AttrLateInit)) {
        if (prop.attrs & AttrLateInitSoft) {
          raise_soft_late_init_prop(cls, prop.name, false);
          auto const& d = g_context->getSoftLateInitDefault();

          if (thisVal.type() == KindOfUninit) {
            tvDup(
              *d.asTypedValue(),
              const_cast<ObjectData*>(this)->propLvalAtOffset(slot)
            );
          }
          if (otherVal.type() == KindOfUninit) {
            tvDup(
              *d.asTypedValue(),
              const_cast<ObjectData*>(&other)->propLvalAtOffset(slot)
            );
          }
        } else {
          throw_late_init_prop(cls, prop.name, false);
        }
      }
      if (!tvEqual(thisVal.tv(), otherVal.tv())) {
        result = false;
        return true;
      }
      return false;
    },
    [&](Cell key, TypedValue thisVal) {
      if (!otherDynProps->exists(key) ||
          !tvEqual(thisVal, otherDynProps->get(key).tv())) {
        result = false;
        return true;
      }
      return false;
    }
  );
  return result;
}

bool ObjectData::less(const ObjectData& other) const {
  // compare is not symmetrical; order of operands matters here
  return compare(other) < 0;
}

bool ObjectData::lessEqual(const ObjectData& other) const {
  // compare is not symmetrical; order of operands matters here
  return compare(other) <= 0;
}

bool ObjectData::more(const ObjectData& other) const {
  // compare is not symmetrical; order of operands matters here
  return other.compare(*this) < 0;
}

bool ObjectData::moreEqual(const ObjectData& other) const {
  // compare is not symmetrical; order of operands matters here
  return other.compare(*this) <= 0;
}

int64_t ObjectData::compare(const ObjectData& other) const {
  if (isCollection() || other.isCollection()) {
    throw_collection_compare_exception();
  }
  if (this == &other) return 0;
  if (UNLIKELY(instanceof(SystemLib::s_DateTimeInterfaceClass) &&
               other.instanceof(SystemLib::s_DateTimeInterfaceClass))) {
    return DateTimeData::compare(this, &other);
  }
  // Return 1 for different classes to match PHP7 behavior.
  if (getVMClass() != other.getVMClass()) return 1;
  if (UNLIKELY(instanceof(SimpleXMLElement_classof()))) {
    if (RuntimeOption::EvalNoticeOnSimpleXMLBehavior) {
      raise_notice("SimpleXMLElement comparison");
    }
    // Compare the whole object (including native data), not just props
    auto ar1 = SimpleXMLElement_objectCast(this, KindOfArray).toArray();
    auto ar2 = SimpleXMLElement_objectCast(&other, KindOfArray).toArray();
    return ArrayData::Compare(ar1.get(), ar2.get());
  }
  if (UNLIKELY(instanceof(c_Closure::classof()))) {
    // comparing different closures with <=> always returns 1
    return 1;
  }

  // check for dynamic props first, because we need to short circuit if there's
  // a different number of them
  auto thisSize = UNLIKELY(getAttribute(HasDynPropArr)) ?
    dynPropArray().size() : 0;
  size_t otherSize = 0;
  ArrayData* otherDynProps = nullptr;
  if (UNLIKELY(other.getAttribute(HasDynPropArr))) {
    otherDynProps = other.dynPropArray().get();
    otherSize = otherDynProps->size();
  }
  if (thisSize > otherSize) {
    return 1;
  } else if (thisSize < otherSize) {
    return -1;
  }

  // Prevent circular referenced objects/arrays or deep ones.
  check_recursion_error();

  int64_t result = 0;
  auto cls = m_cls;
  IteratePropToArrayOrderNoInc(
    this,
    [&](Slot slot, const Class::Prop& prop, tv_rval thisVal) {
      auto otherVal = other.propRvalAtOffset(slot);
      if ((UNLIKELY(thisVal.type() == KindOfUninit) ||
           UNLIKELY(otherVal.type() == KindOfUninit)) &&
          (prop.attrs & AttrLateInit)) {
        if (prop.attrs & AttrLateInitSoft) {
          raise_soft_late_init_prop(cls, prop.name, false);
          auto const& d = g_context->getSoftLateInitDefault();

          if (thisVal.type() == KindOfUninit) {
            tvDup(
              *d.asTypedValue(),
              const_cast<ObjectData*>(this)->propLvalAtOffset(slot)
            );
          }
          if (otherVal.type() == KindOfUninit) {
            tvDup(
              *d.asTypedValue(),
              const_cast<ObjectData*>(&other)->propLvalAtOffset(slot)
            );
          }
        } else {
          throw_late_init_prop(cls, prop.name, false);
        }
      }
      auto cmp = tvCompare(thisVal.tv(), otherVal.tv());
      if (cmp != 0) {
        result = cmp;
        return true;
      }
      return false;
    },
    [&](Cell key, TypedValue thisVal) {
      if (!otherDynProps->exists(key)) {
        result = 1;
        return true;
      }
      auto cmp = tvCompare(thisVal, otherDynProps->get(key).tv());
      if (cmp != 0) {
        result = cmp;
        return true;
      }
      return false;
    }
  );
  return result;
}

///////////////////////////////////////////////////////////////////////////////

const StaticString
  s___get("__get"),
  s___set("__set"),
  s___isset("__isset"),
  s___unset("__unset"),
  s___sleep("__sleep"),
  s___toDebugDisplay("__toDebugDisplay"),
  s___wakeup("__wakeup"),
  s___debugInfo("__debugInfo");

void deepInitHelper(TypedValue* propVec, const TypedValueAux* propData,
                    size_t nProps) {
  auto dst = propVec;
  auto src = propData;
  for (; src != propData + nProps; ++src, ++dst) {
    *dst = *src;
    // m_aux.u_deepInit is true for properties that need "deep" initialization
    if (src->deepInit()) {
      tvIncRefGen(*dst);
      collections::deepCopy(dst);
    }
  }
}

void ObjectData::setReifiedGenerics(Class* cls, ArrayData* reifiedTypes) {
  auto const arg = RuntimeOption::EvalHackArrDVArrs
    ? make_tv<KindOfVec>(reifiedTypes) : make_tv<KindOfArray>(reifiedTypes);
  auto const meth = cls->lookupMethod(s_86reifiedinit.get());
  assertx(meth != nullptr);
  g_context->invokeMethod(this, meth, InvokeArgs(&arg, 1));
}

// called from jit code
ObjectData* ObjectData::newInstanceRawSmall(Class* cls, size_t size,
                                            size_t index) {
  assertx(size <= kMaxSmallSize);
  assertx(!cls->hasMemoSlots());
  auto mem = tl_heap->mallocSmallIndexSize(index, size);
  return new (NotNull{}, mem) ObjectData(cls, InitRaw{}, IsBeingConstructed);
}

ObjectData* ObjectData::newInstanceRawBig(Class* cls, size_t size) {
  assertx(!cls->hasMemoSlots());
  auto mem = tl_heap->mallocBigSize(size);
  return new (NotNull{}, mem) ObjectData(cls, InitRaw{}, IsBeingConstructed);
}

// called from jit code
ObjectData* ObjectData::newInstanceRawMemoSmall(Class* cls,
                                                size_t size,
                                                size_t index,
                                                size_t objoff) {
  assertx(size <= kMaxSmallSize);
  assertx(cls->hasMemoSlots());
  assertx(!cls->getNativeDataInfo());
  assertx(objoff == ObjectData::objOffFromMemoNode(cls));
  auto mem = tl_heap->mallocSmallIndexSize(index, size);
  new (NotNull{}, mem) MemoNode(objoff);
  return new (NotNull{}, reinterpret_cast<char*>(mem) + objoff)
    ObjectData(cls, InitRaw{}, IsBeingConstructed);
}

ObjectData* ObjectData::newInstanceRawMemoBig(Class* cls,
                                              size_t size,
                                              size_t objoff) {
  assertx(cls->hasMemoSlots());
  assertx(!cls->getNativeDataInfo());
  assertx(objoff == ObjectData::objOffFromMemoNode(cls));
  auto mem = tl_heap->mallocBigSize(size);
  new (NotNull{}, mem) MemoNode(objoff);
  return new (NotNull{}, reinterpret_cast<char*>(mem) + objoff)
    ObjectData(cls, InitRaw{}, IsBeingConstructed);
}

// Note: the normal object destruction path does not actually call this
// destructor.  See ObjectData::release.
ObjectData::~ObjectData() {
  if (UNLIKELY(slowDestroyCheck())) {
    // The only builtin classes that use ~ObjectData and support memoization
    // are ones with native data, and the memo slot cleanup for them happens
    // in nativeDataInstanceDtor.
    assertx(!getAttribute(UsedMemoCache) || hasNativeData());
    if (getAttribute(HasDynPropArr)) freeDynPropArray(this);
    if (getAttribute(IsWeakRefed)) {
      WeakRefData::invalidateWeakRef((uintptr_t)this);
    }
  }
}

Object ObjectData::FromArray(ArrayData* properties) {
  assertx(properties->isPHPArray());
  Object retval{SystemLib::s_stdclassClass};
  retval->setAttribute(HasDynPropArr);
  g_context->dynPropTable.emplace(retval.get(), properties);
  return retval;
}

NEVER_INLINE
void ObjectData::throwMutateConstProp(Slot prop) const {
  throw_cannot_modify_const_prop(
    getClassName().data(),
    m_cls->declProperties()[prop].name->data()
  );
}

template <bool forWrite, bool forRead, bool ignoreLateInit>
ALWAYS_INLINE
ObjectData::PropLookup ObjectData::getPropImpl(
  const Class* ctx,
  const StringData* key
) {
  auto const lookup = m_cls->getDeclPropIndex(ctx, key);
  auto const propIdx = lookup.slot;

  if (LIKELY(propIdx != kInvalidSlot)) {
    // We found a visible property, but it might not be accessible.  No need to
    // check if there is a dynamic property with this name.
    auto const prop = const_cast<TypedValue*>(&propVec()[propIdx]);
    assertx(assertTypeHint(prop, propIdx));

    auto const& declProp = m_cls->declProperties()[propIdx];
    if (!ignoreLateInit && lookup.accessible) {
      if (UNLIKELY(prop->m_type == KindOfUninit) &&
          (declProp.attrs & AttrLateInit)) {
        if (declProp.attrs & AttrLateInitSoft) {
          raise_soft_late_init_prop(declProp.cls, key, false);
          tvDup(
            *g_context->getSoftLateInitDefault().asTypedValue(),
            prop
          );
        } else {
          throw_late_init_prop(declProp.cls, key, false);
        }
      }
    }

    return {
     prop,
     &declProp,
     propIdx,
     lookup.accessible,
     // we always return true in the !forWrite case; this way the compiler
     // may optimize away this value, and if a caller intends to write but
     // instantiates with false by mistake it will always see const
     forWrite
       ? bool(declProp.attrs & AttrIsConst)
       : true
    };
  }

  // We could not find a visible declared property. We need to check for a
  // dynamic property with this name.
  if (UNLIKELY(getAttribute(HasDynPropArr))) {
    auto& arr = dynPropArray();
    if (auto const rval = arr->rval(key)) {
      if (forRead && RuntimeOption::EvalNoticeOnReadDynamicProp) {
        raiseReadDynamicProp(key);
      }
      // Returning a non-declared property. We know that it is accessible and
      // not const since all dynamic properties are. If we may write to
      // the property we need to allow the array to escalate.
      if (forWrite) {
        auto const lval = arr.lvalAt(StrNR(key), AccessFlags::Key);
        return { lval, nullptr, kInvalidSlot, true, false };
      }
      return { rval.as_lval(), nullptr, kInvalidSlot, true, true };
    }
  }

  return { nullptr, nullptr, kInvalidSlot, false, forWrite ? false : true };
}

tv_lval ObjectData::getPropLval(const Class* ctx, const StringData* key) {
  auto const lookup = getPropImpl<true, false, true>(ctx, key);
  if (UNLIKELY(lookup.isConst) && !isBeingConstructed()) {
    throwMutateConstProp(lookup.slot);
  }
  return lookup.val && lookup.accessible ? lookup.val : nullptr;
}

tv_rval ObjectData::getProp(const Class* ctx, const StringData* key) const {
  auto const lookup = const_cast<ObjectData*>(this)
    ->getPropImpl<false, true, false>(ctx, key);
  return lookup.val && lookup.accessible ? lookup.val : nullptr;
}

tv_rval ObjectData::getPropIgnoreLateInit(const Class* ctx,
                                          const StringData* key) const {
  auto const lookup = const_cast<ObjectData*>(this)
    ->getPropImpl<false, true, true>(ctx, key);
  return lookup.val && lookup.accessible ? lookup.val : nullptr;
}

tv_rval ObjectData::getPropIgnoreAccessibility(const StringData* key) {
  auto const lookup = getPropImpl<false, true, true>(nullptr, key);
  auto prop = lookup.val;
  if (!prop) return nullptr;
  if (lookup.prop && type(prop) == KindOfUninit &&
      (lookup.prop->attrs & AttrLateInit)) {

    if (lookup.prop->attrs & AttrLateInitSoft) {
      raise_soft_late_init_prop(lookup.prop->cls, key, false);
      tvDup(
        *g_context->getSoftLateInitDefault().asTypedValue(),
        prop
      );
      return prop;
    }
    throw_late_init_prop(lookup.prop->cls, key, false);
  }
  return prop;
}

//////////////////////////////////////////////////////////////////////

inline InvokeResult::InvokeResult(bool ok, Variant&& v) :
  val(*v.asTypedValue()) {
  tvWriteUninit(*v.asTypedValue());
  val.m_aux.u_ok = ok;
}

struct PropAccessInfo {
  struct Hash;

  bool operator==(const PropAccessInfo& o) const {
    return obj == o.obj && rt_attr == o.rt_attr && key->same(o.key);
  }

  ObjectData* obj;
  const StringData* key;      // note: not necessarily static
  Class::RuntimeAttribute rt_attr;
};

struct PropAccessInfo::Hash {
  size_t operator()(PropAccessInfo const& info) const {
    return hash_int64_pair(reinterpret_cast<intptr_t>(info.obj),
                           info.key->hash() |
                           (static_cast<int64_t>(info.rt_attr) << 32));
  }
};

struct PropRecurInfo {
  using RecurSet = req::fast_set<PropAccessInfo, PropAccessInfo::Hash>;
  // activePropInfo optimizes the common non-recursive case, when our activeSet
  // would only contain one entry. When we add a second entry, we start using
  // activeSet.
  const PropAccessInfo* activePropInfo{nullptr};
  RecurSet activeSet;
};

namespace {

/*
 * Recursion of magic property accessors is allowed, but if you
 * recurse on the same object, for the same property, for the same
 * kind of magic method, it doesn't actually enter the magic method
 * anymore.  This matches zend behavior.
 *
 * This means we need to track all active property getters and ensure
 * we aren't recursing for the same one.  Since most accesses to magic
 * property getters aren't going to recurse, we optimize for the case
 * where only a single getter is active.  If it recurses again, we
 * promote to a hash set to track all the information needed.
 *
 * The various invokeFoo functions are the entry points here.  They
 * require that the appropriate ObjectData::Attribute has been checked
 * first, and return false if they refused to run the magic method due
 * to a recursion error.
 */

RDS_LOCAL(PropRecurInfo, propRecurInfo);

template <class Invoker>
InvokeResult
magic_prop_impl(const StringData* /*key*/, const PropAccessInfo& info,
                Invoker invoker) {
  auto recur_info = propRecurInfo.get();
  if (UNLIKELY(recur_info->activePropInfo != nullptr)) {
    auto& activeSet = recur_info->activeSet;
    if (activeSet.empty()) {
      activeSet.insert(*recur_info->activePropInfo);
    }
    if (!activeSet.insert(info).second) {
      // We're already running a magic method on the same type here.
      return {false, make_tv<KindOfUninit>()};
    }
    SCOPE_EXIT {
      activeSet.erase(info);
    };

    return {true, invoker()};
  }

  recur_info->activePropInfo = &info;
  SCOPE_EXIT {
    recur_info->activePropInfo = nullptr;
    PropRecurInfo::RecurSet{}.swap(recur_info->activeSet);
  };

  return {true, invoker()};
}

// Helper for making invokers for the single-argument magic property
// methods.  __set takes 2 args, so it uses its own function.
struct MagicInvoker {
  const StringData* magicFuncName;
  const PropAccessInfo& info;

  TypedValue operator()() const {
    auto const meth = info.obj->getVMClass()->lookupMethod(magicFuncName);
    TypedValue args[1] = {
      make_tv<KindOfString>(const_cast<StringData*>(info.key))
    };
    return g_context->invokeMethod(info.obj, meth, folly::range(args), false);
  }
};

}

bool ObjectData::invokeSet(const StringData* key, Cell val) {
  auto const info = PropAccessInfo { this, key, Class::UseSet };
  auto r = magic_prop_impl(key, info, [&] {
    auto const meth = m_cls->lookupMethod(s___set.get());
    TypedValue args[2] = {
      make_tv<KindOfString>(const_cast<StringData*>(key)),
      val
    };
    return g_context->invokeMethod(this, meth, folly::range(args), false);
  });
  if (r) tvDecRefGen(r.val);
  return r.ok();
}

InvokeResult ObjectData::invokeGet(const StringData* key) {
  auto const info = PropAccessInfo { this, key, Class::UseGet };
  return magic_prop_impl(
    key,
    info,
    MagicInvoker { s___get.get(), info }
  );
}

InvokeResult ObjectData::invokeIsset(const StringData* key) {
  auto const info = PropAccessInfo { this, key, Class::UseIsset };
  return magic_prop_impl(
    key,
    info,
    MagicInvoker { s___isset.get(), info }
  );
}

bool ObjectData::invokeUnset(const StringData* key) {
  auto const info = PropAccessInfo { this, key, Class::UseUnset };
  auto r = magic_prop_impl(key, info,
                           MagicInvoker{s___unset.get(), info});
  if (r) tvDecRefGen(r.val);
  return r.ok();
}

static InvokeResult guardedNativePropResult(Variant result) {
  if (!Native::isPropHandled(result)) {
    return {false, make_tv<KindOfUninit>()};
  }
  return InvokeResult{true, std::move(result)};
}

InvokeResult ObjectData::invokeNativeGetProp(const StringData* key) {
  return guardedNativePropResult(
      Native::getProp(Object{this}, StrNR(key))
  );
}

bool ObjectData::invokeNativeSetProp(const StringData* key, Cell val) {
  auto r = guardedNativePropResult(
    Native::setProp(Object{this}, StrNR(key), tvAsCVarRef(&val))
  );
  tvDecRefGen(r.val);
  return r.ok();
}

InvokeResult ObjectData::invokeNativeIssetProp(const StringData* key) {
  return guardedNativePropResult(
      Native::issetProp(Object{this}, StrNR(key))
  );
}

bool ObjectData::invokeNativeUnsetProp(const StringData* key) {
  auto r = guardedNativePropResult(
      Native::unsetProp(Object{this}, StrNR(key))
  );
  tvDecRefGen(r.val);
  return r.ok();
}

//////////////////////////////////////////////////////////////////////

template<ObjectData::PropMode mode>
ALWAYS_INLINE
tv_lval ObjectData::propImpl(TypedValue* tvRef, const Class* ctx,
                             const StringData* key, MInstrPropState* pState) {
  auto constexpr write = (mode == PropMode::DimForWrite);
  auto constexpr read = (mode == PropMode::ReadNoWarn) ||
                        (mode == PropMode::ReadWarn);
  auto const lookup = getPropImpl<write, read, false>(ctx, key);
  auto const prop = lookup.val;

  if (prop) {
    if (lookup.accessible) {
      auto const checkConstProp = [&]() {
        if (mode == PropMode::DimForWrite) {
          if (UNLIKELY(lookup.isConst) && !isBeingConstructed()) {
            throwMutateConstProp(lookup.slot);
          }
        }
        if (write && pState) {
          *pState = MInstrPropState{m_cls, lookup.slot, false};
        }
        return prop;
      };

      // Property exists, is accessible, and is not unset.
      if (type(prop) != KindOfUninit) return checkConstProp();

      // Property is unset, try __get.
      if (m_cls->rtAttribute(Class::UseGet)) {
        if (auto r = invokeGet(key)) {
          tvCopy(r.val, *tvRef);
          if (write && pState) *pState = MInstrPropState{};
          return tvRef;
        }
      }

      if (mode == PropMode::ReadWarn) raiseUndefProp(key);
      if (write) return checkConstProp();
      return const_cast<TypedValue*>(&immutable_null_base);
    }

    // Property is not accessible, try __get.
    if (m_cls->rtAttribute(Class::UseGet)) {
      if (auto r = invokeGet(key)) {
        tvCopy(r.val, *tvRef);
        if (write && pState) *pState = MInstrPropState{};
        return tvRef;
      }
    }

    // Property exists, but it is either protected or private since accessible
    // is false.
    auto const propInd = m_cls->lookupDeclProp(key);
    auto const attrs = m_cls->declProperties()[propInd].attrs;
    auto const priv = (attrs & AttrPrivate) ? "private" : "protected";

    raise_error(
      "Cannot access %s property %s::$%s",
      priv,
      m_cls->preClass()->name()->data(),
      key->data()
    );
  }

  // First see if native getter is implemented.
  if (m_cls->rtAttribute(Class::HasNativePropHandler)) {
    if (auto r = invokeNativeGetProp(key)) {
      tvCopy(r.val, *tvRef);
      if (write && pState) *pState = MInstrPropState{};
      return tvRef;
    }
  }

  // Next try calling user-level `__get` if it's used.
  if (m_cls->rtAttribute(Class::UseGet)) {
    if (auto r = invokeGet(key)) {
      tvCopy(r.val, *tvRef);
      if (write && pState) *pState = MInstrPropState{};
      return tvRef;
    }
  }

  if (UNLIKELY(!*key->data())) {
    throw_invalid_property_name(StrNR(key));
  }

  if (mode == PropMode::ReadWarn) raiseUndefProp(key);
  if (write) {
    if (pState) *pState = MInstrPropState{};
    return makeDynProp(key);
  }
  return const_cast<TypedValue*>(&immutable_null_base);
}

tv_lval ObjectData::prop(
  TypedValue* tvRef,
  const Class* ctx,
  const StringData* key
) {
  return propImpl<PropMode::ReadNoWarn>(tvRef, ctx, key, nullptr);
}

tv_lval ObjectData::propW(
  TypedValue* tvRef,
  const Class* ctx,
  const StringData* key
) {
  return propImpl<PropMode::ReadWarn>(tvRef, ctx, key, nullptr);
}

tv_lval ObjectData::propU(
  TypedValue* tvRef,
  const Class* ctx,
  const StringData* key
) {
  return propImpl<PropMode::DimForWrite>(tvRef, ctx, key, nullptr);
}

tv_lval ObjectData::propD(
  TypedValue* tvRef,
  const Class* ctx,
  const StringData* key,
  MInstrPropState* pState
) {
  return propImpl<PropMode::DimForWrite>(tvRef, ctx, key, pState);
}

bool ObjectData::propIsset(const Class* ctx, const StringData* key) {
  auto const lookup = getPropImpl<false, true, true>(ctx, key);
  if (lookup.val && lookup.accessible) {
    if (lookup.val.type() != KindOfUninit) {
      return lookup.val.unboxed().type() != KindOfNull;
    }
    if (lookup.prop && (lookup.prop->attrs & AttrLateInit)) {
      return false;
    }
  }

  if (m_cls->rtAttribute(Class::HasNativePropHandler)) {
    if (auto r = invokeNativeIssetProp(key)) {
      tvCastToBooleanInPlace(&r.val);
      return r.val.m_data.num;
    }
  }

  if (!m_cls->rtAttribute(Class::UseIsset)) return false;
  auto r = invokeIsset(key);
  if (!r) return false;
  tvCastToBooleanInPlace(&r.val);
  return r.val.m_data.num;
}

bool ObjectData::propEmptyImpl(const Class* ctx, const StringData* key) {
  auto const lookup = getPropImpl<false, true, true>(ctx, key);
  if (lookup.val && lookup.accessible) {
    if (lookup.val.type() != KindOfUninit) {
      return !cellToBool(lookup.val.unboxed().tv());
    }
    if (lookup.prop && (lookup.prop->attrs & AttrLateInit)) {
      return true;
    }
  }

  if (m_cls->rtAttribute(Class::HasNativePropHandler)) {
    if (auto r = invokeNativeIssetProp(key)) {
      tvCastToBooleanInPlace(&r.val);
      if (!r.val.m_data.num) return true;
      if (auto r2 = invokeNativeGetProp(key)) {
        auto const emptyResult = !cellToBool(*tvToCell(&r2.val));
        tvDecRefGen(&r2.val);
        return emptyResult;
      }
      return false;
    }
  }

  if (!m_cls->rtAttribute(Class::UseIsset)) return true;
  auto r = invokeIsset(key);
  if (!r) return true;

  tvCastToBooleanInPlace(&r.val);
  if (!r.val.m_data.num) return true;

  if (m_cls->rtAttribute(Class::UseGet)) {
    if (auto r = invokeGet(key)) {
      auto const emptyResult = !cellToBool(*tvToCell(&r.val));
      tvDecRefGen(&r.val);
      return emptyResult;
    }
  }
  return false;
}

bool ObjectData::propEmpty(const Class* ctx, const StringData* key) {
  if (UNLIKELY(m_cls->rtAttribute(Class::CallToImpl))) {
    // We only get here for SimpleXMLElement or collections
    if (LIKELY(!isCollection())) {
      assertx(instanceof(SimpleXMLElement_classof()));
      if (RuntimeOption::EvalNoticeOnSimpleXMLBehavior) {
        raise_notice("SimpleXMLElement empty() property check");
      }
      return SimpleXMLElement_propEmpty(this, key);
    }
  }
  return propEmptyImpl(ctx, key);
}

void ObjectData::setProp(Class* ctx, const StringData* key, Cell val) {
  assertx(cellIsPlausible(val));
  assertx(val.m_type != KindOfUninit);

  auto const lookup = getPropImpl<true, false, true>(ctx, key);
  auto const prop = lookup.val;

  if (prop && lookup.accessible) {
    if (type(prop) != KindOfUninit ||
        !m_cls->rtAttribute(Class::UseSet) ||
        (lookup.prop && (lookup.prop->attrs & AttrLateInit)) ||
        !invokeSet(key, val)) {
      if (UNLIKELY(lookup.isConst) && !isBeingConstructed()) {
        throwMutateConstProp(lookup.slot);
      }
      verifyTypeHint(m_cls, lookup.prop, &val);
      tvSet(val, prop);
    }
    return;
  }

  // First see if native setter is implemented.
  if (m_cls->rtAttribute(Class::HasNativePropHandler) &&
      invokeNativeSetProp(key, val)) {
    return;
  }

  // Then go to user-level `__set`.
  if (!m_cls->rtAttribute(Class::UseSet) || !invokeSet(key, val)) {
    if (prop) {
      /*
       * Note: this differs from Zend right now in the case of a
       * failed recursive __set.  In Zend, the __set is silently
       * dropped, and the protected property is not modified.
       */
      raise_error("Cannot access protected property");
    }
    if (UNLIKELY(!*key->data())) {
      throw_invalid_property_name(StrNR(key));
    }
    setDynProp(key, val);
    return;
  }
}

tv_lval ObjectData::setOpProp(TypedValue& tvRef,
                              Class* ctx,
                              SetOpOp op,
                              const StringData* key,
                              Cell* val) {
  auto const lookup = getPropImpl<true, true, false>(ctx, key);
  auto prop = lookup.val;

  if (prop && lookup.accessible) {
    if (type(prop) == KindOfUninit && m_cls->rtAttribute(Class::UseGet)) {
      if (auto r = invokeGet(key)) {
        SCOPE_EXIT { tvDecRefGen(r.val); };
        // don't unbox until after setopBody; see longer comment below
        setopBody(tvToCell(&r.val), op, val);
        tvUnboxIfNeeded(r.val);
        if (m_cls->rtAttribute(Class::UseSet)) {
          cellDup(tvAssertCell(r.val), tvRef);
          if (invokeSet(key, tvAssertCell(tvRef))) {
            return &tvRef;
          }
          tvRef.m_type = KindOfUninit;
        }
        if (UNLIKELY(lookup.isConst) && !isBeingConstructed()) {
          throwMutateConstProp(lookup.slot);
        }
        verifyTypeHint(m_cls, lookup.prop, tvAssertCell(&r.val));
        cellDup(tvAssertCell(r.val), prop);
        return prop;
      }
    }
    if (UNLIKELY(lookup.isConst) && !isBeingConstructed()) {
      throwMutateConstProp(lookup.slot);
    }
    prop = tvToCell(prop);

    if (lookup.prop &&
        setOpNeedsTypeCheck(lookup.prop->typeConstraint, op, prop)) {
      /*
       * If this property has a type-hint, we can't do the setop truly in
       * place. We need to verify that the new value satisfies the type-hint
       * before assigning back to the property (if we raise a warning and throw,
       * we don't want to have already put the value into the prop).
       */
      Cell temp;
      cellDup(*prop, temp);
      SCOPE_FAIL { tvDecRefGen(&temp); };
      setopBody(&temp, op, val);
      verifyTypeHint(m_cls, lookup.prop, &temp);
      cellMove(temp, prop);
    } else {
      setopBody(prop, op, val);
    }
    return prop;
  }

  if (UNLIKELY(!*key->data())) throw_invalid_property_name(StrNR(key));

  // Native accessors.
  if (m_cls->rtAttribute(Class::HasNativePropHandler)) {
    if (auto r = invokeNativeGetProp(key)) {
      tvCopy(r.val, tvRef);
      setopBody(tvToCell(&tvRef), op, val);
      if (invokeNativeSetProp(key, tvToCell(tvRef))) {
        return &tvRef;
      }
    }
    // XXX else, write tvRef = null?
  }

  auto const useSet = m_cls->rtAttribute(Class::UseSet);
  auto const useGet = m_cls->rtAttribute(Class::UseGet);

  if (useGet && !useSet) {
    auto r = invokeGet(key);
    if (!r) tvWriteNull(r.val);
    SCOPE_EXIT { tvDecRefGen(r.val); };

    // Note: the tvUnboxIfNeeded comes *after* the setop on purpose
    // here, even though it comes before the IncDecOp in the analogous
    // situation in incDecProp.  This is to match zend 5.5 behavior.
    setopBody(tvToCell(&r.val), op, val);
    tvUnboxIfNeeded(r.val);

    if (prop) raise_error("Cannot access protected property");
    prop = makeDynProp(key);

    // Normally this code path is defining a new dynamic property, but
    // unlike the non-magic case below, we may have already created it
    // under the recursion into invokeGet above, so we need to do a
    // tvSet here.
    tvSet(r.val, prop);
    return prop;
  }

  if (useGet && useSet) {
    if (auto r = invokeGet(key)) {
      // Matching zend again: incDecProp does an unbox before the
      // operation, but setop doesn't need to here.  (We'll unbox the
      // value that gets passed to the magic setter, though, since
      // __set functions can't take parameters by reference.)
      tvCopy(r.val, tvRef);
      setopBody(tvToCell(&tvRef), op, val);
      invokeSet(key, tvToCell(tvRef));
      return &tvRef;
    }
  }

  if (prop) raise_error("Cannot access protected property");

  // No visible/accessible property, and no applicable magic method:
  // create a new dynamic property.  (We know this is a new property,
  // or it would've hit the visible && accessible case above.)
  prop = makeDynProp(key);
  assertx(type(prop) == KindOfNull); // cannot exist yet
  setopBody(prop, op, val);
  return prop;
}

Cell ObjectData::incDecProp(Class* ctx, IncDecOp op, const StringData* key) {
  auto const lookup = getPropImpl<true, true, false>(ctx, key);
  auto prop = lookup.val;

  if (prop && lookup.accessible) {
    if (type(prop) == KindOfUninit && m_cls->rtAttribute(Class::UseGet)) {
      if (auto r = invokeGet(key)) {
        SCOPE_EXIT { tvDecRefGen(r.val); };
        tvUnboxIfNeeded(r.val);
        auto const dest = IncDecBody(op, tvAssertCell(&r.val));
        if (m_cls->rtAttribute(Class::UseSet)) {
          invokeSet(key, tvAssertCell(r.val));
          return dest;
        }
        if (UNLIKELY(lookup.isConst) && !isBeingConstructed()) {
          throwMutateConstProp(lookup.slot);
        }
        verifyTypeHint(m_cls, lookup.prop, tvAssertCell(&r.val));
        cellCopy(tvAssertCell(r.val), prop);
        tvWriteNull(r.val); // suppress decref
        return dest;
      }
    }
    if (UNLIKELY(lookup.isConst) && !isBeingConstructed()) {
      throwMutateConstProp(lookup.slot);
    }
    if (type(prop) == KindOfUninit) {
      tvWriteNull(prop);
    } else {
      prop = tvToCell(prop);
    }

    /*
     * If this property has a type-hint, we can't do the inc-dec truely in
     * place. We need to verify that the new value satisfies the type-hint
     * before assigning back to the property (if we raise a warning and throw,
     * we don't want to have already put the value into the prop).
     *
     * If the prop is an integer and we're doing the common pre/post inc/dec
     * ops, we know the type won't change, so we can skip the type-hint check in
     * that case.
     */
    auto const fast = [&]{
      if (RuntimeOption::EvalCheckPropTypeHints <= 0) return true;
      if (!lookup.prop || !lookup.prop->typeConstraint.isCheckable()) {
        return true;
      }
      if (!isIntType(type(prop))) return false;
      return
        op == IncDecOp::PreInc || op == IncDecOp::PostInc ||
        op == IncDecOp::PreDec || op == IncDecOp::PostDec;
    }();
    if (fast) return IncDecBody(op, tvAssertCell(prop));

    Cell temp;
    cellDup(tvAssertCell(*prop), temp);
    SCOPE_FAIL { tvDecRefGen(&temp); };
    auto result = IncDecBody(op, &temp);
    SCOPE_FAIL { tvDecRefGen(&result); };
    verifyTypeHint(m_cls, lookup.prop, &temp);
    tvMove(temp, tvAssertCell(prop));
    return result;
  }

  if (UNLIKELY(!*key->data())) throw_invalid_property_name(StrNR(key));

  // Native accessors.
  if (m_cls->rtAttribute(Class::HasNativePropHandler)) {
    if (auto r = invokeNativeGetProp(key)) {
      SCOPE_EXIT { tvDecRefGen(r.val); };
      tvUnboxIfNeeded(r.val);
      auto const dest = IncDecBody(op, tvAssertCell(&r.val));
      if (invokeNativeSetProp(key, tvAssertCell(r.val))) {
        return dest;
      }
    }
  }

  auto const useSet = m_cls->rtAttribute(Class::UseSet);
  auto const useGet = m_cls->rtAttribute(Class::UseGet);

  if (useGet && !useSet) {
    auto r = invokeGet(key);
    if (!r) tvWriteNull(r.val);
    SCOPE_EXIT { tvDecRefGen(r.val); };
    tvUnboxIfNeeded(r.val);
    auto const dest = IncDecBody(op, tvAssertCell(&r.val));
    if (prop) raise_error("Cannot access protected property");
    prop = makeDynProp(key);

    // Normally this code path is defining a new dynamic property, but
    // unlike the non-magic case below, we may have already created it
    // under the recursion into invokeGet above, so we need to do a
    // tvSet here.
    tvSet(r.val, prop);
    return dest;
  }

  if (useGet && useSet) {
    if (auto r = invokeGet(key)) {
      SCOPE_EXIT { tvDecRefGen(r.val); };
      tvUnboxIfNeeded(r.val);
      auto const dest = IncDecBody(op, tvAssertCell(&r.val));
      invokeSet(key, tvAssertCell(r.val));
      return dest;
    }
  }

  if (prop) raise_error("Cannot access protected property");

  // No visible/accessible property, and no applicable magic method:
  // create a new dynamic property.  (We know this is a new property,
  // or it would've hit the visible && accessible case above.)
  prop = makeDynProp(key);
  assertx(type(prop) == KindOfNull); // cannot exist yet
  return IncDecBody(op, prop);
}

void ObjectData::unsetProp(Class* ctx, const StringData* key) {
  auto const lookup = getPropImpl<true, false, true>(ctx, key);
  auto const prop = lookup.val;

  if (prop && lookup.accessible &&
      (type(prop) != KindOfUninit ||
       (lookup.prop && (lookup.prop->attrs & AttrLateInit)))) {
    if (lookup.slot != kInvalidSlot) {
      // Declared property.
      if (UNLIKELY(lookup.isConst) && !isBeingConstructed()) {
        throwMutateConstProp(lookup.slot);
      }
      unsetTypeHint(lookup.prop);
      tvSetIgnoreRef(*uninit_variant.asTypedValue(), prop);
    } else {
      // Dynamic property.
      dynPropArray().remove(StrNR(key).asString(), true /* isString */);
    }
    return;
  }

  // Native unset first.
  if (m_cls->rtAttribute(Class::HasNativePropHandler) &&
      invokeNativeUnsetProp(key)) {
    return;
  }

  auto const tryUnset = m_cls->rtAttribute(Class::UseUnset);

  if (prop && !lookup.accessible && !tryUnset) {
    // Defined property that is not accessible.
    raise_error("Cannot unset inaccessible property");
  }

  if (!tryUnset || !invokeUnset(key)) {
    if (UNLIKELY(!*key->data())) {
      throw_invalid_property_name(StrNR(key));
    }
    return;
  }
}

void ObjectData::raiseObjToIntNotice(const char* clsName) {
  raise_notice("Object of class %s could not be converted to int", clsName);
}

void ObjectData::raiseObjToDoubleNotice(const char* clsName) {
  raise_notice("Object of class %s could not be converted to float", clsName);
}

void ObjectData::raiseAbstractClassError(Class* cls) {
  Attr attrs = cls->attrs();
  raise_error("Cannot instantiate %s %s",
              (attrs & AttrInterface) ? "interface" :
              (attrs & AttrTrait)     ? "trait" :
              (attrs & AttrEnum)      ? "enum" : "abstract class",
              cls->preClass()->name()->data());
}

void ObjectData::raiseUndefProp(const StringData* key) const {
  raise_notice("Undefined property: %s::$%s",
               m_cls->name()->data(), key->data());
}

void ObjectData::raiseCreateDynamicProp(const StringData* key) const {
  if (m_cls == SystemLib::s_stdclassClass ||
      m_cls == SystemLib::s___PHP_Incomplete_ClassClass) {
    // these classes (but not classes derived from them) don't get notices
    return;
  }
  if (key->isStatic()) {
    raise_notice("Created dynamic property with static name %s::%s",
                 m_cls->name()->data(), key->data());
  } else {
    raise_notice("Created dynamic property with dynamic name %s::%s",
                 m_cls->name()->data(), key->data());
  }
}

void ObjectData::raiseReadDynamicProp(const StringData* key) const {
  if (m_cls == SystemLib::s_stdclassClass ||
      m_cls == SystemLib::s___PHP_Incomplete_ClassClass) {
    // these classes (but not classes derived from them) don't get notices
    return;
  }
  if (key->isStatic()) {
    raise_notice("Read dynamic property with static name %s::%s",
                 m_cls->name()->data(), key->data());
  } else {
    raise_notice("Read dynamic property with dynamic name %s::%s",
                 m_cls->name()->data(), key->data());
  }
}

void ObjectData::raiseImplicitInvokeToString() const {
  raise_notice("Implicitly invoked %s::__toString", m_cls->name()->data());
}

Variant ObjectData::InvokeSimple(ObjectData* obj, const StaticString& name) {
  auto const meth = obj->methodNamed(name.get());
  return meth
    ? g_context->invokeMethodV(obj, meth, InvokeArgs{}, false)
    : uninit_null();
}

Variant ObjectData::invokeSleep() {
  return InvokeSimple(this, s___sleep);
}

Variant ObjectData::invokeToDebugDisplay() {
  return InvokeSimple(this, s___toDebugDisplay);
}

Variant ObjectData::invokeWakeup() {
  return InvokeSimple(this, s___wakeup);
}

Variant ObjectData::invokeDebugInfo() {
  return InvokeSimple(this, s___debugInfo);
}

String ObjectData::invokeToString() {
  if (RuntimeOption::EvalFatalOnConvertObjectToString) {
    raise_convert_object_to_string(classname_cstr());
  }

  const Func* method = m_cls->getToString();
  if (!method) {
    // If the object does not define a __toString() method, raise a
    // recoverable error
    raise_recoverable_error(
      "Object of class %s could not be converted to string",
      classname_cstr()
    );
    // If the user error handler decides to allow execution to continue,
    // we return the empty string.
    return empty_string();
  }
  if (RuntimeOption::EvalNoticeOnImplicitInvokeToString) {
    raiseImplicitInvokeToString();
  }
  auto const tv = g_context->invokeMethod(this, method, InvokeArgs{}, false);
  if (!isStringType(tv.m_type)) {
    // Discard the value returned by the __toString() method and raise
    // a recoverable error
    tvDecRefGen(tv);
    raise_recoverable_error(
      "Method %s::__toString() must return a string value",
      m_cls->preClass()->name()->data());
    // If the user error handler decides to allow execution to continue,
    // we return the empty string.
    return empty_string();
  }

  return String::attach(tv.m_data.pstr);
}

bool ObjectData::hasToString() {
  return (m_cls->getToString() != nullptr);
}

const char* ObjectData::classname_cstr() const {
  return getClassName().data();
}

} // HPHP
