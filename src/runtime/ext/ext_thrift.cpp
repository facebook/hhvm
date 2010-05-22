/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/ext/ext_thrift.h>
#include <runtime/ext/ext_class.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <endian.h>
#include <byteswap.h>
#include <stdexcept>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define htonll(x) bswap_64(x)
#define ntohll(x) bswap_64(x)
#else
#define htonll(x) x
#define ntohll(x) x
#endif

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(thrift_protocol);
///////////////////////////////////////////////////////////////////////////////

enum TType {
  T_STOP       = 0,
  T_VOID       = 1,
  T_BOOL       = 2,
  T_BYTE       = 3,
  T_I08        = 3,
  T_I16        = 6,
  T_I32        = 8,
  T_U64        = 9,
  T_I64        = 10,
  T_DOUBLE     = 4,
  T_STRING     = 11,
  T_UTF7       = 11,
  T_STRUCT     = 12,
  T_MAP        = 13,
  T_SET        = 14,
  T_LIST       = 15,
  T_UTF8       = 16,
  T_UTF16      = 17
};

const int32_t VERSION_MASK = 0xffff0000;
const int32_t VERSION_1 = 0x80010000;
const int8_t T_CALL = 1;
const int8_t T_REPLY = 2;
const int8_t T_EXCEPTION = 3;
// tprotocolexception
const int INVALID_DATA = 1;
const int BAD_VERSION = 4;


class PHPTransport {
public:
  Object protocol() { return p; }
  Object transport() { return t; }
protected:
  PHPTransport() {}

  void construct_with_zval(CObjRef _p, size_t _buffer_size) {
    buffer = reinterpret_cast<char*>(malloc(_buffer_size));
    buffer_ptr = buffer;
    buffer_used = 0;
    buffer_size = _buffer_size;
    p = _p;
    t = p->o_invoke("getTransport", Array(), -1);
  }
  ~PHPTransport() {
    free(buffer);
  }

  char* buffer;
  char* buffer_ptr;
  size_t buffer_used;
  size_t buffer_size;

  Object p;
  Object t;
};


class PHPOutputTransport : public PHPTransport {
public:
  PHPOutputTransport(CObjRef _p, size_t _buffer_size = 8192) {
    construct_with_zval(_p, _buffer_size);
  }

  ~PHPOutputTransport() {
    flush();
    directFlush();
  }

  void write(const char* data, size_t len) {
    if ((len + buffer_used) > buffer_size) {
      flush();
    }
    if (len > buffer_size) {
      directWrite(data, len);
    } else {
      memcpy(buffer_ptr, data, len);
      buffer_used += len;
      buffer_ptr += len;
    }
  }

  void writeI64(int64_t i) {
    i = htonll(i);
    write((const char*)&i, 8);
  }

  void writeU32(uint32_t i) {
    i = htonl(i);
    write((const char*)&i, 4);
  }

  void writeI32(int32_t i) {
    i = htonl(i);
    write((const char*)&i, 4);
  }

  void writeI16(int16_t i) {
    i = htons(i);
    write((const char*)&i, 2);
  }

  void writeI8(int8_t i) {
    write((const char*)&i, 1);
  }

  void writeString(const char* str, size_t len) {
    writeU32(len);
    write(str, len);
  }

  void flush() {
    if (buffer_used) {
      directWrite(buffer, buffer_used);
      buffer_ptr = buffer;
      buffer_used = 0;
    }
  }

protected:
  void directFlush() {
    t->o_invoke("flush", Array(), -1);
  }
  void directWrite(const char* data, size_t len) {
    Array args = CREATE_VECTOR1(String(buffer, buffer_used, CopyString));
    t->o_invoke("write", args, -1);
  }
};

class PHPInputTransport : public PHPTransport {
public:
  PHPInputTransport(Object _p, size_t _buffer_size = 8192) {
    construct_with_zval(_p, _buffer_size);
  }

  ~PHPInputTransport() {
    put_back();
  }

  void put_back() {
    if (buffer_used) {
      t->o_invoke("putBack",
                  CREATE_VECTOR1(String(buffer_ptr, buffer_used, CopyString)),
                  -1);
    }
    buffer_used = 0;
    buffer_ptr = buffer;
  }

