/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Hyves (http://www.hyves.nl)                       |
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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/ext/memcached/libmemcached_portability.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/ext/json/ext_json.h"
#include <map>
#include <memory>
#include <vector>
#include <fastlz.h>
#include <zlib.h>

#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// Payload value flags
#define MEMC_VAL_TYPE_MASK     0xf

#define MEMC_VAL_IS_STRING     0
#define MEMC_VAL_IS_LONG       1
#define MEMC_VAL_IS_DOUBLE     2
#define MEMC_VAL_IS_BOOL       3
#define MEMC_VAL_IS_SERIALIZED 4
#define MEMC_VAL_IS_IGBINARY   5
#define MEMC_VAL_IS_JSON       6

#define MEMC_VAL_COMPRESSED         (1<<4)
#define MEMC_VAL_COMPRESSION_ZLIB   (1<<5)
#define MEMC_VAL_COMPRESSION_FASTLZ (1<<6)

#define MEMC_COMPRESS_THRESHOLD 100

#if defined(LIBMEMCACHED_VERSION_HEX) && LIBMEMCACHED_VERSION_HEX < 0x00052000
#  define MEMCACHED_SERVER_TEMPORARILY_DISABLED (1024 << 2)
#endif

// Class options
const int64_t q_Memcached$$OPT_COMPRESSION = -1001;
const int64_t q_Memcached$$OPT_PREFIX_KEY  = -1002;
const int64_t q_Memcached$$OPT_SERIALIZER  = -1003;

// Indicate whether igbinary serializer is available
const bool q_Memcached$$HAVE_IGBINARY = false;

// Indicate whether json serializer is available
const bool q_Memcached$$HAVE_JSON = true;

// libmemcached behavior options
const int64_t q_Memcached$$OPT_HASH
          = MEMCACHED_BEHAVIOR_HASH;
const int64_t q_Memcached$$HASH_DEFAULT
          = MEMCACHED_HASH_DEFAULT;
const int64_t q_Memcached$$HASH_MD5
          = MEMCACHED_HASH_MD5;
const int64_t q_Memcached$$HASH_CRC
          = MEMCACHED_HASH_CRC;
const int64_t q_Memcached$$HASH_FNV1_64
          = MEMCACHED_HASH_FNV1_64;
const int64_t q_Memcached$$HASH_FNV1A_64
          = MEMCACHED_HASH_FNV1A_64;
const int64_t q_Memcached$$HASH_FNV1_32
          = MEMCACHED_HASH_FNV1_32;
const int64_t q_Memcached$$HASH_FNV1A_32
          = MEMCACHED_HASH_FNV1A_32;
const int64_t q_Memcached$$HASH_HSIEH
          = MEMCACHED_HASH_HSIEH;
const int64_t q_Memcached$$HASH_MURMUR
          = MEMCACHED_HASH_MURMUR;
const int64_t q_Memcached$$OPT_DISTRIBUTION
          = MEMCACHED_BEHAVIOR_DISTRIBUTION;
const int64_t q_Memcached$$DISTRIBUTION_MODULA
          = MEMCACHED_DISTRIBUTION_MODULA;
const int64_t q_Memcached$$DISTRIBUTION_CONSISTENT
          = MEMCACHED_DISTRIBUTION_CONSISTENT;
const int64_t q_Memcached$$DISTRIBUTION_CONSISTENT_KETAMA
          = MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA;
#ifdef MEMCACHED_DISTRIBUTION_CONSISTENT_WEIGHTED
const int64_t q_Memcached$$DISTRIBUTION_CONSISTENT_WEIGHTED
          = MEMCACHED_DISTRIBUTION_CONSISTENT_WEIGHTED;
#endif
const int64_t q_Memcached$$OPT_LIBKETAMA_COMPATIBLE
          = MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED;
const int64_t q_Memcached$$OPT_BUFFER_WRITES
          = MEMCACHED_BEHAVIOR_BUFFER_REQUESTS;
const int64_t q_Memcached$$OPT_BINARY_PROTOCOL
          = MEMCACHED_BEHAVIOR_BINARY_PROTOCOL;
const int64_t q_Memcached$$OPT_NO_BLOCK
          = MEMCACHED_BEHAVIOR_NO_BLOCK;
const int64_t q_Memcached$$OPT_TCP_NODELAY
          = MEMCACHED_BEHAVIOR_TCP_NODELAY;
const int64_t q_Memcached$$OPT_SOCKET_SEND_SIZE
          = MEMCACHED_BEHAVIOR_SOCKET_SEND_SIZE;
const int64_t q_Memcached$$OPT_SOCKET_RECV_SIZE
          = MEMCACHED_BEHAVIOR_SOCKET_RECV_SIZE;
const int64_t q_Memcached$$OPT_CONNECT_TIMEOUT
          = MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT;
const int64_t q_Memcached$$OPT_RETRY_TIMEOUT
          = MEMCACHED_BEHAVIOR_RETRY_TIMEOUT;
const int64_t q_Memcached$$OPT_SEND_TIMEOUT
          = MEMCACHED_BEHAVIOR_SND_TIMEOUT;
const int64_t q_Memcached$$OPT_RECV_TIMEOUT
          = MEMCACHED_BEHAVIOR_RCV_TIMEOUT;
const int64_t q_Memcached$$OPT_POLL_TIMEOUT
          = MEMCACHED_BEHAVIOR_POLL_TIMEOUT;
const int64_t q_Memcached$$OPT_CACHE_LOOKUPS
          = MEMCACHED_BEHAVIOR_CACHE_LOOKUPS;
const int64_t q_Memcached$$OPT_SERVER_FAILURE_LIMIT
          = MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT;
#if defined(LIBMEMCACHED_VERSION_HEX) && LIBMEMCACHED_VERSION_HEX >= 0x01000003
const int64_t q_Memcached$$OPT_DEAD_TIMEOUT
          = MEMCACHED_BEHAVIOR_DEAD_TIMEOUT;
#endif
#if defined(LIBMEMCACHED_VERSION_HEX) && LIBMEMCACHED_VERSION_HEX >= 0x00049000
const int64_t q_Memcached$$OPT_REMOVE_FAILED_SERVERS
          = MEMCACHED_BEHAVIOR_REMOVE_FAILED_SERVERS;
#endif

// libmemcached result codes
const int64_t q_Memcached$$RES_SUCCESS
          = MEMCACHED_SUCCESS;
const int64_t q_Memcached$$RES_FAILURE
          = MEMCACHED_FAILURE;
const int64_t q_Memcached$$RES_HOST_LOOKUP_FAILURE
          = MEMCACHED_HOST_LOOKUP_FAILURE;
const int64_t q_Memcached$$RES_UNKNOWN_READ_FAILURE
          = MEMCACHED_UNKNOWN_READ_FAILURE;
const int64_t q_Memcached$$RES_PROTOCOL_ERROR
          = MEMCACHED_PROTOCOL_ERROR;
const int64_t q_Memcached$$RES_CLIENT_ERROR
          = MEMCACHED_CLIENT_ERROR;
const int64_t q_Memcached$$RES_SERVER_ERROR
          = MEMCACHED_SERVER_ERROR;
const int64_t q_Memcached$$RES_WRITE_FAILURE
          = MEMCACHED_WRITE_FAILURE;
const int64_t q_Memcached$$RES_DATA_EXISTS
          = MEMCACHED_DATA_EXISTS;
const int64_t q_Memcached$$RES_NOTSTORED
          = MEMCACHED_NOTSTORED;
const int64_t q_Memcached$$RES_NOTFOUND
          = MEMCACHED_NOTFOUND;
const int64_t q_Memcached$$RES_PARTIAL_READ
          = MEMCACHED_PARTIAL_READ;
const int64_t q_Memcached$$RES_SOME_ERRORS
          = MEMCACHED_SOME_ERRORS;
const int64_t q_Memcached$$RES_NO_SERVERS
          = MEMCACHED_NO_SERVERS;
const int64_t q_Memcached$$RES_END
          = MEMCACHED_END;
const int64_t q_Memcached$$RES_ERRNO
          = MEMCACHED_ERRNO;
const int64_t q_Memcached$$RES_BUFFERED
          = MEMCACHED_BUFFERED;
const int64_t q_Memcached$$RES_TIMEOUT
          = MEMCACHED_TIMEOUT;
const int64_t q_Memcached$$RES_BAD_KEY_PROVIDED
          = MEMCACHED_BAD_KEY_PROVIDED;
const int64_t q_Memcached$$RES_CONNECTION_SOCKET_CREATE_FAILURE
          = MEMCACHED_CONNECTION_SOCKET_CREATE_FAILURE;
const int64_t q_Memcached$$RES_NOT_SUPPORTED
          = MEMCACHED_NOT_SUPPORTED;
const int64_t q_Memcached$$RES_INVALID_HOST_PROTOCOL
          = MEMCACHED_INVALID_HOST_PROTOCOL;
const int64_t q_Memcached$$OPT_VERIFY_KEY
          = MEMCACHED_BEHAVIOR_VERIFY_KEY;
const int64_t q_Memcached$$OPT_SORT_HOSTS
          = MEMCACHED_BEHAVIOR_SORT_HOSTS;
const int64_t q_Memcached$$RES_SERVER_MARKED_DEAD
          = MEMCACHED_SERVER_MARKED_DEAD;
const int64_t q_Memcached$$RES_SERVER_TEMPORARILY_DISABLED
          = MEMCACHED_SERVER_TEMPORARILY_DISABLED;

// Our result codes
const int64_t q_Memcached$$RES_PAYLOAD_FAILURE = -1001;

// Serializer types
const int64_t q_Memcached$$SERIALIZER_PHP      = 1;
const int64_t q_Memcached$$SERIALIZER_IGBINARY = 2;
const int64_t q_Memcached$$SERIALIZER_JSON     = 3;

// Flags
const int64_t q_Memcached$$GET_PRESERVE_ORDER = 1;

// Keys
const StaticString
  s_key("key"),
  s_value("value"),
  s_cas("cas");

// INI settings
struct MEMCACHEDGlobals final {
  std::string sess_prefix;
};
static __thread MEMCACHEDGlobals* s_memcached_globals;
#define MEMCACHEDG(name) s_memcached_globals->name

namespace {
class MemcachedResultWrapper {
public:
  memcached_result_st value;
  explicit MemcachedResultWrapper(memcached_st *memcached) {
    memcached_result_create(memcached, &value);
  }
  ~MemcachedResultWrapper() {
    memcached_result_free(&value);
  }
};
}

/*
 * Since libmemcachd is in C, we cannot use lambda functions as lambda functions
 * are special typed and normally passed by templated types.
 */
static memcached_return_t memcached_dump_callback(const memcached_st*,
                                                  const char* key,
                                                  size_t len, void* context) {
  ((Array*)context)->append(makeStaticString(key, len));
  return MEMCACHED_SUCCESS;
}

const StaticString s_MemcachedData("MemcachedData");

class MemcachedData {
 public:
  class Impl {
   public:
    Impl() :
      compression(true),
      serializer(q_Memcached$$SERIALIZER_PHP),
      rescode(q_Memcached$$RES_SUCCESS) {
      memcached_create(&memcached);
    };
    ~Impl() {
      memcached_free(&memcached);
    }

    memcached_st memcached;
    bool compression;
    int serializer;
    int rescode;
  };
  MemcachedData() {}
  ~MemcachedData() {}

  typedef std::shared_ptr<Impl> ImplPtr;
  ImplPtr m_impl;

  bool handleError(memcached_return status) {
    switch (status) {
      case MEMCACHED_SUCCESS:
      case MEMCACHED_STORED:
      case MEMCACHED_DELETED:
      case MEMCACHED_STAT:
        return true;
      case MEMCACHED_END:
      case MEMCACHED_BUFFERED:
        m_impl->rescode = status;
        return true;
      default:
        m_impl->rescode = status;
        return false;
    }
  }
  void toPayload(const Variant& value, std::vector<char> &payload,
                 uint32_t &flags) {
    String encoded;
    if (value.isString() || value.isNumeric()) {
      encoded = value.toString();
      if      (value.isString())  flags = MEMC_VAL_IS_STRING;
      else if (value.isInteger()) flags = MEMC_VAL_IS_LONG;
      else if (value.isDouble())  flags = MEMC_VAL_IS_DOUBLE;
      else if (value.isBoolean()) flags = MEMC_VAL_IS_BOOL;
      else not_reached();
    } else {
      switch (m_impl->serializer) {
      case q_Memcached$$SERIALIZER_JSON:
        encoded = HHVM_FN(json_encode)(value);
        flags = MEMC_VAL_IS_JSON;
        break;
      default:
        encoded = f_serialize(value);
        flags = MEMC_VAL_IS_SERIALIZED;
        break;
      }
    }

    if (m_impl->compression && encoded.length() >= MEMC_COMPRESS_THRESHOLD) {
      unsigned long payloadCompLength = compressBound(encoded.length());
      payload.resize(payloadCompLength);
      if (compress((Bytef*)payload.data(), &payloadCompLength,
                   (const Bytef*)encoded.data(), encoded.length()) == Z_OK) {
        payload.resize(payloadCompLength);
        flags |= MEMC_VAL_COMPRESSED;
        return;
      }
      raise_warning("could not compress value");
    }

    payload.resize(0);
    payload.insert(payload.end(),
                   encoded.data(), encoded.data() + encoded.length());
  }
  bool toObject(Variant& value, const memcached_result_st &result) {
    const char *payload  = memcached_result_value(&result);
    size_t payloadLength = memcached_result_length(&result);
    uint32_t flags       = memcached_result_flags(&result);

    String decompPayload;
    if (flags & MEMC_VAL_COMPRESSED) {
      bool done = false;
      std::vector<char> buffer;
      unsigned long bufferSize;
      uint32_t maxLength;
      int status;

      /* new-style */
      if ((flags & MEMC_VAL_COMPRESSION_FASTLZ ||
           flags & MEMC_VAL_COMPRESSION_ZLIB)
          && payloadLength > sizeof(uint32_t)) {
        memcpy(&maxLength, payload, sizeof(uint32_t));
        if (maxLength < std::numeric_limits<uint32_t>::max()) {
          buffer.resize(maxLength + 1);
          payloadLength -= sizeof(uint32_t);
          payload += sizeof(uint32_t);
          bufferSize = maxLength;

          if (flags & MEMC_VAL_COMPRESSION_FASTLZ) {
            bufferSize = fastlz_decompress(payload, payloadLength,
                                           buffer.data(), maxLength);
            done = (bufferSize > 0);
          } else if (flags & MEMC_VAL_COMPRESSION_ZLIB) {
            status = uncompress((Bytef *)buffer.data(), &bufferSize,
                                (const Bytef *)payload, (uLong)payloadLength);
            done = (status == Z_OK);
          }
        }
      }

      /* old-style */
      if (!done) {
        for (int factor = 1; factor <= 16; ++factor) {
          if (payloadLength >=
              std::numeric_limits<unsigned long>::max() / (1 << factor)) {
            break;
          }
          bufferSize = payloadLength * (1 << factor) + 1;
          buffer.resize(bufferSize);
          status = uncompress((Bytef*)buffer.data(), &bufferSize,
                              (const Bytef*)payload, (uLong)payloadLength);
          if (status == Z_OK) {
            done = true;
            break;
          } else if (status != Z_BUF_ERROR) {
            break;
          }
        }
      }
      if (!done) {
        raise_warning("could not uncompress value");
        return false;
      }
      decompPayload =
        String::attach(StringData::Make(buffer.data(), bufferSize, CopyString));
    } else {
      decompPayload =
        String::attach(StringData::Make(payload, payloadLength, CopyString));
    }

    switch (flags & MEMC_VAL_TYPE_MASK) {
    case MEMC_VAL_IS_STRING:
      value = decompPayload;
      break;
    case MEMC_VAL_IS_LONG:
      value = decompPayload.toInt64();
      break;
    case MEMC_VAL_IS_DOUBLE:
      value = decompPayload.toDouble();
      break;
    case MEMC_VAL_IS_BOOL:
      value = decompPayload.toBoolean();
      break;
    case MEMC_VAL_IS_JSON:
      value = HHVM_FN(json_decode)(decompPayload);
      break;
    case MEMC_VAL_IS_SERIALIZED:
      value = unserialize_from_string(decompPayload);
      break;
    case MEMC_VAL_IS_IGBINARY:
      raise_warning("could not unserialize value, no igbinary support");
      return false;
    default:
      raise_warning("unknown payload type");
      return false;
    }
    return true;
  }
  memcached_return doCacheCallback(const Variant& callback, ObjectData* this_,
                                   const String& key, Variant& value) {
    Array params(PackedArrayInit(3).append(Variant(this_))
                                   .append(key)
                                   .appendRef(value).toArray());
    if (!vm_call_user_func(callback, params).toBoolean()) {
      return MEMCACHED_NOTFOUND;
    }

    std::vector<char> payload; uint32_t flags;
    toPayload(value, payload, flags);
    return memcached_set(&m_impl->memcached, key.c_str(), key.length(),
                         payload.data(), payload.size(), 0, flags);
  }
  bool getMultiImpl(const String& server_key, const Array& keys, bool enableCas,
                    Array *returnValue) {
    std::vector<const char*> keysCopy;
    keysCopy.reserve(keys.size());
    std::vector<size_t> keysLengthCopy;
    keysLengthCopy.reserve(keys.size());
    for (ArrayIter iter(keys); iter; ++iter) {
      Variant vKey = iter.second();
      if (!vKey.isString()) continue;
      StringData *key = vKey.getStringData();
      if (key->empty()) continue;
      keysCopy.push_back(key->data());
      keysLengthCopy.push_back(key->size());
      if (returnValue) returnValue->set(String(key), init_null(), true);
    }
    if (keysCopy.size() == 0) {
      m_impl->rescode = q_Memcached$$RES_BAD_KEY_PROVIDED;
      return false;
    }

    memcached_behavior_set(&m_impl->memcached, MEMCACHED_BEHAVIOR_SUPPORT_CAS,
                           enableCas ? 1 : 0);
    const char *myServerKey = server_key.empty() ? nullptr : server_key.c_str();
    size_t myServerKeyLen = server_key.length();
    return handleError(memcached_mget_by_key(&m_impl->memcached,
        myServerKey, myServerKeyLen, keysCopy.data(), keysLengthCopy.data(),
        keysCopy.size()));
  }
  bool fetchImpl(memcached_result_st &result, Array &item) {
    memcached_return status;
    if (!memcached_fetch_result(&m_impl->memcached, &result, &status)) {
      handleError(status);
      return false;
    }

    Variant value;
    if (!toObject(value, result)) {
      m_impl->rescode = q_Memcached$$RES_PAYLOAD_FAILURE;
      return false;
    }

    const char *key  = memcached_result_key_value(&result);
    size_t keyLength = memcached_result_key_length(&result);
    String sKey(key, keyLength, CopyString);
    double cas = (double) memcached_result_cas(&result);

    item = make_map_array(s_key, sKey, s_value, value, s_cas, cas);
    return true;
  }
  typedef memcached_return_t (*SetOperation)(memcached_st *,
      const char *, size_t, const char *, size_t, const char *, size_t,
      time_t, uint32_t);

  bool setOperationImpl(SetOperation op, const String& server_key,
                        const String& key, const Variant& value,
                        int expiration) {
    m_impl->rescode = q_Memcached$$RES_SUCCESS;
    if (key.empty()) {
      m_impl->rescode = q_Memcached$$RES_BAD_KEY_PROVIDED;
      return false;
    }

    std::vector<char> payload; uint32_t flags;
    toPayload(value, payload, flags);

    const String& myServerKey = server_key.empty() ? key : server_key;
    return handleError(op(&m_impl->memcached, myServerKey.c_str(),
                          myServerKey.length(), key.c_str(), key.length(),
                          payload.data(), payload.size(), expiration, flags));
  }

  Variant incDecOp(bool isInc,
                   const StringData* server_key, const StringData* key,
                   int64_t offset, int64_t initial_value, int64_t expiry) {
    m_impl->rescode = q_Memcached$$RES_SUCCESS;
    if (key->empty() || strchr(key->data(), ' ')) {
      m_impl->rescode = q_Memcached$$RES_BAD_KEY_PROVIDED;
      return false;
    }
    if (offset < 0) {
      raise_warning("offset has to be >= 0");
      return false;
    }

    // Dispatch to the correct memcached_* function depending on initial_value,
    // server_key, and isInc.
    uint64_t value;
    memcached_return_t status;

    // XXX(#3862): use_initial should really depend on the number of arguments
    // passed to the function but this isn't currently supported by HNI.
    bool use_initial = isBinaryProtocol();
    auto mc = &m_impl->memcached;
    if (use_initial) {
      if (!isBinaryProtocol()) {
        raise_warning("Initial value is only supported with binary protocol");
        return false;
      }

      if (server_key) {
        if (isInc) {
          status = memcached_increment_with_initial_by_key(
            mc,
            server_key->data(), server_key->size(), key->data(), key->size(),
            offset, initial_value, expiry, &value);
        } else {
          status = memcached_decrement_with_initial_by_key(
            mc,
            server_key->data(), server_key->size(), key->data(), key->size(),
            offset, initial_value, expiry, &value);
        }
      } else {
        if (isInc) {
          status = memcached_increment_with_initial(
            mc, key->data(), key->size(),
            offset, initial_value, expiry, &value);
        } else {
          status = memcached_decrement_with_initial(
            mc, key->data(), key->size(),
            offset, initial_value, expiry, &value);
        }
      }
    } else {
      if (server_key) {
        if (isInc) {
          status = memcached_increment_by_key(
            mc,
            server_key->data(), server_key->size(), key->data(), key->size(),
            offset, &value);
        } else {
          status = memcached_decrement_by_key(
            mc,
            server_key->data(), server_key->size(), key->data(), key->size(),
            offset, &value);
        }
      } else {
        if (isInc) {
          status = memcached_increment(
            mc, key->data(), key->size(), offset, &value);
        } else {
          status = memcached_decrement(
            mc, key->data(), key->size(), offset, &value);
        }
      }
    }

    if (!handleError(status)) return false;
    return (int64_t)value;
  }

  bool isBinaryProtocol() {
    return memcached_behavior_get(&m_impl->memcached,
                                  MEMCACHED_BEHAVIOR_BINARY_PROTOCOL);
  }

  typedef std::map<std::string, ImplPtr> ImplMap;
  static DECLARE_THREAD_LOCAL(ImplMap, s_persistentMap);
};

void HHVM_METHOD(Memcached, __construct,
                            const Variant& persistent_id /*= null*/) {
  auto data = Native::data<MemcachedData>(this_);
  if (persistent_id.isNull()) {
    data->m_impl.reset(new MemcachedData::Impl);
  } else {
    MemcachedData::ImplPtr &impl = (*data->s_persistentMap)[
      persistent_id.toString().toCppString()
    ];
    if (!impl) impl.reset(new MemcachedData::Impl);
    data->m_impl = impl;
  }
}

bool HHVM_METHOD(Memcached, quit) {
  auto data = Native::data<MemcachedData>(this_);
  memcached_quit(&data->m_impl->memcached);
  return true;
}

Variant HHVM_METHOD(Memcached, getallkeys) {
  auto data = Native::data<MemcachedData>(this_);
  memcached_dump_fn callbacks[] = {
    &memcached_dump_callback,
  };

  Array allKeys;
  memcached_return status = memcached_dump(&data->m_impl->memcached, callbacks,
                                           &allKeys,
                                           sizeof(callbacks) /
                                             sizeof(memcached_dump_fn));
  if (!data->handleError(status)) {
    return false;
  }

  return allKeys;
}

Variant HHVM_METHOD(Memcached, getbykey, const String& server_key,
                                         const String& key,
                                         const Variant& cache_cb /*= null*/,
                                         VRefParam cas_token /*= null*/) {
  auto data = Native::data<MemcachedData>(this_);
  data->m_impl->rescode = q_Memcached$$RES_SUCCESS;
  if (key.empty()) {
    data->m_impl->rescode = q_Memcached$$RES_BAD_KEY_PROVIDED;
    return false;
  }

  memcached_behavior_set(&data->m_impl->memcached,
                         MEMCACHED_BEHAVIOR_SUPPORT_CAS,
                         cas_token.isReferenced() ? 1 : 0);
  const char *myServerKey = server_key.empty() ? nullptr : server_key.c_str();
  size_t myServerKeyLen = server_key.length();
  const char *myKey = key.c_str();
  size_t myKeyLen = key.length();
  memcached_return status = memcached_mget_by_key(&data->m_impl->memcached,
      myServerKey, myServerKeyLen, &myKey, &myKeyLen, 1);
  if (!data->handleError(status)) return false;

  Variant returnValue;
  MemcachedResultWrapper result(&data->m_impl->memcached);
  if (!memcached_fetch_result(&data->m_impl->memcached,
                              &result.value, &status)) {
    if (status == MEMCACHED_END) status = MEMCACHED_NOTFOUND;
    if (status == MEMCACHED_NOTFOUND && !cache_cb.isNull()) {
      status = data->doCacheCallback(cache_cb, this_, key, returnValue);
      if (!data->handleError(status)) return false;
      if (cas_token.isReferenced()) cas_token = 0.0;
      return returnValue;
    }
    data->handleError(status);
    return false;
  }

  if (!data->toObject(returnValue, result.value)) {
    data->m_impl->rescode = q_Memcached$$RES_PAYLOAD_FAILURE;
    return false;
  }
  if (cas_token.isReferenced()) {
    cas_token = (double) memcached_result_cas(&result.value);
  }
  return returnValue;
}

Variant HHVM_METHOD(Memcached, getmultibykey, const String& server_key,
                               const Array& keys,
                               VRefParam cas_tokens /*= null_variant*/,
                               int flags /*= 0*/) {
  auto data = Native::data<MemcachedData>(this_);
  data->m_impl->rescode = q_Memcached$$RES_SUCCESS;

  bool preserveOrder = flags & q_Memcached$$GET_PRESERVE_ORDER;
  Array returnValue;
  if (!data->getMultiImpl(server_key, keys, cas_tokens.isReferenced(),
                          preserveOrder ? &returnValue : nullptr)) {
    return false;
  }

  Array cas_tokens_arr;
  SCOPE_EXIT { if (cas_tokens.isReferenced()) cas_tokens = cas_tokens_arr; };

  MemcachedResultWrapper result(&data->m_impl->memcached);
  memcached_return status;
  while (memcached_fetch_result(&data->m_impl->memcached, &result.value,
                                &status)) {
    Variant value;
    if (!data->toObject(value, result.value)) {
      data->m_impl->rescode = q_Memcached$$RES_PAYLOAD_FAILURE;
      return false;
    }
    const char *key  = memcached_result_key_value(&result.value);
    size_t keyLength = memcached_result_key_length(&result.value);
    String sKey(key, keyLength, CopyString);
    returnValue.set(sKey, value, true);
    if (cas_tokens.isReferenced()) {
      double cas = (double) memcached_result_cas(&result.value);
      cas_tokens_arr.set(sKey, cas, true);
    }
  }

  if (status != MEMCACHED_END && !data->handleError(status)) return false;
  return returnValue;
}

bool HHVM_METHOD(Memcached, getdelayedbykey, const String& server_key,
                            const Array& keys, bool with_cas /*= false*/,
                            const Variant& value_cb /*= null_variant*/) {
  auto data = Native::data<MemcachedData>(this_);
  data->m_impl->rescode = q_Memcached$$RES_SUCCESS;

  if (!data->getMultiImpl(server_key, keys, with_cas, nullptr)) return false;
  if (value_cb.isNull()) return true;

  MemcachedResultWrapper result(&data->m_impl->memcached); Array item;
  while (data->fetchImpl(result.value, item)) {
    vm_call_user_func(value_cb, make_packed_array(Variant(this_), item));
  }

  if (data->m_impl->rescode != q_Memcached$$RES_END) return false;
  data->m_impl->rescode = q_Memcached$$RES_SUCCESS;
  return true;
}

Variant HHVM_METHOD(Memcached, fetch) {
  auto data = Native::data<MemcachedData>(this_);
  data->m_impl->rescode = q_Memcached$$RES_SUCCESS;

  MemcachedResultWrapper result(&data->m_impl->memcached); Array item;
  if (!data->fetchImpl(result.value, item)) return false;

  return item;
}

Variant HHVM_METHOD(Memcached, fetchall) {
  auto data = Native::data<MemcachedData>(this_);
  data->m_impl->rescode = q_Memcached$$RES_SUCCESS;

  Array returnValue;
  MemcachedResultWrapper result(&data->m_impl->memcached); Array item;
  while (data->fetchImpl(result.value, item)) {
    returnValue.append(item);
  }

  if (data->m_impl->rescode != q_Memcached$$RES_END) return false;
  return returnValue;
}

bool HHVM_METHOD(Memcached, setbykey, const String& server_key,
                                      const String& key, const Variant& value,
                                      int expiration /*= 0*/) {
  auto data = Native::data<MemcachedData>(this_);
  return data->setOperationImpl(memcached_set_by_key, server_key, key, value,
                          expiration);
}

bool HHVM_METHOD(Memcached, addbykey, const String& server_key,
                                      const String& key, const Variant& value,
                                      int expiration /*= 0*/) {
  auto data = Native::data<MemcachedData>(this_);
  return data->setOperationImpl(memcached_add_by_key, server_key, key, value,
                                                      expiration);
}

bool HHVM_METHOD(Memcached, appendbykey, const String& server_key,
                                         const String& key,
                                         const String& value) {
  auto data = Native::data<MemcachedData>(this_);
  if (data->m_impl->compression) {
    raise_warning("cannot append/prepend with compression turned on");
    return false;
  }
  return data->setOperationImpl(memcached_append_by_key, server_key, key,
                                value, 0);
}

bool HHVM_METHOD(Memcached, prependbykey, const String& server_key,
                                          const String& key,
                                          const String& value) {
  auto data = Native::data<MemcachedData>(this_);
  if (data->m_impl->compression) {
    raise_warning("cannot append/prepend with compression turned on");
    return false;
  }
  return data->setOperationImpl(memcached_prepend_by_key, server_key, key,
                                value, 0);
}

bool HHVM_METHOD(Memcached, replacebykey, const String& server_key,
                                          const String& key,
                                          const Variant& value,
                                          int expiration /*= 0*/) {
  auto data = Native::data<MemcachedData>(this_);
  return data->setOperationImpl(memcached_replace_by_key, server_key, key,
                                value, expiration);
}

bool HHVM_METHOD(Memcached, casbykey, double cas_token,
                                      const String& server_key,
                                      const String& key,
                                      const Variant& value,
                                      int expiration /*= 0*/) {
  auto data = Native::data<MemcachedData>(this_);
  data->m_impl->rescode = q_Memcached$$RES_SUCCESS;
  if (key.empty()) {
    data->m_impl->rescode = q_Memcached$$RES_BAD_KEY_PROVIDED;
    return false;
  }

  std::vector<char> payload; uint32_t flags;
  data->toPayload(value, payload, flags);

  const String& myServerKey = server_key.empty() ? key : server_key;
  return data->handleError(memcached_cas_by_key(&data->m_impl->memcached,
      myServerKey.c_str(), myServerKey.length(), key.c_str(), key.length(),
      payload.data(), payload.size(), expiration, flags, (uint64_t)cas_token));
}

bool HHVM_METHOD(Memcached, deletebykey, const String& server_key,
                                         const String& key,
                                         int time /*= 0*/) {
  auto data = Native::data<MemcachedData>(this_);
  data->m_impl->rescode = q_Memcached$$RES_SUCCESS;
  if (key.empty()) {
    data->m_impl->rescode = q_Memcached$$RES_BAD_KEY_PROVIDED;
    return false;
  }

  const String& myServerKey = server_key.empty() ? key : server_key;
  return data->handleError(memcached_delete_by_key(&data->m_impl->memcached,
                     myServerKey.c_str(), myServerKey.length(),
                     key.c_str(), key.length(), time));
}

Variant HHVM_METHOD(Memcached, increment,
                    const String& key,
                    int64_t offset /* = 1 */,
                    int64_t initial_value /* = 0 */,
                    int64_t expiry /* = 0 */) {
  return Native::data<MemcachedData>(this_)->incDecOp(
    true, nullptr, key.get(), offset, initial_value, expiry);
}

Variant HHVM_METHOD(Memcached, incrementbykey,
                    const String& server_key,
                    const String& key,
                    int64_t offset /* = 1 */,
                    int64_t initial_value /* = 0 */,
                    int64_t expiry /* = 0 */) {
  return Native::data<MemcachedData>(this_)->incDecOp(
    true, server_key.get(), key.get(), offset, initial_value, expiry);
}

Variant HHVM_METHOD(Memcached, decrement,
                    const String& key,
                    int64_t offset /* = 1 */,
                    int64_t initial_value /* = 0 */,
                    int64_t expiry /* = 0 */) {
  return Native::data<MemcachedData>(this_)->incDecOp(
    false, nullptr, key.get(), offset, initial_value, expiry);
}

Variant HHVM_METHOD(Memcached, decrementbykey,
                    const String& server_key,
                    const String& key,
                    int64_t offset /* = 1 */,
                    int64_t initial_value /* = 0 */,
                    int64_t expiry /* = 0 */) {
  return Native::data<MemcachedData>(this_)->incDecOp(
    false, server_key.get(), key.get(), offset, initial_value, expiry);
}

bool HHVM_METHOD(Memcached, addserver, const String& host, int port,
                                       int weight /*= 0*/) {
  auto data = Native::data<MemcachedData>(this_);
  data->m_impl->rescode = q_Memcached$$RES_SUCCESS;
  if (!host.empty() && host[0] == '/') {
    return data->handleError(memcached_server_add_unix_socket_with_weight(
        &data->m_impl->memcached, host.c_str(), weight));
  } else {
    return data->handleError(memcached_server_add_with_weight(
      &data->m_impl->memcached, host.c_str(), port, weight
    ));
  }
}

namespace {

const StaticString s_host("host"), s_port("port");
#ifdef LMCD_SERVER_QUERY_INCLUDES_WEIGHT
const StaticString s_weight("weight");
#endif

memcached_return_t doServerListCallback(const memcached_st *ptr,
    LMCD_SERVER_CALLBACK_INSTANCE_TYPE server, void *context) {
  Array *returnValue = (Array*) context;
  const char* hostname = LMCD_SERVER_HOSTNAME(server);
  in_port_t port = LMCD_SERVER_PORT(server);
#ifdef LMCD_SERVER_QUERY_INCLUDES_WEIGHT
  returnValue->append(make_map_array(s_host, String(hostname, CopyString),
                                     s_port, (int32_t)port,
                                     s_weight, (int32_t)server->weight));
#else
  returnValue->append(make_map_array(s_host, String(hostname, CopyString),
                                     s_port, (int32_t)port));
#endif
  return MEMCACHED_SUCCESS;
}
}

Array HHVM_METHOD(Memcached, getserverlist) {
  auto data = Native::data<MemcachedData>(this_);
  Array returnValue = Array::Create();
  memcached_server_function callbacks[] = { doServerListCallback };
  memcached_server_cursor(&data->m_impl->memcached, callbacks, &returnValue, 1);
  return returnValue;
}

bool HHVM_METHOD(Memcached, resetserverlist) {
  auto data = Native::data<MemcachedData>(this_);
  memcached_servers_reset(&data->m_impl->memcached);
  return true;
}

Variant HHVM_METHOD(Memcached, getserverbykey, const String& server_key) {
  auto data = Native::data<MemcachedData>(this_);
  data->m_impl->rescode = q_Memcached$$RES_SUCCESS;
  if (server_key.empty()) {
    data->m_impl->rescode = q_Memcached$$RES_BAD_KEY_PROVIDED;
    return false;
  }

  memcached_return_t error;
  LMCD_SERVER_BY_KEY_INSTANCE_TYPE server = memcached_server_by_key(
    &data->m_impl->memcached, server_key.c_str(), server_key.size(), &error);
  if (!server) {
    data->handleError(error);
    return false;
  }

  const char* hostname = LMCD_SERVER_HOSTNAME(server);
  in_port_t port = LMCD_SERVER_PORT(server);
#ifdef LMCD_SERVER_QUERY_INCLUDES_WEIGHT
  Array returnValue = make_map_array(s_host, String(hostname, CopyString),
                                     s_port, (int32_t)port,
                                     s_weight, (int32_t)server->weight);
#else
  Array returnValue = make_map_array(s_host, String(hostname, CopyString),
                                     s_port, (int32_t)port);
#endif
  return returnValue;
}

namespace {
struct StatsContext {
  memcached_stat_st *stats;
  Array returnValue;
};

const StaticString
  s_pid("pid"),
  s_uptime("uptime"),
  s_threads("threads"),
  s_time("time"),
  s_pointer_size("pointer_size"),
  s_rusage_user_seconds("rusage_user_seconds"),
  s_rusage_user_microseconds("rusage_user_microseconds"),
  s_rusage_system_seconds("rusage_system_seconds"),
  s_rusage_system_microseconds("rusage_system_microseconds"),
  s_curr_items("curr_items"),
  s_total_items("total_items"),
  s_limit_maxbytes("limit_maxbytes"),
  s_curr_connections("curr_connections"),
  s_total_connections("total_connections"),
  s_connection_structures("connection_structures"),
  s_bytes("bytes"),
  s_cmd_get("cmd_get"),
  s_cmd_set("cmd_set"),
  s_get_hits("get_hits"),
  s_get_misses("get_misses"),
  s_evictions("evictions"),
  s_bytes_read("bytes_read"),
  s_bytes_written("bytes_written"),
  s_version("version");

memcached_return_t doStatsCallback(const memcached_st *ptr,
    LMCD_SERVER_CALLBACK_INSTANCE_TYPE server, void *inContext) {
  StatsContext *context = (StatsContext*) inContext;
  char key[NI_MAXHOST + 6];
  const char* hostname = LMCD_SERVER_HOSTNAME(server);
  in_port_t port = LMCD_SERVER_PORT(server);
  snprintf(key, sizeof(key), "%s:%d", hostname, port);
  memcached_stat_st *stats = context->stats;
  ssize_t i = context->returnValue.size();

  context->returnValue.set(String(key, CopyString),
    make_map_array(
      s_pid,                        (int64_t)stats[i].pid,
      s_uptime,                     (int64_t)stats[i].uptime,
      s_threads,                    (int64_t)stats[i].threads,
      s_time,                       (int64_t)stats[i].time,
      s_pointer_size,               (int64_t)stats[i].pointer_size,
      s_rusage_user_seconds,        (int64_t)stats[i].rusage_user_seconds,
      s_rusage_user_microseconds,   (int64_t)stats[i]
                                                .rusage_user_microseconds,
      s_rusage_system_seconds,      (int64_t)stats[i].rusage_system_seconds,
      s_rusage_system_microseconds, (int64_t)stats[i]
                                                .rusage_system_microseconds,
      s_curr_items,                 (int64_t)stats[i].curr_items,
      s_total_items,                (int64_t)stats[i].total_items,
      s_limit_maxbytes,             (int64_t)stats[i].limit_maxbytes,
      s_curr_connections,           (int64_t)stats[i].curr_connections,
      s_total_connections,          (int64_t)stats[i].total_connections,
      s_connection_structures,      (int64_t)stats[i].connection_structures,
      s_bytes,                      (int64_t)stats[i].bytes,
      s_cmd_get,                    (int64_t)stats[i].cmd_get,
      s_cmd_set,                    (int64_t)stats[i].cmd_set,
      s_get_hits,                   (int64_t)stats[i].get_hits,
      s_get_misses,                 (int64_t)stats[i].get_misses,
      s_evictions,                  (int64_t)stats[i].evictions,
      s_bytes_read,                 (int64_t)stats[i].bytes_read,
      s_bytes_written,              (int64_t)stats[i].bytes_written,
      s_version,                    String(stats[i].version, CopyString)
    )
  );

  return MEMCACHED_SUCCESS;
}
}

Variant HHVM_METHOD(Memcached, getstats) {
  auto data = Native::data<MemcachedData>(this_);
  memcached_return_t error;
  memcached_stat_st *stats = memcached_stat(&data->m_impl->memcached,
                                            nullptr, &error);
  if (!stats) {
    data->handleError(error);
    return false;
  }

  memcached_server_function callbacks[] = { doStatsCallback };
  StatsContext context; context.stats = stats;
  memcached_server_cursor(&data->m_impl->memcached, callbacks, &context, 1);

  memcached_stat_free(&data->m_impl->memcached, stats);
  return context.returnValue;
}

namespace {
memcached_return_t doVersionCallback(const memcached_st *ptr,
    LMCD_SERVER_CALLBACK_INSTANCE_TYPE server, void *context) {
  Array *returnValue = (Array*) context;
  char key[NI_MAXHOST + 6], version[16];

  const char* hostname = LMCD_SERVER_HOSTNAME(server);
  in_port_t port = LMCD_SERVER_PORT(server);
  uint8_t majorVersion = LMCD_SERVER_MAJOR_VERSION(server);
  uint8_t minorVersion = LMCD_SERVER_MINOR_VERSION(server);
  uint8_t microVersion = LMCD_SERVER_MICRO_VERSION(server);

  snprintf(key, sizeof(key), "%s:%d", hostname, port);
  snprintf(version, sizeof(version), "%" PRIu8 ".%" PRIu8 ".%" PRIu8,
           majorVersion, minorVersion, microVersion);
  returnValue->set(String(key, CopyString), String(version, CopyString));
  return MEMCACHED_SUCCESS;
}
}

Variant HHVM_METHOD(Memcached, getversion) {
  auto data = Native::data<MemcachedData>(this_);
  memcached_return_t status = memcached_version(&data->m_impl->memcached);
  if (!data->handleError(status)) return false;

  Array returnValue;
  memcached_server_function callbacks[] = { doVersionCallback };
  memcached_server_cursor(&data->m_impl->memcached, callbacks, &returnValue, 1);
  return returnValue;
}

bool HHVM_METHOD(Memcached, flush, int delay /*= 0*/) {
  auto data = Native::data<MemcachedData>(this_);
  return data->handleError(memcached_flush(&data->m_impl->memcached, delay));
}

Variant HHVM_METHOD(Memcached, getoption, int option) {
  auto data = Native::data<MemcachedData>(this_);
  switch (option) {
  case q_Memcached$$OPT_COMPRESSION:
    return data->m_impl->compression;

  case q_Memcached$$OPT_PREFIX_KEY:
    {
      memcached_return retval;
      char *result = (char*) memcached_callback_get(&data->m_impl->memcached,
          MEMCACHED_CALLBACK_PREFIX_KEY, &retval);
      if (retval == MEMCACHED_SUCCESS && result) {
        return String(result, CopyString);
      }
      else return empty_string_variant();
    }

  case q_Memcached$$OPT_SERIALIZER:
    return data->m_impl->serializer;

  case MEMCACHED_BEHAVIOR_SOCKET_SEND_SIZE:
  case MEMCACHED_BEHAVIOR_SOCKET_RECV_SIZE:
    if (memcached_server_count(&data->m_impl->memcached) == 0) {
      raise_warning("no servers defined");
      return init_null();
    }
    // fall through

  default:
    // Assume that it's a libmemcached behavior option
    return (int64_t) memcached_behavior_get(&data->m_impl->memcached,
                                          (memcached_behavior_t)option);
  }
}

bool HHVM_METHOD(Memcached, setoption, int option, const Variant& value) {
  auto data = Native::data<MemcachedData>(this_);
  switch (option) {
  case q_Memcached$$OPT_COMPRESSION:
    data->m_impl->compression = value.toBoolean();
    break;

  case q_Memcached$$OPT_PREFIX_KEY:
    {
      String sValue = value.toString();
      char *key = const_cast<char*>(sValue.empty() ? nullptr : sValue.c_str());
      if (memcached_callback_set(&data->m_impl->memcached,
          MEMCACHED_CALLBACK_PREFIX_KEY, key) == MEMCACHED_BAD_KEY_PROVIDED) {
        raise_warning("bad key provided");
        return false;
      }
      break;
    }

  case MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED:
    {
      uint64_t lValue = value.toInt64();
      if (memcached_behavior_set(&data->m_impl->memcached,
          MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED, lValue) == MEMCACHED_FAILURE) {
        raise_warning("error setting memcached option");
        return false;
      }

      /* This is necessary because libmemcached doesn't reset hash/distribution
       * options on false case, like it does for MEMCACHED_BEHAVIOR_KETAMA
       * (non-weighted) case. We have to clean up ourselves.
       */
      if (!lValue) {
        memcached_behavior_set_key_hash(&data->m_impl->memcached,
                                        MEMCACHED_HASH_DEFAULT);
        memcached_behavior_set_distribution_hash(&data->m_impl->memcached,
                                                 MEMCACHED_HASH_DEFAULT);
        memcached_behavior_set_distribution(&data->m_impl->memcached,
                                            MEMCACHED_DISTRIBUTION_MODULA);
      }
      break;
    }

  case q_Memcached$$OPT_SERIALIZER:
    {
      int iValue = value.toInt32(10);
      switch (iValue) {
      case q_Memcached$$SERIALIZER_PHP:
      case q_Memcached$$SERIALIZER_JSON:
        data->m_impl->serializer = iValue;
        break;
      default:
        data->m_impl->serializer = q_Memcached$$SERIALIZER_PHP;
        raise_warning("invalid serializer provided");
        return false;
      }
      break;
    }

  default:
    {
      if ((option < 0) || (option >= MEMCACHED_BEHAVIOR_MAX)) {
        raise_warning("error setting memcached option");
        return false;
      }

      // Assume that it's a libmemcached behavior option
      uint64_t lValue = value.toInt64();
      if (memcached_behavior_set(&data->m_impl->memcached,
          (memcached_behavior_t)option, lValue) == MEMCACHED_FAILURE) {
        raise_warning("error setting memcached option");
        return false;
      }
      break;
    }
  }
  return true;
}

int64_t HHVM_METHOD(Memcached, getresultcode) {
  auto data = Native::data<MemcachedData>(this_);
  return data->m_impl->rescode;
}

String HHVM_METHOD(Memcached, getresultmessage) {
  auto data = Native::data<MemcachedData>(this_);
  if (data->m_impl->rescode == q_Memcached$$RES_PAYLOAD_FAILURE) {
    return "PAYLOAD FAILURE";
  } else {
    return memcached_strerror(&data->m_impl->memcached,
                              (memcached_return_t)data->m_impl->rescode);
  }
}

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL(MemcachedData::ImplMap, MemcachedData::s_persistentMap);

const StaticString s_Memcached("Memcached");
const StaticString s_DISTRIBUTION_CONSISTENT("DISTRIBUTION_CONSISTENT");
const StaticString s_DISTRIBUTION_CONSISTENT_KETAMA("DISTRIBUTION_CONSISTENT_KETAMA");
#ifdef MEMCACHED_DISTRIBUTION_CONSISTENT_WEIGHTED
const StaticString s_DISTRIBUTION_CONSISTENT_WEIGHTED("DISTRIBUTION_CONSISTENT_WEIGHTED");
#endif
const StaticString s_DISTRIBUTION_MODULA("DISTRIBUTION_MODULA");
const StaticString s_GET_PRESERVE_ORDER("GET_PRESERVE_ORDER");
const StaticString s_HASH_CRC("HASH_CRC");
const StaticString s_HASH_DEFAULT("HASH_DEFAULT");
const StaticString s_HASH_FNV1_32("HASH_FNV1_32");
const StaticString s_HASH_FNV1_64("HASH_FNV1_64");
const StaticString s_HASH_FNV1A_32("HASH_FNV1A_32");
const StaticString s_HASH_FNV1A_64("HASH_FNV1A_64");
const StaticString s_HASH_HSIEH("HASH_HSIEH");
const StaticString s_HASH_MD5("HASH_MD5");
const StaticString s_HASH_MURMUR("HASH_MURMUR");
const StaticString s_HAVE_IGBINARY("HAVE_IGBINARY");
const StaticString s_HAVE_JSON("HAVE_JSON");
const StaticString s_OPT_BINARY_PROTOCOL("OPT_BINARY_PROTOCOL");
const StaticString s_OPT_BUFFER_WRITES("OPT_BUFFER_WRITES");
const StaticString s_OPT_CACHE_LOOKUPS("OPT_CACHE_LOOKUPS");
const StaticString s_OPT_COMPRESSION("OPT_COMPRESSION");
const StaticString s_OPT_CONNECT_TIMEOUT("OPT_CONNECT_TIMEOUT");
#if defined(LIBMEMCACHED_VERSION_HEX) && LIBMEMCACHED_VERSION_HEX >= 0x01000003
const StaticString s_OPT_DEAD_TIMEOUT("OPT_DEAD_TIMEOUT");
#endif
const StaticString s_OPT_DISTRIBUTION("OPT_DISTRIBUTION");
const StaticString s_OPT_HASH("OPT_HASH");
const StaticString s_OPT_LIBKETAMA_COMPATIBLE("OPT_LIBKETAMA_COMPATIBLE");
const StaticString s_OPT_NO_BLOCK("OPT_NO_BLOCK");
const StaticString s_OPT_POLL_TIMEOUT("OPT_POLL_TIMEOUT");
const StaticString s_OPT_PREFIX_KEY("OPT_PREFIX_KEY");
const StaticString s_OPT_RECV_TIMEOUT("OPT_RECV_TIMEOUT");
#if defined(LIBMEMCACHED_VERSION_HEX) && LIBMEMCACHED_VERSION_HEX >= 0x00049000
const StaticString s_OPT_REMOVE_FAILED_SERVERS("OPT_REMOVE_FAILED_SERVERS");
#endif
const StaticString s_OPT_RETRY_TIMEOUT("OPT_RETRY_TIMEOUT");
const StaticString s_OPT_SEND_TIMEOUT("OPT_SEND_TIMEOUT");
const StaticString s_OPT_SERIALIZER("OPT_SERIALIZER");
const StaticString s_OPT_SERVER_FAILURE_LIMIT("OPT_SERVER_FAILURE_LIMIT");
const StaticString s_OPT_SOCKET_RECV_SIZE("OPT_SOCKET_RECV_SIZE");
const StaticString s_OPT_SOCKET_SEND_SIZE("OPT_SOCKET_SEND_SIZE");
const StaticString s_OPT_SORT_HOSTS("OPT_SORT_HOSTS");
const StaticString s_OPT_TCP_NODELAY("OPT_TCP_NODELAY");
const StaticString s_OPT_VERIFY_KEY("OPT_VERIFY_KEY");
const StaticString s_RES_BAD_KEY_PROVIDED("RES_BAD_KEY_PROVIDED");
const StaticString s_RES_BUFFERED("RES_BUFFERED");
const StaticString s_RES_CLIENT_ERROR("RES_CLIENT_ERROR");
const StaticString
 s_RES_CONNECTION_SOCKET_CREATE_FAILURE("RES_CONNECTION_SOCKET_CREATE_FAILURE");
const StaticString s_RES_DATA_EXISTS("RES_DATA_EXISTS");
const StaticString s_RES_END("RES_END");
const StaticString s_RES_ERRNO("RES_ERRNO");
const StaticString s_RES_FAILURE("RES_FAILURE");
const StaticString s_RES_HOST_LOOKUP_FAILURE("RES_HOST_LOOKUP_FAILURE");
const StaticString s_RES_INVALID_HOST_PROTOCOL("RES_INVALID_HOST_PROTOCOL");
const StaticString s_RES_NO_SERVERS("RES_NO_SERVERS");
const StaticString s_RES_NOT_SUPPORTED("RES_NOT_SUPPORTED");
const StaticString s_RES_NOTFOUND("RES_NOTFOUND");
const StaticString s_RES_NOTSTORED("RES_NOTSTORED");
const StaticString s_RES_PARTIAL_READ("RES_PARTIAL_READ");
const StaticString s_RES_PAYLOAD_FAILURE("RES_PAYLOAD_FAILURE");
const StaticString s_RES_PROTOCOL_ERROR("RES_PROTOCOL_ERROR");
const StaticString s_RES_SERVER_ERROR("RES_SERVER_ERROR");
const StaticString s_RES_SERVER_MARKED_DEAD("RES_SERVER_MARKED_DEAD");
const StaticString
 s_RES_SERVER_TEMPORARILY_DISABLED("RES_SERVER_TEMPORARILY_DISABLED");
const StaticString s_RES_SOME_ERRORS("RES_SOME_ERRORS");
const StaticString s_RES_SUCCESS("RES_SUCCESS");
const StaticString s_RES_TIMEOUT("RES_TIMEOUT");
const StaticString s_RES_UNKNOWN_READ_FAILURE("RES_UNKNOWN_READ_FAILURE");
const StaticString s_RES_WRITE_FAILURE("RES_WRITE_FAILURE");
const StaticString s_SERIALIZER_IGBINARY("SERIALIZER_IGBINARY");
const StaticString s_SERIALIZER_JSON("SERIALIZER_JSON");
const StaticString s_SERIALIZER_PHP("SERIALIZER_PHP");

class MemcachedExtension final : public Extension {
 public:
  MemcachedExtension() : Extension("memcached", "2.2.0b1") {}
  void threadInit() override {
    if (s_memcached_globals) {
      return;
    }
    s_memcached_globals = new MEMCACHEDGlobals;
    IniSetting::Bind(this, IniSetting::PHP_INI_ALL,
                     "memcached.sess_prefix", &MEMCACHEDG(sess_prefix));
  }

  void threadShutdown() override {
    delete s_memcached_globals;
    s_memcached_globals = nullptr;
  }

  void moduleInit() override {
    HHVM_ME(Memcached, __construct);
    HHVM_ME(Memcached, quit);
    HHVM_ME(Memcached, getallkeys);
    HHVM_ME(Memcached, getbykey);
    HHVM_ME(Memcached, getmultibykey);
    HHVM_ME(Memcached, getdelayedbykey);
    HHVM_ME(Memcached, fetch);
    HHVM_ME(Memcached, fetchall);
    HHVM_ME(Memcached, setbykey);
    HHVM_ME(Memcached, addbykey);
    HHVM_ME(Memcached, appendbykey);
    HHVM_ME(Memcached, prependbykey);
    HHVM_ME(Memcached, replacebykey);
    HHVM_ME(Memcached, casbykey);
    HHVM_ME(Memcached, deletebykey);
    HHVM_ME(Memcached, increment);
    HHVM_ME(Memcached, incrementbykey);
    HHVM_ME(Memcached, decrement);
    HHVM_ME(Memcached, decrementbykey);
    HHVM_ME(Memcached, addserver);
    HHVM_ME(Memcached, getserverlist);
    HHVM_ME(Memcached, resetserverlist);
    HHVM_ME(Memcached, getserverbykey);
    HHVM_ME(Memcached, getstats);
    HHVM_ME(Memcached, getversion);
    HHVM_ME(Memcached, flush);
    HHVM_ME(Memcached, getoption);
    HHVM_ME(Memcached, setoption);
    HHVM_ME(Memcached, getresultcode);
    HHVM_ME(Memcached, getresultmessage);

    Native::registerNativeDataInfo<MemcachedData>(s_MemcachedData.get());

    Native::registerClassConstant<KindOfBoolean>(
      s_Memcached.get(), s_HAVE_IGBINARY.get(), q_Memcached$$HAVE_IGBINARY
    );
    Native::registerClassConstant<KindOfBoolean>(
      s_Memcached.get(), s_HAVE_JSON.get(), q_Memcached$$HAVE_JSON
    );

    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_DISTRIBUTION_CONSISTENT.get(),
      q_Memcached$$DISTRIBUTION_CONSISTENT
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_DISTRIBUTION_CONSISTENT_KETAMA.get(),
      q_Memcached$$DISTRIBUTION_CONSISTENT_KETAMA
    );
#ifdef MEMCACHED_DISTRIBUTION_CONSISTENT_WEIGHTED
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_DISTRIBUTION_CONSISTENT_WEIGHTED.get(),
      q_Memcached$$DISTRIBUTION_CONSISTENT_WEIGHTED
    );
#endif
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_DISTRIBUTION_MODULA.get(),
      q_Memcached$$DISTRIBUTION_MODULA
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_GET_PRESERVE_ORDER.get(),
      q_Memcached$$GET_PRESERVE_ORDER
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_HASH_CRC.get(), q_Memcached$$HASH_CRC
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_HASH_DEFAULT.get(), q_Memcached$$HASH_DEFAULT
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_HASH_FNV1_32.get(), q_Memcached$$HASH_FNV1_32
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_HASH_FNV1_64.get(), q_Memcached$$HASH_FNV1_64
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_HASH_FNV1A_32.get(), q_Memcached$$HASH_FNV1A_32
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_HASH_FNV1A_64.get(), q_Memcached$$HASH_FNV1A_64
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_HASH_HSIEH.get(), q_Memcached$$HASH_HSIEH
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_HASH_MD5.get(), q_Memcached$$HASH_MD5
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_HASH_MURMUR.get(), q_Memcached$$HASH_MURMUR
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_BINARY_PROTOCOL.get(),
      q_Memcached$$OPT_BINARY_PROTOCOL
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_BUFFER_WRITES.get(),
      q_Memcached$$OPT_BUFFER_WRITES
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_CACHE_LOOKUPS.get(),
      q_Memcached$$OPT_CACHE_LOOKUPS
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_COMPRESSION.get(), q_Memcached$$OPT_COMPRESSION
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_CONNECT_TIMEOUT.get(),
      q_Memcached$$OPT_CONNECT_TIMEOUT
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_DISTRIBUTION.get(), q_Memcached$$OPT_DISTRIBUTION
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_HASH.get(), q_Memcached$$OPT_HASH
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_LIBKETAMA_COMPATIBLE.get(),
      q_Memcached$$OPT_LIBKETAMA_COMPATIBLE
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_NO_BLOCK.get(), q_Memcached$$OPT_NO_BLOCK
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_POLL_TIMEOUT.get(), q_Memcached$$OPT_POLL_TIMEOUT
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_PREFIX_KEY.get(), q_Memcached$$OPT_PREFIX_KEY
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_RECV_TIMEOUT.get(), q_Memcached$$OPT_RECV_TIMEOUT
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_RETRY_TIMEOUT.get(),
      q_Memcached$$OPT_RETRY_TIMEOUT
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_SEND_TIMEOUT.get(), q_Memcached$$OPT_SEND_TIMEOUT
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_SERIALIZER.get(), q_Memcached$$OPT_SERIALIZER
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_SERVER_FAILURE_LIMIT.get(),
      q_Memcached$$OPT_SERVER_FAILURE_LIMIT
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_SOCKET_RECV_SIZE.get(),
      q_Memcached$$OPT_SOCKET_RECV_SIZE
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_SOCKET_SEND_SIZE.get(),
      q_Memcached$$OPT_SOCKET_SEND_SIZE
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_TCP_NODELAY.get(), q_Memcached$$OPT_TCP_NODELAY
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_BAD_KEY_PROVIDED.get(),
      q_Memcached$$RES_BAD_KEY_PROVIDED
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_BUFFERED.get(), q_Memcached$$RES_BUFFERED
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_CLIENT_ERROR.get(), q_Memcached$$RES_CLIENT_ERROR
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_CONNECTION_SOCKET_CREATE_FAILURE.get(),
      q_Memcached$$RES_CONNECTION_SOCKET_CREATE_FAILURE
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_DATA_EXISTS.get(), q_Memcached$$RES_DATA_EXISTS
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_END.get(), q_Memcached$$RES_END
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_ERRNO.get(), q_Memcached$$RES_ERRNO
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_FAILURE.get(), q_Memcached$$RES_FAILURE
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_HOST_LOOKUP_FAILURE.get(),
      q_Memcached$$RES_HOST_LOOKUP_FAILURE
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_INVALID_HOST_PROTOCOL.get(),
      q_Memcached$$RES_INVALID_HOST_PROTOCOL
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_NO_SERVERS.get(), q_Memcached$$RES_NO_SERVERS
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_NOT_SUPPORTED.get(),
      q_Memcached$$RES_NOT_SUPPORTED
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_NOTFOUND.get(), q_Memcached$$RES_NOTFOUND
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_NOTSTORED.get(), q_Memcached$$RES_NOTSTORED
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_PARTIAL_READ.get(), q_Memcached$$RES_PARTIAL_READ
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_PAYLOAD_FAILURE.get(),
      q_Memcached$$RES_PAYLOAD_FAILURE
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_PROTOCOL_ERROR.get(),
      q_Memcached$$RES_PROTOCOL_ERROR
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_SERVER_ERROR.get(), q_Memcached$$RES_SERVER_ERROR
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_SOME_ERRORS.get(), q_Memcached$$RES_SOME_ERRORS
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_SUCCESS.get(), q_Memcached$$RES_SUCCESS
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_TIMEOUT.get(), q_Memcached$$RES_TIMEOUT
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_UNKNOWN_READ_FAILURE.get(),
      q_Memcached$$RES_UNKNOWN_READ_FAILURE
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_WRITE_FAILURE.get(),
      q_Memcached$$RES_WRITE_FAILURE
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_SERIALIZER_IGBINARY.get(),
      q_Memcached$$SERIALIZER_IGBINARY
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_SERIALIZER_JSON.get(), q_Memcached$$SERIALIZER_JSON
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_SERIALIZER_PHP.get(), q_Memcached$$SERIALIZER_PHP
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_VERIFY_KEY.get(), q_Memcached$$OPT_VERIFY_KEY
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_SORT_HOSTS.get(), q_Memcached$$OPT_SORT_HOSTS
    );
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_SERVER_MARKED_DEAD.get(),
      q_Memcached$$RES_SERVER_MARKED_DEAD
    );
#if defined(LIBMEMCACHED_VERSION_HEX) && LIBMEMCACHED_VERSION_HEX >= 0x00049000
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_REMOVE_FAILED_SERVERS.get(),
      q_Memcached$$OPT_REMOVE_FAILED_SERVERS
    );
#endif
#if defined(LIBMEMCACHED_VERSION_HEX) && LIBMEMCACHED_VERSION_HEX >= 0x01000003
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_OPT_DEAD_TIMEOUT.get(),
      q_Memcached$$OPT_DEAD_TIMEOUT
    );
#endif
    Native::registerClassConstant<KindOfInt64>(
      s_Memcached.get(), s_RES_SERVER_TEMPORARILY_DISABLED.get(),
      q_Memcached$$RES_SERVER_TEMPORARILY_DISABLED
    );


    loadSystemlib();
  }
} s_memcached_extension;

}
