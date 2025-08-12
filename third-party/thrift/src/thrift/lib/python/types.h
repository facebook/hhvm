/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <Python.h>

#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <folly/container/F14Map.h>

#include <folly/Conv.h>
#include <folly/Optional.h>
#include <folly/python/error.h>

#include <thrift/lib/cpp2/protocol/TableBasedSerializer.h>

namespace apache::thrift::python {

inline PyObject* const* toPyObjectPtr(const void* objectPtr) {
  return static_cast<PyObject* const*>(objectPtr);
}

inline PyObject** toPyObjectPtr(void* objectPtr) {
  return static_cast<PyObject**>(objectPtr);
}

inline const PyObject* toPyObject(const void* object) {
  return static_cast<const PyObject*>(object);
}

inline PyObject* toPyObject(void* object) {
  return static_cast<PyObject*>(object);
}

#define THRIFT_PY3_STRINGIFY_LINE(LINE) #LINE
#define THRIFT_PY3_ERROR_LOCATION(LINE) \
  "Python error occurred at " __FILE__ ":" THRIFT_PY3_STRINGIFY_LINE(LINE) ": "
#define THRIFT_PY3_CHECK_ERROR() \
  folly::python::handlePythonError(THRIFT_PY3_ERROR_LOCATION(__LINE__))

#define THRIFT_PY3_CHECK_POSSIBLE_ERROR() \
  do {                                    \
    if (PyErr_Occurred()) {               \
      THRIFT_PY3_CHECK_ERROR();           \
    }                                     \
  } while (false)

/**
 * Creates a new "union tuple" initialized for an empty union.
 *
 * The returned tuple has 2 items:
 *  1. The enum value of the field that is currently set for this union, or
 *     the special "empty" (with value 0) if the union is empty.
 *  2. The value of the current field for this enum, or Py_None if the union
 *     is empty.
 *
 * The tuple returned by this function always has values `(0, Py_None)`.
 */
PyObject* createUnionTuple();

PyObject* createMutableUnionDataHolder();

/***
 * Returns a new "struct tuple" whose field elements are uninitialized.
 *
 * The returned tuple has `numFields + 1` elements.
 *
 * The first element is a bytearray of `numFields` bytes, all of which are
 * initialized to 0. It is typically meant to be used as an array of isset
 * flags.
 *
 * The remaining `numFields` elements of the tuple are uninitialized.
 *
 * See also: `createStructTupleWithDefaultValues()`.
 */
PyObject* createStructTuple(int16_t numFields);

/**
 * Returns a new "struct list" whose field elements are uninitialized.
 *
 * There are function pairs such as *StructTuple* and *StructList*. These
 * functions offer similar functionalities. The key distinction is the type of
 * underlying container used: `StructTuple` uses a Python `tuple`, while
 * `StructList` uses a Python `list`. The use of a `list` in `StructList`
 * specific to mutable types, enabling modifications to the elements and
 * supporting deep-copy operations.
 */
PyObject* createStructList(int16_t numFields);

/**
 * Returns a new "struct tuple" associated with an immutable Thrift struct,
 * all elements initialized with default values.
 *
 * As in `createStructTuple()`, the first element of the tuple is a
 * 0-initialized bytearray with `numFields` bytes (to be used as isset flags).
 *
 * However, the remaining elements (1 through `numFields + 1`) are initialized
 * with the appropriate default value for the corresponding field (see below).
 * The order corresponds to the order of fields in the given `structInfo` (i.e.,
 * the insertion order, NOT the field ids).
 *
 * The default value for optional fields is always `Py_None`. For other fields,
 * the default value is either specified by the user or the following "standard"
 * value for the corresponding type:
 *   * `0L` for integral numbers.
 *   * `0d` for floating-point numbers.
 *   * `false` for booleans.
 *   * `""` (i.e., the empty string) for strings and `binary` fields (or an
 *      empty `IOBuf` if  applicable).
 *   * empty tuple for lists and maps.
 *   * empty `frozenset` for sets.
 *   * recursively default-initialized instance for structs and unions.
 *
 * All values in the returned tuple are stored in the "internal data"
 * representation (as opposed to the "Python value" representation - see the
 * various `*TypeInfo` Python classes). For example, `false` is actually
 * `Py_False`.
 */
PyObject* createImmutableStructTupleWithDefaultValues(
    const ::apache::thrift::detail::StructInfo& structInfo);

/**
 * Returns a new "struct list" associated with an mutable Thrift struct,
 * all elements initialized with default values.
 *
 * This function is very similar to its immutable counterpart. Please see the
 * `createImmutableStructTupleWithDefaultValues()` documentation for more
 * details.
 *
 * The following list only highlights the difference for the "standard" value
 * for the corresponding type:
 *   * In the mutable version, the standard value for lists is an empty `list`.
 */
PyObject* createMutableStructListWithDefaultValues(
    const ::apache::thrift::detail::StructInfo& structInfo);

/**
 * Sets the "isset" flag of the `index`-th field of the given struct tuple
 * `object` to the given `value`.
 *
 * @param structTuple Pointer to a "struct tuple" (see `createStructTuple()`
 *        above). This is assumed to be a `PyTupleObject`. The first element of
 *        the tuple contains the isset bytearray. If the bytearray is not
 *        properly initialized, or if the `index` is invalid (i.e., negative or
 *        greater than the number of fields), the behavior is undefined.
 *
 * @throws if unable to read a bytearray from the expected isset flags bytearray
 *         (see `object` param documentation above).
 */
void setStructIsset(PyObject* structTuple, int16_t index, bool value);

/**
 * Sets the "isset" flag of the `index`-th field of the given 'struct list'
 * `object` to the given `value`.
 *
 * Please see `createStructList()`.
 */
void setMutableStructIsset(PyObject* structList, int16_t index, bool value);

/*
 * Returns a new "struct tuple" with all its elements set to `None`
 * (i.e., `Py_None`).
 *
 * As in `createStructTuple()`, the first element of the tuple is a
 * 0-initialized bytearray with `numFields` bytes (to be used as isset flags).
 *
 * However, the remaining elements (1 through `numFields + 1`) are set to `None`
 *
 */
PyObject* createStructTupleWithNones(
    const ::apache::thrift::detail::StructInfo& structInfo);

/*
 * Returns a new "struct list" with all its elements set to `None`
 * (i.e., `Py_None`).
 *
 * Please see `createStructList()`.
 */
PyObject* createStructListWithNones(
    const ::apache::thrift::detail::StructInfo& structInfo);

/**
 * Populates unset fields of a immutable Thrift struct's "struct tuple" with
 * default values.
 *
 * The `object` should be a valid `tuple` created by `createStructTuple()`
 *
 * Iterates through the elements (from 1 to `numFields + 1`). If a field
 * is unset, it gets populated with the corresponding default value.
 *
 * Throws on error
 *
 */
void populateImmutableStructTupleUnsetFieldsWithDefaultValues(
    PyObject* object, const ::apache::thrift::detail::StructInfo& structInfo);

/**
 * Populates unset fields of a mutable Thrift struct's "struct list" with
 * default values.
 *
 * This function is very similar to its immutable counterpart. Please see the
 * `populateImmutableStructTupleUnsetFieldsWithDefaultValues()` documentation.
 * The only difference is the "standard" value for the corresponding type.
 *
 * See `getStandard{Mutable,Immutable}DefaultValueForType()` documentation.
 *
 * Throws on error
 */
void populateMutableStructListUnsetFieldsWithDefaultValues(
    PyObject* object, const ::apache::thrift::detail::StructInfo& structInfo);

/**
 * Resets the field at `index` of the "struct list" with the default value.
 *
 * Throws on error
 */
void resetFieldToStandardDefault(
    PyObject* structList,
    const ::apache::thrift::detail::StructInfo& structInfo,
    int index);

struct PyObjectDeleter {
  void operator()(PyObject* p) { Py_XDECREF(p); }
};

using UniquePyObjectPtr = std::unique_ptr<PyObject, PyObjectDeleter>;

/**
 * Sets the Python object pointed to by `objectPtr` to the given `value`
 * (releasing the previous one, if any).
 *
 * @params objectPtr double pointer to a `PyObject` (i.e., `PyObject**`).
 *
 * @return the newly set Python object pointer, i.e. the pointer previously held
 *         by the `value` parameter (and now pointed to by `objectPtr`).
 */
inline PyObject* setPyObject(void* objectPtr, UniquePyObjectPtr value) {
  PyObject** pyObjPtr = toPyObjectPtr(objectPtr);
  PyObject* oldObject = *pyObjPtr;
  *pyObjPtr = value.release();
  Py_XDECREF(oldObject);
  return *pyObjPtr;
}

extern const ::apache::thrift::detail::TypeInfo& boolTypeInfo;
extern const ::apache::thrift::detail::TypeInfo& byteTypeInfo;
extern const ::apache::thrift::detail::TypeInfo& i16TypeInfo;
extern const ::apache::thrift::detail::TypeInfo& i32TypeInfo;
extern const ::apache::thrift::detail::TypeInfo& i64TypeInfo;
extern const ::apache::thrift::detail::TypeInfo& doubleTypeInfo;
extern const ::apache::thrift::detail::TypeInfo& floatTypeInfo;
extern const ::apache::thrift::detail::TypeInfo stringTypeInfo;
extern const ::apache::thrift::detail::TypeInfo binaryTypeInfo;
extern const ::apache::thrift::detail::TypeInfo iobufTypeInfo;

::apache::thrift::detail::OptionalThriftValue getStruct(
    const void* objectPtr,
    const ::apache::thrift::detail::TypeInfo& /* typeInfo */);

inline void* setContainer(void* objectPtr) {
  if (!setPyObject(objectPtr, UniquePyObjectPtr{PyTuple_New(0)})) {
    THRIFT_PY3_CHECK_ERROR();
  }
  return objectPtr;
}

inline void* setFrozenSet(void* objectPtr) {
  if (!setPyObject(objectPtr, UniquePyObjectPtr{PyFrozenSet_New(nullptr)})) {
    THRIFT_PY3_CHECK_ERROR();
  }
  return objectPtr;
}

/**
 * Sets the Python object pointed to by `object` to an empty Python `list`
 * (Releases a strong reference to the previous object, if there was one).
 *
 * @param objectPtr A double pointer to a `PyObject` (i.e., `PyObject**`).
 *
 * @return The newly set Python object pointer that points to an empty Python
 * `list`.
 */
inline void* setList(void* objectPtr) {
  if (!setPyObject(objectPtr, UniquePyObjectPtr{PyList_New(0)})) {
    THRIFT_PY3_CHECK_ERROR();
  }
  return objectPtr;
}

// Sets the Python object pointed to by `objectPtr` to an empty Python `set`
inline void* setMutableSet(void* objectPtr) {
  if (!setPyObject(objectPtr, UniquePyObjectPtr{PySet_New(nullptr)})) {
    THRIFT_PY3_CHECK_ERROR();
  }
  return objectPtr;
}

// Sets the Python object pointed to by `objectPtr` to an empty Python
// `dictionary`
inline void* setMutableMap(void* objectPtr) {
  if (!setPyObject(objectPtr, UniquePyObjectPtr{PyDict_New()})) {
    THRIFT_PY3_CHECK_ERROR();
  }
  return objectPtr;
}

class ListTypeInfo {
 public:
  static std::uint32_t size(const void* object) {
    return folly::to<std::uint32_t>(PyTuple_GET_SIZE(toPyObject(object)));
  }

