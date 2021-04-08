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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/collections.h"

#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/reflection/ext_reflection.h"
#include "hphp/runtime/ext/std/ext_std_classobj.h"
#include "hphp/runtime/ext/thrift/adapter.h"
#include "hphp/runtime/ext/thrift/ext_thrift.h"
#include "hphp/runtime/ext/thrift/spec-holder.h"
#include "hphp/runtime/ext/thrift/transport.h"
#include "hphp/runtime/ext/thrift/util.h"

#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/perf-counters.h"

#include "hphp/util/logger.h"

#include <folly/portability/Sockets.h>
#include <folly/portability/Unistd.h>

#include <sys/types.h>
#include <stdexcept>

namespace HPHP { namespace thrift {
/////////////////////////////////////////////////////////////////////////////

const StaticString
  s_getTransport("getTransport"),
  s_flush("flush"),
  s_onewayFlush("onewayFlush"),
  s_write("write"),
  s_putBack("putBack"),
  s_read("read"),
  s__type("_type"),
  s_collection("collection"),
  s_harray("harray"),
  s_TProtocolException("TProtocolException"),
  s_TApplicationException("TApplicationException");

///////////////////////////////////////////////////////////////////////////////

const int32_t VERSION_MASK = 0xffff0000;
const int32_t VERSION_1 = 0x80010000;
const int8_t T_CALL UNUSED = 1;
const int8_t T_REPLY UNUSED = 2;
const int8_t T_EXCEPTION = 3;
// tprotocolexception
const int INVALID_DATA = 1;
const int BAD_VERSION = 4;

void binary_deserialize_struct(const Object& zthis,
                               PHPInputTransport& transport,
                               int options);
Variant binary_deserialize(int8_t thrift_typeID,
                           PHPInputTransport& transport,
                           const FieldSpec& fieldspec,
                           int options);
void binary_serialize_struct(const Object& zthis,
                             PHPOutputTransport& transport);
void binary_serialize(int8_t thrift_typeID,
                      PHPOutputTransport& transport,
                      const Variant& value,
                      const FieldSpec& fieldspec);
void skip_element(long thrift_typeID, PHPInputTransport& transport);

// Create a PHP object given a typename and call the ctor,
//optionally passing up to 2 arguments
Object createObject(const String& obj_typename, int nargs = 0,
                    const Variant& arg1 = uninit_variant,
                    const Variant& arg2 = uninit_variant) {
  if (!HHVM_FN(class_exists)(obj_typename)) {
    raise_warning("runtime/ext_thrift: Class %s does not exist",
                  obj_typename.data());
    return Object();
  }
  Array args;
  if (nargs == 1) {
    args = make_vec_array(arg1);
  } else if (nargs == 2 ) {
    args = make_vec_array(arg1, arg2);
  }
  return create_object(obj_typename, args);
}

[[noreturn]] NEVER_INLINE
void throw_tprotocolexception(const String& what, long errorcode) {
  throw_object(createObject(s_TProtocolException, 2, what, errorcode));
}

Variant binary_deserialize_internal(int8_t thrift_typeID,
                                    PHPInputTransport& transport,
                                    const FieldSpec& fieldspec,
                                    int options) {
  switch (thrift_typeID) {
    case T_STOP:
    case T_VOID:
      return init_null();
    case T_STRUCT: {
      Variant val;
      Object ret(createObject(fieldspec.className()));
      if (ret.isNull()) {
        // unable to create class entry
        skip_element(T_STRUCT, transport);
        return init_null();
      }
      binary_deserialize_struct(ret, transport, options);
      return ret;
    }
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
        char* strbuf = s.mutableData();
        transport.readBytes(strbuf, size);
        s.setSize(size);
        return s;
      } else {
        return empty_string_variant();
      }
    }
    case T_MAP: { // array of key -> value
      uint8_t types[2];
      transport.readBytes(types, 2);
      uint32_t size = transport.readU32();
      if (s_harray.equal(fieldspec.format)) {
        DictInit arr(size);
        for (uint32_t i = 0; i < size; i++) {
          switch (types[0]) {
            case TType::T_I08:
            case TType::T_I16:
            case TType::T_I32:
            case TType::T_I64: {
              int64_t key = binary_deserialize(
                              types[0], transport, fieldspec.key(), options)
                              .toInt64();
              Variant value = binary_deserialize(
                types[1], transport, fieldspec.val(), options);
              arr.set(key, value);
              break;
            }
            case TType::T_STRING: {
              String key = binary_deserialize(
                             types[0], transport, fieldspec.key(), options)
                             .toString();
              Variant value = binary_deserialize(
                types[1], transport, fieldspec.val(), options);
              arr.set(key, value);
              break;
            }
            default:
              thrift_error(
                  "Unable to deserialize non int/string array keys",
                  ERR_INVALID_DATA);
          }
        }
        return arr.toVariant();
      } else if (s_collection.equal(fieldspec.format)) {
        auto obj(req::make<c_Map>(size));
        for (uint32_t s = 0; s < size; ++s) {
          Variant key = binary_deserialize(
            types[0], transport, fieldspec.key(), options);
          Variant value = binary_deserialize(
            types[1], transport, fieldspec.val(), options);
          collections::set(obj.get(), key.asTypedValue(), value.asTypedValue());
        }
        return Variant(std::move(obj));
      } else {
        DArrayInit arr(size);
        if (options & k_THRIFT_MARK_LEGACY_ARRAYS) {
          arr.setLegacyArray(true);
        }
        for (uint32_t i = 0; i < size; i++) {
          auto key = binary_deserialize(
            types[0], transport, fieldspec.key(), options);
          auto val = binary_deserialize(
            types[1], transport, fieldspec.val(), options);
          set_with_intish_key_cast(arr, key, val);
        }
        return arr.toVariant();
      }
    }
    case T_LIST: { // array with autogenerated numeric keys
      int8_t type = transport.readI8();
      uint32_t size = transport.readU32();
      if (s_harray.equal(fieldspec.format)) {
        VecInit arr(size);
        for (uint32_t i = 0; i < size; i++) {
          arr.append(
            binary_deserialize(type, transport, fieldspec.val(), options));
        }
        return arr.toVariant();
      } else if (s_collection.equal(fieldspec.format)) {
        if (size == 0) {
          return Variant(req::make<c_Vector>());
        }
        auto vec = req::make<c_Vector>(size);
        int64_t i = 0;
        do {
          auto val = binary_deserialize(
            type, transport, fieldspec.val(), options);
          tvDup(*val.asTypedValue(), vec->appendForUnserialize(i));
        } while (++i < size);
        return Variant(std::move(vec));
      } else {
        VArrayInit vai(size);
        if (options & k_THRIFT_MARK_LEGACY_ARRAYS) {
          vai.setLegacyArray(true);
        }
        for (auto s = uint32_t{0}; s < size; ++s) {
          vai.append(
            binary_deserialize(type, transport, fieldspec.val(), options));
        }
        return vai.toVariant();
      }
    }
    case T_SET: { // array of key -> TRUE
      uint8_t type;
      uint32_t size;
      transport.readBytes(&type, 1);
      transport.readBytes(&size, 4);
      size = ntohl(size);
      if (s_harray.equal(fieldspec.format)) {
        KeysetInit arr(size);
        for (uint32_t i = 0; i < size; i++) {
          arr.add(
            binary_deserialize(type, transport, fieldspec.val(), options));
        }
        return arr.toVariant();
      } else if (s_collection.equal(fieldspec.format)) {
        auto set_ret(req::make<c_Set>(size));
        for (uint32_t s = 0; s < size; ++s) {
          Variant key = binary_deserialize(
            type, transport, fieldspec.val(), options);
          if (key.isInteger()) {
            set_ret->add(key);
          } else {
            set_ret->add(key.toString());
          }
        }

        return Variant(std::move(set_ret));
      } else {
        DArrayInit init(size);
        if (options & k_THRIFT_MARK_LEGACY_ARRAYS) {
          init.setLegacyArray(true);
        }
        for (uint32_t s = 0; s < size; ++s) {
          Variant key = binary_deserialize(
            type, transport, fieldspec.val(), options);
          set_with_intish_key_cast(init, key, true);
        }
        return init.toVariant();
      }
    }
  };

