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
#include "hphp/runtime/base/strings.h"

#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/reflection/ext_reflection.h"
#include "hphp/runtime/ext/std/ext_std_classobj.h"
#include "hphp/runtime/ext/thrift/adapter.h"
#include "hphp/runtime/ext/thrift/field_wrapper.h"
#include "hphp/runtime/ext/thrift/type_wrapper.h"
#include "hphp/runtime/ext/thrift/ext_thrift.h"
#include "hphp/runtime/ext/thrift/spec-holder.h"
#include "hphp/runtime/ext/thrift/transport.h"
#include "hphp/runtime/ext/thrift/util.h"

#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/runtime/vm/coeffects.h"

#include "hphp/runtime/vm/jit/perf-counters.h"

#include <folly/portability/Sockets.h>
#include <folly/portability/Unistd.h>

namespace HPHP::thrift {
/////////////////////////////////////////////////////////////////////////////

const StaticString
  s_getBuffer("getBuffer"),
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
  s_TTransportException("TTransportException"),
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

[[noreturn]] NEVER_INLINE
void throw_tprotocolexception(const String& what, long errorcode) {
  throw_object(s_TProtocolException, make_vec_array(what, errorcode));
}

inline bool ttype_is_int(int8_t t) {
  return ((t == T_BYTE) || ((t >= T_I16)  && (t <= T_I64)));
}

inline bool ttypes_are_compatible(int8_t t1, int8_t t2) {
  // Integer types of different widths are considered compatible;
  // otherwise the typeID must match.
  return ((t1 == t2) || (ttype_is_int(t1) && ttype_is_int(t2)));
}

template<typename Transport>
struct BinaryReader {
  explicit BinaryReader(Transport&& transport, int options)
      : transport(transport), options(options) {}

 public:
  Object read(const String& resultClassName, 
                     bool strict_read) {
    int8_t messageType = 0;
    int32_t sz = transport.template readBE<int32_t>();

    if (sz < 0) {
      // Check for correct version number
      int32_t version = sz & VERSION_MASK;
      if (version != VERSION_1) {
        char errbuf[128];
        snprintf(errbuf, sizeof(errbuf), "Bad version identifier, sz=%d", sz);
        throw_tprotocolexception(String(errbuf, CopyString), BAD_VERSION);
      }
      messageType = (sz & 0x000000ff);
      int32_t namelen = transport.template readBE<int32_t>();
      // skip the name string and the sequence ID, we don't care about those
      transport.skip(namelen + 4);
    } else {
      if (strict_read) {
        char errbuf[128];
        snprintf(
            errbuf,
            sizeof(errbuf),
            "No version identifier... "
            "old protocol client in strict mode? sz=%d",
            sz);
        throw_tprotocolexception(String(errbuf, CopyString), BAD_VERSION);
      } else {
        // Handle pre-versioned input
        transport.skip(sz); // skip string body
        messageType = transport.template readBE<int8_t>();
        transport.skip(4); // skip sequence number
      }
    }

    if (messageType == T_EXCEPTION) {
      throw_object(readStruct(resultClassName));
    }

    return readStruct(resultClassName);
  }

  Object readStruct(const String& clsName) {
    return binary_deserialize_struct(clsName);
  }

 private:
  Transport transport;
  int options;

