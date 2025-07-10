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

#include <thrift/lib/python/types.h>

// To access Cython C APIs, such as create_IOBuf(). See `types.pxd`.
#include <thrift/lib/python/types_api.h> // @manual

#include <folly/Indestructible.h>
#include <folly/Range.h>
#include <folly/container/F14Map.h>
#include <folly/io/IOBuf.h>
#include <folly/lang/New.h>
#include <folly/python/import.h>
#include <thrift/lib/cpp2/protocol/TableBasedSerializer.h>

namespace apache::thrift::python {

constexpr const size_t kHeadOffset = sizeof(PyVarObject);
constexpr const size_t kFieldOffset = sizeof(PyObject*);

/**
 * In this module, some of the functions have two versions: one for 'Mutable'
 * and one for "Immutable" in the name. There are slight differences between
 * these versions. For instance, when dealing with standard default value
 * related functions, immutable types utilize Python `tuple` for a Thrift list,
 * whereas mutable types utilize Python `list`.
 */

namespace {

/***
 * Imports the thrift python types module, and returns true iff it succeeded.
 */
bool ensure_module_imported() {
  static ::folly::python::import_cache_nocapture import(
      ::import_thrift__python__types);
  return import();
}

/**
 * Throws an exception if the thrift python types module could not be imported.
 */
void ensureImportOrThrow() {
  if (!ensure_module_imported()) {
    throw std::runtime_error("import_thrift__python__types failed");
  }
}

enum class PythonVariant { Immutable, Mutable };

/**
 * Which types must be copied in mutable python for IDL custom defaults
 */
bool requiresDeepCopy(const detail::TypeInfo& typeInfo) {
  switch (typeInfo.type) {
    case protocol::TType::T_BOOL:
    case protocol::TType::T_BYTE:
    case protocol::TType::T_I16:
    case protocol::TType::T_I32:
    case protocol::TType::T_I64:
    case protocol::TType::T_DOUBLE:
    case protocol::TType::T_FLOAT:
      // these are immutable
      return false;
    case protocol::TType::T_STRING:
      switch (*static_cast<const detail::StringFieldType*>(typeInfo.typeExt)) {
        case detail::StringFieldType::String:
        case detail::StringFieldType::StringView:
        case detail::StringFieldType::Binary:
        case detail::StringFieldType::BinaryStringView:
          // str and bytes are immutable
          return false;
        case detail::StringFieldType::IOBuf:
        case detail::StringFieldType::IOBufPtr:
        case detail::StringFieldType::IOBufObj:
          // IOBuf is not immutable
          return true;
      }
    case protocol::TType::T_LIST:
    case protocol::TType::T_MAP:
    case protocol::TType::T_SET:
    case protocol::TType::T_STRUCT:
      // all this jank is mutable
      return true;
    default:
      LOG(FATAL) << "invalid typeInfo TType " << typeInfo.type;
  }
}

/**
 * Returns a *new* owned reference to a PyObject* if generated.
 * If a type is uncached, returns std::nullopt.

 * This function should be used for types that are *never* cached because they
 * are mutatable from user code. In immutable Python, this only applies to
 * IOBuf only.
 */
std::optional<PyObject*> genUncachedDefaultValue(
    const detail::TypeInfo& typeInfo) {
  if (typeInfo.type != protocol::TType::T_STRING) {
    return std::nullopt;
  }
  switch (*static_cast<const detail::StringFieldType*>(typeInfo.typeExt)) {
    case detail::StringFieldType::String:
    case detail::StringFieldType::StringView:
    case detail::StringFieldType::Binary:
    case detail::StringFieldType::BinaryStringView:
      return std::nullopt;
    case detail::StringFieldType::IOBuf:
    case detail::StringFieldType::IOBufPtr:
    case detail::StringFieldType::IOBufObj: {
      // IOBuf should ***never*** be cached because not immutable
      PyObject* ptr = create_IOBuf(folly::IOBuf::create(0));
      if (ptr == nullptr) {
        THRIFT_PY3_CHECK_ERROR();
      }
      return ptr;
    }
  }
}

/**
 * Returns the standard default value for a thrift field of the given type.
 *
 * The returned value will either be the one provided by the user (in the Thrift
 * IDL), or the standard default value for the given `typeInfo`.
 *
 * @param `index` of the field in `userDefaultValues`, i.e. insertion order (NOT
 *        field ID).
 *
 * @throws if the thrift python types module could not be imported.
 */
UniquePyObjectPtr getDefaultValueForImmutableField(
    const detail::TypeInfo* typeInfo,
    const FieldValueMap& userDefaultValues,
    int16_t index) {
  ensureImportOrThrow();

  // 1. If the user explicitly provided a default value, use it.
  auto userDefaultValueIt = userDefaultValues.find(index);
  if (userDefaultValueIt != userDefaultValues.end()) {
    PyObject* value = userDefaultValueIt->second;
    Py_INCREF(value);
    return UniquePyObjectPtr(value);
  }

  // 2. Check if un-cacheable type (IOBuf)
  auto maybeDefault = genUncachedDefaultValue(*typeInfo);
  if (UNLIKELY(maybeDefault.has_value())) {
    return UniquePyObjectPtr(*maybeDefault);
  }

  // 3. Check local cache for an existing default value.
  static folly::Indestructible<
      folly::F14FastMap<const detail::TypeInfo*, PyObject*>>
      defaultValueCache;
  auto cachedDefaultValueIt = defaultValueCache->find(typeInfo);
  if (cachedDefaultValueIt != defaultValueCache->end()) {
    UniquePyObjectPtr value(cachedDefaultValueIt->second);
    Py_INCREF(value.get());
    return value;
  }

  // 4. No cached value found. Determine the default value, and update cache (if
  // applicable).
  auto value = getStandardImmutableDefaultValuePtrForType(*typeInfo);
  defaultValueCache->emplace(typeInfo, value);
  Py_INCREF(value);
  return UniquePyObjectPtr(value);
}

/**
 * Returns a deep copy of the given object. Since ListContainer acts as a
 * data holder policy for mutable types, a real deep copy is necessary assuming
 * the type holds at least one mutable field (struct, list, map, set, IOBuf)
 */
static PyObject* deepCopy(PyObject* p) {
  ensureImportOrThrow();
  return deepcopy(p);
}

/**
 * Returns the standard default value for a thrift field of the given type.
 *
 * The returned value will either be the one provided by the user (in the Thrift
 * IDL), or the standard default value for the given `typeInfo`.
 *
 * @param `index` of the field in `userDefaultValues`, i.e. insertion order (NOT
 *        field ID).
 *
 * @throws if the thrift python types module could not be imported.
 */
UniquePyObjectPtr getDefaultValueForMutableField(
    const detail::TypeInfo* typeInfo,
    const FieldValueMap& userDefaultValues,
    int16_t index) {
  ensureImportOrThrow();

  // 1. If the user explicitly provided a default value, use it.
  auto userDefaultValueIt = userDefaultValues.find(index);
  if (userDefaultValueIt != userDefaultValues.end()) {
    PyObject* value = userDefaultValueIt->second;
    if (requiresDeepCopy(*typeInfo)) {
      return UniquePyObjectPtr(deepCopy(value));
    } else {
      Py_INCREF(value);
      return UniquePyObjectPtr(value);
    }
  }

  // 2. For non-IDL default, just generate the default value. We can't
  // cache it because then it would require a deepcopy
  auto value = getStandardMutableDefaultValuePtrForType(*typeInfo);
  return UniquePyObjectPtr(value);
}

/*
 * A policy for handling Python tuples.
 */
struct TupleContainer final {
  /**
   * Return a new tuple object of given size, or NULL on failure.
   */
  static PyObject* New(Py_ssize_t size) { return PyTuple_New(size); }

  /**
   * Returns a new "struct tuple" whose field elements are uninitialized, see
   * `createStructTuple()` function.
   */
  static PyObject* createStructContainer(int16_t numFields) {
    return createStructTuple(numFields);
  }

  /**
   * Insert a reference to `object` at position `pos` of the `tuple`.
   */
  static void SET_ITEM(PyObject* tuple, Py_ssize_t pos, PyObject* object) {
    PyTuple_SET_ITEM(tuple, pos, object);
  }

  /**
   * Return the object at position `pos` in the `tuple`.
   * The returned reference is borrowed from the `tuple` (that is: it is only
   * valid as long as you hold a reference to `tuple`)
   */
  static PyObject* GET_ITEM(const PyObject* tuple, Py_ssize_t pos) {
    return PyTuple_GET_ITEM(tuple, pos);
  }