  char errbuf[128];
  snprintf(errbuf, sizeof(errbuf), "Unknown thrift typeID %d", thrift_typeID);
  throw_tprotocolexception(String(errbuf, CopyString), INVALID_DATA);
  return init_null();
}

Variant binary_deserialize(int8_t thrift_typeID,
                           PHPInputTransport& transport,
                           const FieldSpec& fieldspec,
                           int options) {
  const auto thriftValue =
      binary_deserialize_internal(thrift_typeID, transport, fieldspec, options);
  return fieldspec.adapter
           ? transformToHackType(thriftValue, *fieldspec.adapter)
           : thriftValue;
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
  snprintf(errbuf, sizeof(errbuf), "Unknown thrift typeID %ld", thrift_typeID);
  throw_tprotocolexception(String(errbuf, CopyString), INVALID_DATA);
}

void binary_serialize_hashtable_key(int8_t keytype,
                                    PHPOutputTransport& transport,
                                    Variant key,
                                    const FieldSpec& fieldspec) {
  bool keytype_is_numeric = (!((keytype == T_STRING) || (keytype == T_UTF8) ||
                               (keytype == T_UTF16)));

  if (keytype_is_numeric) {
    key = key.toInt64();
  } else {
    key = key.toString();
  }
  binary_serialize(keytype, transport, key, fieldspec);
}