  static void clear(void* object) { setContainer(object); }

  static void read(
      const void* context,
      void* objectPtr,
      std::uint32_t listSize,
      void (*reader)(const void* /*context*/, void* /*val*/));
  static size_t write(
      const void* context,
      const void* object,
      size_t (*writer)(const void* /*context*/, const void* /*val*/));

  static void consumeElem(
      const void* context,
      void* object,
      void (*reader)(const void* /*context*/, void* /*val*/));

  explicit ListTypeInfo(const ::apache::thrift::detail::TypeInfo* valInfo)
      : ext_{
            /* .valInfo */ valInfo,
            /* .size */ size,
            /* .clear */ clear,
            /* .consumeElem */ consumeElem,
            /* .readList */ read,
            /* .writeList */ write,
        },
        typeinfo_{
            protocol::TType::T_LIST,
            getStruct,
            reinterpret_cast<::apache::thrift::detail::VoidPtrFuncPtr>(
                setContainer),
            &ext_,
        } {}
  inline const ::apache::thrift::detail::TypeInfo* get() const {
    return &typeinfo_;
  }

 private:
  const ::apache::thrift::detail::ListFieldExt ext_;
  const ::apache::thrift::detail::TypeInfo typeinfo_;
};

/**
 * `MutableListTypeInfo` is a counterpart to `ListTypeInfo`, specifically
 * tailored for mutable Thrift struct. They differ in their internal data
 * representation. `MutableListTypeInfo` uses a Python `list` for mutability,
 * whereas its counterpart, `ListTypeInfo`, uses a Python `tuple`.
 */
class MutableListTypeInfo {
 public:
  static std::uint32_t size(const void* object) {
    return folly::to<std::uint32_t>(PyList_GET_SIZE(toPyObject(object)));
  }

