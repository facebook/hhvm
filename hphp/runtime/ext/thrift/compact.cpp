/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/thrift/ext_thrift.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/strings.h"

#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/ext/reflection/ext_reflection.h"
#include "hphp/runtime/ext/thrift/adapter.h"
#include "hphp/runtime/ext/thrift/field_wrapper.h"
#include "hphp/runtime/ext/thrift/type_wrapper.h"
#include "hphp/runtime/ext/thrift/spec-holder.h"
#include "hphp/runtime/ext/thrift/transport.h"
#include "hphp/runtime/ext/thrift/util.h"

#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/runtime/vm/coeffects.h"

#include "hphp/runtime/vm/jit/perf-counters.h"

#include "hphp/util/fixed-vector.h"
#include "hphp/util/rds-local.h"

#include <cstdint>
#include <folly/AtomicHashMap.h>
#include <folly/Bits.h>
#include <folly/Format.h>
#include <folly/Likely.h>

#include <limits>
#include <stack>
#include <type_traits>
#include <utility>

namespace HPHP::thrift {
/////////////////////////////////////////////////////////////////////////////

namespace {
const StaticString SKIP_CHECKS_ATTR("ThriftDeprecatedSkipSerializerChecks");

// Assumes that at least 10 bytes available in the input buffer
static int64_t readVarintFast(const uint8_t **ptr) {
  uint8_t byte;
  uint64_t result;

#ifndef __AVX2__

  byte = *((*ptr)++);
  result = (uint64_t)(byte & 0x7f);
  if ((byte & 0x80) == 0) goto ret;
  // Byte 2
  byte = *((*ptr)++);
  result = result | (uint64_t)(byte & 0x7f) << 7;
  if ((byte & 0x80) == 0) goto ret;
  // Byte 3
  byte = *((*ptr)++);
  result = result | (uint64_t)(byte & 0x7f) << 14;
  if ((byte & 0x80) == 0) goto ret;
  // Byte 4
  byte = *((*ptr)++);
  result = result | (uint64_t)(byte & 0x7f) << 21;
  if ((byte & 0x80) == 0) goto ret;

#else

  // Optimization for single byte values
  if (!(**ptr & 0x80)) {
    return static_cast<int64_t>(*((*ptr)++));
  }
  uint64_t v;
  memcpy(&v, *ptr, sizeof(uint64_t));
  const size_t l = _tzcnt_u64(~v & 0x8080808080808080ULL) / 8;
  if (l == 1) {
    *ptr += 2;
    return static_cast<int64_t>(_pext_u64(v, 0x7f7fULL));
  }
  if (l == 2) {
    *ptr += 3;
    return static_cast<int64_t>(_pext_u64(v, 0x7f7f7fULL));
  }
  if (l == 3) {
    *ptr += 4;
    return static_cast<int64_t>(_pext_u64(v, 0x7f7f7f7fULL));
  }
  *ptr += 4;
  result = _pext_u64(v, 0x7f7f7f7fULL);

#endif // __AVX2__

  // Byte 5
  byte = *((*ptr)++);
  result = result | (uint64_t)(byte & 0x7f) << 28;
  if ((byte & 0x80) == 0) goto ret;
  // Byte 6
  byte = *((*ptr)++);
  result = result | (uint64_t)(byte & 0x7f) << 35;
  if ((byte & 0x80) == 0) goto ret;
  // Byte 7
  byte = *((*ptr)++);
  result = result | (uint64_t)(byte & 0x7f) << 42;
  if ((byte & 0x80) == 0) goto ret;
  // Byte 8
  byte = *((*ptr)++);
  result = result | (uint64_t)(byte & 0x7f) << 49;
  if ((byte & 0x80) == 0) goto ret;
  // Byte 9
  byte = *((*ptr)++);
  result = result | (uint64_t)(byte & 0x7f) << 56;
  if ((byte & 0x80) == 0) goto ret;
  // Byte 10
  byte = *((*ptr)++);
  result = result | (uint64_t)(byte & 0x7f) << 63;
  if ((byte & 0x80) == 0) goto ret;

  thrift_error("Variable-length int over 10 bytes", ERR_INVALID_DATA);

ret:
  return result;
}

}

const uint8_t VERSION_MASK = 0x1f;
const uint8_t VERSION = 2;
const uint8_t VERSION_LOW = 1;
const uint8_t VERSION_DOUBLE_BE = 2;
const uint8_t PROTOCOL_ID = 0x82;
const uint8_t TYPE_MASK = 0xe0;
const uint8_t TYPE_SHIFT_AMOUNT = 5;

enum CState {
  STATE_CLEAR,
  STATE_FIELD_WRITE,
  STATE_VALUE_WRITE,
  STATE_CONTAINER_WRITE,
  STATE_BOOL_WRITE,
  STATE_FIELD_READ,
  STATE_CONTAINER_READ,
  STATE_VALUE_READ,
  STATE_BOOL_READ
};

enum CType {
  C_STOP = 0x00,
  C_TRUE = 0x01,
  C_FALSE = 0x02,
  C_BYTE = 0x03,
  C_I16 = 0x04,
  C_I32 = 0x05,
  C_I64 = 0x06,
  C_DOUBLE = 0x07,
  C_BINARY = 0x08,
  C_LIST = 0x09,
  C_SET = 0x0A,
  C_MAP = 0x0B,
  C_STRUCT = 0x0C,
  C_FLOAT = 0x0D
};

enum CListType {
  C_LIST_LIST,
  C_LIST_SET
};

enum TResponseType {
  T_REPLY = 2,
  T_EXCEPTION = 3
};

static CType ttype_to_ctype(TType x) {
  switch (x) {
    case T_STOP:
      return C_STOP;
    case T_BOOL:
      return C_TRUE;
    case T_BYTE:
      return C_BYTE;
    case T_I16:
      return C_I16;
    case T_I32:
      return C_I32;
    case T_I64:
      return C_I64;
    case T_DOUBLE:
      return C_DOUBLE;
    case T_STRING:
      return C_BINARY;
    case T_STRUCT:
      return C_STRUCT;
    case T_LIST:
      return C_LIST;
    case T_SET:
      return C_SET;
    case T_MAP:
      return C_MAP;
    case T_FLOAT:
      return C_FLOAT;
    default:
      thrift_error(
        folly::to<std::string>(
          "Unknown Thrift data type ", static_cast<int>(x)),
        ERR_INVALID_DATA);
  }
}

static const TType s_ctype_to_ttype_map[14] {
  T_STOP,
  T_BOOL,
  T_BOOL,
  T_BYTE,
  T_I16,
  T_I32,
  T_I64,
  T_DOUBLE,
  T_STRING,
  T_LIST,
  T_SET,
  T_MAP,
  T_STRUCT,
  T_FLOAT
};

static TType ctype_to_ttype(CType x) {
  uint8_t index = static_cast<uint8_t>(x);
  if (UNLIKELY(index > C_FLOAT)) {
    thrift_error(
      folly::to<std::string>(
        "Unknown Compact data type ", static_cast<int>(x)),
      ERR_INVALID_DATA);
  }
  return s_ctype_to_ttype_map[index];
}

struct CompactRequestData final : RequestEventHandler {
  CompactRequestData() : version(VERSION) { }
  void clear() { version = VERSION; }

