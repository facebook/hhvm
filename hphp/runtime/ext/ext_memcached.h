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

#ifndef incl_HPHP_EXT_MEMCACHED_H_
#define incl_HPHP_EXT_MEMCACHED_H_

#include "hphp/runtime/base/base-includes.h"
#include <libmemcached/memcached.h>
#include <map>
#include <memory>
#include <vector>
namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const int64_t q_Memcached$$OPT_COMPRESSION;
extern const int64_t q_Memcached$$OPT_SERIALIZER;
extern const int64_t q_Memcached$$SERIALIZER_PHP;
extern const int64_t q_Memcached$$SERIALIZER_IGBINARY;
extern const int64_t q_Memcached$$SERIALIZER_JSON;
extern const int64_t q_Memcached$$OPT_PREFIX_KEY;
extern const int64_t q_Memcached$$OPT_HASH;
extern const int64_t q_Memcached$$HASH_DEFAULT;
extern const int64_t q_Memcached$$HASH_MD5;
extern const int64_t q_Memcached$$HASH_CRC;
extern const int64_t q_Memcached$$HASH_FNV1_64;
extern const int64_t q_Memcached$$HASH_FNV1A_64;
extern const int64_t q_Memcached$$HASH_FNV1_32;
extern const int64_t q_Memcached$$HASH_FNV1A_32;
extern const int64_t q_Memcached$$HASH_HSIEH;
extern const int64_t q_Memcached$$HASH_MURMUR;
extern const int64_t q_Memcached$$OPT_DISTRIBUTION;
extern const int64_t q_Memcached$$DISTRIBUTION_MODULA;
extern const int64_t q_Memcached$$DISTRIBUTION_CONSISTENT;
extern const int64_t q_Memcached$$OPT_LIBKETAMA_COMPATIBLE;
extern const int64_t q_Memcached$$OPT_BUFFER_WRITES;
extern const int64_t q_Memcached$$OPT_BINARY_PROTOCOL;
extern const int64_t q_Memcached$$OPT_NO_BLOCK;
extern const int64_t q_Memcached$$OPT_TCP_NODELAY;
extern const int64_t q_Memcached$$OPT_SOCKET_SEND_SIZE;
extern const int64_t q_Memcached$$OPT_SOCKET_RECV_SIZE;
extern const int64_t q_Memcached$$OPT_CONNECT_TIMEOUT;
extern const int64_t q_Memcached$$OPT_RETRY_TIMEOUT;
extern const int64_t q_Memcached$$OPT_SEND_TIMEOUT;
extern const int64_t q_Memcached$$OPT_RECV_TIMEOUT;
extern const int64_t q_Memcached$$OPT_POLL_TIMEOUT;
extern const int64_t q_Memcached$$OPT_CACHE_LOOKUPS;
extern const int64_t q_Memcached$$OPT_SERVER_FAILURE_LIMIT;
extern const bool q_Memcached$$HAVE_IGBINARY;
extern const bool q_Memcached$$HAVE_JSON;
extern const int64_t q_Memcached$$GET_PRESERVE_ORDER;
extern const int64_t q_Memcached$$RES_SUCCESS;
extern const int64_t q_Memcached$$RES_FAILURE;
extern const int64_t q_Memcached$$RES_HOST_LOOKUP_FAILURE;
extern const int64_t q_Memcached$$RES_UNKNOWN_READ_FAILURE;
extern const int64_t q_Memcached$$RES_PROTOCOL_ERROR;
extern const int64_t q_Memcached$$RES_CLIENT_ERROR;
extern const int64_t q_Memcached$$RES_SERVER_ERROR;
extern const int64_t q_Memcached$$RES_WRITE_FAILURE;
extern const int64_t q_Memcached$$RES_DATA_EXISTS;
extern const int64_t q_Memcached$$RES_NOTSTORED;
extern const int64_t q_Memcached$$RES_NOTFOUND;
extern const int64_t q_Memcached$$RES_PARTIAL_READ;
extern const int64_t q_Memcached$$RES_SOME_ERRORS;
extern const int64_t q_Memcached$$RES_NO_SERVERS;
extern const int64_t q_Memcached$$RES_END;
extern const int64_t q_Memcached$$RES_ERRNO;
extern const int64_t q_Memcached$$RES_BUFFERED;
extern const int64_t q_Memcached$$RES_TIMEOUT;
extern const int64_t q_Memcached$$RES_BAD_KEY_PROVIDED;
extern const int64_t q_Memcached$$RES_CONNECTION_SOCKET_CREATE_FAILURE;
extern const int64_t q_Memcached$$RES_PAYLOAD_FAILURE;
extern const int64_t q_Memcached$$RES_NOT_SUPPORTED;
extern const int64_t q_Memcached$$RES_INVALID_HOST_PROTOCOL;