  static void clear(void* object) { setList(object); }

  static void read(
      const void* context,
      void* objectPtr,
      std::uint32_t listSize,
      void (*reader)(const void* /*context*/, void* /*val*/));

  static size_t write(
      const void* context,
      const void* object,
      size_t (*writer)(const void* /*context*/, const void* /*val*/));

  static void consumeElem(
      const void* context,
      void* object,
      void (*reader)(const void* /*context*/, void* /*val*/));

  explicit MutableListTypeInfo(
      const ::apache::thrift::detail::TypeInfo* valInfo)
      : ext_{
            /* .valInfo */ valInfo,
            /* .size */ size,
            /* .clear */ clear,
            /* .consumeElem */ consumeElem,
            /* .readList */ read,
            /* .writeList */ write,
        },
        typeinfo_{
            protocol::TType::T_LIST,
            getStruct,
            reinterpret_cast<::apache::thrift::detail::VoidPtrFuncPtr>(setList),
            &ext_,
        } {}

  inline const ::apache::thrift::detail::TypeInfo* get() const {
    return &typeinfo_;
  }

 private:
  const ::apache::thrift::detail::ListFieldExt ext_;
  const ::apache::thrift::detail::TypeInfo typeinfo_;
};

/**
 * An abstract base class for set types so that the cython SetTypeInfo class
 * can be used for both unsorted and sorted sets. Also used for
 * MutableSetTypeInfo.
 */
class SetTypeInfoBase {
 public:
  virtual const ::apache::thrift::detail::TypeInfo* get() const = 0;
  virtual ::std::unique_ptr<SetTypeInfoBase> asKeySorted() const = 0;
  virtual ~SetTypeInfoBase() = default;
};

/**
 * Given owned reference UniquePyObjectPtr set, an empty `set`, read the
 * elements using context and reader, adding the read elements to `set`, and set
 * objectPtr to the read `set`.
 */
void SetTypeInfoTemplate_read(
    UniquePyObjectPtr set,
    const void* context,
    void* objectPtr,
    std::uint32_t setSize,
    void (*reader)(const void* /*context*/, void* /*val*/));

/**
 * Given PyObject* `set` (may be a frozenset), sort the elements and return
 * UniquePyObjectPtr (owned reference) to iterator over the sorted elements
 */
UniquePyObjectPtr SetTypeInfo_sortElem(PyObject* set);

/**
 * Given UniquePyObjectPtr `iter`, an owned reference over set's elements, write
 * the elements using `context` and `writer`. Returns the written size.
 */
size_t SetTypeInfo_write(
    UniquePyObjectPtr iter,
    const void* context,
    size_t (*writer)(const void* /*context*/, const void* /*val*/));

void SetTypeInfo_consumeElem(
    const void* context,
    void* objectPtr,
    void (*reader)(const void* /*context*/, void* /*val*/));

/**
 * An implementation of SetTypeInfoBase. Implements functions required by the
 * `SetFieldExt` interface of the table-based serializer. All implementations
 * are static methods to avoid lifetime issues and comply with function
 * signatures expected by `SetFieldExt` interface.
 *
 * The `Handler` parameter represents cpython operations for either mutable or
 * immutable sets, based on their respective internal data types.
 *
 * The `KeySorted` parameter determines whether the set elements are sorted
 * during `write` (serialize). The default is `false`. Do not use this without
 * contacting the Thrift team. It may be removed after migrating from
 * thrift-py3.
 */
template <typename Handler, bool KeySorted>
class SetTypeInfoImpl final : public SetTypeInfoBase {
 public:
  static std::uint32_t size(const void* object) {
    return folly::to<std::uint32_t>(PySet_GET_SIZE(toPyObject(object)));
  }