inline bool ttype_is_int(int8_t t) {
  return ((t == T_BYTE) || ((t >= T_I16)  && (t <= T_I64)));
}

inline bool ttypes_are_compatible(int8_t t1, int8_t t2) {
  // Integer types of different widths are considered compatible;
  // otherwise the typeID must match.
  return ((t1 == t2) || (ttype_is_int(t1) && ttype_is_int(t2)));
}

NEVER_INLINE
void binary_deserialize_slow(const Object& zthis,
                             const StructSpec& spec,
                             int16_t fieldno,
                             TType ttype,
                             PHPInputTransport& transport,
                             int options) {
  INC_TPC(thrift_read_slow);
  while (ttype != T_STOP) {
    if (const auto* fieldspec = getFieldSlow(spec, fieldno)) {
      if (ttypes_are_compatible(ttype, fieldspec->type)) {
        Variant rv = binary_deserialize(ttype, transport, *fieldspec, options);
        zthis->o_set(StrNR(fieldspec->name), rv, zthis->getClassName());
        if (fieldspec->isUnion) {
          zthis->o_set(s__type, Variant(fieldno), zthis->getClassName());
        }
      } else {
        skip_element(ttype, transport);
      }
    } else {
      skip_element(ttype, transport);
    }
    ttype = static_cast<TType>(transport.readI8());
    if (ttype == T_STOP) return;
    fieldno = transport.readI16();
  }
  assertx(zthis->assertPropTypeHints());
}