  void skip(size_t len) {
    while (len) {
      size_t chunk_size = len < buffer_used ? len : buffer_used;
      if (chunk_size) {
        buffer_ptr = reinterpret_cast<char*>(buffer_ptr) + chunk_size;
        buffer_used -= chunk_size;
        len -= chunk_size;
      }
      if (! len) break;
      refill();
    }
  }

  void readBytes(void* buf, size_t len) {
    while (len) {
      size_t chunk_size = len < buffer_used ? len : buffer_used;
      if (chunk_size) {
        memcpy(buf, buffer_ptr, chunk_size);
        buffer_ptr = reinterpret_cast<char*>(buffer_ptr) + chunk_size;
        buffer_used -= chunk_size;
        buf = reinterpret_cast<char*>(buf) + chunk_size;
        len -= chunk_size;
      }
      if (! len) break;
      refill();
    }
  }

  int8_t readI8() {
    int8_t c;
    readBytes(&c, 1);
    return c;
  }

  int16_t readI16() {
    int16_t c;
    readBytes(&c, 2);
    return (int16_t)ntohs(c);
  }

  uint32_t readU32() {
    uint32_t c;
    readBytes(&c, 4);
    return (uint32_t)ntohl(c);
  }

  int32_t readI32() {
    int32_t c;
    readBytes(&c, 4);
    return (int32_t)ntohl(c);
  }

protected:
  void refill() {
    ASSERT(buffer_used == 0);
    String ret = t->o_invoke("read",
                             CREATE_VECTOR1((int64)buffer_size), -1).toString();
    buffer_used = ret.size();
    memcpy(buffer, ret.data(), buffer_used);
    buffer_ptr = buffer;
  }

};

void binary_deserialize_spec(CObjRef zthis, PHPInputTransport& transport, CArrRef spec);
void binary_serialize_spec(CObjRef zthis, PHPOutputTransport& transport, CArrRef spec);
void binary_serialize(int8_t thrift_typeID, PHPOutputTransport& transport, CVarRef value, CArrRef fieldspec);
void skip_element(long thrift_typeID, PHPInputTransport& transport);

// Create a PHP object given a typename and call the ctor, optionally passing up to 2 arguments
Object createObject(CStrRef obj_typename, int nargs = 0,
                    CVarRef arg1 = null_variant, CVarRef arg2 = null_variant) {
  if (!f_class_exists(obj_typename)) {
    raise_warning("runtime/ext_thrift: Class %s does not exist",
                  obj_typename.data());
    return Object();
  }
  Array args;
  if (nargs == 1) {
    args = CREATE_VECTOR1(arg1);
  } else if (nargs == 2 ) {
    args = CREATE_VECTOR2(arg1, arg2);
  }
  return create_object(obj_typename.data(), args);
}

void throw_tprotocolexception(CStrRef what, long errorcode) {
  Object ex = createObject("TProtocolException", 2, what, errorcode);
  throw ex;
}