  static void clear(void* object) { Handler::clear(object); }

  static void read(
      const void* context,
      void* objectPtr,
      std::uint32_t setSize,
      void (*reader)(const void* /*context*/, void* /*val*/)) {
    UniquePyObjectPtr set{Handler::create(nullptr)};
    SetTypeInfoTemplate_read(
        std::move(set), context, objectPtr, setSize, reader);
  }

  static size_t write(
      const void* context,
      const void* object,
      bool protocolSortKeys,
      size_t (*writer)(const void* /*context*/, const void* /*val*/)) {
    PyObject* set = const_cast<PyObject*>(toPyObject(object));
    const bool sortKeys = protocolSortKeys || KeySorted;
    UniquePyObjectPtr iter = UNLIKELY(sortKeys)
        ? SetTypeInfo_sortElem(set)
        : UniquePyObjectPtr{PyObject_GetIter(set)};

    return SetTypeInfo_write(std::move(iter), context, writer);
  }

  static void consumeElem(
      const void* context,
      void* objectPtr,
      void (*reader)(const void* /*context*/, void* /*val*/)) {
    return SetTypeInfo_consumeElem(context, objectPtr, reader);
  }

  explicit SetTypeInfoImpl(const ::apache::thrift::detail::TypeInfo* valInfo)
      : ext_{
            /* .valInfo */ valInfo,
            /* .size */ size,
            /* .clear */ clear,
            /* .consumeElem */ consumeElem,
            /* .readSet */ read,
            /* .writeSet */ write,
        },
        typeinfo_{
            protocol::TType::T_SET,
            getStruct,
            reinterpret_cast<::apache::thrift::detail::VoidPtrFuncPtr>(
                Handler::clear),
            &ext_,
        } {}
  const ::apache::thrift::detail::TypeInfo* get() const override {
    return &typeinfo_;
  }

