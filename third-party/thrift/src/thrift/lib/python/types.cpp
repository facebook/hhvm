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
#include <thrift/lib/python/types_api.h> // @manual

#include <folly/Indestructible.h>
#include <folly/Range.h>
#include <folly/ScopeGuard.h>
#include <folly/lang/New.h>
#include <folly/python/import.h>
#include <thrift/lib/cpp2/protocol/TableBasedSerializer.h>

namespace apache {
namespace thrift {
namespace python {

constexpr const size_t kHeadOffset = sizeof(PyVarObject);
constexpr const size_t kFieldOffset = sizeof(PyObject*);

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

/**
 * Returns the appropriate default value for a thrift field of the given type.
 *
 * The returned value will either be the one provided by the user (in the Thrift
 * IDL), or the standard default value for the given `typeInfo`.
 *
 * @param `index` of the field in `userDefaultValues`, i.e. insertion order (NOT
 *        field ID).
 *
 * @throws if the thrift python types module could not be imported.
 */
UniquePyObjectPtr getDefaultValue(
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

  // 2. Check local cache for an existing default value.
  static folly::Indestructible<
      std::unordered_map<const detail::TypeInfo*, PyObject*>>
      defaultValueCache;
  auto cachedDefaultValueIt = defaultValueCache->find(typeInfo);
  if (cachedDefaultValueIt != defaultValueCache->end()) {
    UniquePyObjectPtr value(UniquePyObjectPtr(cachedDefaultValueIt->second));
    Py_INCREF(value.get());
    return value;
  }

  // 3. No cached value found. Determine the default value, and update cache (if
  // applicable).
  UniquePyObjectPtr value;
  bool addValueToCache = true;
  switch (typeInfo->type) {
    case protocol::TType::T_BYTE:
    case protocol::TType::T_I16:
    case protocol::TType::T_I32:
    case protocol::TType::T_I64:
      // For integral values, the default is `0L`.
      value = UniquePyObjectPtr(PyLong_FromLong(0));
      break;
    case protocol::TType::T_DOUBLE:
    case protocol::TType::T_FLOAT:
      // For floating point values, the default is `0d`.
      value = UniquePyObjectPtr(PyFloat_FromDouble(0));
      break;
    case protocol::TType::T_BOOL:
      // For booleans, the default is `false`.
      value = UniquePyObjectPtr(Py_False);
      Py_INCREF(Py_False);
      break;
    case protocol::TType::T_STRING:
      // For strings, the default value is the empty string (or, if `IOBuf`s are
      // used, an empty `IOBuf`).
      switch (*static_cast<const detail::StringFieldType*>(typeInfo->typeExt)) {
        case detail::StringFieldType::String:
        case detail::StringFieldType::StringView:
        case detail::StringFieldType::Binary:
        case detail::StringFieldType::BinaryStringView:
          value = UniquePyObjectPtr(PyBytes_FromString(""));
          break;
        case detail::StringFieldType::IOBuf:
        case detail::StringFieldType::IOBufPtr:
        case detail::StringFieldType::IOBufObj:
          auto buf = create_IOBuf(folly::IOBuf::create(0));
          value = UniquePyObjectPtr(buf);
          addValueToCache = false;
          break;
      }
      break;
    case protocol::TType::T_LIST:
    case protocol::TType::T_MAP:
      // For lists and maps, the default value is an empty tuple.
      value = UniquePyObjectPtr(PyTuple_New(0));
      break;
    case protocol::TType::T_SET:
      // For sets, the default value is an empty `frozenset`.
      value = UniquePyObjectPtr(PyFrozenSet_New(nullptr));
      break;
    case protocol::TType::T_STRUCT: {
      // For struct and unions, the default value is a (recursively)
      // default-initialized instance.
      auto structInfo =
          static_cast<const detail::StructInfo*>(typeInfo->typeExt);
      value = UniquePyObjectPtr(
          structInfo->unionExt != nullptr
              ? createUnionTuple()
              : createStructTupleWithDefaultValues(*structInfo));
      break;
    }
    default:
      LOG(FATAL) << "invalid typeInfo TType " << typeInfo->type;
  }
  if (value == nullptr) {
    THRIFT_PY3_CHECK_ERROR();
  }

  if (addValueToCache) {
    defaultValueCache->emplace(typeInfo, value.get());
    Py_INCREF(value.get());
  }
  return value;
}

} // namespace

PyObject* createUnionTuple() {
  UniquePyObjectPtr tuple{
      PyTuple_New(2)}; // one for the type and the other for the value
  if (!tuple) {
    THRIFT_PY3_CHECK_ERROR();
  }
  PyTuple_SET_ITEM(
      tuple.get(), 0, PyLong_FromLong(0)); // init type to __EMPTY__ (0)
  PyTuple_SET_ITEM(tuple.get(), 1, Py_None); // init value to None
  Py_INCREF(Py_None);
  return tuple.release();
}

PyObject* createStructTuple(int16_t numFields) {
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

  // Create tuple, with isset byte array as first element (followed by
  // `numFields` uninitialized elements).
  PyObject* tuple{PyTuple_New(numFields + 1)};
  if (tuple == nullptr) {
    return nullptr;
  }
  PyTuple_SET_ITEM(tuple, 0, issetArr.release());
  return tuple;
}

PyObject* createStructTupleWithDefaultValues(
    const detail::StructInfo& structInfo) {
  const int16_t numFields = structInfo.numFields;
  UniquePyObjectPtr tuple{createStructTuple(numFields)};
  if (tuple == nullptr) {
    THRIFT_PY3_CHECK_ERROR();
  }

  // Initialize tuple[1:numFields+1] with default field values.
  const auto& defaultValues =
      *static_cast<const FieldValueMap*>(structInfo.customExt);
  for (int i = 0; i < numFields; ++i) {
    const detail::FieldInfo& fieldInfo = structInfo.fieldInfos[i];
    if (fieldInfo.qualifier == detail::FieldQualifier::Optional) {
      PyTuple_SET_ITEM(tuple.get(), i + 1, Py_None);
      Py_INCREF(Py_None);
    } else {
      PyTuple_SET_ITEM(
          tuple.get(),
          i + 1,
          getDefaultValue(fieldInfo.typeInfo, defaultValues, i).release());
    }
  }
  return tuple.release();
}

void setStructIsset(void* object, int16_t index, bool set) {
  PyObject** isset_bytes_ptr =
      toPyObjectPtr(static_cast<char*>(object) + kHeadOffset);
  char* flags = PyBytes_AsString(*isset_bytes_ptr);
  if (!flags) {
    THRIFT_PY3_CHECK_ERROR();
  }
  flags[index] = set;
}

void* setStruct(void* object, const detail::TypeInfo& typeInfo) {
  return setPyObject(
      object,
      UniquePyObjectPtr{createStructTupleWithDefaultValues(
          *static_cast<const detail::StructInfo*>(typeInfo.typeExt))});
}

void* setUnion(void* object, const detail::TypeInfo& /* typeInfo */) {
  return setPyObject(object, UniquePyObjectPtr{createUnionTuple()});
}

bool getIsset(const void* object, ptrdiff_t offset) {
  PyObject* isset =
      *toPyObjectPtr(static_cast<const char*>(object) + kHeadOffset);
  const char* flags = PyBytes_AsString(isset);
  if (!flags) {
    THRIFT_PY3_CHECK_ERROR();
  }
  return flags[offset];
}

void setIsset(void* object, ptrdiff_t offset, bool set) {
  return setStructIsset(object, offset, set);
}

void clearUnion(void* object) {
  PyObject* pyObj = toPyObject(object);
  PyObject* oldType = PyTuple_GET_ITEM(pyObj, 0);
  UniquePyObjectPtr zero{PyLong_FromLong(0)};
  if (!zero) {
    THRIFT_PY3_CHECK_ERROR();
  }
  PyTuple_SET_ITEM(pyObj, 0, zero.release());
  Py_XDECREF(oldType);
  PyObject* oldValue = PyTuple_GET_ITEM(pyObj, 1);
  PyTuple_SET_ITEM(pyObj, 1, Py_None);
  Py_INCREF(Py_None);
  Py_XDECREF(oldValue);
}

int getActiveId(const void* object) {
  PyObject* const* pyObj = toPyObjectPtr(object);
  auto id = PyLong_AsLong(PyTuple_GET_ITEM(pyObj, 0));
  if (id == -1) {
    THRIFT_PY3_CHECK_POSSIBLE_ERROR();
  }
  return id;
}

void setActiveId(void* object, int value) {
  PyObject* pyObj = toPyObject(object);
  PyObject* oldValue = PyTuple_GET_ITEM(pyObj, 0);
  UniquePyObjectPtr valueObj{PyLong_FromLong(value)};
  if (!valueObj) {
    THRIFT_PY3_CHECK_ERROR();
  }
  PyTuple_SET_ITEM(pyObj, 0, valueObj.release());
  Py_DECREF(oldValue);
}

const detail::UnionExtN<1> unionExt = {
    /* .clear */ clearUnion,
    /* .unionTypeOffset */ 0,
    /* .getActiveId */ getActiveId,
    /* .setActiveId */ setActiveId,
    /* .initMember */ {nullptr},
};

detail::OptionalThriftValue getString(
    const void* object, const detail::TypeInfo& /* typeInfo */) {
  PyObject* pyObj = *toPyObjectPtr(object);
  Py_ssize_t len = 0;
  char* buf = nullptr;
  if (PyBytes_AsStringAndSize(pyObj, &buf, &len) == -1) {
    THRIFT_PY3_CHECK_ERROR();
  }
  return folly::make_optional<detail::ThriftValue>(
      folly::StringPiece{buf, static_cast<std::size_t>(len)});
}

void setString(void* object, const std::string& value) {
  UniquePyObjectPtr bytesObj{
      PyBytes_FromStringAndSize(value.data(), value.size())};
  if (!bytesObj) {
    THRIFT_PY3_CHECK_ERROR();
  }
  setPyObject(object, std::move(bytesObj));
}

detail::OptionalThriftValue getIOBuf(
    const void* object, const detail::TypeInfo& /* typeInfo */) {
  ensureImportOrThrow();
  PyObject* pyObj = *toPyObjectPtr(object);
  folly::IOBuf* buf = pyObj != nullptr ? get_cIOBuf(pyObj) : nullptr;
  return buf ? folly::make_optional<detail::ThriftValue>(buf)
             : detail::OptionalThriftValue{};
}

void setIOBuf(void* object, const folly::IOBuf& value) {
  ensureImportOrThrow();
  const auto buf = create_IOBuf(value.clone());
  UniquePyObjectPtr iobufObj{buf};
  if (!buf) {
    THRIFT_PY3_CHECK_ERROR();
  }
  setPyObject(object, std::move(iobufObj));
}

detail::OptionalThriftValue getStruct(
    const void* object, const detail::TypeInfo& /* typeInfo */) {
  PyObject* pyObj = *toPyObjectPtr(object);
  return folly::make_optional<detail::ThriftValue>(pyObj);
}

detail::TypeInfo createStructTypeInfo(
    const DynamicStructInfo& dynamicStructInfo) {
  return {
      /* .type */ protocol::TType::T_STRUCT,
      /* .get */ getStruct,
      /* .set */
      reinterpret_cast<detail::VoidFuncPtr>(
          dynamicStructInfo.isUnion() ? setUnion : setStruct),
      /* .typeExt */ &dynamicStructInfo.getStructInfo(),
  };
}

void ListTypeInfo::read(
    const void* context,
    void* object,
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
  setPyObject(object, std::move(list));
}

size_t ListTypeInfo::write(
    const void* context,
    const void* object,
    size_t (*writer)(const void* /*context*/, const void* /*val*/)) {
  const PyObject* list = toPyObject(object);
  auto size = PyTuple_GET_SIZE(list);
  size_t written = 0;
  for (Py_ssize_t i = 0; i < size; i++) {
    auto elem = PyTuple_GET_ITEM(list, i);
    written += writer(context, &elem);
  }
  return written;
}

void ListTypeInfo::consumeElem(
    const void* context,
    void* object,
    void (*reader)(const void* /*context*/, void* /*val*/)) {
  PyObject* elem = nullptr;
  reader(context, &elem);
  PyObject** pyObjPtr = toPyObjectPtr(object);
  auto currentSize = PyTuple_GET_SIZE(*pyObjPtr);
  if (_PyTuple_Resize(pyObjPtr, currentSize + 1) == -1) {
    THRIFT_PY3_CHECK_ERROR();
  }

  PyTuple_SET_ITEM(*pyObjPtr, currentSize, elem);
}

void SetTypeInfo::read(
    const void* context,
    void* object,
    std::uint32_t setSize,
    void (*reader)(const void* /*context*/, void* /*val*/)) {
  UniquePyObjectPtr frozenset{PyFrozenSet_New(nullptr)};
  if (!frozenset) {
    THRIFT_PY3_CHECK_ERROR();
  }
  for (std::uint32_t i = 0; i < setSize; ++i) {
    PyObject* elem{};
    reader(context, &elem);
    if (PySet_Add(frozenset.get(), elem) == -1) {
      THRIFT_PY3_CHECK_ERROR();
    }
    Py_DECREF(elem);
  }
  setPyObject(object, std::move(frozenset));
}

size_t SetTypeInfo::write(
    const void* context,
    const void* object,
    bool protocolSortKeys,
    size_t (*writer)(const void* /*context*/, const void* /*val*/)) {
  size_t written = 0;
  PyObject* set = const_cast<PyObject*>(toPyObject(object));
  UniquePyObjectPtr iter;
  if (protocolSortKeys) {
    UniquePyObjectPtr seq{PySequence_List(set)};
    if (!seq) {
      THRIFT_PY3_CHECK_ERROR();
    }
    if (PyList_Sort(seq.get()) == -1) {
      THRIFT_PY3_CHECK_ERROR();
    }
    iter = UniquePyObjectPtr{PyObject_GetIter(seq.get())};
  } else {
    iter = UniquePyObjectPtr{PyObject_GetIter(set)};
  }
  if (!iter) {
    THRIFT_PY3_CHECK_ERROR();
  }
  PyObject* elem;
  while ((elem = PyIter_Next(iter.get())) != nullptr) {
    written += writer(context, &elem);
    Py_DECREF(elem);
  }
  return written;
}

// keep until python3.9, where Py_SET_REFCNT is available officially
inline void _fbthrift_Py_SET_REFCNT(PyObject* ob, Py_ssize_t refcnt) {
  ob->ob_refcnt = refcnt;
}

void SetTypeInfo::consumeElem(
    const void* context,
    void* object,
    void (*reader)(const void* /*context*/, void* /*val*/)) {
  PyObject** pyObjPtr = toPyObjectPtr(object);
  DCHECK(*pyObjPtr);
  PyObject* elem = nullptr;
  reader(context, &elem);
  DCHECK(elem);
  // This is nasty hack since Cython generated code will incr the refcnt
  // so PySet_Add will fail. Need to temporarily decrref.
  auto currentRefCnt = Py_REFCNT(*pyObjPtr);
  _fbthrift_Py_SET_REFCNT(*pyObjPtr, 1);
  if (PySet_Add(*pyObjPtr, elem) == -1) {
    _fbthrift_Py_SET_REFCNT(*pyObjPtr, currentRefCnt);
    THRIFT_PY3_CHECK_ERROR();
  }
  Py_DECREF(elem);
  _fbthrift_Py_SET_REFCNT(*pyObjPtr, currentRefCnt);
}

void MapTypeInfo::read(
    const void* context,
    void* object,
    std::uint32_t mapSize,
    void (*keyReader)(const void* context, void* key),
    void (*valueReader)(const void* context, void* val)) {
  // use a tuple to represent a map for immutablitity and hashability
  UniquePyObjectPtr map{PyTuple_New(mapSize)};
  if (!map) {
    THRIFT_PY3_CHECK_ERROR();
  }
  for (std::uint32_t i = 0; i < mapSize; ++i) {
    auto read = [=](auto reader) {
      PyObject* obj = nullptr;
      reader(context, &obj);
      return UniquePyObjectPtr(obj);
    };
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
  setPyObject(object, std::move(map));
}

size_t MapTypeInfo::write(
    const void* context,
    const void* object,
    bool protocolSortKeys,
    size_t (*writer)(
        const void* context, const void* keyElem, const void* valueElem)) {
  size_t written = 0;
  PyObject* map = const_cast<PyObject*>(toPyObject(object));
  auto size = PyTuple_GET_SIZE(map);
  UniquePyObjectPtr seq;
  if (protocolSortKeys) {
    seq = UniquePyObjectPtr{PySequence_List(map)};
    if (!seq) {
      THRIFT_PY3_CHECK_ERROR();
    }
    map = seq.get();
    if (PyList_Sort(map) == -1) {
      THRIFT_PY3_CHECK_ERROR();
    }
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
    void* object,
    void (*keyReader)(const void* context, void* key),
    void (*valueReader)(const void* context, void* val)) {
  PyObject** pyObjPtr = toPyObjectPtr(object);
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
  auto currentSize = PyTuple_GET_SIZE(*pyObjPtr);
  if (_PyTuple_Resize(pyObjPtr, currentSize + 1) == -1) {
    THRIFT_PY3_CHECK_ERROR();
  }
  PyTuple_SET_ITEM(*pyObjPtr, currentSize, elem.release());
}

DynamicStructInfo::DynamicStructInfo(
    const char* name, int16_t numFields, bool isUnion)
    : name_{name} {
  structInfo_ = static_cast<detail::StructInfo*>(folly::operator_new(
      sizeof(detail::StructInfo) + sizeof(detail::FieldInfo) * numFields,
      folly::align_val_t{alignof(detail::StructInfo)}));
  // reserve vector as we are assigning const char* from the string in
  // vector
  fieldNames_.reserve(numFields);
  structInfo_->numFields = numFields;
  structInfo_->name = name_.c_str();
  structInfo_->unionExt =
      isUnion ? reinterpret_cast<const detail::UnionExt*>(&unionExt) : nullptr;
  structInfo_->getIsset = getIsset;
  structInfo_->setIsset = setIsset;
  structInfo_->customExt = &fieldValues_;
}

DynamicStructInfo::~DynamicStructInfo() {
  for (auto kv : fieldValues_) {
    Py_DECREF(kv.second);
  }
  folly::operator_delete(
      structInfo_,
      sizeof(detail::StructInfo) +
          sizeof(detail::FieldInfo) * structInfo_->numFields,
      folly::align_val_t{alignof(detail::StructInfo)});
}

void DynamicStructInfo::addFieldInfo(
    detail::FieldID id,
    detail::FieldQualifier qualifier,
    const char* name,
    const detail::TypeInfo* typeInfo) {
  fieldNames_.push_back(name);
  int16_t idx = fieldNames_.size() - 1;
  structInfo_->fieldInfos[idx] = detail::FieldInfo{
      /* .id */ id,
      /* .qualifier */ qualifier,
      /* .name */ fieldNames_[idx].c_str(),
      /* .memberOffset */
      static_cast<ptrdiff_t>(
          kHeadOffset + kFieldOffset * (isUnion() ? 1 : idx + 1)),
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
    PrimitiveTypeInfo<bool, protocol::TType::T_BOOL>::typeInfo;
const detail::TypeInfo& byteTypeInfo =
    PrimitiveTypeInfo<std::int8_t, protocol::TType::T_BYTE>::typeInfo;
const detail::TypeInfo& i16TypeInfo =
    PrimitiveTypeInfo<std::int16_t, protocol::TType::T_I16>::typeInfo;
const detail::TypeInfo& i32TypeInfo =
    PrimitiveTypeInfo<std::int32_t, protocol::TType::T_I32>::typeInfo;
const detail::TypeInfo& i64TypeInfo =
    PrimitiveTypeInfo<std::int64_t, protocol::TType::T_I64>::typeInfo;
const detail::TypeInfo& doubleTypeInfo =
    PrimitiveTypeInfo<double, protocol::TType::T_DOUBLE>::typeInfo;
const detail::TypeInfo& floatTypeInfo =
    PrimitiveTypeInfo<float, protocol::TType::T_FLOAT>::typeInfo;

const detail::StringFieldType stringFieldType =
    detail::StringFieldType::StringView;

const detail::StringFieldType binaryFieldType =
    detail::StringFieldType::BinaryStringView;

const detail::StringFieldType ioBufFieldType =
    detail::StringFieldType::IOBufObj;

const detail::TypeInfo stringTypeInfo{
    /* .type */ protocol::TType::T_STRING,
    /* .get */ getString,
    /* .set */ reinterpret_cast<detail::VoidFuncPtr>(setString),
    /* .typeExt */ &stringFieldType,
};

const detail::TypeInfo binaryTypeInfo{
    /* .type */ protocol::TType::T_STRING,
    /* .get */ getString,
    /* .set */ reinterpret_cast<detail::VoidFuncPtr>(setString),
    /* .typeExt */ &binaryFieldType,
};

const detail::TypeInfo iobufTypeInfo{
    /* .type */ protocol::TType::T_STRING,
    /* .get */ getIOBuf,
    /* .set */ reinterpret_cast<detail::VoidFuncPtr>(setIOBuf),
    /* .typeExt */ &ioBufFieldType,
};

namespace capi {
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

} // namespace capi

} // namespace python
} // namespace thrift
} // namespace apache