  void requestInit() override {
    clear();
  }

  void requestShutdown() override {
    clear();
  }

  uint8_t version;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(CompactRequestData, s_compact_request_data);

namespace {
struct FieldInfo {
  Class* cls = nullptr;
  // A pointer to a property which may be set lazily by calling
  // cls->lookupDeclProp(fieldName) first time we need the property.
  mutable const Class::Prop *prop = nullptr;
  const StringData* fieldName = nullptr;
  int16_t fieldNum = 0;

  const Class::Prop* getProp() const {
    if (!prop) {
      auto slot = cls->lookupDeclProp(fieldName);
      if (slot != kInvalidSlot) {
        prop = &cls->declProperties()[slot];
      }
    }
    return prop;
  }
};
}

struct CompactWriter {
    explicit CompactWriter(PHPOutputTransport *transport) :
      transport(transport),
      version(VERSION),
      state(STATE_CLEAR),
      lastFieldNum(0),
      boolFieldNum(0),
      structHistory(),
      containerHistory() {
    }

    void setWriteVersion(uint8_t _version) {
      version = _version;
    }

    void writeHeader(const String& name, uint8_t msgtype, uint32_t seqid) {
      writeUByte(PROTOCOL_ID);
      writeUByte(version | (msgtype << TYPE_SHIFT_AMOUNT));
      writeVarint(seqid);
      writeString(name);

      state = STATE_VALUE_WRITE;
    }

    void write(const Object& obj) {
      writeStruct(obj);
    }

  private:
    PHPOutputTransport* transport;

    uint8_t version;
    CState state;
    uint16_t lastFieldNum;
    uint16_t boolFieldNum;
    using StructHistoryState = std::pair<CState, uint16_t>;
    std::stack<StructHistoryState, std::vector<StructHistoryState>> structHistory;
    std::stack<CState> containerHistory;

    void writeSlow(const FieldSpec& field, const Object& obj) {
      INC_TPC(thrift_write_slow);
      StrNR fieldName(field.name);
      Variant fieldVal;
      if (field.isWrapped) {
        fieldVal = getThriftType(obj, fieldName);
      } else {
        fieldVal = obj->o_get(fieldName, true, obj->getClassName());
      }
      if (!fieldVal.isNull()) {
        TType fieldType = field.type;
        writeFieldBegin(field.fieldNum, fieldType);
        auto fieldInfo = FieldInfo();
        fieldInfo.cls = obj->getVMClass();
        fieldInfo.fieldName = field.name;
        fieldInfo.fieldNum = field.fieldNum;
        writeField(fieldVal, field, fieldType, fieldInfo);
        writeFieldEnd();
      }
    }

    void writeStruct(const Object& obj) {
      // Save state
      structHistory.emplace(state, lastFieldNum);
      state = STATE_FIELD_WRITE;
      lastFieldNum = 0;

      // Get field specification
      Class* cls = obj->getVMClass();
      SpecHolder specHolder;
      auto const& fields = specHolder.getSpec(cls).fields;
      auto prop = cls->declProperties().begin();
      obj->deserializeAllLazyProps();
      auto objProps = obj->props();
      const size_t numProps = cls->numDeclProperties();
      const size_t numFields = fields.size();
      // Write each member
      for (int slot = 0; slot < numFields; ++slot) {
        if (slot < numProps && fields[slot].name == prop[slot].name) {
          auto index = cls->propSlotToIndex(slot);
          Variant fieldVal;
          if (fields[slot].isWrapped) {
            fieldVal = getThriftType(obj, StrNR(fields[slot].name));
          } else {
            fieldVal = VarNR{objProps->at(index).tv()};
          }
          if (!fieldVal.isNull()) {
            TType fieldType = fields[slot].type;
            if (fields[slot].isTypeWrapped && fieldVal.isObject()) {
              fieldVal = getThriftField(fieldVal.toObject());
            }
            if (fields[slot].adapter) {
              fieldVal = transformToThriftType(fieldVal, *fields[slot].adapter);
            }
            if(fields[slot].isTerse && is_value_type_default(fieldType, fieldVal)) {
              continue;
            }
            writeFieldBegin(fields[slot].fieldNum, fieldType);
            auto fieldInfo = FieldInfo();
            fieldInfo.cls = cls;
            fieldInfo.prop = &prop[slot];
            fieldInfo.fieldNum = fields[slot].fieldNum;
            writeFieldInternal(fieldVal, fields[slot], fieldType, fieldInfo);
            writeFieldEnd();
          } else if (UNLIKELY(fieldVal.is(KindOfUninit)) &&
                     (prop[slot].attrs & AttrLateInit)) {
            throw_late_init_prop(prop[slot].cls, prop[slot].name, false);
          }
        } else {
          writeSlow(fields[slot], obj);
        }
      }

      // Write stop
      writeUByte(0);

      // Restore state
      std::pair<CState, uint16_t> prev = structHistory.top();
      state = prev.first;
      lastFieldNum = prev.second;
      structHistory.pop();
    }