  /**
   * Return true if `p` is a tuple object or an instance of a subtype of the
   * tuple type.
   */
  static bool Check(const PyObject* p) { return PyTuple_Check(p); }

  static Py_ssize_t Size(PyObject* p) { return PyTuple_Size(p); }

  static UniquePyObjectPtr getDefaultValueForField(
      const detail::TypeInfo* typeInfo,
      const FieldValueMap& userDefaultValues,
      int16_t index) {
    return getDefaultValueForImmutableField(typeInfo, userDefaultValues, index);
  }
};

/*
 * A policy for handling Python lists.
 */
struct ListContainer final {
  /**
   * Return a new list object of given size, or NULL on failure.
   */
  static PyObject* New(Py_ssize_t size) { return PyList_New(size); }

  /**
   * Returns a new "struct list" whose field elements are uninitialized, see
   * `createStructList()` function.
   */
  static PyObject* createStructContainer(int16_t numFields) {
    return createStructList(numFields);
  }

  /**
   * Insert a reference to `object` at position `pos` of the `list`.
   */
  static void SET_ITEM(PyObject* list, Py_ssize_t pos, PyObject* object) {
    PyList_SET_ITEM(list, pos, object);
  }

  /**
   * Return the object at position `pos` in the `list`.
   * The returned reference is borrowed from the `list` (that is: it is only
   * valid as long as you hold a reference to `list`)
   */
  static PyObject* GET_ITEM(const PyObject* p, Py_ssize_t pos) {
    return PyList_GET_ITEM(p, pos);
  }

  /**
   * Return true if `p` is a list object or an instance of a subtype of the
   * list type.
   */
  static bool Check(const PyObject* p) { return PyList_Check(p); }

  static Py_ssize_t Size(PyObject* p) { return PyList_Size(p); }

