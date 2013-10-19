/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Hyves (http://www.hyves.nl)                       |
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_memcached.h"
#include "hphp/runtime/ext/libmemcached_portability.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/ext/ext_json.h"
#include <zlib.h>

#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DEFAULT_EXTENSION(memcached);
IMPLEMENT_THREAD_LOCAL(c_Memcached::ImplMap, c_Memcached::s_persistentMap);

// Payload value flags
#define MEMC_VAL_TYPE_MASK     0xf

#define MEMC_VAL_IS_STRING     0
#define MEMC_VAL_IS_LONG       1
#define MEMC_VAL_IS_DOUBLE     2
#define MEMC_VAL_IS_BOOL       3
#define MEMC_VAL_IS_SERIALIZED 4
#define MEMC_VAL_IS_IGBINARY   5
#define MEMC_VAL_IS_JSON       6

#define MEMC_VAL_COMPRESSED    (1<<4)

#define MEMC_COMPRESS_THRESHOLD 100

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

// Our result codes
const int64_t q_Memcached$$RES_PAYLOAD_FAILURE = -1001;

// Serializer types
const int64_t q_Memcached$$SERIALIZER_PHP      = 1;
const int64_t q_Memcached$$SERIALIZER_IGBINARY = 2;
const int64_t q_Memcached$$SERIALIZER_JSON     = 3;

// Flags
const int64_t q_Memcached$$GET_PRESERVE_ORDER = 1;


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


c_Memcached::c_Memcached(Class* cb) :
    ExtObjectData(cb) {
}
c_Memcached::~c_Memcached() {
}

c_Memcached::Impl::Impl() :
    compression(true),
    serializer(q_Memcached$$SERIALIZER_PHP),
    rescode(q_Memcached$$RES_SUCCESS) {
  memcached_create(&memcached);
}

c_Memcached::Impl::~Impl() {
  memcached_free(&memcached);
}

void c_Memcached::t___construct(const String& persistent_id /*= null_string*/) {
  if (persistent_id.isNull()) {
    m_impl.reset(new Impl);
  } else {
    ImplPtr &impl = (*s_persistentMap)[persistent_id->toCPPString()];
    if (!impl) impl.reset(new Impl);
    m_impl = impl;
  }
}

Variant c_Memcached::t_get(const String& key, CVarRef cache_cb /*= null_variant*/,
                           VRefParam cas_token /*= null_variant*/) {
  return t_getbykey(null_string, key, cache_cb, cas_token);
}

Variant c_Memcached::t_getbykey(const String& server_key, const String& key,
                                CVarRef cache_cb /*= null_variant*/,
                                VRefParam cas_token /*= null_variant*/) {
  m_impl->rescode = q_Memcached$$RES_SUCCESS;
  if (key.empty()) {
    m_impl->rescode = q_Memcached$$RES_BAD_KEY_PROVIDED;
    return false;
  }

  memcached_behavior_set(&m_impl->memcached, MEMCACHED_BEHAVIOR_SUPPORT_CAS,
                         cas_token.isReferenced() ? 1 : 0);
  const char *myServerKey = server_key.empty() ? NULL : server_key.c_str();
  size_t myServerKeyLen = server_key.length();
  const char *myKey = key.c_str();
  size_t myKeyLen = key.length();
  memcached_return status = memcached_mget_by_key(&m_impl->memcached,
      myServerKey, myServerKeyLen, &myKey, &myKeyLen, 1);
  if (!handleError(status)) return false;

  Variant returnValue;
  MemcachedResultWrapper result(&m_impl->memcached);
  if (!memcached_fetch_result(&m_impl->memcached, &result.value, &status)) {
    if (status == MEMCACHED_END) status = MEMCACHED_NOTFOUND;
    if (status == MEMCACHED_NOTFOUND && !cache_cb.isNull()) {
      status = doCacheCallback(cache_cb, key, returnValue);
      if (!handleError(status)) return false;
      if (cas_token.isReferenced()) cas_token = 0.0;
      return returnValue;
    }
    handleError(status);
    return false;
  }

  if (!toObject(returnValue, result.value)) {
    m_impl->rescode = q_Memcached$$RES_PAYLOAD_FAILURE;
    return false;
  }
  if (cas_token.isReferenced()) {
    cas_token = (double) memcached_result_cas(&result.value);
  }
  return returnValue;
}

Variant c_Memcached::t_getmulti(CArrRef keys,
                                VRefParam cas_tokens /*= null_variant*/,
                                int flags /*= 0*/) {
  return t_getmultibykey(null_string, keys, cas_tokens, flags);
}

Variant c_Memcached::t_getmultibykey(const String& server_key, CArrRef keys,
                                     VRefParam cas_tokens /*= null_variant*/,
                                     int flags /*= 0*/) {
  m_impl->rescode = q_Memcached$$RES_SUCCESS;

  bool preserveOrder = flags & q_Memcached$$GET_PRESERVE_ORDER;
  Array returnValue;
  if (!getMultiImpl(server_key, keys, cas_tokens.isReferenced(),
                    preserveOrder ? &returnValue : NULL)) {
    return false;
  }

  if (cas_tokens.isReferenced()) cas_tokens = Array();
  MemcachedResultWrapper result(&m_impl->memcached);
  memcached_return status;
  while (memcached_fetch_result(&m_impl->memcached, &result.value, &status)) {
    Variant value;
    if (!toObject(value, result.value)) {
      m_impl->rescode = q_Memcached$$RES_PAYLOAD_FAILURE;
      return false;
    }
    const char *key  = memcached_result_key_value(&result.value);
    size_t keyLength = memcached_result_key_length(&result.value);
    String sKey(key, keyLength, CopyString);
    returnValue.set(sKey, value, true);
    if (cas_tokens.isReferenced()) {
      double cas = (double) memcached_result_cas(&result.value);
      cas_tokens->set(sKey, cas, true);
    }
  }

  if (status != MEMCACHED_END && !handleError(status)) return false;
  return returnValue;
}

bool c_Memcached::t_getdelayed(CArrRef keys, bool with_cas /*= false*/,
                               CVarRef value_cb /*= null_variant*/) {
  return t_getdelayedbykey(null_string, keys, with_cas, value_cb);
}

bool c_Memcached::t_getdelayedbykey(const String& server_key, CArrRef keys,
    bool with_cas /*= false*/, CVarRef value_cb /*= null_variant*/) {
  m_impl->rescode = q_Memcached$$RES_SUCCESS;

  if (!getMultiImpl(server_key, keys, with_cas, NULL)) return false;
  if (value_cb.isNull()) return true;

  MemcachedResultWrapper result(&m_impl->memcached); Array item;
  while (fetchImpl(result.value, item)) {
    vm_call_user_func(value_cb, make_packed_array(Variant(this), item));
  }

  if (m_impl->rescode != q_Memcached$$RES_END) return false;
  m_impl->rescode = q_Memcached$$RES_SUCCESS;
  return true;
}

Variant c_Memcached::t_fetch() {
  m_impl->rescode = q_Memcached$$RES_SUCCESS;

  MemcachedResultWrapper result(&m_impl->memcached); Array item;
  if (!fetchImpl(result.value, item)) return false;

  return item;
}

Variant c_Memcached::t_fetchall() {
  m_impl->rescode = q_Memcached$$RES_SUCCESS;

  Array returnValue;
  MemcachedResultWrapper result(&m_impl->memcached); Array item;
  while (fetchImpl(result.value, item)) {
    returnValue.append(item);
  }

  if (m_impl->rescode != q_Memcached$$RES_END) return false;
  return returnValue;
}

bool c_Memcached::getMultiImpl(const String& server_key, CArrRef keys,
                               bool enableCas, Array *returnValue) {
  vector<const char*> keysCopy;
  keysCopy.reserve(keys.size());
  vector<size_t> keysLengthCopy;
  keysLengthCopy.reserve(keys.size());
  for (ArrayIter iter(keys); iter; ++iter) {
    Variant vKey = iter.second();
    if (!vKey.isString()) continue;
    StringData *key = vKey.getStringData();
    if (key->empty()) continue;
    keysCopy.push_back(key->data());
    keysLengthCopy.push_back(key->size());
    if (returnValue) returnValue->set(String(key), null_variant, true);
  }
  if (keysCopy.size() == 0) {
    m_impl->rescode = q_Memcached$$RES_BAD_KEY_PROVIDED;
    return false;
  }

  memcached_behavior_set(&m_impl->memcached, MEMCACHED_BEHAVIOR_SUPPORT_CAS,
                         enableCas ? 1 : 0);
  const char *myServerKey = server_key.empty() ? NULL : server_key.c_str();
  size_t myServerKeyLen = server_key.length();
  return handleError(memcached_mget_by_key(&m_impl->memcached,
      myServerKey, myServerKeyLen, keysCopy.data(), keysLengthCopy.data(),
      keysCopy.size()));
}

const StaticString
  s_key("key"),
  s_value("value"),
  s_cas("cas");

bool c_Memcached::fetchImpl(memcached_result_st &result, Array &item) {
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

bool c_Memcached::t_set(const String& key, CVarRef value, int expiration /*= 0*/) {
  return t_setbykey(null_string, key, value, expiration);
}

bool c_Memcached::t_setbykey(const String& server_key, const String& key, CVarRef value,
                             int expiration /*= 0*/) {
  return setOperationImpl(memcached_set_by_key, server_key, key, value,
                          expiration);
}

bool c_Memcached::t_setmulti(CArrRef items, int expiration /*= 0*/) {
  return t_setmultibykey(null_string, items, expiration);
}

bool c_Memcached::t_setmultibykey(const String& server_key, CArrRef items,
                                  int expiration /*= 0*/) {
  m_impl->rescode = q_Memcached$$RES_SUCCESS;

  for (ArrayIter iter(items); iter; ++iter) {
    Variant key = iter.first();
    if (!key.isString()) continue;
    if (!t_setbykey(server_key, key.toString(), iter.second(), expiration)) {
      return false;
    }
  }
  return true;
}

bool c_Memcached::t_add(const String& key, CVarRef value, int expiration /*= 0*/) {
  return t_addbykey(null_string, key, value, expiration);
}

bool c_Memcached::t_addbykey(const String& server_key, const String& key, CVarRef value,
                             int expiration /*= 0*/) {
  return setOperationImpl(memcached_add_by_key, server_key, key, value,
                          expiration);
}

bool c_Memcached::t_append(const String& key, const String& value) {
  return t_appendbykey(null_string, key, value);
}

bool c_Memcached::t_appendbykey(const String& server_key, const String& key,
                                const String& value) {
  if (m_impl->compression) {
    raise_warning("cannot append/prepend with compression turned on");
    return false;
  }
  return setOperationImpl(memcached_append_by_key, server_key, key, value, 0);
}

bool c_Memcached::t_prepend(const String& key, const String& value) {
  return t_prependbykey(null_string, key, value);
}

bool c_Memcached::t_prependbykey(const String& server_key, const String& key,
                                 const String& value) {
  if (m_impl->compression) {
    raise_warning("cannot append/prepend with compression turned on");
    return false;
  }
  return setOperationImpl(memcached_prepend_by_key, server_key, key, value, 0);
}

bool c_Memcached::t_replace(const String& key, CVarRef value,
                            int expiration /*= 0*/) {
  return t_replacebykey(null_string, key, value, expiration);
}

bool c_Memcached::t_replacebykey(const String& server_key, const String& key,
                                 CVarRef value, int expiration /*= 0*/) {
  return setOperationImpl(memcached_replace_by_key, server_key, key, value,
                          expiration);
}

bool c_Memcached::setOperationImpl(SetOperation op, const String& server_key,
                                   const String& key, CVarRef value,
                                   int expiration) {
  m_impl->rescode = q_Memcached$$RES_SUCCESS;
  if (key.empty()) {
    m_impl->rescode = q_Memcached$$RES_BAD_KEY_PROVIDED;
    return false;
  }

  vector<char> payload; uint32_t flags;
  toPayload(value, payload, flags);

  const String& myServerKey = server_key.empty() ? key : server_key;
  return handleError(op(&m_impl->memcached, myServerKey.c_str(),
                        myServerKey.length(), key.c_str(), key.length(),
                        payload.data(), payload.size(), expiration, flags));
}

bool c_Memcached::t_cas(double cas_token, const String& key, CVarRef value,
                        int expiration /*= 0*/) {
  return t_casbykey(cas_token, null_string, key, value, expiration);
}

bool c_Memcached::t_casbykey(double cas_token, const String& server_key, const String& key,
                             CVarRef value, int expiration /*= 0*/) {
  m_impl->rescode = q_Memcached$$RES_SUCCESS;
  if (key.empty()) {
    m_impl->rescode = q_Memcached$$RES_BAD_KEY_PROVIDED;
    return false;
  }

  vector<char> payload; uint32_t flags;
  toPayload(value, payload, flags);

  const String& myServerKey = server_key.empty() ? key : server_key;
  return handleError(memcached_cas_by_key(&m_impl->memcached,
      myServerKey.c_str(), myServerKey.length(), key.c_str(), key.length(),
      payload.data(), payload.size(), expiration, flags, (uint64_t)cas_token));
}

bool c_Memcached::t_delete(const String& key, int time /*= 0*/) {
  return t_deletebykey(null_string, key, time);
}

bool c_Memcached::t_deletebykey(const String& server_key, const String& key,
                                int time /*= 0*/) {
  m_impl->rescode = q_Memcached$$RES_SUCCESS;
  if (key.empty()) {
    m_impl->rescode = q_Memcached$$RES_BAD_KEY_PROVIDED;
    return false;
  }

  const String& myServerKey = server_key.empty() ? key : server_key;
  return handleError(memcached_delete_by_key(&m_impl->memcached,
                     myServerKey.c_str(), myServerKey.length(),
                     key.c_str(), key.length(), time));
}

Variant c_Memcached::t_increment(const String& key, int64_t offset /*= 1*/) {
  return incDecOperationImpl(memcached_increment, key, offset);
}

Variant c_Memcached::t_decrement(const String& key, int64_t offset /*= 1*/) {
  return incDecOperationImpl(memcached_decrement, key, offset);
}

Variant c_Memcached::incDecOperationImpl(IncDecOperation op, const String& key,
                                         int64_t offset) {
  m_impl->rescode = q_Memcached$$RES_SUCCESS;
  if (key.empty()) {
    m_impl->rescode = q_Memcached$$RES_BAD_KEY_PROVIDED;
    return false;
  }
  if (offset < 0) {
    raise_warning("offset has to be >= 0");
    return false;
  }

  uint64_t value;
  if (!handleError(op(&m_impl->memcached, key.c_str(), key.length(),
                      (uint32_t)offset, &value))) {
    return false;
  }
  return (int64_t)value;
}

bool c_Memcached::t_addserver(const String& host, int port, int weight /*= 0*/) {
  m_impl->rescode = q_Memcached$$RES_SUCCESS;
  return handleError(memcached_server_add_with_weight(&m_impl->memcached,
      host.c_str(), port, weight));
}

bool c_Memcached::t_addservers(CArrRef servers) {
  int i = 1;
  for (ArrayIter iter(servers); iter; ++iter, ++i) {
    Variant entry = iter.second();
    if (!entry.isArray()) {
      raise_warning("server list entry #%d is not an array", i);
      continue;
    }

    ArrayIter entryIter(entry.getArrayData());
    if (!entryIter) {
      raise_warning("could not get server host for entry #%d", i);
      continue;
    }
    String host = entryIter.second().toString();

    ++entryIter;
    if (!entryIter) {
      raise_warning("could not get server port for entry #%d", i);
      continue;
    }
    int port = entryIter.second().toInt32(10);

    int weight = 0;
    ++entryIter;
    if (entryIter) {
      weight = entryIter.second().toInt32(10);
    }

    if (!handleError(memcached_server_add_with_weight(&m_impl->memcached,
          host.c_str(), port, weight))) {
      raise_warning("could not add entry #%d to the server list", i);
    }
  }
  return true;
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

Array c_Memcached::t_getserverlist() {
  Array returnValue;
  memcached_server_function callbacks[] = { doServerListCallback };
  memcached_server_cursor(&m_impl->memcached, callbacks, &returnValue, 1);
  return returnValue;
}

Variant c_Memcached::t_getserverbykey(const String& server_key) {
  m_impl->rescode = q_Memcached$$RES_SUCCESS;
  if (server_key.empty()) {
    m_impl->rescode = q_Memcached$$RES_BAD_KEY_PROVIDED;
    return false;
  }

  memcached_return_t error;
  LMCD_SERVER_BY_KEY_INSTANCE_TYPE server = memcached_server_by_key(
    &m_impl->memcached, server_key.c_str(), server_key.size(), &error);
  if (!server) {
    handleError(error);
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

  context->returnValue.set(String(key, CopyString), Array(ArrayInit(24)
      .set(s_pid,                        (int64_t)stats[i].pid)
      .set(s_uptime,                     (int64_t)stats[i].uptime)
      .set(s_threads,                    (int64_t)stats[i].threads)
      .set(s_time,                       (int64_t)stats[i].time)
      .set(s_pointer_size,               (int64_t)stats[i].pointer_size)
      .set(s_rusage_user_seconds,        (int64_t)stats[i].rusage_user_seconds)
      .set(s_rusage_user_microseconds,   (int64_t)stats[i]
                                                .rusage_user_microseconds)
      .set(s_rusage_system_seconds,      (int64_t)stats[i].rusage_system_seconds)
      .set(s_rusage_system_microseconds, (int64_t)stats[i]
                                                .rusage_system_microseconds)
      .set(s_curr_items,                 (int64_t)stats[i].curr_items)
      .set(s_total_items,                (int64_t)stats[i].total_items)
      .set(s_limit_maxbytes,             (int64_t)stats[i].limit_maxbytes)
      .set(s_curr_connections,           (int64_t)stats[i].curr_connections)
      .set(s_total_connections,          (int64_t)stats[i].total_connections)
      .set(s_connection_structures,      (int64_t)stats[i].connection_structures)
      .set(s_bytes,                      (int64_t)stats[i].bytes)
      .set(s_cmd_get,                    (int64_t)stats[i].cmd_get)
      .set(s_cmd_set,                    (int64_t)stats[i].cmd_set)
      .set(s_get_hits,                   (int64_t)stats[i].get_hits)
      .set(s_get_misses,                 (int64_t)stats[i].get_misses)
      .set(s_evictions,                  (int64_t)stats[i].evictions)
      .set(s_bytes_read,                 (int64_t)stats[i].bytes_read)
      .set(s_bytes_written,              (int64_t)stats[i].bytes_written)
      .set(s_version,                    String(stats[i].version, CopyString))
      .create()));

  return MEMCACHED_SUCCESS;
}
}

Variant c_Memcached::t_getstats() {
  memcached_return_t error;
  memcached_stat_st *stats = memcached_stat(&m_impl->memcached, NULL, &error);
  if (!stats) {
    handleError(error);
    return false;
  }

  memcached_server_function callbacks[] = { doStatsCallback };
  StatsContext context; context.stats = stats;
  memcached_server_cursor(&m_impl->memcached, callbacks, &context, 1);

  memcached_stat_free(&m_impl->memcached, stats);
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

Variant c_Memcached::t_getversion() {
  memcached_return_t status = memcached_version(&m_impl->memcached);
  if (!handleError(status)) return false;

  Array returnValue;
  memcached_server_function callbacks[] = { doVersionCallback };
  memcached_server_cursor(&m_impl->memcached, callbacks, &returnValue, 1);
  return returnValue;
}

bool c_Memcached::t_flush(int delay /*= 0*/) {
  return handleError(memcached_flush(&m_impl->memcached, delay));
}

Variant c_Memcached::t_getoption(int option) {
  switch (option) {
  case q_Memcached$$OPT_COMPRESSION:
    return m_impl->compression;

  case q_Memcached$$OPT_PREFIX_KEY:
    {
      memcached_return retval;
      char *result = (char*) memcached_callback_get(&m_impl->memcached,
          MEMCACHED_CALLBACK_PREFIX_KEY, &retval);
      if (retval == MEMCACHED_SUCCESS) return String(result, CopyString);
      else return empty_string;
    }

  case q_Memcached$$OPT_SERIALIZER:
    return m_impl->serializer;

  case MEMCACHED_BEHAVIOR_SOCKET_SEND_SIZE:
  case MEMCACHED_BEHAVIOR_SOCKET_RECV_SIZE:
    if (memcached_server_count(&m_impl->memcached) == 0) {
      raise_warning("no servers defined");
      return null_variant;
    }
    // fall through

  default:
    // Assume that it's a libmemcached behavior option
    return (int64_t) memcached_behavior_get(&m_impl->memcached,
                                          (memcached_behavior_t)option);
  }
}

bool c_Memcached::t_setoption(int option, CVarRef value) {
  switch (option) {
  case q_Memcached$$OPT_COMPRESSION:
    m_impl->compression = value.toBoolean();
    break;

  case q_Memcached$$OPT_PREFIX_KEY:
    {
      String sValue = value.toString();
      char *key = const_cast<char*>(sValue.empty() ? NULL : sValue.c_str());
      if (memcached_callback_set(&m_impl->memcached,
          MEMCACHED_CALLBACK_PREFIX_KEY, key) == MEMCACHED_BAD_KEY_PROVIDED) {
        raise_warning("bad key provided");
        return false;
      }
      break;
    }

  case MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED:
    {
      uint64_t lValue = value.toInt64();
      if (memcached_behavior_set(&m_impl->memcached,
          MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED, lValue) == MEMCACHED_FAILURE) {
        raise_warning("error setting memcached option");
        return false;
      }

      /* This is necessary because libmemcached doesn't reset hash/distribution
       * options on false case, like it does for MEMCACHED_BEHAVIOR_KETAMA
       * (non-weighted) case. We have to clean up ourselves.
       */
      memcached_behavior_set_key_hash(&m_impl->memcached,
                                      MEMCACHED_HASH_DEFAULT);
      memcached_behavior_set_distribution_hash(&m_impl->memcached,
                                               MEMCACHED_HASH_DEFAULT);
      memcached_behavior_set_distribution(&m_impl->memcached,
                                          MEMCACHED_DISTRIBUTION_MODULA);
      break;
    }

  case q_Memcached$$OPT_SERIALIZER:
    {
      int iValue = value.toInt32(10);
      switch (iValue) {
      case q_Memcached$$SERIALIZER_PHP:
      case q_Memcached$$SERIALIZER_JSON:
        m_impl->serializer = iValue;
        break;
      default:
        m_impl->serializer = q_Memcached$$SERIALIZER_PHP;
        raise_warning("invalid serializer provided");
        return false;
      }
      break;
    }

  default:
    {
      // Assume that it's a libmemcached behavior option
      uint64_t lValue = value.toInt64();
      if (memcached_behavior_set(&m_impl->memcached,
          (memcached_behavior_t)option, lValue) == MEMCACHED_FAILURE) {
        raise_warning("error setting memcached option");
        return false;
      }
      break;
    }
  }
  return true;
}

int64_t c_Memcached::t_getresultcode() {
  return m_impl->rescode;
}

String c_Memcached::t_getresultmessage() {
  if (m_impl->rescode == q_Memcached$$RES_PAYLOAD_FAILURE) {
    return "PAYLOAD FAILURE";
  } else {
    return memcached_strerror(&m_impl->memcached,
                              (memcached_return_t)m_impl->rescode);
  }
}

bool c_Memcached::handleError(memcached_return status) {
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

void c_Memcached::toPayload(CVarRef value, vector<char> &payload,
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
      encoded = f_json_encode(value);
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

bool c_Memcached::toObject(Variant& value, const memcached_result_st &result) {
  const char *payload  = memcached_result_value(&result);
  size_t payloadLength = memcached_result_length(&result);
  uint32_t flags         = memcached_result_flags(&result);

  String decompPayload;
  if (flags & MEMC_VAL_COMPRESSED) {
    bool done = false;
    vector<char> buffer;
    unsigned long bufferSize;
    for (int factor = 1; !done && factor <= 16; ++factor) {
      bufferSize = payloadLength * (1 << factor) + 1;
      buffer.resize(bufferSize);
      if (uncompress((Bytef*)buffer.data(), &bufferSize,
                     (const Bytef*)payload, (uLong)payloadLength) == Z_OK) {
        done = true;
      }
    }
    if (!done) {
      raise_warning("could not uncompress value");
      return false;
    }
    decompPayload = StringData::Make(buffer.data(), bufferSize, CopyString);
  } else {
    decompPayload = StringData::Make(payload, payloadLength, CopyString);
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
    value = f_json_decode(decompPayload);
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

memcached_return c_Memcached::doCacheCallback(CVarRef callback, const String& key,
                                              Variant& value) {
  Array params(ArrayInit(3).set(Variant(this))
                           .set(key)
                           .setRef(value).create());
  if (!vm_call_user_func(callback, params).toBoolean()) {
    return MEMCACHED_NOTFOUND;
  }

  vector<char> payload; uint32_t flags;
  toPayload(value, payload, flags);
  return memcached_set(&m_impl->memcached, key.c_str(), key.length(),
                       payload.data(), payload.size(), 0, flags);
}

///////////////////////////////////////////////////////////////////////////////
}
