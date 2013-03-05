/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Hyves (http://www.hyves.nl)                       |
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

#ifndef __EXT_MEMCACHED_H__
#define __EXT_MEMCACHED_H__

#include <runtime/base/base_includes.h>
#include <libmemcached/memcached.h>
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

///////////////////////////////////////////////////////////////////////////////
// class Memcached

FORWARD_DECLARE_CLASS_BUILTIN(Memcached);
class c_Memcached : public ExtObjectData, public Sweepable {
 public:
  DECLARE_CLASS(Memcached, Memcached, ObjectData)

  // need to implement
  public: c_Memcached(VM::Class* cls = c_Memcached::s_cls);
  public: ~c_Memcached();
  public: void t___construct(CStrRef persistent_id = null_string);
  public: bool t_add(CStrRef key, CVarRef value, int expiration = 0);
  public: bool t_addbykey(CStrRef server_key, CStrRef key, CVarRef value, int expiration = 0);
  public: bool t_addserver(CStrRef host, int port, int weight = 0);
  public: bool t_addservers(CArrRef servers);
  public: bool t_append(CStrRef key, CStrRef value);
  public: bool t_appendbykey(CStrRef server_key, CStrRef key, CStrRef value);
  public: bool t_cas(double cas_token, CStrRef key, CVarRef value, int expiration = 0);
  public: bool t_casbykey(double cas_token, CStrRef server_key, CStrRef key, CVarRef value, int expiration = 0);
  public: Variant t_decrement(CStrRef key, int64_t offset = 1);
  public: bool t_delete(CStrRef key, int time = 0);
  public: bool t_deletebykey(CStrRef server_key, CStrRef key, int time = 0);
  public: Variant t_fetch();
  public: Variant t_fetchall();
  public: bool t_flush(int delay = 0);
  public: Variant t_get(CStrRef key, CVarRef cache_cb = null_variant, VRefParam cas_token = null_variant);
  public: Variant t_getbykey(CStrRef server_key, CStrRef key, CVarRef cache_cb = null_variant, VRefParam cas_token = null_variant);
  public: bool t_getdelayed(CArrRef keys, bool with_cas = false, CVarRef value_cb = null_variant);
  public: bool t_getdelayedbykey(CStrRef server_key, CArrRef keys, bool with_cas = false, CVarRef value_cb = null_variant);
  public: Variant t_getmulti(CArrRef keys, VRefParam cas_tokens = null_variant, int flags = 0);
  public: Variant t_getmultibykey(CStrRef server_key, CArrRef keys, VRefParam cas_tokens = null_variant, int flags = 0);
  public: Variant t_getoption(int option);
  public: int64_t t_getresultcode();
  public: String t_getresultmessage();
  public: Variant t_getserverbykey(CStrRef server_key);
  public: Array t_getserverlist();
  public: Variant t_getstats();
  public: Variant t_getversion();
  public: Variant t_increment(CStrRef key, int64_t offset = 1);
  public: bool t_prepend(CStrRef key, CStrRef value);
  public: bool t_prependbykey(CStrRef server_key, CStrRef key, CStrRef value);
  public: bool t_replace(CStrRef key, CVarRef value, int expiration = 0);
  public: bool t_replacebykey(CStrRef server_key, CStrRef key, CVarRef value, int expiration = 0);
  public: bool t_set(CStrRef key, CVarRef value, int expiration = 0);
  public: bool t_setbykey(CStrRef server_key, CStrRef key, CVarRef value, int expiration = 0);
  public: bool t_setmulti(CArrRef items, int expiration = 0);
  public: bool t_setmultibykey(CStrRef server_key, CArrRef items, int expiration = 0);
  public: bool t_setoption(int option, CVarRef value);
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
  typedef boost::shared_ptr<Impl> ImplPtr;
  ImplPtr m_impl;

  bool handleError(memcached_return status);
  void toPayload(CVarRef value, std::vector<char> &payload, uint32_t &flags);
  bool toObject(Variant& value, const memcached_result_st &result);
  memcached_return doCacheCallback(CVarRef callback, CStrRef key,
                                   Variant& value);
  bool getMultiImpl(CStrRef server_key, CArrRef keys, bool enableCas,
                    Array *returnValue);
  bool fetchImpl(memcached_result_st &result, Array &item);
  typedef memcached_return_t (*SetOperation)(memcached_st *,
      const char *, size_t, const char *, size_t, const char *, size_t,
      time_t, uint32_t);
  bool setOperationImpl(SetOperation op, CStrRef server_key,
                        CStrRef key, CVarRef value, int expiration);
  typedef memcached_return_t (*IncDecOperation)(memcached_st *,
      const char *, size_t, uint32_t, uint64_t *);
  Variant incDecOperationImpl(IncDecOperation op, CStrRef key, int64_t offset);

  typedef std::map<std::string, ImplPtr> ImplMap;
  static DECLARE_THREAD_LOCAL(ImplMap, s_persistentMap);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_MEMCACHED_H__