  std::unique_ptr<SetTypeInfoBase> asKeySorted() const override {
    return std::make_unique<SetTypeInfoImpl<Handler, true>>(ext_.valInfo);
  }

 private:
  const ::apache::thrift::detail::SetFieldExt ext_;
  const ::apache::thrift::detail::TypeInfo typeinfo_;
};

/**
 * This class implements `frozenset` handler methods for immutable
 * thrift-python, for the `SetTypeInfoImpl` class.
 *
 * Immutable Thrift structs use Python's `frozenset` for internal data
 * representation. The `ImmutableSetHandler` class provides methods to create
 * and clear the set.
 */
struct ImmutableSetHandler {
  static PyObject* create(PyObject* iterable) {
    return PyFrozenSet_New(iterable);
  }
  static void* clear(void* object) { return setFrozenSet(object); }
};

using SetTypeInfo = SetTypeInfoImpl<ImmutableSetHandler, false>;

/**
 * This class implements `set` handler methods for mutable thrift-python,
 * for the `SetTypeInfoImpl` class.
 *
 * Mutable Thrift structs use Python's `set` for internal data
 * representation. The `MutableSetHandler` class provides methods to create
 * and clear the set.
 */
struct MutableSetHandler {
  static PyObject* create(PyObject* iterable) { return PySet_New(iterable); }
  static void* clear(void* object) { return setMutableSet(object); }
};

using MutableSetTypeInfo = SetTypeInfoImpl<MutableSetHandler, false>;

/**
 * This helper method sorts map keys and writes them to wire format. It may be
 * called for both mutable and immutable maps.
 */
size_t writeMapSorted(
    const void* context,
    const void* object,
    UniquePyObjectPtr (*toItemList)(PyObject* dict),
    size_t (*writer)(
        const void* context, const void* keyElem, const void* valueElem));

/**
 * An abstract base class for map types so that the cython MapTypeInfo class
 * can be used for both unsorted and key-sorted maps. Also used for
 * MutableMapTypeInfo.
 */
class MapTypeInfoBase {
 public:
  virtual const ::apache::thrift::detail::TypeInfo* get() const = 0;
  virtual ::std::unique_ptr<MapTypeInfoBase> asKeySorted() const = 0;
  virtual ~MapTypeInfoBase() = default;
};

/**
 * An implementation of MapTypeInfoBase. Implements functions required by the
 * `MapFieldExt` interface of the table-based serializer. All implementations
 * are static methods to avoid lifetime issues and comply with function
 * signatures expected by `MapFieldExt` interface.
 *
 * The `Handler` parameter represents cpython operations for either mutable or
 * immutable maps, based on their respective internal data types.
 *
 * The `KeySorted` parameter determines whether the map is sorted by key
 * during `write` (serialize). The default is `false`. Do not use this without
 * contacting the Thrift team. It may be removed after migrating from
 * thrift-py3.
 */
template <typename Handler, bool KeySorted>
class MapTypeInfoImpl final : public MapTypeInfoBase {
 public:
  static std::uint32_t size(const void* object) {
    return folly::to<std::uint32_t>(Handler::size(toPyObject(object)));
  }

  static void clear(void* object) { Handler::clear(object); }

  static void read(
      const void* context,
      void* objectPtr,
      std::uint32_t mapSize,
      void (*keyReader)(const void* context, void* key),
      void (*valueReader)(const void* context, void* val)) {
    Handler::read(context, objectPtr, mapSize, keyReader, valueReader);
  }

  static size_t write(
      const void* context,
      const void* object,
      bool protocolSortKeys,
      size_t (*writer)(
          const void* context, const void* keyElem, const void* valueElem)) {
    if (UNLIKELY(protocolSortKeys) || KeySorted) {
      return writeMapSorted(context, object, Handler::toItemList, writer);
    }
    return Handler::writeUnsorted(context, object, writer);
  }