Variant binary_deserialize(int8_t thrift_typeID, PHPInputTransport& transport,
                           CArrRef fieldspec) {
  Variant ret;
  switch (thrift_typeID) {
    case T_STOP:
    case T_VOID:
      return null;
    case T_STRUCT: {
      Variant val;
      if ((val = fieldspec.rvalAt("class")).isNull()) {
        throw_tprotocolexception("no class type in spec", INVALID_DATA);
        skip_element(T_STRUCT, transport);
        return null;
      }
      String structType = val.toString();
      ret = createObject(structType);
      if (ret.isNull()) {
        // unable to create class entry
        skip_element(T_STRUCT, transport);
        return null;
      }
      Variant spec = get_static_property(structType, "_TSPEC");
      if (!spec.is(KindOfArray)) {
        char errbuf[128];
        snprintf(errbuf, 128, "spec for %s is wrong type: %d\n",
                 structType.data(), ret.getType());
        throw_tprotocolexception(String(errbuf, CopyString), INVALID_DATA);
        return null;
      }
      binary_deserialize_spec(ret, transport, spec.toArray());
      return ret;
    } break;
    case T_BOOL: {
      uint8_t c;
      transport.readBytes(&c, 1);
      return c != 0;
    }
  //case T_I08: // same numeric value as T_BYTE
    case T_BYTE: {
      uint8_t c;
      transport.readBytes(&c, 1);
      return c;
    }
    case T_I16: {
      uint16_t c;
      transport.readBytes(&c, 2);
      return ntohs(c);
    }
    case T_I32: {
      uint32_t c;
      transport.readBytes(&c, 4);
      return Variant((int)ntohl(c));
    }
    case T_U64:
    case T_I64: {
      uint64_t c;
      transport.readBytes(&c, 8);
      return Variant((int64)ntohll(c));
    }
    case T_DOUBLE: {
      union {
        uint64_t c;
        double d;
      } a;
      transport.readBytes(&(a.c), 8);
      a.c = ntohll(a.c);
      return a.d;
    }
    //case T_UTF7: // aliases T_STRING
    case T_UTF8:
    case T_UTF16:
    case T_STRING: {
      uint32_t size = transport.readU32();
      if (size && (size + 1)) {
        char* strbuf = (char*) malloc(size + 1);
        transport.readBytes(strbuf, size);
        strbuf[size] = '\0';
        return String(strbuf, size, AttachString);
      } else {
        return "";
      }
    }
    case T_MAP: { // array of key -> value
      uint8_t types[2];
      transport.readBytes(types, 2);
      uint32_t size = transport.readU32();

      Array keyspec = fieldspec.rvalAt("key", -1).toArray();
      Array valspec = fieldspec.rvalAt("val", -1).toArray();
      ret = Array::Create();

      for (uint32_t s = 0; s < size; ++s) {
        Variant key = binary_deserialize(types[0], transport, keyspec);
        Variant value = binary_deserialize(types[1], transport, valspec);
        ret.set(key, value);
      }
      return ret; // return_value already populated
    }
    case T_LIST: { // array with autogenerated numeric keys
      int8_t type = transport.readI8();
      uint32_t size = transport.readU32();
      Variant elemvar = fieldspec.rvalAt("elem", -1);
      Array elemspec = elemvar.toArray();
      ret = Array::Create();

      for (uint32_t s = 0; s < size; ++s) {
        Variant value = binary_deserialize(type, transport, elemspec);
        ret.append(value);
      }
      return ret;
    }
    case T_SET: { // array of key -> TRUE
      uint8_t type;
      uint32_t size;
      transport.readBytes(&type, 1);
      transport.readBytes(&size, 4);
      size = ntohl(size);
      Variant elemvar = fieldspec.rvalAt("elem", -1);
      Array elemspec = elemvar.toArray();
      ret = Array::Create();

      for (uint32_t s = 0; s < size; ++s) {
        Variant key = binary_deserialize(type, transport, elemspec);

        if (key.isInteger()) {
          ret.set(key, true);
        } else {
          ret.set(key.toString(), true);
        }
      }
      return ret;
    }
  };

  char errbuf[128];
  sprintf(errbuf, "Unknown thrift typeID %d", thrift_typeID);
  throw_tprotocolexception(String(errbuf, CopyString), INVALID_DATA);
  return null;
}

void skip_element(long thrift_typeID, PHPInputTransport& transport) {
  switch (thrift_typeID) {
    case T_STOP:
    case T_VOID:
      return;
    case T_STRUCT:
      while (true) {
        int8_t ttype = transport.readI8(); // get field type
        if (ttype == T_STOP) break;
        transport.skip(2); // skip field number, I16
        skip_element(ttype, transport); // skip field payload
      }
      return;
    case T_BOOL:
    case T_BYTE:
      transport.skip(1);
      return;
    case T_I16:
      transport.skip(2);
      return;
    case T_I32:
      transport.skip(4);
      return;
    case T_U64:
    case T_I64:
    case T_DOUBLE:
      transport.skip(8);
      return;
    //case T_UTF7: // aliases T_STRING
    case T_UTF8:
    case T_UTF16:
    case T_STRING: {
      uint32_t len = transport.readU32();
      transport.skip(len);
      } return;
    case T_MAP: {
      int8_t keytype = transport.readI8();
      int8_t valtype = transport.readI8();
      uint32_t size = transport.readU32();
      for (uint32_t i = 0; i < size; ++i) {
        skip_element(keytype, transport);
        skip_element(valtype, transport);
      }
    } return;
    case T_LIST:
    case T_SET: {
      int8_t valtype = transport.readI8();
      uint32_t size = transport.readU32();
      for (uint32_t i = 0; i < size; ++i) {
        skip_element(valtype, transport);
      }
    } return;
  };

  char errbuf[128];
  sprintf(errbuf, "Unknown thrift typeID %ld", thrift_typeID);
  throw_tprotocolexception(String(errbuf, CopyString), INVALID_DATA);
}