    void writeFieldBegin(uint16_t fieldNum, TType fieldType) {
      if (fieldType == T_BOOL) {
        state = STATE_BOOL_WRITE;
        boolFieldNum = fieldNum;
        // the value and header are written together in writeField
      } else {
        state = STATE_VALUE_WRITE;
        writeFieldHeader(fieldNum, ttype_to_ctype(fieldType));
      }
    }

    void writeFieldEnd(void) {
      state = STATE_FIELD_WRITE;
    }

    void writeFieldHeader(uint16_t fieldNum, CType fieldType) {
      int delta = fieldNum - lastFieldNum;

      if (0 < delta && delta <= 15) {
        writeUByte((delta << 4) | fieldType);
      } else {
        writeUByte(fieldType);
        writeI(fieldNum);
      }

      lastFieldNum = fieldNum;
    }

    void writeField(const Variant& value,
                    const FieldSpec& valueSpec,
                    TType type,
                    const FieldInfo& fieldInfo) {
      if (valueSpec.isTypeWrapped && value.isObject()) {
        writeField(getThriftField(value.toObject()), valueSpec, type, fieldInfo);
        return;
      }
      if (valueSpec.adapter) {
        const auto& thriftValue = transformToThriftType(value, *valueSpec.adapter);
        writeFieldInternal(thriftValue, valueSpec, type, fieldInfo);
      } else {
        writeFieldInternal(value, valueSpec, type, fieldInfo);
      }
    }

    void writeFieldInternal(const Variant& value,
                            const FieldSpec& valueSpec,
                            TType type,
                            const FieldInfo& fieldInfo) {
      switch (type) {
        case T_STOP:
        case T_VOID:
          break;

        case T_STRUCT:
          if (!value.is(KindOfObject)) {
            thrift_error("Attempt to send non-object type as T_STRUCT",
              ERR_INVALID_DATA);
          }
          writeStruct(value.toObject());
          break;

        case T_BOOL: {
            bool b = value.toBoolean();

            if (state == STATE_BOOL_WRITE) {
              CType t = b ? C_TRUE : C_FALSE;
              writeFieldHeader(boolFieldNum, t);
            } else if (state == STATE_CONTAINER_WRITE) {
              writeUByte(b ? C_TRUE : C_FALSE);
            } else {
              thrift_error("Invalid state in compact protocol", ERR_UNKNOWN);
            }
          }
          break;

        case T_BYTE:
          writeUByte((char)value.toInt64());
          break;

        case T_I16:
          writeIChecked<std::int16_t>(value, fieldInfo);
          break;
        case T_I32:
          writeIChecked<std::int32_t>(value, fieldInfo);
          break;
        case T_I64:
        case T_U64:
          writeI(value.toInt64());
          break;

        case T_DOUBLE: {
            double d = value.toDouble();
            uint64_t bits = folly::bit_cast<uint64_t>(d);
            if (version >= VERSION_DOUBLE_BE) {
              bits = htonll(bits);
            } else {
              bits = htolell(bits);
            }

            transport->write((char*)&bits, 8);
          }
          break;

        case T_FLOAT: {
            float d = (float)value.toDouble();
            uint32_t bits = htonl(folly::bit_cast<uint32_t>(d));
            transport->write((char*)&bits, 4);
          }
          break;

        case T_UTF8:
        case T_UTF16:
        case T_STRING: {
            if (value.is(KindOfObject)) {
              thrift_error("Attempt to send object type as a T_STRING",
                ERR_INVALID_DATA);
            }
            auto s = value.toString();
            auto slice = s.slice();
            writeVarint(slice.size());
            transport->write(slice.data(), slice.size());
            break;
          }

        case T_MAP:
          writeMap(value, valueSpec, fieldInfo);
          break;

        case T_LIST:
          writeList(value, valueSpec, C_LIST_LIST, fieldInfo);
          break;

        case T_SET:
          writeList(value, valueSpec, C_LIST_SET, fieldInfo);
          break;

        default:
          thrift_error("Unknown Thrift data type",
            ERR_INVALID_DATA);
      }
    }

    void writeMap(const Variant& map,
                  const FieldSpec& spec,
                  const FieldInfo& fieldInfo) {
      auto elemWriter = [&](TypedValue k, TypedValue v) {
        writeField(VarNR(k), spec.key(), spec.ktype, fieldInfo);
        writeField(VarNR(v), spec.val(), spec.vtype, fieldInfo);
        return false;
      };

      if (isContainer(map)) {
        writeMapBegin(spec.ktype, spec.vtype, getContainerSize(map));
        IterateKV(*map.asTypedValue(), elemWriter);
      } else {
        auto const arr = map.toArray();
        writeMapBegin(spec.ktype, spec.vtype, arr.size());
        IterateKV(arr.get(), elemWriter);
      }

      writeCollectionEnd();
    }