  static void consumeElem(
      const void* context,
      void* object,
      void (*keyReader)(const void* /*context*/, void* /*val*/),
      void (*valueReader)(const void* /*context*/, void* /*val*/)) {
    Handler::consumeElem(context, object, keyReader, valueReader);
  }

  explicit MapTypeInfoImpl(
      const ::apache::thrift::detail::TypeInfo* keyInfo,
      const ::apache::thrift::detail::TypeInfo* valInfo)
      : ext_{
            /* .keyInfo */ keyInfo,
            /* .valInfo */ valInfo,
            /* .size */ size,
            /* .clear */ clear,
            /* .consumeElem */ consumeElem,
            /* .readMap */ read,
            /* .writeMap */ write,
        },
        typeinfo_{
            protocol::TType::T_MAP,
            getStruct,
            reinterpret_cast<::apache::thrift::detail::VoidPtrFuncPtr>(
                Handler::clear),
            &ext_,
        } {}

  const ::apache::thrift::detail::TypeInfo* get() const override {
    return &typeinfo_;
  }

  std::unique_ptr<MapTypeInfoBase> asKeySorted() const override {
    return std::make_unique<MapTypeInfoImpl<Handler, true>>(
        ext_.keyInfo, ext_.valInfo);
  }

 private:
  const ::apache::thrift::detail::MapFieldExt ext_;
  const ::apache::thrift::detail::TypeInfo typeinfo_;
};

class ImmutableMapHandler {
 public:
  static int64_t size(const PyObject* object) {
    return PyTuple_GET_SIZE(object);
  }

  // Result is used by writeMapSorted as input to PySequence_list
  // Since immutable map representation is tuple, this is a no-op.
  // Since maps can't have duplicate keys, the value (second value in tuple)
  // will never be used for comparison, only the first value of each.
  static UniquePyObjectPtr toItemList(PyObject* dictTuple) {
    // copy the tuple of key-value pairs into list of key-value pairs
    PyObject* list = PySequence_List(dictTuple);
    if (!list) {
      THRIFT_PY3_CHECK_ERROR();
    }
    // the UniquePyObjectPtr will free the list when done
    return UniquePyObjectPtr(list);
  }

  static void* clear(void* object) { return setContainer(object); }

  static void read(
      const void* context,
      void* objectPtr,
      std::uint32_t mapSize,
      void (*keyReader)(const void* context, void* key),
      void (*valueReader)(const void* context, void* val));

  static size_t writeUnsorted(
      const void* context,
      const void* object,
      size_t (*writer)(
          const void* context, const void* keyElem, const void* valueElem));

  static void consumeElem(
      const void* context,
      void* object,
      void (*keyReader)(const void* /*context*/, void* /*val*/),
      void (*valueReader)(const void* /*context*/, void* /*val*/));
};

using MapTypeInfo = MapTypeInfoImpl<ImmutableMapHandler, false>;

class MutableMapHandler {
 public:
  static int64_t size(const PyObject* object) {
    return PyDict_Size(const_cast<PyObject*>(object));
  }

  // Returns sortable list of key-value pairs from dict
  static UniquePyObjectPtr toItemList(PyObject* dict) {
    PyObject* itemList = PyDict_Items(dict);
    if (itemList == nullptr) {
      THRIFT_PY3_CHECK_ERROR();
    }
    return UniquePyObjectPtr(itemList);
  }

  static void* clear(void* objectPtr) { return setMutableMap(objectPtr); }

  /**
   * Deserializes a dict (with `mapSize` key/value pairs) into `objectPtr`.
   *
   * The function pointer arguments (`keyReader` and `valueReader`) are expected
   * to take two (type-erased, i.e. `void*`) arguments:
   *   1. The given `context`
   *   2. A pointer to a `PyObject*`, that should be set by the function to the
   *      (next) key and value, respectively.
   *
   * @param context Will be passed to every call to `keyReader` and
   *        `valueReader`.
   * @param objectPtr Pointer to a `PyObject*`, that will be set to a new
   *        `PyDict` instance containing `mapSize` elements whose keys and
   *        values are obtained by calling the given `*Reader` functions.
   */
  static void read(
      const void* context,
      void* objectPtr,
      std::uint32_t mapSize,
      void (*keyReader)(const void* context, void* key),
      void (*valueReader)(const void* context, void* val));

  /**
   * Serializes the dict given in `object`.
   *
   * The `writer` function will be called for every item in the dict pointed by
   * the given `object`. It is expected to take the arguments described below,
   * and return the number of bytes written:
   *   1. The given `context`
   *   2 & 3. `PyObject**` pointing to the next key and value from dict,
   * respectively.
   *
   * @param context Passed to `writer` on every call.
   * @param object Input `PyObject*`, holds the `PyDict` to serialize.
   *
   * @return Total number of bytes written.
   */
  static size_t writeUnsorted(
      const void* context,
      const void* object,
      size_t (*writer)(
          const void* context, const void* keyElem, const void* valueElem));