///////////////////////////////////////////////////////////////////////////////
// class Memcached

FORWARD_DECLARE_CLASS(Memcached);
class c_Memcached : public ExtObjectData, public Sweepable {
 public:
  DECLARE_CLASS(Memcached)

  // need to implement
  public: c_Memcached(Class* cls = c_Memcached::classof());
  public: ~c_Memcached();
  public: void t___construct(const String& persistent_id = null_string);
  public: bool t_add(const String& key, const Variant& value, int expiration = 0);
  public: bool t_addbykey(const String& server_key, const String& key, const Variant& value, int expiration = 0);
  public: bool t_addserver(const String& host, int port, int weight = 0);
  public: bool t_addservers(const Array& servers);
  public: bool t_append(const String& key, const String& value);
  public: bool t_appendbykey(const String& server_key, const String& key, const String& value);
  public: bool t_cas(double cas_token, const String& key, const Variant& value, int expiration = 0);
  public: bool t_casbykey(double cas_token, const String& server_key, const String& key, const Variant& value, int expiration = 0);
  public: Variant t_decrement(const String& key, int64_t offset = 1);
  public: bool t_delete(const String& key, int time = 0);
  public: bool t_deletebykey(const String& server_key, const String& key, int time = 0);
  public: Variant t_fetch();
  public: Variant t_fetchall();
  public: bool t_flush(int delay = 0);
  public: Variant t_get(const String& key, const Variant& cache_cb = null_variant, VRefParam cas_token = null_variant);
  public: Variant t_getbykey(const String& server_key, const String& key, const Variant& cache_cb = null_variant, VRefParam cas_token = null_variant);
  public: bool t_getdelayed(const Array& keys, bool with_cas = false, const Variant& value_cb = null_variant);
  public: bool t_getdelayedbykey(const String& server_key, const Array& keys, bool with_cas = false, const Variant& value_cb = null_variant);
  public: Variant t_getmulti(const Array& keys, VRefParam cas_tokens = null_variant, int flags = 0);
  public: Variant t_getmultibykey(const String& server_key, const Array& keys, VRefParam cas_tokens = null_variant, int flags = 0);
  public: Variant t_getoption(int option);
  public: int64_t t_getresultcode();
  public: String t_getresultmessage();
  public: Variant t_getserverbykey(const String& server_key);
  public: Array t_getserverlist();
  public: Variant t_getstats();
  public: Variant t_getversion();
  public: Variant t_increment(const String& key, int64_t offset = 1);
  public: bool t_prepend(const String& key, const String& value);
  public: bool t_prependbykey(const String& server_key, const String& key, const String& value);
  public: bool t_replace(const String& key, const Variant& value, int expiration = 0);
  public: bool t_replacebykey(const String& server_key, const String& key, const Variant& value, int expiration = 0);
  public: bool t_set(const String& key, const Variant& value, int expiration = 0);
  public: bool t_setbykey(const String& server_key, const String& key, const Variant& value, int expiration = 0);
  public: bool t_setmulti(const Array& items, int expiration = 0);
  public: bool t_setmultibykey(const String& server_key, const Array& items, int expiration = 0);
  public: bool t_setoption(int option, const Variant& value);
 private:
  class Impl {
  public:
    Impl();
    ~Impl();

    memcached_st memcached;
    bool compression;
    int serializer;
    int rescode;
  };
  typedef std::shared_ptr<Impl> ImplPtr;
  ImplPtr m_impl;

  bool handleError(memcached_return status);
  void toPayload(const Variant& value, std::vector<char> &payload, uint32_t &flags);
  bool toObject(Variant& value, const memcached_result_st &result);
  memcached_return doCacheCallback(const Variant& callback, const String& key,
                                   Variant& value);
  bool getMultiImpl(const String& server_key, const Array& keys, bool enableCas,
                    Array *returnValue);
  bool fetchImpl(memcached_result_st &result, Array &item);
  typedef memcached_return_t (*SetOperation)(memcached_st *,
      const char *, size_t, const char *, size_t, const char *, size_t,
      time_t, uint32_t);
  bool setOperationImpl(SetOperation op, const String& server_key,
                        const String& key, const Variant& value, int expiration);
  typedef memcached_return_t (*IncDecOperation)(memcached_st *,
      const char *, size_t, uint32_t, uint64_t *);
  Variant incDecOperationImpl(IncDecOperation op, const String& key, int64_t offset);

  typedef std::map<std::string, ImplPtr> ImplMap;
  static DECLARE_THREAD_LOCAL(ImplMap, s_persistentMap);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_MEMCACHED_H_
