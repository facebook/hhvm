/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/ext/thrift/transport.h>
#include <runtime/ext/ext_thrift.h>

#include <stack>
#include <utility>

namespace HPHP {

static const uint8_t VERSION = 2;
static const uint8_t PROTOCOL_ID = 0x82;
static const uint8_t TYPE_SHIFT_AMOUNT = 5;

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
  C_STRUCT = 0x0C
};

enum CListType {
  C_LIST_LIST,
  C_LIST_SET
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
    default:
      throw InvalidArgumentException("unknown TType", x);
  }
}

enum TError {
  ERR_UNKNOWN = 0,
  ERR_INVALID_DATA = 1,
  ERR_BAD_VERSION = 4
};

static void thrift_error(CStrRef what, TError why) {
  throw create_object("TProtocolException", CREATE_VECTOR2(what, why));
}

class CompactWriter {
  public:
    CompactWriter(CObjRef _transportobj) :
      transport(_transportobj),
      state(STATE_CLEAR),
      lastFieldNum(0),
      boolFieldNum(0),
      structHistory(),
      containerHistory() {
    }

    void writeHeader(CStrRef name, uint8_t msgtype, uint32_t seqid) {
      writeUByte(PROTOCOL_ID);
      writeUByte(VERSION | (msgtype << TYPE_SHIFT_AMOUNT));
      writeVarint(seqid);
      writeString(name);

      state = STATE_VALUE_WRITE;
    }

    void write(CObjRef obj) {
      writeStruct(obj);
    }


  private:
    PHPOutputTransport transport;
    CState state;
    uint16_t lastFieldNum;
    uint16_t boolFieldNum;
    std::stack<std::pair<CState, uint16_t> > structHistory;
    std::stack<CState> containerHistory;

    void writeStruct(CObjRef obj) {
      // Save state
      structHistory.push(std::make_pair(state, lastFieldNum));
      state = STATE_FIELD_WRITE;
      lastFieldNum = 0;

      // Get field specification
      CArrRef spec = get_static_property(obj->o_getClassName(), "_TSPEC")
        .toArray();

      // Write each member
      for (ArrayIter specIter = spec.begin(); !specIter.end(); ++specIter) {
        Variant key = specIter.first();
        if (!key.isInteger()) {
          thrift_error("Bad keytype in TSPEC (expected 'long')",
            ERR_INVALID_DATA);
        }

        uint16_t fieldNo = key.toInt16();
        Array fieldSpec = specIter.second().toArray();

        String fieldName = fieldSpec
          .rvalAt(s_var, AccessFlags::Error_Key).toString();
        TType fieldType = (TType)fieldSpec
          .rvalAt(s_type, AccessFlags::Error_Key).toByte();

        Variant fieldVal = obj->o_get(fieldName);

        if (!fieldVal.isNull()) {
          writeFieldBegin(fieldNo, fieldType);
          writeField(fieldVal, fieldSpec, fieldType);
          writeFieldEnd();
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

    void writeField(CVarRef value,
                    CArrRef valueSpec,
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
          writeStruct(value);
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
            uint64_t bits = htonll(u.i);

            transport.write((char*)&bits, 8);
          }
          break;

        case T_UTF8:
        case T_UTF16:
        case T_STRING: {
            String s = value.toString();
            uint32_t len = s.size();
            writeVarint(len);
            transport.write(s, len);
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
      }
    }

    void writeMap(Array arr, CArrRef spec) {
      TType keyType = (TType)spec
        .rvalAt(s_ktype, AccessFlags::Error_Key).toByte();
      TType valueType = (TType)spec
        .rvalAt(s_vtype, AccessFlags::Error_Key).toByte();

      Array keySpec = spec.rvalAt(s_key, AccessFlags::Error_Key).toArray();
      Array valueSpec = spec.rvalAt(s_val, AccessFlags::Error_Key).toArray();

      writeMapBegin(keyType, valueType, arr.size());

      for (ArrayIter arrIter = arr.begin(); !arrIter.end(); ++arrIter) {
        writeField(arrIter.first(), keySpec, keyType);
        writeField(arrIter.second(), valueSpec, valueType);
      }

      writeCollectionEnd();
    }

    void writeList(Array arr, CArrRef spec, CListType listType) {
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
      transport.writeI8(n);
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

      transport.write((char*)buf, wsize);
    }

    void writeString(CStrRef s) {
      uint32_t len = s.size();
      writeVarint(len);
      transport.write(s, len);
    }

    uint64_t i64ToZigzag(int64_t n) {
      return (n << 1) ^ (n >> 63);
    }
};

void f_thrift_protocol_write_compact(CObjRef transportobj,
                                     CStrRef method_name,
                                     int64 msgtype,
                                     CObjRef request_struct,
                                     int seqid) {
  CompactWriter writer(transportobj);
  writer.writeHeader(method_name, (uint8_t)msgtype, (uint32_t)seqid);
  writer.write(request_struct);
}

Variant f_thrift_protocol_read_compact(CObjRef transportobj,
                                       CStrRef obj_typename) {
  throw NotImplementedException(__func__);
}

}