    void writeList(const Variant& list,
                   const FieldSpec& spec,
                   CListType listType,
                   const FieldInfo& fieldInfo) {
      auto const listWriter = [&](TypedValue v) {
        writeField(VarNR(v), spec.val(), spec.vtype, fieldInfo);
      };
      auto const setWriter = [&](TypedValue k, TypedValue /*v*/) {
        writeField(VarNR(k), spec.val(), spec.vtype, fieldInfo);
      };

      always_assert(listType == C_LIST_LIST ||
                    listType == C_LIST_SET);

      if (isContainer(list)) {
        writeListBegin(spec.vtype, getContainerSize(list));
        if (listType == C_LIST_LIST) {
          IterateV(*list.asTypedValue(), listWriter);
        } else {
          IterateKV(*list.asTypedValue(), setWriter);
        }
      } else {
        auto const arr = list.toArray();
        writeListBegin(spec.vtype, arr.size());
        if (listType == C_LIST_LIST) {
          IterateV(arr.get(), listWriter);
        } else {
          IterateKV(arr.get(), setWriter);
        }
      }

      writeCollectionEnd();
    }

    void writeMapBegin(TType keyType, TType valueType,  uint32_t size) {
      if (size == 0) {
        writeUByte(0);
      } else {
        writeVarint(size);

        CType keyCType = ttype_to_ctype(keyType);
        CType valueCType = ttype_to_ctype(valueType);
        writeUByte((keyCType << 4) | valueCType);
      }

      containerHistory.push(state);
      state = STATE_CONTAINER_WRITE;
    }

    void writeListBegin(TType elemType, uint32_t size) {
      if (size <= 14) {
        writeUByte((size << 4) | ttype_to_ctype(elemType));
      } else {
        writeUByte(0xf0 | ttype_to_ctype(elemType));
        writeVarint(size);
      }

      containerHistory.push(state);
      state = STATE_CONTAINER_WRITE;
    }

    void writeCollectionEnd(void) {
      state = containerHistory.top();
      containerHistory.pop();
    }

    void writeUByte(uint8_t n) {
      transport->writeI8(n);
    }

    void writeI(int64_t n) {
      writeVarint(i64ToZigzag(n));
    }

    template <typename T>
    void writeIChecked(const Variant& value, const FieldInfo& fieldInfo) {
      static_assert(std::is_integral<T>::value, "not an integral type");
      auto n = value.toInt64();
      using limits = std::numeric_limits<T>;
      if ((n < limits::min() || n > limits::max())) {
        const auto& structName = fieldInfo.cls->nameStr();
        std::string message = folly::sformat(
            "Value {} is out of range in field {} of {}",
            n,
            fieldInfo.fieldNum,
            structName.c_str());
        auto hasSkipChecksAttr = [&]() {
          const Class::Prop* prop = fieldInfo.getProp();
          return prop &&
            prop->preProp->userAttributes().count(
                LowStringPtr(SKIP_CHECKS_ATTR.get())) != 0;
        };
        if (hasSkipChecksAttr()) {
          raise_warning("[Suppressed] " + message);
        } else {
          thrift_error(message, ERR_INVALID_DATA);
        }
      }
      writeI(n);
    }

    void writeVarint(uint64_t n) {
      uint8_t buf[10];
      uint8_t wsize = 0;

      while (true) {
        if ((n & ~0x7FL) == 0) {
          buf[wsize++] = (int8_t)n;
          break;
        } else {
          buf[wsize++] = (int8_t)((n & 0x7F) | 0x80);
          n >>= 7;
        }
      }

      transport->write((char*)buf, wsize);
    }

    void writeString(const String& s) {
      auto slice = s.slice();
      writeVarint(slice.size());
      transport->write(slice.data(), slice.size());
    }

    uint64_t i64ToZigzag(int64_t n) {
      return (static_cast<uint64_t>(n) << 1) ^ (n >> 63);
    }
};

struct CompactReader {
    explicit CompactReader(const Object& _transportobj, int options) :
      transport(_transportobj),
      options(options),
      version(VERSION),
      state(STATE_CLEAR),
      lastFieldNum(0),
      boolValue(true),
      structHistory(),
      containerHistory() {
    }

    Variant read(const String& resultClassName) {
      uint8_t protoId = readUByte();
      if (protoId != PROTOCOL_ID) {
        thrift_error("Bad protocol id in TCompact message", ERR_BAD_VERSION);
      }

      uint8_t versionAndType = readUByte();
      uint8_t type = (versionAndType & TYPE_MASK) >> TYPE_SHIFT_AMOUNT;
      version = versionAndType & VERSION_MASK;
      if (version < VERSION_LOW || version > VERSION) {
        thrift_error("Bad version in TCompact message", ERR_BAD_VERSION);
      }

      // TODO: we eventually want to return seqid to the caller
      readVarint(); // seqid (unused)
      skip(T_STRING); // name (unused)

      if (type == T_REPLY) {
        return readStruct(resultClassName);
      } else if (type == T_EXCEPTION) {
        throw_object(readStruct(s_TApplicationException));
      } else {
        thrift_error("Invalid response type", ERR_INVALID_DATA);
      }
    }

    NEVER_INLINE
    void readStructSlow(const Object& dest,
                        const StructSpec& spec,
                        int16_t fieldNum,
                        TType fieldType) {
      INC_TPC(thrift_read_slow);
      while (fieldType != T_STOP) {
        bool readComplete = false;

        const auto* fieldSpec = getFieldSlow(spec, fieldNum);
        if (fieldSpec) {
          if (typesAreCompatible(fieldType, fieldSpec->type)) {
            readComplete = true;
            bool hasTypeWrapper = false;
            Variant fieldValue = readField(
              *fieldSpec, fieldType, hasTypeWrapper);
            if (hasTypeWrapper) {
              setThriftField(fieldValue, dest, StrNR(fieldSpec->name));
            } else if (fieldSpec->isWrapped) {
              setThriftType(fieldValue, dest, StrNR(fieldSpec->name));
            } else {
              dest->o_set(
                StrNR(fieldSpec->name), fieldValue, dest->getClassName());
            }
            if (fieldSpec->isUnion) {
              dest->o_set(s__type, Variant(fieldNum), dest->getClassName());
            }
          }
        }

        if (!readComplete) {
          INC_TPC(thrift_spec_slow);
          skip(fieldType);
        }
        readFieldEnd();
        readFieldBegin(fieldNum, fieldType);
      }
      readStructEnd();
      assertx(dest->assertPropTypeHints());
    }