  static UniquePyObjectPtr getDefaultValueForField(
      const detail::TypeInfo* typeInfo,
      const FieldValueMap& userDefaultValues,
      int16_t index) {
    return getDefaultValueForMutableField(typeInfo, userDefaultValues, index);
  }
};

/**
 * Returns pointer to a contiguous memory area of at least `numFields` bytes,
 * each one of which holds the "is set" flag of the corresponding field (in the
 * order of the corresponding `StructInfo.fieldInfos`).
 */
template <typename Container>
const char* getDataHolderIssetFlags(const PyObject* structDataHolder) {
  PyObject* isset = Container::GET_ITEM(structDataHolder, 0);
  const char* issetFlags = PyBytes_AsString(isset);
  if (issetFlags == nullptr) {
    THRIFT_PY3_CHECK_ERROR();
  }
  return issetFlags;
}

/**
 * Returns a new "struct container" with all its elements initialized.
 *
 * As in `createStructContainer()`, the first element of the tuple is a
 * 0-initialized bytearray with `numFields` bytes (to be used as isset flags).
 *
 * However, the remaining elements (1 through `numFields + 1`) are initialized
 * with the appropriate default value for the corresponding field (see below).
 * The order corresponds to the order of fields in the given `structInfo`
 * (i.e., the insertion order, NOT the field ids).
 *
 * The default value for optional fields is always `Py_None`. For other fields,
 * the default value is either specified by the user or the "standard" value
 * for the corresponding type. This is identified by the function parameter
 * `getStandardDefaultValueForTypeFunc`.
 *
 * For more information, see `getStandardImmutableDefaultValueForType()` and
 * `getStandardMutableDefaultValueType()` functions, which are potential values
 * for the `getStandardDefaultValueForTypeFunc` parameter.
 */
template <typename Container>
PyObject* createStructContainerWithDefaultValues(
    const detail::StructInfo& structInfo) {
  const int16_t numFields = structInfo.numFields;
  UniquePyObjectPtr container{Container::createStructContainer(numFields)};
  if (container == nullptr) {
    THRIFT_PY3_CHECK_ERROR();
  }

  // Initialize container[1:numFields+1] with default field values.
  const FieldValueMap& defaultValues =
      *static_cast<const FieldValueMap*>(structInfo.customExt);
  for (int fieldIndex = 0; fieldIndex < numFields; ++fieldIndex) {
    const detail::FieldInfo& fieldInfo = structInfo.fieldInfos[fieldIndex];
    if (fieldInfo.qualifier == detail::FieldQualifier::Optional) {
      Container::SET_ITEM(container.get(), fieldIndex + 1, Py_None);
      Py_INCREF(Py_None);
    } else {
      Container::SET_ITEM(
          container.get(),
          fieldIndex + 1,
          Container::getDefaultValueForField(
              fieldInfo.typeInfo, defaultValues, fieldIndex)
              .release());
    }
  }

  // The policy determines the actual deep copy operation; it performs a no-op
  // for immutable types.
  return container.release();
}

/**
 * Returns a new "struct list" with all its elements initialized.
 */
PyObject* createStructListWithDefaultValues(
    const detail::StructInfo& structInfo) {
  return createStructContainerWithDefaultValues<ListContainer>(structInfo);
}

/**
 * Returns a new "struct tuple" with all its elements initialized.
 */
PyObject* createStructTupleWithDefaultValues(
    const detail::StructInfo& structInfo) {
  return createStructContainerWithDefaultValues<TupleContainer>(structInfo);
}

/**
 * Returns the appropriate standard default value for the given `typeInfo`,
 * along with a boolean indicating whether it should be added to the cache.
 *
 * The standard default values are as follows:
 *   * `0L` for integral numbers.
 *   * `0d` for floating-point numbers.
 *   * `false` for booleans.
 *   * `""` (i.e., the empty string) for strings and `binary` fields (or an
 *      empty `IOBuf` if applicable).
 *
 * The default value for container types change based on whether they are
 * mutable or not:
 *
 * Mutable:
 *    * An empty `list` for lists.
 *    * An empty `dict` for maps.
 *    * An empty `set` for sets.
 *    * A recursively default-initialized instance for structs and unions.
 *  Immutable:
 *    * An empty `tuple` for lists and maps.
 *    * An empty `frozenset` for sets.
 *    * A recursively default-initialized instance for structs and unions.
 *
 * @throws if there is no standard default value
 */
template <PythonVariant Variant>
UniquePyObjectPtr getStandardDefaultValueForType(
    const detail::TypeInfo& typeInfo) {
  PyObject* ptr = nullptr;

  // Immutable types use a default-value cache and initialize the fields with
  // the same cached Python object repeatedly. However, this approach does not
  // work well with mutable types when there is no copy-on-write mechanism in
  // place. The issue is particularly problematic with container types and
  // structs/unions. However, since we deep copy the default values for mutable
  // types above the cache layer, it is fine to keep `addValueToCache = true`
  // even for containers.
  switch (typeInfo.type) {
    case protocol::TType::T_BYTE:
    case protocol::TType::T_I16:
    case protocol::TType::T_I32:
    case protocol::TType::T_I64:
      // For integral values, the default is `0L`.
      ptr = PyLong_FromLong(0);
      break;
    case protocol::TType::T_DOUBLE:
    case protocol::TType::T_FLOAT:
      // For floating point values, the default is `0d`.
      ptr = PyFloat_FromDouble(0);
      break;
    case protocol::TType::T_BOOL:
      // For booleans, the default is `false`.
      ptr = Py_False;
      Py_INCREF(Py_False);
      break;
    case protocol::TType::T_STRING:
      // For strings, the default value is the empty string (or, if `IOBuf`s are
      // used, an empty `IOBuf`).
      switch (*static_cast<const detail::StringFieldType*>(typeInfo.typeExt)) {
        case detail::StringFieldType::String:
        case detail::StringFieldType::StringView:
        case detail::StringFieldType::Binary:
        case detail::StringFieldType::BinaryStringView:
          ptr = PyBytes_FromString("");
          break;
        case detail::StringFieldType::IOBuf:
        case detail::StringFieldType::IOBufPtr:
        case detail::StringFieldType::IOBufObj:
          ptr = create_IOBuf(folly::IOBuf::create(0));
      }
      break;
    case protocol::TType::T_LIST:
      ptr = Variant == PythonVariant::Mutable ? PyList_New(0) : PyTuple_New(0);
      break;
    case protocol::TType::T_MAP:
      ptr = Variant == PythonVariant::Mutable ? PyDict_New() : PyTuple_New(0);
      break;
    case protocol::TType::T_SET:
      // For sets, the default value is an empty `frozenset`.
      ptr = Variant == PythonVariant::Mutable ? PySet_New(nullptr)
                                              : PyFrozenSet_New(nullptr);
      break;
    case protocol::TType::T_STRUCT: {
      // For struct and unions, the default value is a (recursively)
      // default-initialized instance.
      auto structInfo =
          static_cast<const detail::StructInfo*>(typeInfo.typeExt);
      if (structInfo->unionExt != nullptr) {
        ptr = Variant == PythonVariant::Mutable ? createMutableUnionDataHolder()
                                                : createUnionTuple();
      } else {
        ptr = Variant == PythonVariant::Mutable
            ? createStructListWithDefaultValues(*structInfo)
            : createStructTupleWithDefaultValues(*structInfo);
      }
      break;
    }
    default:
      LOG(FATAL) << "invalid typeInfo TType " << typeInfo.type;
  }
  if (ptr == nullptr) {
    THRIFT_PY3_CHECK_ERROR();
  }
  return UniquePyObjectPtr(ptr);
}

/**
 * Returns the appropriate standard immutable default value for the given
 * `typeInfo`, along with a boolean indicating whether it should be added to
 * the cache.
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
UniquePyObjectPtr getStandardImmutableDefaultValueForType(
    const detail::TypeInfo& typeInfo) {
  return getStandardDefaultValueForType<PythonVariant::Immutable>(typeInfo);
}

/**
 * Returns the appropriate standard mutable default value for the given
 * `typeInfo`, along with a boolean indicating whether it should be added to
 * the cache.
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
UniquePyObjectPtr getStandardMutableDefaultValueForType(
    const detail::TypeInfo& typeInfo) {
  return getStandardDefaultValueForType<PythonVariant::Mutable>(typeInfo);
}

/**
 * Populates only the unset fields of a "struct container" with default values.
 *
 * The `container` parameter should be a valid Python object, created by the
 * `createStructContainer()`.
 *
 * Iterates through the elements (from 1 to `numFields + 1`). If a field
 * is unset, it is populated with the corresponding default value.
 * The mechanism for determining the default value is the same as in the
 * `createStructContainerWithDefaultValues()` function. Please see the
 * documentation of `createStructContainerWithDefaultValues()` for details on
 * how the default value is identified.
 *
 * Throws on error
 */
template <typename Container>
void populateStructContainerUnsetFieldsWithDefaultValues(
    PyObject* container, const detail::StructInfo& structInfo) {
  if (container == nullptr) {
    throw std::runtime_error("null container!");
  }

  DCHECK(Container::Check(container));
  const int16_t numFields = structInfo.numFields;
  DCHECK(Container::Size(container) == numFields + 1);

  const FieldValueMap& defaultValues =
      *static_cast<const FieldValueMap*>(structInfo.customExt);
  const char* issetFlags = getDataHolderIssetFlags<Container>(container);
  for (int i = 0; i < numFields; ++i) {
    // If the field is already set, this implies that the constructor has
    // already assigned a value to the field. In this case, we skip it and
    // avoid overwriting it with the default value.
    if (issetFlags[i]) {
      continue;
    }

    const detail::FieldInfo& fieldInfo = structInfo.fieldInfos[i];
    PyObject* oldValue = Container::GET_ITEM(container, i + 1);
    if (fieldInfo.qualifier == detail::FieldQualifier::Optional) {
      Container::SET_ITEM(container, i + 1, Py_None);
      Py_INCREF(Py_None);
    } else {
      // getDefaultValueForField calls `Py_INCREF`
      Container::SET_ITEM(
          container,
          i + 1,
          Container::getDefaultValueForField(
              fieldInfo.typeInfo, defaultValues, i)
              .release());
    }
    Py_DECREF(oldValue);
  }
}

/**
 * Populates only the unset fields of a "struct tuple" with default values.
 */
void populateStructTupleUnsetFieldsWithDefaultValues(
    PyObject* tuple, const detail::StructInfo& structInfo) {
  populateStructContainerUnsetFieldsWithDefaultValues<TupleContainer>(
      tuple, structInfo);
}

/**
 * Populates only the unset fields of a "struct list" with default values.
 */
void populateStructListUnsetFieldsWithDefaultValues(
    PyObject* tuple, const detail::StructInfo& structInfo) {
  populateStructContainerUnsetFieldsWithDefaultValues<ListContainer>(
      tuple, structInfo);
}

void* setImmutableStruct(void* objectPtr, const detail::TypeInfo& typeInfo) {
  return setPyObject(
      objectPtr,
      UniquePyObjectPtr{createStructTupleWithDefaultValues(
          *static_cast<const detail::StructInfo*>(typeInfo.typeExt))});
}

void* setMutableStruct(void* objectPtr, const detail::TypeInfo& typeInfo) {
  PyObject* list = setPyObject(
      objectPtr,
      UniquePyObjectPtr{createStructListWithDefaultValues(
          *static_cast<const detail::StructInfo*>(typeInfo.typeExt))});

  return getListObjectItemBase(list);
}

void* setImmutableUnion(
    void* objectPtr, const detail::TypeInfo& /* typeInfo */) {
  return setPyObject(objectPtr, UniquePyObjectPtr{createUnionTuple()});
}

/**
 * @param object The (`PyObject*`) data holder of the target immutable struct
 * instance. This should correspond to a `PyTuple` instance.
 */
bool getImmutableIsset(const void* object, ptrdiff_t offset) {
  const char* flags = getDataHolderIssetFlags<TupleContainer>(
      static_cast<const PyObject*>(object));
  return flags[offset];
}

void* setMutableUnion(void* objectPtr, const detail::TypeInfo& /* typeInfo */) {
  PyObject* list =
      setPyObject(objectPtr, UniquePyObjectPtr{createMutableUnionDataHolder()});

  return getListObjectItemBase(list);
}

/**
 * Gets the "isset" flag of the `index`-th field of the 'struct list'
 *
 * The `objectPtr` is double pointer to the allocated memory in PyListObject,
 * please see `DynamicStructInfo::addMutableFieldInfo()`
 */
bool getMutableIsset(const void* objectPtr, ptrdiff_t offset) {
  const char* issetFlags =
      PyBytes_AsString(*static_cast<PyObject* const*>(objectPtr));
  if (issetFlags == nullptr) {
    THRIFT_PY3_CHECK_ERROR();
  }
  return issetFlags[offset];
}

void setIsset(void* objectPtr, ptrdiff_t offset, bool value) {
  return setStructIsset(static_cast<PyObject*>(objectPtr), offset, value);
}

/**
 * Sets the "isset" flag of the `index`-th field of the 'struct list'
 *
 * The `objectPtr` is double pointer to the allocated memory in PyListObject,
 * please see `DynamicStructInfo::addMutableFieldInfo()`
 */
void setMutableIsset(void* objectPtr, ptrdiff_t offset, bool value) {
  char* flags = PyBytes_AsString(*static_cast<PyObject**>(objectPtr));
  if (flags == nullptr) {
    THRIFT_PY3_CHECK_ERROR();
  }
  flags[offset] = value;
}

/**
 * Clears a thrift-python union.
 *
 * @param object A `PyObject*` that corresponds to the "data holder" for a
 *        thrift-python Union class. Must not be nullptr.
 */
template <typename TDataHolderPolicy>
void clearUnionDataHolder(void* unionDataHolderObject) {
  PyObject* const unionDataHolder = toPyObject(unionDataHolderObject);

  // Clear field id of "present field" (if any).
  PyObject* const previousActiveFieldId =
      TDataHolderPolicy::GET_ITEM(unionDataHolder, 0);
  UniquePyObjectPtr zero{PyLong_FromLong(0)};
  if (zero == nullptr) {
    THRIFT_PY3_CHECK_ERROR();
  }
  TDataHolderPolicy::SET_ITEM(unionDataHolder, 0, zero.release());
  Py_XDECREF(previousActiveFieldId);

  // Clear value (if any).
  PyObject* const previousValue =
      TDataHolderPolicy::GET_ITEM(unionDataHolder, 1);
  TDataHolderPolicy::SET_ITEM(unionDataHolder, 1, Py_None);
  Py_INCREF(Py_None);
  Py_XDECREF(previousValue);
}

void clearImmutableUnion(void* object) {
  clearUnionDataHolder<TupleContainer>(object);
}

void clearMutableUnion(void* object) {
  PyObject** unionDataHolder = toPyObjectPtr(object);

  // Clear field id of "present field" (if any).
  PyObject* const previousActiveFieldId = *unionDataHolder;
  UniquePyObjectPtr zero{PyLong_FromLong(0)};
  if (zero == nullptr) {
    THRIFT_PY3_CHECK_ERROR();
  }
  *unionDataHolder = zero.release();
  Py_XDECREF(previousActiveFieldId);

  // Clear value (if any).
  PyObject* const previousValue = *++unionDataHolder;
  *unionDataHolder = Py_None;
  Py_INCREF(Py_None);
  Py_XDECREF(previousValue);
}

/**
 * Returns the id of the field that is currently set for the given union data
 * holder, or 0 if the union is empty.
 *
 * @param unionDataHolderObject `PyObject*` that holds the "data holder" for a
 *        thrift-python Union class. Must not be nullptr.
 */
template <typename TDataHolderPolicy>
int getUnionDataHolderActiveFieldId(const void* unionDataHolderObject) {
  const PyObject* const unionDataHolder = toPyObject(unionDataHolderObject);
  const long id =
      PyLong_AsLong(TDataHolderPolicy::GET_ITEM(unionDataHolder, 0));
  if (id == -1) {
    THRIFT_PY3_CHECK_POSSIBLE_ERROR();
  }
  return id;
}

int getImmutableUnionActiveFieldId(const void* object) {
  return getUnionDataHolderActiveFieldId<TupleContainer>(object);
}

int getMutableUnionActiveFieldId(const void* object) {
  PyObject* const unionDataHolder = *toPyObjectPtr(object);
  const long id = PyLong_AsLong(unionDataHolder);
  if (id == -1) {
    THRIFT_PY3_CHECK_POSSIBLE_ERROR();
  }
  return id;
}

/**
 * Updates the given union data holder to indicate that the given fieldId is
 * currently set for that union.
 *
 * @param unionDataHolderObject A `PyObject*` that corresponds to a "data
 *        holder" for a thrift-python Union class. Must not be nullptr.
 * @param fieldId of the field that is marked as "present" for the given union.
 *        Should be > 0.
 */
template <typename TDataHolderPolicy>
void setUnionDataHolderActiveFieldId(void* unionDataHolderObject, int fieldId) {
  UniquePyObjectPtr fieldIdPyObj{PyLong_FromLong(fieldId)};
  if (fieldIdPyObj == nullptr) {
    THRIFT_PY3_CHECK_ERROR();
  }

  PyObject* unionDataHolder = toPyObject(unionDataHolderObject);
  PyObject* previousFieldId = TDataHolderPolicy::GET_ITEM(unionDataHolder, 0);
  TDataHolderPolicy::SET_ITEM(unionDataHolder, 0, fieldIdPyObj.release());
  Py_DECREF(previousFieldId);
}

void setImmutableUnionActiveFieldId(void* object, int fieldId) {
  setUnionDataHolderActiveFieldId<TupleContainer>(object, fieldId);
}

void setMutableUnionActiveFieldId(void* object, int fieldId) {
  UniquePyObjectPtr fieldIdPyObj{PyLong_FromLong(fieldId)};
  if (fieldIdPyObj == nullptr) {
    THRIFT_PY3_CHECK_ERROR();
  }

  PyObject** unionDataHolder = toPyObjectPtr(object);
  PyObject* previousFieldId = *unionDataHolder;
  *unionDataHolder = fieldIdPyObj.release();
  Py_DECREF(previousFieldId);
}

const detail::UnionExtN<1> kImmutableUnionExt = {
    /* .clear */ clearImmutableUnion,
    /* .unionTypeOffset */ 0,
    /* .getActiveId */ getImmutableUnionActiveFieldId,
    /* .setActiveId */ setImmutableUnionActiveFieldId,
    /* .initMember */ {nullptr},
};

const detail::UnionExtN<1> kMutableUnionExt = {
    /* .clear */ clearMutableUnion,
    /* .unionTypeOffset */ 0,
    /* .getActiveId */ getMutableUnionActiveFieldId,
    /* .setActiveId */ setMutableUnionActiveFieldId,
    /* .initMember */ {nullptr},
};

const detail::UnionExt* FOLLY_NULLABLE
getStructInfoUnionExt(bool isUnion, bool isMutable) {
  if (!isUnion) {
    return nullptr;
  }

  return reinterpret_cast<const detail::UnionExt*>(
      isMutable ? &kMutableUnionExt : &kImmutableUnionExt);
}

/**
 * Creates a new (table-based) serializer StructInfo for the thrift-python
 * structured type (struct, union or exception) with the given properties.
 *
 * The returned StructInfo holds (pointers to) methods that can perform the
 * operations required for (de)serializing instances of this type, given a
 * type-erased (i.e., `void*`) "target object".
 *
 * For immutable Thrift structs, the target object corresponds to the "data
 * holder" container, i.e. a PyTuple instance of size `numFields + 1`.
 *
 * TODO: immutable Thrift unions? mutable structs? mutable unions?
 *
 * @param namePtr Name of the Thrift type. The returned object holds a pointer
 *        to this string, but does not take ownership.
 * @param fieldValues Map of default values for the fields of the returned type.
 *        The return StructInfo maintains a pointer to this map (as its
 *        `customExt`), but does not take ownership.
 */
detail::StructInfo* newTableBasedSerializerStructInfo(
    const char* namePtr,
    int16_t numFields,
    bool isUnion,
    FieldValueMap& fieldValues,
    bool isMutable) {
  auto* structInfo = static_cast<detail::StructInfo*>(folly::operator_new(
      sizeof(detail::StructInfo) + sizeof(detail::FieldInfo) * numFields,
      std::align_val_t{alignof(detail::StructInfo)}));
  structInfo->numFields = numFields;
  structInfo->name = namePtr;
  structInfo->unionExt = getStructInfoUnionExt(isUnion, isMutable);
  structInfo->getIsset = isMutable ? getMutableIsset : getImmutableIsset;
  structInfo->setIsset = isMutable ? setMutableIsset : setIsset;
  structInfo->getFieldValuesBasePtr = nullptr;
  structInfo->customExt = &fieldValues;
  return structInfo;
}

/**
 * Returns a view into the string contained in the given Python bytes referred
 * to by `objectPtr`.
 *
 * Note that, if this method returns, the returned optional always holds a
 * `ThriftValue` with a `stringViewValue`.
 *
 * @param objectPtr double pointer to a Python bytes object (i.e., a
 *        [`PyBytesObject**`](https://docs.python.org/3/c-api/bytes.html#c.PyBytesObject))
 *
 * @throws if `objectPtr` does not contain a valid string.
 */
detail::OptionalThriftValue getString(
    const void* objectPtr, const detail::TypeInfo& /* typeInfo */) {
  // Note: `PyObject` is a parent class of `PyBytesObject`, so the following
  // assignment is correct.
  PyObject* pyBytesObject = *toPyObjectPtr(objectPtr);

  Py_ssize_t len = 0;
  char* buf = nullptr;
  if (PyBytes_AsStringAndSize(pyBytesObject, &buf, &len) == -1) {
    THRIFT_PY3_CHECK_ERROR();
  }
  return folly::make_optional<detail::ThriftValue>(
      folly::StringPiece{buf, static_cast<std::size_t>(len)});
}

/**
 * Copies the given string `value` into a new `PyBytesObject` instance, and
 * updates the given `object` to hold a pointer to that instance.
 *
 * @param objectPtr a `PyBytesObject**` (see `getString()` above).
 * @param value String whose copy will be in a new Python bytes object.
 *
 * @throws if `value` cannot be copied to a new `PyBytesObject`.
 */
void* setString(void* objectPtr, const std::string& value) {
  UniquePyObjectPtr bytesObj{
      PyBytes_FromStringAndSize(value.data(), value.size())};
  if (bytesObj == nullptr) {
    THRIFT_PY3_CHECK_ERROR();
  }
  setPyObject(objectPtr, std::move(bytesObj));
  return nullptr;
}

detail::OptionalThriftValue getIOBuf(
    const void* objectPtr, const detail::TypeInfo& /* typeInfo */) {
  ensureImportOrThrow();
  PyObject* pyObj = *toPyObjectPtr(objectPtr);
  folly::IOBuf* buf = pyObj != nullptr ? get_cIOBuf(pyObj) : nullptr;
  return buf ? folly::make_optional<detail::ThriftValue>(buf)
             : detail::OptionalThriftValue{};
}

void* setIOBuf(void* objectPtr, const folly::IOBuf& value) {
  ensureImportOrThrow();
  PyObject* buf = create_IOBuf(value.clone());
  UniquePyObjectPtr iobufObj{buf};
  if (buf == nullptr) {
    THRIFT_PY3_CHECK_ERROR();
  }
  setPyObject(objectPtr, std::move(iobufObj));
  return nullptr;
}

// This helper method for `MutableMapTypeInfo::write()` sorts the map keys and
// writes them to the wire. It is called when the `protocolSortKeys` parameter
// of `write()` is set to `true`.
size_t writeMapSorted(
    const void* context,
    const void* object,
    size_t (*writer)(
        const void* context, const void* keyElem, const void* valueElem)) {
  PyObject* dict = const_cast<PyObject*>(toPyObject(object));
  DCHECK(PyDict_Check(dict));
  UniquePyObjectPtr listPtr =
      UniquePyObjectPtr{PySequence_List(PyDict_Items(dict))};
  if (!listPtr) {
    THRIFT_PY3_CHECK_ERROR();
  }
  if (PyList_Sort(listPtr.get()) == -1) {
    THRIFT_PY3_CHECK_ERROR();
  }

  size_t written = 0;
  const Py_ssize_t size = PyList_Size(listPtr.get());
  for (std::uint32_t i = 0; i < size; ++i) {
    PyObject* pair = PyList_GET_ITEM(listPtr.get(), i);
    PyObject* key = PyTuple_GET_ITEM(pair, 0);
    PyObject* value = PyTuple_GET_ITEM(pair, 1);
    written += writer(context, &key, &value);
  }

  return written;
}

inline UniquePyObjectPtr primitiveCppToPython(bool value) {
  PyObject* ret = value ? Py_True : Py_False;
  Py_INCREF(ret);
  return UniquePyObjectPtr{ret};
}

inline UniquePyObjectPtr primitiveCppToPython(std::int32_t value) {
  if (UniquePyObjectPtr ret{PyLong_FromLong(value)}) {
    return ret;
  }
  THRIFT_PY3_CHECK_ERROR();
}

inline UniquePyObjectPtr primitiveCppToPython(std::int8_t value) {
  return primitiveCppToPython(static_cast<std::int32_t>(value));
}

inline UniquePyObjectPtr primitiveCppToPython(std::int16_t value) {
  return primitiveCppToPython(static_cast<std::int32_t>(value));
}

inline UniquePyObjectPtr primitiveCppToPython(std::int64_t value) {
  if (UniquePyObjectPtr ret{PyLong_FromLongLong(value)}) {
    return ret;
  }
  THRIFT_PY3_CHECK_ERROR();
}

inline UniquePyObjectPtr primitiveCppToPython(float value) {
  if (UniquePyObjectPtr ret{PyFloat_FromDouble(value)}) {
    return ret;
  }
  THRIFT_PY3_CHECK_ERROR();
}

inline UniquePyObjectPtr primitiveCppToPython(double value) {
  if (UniquePyObjectPtr ret{PyFloat_FromDouble(value)}) {
    return ret;
  }
  THRIFT_PY3_CHECK_ERROR();
}

/**
 * Converts the given integral value to the specified target type.
 *
 * Both source and target types MUST be integrals (or this will fail to
 * compile).
 *
 * Terminates the process (i.e., aborts) if the input value does not fit in the
 * target type range.
 */
template <typename T, typename V>
inline T convInt(V v) {
  static_assert(std::is_integral_v<T> && std::is_integral_v<V>);
  if (v >= std::numeric_limits<T>::min() &&
      v <= std::numeric_limits<T>::max()) {
    return static_cast<T>(v);
  }
  LOG(FATAL) << "int out of range";
}

/**
 * Converts the given python `object` to the corresponding native C++ type.
 *
 * The returned `ThriftValue` union instance will be initialized with the field
 * corresponding to the given `TCppType`.
 *
 * Note: by design, no definition is provided for this primary template.
 * Instead, explicit specializations are defined for all supported target C++
 * types (i.e., `TCppType` template parameters).
 */
template <typename TCppType>
detail::ThriftValue primitivePythonToCpp(PyObject* object);

template <>
inline detail::ThriftValue primitivePythonToCpp<bool>(PyObject* object) {
  DCHECK(object == Py_True || object == Py_False);
  return detail::ThriftValue{object == Py_True};
}

/**
 * Converts the given `PyLong` object to the C++ integral type (`TCppIntType`).
 *
 * Throws `std::runtime_error` if `object` does not hold a (valid) `PyLong`.
 * Terminates the process (i.e., aborts) if the value of the given long `object`
 * does not fit in the target `TCppIntType`.
 */
template <typename TCppIntType>
inline detail::ThriftValue pyLongToCpp(PyObject* object) {
  const long value = PyLong_AsLong(object);
  if (value == -1) {
    THRIFT_PY3_CHECK_POSSIBLE_ERROR();
  }
  return detail::ThriftValue{convInt<TCppIntType>(value)};
}

template <>
inline detail::ThriftValue primitivePythonToCpp<std::int8_t>(PyObject* object) {
  return pyLongToCpp<std::int8_t>(object);
}

template <>
inline detail::ThriftValue primitivePythonToCpp<std::int16_t>(
    PyObject* object) {
  return pyLongToCpp<std::int16_t>(object);
}

template <>
inline detail::ThriftValue primitivePythonToCpp<std::int32_t>(
    PyObject* object) {
  return pyLongToCpp<std::int32_t>(object);
}

template <>
inline detail::ThriftValue primitivePythonToCpp<std::int64_t>(
    PyObject* object) {
  const long long value = PyLong_AsLongLong(object);
  if (value == -1) {
    THRIFT_PY3_CHECK_POSSIBLE_ERROR();
  }
  return detail::ThriftValue{convInt<std::int64_t>(value)};
}

template <>
inline detail::ThriftValue primitivePythonToCpp<double>(PyObject* object) {
  const double value = PyFloat_AsDouble(object);
  if (value == -1.0) {
    THRIFT_PY3_CHECK_POSSIBLE_ERROR();
  }
  return detail::ThriftValue{value};
}

template <>
inline detail::ThriftValue primitivePythonToCpp<float>(PyObject* object) {
  const double value = PyFloat_AsDouble(object);
  if (value == -1.0) {
    THRIFT_PY3_CHECK_POSSIBLE_ERROR();
  }
  return detail::ThriftValue{static_cast<float>(value)};
}

/**
 * Convenience class template to provide `TypeInfo` implementations for all
 * primitive C++/Python type combinations.
 */
template <typename TCppType, protocol::TType TThriftProtocolTypeEnum>
class PrimitiveTypeInfoHelper final {
 public:
  /**
   * Type info suitable for Thrift Table-Based serialization of thrift-python
   * values, for the given template parameters.
   */
  static const detail::TypeInfo kTypeInfo;