void binary_serialize_hashtable_key(int8_t keytype, PHPOutputTransport& transport,
                                    Variant key) {
  bool keytype_is_numeric = (!((keytype == T_STRING) || (keytype == T_UTF8) ||
                               (keytype == T_UTF16)));

  if (keytype_is_numeric) {
    key = key.toInt64();
  } else {
    key = key.toString();
  }
  binary_serialize(keytype, transport, key, Array());
}

inline bool ttype_is_int(int8_t t) {
  return ((t == T_BYTE) || ((t >= T_I16)  && (t <= T_I64)));
}

inline bool ttypes_are_compatible(int8_t t1, int8_t t2) {
  // Integer types of different widths are considered compatible;
  // otherwise the typeID must match.
  return ((t1 == t2) || (ttype_is_int(t1) && ttype_is_int(t2)));
}

void binary_deserialize_spec(CObjRef zthis, PHPInputTransport& transport,
                             CArrRef spec) {
  // SET and LIST have 'elem' => array('type', [optional] 'class')
  // MAP has 'val' => array('type', [optiona] 'class')
  while (true) {
    Variant val;

    int8_t ttype = transport.readI8();
    if (ttype == T_STOP) return;
    int16_t fieldno = transport.readI16();
    if (!(val = spec.rvalAt(fieldno)).isNull()) {
      Array fieldspec = val.toArray();
      // pull the field name
      // zend hash tables use the null at the end in the length... so strlen(hash key) + 1.
      String varname = fieldspec.rvalAt("var").toString();

      // and the type
      int8_t expected_ttype = fieldspec.rvalAt("type").toInt64();

      if (ttypes_are_compatible(ttype, expected_ttype)) {
        Variant rv = binary_deserialize(ttype, transport, fieldspec);
        zthis->set(varname, rv);
      } else {
        skip_element(ttype, transport);
      }
    } else {
      skip_element(ttype, transport);
    }
  }
}

void binary_serialize(int8_t thrift_typeID, PHPOutputTransport& transport,
                      CVarRef value, CArrRef fieldspec) {
  // At this point the typeID (and field num, if applicable) should've already
  // been written to the output so all we need to do is write the payload.
  switch (thrift_typeID) {
    case T_STOP:
    case T_VOID:
      return;
    case T_STRUCT: {
      if (!value.is(KindOfObject)) {
        throw_tprotocolexception("Attempt to send non-object "
                                 "type as a T_STRUCT", INVALID_DATA);
      }
      binary_serialize_spec(value, transport,
                            get_static_property(toObject(value)->
                                                o_getClassName(),
                                                "_TSPEC").toArray());
    } return;
    case T_BOOL:
      transport.writeI8(value.toBoolean() ? 1 : 0);
      return;
    case T_BYTE:
      transport.writeI8(value.toByte());
      return;
    case T_I16:
      transport.writeI16(value.toInt16());
      return;
    case T_I32:
      transport.writeI32(value.toInt32());
      return;
    case T_I64:
    case T_U64:
      transport.writeI64(value.toInt64());
      return;
    case T_DOUBLE: {
      union {
        int64_t c;
        double d;
      } a;
      a.d = value.toDouble();
      transport.writeI64(a.c);
    } return;
    //case T_UTF7:
    case T_UTF8:
    case T_UTF16:
    case T_STRING: {
        String sv = value.toString();
        transport.writeString(sv, sv.size());
    } return;
    case T_MAP: {
      Array ht = value.toArray();
      uint8_t keytype = fieldspec.rvalAt("ktype", -1).toByte();
      transport.writeI8(keytype);
      uint8_t valtype = fieldspec.rvalAt("vtype", -1).toByte();
      transport.writeI8(valtype);

      Array valspec = fieldspec.rvalAt("val", -1).toArray();

      transport.writeI32(ht.size());
      for (ArrayIter key_ptr = ht.begin(); !key_ptr.end(); ++key_ptr) {
        binary_serialize_hashtable_key(keytype, transport, key_ptr.first());
        binary_serialize(valtype, transport, key_ptr.second(), valspec);
      }
    } return;
    case T_LIST: {
      Array ht = value.toArray();
      Variant val;

      uint8_t valtype = fieldspec.rvalAt("etype", -1).toInt64();
      transport.writeI8(valtype);
      Array valspec = fieldspec.rvalAt("elem", -1).toArray();
      transport.writeI32(ht.size());
      for (ArrayIter key_ptr = ht.begin(); !key_ptr.end(); ++key_ptr) {
        binary_serialize(valtype, transport, key_ptr.second(), valspec);
      }
    } return;
    case T_SET: {
      Array ht = value.toArray();

      uint8_t keytype = fieldspec.rvalAt("etype", -1).toByte();
      transport.writeI8(keytype);

      transport.writeI32(ht.size());
      for (ArrayIter key_ptr = ht.begin(); !key_ptr.end(); ++key_ptr) {
        binary_serialize_hashtable_key(keytype, transport, key_ptr.first());
      }
    } return;
  };
  char errbuf[128];
  sprintf(errbuf, "Unknown thrift typeID %d", thrift_typeID);
  throw_tprotocolexception(String(errbuf, CopyString), INVALID_DATA);
}