  /**
   * Deserializes a single key, value pair (using the given function pointers)
   * and adds it to the given dict.
   *
   * See `read()` for the expectations on `keyReader` and `valueReader`.
   *
   * @param context Passed to `keyReader` and `valueReader` on every call.
   * @param objectPtr Pointer to a `PyObject*` that holds a dict, to which the
   *        new key/value pair will be added.
   */
  static void consumeElem(
      const void* context,
      void* object,
      void (*keyReader)(const void* /*context*/, void* /*val*/),
      void (*valueReader)(const void* /*context*/, void* /*val*/));
};

using MutableMapTypeInfo = MapTypeInfoImpl<MutableMapHandler, false>;

using FieldValueMap = folly::F14FastMap<int16_t, PyObject*>;

/**
 * Holds the information required to (de)serialize a thrift-python structured
 * type (i.e., struct, union or exception).
 */
class DynamicStructInfo {
 public:
  DynamicStructInfo(
      const char* name, int16_t numFields, bool isUnion, bool isMutable);

  // DynamicStructInfo is non-copyable
  DynamicStructInfo(const DynamicStructInfo&) = delete;
  DynamicStructInfo& operator=(const DynamicStructInfo&) = delete;
  DynamicStructInfo(DynamicStructInfo&&) = delete;
  DynamicStructInfo& operator=(DynamicStructInfo&&) = delete;

  ~DynamicStructInfo();

  /**
   * Returns the underlying (table-based) serializer `StructInfo`, populated
   * with all field infos and values added via the `addField*()` methods.
   */
  const ::apache::thrift::detail::StructInfo& getStructInfo() const {
    return *tableBasedSerializerStructInfo_;
  }

  /**
   * Adds information for a new field to the underlying (table-based) serializer
   * `StructInfo`.
   *
   * The order of `FieldInfo` in the underlying `StructInfo` corresponds to the
   * order in which this method is called. Calling this method more than the
   * `numFields` value specified at construction time results in undefined
   * behavior.
   */
  void addFieldInfo(
      ::apache::thrift::detail::FieldID id,
      ::apache::thrift::detail::FieldQualifier qualifier,
      const char* name,
      const ::apache::thrift::detail::TypeInfo* typeInfo);

  void addMutableFieldInfo(
      ::apache::thrift::detail::FieldID id,
      ::apache::thrift::detail::FieldQualifier qualifier,
      const char* name,
      const ::apache::thrift::detail::TypeInfo* typeInfo);

  // DO_BEFORE(aristidis,20240729): Rename to set(Default?)FieldValue.
  /**
   * Sets the value for the field with the given `index`.
   *
   * The given `fieldValue` will be included in the underlying
   * `StructInfo::customExt`, at the given `index`. It is expected to be in the
   * "internal data" representation (see `to_internal_data()` methods in the
   * various `*TypeInfo` classes in Python).
   *
   * If `fieldValue` is `nullptr`, behavior is undefined.
   */
  void addFieldValue(int16_t index, PyObject* fieldValue);

  bool isUnion() const {
    return tableBasedSerializerStructInfo_->unionExt != nullptr;
  }

 private:
  /**
   * Name of this Thrift type (struct/union/exception).
   */
  std::string name_;

  /**
   * Default values (if any) for each field (indexed by field position in
   * `fieldNames_`).
   *
   * If present, values are in the "internal data" representation (as opposed to
   * the "Python value" representation - see the various `*TypeInfo` Python
   * classes).
   */
  FieldValueMap fieldValues_;

  /**
   * Pointer to a region of memory that holds a (table-based) serializer
   * `StructInfo` followed by `numFields` instances of `FieldInfo` (accessible
   * through `structInfo->fieldInfos`).
   *
   * ------------------------------------------------
   * | StructInfo    |  FieldInfo | ... | FieldInfo |
   * ------------------------------------------------
   * ^                      (numFields times)
   *
   * The `customExt` field of this StructInfo points to `fieldValues_`.
   */
  ::apache::thrift::detail::StructInfo* tableBasedSerializerStructInfo_;