    Object readStruct(const String& clsName) {
      auto const cls = Class::load(clsName.get());
      if (cls == nullptr) raise_error(Strings::UNKNOWN_CLASS, clsName.data());

      SpecHolder specHolder;
      auto const& spec = specHolder.getSpec(cls);
      Object dest = spec.newObject(cls);
      spec.clearTerseFields(cls, dest);

      readStructBegin();
      int16_t fieldNum;
      TType fieldType;
      readFieldBegin(fieldNum, fieldType);

      auto const& fields = spec.fields;
      const size_t numFields = fields.size();
      if (cls->numDeclProperties() < numFields) {
        readStructSlow(dest, spec, fieldNum, fieldType);
        return dest;
      }
      auto objProp = dest->props();
      auto prop = cls->declProperties().begin();
      int slot = -1;
      while (fieldType != T_STOP) {
        do {
          ++slot;
        } while (slot < numFields && fields[slot].fieldNum != fieldNum);
        if (slot == numFields ||
            prop[slot].name != fields[slot].name ||
            !typesAreCompatible(fieldType, fields[slot].type)) {
          readStructSlow(dest, spec, fieldNum, fieldType);
          return dest;
        }
        if (fields[slot].isUnion) {
          if (s__type.equal(prop[numFields].name)) {
            auto index = cls->propSlotToIndex(numFields);
            tvSetInt(fieldNum, objProp->at(index));
          } else {
            readStructSlow(dest, spec, fieldNum, fieldType);
            return dest;
          }
        }
        auto index = cls->propSlotToIndex(slot);
        bool hasTypeWrapper = false;
        auto value = readField(fields[slot], fieldType, hasTypeWrapper);

        if (hasTypeWrapper) {
          setThriftField(value, dest, StrNR(fields[slot].name));
        } else if (fields[slot].isWrapped) {
          setThriftType(value, dest, StrNR(fields[slot].name));
        } else {
          tvSet(*value.asTypedValue(), objProp->at(index));
        }
        if (!fields[slot].noTypeCheck) {
          dest->verifyPropTypeHint(slot);
          if (fields[slot].isUnion) dest->verifyPropTypeHint(numFields);
        }
        readFieldEnd();
        readFieldBegin(fieldNum, fieldType);
      }
      readStructEnd();
      assertx(dest->assertPropTypeHints());
      return dest;
    }

  private:
    PHPInputTransport transport;
    int options;
    uint8_t version;
    CState state;
    uint16_t lastFieldNum;
    bool boolValue;
    using StructHistoryState = std::pair<CState, uint16_t>;
    std::stack<StructHistoryState, std::vector<StructHistoryState>> structHistory;
    std::stack<CState> containerHistory;

    void readStructBegin(void) {
      structHistory.emplace(state, lastFieldNum);
      state = STATE_FIELD_READ;
      lastFieldNum = 0;
    }

    void readStructEnd(void) {
      std::pair<CState, uint16_t> prev = structHistory.top();
      state = prev.first;
      lastFieldNum = prev.second;
      structHistory.pop();
    }

    void readFieldBegin(int16_t &fieldNum, TType &fieldType) {
      uint8_t fieldTypeAndDelta = readUByte();
      int delta = fieldTypeAndDelta >> 4;
      CType fieldCType = (CType)(fieldTypeAndDelta & 0x0f);
      fieldType = ctype_to_ttype(fieldCType);

      if (fieldCType == C_STOP) {
        fieldNum = 0;
        return;
      }

      if (delta == 0) {
        fieldNum = readI();
      } else {
        fieldNum = lastFieldNum + delta;
      }

      lastFieldNum = fieldNum;

      if (fieldCType == C_TRUE) {
        state = STATE_BOOL_READ;
        boolValue = true;
      } else if (fieldCType == C_FALSE) {
        state = STATE_BOOL_READ;
        boolValue = false;
      } else {
        state = STATE_VALUE_READ;
      }
    }

    void readFieldEnd(void) {
      state = STATE_FIELD_READ;
    }

    Variant readField(const FieldSpec& spec, TType type, bool& hasTypeWrapper) {
      const auto thriftValue = readFieldInternal(spec, type, hasTypeWrapper);
      hasTypeWrapper = hasTypeWrapper || spec.isTypeWrapped;
      if (UNLIKELY(spec.adapter != nullptr)) {
        return transformToHackType(thriftValue, *spec.adapter);
      }
      return thriftValue;
    }

    Variant readFieldInternal(
      const FieldSpec& spec,
      TType type,
      bool& hasTypeWrapper
    ) {
      switch (type) {
        case T_STOP:
        case T_VOID:
          return init_null();

        case T_STRUCT:
          return readStruct(spec.className());

        case T_BOOL:
          if (state == STATE_BOOL_READ) {
            return boolValue;
          } else if (state == STATE_CONTAINER_READ) {
            return (readUByte() == C_TRUE);
          } else {
            thrift_error("Invalid state in compact protocol", ERR_UNKNOWN);
          }

        case T_BYTE:
          return transport.template readBE<int8_t>();

        case T_I16:
        case T_I32:
        case T_I64:
        case T_U64:
          return readI();

        case T_DOUBLE: {
            uint64_t i;
            transport.pull(&i, 8);
            if (version >= VERSION_DOUBLE_BE) {
              i = ntohll(i);
            } else {
              i = letohll(i);
            }
            return folly::bit_cast<double>(i);
          }

        case T_FLOAT: {
            uint32_t i;
            transport.pull(&i, 4);
            i = ntohl(i);
            return folly::bit_cast<float>(i);
          }

        case T_UTF8:
        case T_UTF16:
        case T_STRING:
          return readString();

        case T_MAP:
          return readMap(spec, hasTypeWrapper);

        case T_LIST:
          return readList(spec, hasTypeWrapper);

        case T_SET:
          return readSet(spec, hasTypeWrapper);

        default:
          thrift_error("Unknown Thrift data type",
            ERR_INVALID_DATA);
      }
    }