void binary_deserialize_struct(const Object& dest,
                               PHPInputTransport& transport,
                               int options) {
  SpecHolder specHolder;
  const auto& fields = specHolder.getSpec(dest->getVMClass(), true);
  const size_t numFields = fields.size();
  Class* cls = dest->getVMClass();
  if (cls->numDeclProperties() < numFields) {
    TType fieldType = static_cast<TType>(transport.readI8());
    int16_t fieldNum = transport.readI16();
    return binary_deserialize_slow(
      dest, fields, fieldNum, fieldType, transport, options);
  }
  auto objProps = dest->props();
  auto prop = cls->declProperties().begin();
  int i = -1;
  TType fieldType = static_cast<TType>(transport.readI8());
  int16_t fieldNum;
  while (fieldType != T_STOP) {
    fieldNum = transport.readI16();
    do {
      ++i;
    } while (i < numFields && fields[i].fieldNum != fieldNum);
    if (i == numFields ||
        prop[i].name != fields[i].name ||
        !ttypes_are_compatible(fieldType, fields[i].type)) {
      // Verify everything we've set so far
      return binary_deserialize_slow(
        dest, fields, fieldNum, fieldType, transport, options);
    }
    if (fields[i].isUnion) {
      if (s__type.equal(prop[numFields].name)) {
        auto index = cls->propSlotToIndex(numFields);
        tvSetInt(fieldNum, objProps->at(index));
      } else {
        return binary_deserialize_slow(
          dest, fields, fieldNum, fieldType, transport, options);
      }
    }
    auto index = cls->propSlotToIndex(i);
    tvSet(*binary_deserialize(fieldType, transport, fields[i], options)
             .asTypedValue(),
          objProps->at(index));

    if (!fields[i].noTypeCheck) {
      dest->verifyPropTypeHint(i);
      if (fields[i].isUnion) dest->verifyPropTypeHint(numFields);
    }
    fieldType = static_cast<TType>(transport.readI8());
  }
  assertx(dest->assertPropTypeHints());
}

void binary_serialize_internal(int8_t thrift_typeID,
                               PHPOutputTransport& transport,
                               const Variant& value,
                               const FieldSpec& fieldspec) {
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
      binary_serialize_struct(value.asCObjRef(), transport);
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
        if (value.is(KindOfObject)) {
          throw_tprotocolexception("Attempt to send object "
                                   "type as a T_STRING", INVALID_DATA);
        }
        String sv = value.toString();
        transport.writeString(sv.data(), sv.size());
    } return;
    case T_MAP: {
      Array ht = value.toArray<IntishCast::Cast>();
      transport.writeI8(fieldspec.ktype);
      transport.writeI8(fieldspec.vtype);
      transport.writeI32(ht.size());
      for (ArrayIter key_ptr = ht.begin(); !key_ptr.end(); ++key_ptr) {
        binary_serialize_hashtable_key(
          fieldspec.ktype, transport, key_ptr.first(), fieldspec.key());
        binary_serialize(
          fieldspec.vtype, transport, key_ptr.second(), fieldspec.val());
      }
    }
      return;
    case T_LIST: {
      Array ht = value.toArray<IntishCast::Cast>();
      transport.writeI8(fieldspec.vtype);
      transport.writeI32(ht.size());
      for (ArrayIter key_ptr = ht.begin(); !key_ptr.end(); ++key_ptr) {
        binary_serialize(
          fieldspec.vtype, transport, key_ptr.second(), fieldspec.val());
      }
    }
      return;
    case T_SET: {
      Array ht = value.toArray<IntishCast::Cast>();
      transport.writeI8(fieldspec.vtype);
      transport.writeI32(ht.size());
      for (ArrayIter key_ptr = ht.begin(); !key_ptr.end(); ++key_ptr) {
        binary_serialize_hashtable_key(
          fieldspec.vtype, transport, key_ptr.first(), fieldspec.val());
      }
    }
      return;
  };
  char errbuf[128];
  snprintf(errbuf, sizeof(errbuf), "Unknown thrift typeID %d", thrift_typeID);
  throw_tprotocolexception(String(errbuf, CopyString), INVALID_DATA);
}

void binary_serialize(int8_t thrift_typeID,
                      PHPOutputTransport& transport,
                      const Variant& value,
                      const FieldSpec& fieldspec) {
  const auto& thriftValue = fieldspec.adapter
                              ? transformToThriftType(value, *fieldspec.adapter)
                              : value;
  binary_serialize_internal(thrift_typeID, transport, thriftValue, fieldspec);
}

void binary_serialize_slow(const FieldSpec& field,
                           const Object& obj,
                           PHPOutputTransport& transport) {
  INC_TPC(thrift_write_slow);
  StrNR fieldName(field.name);
  Variant fieldVal = obj->o_get(fieldName, true, obj->getClassName());
  if (!fieldVal.isNull()) {
    TType fieldType = field.type;
    transport.writeI8(fieldType);
    transport.writeI16(field.fieldNum);
    binary_serialize(fieldType, transport, fieldVal, field);
  }
}

