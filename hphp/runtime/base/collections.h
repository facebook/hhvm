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

#ifndef incl_HPHP_COLLECTIONS_H_
#define incl_HPHP_COLLECTIONS_H_

#include <folly/Optional.h>

#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/type-string.h"

namespace HPHP {
class ArrayData;
class c_Pair;
class c_Vector;
class c_ImmVector;
class c_Map;
class c_ImmMap;
class c_Set;
class c_ImmSet;
class VariableSerializer;
class VariableUnserializer;
}

namespace HPHP { namespace collections {
/////////////////////////////////////////////////////////////////////////////

#define COLLECTIONS_PAIRED_TYPES(MACRO) \
  MACRO(Vector) MACRO(ImmVector) \
  MACRO(Map)    MACRO(ImmMap) \
  MACRO(Set)    MACRO(ImmSet)

#define COLLECTIONS_ALL_TYPES(MACRO) \
  MACRO(Pair) \
  COLLECTIONS_PAIRED_TYPES(MACRO)

#define X(type) extern const StaticString s_##type;
COLLECTIONS_ALL_TYPES(X)
#undef X

/////////////////////////////////////////////////////////////////////////////
// Constructor/Initializer

using newEmptyInstanceFunc = ObjectData* (*)();
using newFromArrayFunc = ObjectData* (*)(ArrayData* arr);

/* Get a function capable of creating a collection class.
 * This is primarily used by the JIT to burn the initializer into the TC.
 */
newEmptyInstanceFunc allocEmptyFunc(CollectionType ctype);
newFromArrayFunc allocFromArrayFunc(CollectionType ctype);

/*
 * Create a new empty collection, with refcount set to 1.
 */
inline ObjectData* alloc(CollectionType ctype) {
  return allocEmptyFunc(ctype)();
}

/*
 * Create a collection from an array, with refcount set to 1.
 *
 * Pre: The array must have a kind that's compatible with the collection type
 * we're creating.
 */
inline ObjectData* alloc(CollectionType ctype, ArrayData* arr) {
  return allocFromArrayFunc(ctype)(arr);
}

/* Preallocate room for {sz} elements in the Collection */
void reserve(ObjectData* obj, int64_t sz);

/* Used by Collections Literals syntax for Maps */
void initMapElem(ObjectData* obj, TypedValue* key, TypedValue* val);
/* Used by Collections Literals syntax for non-Maps */
void initElem(ObjectData* obj, TypedValue* val);

/////////////////////////////////////////////////////////////////////////////
// Misc

bool isType(const Class* cls, CollectionType type);
template<typename ...Args>
bool isType(const Class* cls, CollectionType type, Args... args) {
  return isType(cls, type) || isType(cls, args...);
}

uint32_t sizeOffset(CollectionType type);
uint32_t dataOffset(CollectionType type);

void unserialize(ObjectData* obj, VariableUnserializer* uns,
                 int64_t sz, char type);
void serialize(ObjectData* obj, VariableSerializer* serializer);

/////////////////////////////////////////////////////////////////////////////
// Casting and Cloing

Array toArray(const ObjectData* obj);
bool toBool(const ObjectData* obj);
ObjectData* clone(ObjectData* obj);

void deepCopy(TypedValue* tv);

/*
 * Return the inner-array for array-backed collections, and nullptr if it's a
 * Pair.  The returned array pointer is not incref'd.
 */
ArrayData* asArray(ObjectData* obj);
inline const ArrayData* asArray(const ObjectData* obj) {
  return asArray(const_cast<ObjectData*>(obj));
}

/////////////////////////////////////////////////////////////////////////////
// Read/Write

/* at() and get() retrieve a collection element for reading.
 * If the key does not exist in the collection, at() will throw an exception
 * while get() will return nullptr
 */
TypedValue* at(ObjectData* obj, const TypedValue* key);
TypedValue* get(ObjectData* obj, const TypedValue* key);

/* atLval() is used to get the address of an element when the
 * caller is NOT going to do direct write per se, but it intends to use
 * the element as the base of a member operation in an "lvalue" context
 * (which could mutate the element in various ways).
 */
TypedValue* atLval(ObjectData* obj, const TypedValue* key);

/* atRw() is used to get the address of an element for reading
 * and writing. It is typically used for read-modify-write operations (the
 * SetOp* and IncDec* instructions).
 */
TypedValue* atRw(ObjectData* obj, const TypedValue* key);

/* Check for {key} within {obj} Collection
 * `contains` merely need to exist
 * `isset` needs to exist and not be null
 * `empty` needs to exist and not be falsy
 */
bool contains(ObjectData* obj, const Variant& offset);
bool isset(ObjectData* obj, const TypedValue* key);
bool empty(ObjectData* obj, const TypedValue* key);

/* Remove element {key} from Collection {obj} */
void unset(ObjectData* obj, const TypedValue* key);

/* Add element {val} to Collection {obj} at {key} or the next slot */
void append(ObjectData* obj, TypedValue* val);
void set(ObjectData* obj, const TypedValue* key, const TypedValue* val);

/* Compare two collections.
 * Only collections of the same type with the same elements are equal.
 * Whether elements must be in the same order is CollectionType dependent.
 */
bool equals(const ObjectData* obj1, const ObjectData* obj2);

/* Take element from end(pop) or beginning(shift) of a mutable collection */
Variant pop(ObjectData* obj);
Variant shift(ObjectData* obj);

/////////////////////////////////////////////////////////////////////////////
// CollectionType <-> Human-readable name conversions

/*
 * Returns a collection type name given a CollectionType.
 */
inline StringData* typeToString(CollectionType ctype) {
  switch (ctype) {
#define X(type) case CollectionType::type: return s_##type.get();
COLLECTIONS_ALL_TYPES(X)
#undef X
  }
  not_reached();
}

/*
 * Returns a CollectionType given a name, folly::none if name is not a
 * collection type.
 */
inline folly::Optional<CollectionType> stringToType(const StringData* name) {
#define X(type) if (name->isame(s_##type.get())) return CollectionType::type;
COLLECTIONS_ALL_TYPES(X)
#undef X
  return folly::none;
}

inline folly::Optional<CollectionType> stringToType(const std::string& s) {
  return stringToType(
    req::ptr<StringData>::attach(StringData::Make(s)).get()
  );
}

inline bool isTypeName(const StringData* str) {
  return stringToType(str).hasValue();
}

/////////////////////////////////////////////////////////////////////////////
}}
#endif