    void skip(TType type) {
      switch (type) {
        case T_STOP:
        case T_VOID:
          thrift_error("Encountered invalid type for skipping T_STOP/T_VOID",
                       ERR_INVALID_DATA);
          break;

        case T_STRUCT: {
            readStructBegin();

            while (true) {
              int16_t fieldNum;
              TType fieldType;
              readFieldBegin(fieldNum, fieldType);

              if (fieldType == T_STOP) {
                break;
              }

              skip(fieldType);

              readFieldEnd();
            }

            readStructEnd();
          }
          break;

        case T_BOOL:
          if (state == STATE_BOOL_READ) {
            // don't need to do anything
          } else if (state == STATE_CONTAINER_READ) {
            readUByte();
          } else {
            thrift_error("Invalid state in compact protocol", ERR_UNKNOWN);
          }
          break;

        case T_BYTE:
          readUByte();
          break;

        case T_I16:
        case T_I32:
        case T_I64:
        case T_U64:
          readI();
          break;

        case T_DOUBLE:
          transport.skip(8);
          break;

        case T_FLOAT:
          transport.skip(4);
          break;

        case T_UTF8:
        case T_UTF16:
        case T_STRING:
          transport.skip(readVarint());
          break;

        case T_MAP: {
            TType keyType, valueType;
            uint32_t size;
            readMapBegin(keyType, valueType, size);

            for (uint32_t i = 0; i < size; i++) {
              skip(keyType);
              skip(valueType);
            }

            readCollectionEnd();
          }
          break;

        case T_LIST:
        case T_SET: {
            TType valueType;
            uint32_t size;
            readListBegin(valueType, size);

            for (uint32_t i = 0; i < size; i++) {
              skip(valueType);
            }

            readCollectionEnd();
          }
          break;

        default:
          thrift_error("Unknown Thrift data type",
            ERR_INVALID_DATA);
      }
    }

    using IntMapInserter = folly::Function<void(int64_t, int64_t)>;

    void readIntMap(IntMapInserter inserter, uint32_t size) {
      const uint8_t *startPtr = transport.data();
      const uint8_t *ptr = startPtr;
      const uint8_t *endPtr = startPtr + transport.length() - 20;
      while ((ptr <= endPtr) && (size > 0)) {
        // Because of 20B offset from the end of the buffer
        // we have enough data to read 2 Varints fast
        int64_t key = zigzagToI64(readVarintFast(&ptr));
        int64_t value = zigzagToI64(readVarintFast(&ptr));
        inserter(key, value);
        size--;
      }
      intptr_t skipLen = ptr - startPtr;
      if (skipLen > transport.length()) {
        thrift_error("Invalid skip value", ERR_INVALID_DATA);
      }
      transport.skipNoAdvance(skipLen);
      // Finish tail using slow byte-by-byte reading
      while (size > 0) {
        int64_t key = readI();
        int64_t value = readI();
        inserter(key, value);
        size--;
      }
    }

    Variant readMapHArray(
      const FieldSpec& spec,
      const TType keyType,
      const TType valueType,
      const uint32_t size,
      bool& hasTypeWrapper
    ) {
      DictInit arr(size);
      switch (keyType) {
        case TType::T_I08:
        case TType::T_I16:
        case TType::T_I32:
        case TType::T_I64: {
          if (keyType != TType::T_BYTE && typeIs16to64Int(valueType) &&
              spec.key().adapter == nullptr && spec.val().adapter == nullptr) {
            hasTypeWrapper = hasTypeWrapper || spec.key().isTypeWrapped;
            readIntMap([&](int64_t key, int64_t val) { arr.set(key, val); }, size);
          } else {
            for (uint32_t i = 0; i < size; i++) {
              int64_t key = readField(spec.key(), keyType, hasTypeWrapper).toInt64();
              Variant value = readField(spec.val(), valueType, hasTypeWrapper);
              arr.set(key, value);
            }
          }
          break;
        }
        case TType::T_STRING: {
          for (uint32_t i = 0; i < size; i++) {
            String key = readField(spec.key(), keyType, hasTypeWrapper).toString();
            Variant value = readField(spec.val(), valueType, hasTypeWrapper);
            arr.set(key, value);
          }
          break;
        }
        default:
          if (size > 0) {
            thrift_error(
                "Unable to deserialize non int/string array keys",
                ERR_INVALID_DATA);
          }
      }
      readCollectionEnd();
      return arr.toVariant();
    }

    Variant readMapCollection(
      const FieldSpec& spec,
      const TType keyType,
      const TType valueType,
      const uint32_t size,
      bool& hasTypeWrapper
    ) {
      auto map(req::make<c_Map>(size));
      if (spec.key().adapter == nullptr && spec.val().adapter == nullptr &&
          typeIs16to64Int(keyType) && typeIs16to64Int(valueType)) {
        hasTypeWrapper = hasTypeWrapper || spec.key().isTypeWrapped || spec.val().isTypeWrapped;
        readIntMap([&](int64_t key, int64_t val) { map.get()->set(key, val); }, size);
        readCollectionEnd();
        return Variant(std::move(map));
      }
      for (uint32_t i = 0; i < size; i++) {
        Variant key = readField(spec.key(), keyType, hasTypeWrapper);
        Variant value = readField(spec.val(), valueType, hasTypeWrapper);
        BaseMap::OffsetSet(map.get(), key.asTypedValue(), value.asTypedValue());
      }
      readCollectionEnd();
      return Variant(std::move(map));
    }