 private:
  /**
   * Returns the Thrift native (C++) value for the given Python `objectPtr`.
   *
   * @param `objectPtr` A (pointer to) `PyObject*` whose (Python) value
   *         corresponds to the specified `TCppType`.
   *
   * @return a `ThriftValue` union instance whose `TCppType` field is set with
   *         the corresponding value, extracted from the given `PyObject*`.
   */
  static detail::OptionalThriftValue get(
      const void* objectPtr, const detail::TypeInfo& /* typeInfo */) {
    PyObject* pyObj = *toPyObjectPtr(objectPtr);
    return folly::make_optional<detail::ThriftValue>(
        primitivePythonToCpp<TCppType>(pyObj));
  }

  /**
   * Updates the `PyObject*` pointed to by `objectPtr` to hold the given `value`
   * (converted to the corresponding Python type).
   *
   * @param `objectPtr` Pointer to a `PyObject*` that will be set to the
   *        resulting value.
   *
   * @param `value` Native C++ value (of the given `TCppType`) that will be
   *        converted to the corresponding Python type and pointed to by the
   *        given `PyObject*`.
   */
  static void* set(void* objectPtr, TCppType value) {
    setPyObject(objectPtr, primitiveCppToPython(value));
    return nullptr;
  }
};

template <typename TCppType, protocol::TType TThriftProtocolTypeEnum>
const detail::TypeInfo
    PrimitiveTypeInfoHelper<TCppType, TThriftProtocolTypeEnum>::kTypeInfo{
        /* .type */ TThriftProtocolTypeEnum,
        /* .get */ get,
        /* .set */ reinterpret_cast<detail::VoidPtrFuncPtr>(set),
        /* .typeExt */ nullptr,
    };

template <typename TDataHolderPolicy>
PyObject* createUnionDataHolder() {
  // Data items: (current field enum value, field value)
  UniquePyObjectPtr unionDataHolder{TDataHolderPolicy::New(/* size */ 2)};
  if (unionDataHolder == nullptr) {
    THRIFT_PY3_CHECK_ERROR();
  }

  // Initialize union data to "empty" union, i.e. `(0, Py_None)`. Indeed, 0 is
  // the special enum value corresponding to an empty union, for all thrift
  // unions.
  TDataHolderPolicy::SET_ITEM(unionDataHolder.get(), 0, PyLong_FromLong(0));
  TDataHolderPolicy::SET_ITEM(unionDataHolder.get(), 1, Py_None);
  Py_INCREF(Py_None);

  return unionDataHolder.release();
}

} // namespace

// DO_BEFORE(aristidis,20240920): For consistency, rename method to
// createImmutableUnionDataHolder() - and update references.
PyObject* createUnionTuple() {
  return createUnionDataHolder<TupleContainer>();
}

PyObject* createMutableUnionDataHolder() {
  return createUnionDataHolder<ListContainer>();
}

template <typename Container>
PyObject* createStructContainer(int16_t numFields) {
  // Allocate and 0-initialize numFields bytes.
  UniquePyObjectPtr issetArr{PyBytes_FromStringAndSize(nullptr, numFields)};
  if (issetArr == nullptr) {
    return nullptr;
  }
  char* flags = PyBytes_AsString(issetArr.get());
  if (flags == nullptr) {
    return nullptr;
  }
  for (Py_ssize_t i = 0; i < numFields; ++i) {
    flags[i] = '\0';
  }

  // Create container, with isset byte array as first element (followed by
  // `numFields` uninitialized elements).
  PyObject* container{Container::New(numFields + 1)};
  if (container == nullptr) {
    return nullptr;
  }
  Container::SET_ITEM(container, 0, issetArr.release());
  return container;
}

PyObject* createStructTuple(int16_t numFields) {
  return createStructContainer<TupleContainer>(numFields);
}

PyObject* createStructList(int16_t numFields) {
  return createStructContainer<ListContainer>(numFields);
}

PyObject* createImmutableStructTupleWithDefaultValues(
    const detail::StructInfo& structInfo) {
  return createStructTupleWithDefaultValues(structInfo);
}

PyObject* createMutableStructListWithDefaultValues(
    const detail::StructInfo& structInfo) {
  return createStructListWithDefaultValues(structInfo);
}

template <typename Container>
PyObject* createStructContainerWithNones(const detail::StructInfo& structInfo) {
  const int16_t numFields = structInfo.numFields;
  UniquePyObjectPtr container{Container::createStructContainer(numFields)};
  if (container == nullptr) {
    return nullptr;
  }

  // Initialize container[1:numFields+1] with 'None'.
  for (int i = 0; i < numFields; ++i) {
    Container::SET_ITEM(container.get(), i + 1, Py_None);
    Py_INCREF(Py_None);
  }
  return container.release();
}

PyObject* createStructTupleWithNones(const detail::StructInfo& structInfo) {
  return createStructContainerWithNones<TupleContainer>(structInfo);
}

PyObject* createStructListWithNones(const detail::StructInfo& structInfo) {
  return createStructContainerWithNones<ListContainer>(structInfo);
}

template <typename Container>
void setStructIsset(PyObject* structTuple, int16_t index, bool value) {
  PyObject* issetPyBytes = Container::GET_ITEM(structTuple, 0);
  char* flags = PyBytes_AsString(issetPyBytes);
  if (flags == nullptr) {
    THRIFT_PY3_CHECK_ERROR();
  }
  flags[index] = value;
}

void setStructIsset(PyObject* structTuple, int16_t index, bool value) {
  setStructIsset<TupleContainer>(structTuple, index, value);
}

void setMutableStructIsset(PyObject* structTuple, int16_t index, bool value) {
  setStructIsset<ListContainer>(structTuple, index, value);
}

void populateImmutableStructTupleUnsetFieldsWithDefaultValues(
    PyObject* tuple, const detail::StructInfo& structInfo) {
  populateStructTupleUnsetFieldsWithDefaultValues(tuple, structInfo);
}

void populateMutableStructListUnsetFieldsWithDefaultValues(
    PyObject* tuple, const detail::StructInfo& structInfo) {
  populateStructListUnsetFieldsWithDefaultValues(tuple, structInfo);
}

void resetFieldToStandardDefault(
    PyObject* structList, const detail::StructInfo& structInfo, int index) {
  ensureImportOrThrow();
  if (structList == nullptr) {
    throw std::runtime_error(fmt::format(
        "Received null list while resetting struct:`{}`, field-index:'{}'",
        structInfo.name,
        index));
  }

  DCHECK(PyList_Check(structList));
  DCHECK(index < structInfo.numFields);

  const FieldValueMap& defaultValues =
      *static_cast<const FieldValueMap*>(structInfo.customExt);
  const detail::FieldInfo& fieldInfo = structInfo.fieldInfos[index];
  PyObject* oldValue = PyList_GET_ITEM(structList, index + 1);
  if (fieldInfo.qualifier == detail::FieldQualifier::Optional) {
    PyList_SET_ITEM(structList, index + 1, Py_None);
    Py_INCREF(Py_None);
    setMutableStructIsset(structList, index, false);
  } else {
    // getDefaultValueForField calls `Py_INCREF`
    // This function is called only to reset the fields of mutable types.
    // Therefore, a deep copy of the given default field value is necessary.
    PyList_SET_ITEM(
        structList,
        index + 1,
        getDefaultValueForMutableField(fieldInfo.typeInfo, defaultValues, index)
            .release());
  }
  Py_DECREF(oldValue);
}

detail::OptionalThriftValue getStruct(
    const void* objectPtr, const detail::TypeInfo& /* typeInfo */) {
  PyObject* pyObj = *toPyObjectPtr(objectPtr);
  return folly::make_optional<detail::ThriftValue>(pyObj);
}

detail::TypeInfo createImmutableStructTypeInfo(
    const DynamicStructInfo& dynamicStructInfo) {
  return {
      /* .type */ protocol::TType::T_STRUCT,
      /* .get */ getStruct,
      /* .set */
      reinterpret_cast<detail::VoidPtrFuncPtr>(
          dynamicStructInfo.isUnion() ? setImmutableUnion : setImmutableStruct),
      /* .typeExt */ &dynamicStructInfo.getStructInfo(),
  };
}

detail::OptionalThriftValue getMutableStruct(
    const void* objectPtr, const detail::TypeInfo& /* typeInfo */) {
  return folly::make_optional<detail::ThriftValue>(
      getListObjectItemBase(*static_cast<PyObject* const*>(objectPtr)));
}

detail::TypeInfo createMutableStructTypeInfo(
    const DynamicStructInfo& dynamicStructInfo) {
  return {
      /* .type */ protocol::TType::T_STRUCT,
      /* .get */ getMutableStruct,
      /* .set */
      reinterpret_cast<detail::VoidPtrFuncPtr>(
          dynamicStructInfo.isUnion() ? setMutableUnion : setMutableStruct),
      /* .typeExt */ &dynamicStructInfo.getStructInfo(),
  };
}

void ListTypeInfo::read(
    const void* context,
    void* objectPtr,
    std::uint32_t listSize,
    void (*reader)(const void* /*context*/, void* /*val*/)) {
  // use a tuple to represent a list field for immutability
  UniquePyObjectPtr list{PyTuple_New(listSize)};
  if (!list) {
    THRIFT_PY3_CHECK_ERROR();
  }
  for (std::uint32_t i = 0; i < listSize; ++i) {
    PyObject* elem = nullptr;
    reader(context, &elem);
    PyTuple_SET_ITEM(list.get(), i, elem);
  }
  setPyObject(objectPtr, std::move(list));
}

size_t ListTypeInfo::write(
    const void* context,
    const void* object,
    size_t (*writer)(const void* /*context*/, const void* /*val*/)) {
  const PyObject* list = toPyObject(object);
  const Py_ssize_t size = PyTuple_GET_SIZE(list);
  size_t written = 0;
  for (Py_ssize_t i = 0; i < size; i++) {
    PyObject* elem = PyTuple_GET_ITEM(list, i);
    written += writer(context, &elem);
  }
  return written;
}

void ListTypeInfo::consumeElem(
    const void* context,
    void* objectPtr,
    void (*reader)(const void* /*context*/, void* /*val*/)) {
  PyObject* elem = nullptr;
  reader(context, &elem);
  PyObject** pyObjPtr = toPyObjectPtr(objectPtr);
  const Py_ssize_t currentSize = PyTuple_GET_SIZE(*pyObjPtr);
  if (_PyTuple_Resize(pyObjPtr, currentSize + 1) == -1) {
    THRIFT_PY3_CHECK_ERROR();
  }

  PyTuple_SET_ITEM(*pyObjPtr, currentSize, elem);
}

void MutableListTypeInfo::read(
    const void* context,
    void* objectPtr,
    std::uint32_t listSize,
    void (*reader)(const void* /*context*/, void* /*val*/)) {
  // use a PyList to represent a list field
  UniquePyObjectPtr list{PyList_New(listSize)};
  if (list == nullptr) {
    THRIFT_PY3_CHECK_ERROR();
  }
  for (std::uint32_t i = 0; i < listSize; ++i) {
    PyObject* elem = nullptr;
    reader(context, &elem);
    PyList_SET_ITEM(list.get(), i, elem);
  }
  setPyObject(objectPtr, std::move(list));
}

size_t MutableListTypeInfo::write(
    const void* context,
    const void* object,
    size_t (*writer)(const void* /*context*/, const void* /*val*/)) {
  const PyObject* list = toPyObject(object);
  const Py_ssize_t size = PyList_GET_SIZE(list);
  size_t written = 0;
  for (Py_ssize_t i = 0; i < size; i++) {
    PyObject* elem = PyList_GET_ITEM(list, i);
    written += writer(context, &elem);
  }
  return written;
}

void MutableListTypeInfo::consumeElem(
    const void* context,
    void* objectPtr,
    void (*reader)(const void* /*context*/, void* /*val*/)) {
  PyObject* elem = nullptr;
  reader(context, &elem);
  DCHECK(elem != nullptr);
  PyObject** pyObjPtr = toPyObjectPtr(objectPtr);
  if (PyList_Append(*pyObjPtr, elem) == -1) {
    THRIFT_PY3_CHECK_ERROR();
  }
}

void MapTypeInfo::read(
    const void* context,
    void* objectPtr,
    std::uint32_t mapSize,
    void (*keyReader)(const void* context, void* key),
    void (*valueReader)(const void* context, void* val)) {
  // use a tuple to represent a map for immutablitity and hashability
  UniquePyObjectPtr map{PyTuple_New(mapSize)};
  if (!map) {
    THRIFT_PY3_CHECK_ERROR();
  }
  auto read = [=](auto readerFn) {
    PyObject* obj = nullptr;
    readerFn(context, &obj);
    return UniquePyObjectPtr(obj);
  };
  for (std::uint32_t i = 0; i < mapSize; ++i) {
    UniquePyObjectPtr mkey = read(keyReader);
    UniquePyObjectPtr mvalue = read(valueReader);
    UniquePyObjectPtr elem{PyTuple_New(2)};
    if (!elem) {
      THRIFT_PY3_CHECK_ERROR();
    }
    PyTuple_SET_ITEM(elem.get(), 0, mkey.release());
    PyTuple_SET_ITEM(elem.get(), 1, mvalue.release());
    PyTuple_SET_ITEM(map.get(), i, elem.release());
  }
  setPyObject(objectPtr, std::move(map));
}

size_t MapTypeInfo::write(
    const void* context,
    const void* object,
    bool protocolSortKeys,
    size_t (*writer)(
        const void* context, const void* keyElem, const void* valueElem)) {
  size_t written = 0;
  PyObject* map = const_cast<PyObject*>(toPyObject(object));
  const Py_ssize_t size = PyTuple_GET_SIZE(map);
  UniquePyObjectPtr seq;
  if (protocolSortKeys) {
    seq = UniquePyObjectPtr{PySequence_List(map)};
    if (!seq) {
      THRIFT_PY3_CHECK_ERROR();
    }
    if (PyList_Sort(seq.get()) == -1) {
      THRIFT_PY3_CHECK_ERROR();
    }
    map = PySequence_Tuple(seq.get());
  }
  for (std::uint32_t i = 0; i < size; ++i) {
    PyObject* pair = PyTuple_GET_ITEM(map, i);
    PyObject* key = PyTuple_GET_ITEM(pair, 0);
    PyObject* value = PyTuple_GET_ITEM(pair, 1);
    written += writer(context, &key, &value);
  }
  return written;
}

void MapTypeInfo::consumeElem(
    const void* context,
    void* objectPtr,
    void (*keyReader)(const void* context, void* key),
    void (*valueReader)(const void* context, void* val)) {
  PyObject** pyObjPtr = toPyObjectPtr(objectPtr);
  CHECK_NOTNULL(*pyObjPtr);
  PyObject* mkey = nullptr;
  keyReader(context, &mkey);
  PyObject* mval = nullptr;
  valueReader(context, &mval);
  UniquePyObjectPtr elem{PyTuple_New(2)};
  if (!elem) {
    THRIFT_PY3_CHECK_ERROR();
  }
  PyTuple_SET_ITEM(elem.get(), 0, mkey);
  PyTuple_SET_ITEM(elem.get(), 1, mval);
  const Py_ssize_t currentSize = PyTuple_GET_SIZE(*pyObjPtr);
  if (_PyTuple_Resize(pyObjPtr, currentSize + 1) == -1) {
    THRIFT_PY3_CHECK_ERROR();
  }
  PyTuple_SET_ITEM(*pyObjPtr, currentSize, elem.release());
}

void MutableMapTypeInfo::read(
    const void* context,
    void* objectPtr,
    std::uint32_t mapSize,
    void (*keyReader)(const void* context, void* key),
    void (*valueReader)(const void* context, void* val)) {
  UniquePyObjectPtr dict{PyDict_New()};
  if (dict == nullptr) {
    THRIFT_PY3_CHECK_ERROR();
  }
  auto read = [context](auto readerFn) {
    PyObject* obj = nullptr;
    readerFn(context, &obj);
    return UniquePyObjectPtr(obj);
  };
  for (std::uint32_t i = 0; i < mapSize; ++i) {
    UniquePyObjectPtr mkey = read(keyReader);
    UniquePyObjectPtr mvalue = read(valueReader);
    PyDict_SetItem(dict.get(), mkey.release(), mvalue.release());
  }
  setPyObject(objectPtr, std::move(dict));
}

size_t MutableMapTypeInfo::write(
    const void* context,
    const void* object,
    bool protocolSortKeys,
    size_t (*writer)(
        const void* context, const void* keyElem, const void* valueElem)) {
  if (protocolSortKeys) {
    return writeMapSorted(context, object, writer);
  }

  PyObject* dict = const_cast<PyObject*>(toPyObject(object));
  size_t written = 0;
  PyObject* key = nullptr;
  PyObject* value = nullptr;
  Py_ssize_t pos = 0;
  while (PyDict_Next(dict, &pos, &key, &value)) {
    written += writer(context, &key, &value);
  }
  return written;
}

void MutableMapTypeInfo::consumeElem(
    const void* context,
    void* objectPtr,
    void (*keyReader)(const void* context, void* key),
    void (*valueReader)(const void* context, void* val)) {
  PyObject** pyObjPtr = toPyObjectPtr(objectPtr);
  DCHECK(*pyObjPtr != nullptr);
  PyObject* mkey = nullptr;
  keyReader(context, &mkey);
  DCHECK(mkey != nullptr);
  PyObject* mval = nullptr;
  valueReader(context, &mval);
  DCHECK(mval != nullptr);
  PyDict_SetItem(*pyObjPtr, mkey, mval);
}

DynamicStructInfo::DynamicStructInfo(
    const char* name, int16_t numFields, bool isUnion, bool isMutable)
    : name_{name},
      tableBasedSerializerStructInfo_{newTableBasedSerializerStructInfo(
          name_.c_str(), numFields, isUnion, fieldValues_, isMutable)} {
  // reserve vector as we are assigning const char* from the string in
  // vector
  fieldNames_.reserve(numFields);
}

DynamicStructInfo::~DynamicStructInfo() {
  for (auto [unused_field_id, field_value_py_object] : fieldValues_) {
    Py_DECREF(field_value_py_object);
  }
  folly::operator_delete(
      tableBasedSerializerStructInfo_,
      sizeof(detail::StructInfo) +
          sizeof(detail::FieldInfo) *
              tableBasedSerializerStructInfo_->numFields,
      std::align_val_t{alignof(detail::StructInfo)});
}

void DynamicStructInfo::addFieldInfo(
    detail::FieldID id,
    detail::FieldQualifier qualifier,
    const char* name,
    const detail::TypeInfo* typeInfo) {
  const std::string& fieldName = fieldNames_.emplace_back(name);
  int16_t idx = fieldNames_.size() - 1;

  // In immutable thrift-python, tuples are used as the internal representation
  // of structs. The first member of the tuple contains the isset flags, while
  // the remaining members represent each field. Since PyTupleObject utilizes
  // C's flexible array member feature, its members are allocated contiguously
  // at the end of the tuple struct, roughly laid out as follows:
  //
  // +-----------------------------------+
  // |        PyTupleObject              |
  // +-----------------------------------+ -> 0
  // |  HEADER                           |
  // +-----------------------------------+
  // |  size (number of items)           |
  // +-----------------------------------+ -> kHeadOffset
  // |  item[0] (PyObject*)(isset flags) |
  // +-----------------------------------+
  // |  item[1] (PyObject*)              |
  // +-----------------------------------+
  // |  ...                              |
  // +-----------------------------------+
  // |  item[n-1] (PyObject*)            |
  // +-----------------------------------+
  //
  // When passing a PyTupleObject to TableBasedSerializer, fields are expected
  // to be identified by their offset from the start address of the struct.
  // The following calculates the offset:
  //
  // memberOffset = kHeadOffset + ((field-order + 1) * sizeof(PyObject*))
  // (+1 accounts for the isset flags at the beginning of the tuple)

  tableBasedSerializerStructInfo_->fieldInfos[idx] = detail::FieldInfo{
      /* .id */ id,
      /* .qualifier */ qualifier,
      /* .name */ fieldName.c_str(),
      /* .memberOffset */
      static_cast<ptrdiff_t>(
          kHeadOffset + kFieldOffset * (isUnion() ? 1 : idx + 1)),
      /* .issetOffset */ isUnion() ? 0 : idx,
      /* .typeInfo */ typeInfo};
}

void DynamicStructInfo::addMutableFieldInfo(
    detail::FieldID id,
    detail::FieldQualifier qualifier,
    const char* name,
    const detail::TypeInfo* typeInfo) {
  const std::string& fieldName = fieldNames_.emplace_back(name);
  int16_t idx = fieldNames_.size() - 1;

  // In mutable thrift-python, lists are used as the internal representation
  // of structs. The first member of the list contains the isset flags, while
  // the remaining members represent each field. PyListObject roughly lays out
  // as follow:
  //
  // +-------------------------------+
  // |         PyListObject          |
  // +-------------------------------+
  // |  HEADER                       |
  // +-------------------------------+      +-------------------------------+
  // |  items (PyObject **)          |----->|  PyObject *item0 (issetFlags) |
  // +-------------------------------+      +-------------------------------+
  // |  ...                          |      |  ...                          |
  // +-------------------------------+      +-------------------------------+
  //                                        |  PyObject *itemN              |
  //                                        +-------------------------------+
  //
  // When passing a PyListObject to TableBasedSerializer, fields are expected
  // to be identified by their offset from the start address of the struct.
  // Therefore, we do not pass PyListObject to the TableBasedSerializer but
  // we pass the allocated memory pointed by `items` and the following
  // calculates the offset:
  //
  // memberOffset = (field-order + 1) * sizeof(PyObject*)
  // (+1 accounts for the isset flags at the beginning of the list)

  tableBasedSerializerStructInfo_->fieldInfos[idx] = detail::FieldInfo{
      /* .id */ id,
      /* .qualifier */ qualifier,
      /* .name */ fieldName.c_str(),
      /* .memberOffset */
      static_cast<ptrdiff_t>(kFieldOffset * (isUnion() ? 1 : idx + 1)),
      /* .issetOffset */ isUnion() ? 0 : idx,
      /* .typeInfo */ typeInfo};
}

void DynamicStructInfo::addFieldValue(int16_t index, PyObject* fieldValue) {
  DCHECK(fieldValue);
  if (fieldValue == Py_None) {
    return;
  }
  Py_INCREF(fieldValue);
  fieldValues_.emplace(index, fieldValue);
}

const detail::TypeInfo& boolTypeInfo =
    PrimitiveTypeInfoHelper<bool, protocol::TType::T_BOOL>::kTypeInfo;
const detail::TypeInfo& byteTypeInfo =
    PrimitiveTypeInfoHelper<std::int8_t, protocol::TType::T_BYTE>::kTypeInfo;
const detail::TypeInfo& i16TypeInfo =
    PrimitiveTypeInfoHelper<std::int16_t, protocol::TType::T_I16>::kTypeInfo;
const detail::TypeInfo& i32TypeInfo =
    PrimitiveTypeInfoHelper<std::int32_t, protocol::TType::T_I32>::kTypeInfo;
const detail::TypeInfo& i64TypeInfo =
    PrimitiveTypeInfoHelper<std::int64_t, protocol::TType::T_I64>::kTypeInfo;
const detail::TypeInfo& doubleTypeInfo =
    PrimitiveTypeInfoHelper<double, protocol::TType::T_DOUBLE>::kTypeInfo;
const detail::TypeInfo& floatTypeInfo =
    PrimitiveTypeInfoHelper<float, protocol::TType::T_FLOAT>::kTypeInfo;

const detail::StringFieldType stringFieldType =
    detail::StringFieldType::StringView;

const detail::StringFieldType binaryFieldType =
    detail::StringFieldType::BinaryStringView;

const detail::StringFieldType ioBufFieldType =
    detail::StringFieldType::IOBufObj;

const detail::TypeInfo stringTypeInfo{
    /* .type */ protocol::TType::T_STRING,
    /* .get */ getString,
    /* .set */ reinterpret_cast<detail::VoidPtrFuncPtr>(setString),
    /* .typeExt */ &stringFieldType,
};

const detail::TypeInfo binaryTypeInfo{
    /* .type */ protocol::TType::T_STRING,
    /* .get */ getString,
    /* .set */ reinterpret_cast<detail::VoidPtrFuncPtr>(setString),
    /* .typeExt */ &binaryFieldType,
};

const detail::TypeInfo iobufTypeInfo{
    /* .type */ protocol::TType::T_STRING,
    /* .get */ getIOBuf,
    /* .set */ reinterpret_cast<detail::VoidPtrFuncPtr>(setIOBuf),
    /* .typeExt */ &ioBufFieldType,
};

PyObject* getStandardImmutableDefaultValuePtrForType(
    const detail::TypeInfo& typeInfo) {
  return getStandardImmutableDefaultValueForType(typeInfo).release();
}
PyObject* getStandardMutableDefaultValuePtrForType(
    const detail::TypeInfo& typeInfo) {
  return getStandardMutableDefaultValueForType(typeInfo).release();
}

void tag_object_as_sequence(PyTypeObject* type_object) {
  DCHECK(PyType_Check(type_object));
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 10
  type_object->tp_flags |= Py_TPFLAGS_SEQUENCE;
#endif
}
void tag_object_as_mapping(PyTypeObject* type_object) {
  DCHECK(PyType_Check(type_object));
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 10
  type_object->tp_flags |= Py_TPFLAGS_MAPPING;
#endif
}
} // namespace apache::thrift::python