void binary_serialize_spec(CObjRef zthis, PHPOutputTransport& transport,
                           CArrRef spec) {
  for (ArrayIter key_ptr = spec.begin(); !key_ptr.end(); ++key_ptr) {
    Variant key = key_ptr.first();
    if (!key.isInteger()) {
      throw_tprotocolexception("Bad keytype in TSPEC (expected 'long')", INVALID_DATA);
      return;
    }
    ulong fieldno = key.toInt64();
    Array fieldspec = key_ptr.second().toArray();

    // field name
    String varname = fieldspec.rvalAt("var", -1).toString();

    // thrift type
    int8_t ttype = fieldspec.rvalAt("type", -1).toByte();

    Variant prop = zthis->o_get(varname, -1);
    if (!prop.isNull()) {
      transport.writeI8(ttype);
      transport.writeI16(fieldno);
      binary_serialize(ttype, transport, prop, fieldspec);
    }
  }
  transport.writeI8(T_STOP); // struct end
}

void f_thrift_protocol_write_binary(CObjRef transportobj, CStrRef method_name,
                                    int64 msgtype, CObjRef request_struct,
                                    int seqid, bool strict_write) {

  PHPOutputTransport transport(transportobj);

  if (strict_write) {
    int32_t version = VERSION_1 | msgtype;
    transport.writeI32(version);
    transport.writeString(method_name, method_name.size());
    transport.writeI32(seqid);
  } else {
    transport.writeString(method_name, method_name.size());
    transport.writeI8(msgtype);
    transport.writeI32(seqid);
  }

  Variant spec = get_static_property(request_struct->o_getClassName(),
                                     "_TSPEC");
  binary_serialize_spec(request_struct, transport, spec.toArray());
}

Variant f_thrift_protocol_read_binary(CObjRef transportobj,
                                      CStrRef obj_typename,
                                      bool strict_read) {
  PHPInputTransport transport(transportobj);
  int8_t messageType = 0;
  int32_t sz = transport.readI32();

  if (sz < 0) {
    // Check for correct version number
    int32_t version = sz & VERSION_MASK;
    if (version != VERSION_1) {
      throw_tprotocolexception("Bad version identifier", BAD_VERSION);
    }
    messageType = (sz & 0x000000ff);
    int32_t namelen = transport.readI32();
    // skip the name string and the sequence ID, we don't care about those
    transport.skip(namelen + 4);
  } else {
    if (strict_read) {
      throw_tprotocolexception("No version identifier... old protocol client in strict mode?", BAD_VERSION);
    } else {
      // Handle pre-versioned input
      transport.skip(sz); // skip string body
      messageType = transport.readI8();
      transport.skip(4); // skip sequence number
    }
  }

  if (messageType == T_EXCEPTION) {
    Object ex = createObject("TApplicationException");
    Variant spec = get_static_property("TApplicationException", "_TSPEC");
    binary_deserialize_spec(ex, transport, spec.toArray());
    throw ex;
  }

  Object ret_val = createObject(obj_typename);
  Variant spec = get_static_property(obj_typename, "_TSPEC");
  binary_deserialize_spec(ret_val, transport, spec.toArray());
  return ret_val;
}

///////////////////////////////////////////////////////////////////////////////
}