    Variant readMap(const FieldSpec& spec, bool& hasTypeWrapper) {
      TType keyType, valueType;
      uint32_t size;
      readMapBegin(keyType, valueType, size);
      hasTypeWrapper = hasTypeWrapper || spec.val().isTypeWrapped;
      if (s_harray.equal(spec.format)) {
        return readMapHArray(spec, keyType, valueType, size, hasTypeWrapper);
      } else if (s_collection.equal(spec.format)) {
        return readMapCollection(spec, keyType, valueType, size, hasTypeWrapper);
      } else {
        DictInit arr(size);
        if (options & k_THRIFT_MARK_LEGACY_ARRAYS) {
          arr.setLegacyArray();
        }
        for (uint32_t i = 0; i < size; i++) {
          auto key = readField(spec.key(), keyType,hasTypeWrapper);
          auto value = readField(spec.val(), valueType, hasTypeWrapper);
          set_with_intish_key_cast(arr, key, value);
        }
        readCollectionEnd();
        return arr.toVariant();
      }
    }

    using IntListInserter = folly::Function<void(int64_t)>;

    void readIntList(IntListInserter inserter, size_t size) {
      const uint8_t *startPtr = transport.data();
      const uint8_t *ptr = startPtr;
      const uint8_t *endPtr = startPtr + transport.length() - 10;
      while ((ptr <= endPtr) && (size > 0)) {
        // Because of 10B offset from the end of the buffer
        // we have enough data to read 1 Varint fast
        inserter(zigzagToI64(readVarintFast(&ptr)));
        size--;
      }
      intptr_t skipLen = ptr - startPtr;
      if (skipLen > transport.length()) {
        thrift_error("Invalid skip value", ERR_INVALID_DATA);
      }
      transport.skipNoAdvance(skipLen);
      // Finish tail using slow byte-by-byte reading
      while (size > 0) {
        inserter(readI());
        size--;
      }
    }

    Variant readListHArray(
      const FieldSpec& spec,
      const TType valueType,
      const uint32_t size,
      bool& hasTypeWrapper
    ) {
      VecInit arr(size);
      if (spec.val().adapter == nullptr && valueType == T_BYTE) {
        for (uint32_t i = 0; i < size; i++) {
          arr.append(transport.template readBE<int8_t>());
        }
      } else if (spec.val().adapter == nullptr && typeIs16to64Int(valueType)) {
        readIntList([&](int64_t val) { arr.append(val); }, size);
      } else {
        for (uint32_t i = 0; i < size; i++) {
          arr.append(readField(spec.val(), valueType, hasTypeWrapper));
        }
      }
      readCollectionEnd();
      return arr.toVariant();
    }

    Variant readListCollection(
      const FieldSpec& spec,
      const TType valueType,
      const uint32_t size,
      bool& hasTypeWrapper
    ) {
      if (size == 0) {
        readCollectionEnd();
        return Variant(req::make<c_Vector>());
      }
      auto vec = req::make<c_Vector>(size);
      if (spec.val().adapter == nullptr && typeIs16to64Int(valueType)) {
        readIntList(
          [&, i = 0LL](int64_t val) mutable {
            tvDup(*Variant(val).asTypedValue(), vec->appendForUnserialize(i++));
          },
          size
        );
      } else {
        int64_t i = 0;
        do {
          auto val = readField(spec.val(), valueType, hasTypeWrapper);
          tvDup(*val.asTypedValue(), vec->appendForUnserialize(i));
        } while (++i < size);
      }
      readCollectionEnd();
      return Variant(std::move(vec));
    }

    Variant readList(const FieldSpec& spec, bool& hasTypeWrapper) {
      TType valueType;
      uint32_t size;
      readListBegin(valueType, size);
      hasTypeWrapper = hasTypeWrapper || spec.val().isTypeWrapped;
      if (s_harray.equal(spec.format)) {
        return readListHArray(spec, valueType, size, hasTypeWrapper);
      } else if (s_collection.equal(spec.format)) {
        return readListCollection(spec, valueType, size, hasTypeWrapper);
      } else {
        VecInit vai(size);
        if (options & k_THRIFT_MARK_LEGACY_ARRAYS) {
          vai.setLegacyArray(true);
        }
        for (auto i = uint32_t{0}; i < size; ++i) {
          vai.append(readField(spec.val(), valueType, hasTypeWrapper));
        }
        readCollectionEnd();
        return vai.toVariant();
      }
    }

    Variant readSet(const FieldSpec& spec, bool& hasTypeWrapper) {
      TType valueType;
      uint32_t size;
      readListBegin(valueType, size);

      if (s_harray.equal(spec.format)) {
        KeysetInit arr(size);
        for (uint32_t i = 0; i < size; i++) {
          arr.add(readField(spec.val(), valueType, hasTypeWrapper));
        }
        readCollectionEnd();
        return arr.toVariant();
      } else if (s_collection.equal(spec.format)) {
        auto set_ret = req::make<c_Set>(size);
        for (uint32_t i = 0; i < size; i++) {
          Variant value = readField(spec.val(), valueType, hasTypeWrapper);
          set_ret->add(value);
        }
        readCollectionEnd();
        return Variant(std::move(set_ret));
      } else {
        DictInit ainit(size);
        if (options & k_THRIFT_MARK_LEGACY_ARRAYS) {
          ainit.setLegacyArray();
        }
        for (uint32_t i = 0; i < size; i++) {
          Variant value = readField(spec.val(), valueType, hasTypeWrapper);
          set_with_intish_key_cast(ainit, value, true);
        }
        readCollectionEnd();
        return ainit.toVariant();
      }
    }

    void readMapBegin(TType &keyType, TType &valueType, uint32_t &size) {
      size = readVarint();
      uint8_t types = 0;
      if (size > 0) {
        types = readUByte();
      }

      valueType = ctype_to_ttype((CType)(types & 0x0f));
      keyType = ctype_to_ttype((CType)(types >> 4));

      containerHistory.push(state);
      state = STATE_CONTAINER_READ;
    }