namespace apache::thrift::python::capi {
PyObject* FOLLY_NULLABLE getThriftData(PyObject* structOrUnion) {
  if (!ensure_module_imported()) {
    return nullptr;
  }
  return _get_fbthrift_data(structOrUnion);
}
PyObject* FOLLY_NULLABLE getExceptionThriftData(PyObject* generatedError) {
  if (!ensure_module_imported()) {
    return nullptr;
  }
  return _get_exception_fbthrift_data(generatedError);
}

/**
 * This is a cpp version is set_struct_field in .pyx, but it saves overhead
 * of checking PyErr_Occurred() that would be necessary with every capi call
 * because the cython version is `except *`.
 *
 * Also, this assumes that struct_tuple has been created from PyTuple_New
 * without setting any fields. If this is used with a struct_tuple created
 * from python, it will leak the old value at index.
 */
int setStructField(PyObject* struct_tuple, int16_t index, PyObject* value) {
  try {
    DCHECK_GT(index, 0);
    setStructIsset(struct_tuple, index - 1, 1);
  } catch (std::runtime_error& e) {
    // In error case, folly::handlePythonError clears error indicator
    // and throws std::runtime_error with message fetched from PyErr.
    //
    PyErr_SetString(PyExc_TypeError, e.what());
    return -1;
  }
  Py_INCREF(value);
  PyTuple_SET_ITEM(struct_tuple, index, value);
  return 0;
}

/**
 * This is a cpp version of Union._fbthrift_update_type_value, but it avoids the
 * overhead of checking PyErr_Occurred(), similar to setStructField.
 */
PyObject* unionTupleFromValue(int64_t type_key, PyObject* value) {
  PyObject* union_tuple = PyTuple_New(2);
  if (union_tuple == nullptr) {
    return nullptr;
  }
  PyObject* py_tag = PyLong_FromLong(type_key);
  if (py_tag == nullptr) {
    Py_DECREF(union_tuple);
    return nullptr;
  }
  Py_INCREF(py_tag);
  PyTuple_SET_ITEM(union_tuple, 0, py_tag);
  Py_INCREF(value);
  PyTuple_SET_ITEM(union_tuple, 1, value);
  return union_tuple;
}

} // namespace apache::thrift::python::capi