  /**
   * Names of the fields added via `addFieldInfo()`, in the order they were
   * added.
   *
   * The same order is used for the corresponding `FieldInfo` entries in
   * `tableBasedSerializerStructInfo_`.
   */
  std::vector<std::string> fieldNames_;
};

::apache::thrift::detail::TypeInfo createImmutableStructTypeInfo(
    const DynamicStructInfo& dynamicStructInfo);

::apache::thrift::detail::TypeInfo createMutableStructTypeInfo(
    const DynamicStructInfo& dynamicStructInfo);

/**
 * Retrieves the start address of the array that holds pointers to the elements
 * in a `PyListObject`.
 */
inline PyObject* getListObjectItemBase(void* pyList) {
  return reinterpret_cast<PyObject*>(&PyList_GET_ITEM(pyList, 0));
}
inline const PyObject* getListObjectItemBase(const void* pyList) {
  return reinterpret_cast<PyObject*>(&PyList_GET_ITEM(pyList, 0));
}

/**
 * Returns the appropriate standard immutable default value for the given
 * `typeInfo`.
 *
 * The standard default values are as follows:
 *   * `0L` for integral numbers.
 *   * `0d` for floating-point numbers.
 *   * `false` for booleans.
 *   * `""` (i.e., the empty string) for strings and `binary` fields (or an
 *      empty `IOBuf` if applicable).
 *   * An empty `tuple` for lists and maps.
 *   * An empty `frozenset` for sets.
 *   * A recursively default-initialized instance for structs and unions.
 *
 * @throws if there is no standard default value
 */
PyObject* getStandardImmutableDefaultValuePtrForType(
    const ::apache::thrift::detail::TypeInfo& typeInfo);

/**
 * Returns the appropriate standard mutable default value for the given
 * `typeInfo`.
 *
 * The standard default values are as follows:
 *   * `0L` for integral numbers.
 *   * `0d` for floating-point numbers.
 *   * `false` for booleans.
 *   * `""` (i.e., the empty string) for strings and `binary` fields (or an
 *      empty `IOBuf` if applicable).
 *   * An empty `list` for lists.
 *   * An empty `dict` for maps.
 *   * An empty `set` for sets.
 *   * A recursively default-initialized instance for structs and unions.
 *
 * @throws if there is no standard default value
 */
PyObject* getStandardMutableDefaultValuePtrForType(
    const ::apache::thrift::detail::TypeInfo& typeInfo);

/*
 * Python introduced structural pattern matching. A type is treated as a
 * sequence if it has the Py_TPFLAGS_SEQUENCE flag, and as a mapping if it has
 * the Py_TPFLAGS_MAPPING flag. For regular classes, you can register them with
 * collections.abc.Sequence or collections.abc.Mapping to enable this behavior.
 * However, this doesn't work for Cython extension types. Cython implemented
 * decorators like cython.collection_type("sequence") or
 * cython.collection_type("mapping"), but these decorators are not publicly
 * available yet. The functions below are helper functions to set the flags to
 * enable structural pattern matching.
 */
void tag_object_as_sequence(PyTypeObject* type_object);
void tag_object_as_mapping(PyTypeObject* type_object);

/**
 * A helper function that teaches Cython how to up-cast a unique_ptr<Child> to
 * unique_ptr<Base>. Will fail C++ compilation if Child does not extend Base.
 */
template <typename Base, typename Child, typename... Args>
std::unique_ptr<Base> make_unique_base(Args&&... args) {
  return std::make_unique<Child>(std::forward<Args>(args)...);
}

namespace capi {
/**
 * Retrieves internal _fbthrift_data from `StructOrUnion`. On import failure,
 * returns nullptr. Caller is responsible for clearing Err indicator on failure.
 * Do not use externally; use capi interface instead.
 */
PyObject* FOLLY_NULLABLE getThriftData(PyObject* structOrUnion);
/**
 * Retrieves internal _fbthrift_data from `GeneratedError`. On import failure,
 * returns nullptr. Caller is responsible for clearing Err indicator on failure.
 * Do not use externally; use capi interface instead.
 */
PyObject* FOLLY_NULLABLE getExceptionThriftData(PyObject* generatedError);

/**
 * Sets index + 1 field of struct_tuple to `value` and records that is set
 * in the isset array at field 0. Returns 0 on success and -1 on failure.
 * Only for use with fresh struct_tuple (i.e., no set field values)
 */
int setStructField(PyObject* struct_tuple, int16_t index, PyObject* value);
/**
 * Creates a union tuple of size 2.
 * Sets index 0 of union_tuple to python int created from type_key
 * Sets index 1 of union_tuple to value.
 * Returns nullptr on failure, union tuple on success.
 */
PyObject* unionTupleFromValue(int64_t type_key, PyObject* value);

} // namespace capi
} // namespace apache::thrift::python