    void readListBegin(TType &elemType, uint32_t &size) {
      uint8_t sizeAndType = readUByte();

      size = sizeAndType >> 4;
      elemType = ctype_to_ttype((CType)(sizeAndType & 0x0f));

      if (size == 15) {
        size = readVarint();
      }

      containerHistory.push(state);
      state = STATE_CONTAINER_READ;
    }

    void readCollectionEnd(void) {
      state = containerHistory.top();
      containerHistory.pop();
    }

    uint8_t readUByte(void) {
      return transport.template readBE<int8_t>();
    }

    int64_t readI(void) {
      return zigzagToI64(readVarint());
    }

    uint64_t readVarint(void) {
      uint8_t byte = readUByte();
      uint64_t result = (uint64_t)(byte & 0x7f);
      if ((byte & 0x80) == 0) goto ret;
      // Byte 2
      byte = readUByte();
      result = result | (uint64_t)(byte & 0x7f) << 7;
      if ((byte & 0x80) == 0) goto ret;
      // Byte 3
      byte = readUByte();
      result = result | (uint64_t)(byte & 0x7f) << 14;
      if ((byte & 0x80) == 0) goto ret;
      // Byte 4
      byte = readUByte();
      result = result | (uint64_t)(byte & 0x7f) << 21;
      if ((byte & 0x80) == 0) goto ret;
      // Byte 5
      byte = readUByte();
      result = result | (uint64_t)(byte & 0x7f) << 28;
      if ((byte & 0x80) == 0) goto ret;
      // Byte 6
      byte = readUByte();
      result = result | (uint64_t)(byte & 0x7f) << 35;
      if ((byte & 0x80) == 0) goto ret;
      // Byte 7
      byte = readUByte();
      result = result | (uint64_t)(byte & 0x7f) << 42;
      if ((byte & 0x80) == 0) goto ret;
      // Byte 8
      byte = readUByte();
      result = result | (uint64_t)(byte & 0x7f) << 49;
      if ((byte & 0x80) == 0) goto ret;
      // Byte 9
      byte = readUByte();
      result = result | (uint64_t)(byte & 0x7f) << 56;
      if ((byte & 0x80) == 0) goto ret;
      // Byte 10
      byte = readUByte();
      result = result | (uint64_t)(byte & 0x7f) << 63;
      if ((byte & 0x80) == 0) goto ret;

      thrift_error("Variable-length int over 10 bytes", ERR_INVALID_DATA);

    ret:
      return result;
    }

    String readString(void) {
      uint32_t size = readVarint();

      if (size && (size + 1)) {
        String s = String(size, ReserveString);
        char* buf = s.mutableData();

        transport.pull(buf, size);
        s.setSize(size);
        return s;
      } else {
        transport.skip(size);
        return empty_string();
      }
    }

    int64_t zigzagToI64(uint64_t n) {
      return (n >> 1) ^ -(n & 1);
    }

    bool typeIsInt(TType t) {
      return (t == T_BYTE) || ((t >= T_I16) && (t <= T_I64));
    }

    bool typeIs16to64Int(TType t) {
      return ((t >= T_I16) && (t <= T_I64));
    }

    bool typesAreCompatible(TType t1, TType t2) {
      return (t1 == t2) || (typeIsInt(t1) && (typeIsInt(t2)));
    }

};

int64_t HHVM_FUNCTION(thrift_protocol_set_compact_version,
                      int64_t version) {
  int result = s_compact_request_data->version;
  s_compact_request_data->version = (uint8_t)version;
  return result;
}

void HHVM_FUNCTION(thrift_protocol_write_compact,
                   const Object& transportobj,
                   const String& method_name,
                   int64_t msgtype,
                   const Object& request_struct,
                   int64_t seqid,
                   bool oneway) {
  CoeffectsAutoGuard _;
  // Suppress class-to-string conversion warnings that occur during
  // serialization and deserialization.
  SuppressClassConversionNotice suppressor;

  PHPOutputTransport transport(transportobj);

  CompactWriter writer(&transport);
  writer.setWriteVersion(s_compact_request_data->version);
  writer.writeHeader(method_name, (uint8_t)msgtype, (uint32_t)seqid);
  writer.write(request_struct);

  if (oneway) {
    transport.onewayFlush();
  } else {
    transport.flush();
  }
}

void HHVM_FUNCTION(thrift_protocol_write_compact2,
                   const Object& transportobj,
                   const String& method_name,
                   int64_t msgtype,
                   const Object& request_struct,
                   int64_t seqid,
                   bool oneway,
                   int64_t version) {
  CoeffectsAutoGuard _;
  // Suppress class-to-string conversion warnings that occur during
  // serialization and deserialization.
  SuppressClassConversionNotice suppressor;

  PHPOutputTransport transport(transportobj);

  CompactWriter writer(&transport);
  writer.setWriteVersion(version);
  writer.writeHeader(method_name, (uint8_t)msgtype, (uint32_t)seqid);
  writer.write(request_struct);

  if (oneway) {
    transport.onewayFlush();
  } else {
    transport.flush();
  }
}

Variant HHVM_FUNCTION(thrift_protocol_read_compact,
                      const Object& transportobj,
                      const String& obj_typename,
                      int64_t options) {
  CoeffectsAutoGuard _;
  // Suppress class-to-string conversion warnings that occur during
  // serialization and deserialization.
  SuppressClassConversionNotice suppressor;

  VMRegAnchor _2;
  CompactReader reader(transportobj, options);
  return reader.read(obj_typename);
}

Object HHVM_FUNCTION(thrift_protocol_read_compact_struct,
                     const Object& transportobj,
                     const String& obj_typename,
                     int64_t options) {
  // Suppress class-to-string conversion warnings that occur during
  // serialization and deserialization.
  SuppressClassConversionNotice suppressor;

  VMRegAnchor _;
  CompactReader reader(transportobj, options);
  return reader.readStruct(obj_typename);
}

}
