/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/ext/thrift/transport.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/ext/reflection/ext_reflection.h"
#include "hphp/runtime/ext/thrift/ext_thrift.h"
#include "hphp/runtime/ext/thrift/spec-holder.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/util/fixed-vector.h"

#include <folly/AtomicHashMap.h>
#include <folly/Format.h>

#include <stack>
#include <utility>

namespace HPHP { namespace thrift {
/////////////////////////////////////////////////////////////////////////////

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

  void vscan(IMarker&) const override {}

  uint8_t version;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(CompactRequestData, s_compact_request_data);

class CompactWriter {
  public:
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
    std::stack<std::pair<CState, uint16_t> > structHistory;
    std::stack<CState> containerHistory;

    void writeSlow(const FieldSpec& field, const Object& obj) {
      INC_TPC(thrift_write_slow);
      StrNR fieldName(field.name);
      Variant fieldVal = obj->o_get(fieldName, true, obj->getClassName());
      if (!fieldVal.isNull()) {
        TType fieldType = field.type;
        writeFieldBegin(field.fieldNum, fieldType);
        ArrNR fieldSpec(field.spec);
        writeField(fieldVal, fieldSpec.asArray(), fieldType);
        writeFieldEnd();
      }
    }

    void writeStruct(const Object& obj) {
      // Save state
      structHistory.push(std::make_pair(state, lastFieldNum));
      state = STATE_FIELD_WRITE;
      lastFieldNum = 0;

      // Get field specification
      Class* cls = obj->getVMClass();
      Array spec(get_tspec(cls));
      SpecHolder specHolder;
      const auto& fields = specHolder.getSpec(spec);
      auto prop = cls->declProperties().begin();
      auto objProp = obj->propVec();
      const size_t numProps = cls->numDeclProperties();
      const size_t numFields = fields.size();
      // Write each member
      for (int i = 0; i < numFields; ++i) {
        if (i <= numProps && fields[i].name == prop[i].name) {
          Variant fieldVal = tvAsVariant(&objProp[i]);
          if (!fieldVal.isNull()) {
            TType fieldType = fields[i].type;
            ArrNR fieldSpec(fields[i].spec);
            writeFieldBegin(fields[i].fieldNum, fieldType);
            writeField(fieldVal, fieldSpec.asArray(), fieldType);
            writeFieldEnd();
          }
        } else {
          writeSlow(fields[i], obj);
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
                    const Array& valueSpec,
                    TType type) {
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
          writeUByte(value.toByte());
          break;

        case T_I16:
        case T_I32:
        case T_I64:
        case T_U64:
          writeI(value.toInt64());
          break;

        case T_DOUBLE: {
            union {
              uint64_t i;
              double d;
            } u;

            u.d = value.toDouble();
            uint64_t bits;
            if (version >= VERSION_DOUBLE_BE) {
              bits = htonll(u.i);
            } else {
              bits = htolell(u.i);
            }

            transport->write((char*)&bits, 8);
          }
          break;

        case T_FLOAT: {
          union {
            uint32_t i;
            float d;
          } u;

          u.d = (float)value.toDouble();
          uint32_t bits = htonl(u.i);
          transport->write((char*)&bits, 4);
          }
          break;

        case T_UTF8:
        case T_UTF16:
        case T_STRING: {
            auto s = value.toString();
            auto slice = s.slice();
            writeVarint(slice.size());
            transport->write(slice.data(), slice.size());
            break;
          }

        case T_MAP:
          writeMap(value.toArray(), valueSpec);
          break;

        case T_LIST:
          writeList(value.toArray(), valueSpec, C_LIST_LIST);
          break;

        case T_SET:
          writeList(value.toArray(), valueSpec, C_LIST_SET);
          break;

        default:
          thrift_error("Unknown Thrift data type",
            ERR_INVALID_DATA);
      }
    }

    void writeMap(Array arr, const Array& spec) {
      TType keyType = (TType)spec
        .rvalAt(s_ktype, AccessFlags::Error_Key).toByte();
      TType valueType = (TType)spec
        .rvalAt(s_vtype, AccessFlags::Error_Key).toByte();

      Array keySpec = spec.rvalAt(s_key, AccessFlags::Error_Key)
        .toArray();
      Array valueSpec = spec.rvalAt(s_val, AccessFlags::Error_Key)
        .toArray();

      writeMapBegin(keyType, valueType, arr.size());

      for (ArrayIter arrIter = arr.begin(); !arrIter.end(); ++arrIter) {
        writeField(arrIter.first(), keySpec, keyType);
        writeField(arrIter.second(), valueSpec, valueType);
      }

      writeCollectionEnd();
    }

    void writeList(Array arr, const Array& spec, CListType listType) {
      TType valueType = (TType)spec
        .rvalAt(s_etype, AccessFlags::Error_Key).toByte();
      Array valueSpec = spec
        .rvalAt(s_elem, AccessFlags::Error_Key).toArray();

      writeListBegin(valueType, arr.size());

      for (ArrayIter arrIter = arr.begin(); !arrIter.end(); ++arrIter) {
        Variant x;
        if (listType == C_LIST_LIST) {
          x = arrIter.second();
        } else if (listType == C_LIST_SET) {
          x = arrIter.first();
        }

        writeField(x, valueSpec, valueType);
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
      return (n << 1) ^ (n >> 63);
    }
};

class CompactReader {
  public:
    explicit CompactReader(const Object& _transportobj) :
      transport(_transportobj),
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
      /*uint32_t seqid =*/ readVarint(); // unused
      /*Variant name =*/ readString(); // unused

      if (type == T_REPLY) {
        Object ret = create_object(resultClassName, Array());
        Array spec(get_tspec(ret->getVMClass()));
        readStruct(ret, spec);
        return ret;
      } else if (type == T_EXCEPTION) {
        Object exn = create_object(s_TApplicationException, Array());
        Array spec(get_tspec(exn->getVMClass()));
        readStruct(exn, spec);
        throw_object(exn);
      } else {
        thrift_error("Invalid response type", ERR_INVALID_DATA);
      }
    }

    NEVER_INLINE
    void readStructSlow(const Object& dest, const Array& spec,
                        int16_t fieldNum, TType fieldType) {
      INC_TPC(thrift_read_slow);
      while (fieldType != T_STOP) {
        bool readComplete = false;

        Variant fieldSpecVariant = spec.rvalAt(fieldNum);
        if (!fieldSpecVariant.isNull()) {
          Array fieldSpec = fieldSpecVariant.toArray();

          String fieldName = fieldSpec.rvalAt(s_var).toString();
          auto expectedType = (TType)fieldSpec.rvalAt(s_type)
            .toInt64();

          if (typesAreCompatible(fieldType, expectedType)) {
            readComplete = true;
            Variant fieldValue = readField(fieldSpec, fieldType);
            dest->o_set(fieldName, fieldValue, dest->getClassName());
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
    }

    void readStruct(const Object& dest, const Array& spec) {
      readStructBegin();
      int16_t fieldNum;
      TType fieldType;
      readFieldBegin(fieldNum, fieldType);
      SpecHolder specHolder;
      const auto& fields = specHolder.getSpec(spec);
      const size_t numFields = fields.size();
      Class* cls = dest->getVMClass();
      if (cls->numDeclProperties() < numFields) {
        return readStructSlow(dest, spec, fieldNum, fieldType);
      }
      auto objProp = dest->propVec();
      auto prop = cls->declProperties().begin();
      int i = -1;
      while (fieldType != T_STOP) {
        do {
          ++i;
        } while (i < numFields && fields[i].fieldNum != fieldNum);
        if (i == numFields ||
            prop[i].name != fields[i].name ||
            !typesAreCompatible(fieldType, fields[i].type)) {
          return readStructSlow(dest, spec, fieldNum, fieldType);
        }
        ArrNR fieldSpec(fields[i].spec);
        tvAsVariant(&objProp[i]) = readField(fieldSpec.asArray(), fieldType);
        readFieldEnd();
        readFieldBegin(fieldNum, fieldType);
      }
      readStructEnd();
    }

  private:
    PHPInputTransport transport;
    uint8_t version;
    CState state;
    uint16_t lastFieldNum;
    bool boolValue;
    std::stack<std::pair<CState, uint16_t> > structHistory;
    std::stack<CState> containerHistory;

    void readStructBegin(void) {
      structHistory.push(std::make_pair(state, lastFieldNum));
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

    Variant readField(const Array& spec, TType type) {
      switch (type) {
        case T_STOP:
        case T_VOID:
          return init_null();

        case T_STRUCT: {
            Variant className = spec.rvalAt(s_class);
            if (className.isNull()) {
              thrift_error("no class type in spec", ERR_INVALID_DATA);
            }

            String classNameString = className.toString();
            Variant newStruct = create_object(classNameString, Array());
            if (newStruct.isNull()) {
              thrift_error("invalid class type in spec", ERR_INVALID_DATA);
            }

            Object obj = newStruct.toObject();
            Array spec(get_tspec(obj->getVMClass()));
            readStruct(obj, spec);
            return newStruct;
          }

        case T_BOOL:
          if (state == STATE_BOOL_READ) {
            return boolValue;
          } else if (state == STATE_CONTAINER_READ) {
            return (readUByte() == C_TRUE);
          } else {
            thrift_error("Invalid state in compact protocol", ERR_UNKNOWN);
          }

        case T_BYTE:
          return transport.readI8();

        case T_I16:
        case T_I32:
        case T_I64:
        case T_U64:
          return readI();

        case T_DOUBLE: {
            union {
              uint64_t i;
              double d;
            } u;

            transport.readBytes(&(u.i), 8);
            if (version >= VERSION_DOUBLE_BE) {
              u.i = ntohll(u.i);
            } else {
              u.i = letohll(u.i);
            }
            return u.d;
          }

        case T_FLOAT: {
             union {
              uint32_t i;
              float d;
            } u;

            transport.readBytes(&(u.i), 4);
            u.i = ntohl(u.i);
            return u.d;
          }

        case T_UTF8:
        case T_UTF16:
        case T_STRING:
          return readString();

        case T_MAP:
          return readMap(spec);

        case T_LIST:
          return readList(spec);

        case T_SET:
          return readSet(spec);

        default:
          thrift_error("Unknown Thrift data type",
            ERR_INVALID_DATA);
      }
    }

    void skip(TType type) {
      switch (type) {
        case T_STOP:
        case T_VOID:
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

    Variant readMap(const Array& spec) {
      TType keyType, valueType;
      uint32_t size;
      readMapBegin(keyType, valueType, size);

      Array keySpec = spec.rvalAt(s_key,
        AccessFlags::Error).toArray();
      Array valueSpec = spec.rvalAt(s_val,
        AccessFlags::Error).toArray();
      String format = spec.rvalAt(s_format,
        AccessFlags::None).toString();
      if (format.equal(s_collection)) {
        auto ret(req::make<c_Map>(size));
        for (uint32_t i = 0; i < size; i++) {
          Variant key = readField(keySpec, keyType);
          Variant value = readField(valueSpec, valueType);
          BaseMap::OffsetSet(ret.get(), key.asCell(), value.asCell());
        }
        readCollectionEnd();
        return Variant(std::move(ret));
      } else {
        ArrayInit ainit(size, ArrayInit::Mixed{});
        for (uint32_t i = 0; i < size; i++) {
          Variant key = readField(keySpec, keyType);
          Variant value = readField(valueSpec, valueType);
          ainit.setUnknownKey(key, value);
        }
        readCollectionEnd();
        return ainit.toVariant();
      }
    }

    Variant readList(const Array& spec) {
      TType valueType;
      uint32_t size;
      readListBegin(valueType, size);

      Array valueSpec = spec.rvalAt(s_elem,
                                    AccessFlags::Error_Key).toArray();
      String format = spec.rvalAt(s_format,
        AccessFlags::None).toString();
      if (format.equal(s_collection)) {
        auto const pvec(req::make<c_Vector>(size));
        for (uint32_t i = 0; i < size; i++) {
          pvec->t_add(readField(valueSpec, valueType));
        }
        readCollectionEnd();
        return Variant(std::move(pvec));
      } else {
        PackedArrayInit pai(size);
        for (auto i = uint32_t{0}; i < size; ++i) {
          pai.append(readField(valueSpec, valueType));
        }
        readCollectionEnd();
        return pai.toVariant();
      }
    }

    Variant readSet(const Array& spec) {
      TType valueType;
      uint32_t size;
      readListBegin(valueType, size);

      Array valueSpec = spec.rvalAt(s_elem,
                                    AccessFlags::Error_Key).toArray();
      String format = spec.rvalAt(s_format,
        AccessFlags::None).toString();
      if (format.equal(s_collection)) {
        auto set_ret = req::make<c_Set>(size);
        for (uint32_t i = 0; i < size; i++) {
          Variant value = readField(valueSpec, valueType);
          set_ret->t_add(value);
        }
        readCollectionEnd();
        return Variant(std::move(set_ret));
      } else {
        // Note: the Mixed{} is just out of uncertainty right now.
        // These probably are generally string keys and this should
        // probably be ArrayInit::Map.
        ArrayInit ainit(size, ArrayInit::Mixed{});
        for (uint32_t i = 0; i < size; i++) {
          Variant value = readField(valueSpec, valueType);
          ainit.setUnknownKey(value, true);
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
      return transport.readI8();
    }

    int64_t readI(void) {
      return zigzagToI64(readVarint());
    }

    uint64_t readVarint(void) {
      uint64_t result = 0;
      uint8_t shift = 0;

      while (true) {
        uint8_t byte = readUByte();
        result |= (uint64_t)(byte & 0x7f) << shift;
        shift += 7;

        if (!(byte & 0x80)) {
          return result;
        }

        // Should never read more than 10 bytes, which is the max for a 64-bit
        // int
        if (shift >= 10 * 7) {
          thrift_error("Variable-length int over 10 bytes", ERR_INVALID_DATA);
        }
      }
    }

    String readString(void) {
      uint32_t size = readVarint();

      if (size && (size + 1)) {
        String s = String(size, ReserveString);
        char* buf = s.mutableData();

        transport.readBytes(buf, size);
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

    bool typesAreCompatible(TType t1, TType t2) {
      return (t1 == t2) || (typeIsInt(t1) && (typeIsInt(t2)));
    }

};

int64_t HHVM_FUNCTION(thrift_protocol_set_compact_version,
                      int version) {
  int result = s_compact_request_data->version;
  s_compact_request_data->version = (uint8_t)version;
  return result;
}

void HHVM_FUNCTION(thrift_protocol_write_compact,
                   const Object& transportobj,
                   const String& method_name,
                   int64_t msgtype,
                   const Object& request_struct,
                   int seqid,
                   bool oneway) {
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

Variant HHVM_FUNCTION(thrift_protocol_read_compact,
                      const Object& transportobj,
                      const String& obj_typename) {
  EagerVMRegAnchor _;
  CompactReader reader(transportobj);
  return reader.read(obj_typename);
}

Object HHVM_FUNCTION(thrift_protocol_read_compact_struct,
                     const Object& transportobj,
                     const String& obj_typename) {
  EagerVMRegAnchor _;
  CompactReader reader(transportobj);
  Object ret = create_object(obj_typename, Array());
  Array spec(get_tspec(ret->getVMClass()));
  reader.readStruct(ret, spec);
  return ret;
}

}}