void binary_serialize_struct(const Object& obj, PHPOutputTransport& transport) {
  Class* cls = obj->getVMClass();
  auto prop = cls->declProperties().begin();
  auto objProps = obj->props();
  const size_t numProps = cls->numDeclProperties();

  SpecHolder specHolder;
  const auto& fields = specHolder.getSpec(cls, true);
  const size_t numFields = fields.size();
  // Write each member
  for (int slot = 0; slot < numFields; ++slot) {
    if (slot < numProps && fields[slot].name == prop[slot].name) {
      auto index = cls->propSlotToIndex(slot);
      VarNR fieldWrapper(objProps->at(index).tv());
      const Variant& fieldVal = fieldWrapper;
      if (!fieldVal.isNull()) {
        TType fieldType = fields[slot].type;
        transport.writeI8(fieldType);
        transport.writeI16(fields[slot].fieldNum);
        binary_serialize(fieldType, transport, fieldVal, fields[slot]);
      } else if (UNLIKELY(fieldVal.is(KindOfUninit)) &&
                 (prop[slot].attrs & AttrLateInit)) {
        throw_late_init_prop(prop[slot].cls, prop[slot].name, false);
      }
    } else {
      binary_serialize_slow(fields[slot], obj, transport);
    }
  }
  transport.writeI8(T_STOP); // struct end
}

void HHVM_FUNCTION(thrift_protocol_write_binary,
                   const Object& transportobj,
                   const String& method_name,
                   int64_t msgtype,
                   const Object& request_struct,
                   int seqid,
                   bool strict_write,
                   bool oneway) {

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

  const Object& obj_request_struct = request_struct;

  binary_serialize_struct(obj_request_struct, transport);

  if (oneway) {
    transport.onewayFlush();
  } else {
    transport.flush();
  }
}

Object HHVM_FUNCTION(thrift_protocol_read_binary,
                     const Object& transportobj,
                     const String& obj_typename,
                     bool strict_read,
                     int options) {
  EagerVMRegAnchor _;
  PHPInputTransport transport(transportobj);
  int8_t messageType = 0;
  int32_t sz = transport.readI32();

  if (sz < 0) {
    // Check for correct version number
    int32_t version = sz & VERSION_MASK;
    if (version != VERSION_1) {
      char errbuf[128];
      snprintf(errbuf, sizeof(errbuf), "Bad version identifier, sz=%d", sz);
      throw_tprotocolexception(String(errbuf, CopyString), BAD_VERSION);
    }
    messageType = (sz & 0x000000ff);
    int32_t namelen = transport.readI32();
    // skip the name string and the sequence ID, we don't care about those
    transport.skip(namelen + 4);
  } else {
    if (strict_read) {
      char errbuf[128];
      snprintf(errbuf,
               sizeof(errbuf),
               "No version identifier... "
               "old protocol client in strict mode? sz=%d",
               sz);
      throw_tprotocolexception(String(errbuf, CopyString), BAD_VERSION);
    } else {
      // Handle pre-versioned input
      transport.skip(sz); // skip string body
      messageType = transport.readI8();
      transport.skip(4); // skip sequence number
    }
  }

  if (messageType == T_EXCEPTION) {
    Object ex = createObject(s_TApplicationException);
    binary_deserialize_struct(ex, transport, options);
    throw_object(ex);
  }

  Object ret_val = createObject(obj_typename);
  binary_deserialize_struct(ret_val, transport, options);
  return ret_val;
}

Variant HHVM_FUNCTION(thrift_protocol_read_binary_struct,
                      const Object& transportobj,
                      const String& obj_typename,
                      int options) {
  EagerVMRegAnchor _;
  PHPInputTransport transport(transportobj);

  Object ret_val = createObject(obj_typename);
  binary_deserialize_struct(ret_val, transport, options);
  return ret_val;
}

///////////////////////////////////////////////////////////////////////////////
}}
