/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/thrift/transport.h"
#include "hphp/runtime/ext/ext_thrift.h"
#include "hphp/runtime/ext/ext_class.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/ext/reflection/ext_reflection.h"
#include "hphp/runtime/base/base-includes.h"
#include "hphp/util/logger.h"

#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>

namespace HPHP {

StaticString PHPTransport::s_getTransport("getTransport");
StaticString PHPTransport::s_flush("flush");
StaticString PHPTransport::s_write("write");
StaticString PHPTransport::s_putBack("putBack");
StaticString PHPTransport::s_read("read");
StaticString PHPTransport::s_class("class");
StaticString PHPTransport::s_key("key");
StaticString PHPTransport::s_val("val");
StaticString PHPTransport::s_elem("elem");
StaticString PHPTransport::s_var("var");
StaticString PHPTransport::s_type("type");
StaticString PHPTransport::s_ktype("ktype");
StaticString PHPTransport::s_vtype("vtype");
StaticString PHPTransport::s_etype("etype");
StaticString PHPTransport::s_format("format");
StaticString PHPTransport::s_collection("collection");

IMPLEMENT_DEFAULT_EXTENSION_VERSION(thrift_protocol, NO_EXTENSION_VERSION_YET);

///////////////////////////////////////////////////////////////////////////////

const int32_t VERSION_MASK = 0xffff0000;
const int32_t VERSION_1 = 0x80010000;
const int8_t T_CALL = 1;
const int8_t T_REPLY = 2;
const int8_t T_EXCEPTION = 3;
// tprotocolexception
const int INVALID_DATA = 1;
const int BAD_VERSION = 4;

void binary_deserialize_spec(const Object& zthis, PHPInputTransport& transport, const Array& spec);
void binary_serialize_spec(const Object& zthis, PHPOutputTransport& transport, const Array& spec);
void binary_serialize(int8_t thrift_typeID, PHPOutputTransport& transport, const Variant& value, const Array& fieldspec);
void skip_element(long thrift_typeID, PHPInputTransport& transport);

// Create a PHP object given a typename and call the ctor, optionally passing up to 2 arguments
Object createObject(const String& obj_typename, int nargs = 0,
                    const Variant& arg1 = null_variant, const Variant& arg2 = null_variant) {
  if (!f_class_exists(obj_typename)) {
    raise_warning("runtime/ext_thrift: Class %s does not exist",
                  obj_typename.data());
    return Object();
  }
  Array args;
  if (nargs == 1) {
    args = make_packed_array(arg1);
  } else if (nargs == 2 ) {
    args = make_packed_array(arg1, arg2);
  }
  return create_object(obj_typename, args);
}

void throw_tprotocolexception(const String& what, long errorcode) {
  Object ex = createObject("TProtocolException", 2, what, errorcode);
  throw ex;
}

const StaticString s_TSPEC("_TSPEC");

Variant binary_deserialize(int8_t thrift_typeID, PHPInputTransport& transport,
                           const Array& fieldspec) {
  Variant ret;
  switch (thrift_typeID) {
    case T_STOP:
    case T_VOID:
      return uninit_null();
    case T_STRUCT: {
      Variant val;
      if ((val = fieldspec.rvalAt(PHPTransport::s_class)).isNull()) {
        throw_tprotocolexception("no class type in spec", INVALID_DATA);
        skip_element(T_STRUCT, transport);
        return uninit_null();
      }
      String structType = val.toString();
      ret = createObject(structType);
      if (ret.isNull()) {
        // unable to create class entry
        skip_element(T_STRUCT, transport);
        return uninit_null();
      }
      Variant spec = HHVM_FN(hphp_get_static_property)(structType, s_TSPEC,
                                                                   false);
      if (!spec.is(KindOfArray)) {
        char errbuf[128];
        snprintf(errbuf, 128, "spec for %s is wrong type: %d\n",
                 structType.data(), ret.getType());
        throw_tprotocolexception(String(errbuf, CopyString), INVALID_DATA);
        return uninit_null();
      }
      binary_deserialize_spec(ret.toObject(), transport, spec.toArray());
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
      return Variant((int8_t)c);
    }
    case T_I16: {
      uint16_t c;
      transport.readBytes(&c, 2);
      return Variant((int16_t)ntohs(c));
    }
    case T_I32: {
      uint32_t c;
      transport.readBytes(&c, 4);
      return Variant((int32_t)ntohl(c));
    }
    case T_U64:
    case T_I64: {
      uint64_t c;
      transport.readBytes(&c, 8);
      return Variant((int64_t)ntohll(c));
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
    case T_FLOAT: {
      union {
        uint32_t c;
        float d;
      } a;
      transport.readBytes(&(a.c), 4);
      a.c = ntohl(a.c);
      return a.d;
    }
    //case T_UTF7: // aliases T_STRING
    case T_UTF8:
    case T_UTF16:
    case T_STRING: {
      uint32_t size = transport.readU32();
      if (size && (size + 1)) {
        String s = String(size, ReserveString);
        char* strbuf = s.bufferSlice().ptr;
        transport.readBytes(strbuf, size);
        return s.setSize(size);
      } else {
        return "";
      }
    }
    case T_MAP: { // array of key -> value
      uint8_t types[2];
      transport.readBytes(types, 2);
      uint32_t size = transport.readU32();

      Array keyspec = fieldspec.rvalAt(PHPTransport::s_key,
                                       AccessFlags::Error_Key).toArray();
      Array valspec = fieldspec.rvalAt(PHPTransport::s_val,
                                       AccessFlags::Error_Key).toArray();
      String format = fieldspec.rvalAt(PHPTransport::s_format,
                                       AccessFlags::None).toString();
      if (format.equal(PHPTransport::s_collection)) {
        ret = NEWOBJ(c_Map)();
        for (uint32_t s = 0; s < size; ++s) {
          Variant key = binary_deserialize(types[0], transport, keyspec);
          Variant value = binary_deserialize(types[1], transport, valspec);
          collectionSet(ret.getObjectData(), key.asCell(), value.asCell());
        }
      } else {
        ret = Array::Create();
        for (uint32_t s = 0; s < size; ++s) {
          Variant key = binary_deserialize(types[0], transport, keyspec);
          Variant value = binary_deserialize(types[1], transport, valspec);
          ret.toArrRef().set(key, value);
        }
      }
      return ret; // return_value already populated
    }
    case T_LIST: { // array with autogenerated numeric keys
      int8_t type = transport.readI8();
      uint32_t size = transport.readU32();
      Variant elemvar = fieldspec.rvalAt(PHPTransport::s_elem,
                                         AccessFlags::Error_Key);
      Array elemspec = elemvar.toArray();
      String format = fieldspec.rvalAt(PHPTransport::s_format,
                                       AccessFlags::None).toString();

      if (format.equal(PHPTransport::s_collection)) {
        auto const pvec = NEWOBJ(c_Vector)();
        ret = pvec;
        for (uint32_t s = 0; s < size; ++s) {
          Variant value = binary_deserialize(type, transport, elemspec);
          pvec->t_add(value);
        }
      } else {
        PackedArrayInit pai(size);
        for (auto s = uint32_t{0}; s < size; ++s) {
          pai.append(binary_deserialize(type, transport, elemspec));
        }
        ret = pai.toArray();
      }

      return ret;
    }
    case T_SET: { // array of key -> TRUE
      uint8_t type;
      uint32_t size;
      transport.readBytes(&type, 1);
      transport.readBytes(&size, 4);
      size = ntohl(size);
      Variant elemvar = fieldspec.rvalAt(PHPTransport::s_elem,
                                         AccessFlags::Error_Key);
      Array elemspec = elemvar.toArray();
      String format = fieldspec.rvalAt(PHPTransport::s_format,
                                       AccessFlags::None).toString();
      if (format.equal(PHPTransport::s_collection)) {
        p_Set set_ret = NEWOBJ(c_Set)();

        for (uint32_t s = 0; s < size; ++s) {
          Variant key = binary_deserialize(type, transport, elemspec);

          if (key.isInteger()) {
            set_ret->t_add(key);
          } else {
            set_ret->t_add(key.toString());
          }
        }

        ret = Variant(set_ret);
      } else {
        ArrayInit init(size);
        for (uint32_t s = 0; s < size; ++s) {
          Variant key = binary_deserialize(type, transport, elemspec);
          if (key.isInteger()) {
            init.set(key, true);
          } else {
            init.set(key.toString(), true);
          }
        }
        ret = init.toArray();
      }
      return ret;
    }
  };

  char errbuf[128];
  sprintf(errbuf, "Unknown thrift typeID %d", thrift_typeID);
  throw_tprotocolexception(String(errbuf, CopyString), INVALID_DATA);
  return uninit_null();
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
    case T_FLOAT:
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

void binary_deserialize_spec(const Object& zthis, PHPInputTransport& transport,
                             const Array& spec) {
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
      String varname = fieldspec.rvalAt(PHPTransport::s_var).toString();

      // and the type
      int8_t expected_ttype = fieldspec.rvalAt(PHPTransport::s_type).toInt64();

      if (ttypes_are_compatible(ttype, expected_ttype)) {
        Variant rv = binary_deserialize(ttype, transport, fieldspec);
        zthis->o_set(varname, rv, zthis->o_getClassName());
      } else {
        skip_element(ttype, transport);
      }
    } else {
      skip_element(ttype, transport);
    }
  }
}

void binary_serialize(int8_t thrift_typeID, PHPOutputTransport& transport,
                      const Variant& value, const Array& fieldspec) {
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
      binary_serialize_spec(value.toObject(), transport,
                            HHVM_FN(hphp_get_static_property)(value.toObject()->
                                                       o_getClassName(),
                                                       s_TSPEC,
                                                       false).toArray());
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
    case T_FLOAT: {
      union {
        int32_t c;
        float d;
      } a;
      a.d = (float)value.toDouble();
      transport.writeI32(a.c);
    } return;
    //case T_UTF7:
    case T_UTF8:
    case T_UTF16:
    case T_STRING: {
        String sv = value.toString();
        transport.writeString(sv.data(), sv.size());
    } return;
    case T_MAP: {
      Array ht = value.toArray();
      uint8_t keytype = fieldspec.rvalAt(PHPTransport::s_ktype,
                                         AccessFlags::Error_Key).toByte();
      transport.writeI8(keytype);
      uint8_t valtype = fieldspec.rvalAt(PHPTransport::s_vtype,
                                         AccessFlags::Error_Key).toByte();
      transport.writeI8(valtype);

      Array valspec = fieldspec.rvalAt(PHPTransport::s_val,
                                       AccessFlags::Error_Key).toArray();

      transport.writeI32(ht.size());
      for (ArrayIter key_ptr = ht.begin(); !key_ptr.end(); ++key_ptr) {
        binary_serialize_hashtable_key(keytype, transport, key_ptr.first());
        binary_serialize(valtype, transport, key_ptr.second(), valspec);
      }
    } return;
    case T_LIST: {
      Array ht = value.toArray();
      Variant val;

      uint8_t valtype = fieldspec.rvalAt(PHPTransport::s_etype,
                                         AccessFlags::Error_Key).toInt64();
      transport.writeI8(valtype);
      Array valspec = fieldspec.rvalAt(PHPTransport::s_elem,
                                       AccessFlags::Error_Key).toArray();
      transport.writeI32(ht.size());
      for (ArrayIter key_ptr = ht.begin(); !key_ptr.end(); ++key_ptr) {
        binary_serialize(valtype, transport, key_ptr.second(), valspec);
      }
    } return;
    case T_SET: {
      Array ht = value.toArray();

      uint8_t keytype = fieldspec.rvalAt(PHPTransport::s_etype,
                                         AccessFlags::Error_Key).toByte();
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


void binary_serialize_spec(const Object& zthis, PHPOutputTransport& transport,
                           const Array& spec) {
  for (ArrayIter key_ptr = spec.begin(); !key_ptr.end(); ++key_ptr) {
    Variant key = key_ptr.first();
    if (!key.isInteger()) {
      throw_tprotocolexception("Bad keytype in TSPEC (expected 'long')", INVALID_DATA);
      return;
    }
    unsigned long fieldno = key.toInt64();
    Array fieldspec = key_ptr.second().toArray();

    // field name
    String varname = fieldspec.rvalAt(PHPTransport::s_var,
                                      AccessFlags::Error_Key).toString();

    // thrift type
    int8_t ttype = fieldspec.rvalAt(PHPTransport::s_type,
                                    AccessFlags::Error_Key).toByte();

    Variant prop = zthis->o_get(varname, true, zthis->o_getClassName());
    if (!prop.isNull()) {
      transport.writeI8(ttype);
      transport.writeI16(fieldno);
      binary_serialize(ttype, transport, prop, fieldspec);
    }
  }
  transport.writeI8(T_STOP); // struct end
}

void f_thrift_protocol_write_binary(const Object& transportobj, const String& method_name,
                                    int64_t msgtype, const Object& request_struct,
                                    int seqid, bool strict_write) {

  PHPOutputTransport transport(transportobj);

  if (strict_write) {
    int32_t version = VERSION_1 | msgtype;
    transport.writeI32(version);
    transport.writeString(method_name.data(), method_name.size());
    transport.writeI32(seqid);
  } else {
    transport.writeString(method_name.data(), method_name.size());
    transport.writeI8(msgtype);
    transport.writeI32(seqid);
  }

  Variant spec = HHVM_FN(hphp_get_static_property)(
                                            request_struct->o_getClassName(),
                                            s_TSPEC,
                                            false);
  binary_serialize_spec(request_struct, transport, spec.toArray());

  transport.flush();
}

Variant f_thrift_protocol_read_binary(const Object& transportobj,
                                      const String& obj_typename,
                                      bool strict_read) {
  PHPInputTransport transport(transportobj);
  int8_t messageType = 0;
  int32_t sz = transport.readI32();

  if (sz < 0) {
    // Check for correct version number
    int32_t version = sz & VERSION_MASK;
    if (version != VERSION_1) {
      char errbuf[128];
      sprintf(errbuf, "Bad version identifier, sz=%d", sz);
      throw_tprotocolexception(String(errbuf, CopyString), BAD_VERSION);
    }
    messageType = (sz & 0x000000ff);
    int32_t namelen = transport.readI32();
    // skip the name string and the sequence ID, we don't care about those
    transport.skip(namelen + 4);
  } else {
    if (strict_read) {
      char errbuf[128];
      sprintf(errbuf, "No version identifier... old protocol client in strict mode? sz=%d", sz);
      throw_tprotocolexception(String(errbuf, CopyString), BAD_VERSION);
    } else {
      // Handle pre-versioned input
      transport.skip(sz); // skip string body
      messageType = transport.readI8();
      transport.skip(4); // skip sequence number
    }
  }

  if (messageType == T_EXCEPTION) {
    Object ex = createObject("TApplicationException");
    Variant spec = HHVM_FN(hphp_get_static_property)("TApplicationException",
                                                     s_TSPEC, false);
    binary_deserialize_spec(ex, transport, spec.toArray());
    throw ex;
  }

  Object ret_val = createObject(obj_typename);
  Variant spec = HHVM_FN(hphp_get_static_property)(obj_typename, s_TSPEC,
                                                   false);
  binary_deserialize_spec(ret_val, transport, spec.toArray());
  return ret_val;
}

///////////////////////////////////////////////////////////////////////////////
}