  Variant binary_deserialize_internal(
      int8_t thrift_typeID,
      const FieldSpec& fieldspec,
      int options,
      bool& hasTypeWrapper) {
    switch (thrift_typeID) {
      case T_STOP:
      case T_VOID:
        return init_null();
      case T_STRUCT: {
        return binary_deserialize_struct(fieldspec.className());
      }
      case T_BOOL: {
        uint8_t c;
        transport.pull(&c, 1);
        return c != 0;
      }
        // case T_I08: // same numeric value as T_BYTE
      case T_BYTE: {
        uint8_t c;
        transport.pull(&c, 1);
        return Variant((int8_t)c);
      }
      case T_I16: {
        uint16_t c;
        transport.pull(&c, 2);
        return Variant((int16_t)ntohs(c));
      }
      case T_I32: {
        uint32_t c;
        transport.pull(&c, 4);
        return Variant((int32_t)ntohl(c));
      }
      case T_U64:
      case T_I64: {
        uint64_t c;
        transport.pull(&c, 8);
        return Variant((int64_t)ntohll(c));
      }
      case T_DOUBLE: {
        union {
          uint64_t c;
          double d;
        } a;
        transport.pull(&(a.c), 8);
        a.c = ntohll(a.c);
        return a.d;
      }
      case T_FLOAT: {
        union {
          uint32_t c;
          float d;
        } a;
        transport.pull(&(a.c), 4);
        a.c = ntohl(a.c);
        return a.d;
      }
      // case T_UTF7: // aliases T_STRING
      case T_UTF8:
      case T_UTF16:
      case T_STRING: {
        uint32_t size = transport.template readBE<uint32_t>();
        if (size && (size + 1)) {
          String s = String(size, ReserveString);
          char* strbuf = s.mutableData();
          transport.pull(strbuf, size);
          s.setSize(size);
          return s;
        } else {
          return empty_string_variant();
        }
      }
      case T_MAP: { // array of key -> value
        uint8_t types[2];
        transport.pull(types, 2);
        uint32_t size = transport.template readBE<uint32_t>();
        check_container_size(size);
        auto const& key_spec = fieldspec.key();
        auto const& val_spec = fieldspec.val();
        hasTypeWrapper = hasTypeWrapper || val_spec.isTypeWrapped;
        if (s_harray.equal(fieldspec.format)) {
          DictInit arr(size);
          for (uint32_t i = 0; i < size; i++) {
            switch (types[0]) {
              case TType::T_I08:
              case TType::T_I16:
              case TType::T_I32:
              case TType::T_I64: {
                int64_t key = binary_deserialize(
                                  types[0], key_spec, options, hasTypeWrapper)
                                  .toInt64();
                Variant value = binary_deserialize(
                    types[1], val_spec, options, hasTypeWrapper);
                arr.set(key, value);
                break;
              }
              case TType::T_STRING: {
                String key = binary_deserialize(
                                 types[0], key_spec, options, hasTypeWrapper)
                                 .toString();
                Variant value = binary_deserialize(
                    types[1], val_spec, options, hasTypeWrapper);
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
            auto key =
                binary_deserialize(types[0], key_spec, options, hasTypeWrapper);
            auto value =
                binary_deserialize(types[1], val_spec, options, hasTypeWrapper);
            collections::set(
                obj.get(), key.asTypedValue(), value.asTypedValue());
          }
          return Variant(std::move(obj));
        } else {
          DictInit arr(size);
          if (options & k_THRIFT_MARK_LEGACY_ARRAYS) {
            arr.setLegacyArray();
          }
          for (uint32_t i = 0; i < size; i++) {
            auto key =
                binary_deserialize(types[0], key_spec, options, hasTypeWrapper);
            auto val =
                binary_deserialize(types[1], val_spec, options, hasTypeWrapper);
            set_with_intish_key_cast(arr, key, val);
          }
          return arr.toVariant();
        }
      }
      case T_LIST: { // array with autogenerated numeric keys
        int8_t type = transport.template readBE<int8_t>();
        uint32_t size = transport.template readBE<uint32_t>();
        check_container_size(size);
        auto const& val_spec = fieldspec.val();
        hasTypeWrapper = hasTypeWrapper || val_spec.isTypeWrapped;
        if (s_harray.equal(fieldspec.format)) {
          auto arr = initialize_array(size);
          for (uint32_t i = 0; i < size; i++) {
            arr.append(
                binary_deserialize(type, val_spec, options, hasTypeWrapper));
          }
          return arr;
        } else if (s_collection.equal(fieldspec.format)) {
          if (size == 0) {
            return Variant(req::make<c_Vector>());
          }
          auto vec = req::make<c_Vector>(size);
          int64_t i = 0;
          do {
            auto val =
                binary_deserialize(type, val_spec, options, hasTypeWrapper);
            tvDup(*val.asTypedValue(), vec->appendForUnserialize(i));
          } while (++i < size);
          return Variant(std::move(vec));
        } else {
          auto vai = initialize_array(size);
          if (options & k_THRIFT_MARK_LEGACY_ARRAYS) {
            vai.setLegacyArray(true);
          }
          for (auto s = uint32_t{0}; s < size; ++s) {
            vai.append(
                binary_deserialize(type, val_spec, options, hasTypeWrapper));
          }
          return vai;
        }
      }
      case T_SET: { // array of key -> TRUE
        uint8_t type;
        uint32_t size;
        transport.pull(&type, 1);
        transport.pull(&size, 4);
        size = ntohl(size);
        check_container_size(size);
        auto const& val_spec = fieldspec.val();
        if (s_harray.equal(fieldspec.format)) {
          KeysetInit arr(size);
          for (uint32_t i = 0; i < size; i++) {
            arr.add(
                binary_deserialize(type, val_spec, options, hasTypeWrapper));
          }
          return arr.toVariant();
        } else if (s_collection.equal(fieldspec.format)) {
          auto set_ret(req::make<c_Set>(size));
          for (uint32_t s = 0; s < size; ++s) {
            Variant key =
                binary_deserialize(type, val_spec, options, hasTypeWrapper);
            if (key.isInteger()) {
              set_ret->add(key);
            } else {
              set_ret->add(key.toString());
            }
          }

          return Variant(std::move(set_ret));
        } else {
          DictInit init(size);
          if (options & k_THRIFT_MARK_LEGACY_ARRAYS) {
            init.setLegacyArray();
          }
          for (uint32_t s = 0; s < size; ++s) {
            Variant key =
                binary_deserialize(type, val_spec, options, hasTypeWrapper);
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

  Variant binary_deserialize_hack(
      int8_t thrift_typeID,
      const FieldSpec& fieldspec,
      int options,
      bool& hasTypeWrapper) {
    auto thriftValue = binary_deserialize_internal(
        thrift_typeID, fieldspec, options, hasTypeWrapper);
    hasTypeWrapper = hasTypeWrapper || fieldspec.isTypeWrapped;
    return transformToHackType(std::move(thriftValue), *fieldspec.adapter);
  }

  Variant binary_deserialize_thrift(
      int8_t thrift_typeID,
      const FieldSpec& fieldspec,
      int options,
      bool& hasTypeWrapper) {
    auto thriftValue = binary_deserialize_internal(
        thrift_typeID, fieldspec, options, hasTypeWrapper);
    hasTypeWrapper = hasTypeWrapper || fieldspec.isTypeWrapped;
    return thriftValue;
  }

  Variant binary_deserialize(
      int8_t thrift_typeID,
      const FieldSpec& fieldspec,
      int options,
      bool& hasTypeWrapper) {
    if (fieldspec.adapter) {
      return binary_deserialize_hack(
          thrift_typeID, fieldspec, options, hasTypeWrapper);
    }
    return binary_deserialize_thrift(
        thrift_typeID, fieldspec, options, hasTypeWrapper);
  }

  void skip_element(long thrift_typeID) {
    switch (thrift_typeID) {
      case T_STOP:
      case T_VOID:
        return;
      case T_STRUCT:
        while (true) {
          int8_t ttype = transport.template readBE<int8_t>(); // get field type
          if (ttype == T_STOP) break;
          transport.skip(2); // skip field number, I16
          skip_element(ttype); // skip field payload
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
      // case T_UTF7: // aliases T_STRING
      case T_UTF8:
      case T_UTF16:
      case T_STRING: {
        uint32_t len = transport.template readBE<uint32_t>();
        transport.skip(len);
      }
        return;
      case T_MAP: {
        int8_t keytype = transport.template readBE<int8_t>();
        int8_t valtype = transport.template readBE<int8_t>();
        uint32_t size = transport.template readBE<uint32_t>();
        check_container_size(size);
        for (uint32_t i = 0; i < size; ++i) {
          skip_element(keytype);
          skip_element(valtype);
        }
      }
        return;
      case T_LIST:
      case T_SET: {
        int8_t valtype = transport.template readBE<int8_t>();
        uint32_t size = transport.template readBE<uint32_t>();
        check_container_size(size);
        for (uint32_t i = 0; i < size; ++i) {
          skip_element(valtype);
        }
      }
        return;
    };

    char errbuf[128];
    snprintf(
        errbuf, sizeof(errbuf), "Unknown thrift typeID %ld", thrift_typeID);
    throw_tprotocolexception(String(errbuf, CopyString), INVALID_DATA);
  }

  NEVER_INLINE
  void binary_deserialize_slow(
      const Object& zthis,
      const StructSpec& spec,
      int16_t fieldno,
      TType ttype,
      StrictUnionChecker& strictUnionChecker,
      int options) {
    INC_TPC(thrift_read_slow);
    while (ttype != T_STOP) {
      if (const auto* fieldspec = getFieldSlow(spec, fieldno)) {
        if (ttypes_are_compatible(ttype, fieldspec->type)) {
          bool hasTypeWrapper = false;
          Variant rv =
              binary_deserialize(ttype, *fieldspec, options, hasTypeWrapper);
          if (hasTypeWrapper) {
            setThriftField(rv, zthis, StrNR(fieldspec->name));
          } else if (fieldspec->isWrapped) {
            setThriftType(rv, zthis, StrNR(fieldspec->name));
          } else {
            zthis->o_set(StrNR(fieldspec->name), rv, zthis->getClassName());
          }
          if (fieldspec->isUnion) {
            strictUnionChecker.markFieldFound();
            zthis->o_set(s__type, Variant(fieldno), zthis->getClassName());
          }
        } else {
          skip_element(ttype);
        }
      } else {
        skip_element(ttype);
      }
      ttype = static_cast<TType>(transport.template readBE<int8_t>());
      if (ttype == T_STOP) return;
      fieldno = transport.template readBE<int16_t>();
    }
    assertx(zthis->assertPropTypeHints());
  }

  Object binary_deserialize_struct(const String& clsName) {
    auto const cls = Class::load(clsName.get());
    if (cls == nullptr) raise_error(Strings::UNKNOWN_CLASS, clsName.data());

    SpecHolder specHolder;
    auto const& spec = specHolder.getSpec(*cls);
    StrictUnionChecker strictUnionChecker{spec.isStrictUnion};
    Object dest = spec.newObject(*cls);
    spec.clearTerseFields(*cls, dest);

    auto const& fields = spec.fields;
    const size_t numFields = fields.size();
    if (cls->numDeclProperties() < numFields) {
      TType fieldType = static_cast<TType>(transport.template readBE<int8_t>());
      int16_t fieldNum = transport.template readBE<int16_t>();
      binary_deserialize_slow(
          dest, spec, fieldNum, fieldType, strictUnionChecker, options);
      return dest;
    }
    auto objProps = dest->props();
    auto prop = cls->declProperties().begin();
    int i = -1;
    TType fieldType = static_cast<TType>(transport.template readBE<int8_t>());
    int16_t fieldNum;
    while (fieldType != T_STOP) {
      fieldNum = transport.template readBE<int16_t>();
      do {
        ++i;
      } while (i < numFields && fields[i].fieldNum != fieldNum);
      if (i == numFields || prop[i].name != fields[i].name ||
          !ttypes_are_compatible(fieldType, fields[i].type)) {
        // Verify everything we've set so far
        binary_deserialize_slow(
            dest, spec, fieldNum, fieldType, strictUnionChecker, options);
        return dest;
      }
      if (fields[i].isUnion) {
        if (s__type.equal(prop[numFields].name)) {
          strictUnionChecker.markFieldFound();
          auto index = cls->propSlotToIndex(numFields);
          tvSetInt(fieldNum, objProps->at(index));
        } else {
          binary_deserialize_slow(
              dest, spec, fieldNum, fieldType, strictUnionChecker, options);
          return dest;
        }
      }
      auto index = cls->propSlotToIndex(i);
      bool hasTypeWrapper = false;
      auto value =
          binary_deserialize(fieldType, fields[i], options, hasTypeWrapper);
      if (hasTypeWrapper) {
        setThriftField(value, dest, StrNR(fields[i].name));
      } else if (fields[i].isWrapped) {
        setThriftType(value, dest, StrNR(fields[i].name));
      } else {
        tvSet(*value.asTypedValue(), objProps->at(index));
      }

      if (!fields[i].noTypeCheck) {
        dest->verifyPropTypeHint(i);
        if (fields[i].isUnion) dest->verifyPropTypeHint(numFields);
      }
      fieldType = static_cast<TType>(transport.template readBE<int8_t>());
    }
    assertx(dest->assertPropTypeHints());
    return dest;
  }
};

template<typename Transport>
struct BinaryWriter {
  explicit BinaryWriter(Transport& transport) : transport(transport) {}

 public:
  void write(const String& method_name,
             int64_t msgtype,
             int64_t seqid,
             bool strict_write,
             const Object& obj) {
    if (strict_write) {
      int32_t version = VERSION_1 | msgtype;
      writeI32(version);
      writeString(method_name);
      writeI32(seqid);
    } else {
      writeString(method_name);
      writeI8(msgtype);
      writeI32(seqid);
    }
    binary_serialize_struct(obj);
 }

  void writeStruct(const Object& obj) {
    binary_serialize_struct(obj);
  }

 private:
  Transport& transport;

  void writeI8(int8_t n) {
    transport.push(reinterpret_cast<const uint8_t*>(&n), 1);
  }

  void writeI64(int64_t i) {
    i = htonll(i);
    transport.push(reinterpret_cast<const uint8_t*>(&i), 8);
  }

  void writeI32(int32_t i) {
    i = htonl(i);
    transport.push(reinterpret_cast<const uint8_t*>(&i), 4);
  }

  void writeI16(int16_t i) {
    i = htons(i);
    transport.push(reinterpret_cast<const uint8_t*>(&i), 2);
  }

  void writeString(const String& s) {
    auto slice = s.slice();
    uint32_t len = htonl(slice.size());
    transport.push(reinterpret_cast<const uint8_t*>(&len), 4);
    transport.push((uint8_t *) slice.data(), slice.size());
  }

  void binary_serialize_hashtable_key(
      int8_t keytype, Variant key, const FieldSpec& fieldspec) {
    bool keytype_is_numeric = (!(
        (keytype == T_STRING) || (keytype == T_UTF8) || (keytype == T_UTF16)));

    if (keytype_is_numeric) {
      key = key.toInt64();
    } else {
      key = key.toString();
    }
    binary_serialize(keytype, key, fieldspec);
  }

  void binary_serialize_internal(
      int8_t thrift_typeID, const Variant& value, const FieldSpec& fieldspec) {
    // At this point the typeID (and field num, if applicable) should've already
    // been written to the output so all we need to do is write the payload.
    switch (thrift_typeID) {
      case T_STOP:
      case T_VOID:
        return;
      case T_STRUCT: {
        if (!value.is(KindOfObject)) {
          throw_tprotocolexception(
              "Attempt to send non-object "
              "type as a T_STRUCT",
              INVALID_DATA);
        }
        binary_serialize_struct(value.asCObjRef());
      }
        return;
      case T_BOOL:
        writeI8(value.toBoolean() ? 1 : 0);
        return;
      case T_BYTE:
        writeI8((char)value.toInt64());
        return;
      case T_I16:
        writeI16((short)value.toInt64());
        return;
      case T_I32:
        writeI32((int)value.toInt64());
        return;
      case T_I64:
      case T_U64:
        writeI64(value.toInt64());
        return;
      case T_DOUBLE: {
        union {
          int64_t c;
          double d;
        } a;
        a.d = value.toDouble();
        writeI64(a.c);
      }
        return;
      case T_FLOAT: {
        union {
          int32_t c;
          float d;
        } a;
        a.d = (float)value.toDouble();
        writeI32(a.c);
      }
        return;
      // case T_UTF7:
      case T_UTF8:
      case T_UTF16:
      case T_STRING: {
        if (value.is(KindOfObject)) {
          throw_tprotocolexception(
              "Attempt to send object "
              "type as a T_STRING",
              INVALID_DATA);
        }
        String sv = value.toString();
        writeString(sv);
      }
        return;
      case T_MAP: {
        Array ht = value.toArray<IntishCast::Cast>();
        writeI8(fieldspec.ktype);
        writeI8(fieldspec.vtype);
        writeI32(ht.size());
        auto const& key_spec = fieldspec.key();
        auto const& val_spec = fieldspec.val();
        for (ArrayIter key_ptr = ht.begin(); !key_ptr.end(); ++key_ptr) {
          binary_serialize_hashtable_key(
              fieldspec.ktype, key_ptr.first(), key_spec);
          binary_serialize(fieldspec.vtype, key_ptr.second(), val_spec);
        }
      }
        return;
      case T_LIST: {
        Array ht = value.toArray<IntishCast::Cast>();
        writeI8(fieldspec.vtype);
        writeI32(ht.size());
        auto const& val_spec = fieldspec.val();
        for (ArrayIter key_ptr = ht.begin(); !key_ptr.end(); ++key_ptr) {
          binary_serialize(fieldspec.vtype, key_ptr.second(), val_spec);
        }
      }
        return;
      case T_SET: {
        Array ht = value.toArray<IntishCast::Cast>();
        writeI8(fieldspec.vtype);
        writeI32(ht.size());
        auto const& val_spec = fieldspec.val();
        for (ArrayIter key_ptr = ht.begin(); !key_ptr.end(); ++key_ptr) {
          binary_serialize_hashtable_key(
              fieldspec.vtype, key_ptr.first(), val_spec);
        }
      }
        return;
    };
    char errbuf[128];
    snprintf(errbuf, sizeof(errbuf), "Unknown thrift typeID %d", thrift_typeID);
    throw_tprotocolexception(String(errbuf, CopyString), INVALID_DATA);
  }

  void binary_serialize(
      int8_t thrift_typeID, const Variant& value, const FieldSpec& fieldspec) {
    bool isTypeWrappedObj = fieldspec.isTypeWrapped && value.isObject();
    if (!fieldspec.adapter && !isTypeWrappedObj) {
      binary_serialize_internal(thrift_typeID, value, fieldspec);
    } else {
      auto valueCopy =
          isTypeWrappedObj ? getThriftField(value.toObject()) : value;
      if (fieldspec.adapter) {
        const auto thriftValue =
            transformToThriftType(std::move(valueCopy), *fieldspec.adapter);
        binary_serialize_internal(thrift_typeID, thriftValue, fieldspec);
      } else {
        binary_serialize_internal(thrift_typeID, valueCopy, fieldspec);
      }
    }
  }

  void binary_serialize_slow(const FieldSpec& field_spec, const Object& obj) {
    INC_TPC(thrift_write_slow);
    StrNR fieldName(field_spec.name);
    Variant fieldVal;
    if (field_spec.isWrapped) {
      fieldVal = getThriftType(obj, fieldName);
    } else {
      fieldVal = obj->o_get(fieldName, true, obj->getClassName());
    }
    if (!fieldVal.isNull()) {
      TType fieldType = field_spec.type;
      if (field_spec.isTypeWrapped && fieldVal.isObject()) {
        fieldVal = getThriftField(fieldVal.toObject());
      }
      if (field_spec.adapter) {
        fieldVal = transformToThriftType(fieldVal, *field_spec.adapter);
      }
      if (!(field_spec.isTerse && is_value_type_default(fieldType, fieldVal))) {
        writeI8(fieldType);
        writeI16(field_spec.fieldNum);
        binary_serialize_internal(fieldType, fieldVal, field_spec);
      }
    }
  }

  void binary_serialize_struct(const Object& obj) {
    Class& cls = *obj->getVMClass();
    auto prop = cls.declProperties().begin();
    obj->deserializeAllLazyProps();
    auto objProps = obj->props();
    const size_t numProps = cls.numDeclProperties();

    SpecHolder specHolder;
    auto const& fields = specHolder.getSpec(cls).fields;
    const size_t numFields = fields.size();
    // Write each member
    for (int slot = 0; slot < numFields; ++slot) {
      if (slot < numProps && fields[slot].name == prop[slot].name) {
        auto index = cls.propSlotToIndex(slot);
        VarNR fieldWrapper(objProps->at(index).tv());
        Variant fieldVal;
        TType fieldType = fields[slot].type;
        if (fields[slot].isWrapped) {
          fieldVal = getThriftType(obj, StrNR(fields[slot].name));
        } else {
          fieldVal = fieldWrapper;
        }
        if (!fieldVal.isNull()) {
          if (fields[slot].isTypeWrapped && fieldVal.isObject()) {
            fieldVal = getThriftField(fieldVal.toObject());
          }
          if (fields[slot].adapter) {
            fieldVal = transformToThriftType(fieldVal, *fields[slot].adapter);
          }
          if (fields[slot].isTerse &&
              is_value_type_default(fieldType, fieldVal)) {
            continue;
          }
          writeI8(fieldType);
          writeI16(fields[slot].fieldNum);
          binary_serialize_internal(fieldType, fieldVal, fields[slot]);
        } else if (
            UNLIKELY(fieldVal.is(KindOfUninit)) &&
            (prop[slot].attrs & AttrLateInit)) {
          throw_late_init_prop(prop[slot].cls, prop[slot].name, false);
        }
      } else {
        binary_serialize_slow(fields[slot], obj);
      }
    }
    writeI8(T_STOP); // struct end
  }
};

void HHVM_FUNCTION(thrift_protocol_write_binary,
                   const Object& transportobj,
                   const String& method_name,
                   int64_t msgtype,
                   const Object& request_struct,
                   int64_t seqid,
                   bool strict_write,
                   bool oneway) {
  CoeffectsAutoGuard _;
  VMRegAnchor _2;

  // Suppress class-to-string conversion warnings that occur during
  // serialization and deserialization.
  SuppressClassConversionNotice suppressor;

  PHPOutputTransport transport(transportobj);
  const Object& obj_request_struct = request_struct;

  BinaryWriter writer(transport);
  writer.write(
    method_name,
    msgtype,
    seqid,
    strict_write,
    obj_request_struct);

  if (oneway) {
    transport.onewayFlush();
  } else {
    transport.flush();
  }
}

void HHVM_FUNCTION(thrift_protocol_write_binary_struct,
                   const Object& transportobj,
                   const Object& request_struct) {
  CoeffectsAutoGuard _;
  VMRegAnchor _2;
  // Suppress class-to-string conversion warnings that occur during
  // serialization and deserialization.
  SuppressClassConversionNotice suppressor;
  PHPOutputTransport transport(transportobj);
  const Object& obj_request_struct = request_struct;
  BinaryWriter<PHPOutputTransport> writer(transport);
  writer.writeStruct(obj_request_struct);
  transport.flush();
}

Object HHVM_FUNCTION(thrift_protocol_read_binary,
                     const Object& transportobj,
                     const String& obj_typename,
                     bool strict_read,
                     int64_t options) {
  CoeffectsAutoGuard _;
  // Suppress class-to-string conversion warnings that occur during
  // serialization and deserialization.
  SuppressClassConversionNotice suppressor;

  VMRegAnchor _2;
  BinaryReader<PHPInputTransport> reader(
    PHPInputTransport(transportobj),
    options);
  return reader.read(obj_typename, strict_read);
}

Variant HHVM_FUNCTION(thrift_protocol_read_binary_struct,
                      const Object& transportobj,
                      const String& obj_typename,
                      int64_t options) {
  // Suppress class-to-string conversion warnings that occur during
  // serialization and deserialization.
  SuppressClassConversionNotice suppressor;

  VMRegAnchor _;
  BinaryReader<PHPInputTransport> reader(
    PHPInputTransport(transportobj),
    options);
  return reader.readStruct(obj_typename);
}

String HHVM_FUNCTION(thrift_protocol_write_binary_struct_to_string,
                     const Object& request_struct) {
  
  CoeffectsAutoGuard _;
  // Suppress class-to-string conversion warnings that occur during
  // serialization and deserialization.
  SuppressClassConversionNotice suppressor;

  VMRegAnchor _2;
  String ret = String(1024, ReserveString);
  auto iobuf = folly::IOBuf::wrapBufferAsValue(ret.mutableData(), ret.capacity());
  iobuf.clear();
  folly::io::Appender appender(&iobuf, 1024);

  BinaryWriter<folly::io::Appender> writer(appender);
  try {
    writer.writeStruct(request_struct);

    if (iobuf.isChained()) {
      return ioBufToString(iobuf);
    }
    ret.setSize(iobuf.length());
    return ret;
  } catch (const Object&) {
    throw;
  } catch (const std::exception& e) {
    thrift_error(e.what(), ERR_UNKNOWN);
  } catch (...) {
    thrift_error("Unknown error", ERR_UNKNOWN);
  }
}

Object HHVM_FUNCTION(thrift_protocol_read_binary_struct_from_string,
                     const String& serialized,
                     const String& obj_typename,
                     int64_t options) {
  CoeffectsAutoGuard _;
  // Suppress class-to-string conversion warnings that occur during
  // serialization and deserialization.
  SuppressClassConversionNotice suppressor;

  VMRegAnchor _2;
  auto iobuf = folly::IOBuf::wrapBufferAsValue(
    serialized.data(),
    serialized.size());

  try {
    BinaryReader<folly::io::Cursor> reader(
      folly::io::Cursor(&iobuf),
      options);
    return reader.readStruct(obj_typename);
  } catch (const Object&) {
    throw;
  } catch (const std::out_of_range& e) {
    thrift_transport_error(e.what(), TTransportError::END_OF_FILE);
  } catch (const std::exception& e) {
    thrift_error(e.what(), ERR_UNKNOWN);
  } catch (...) {
    thrift_error("Unknown error", ERR_UNKNOWN);
  }
}
///////////////////////////////////////////////////////////////////////////////
}
