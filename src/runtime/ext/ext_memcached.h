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

extern const int q_Memcached_OPT_COMPRESSION;
extern const int q_Memcached_OPT_SERIALIZER;
extern const int q_Memcached_SERIALIZER_PHP;
extern const int q_Memcached_SERIALIZER_IGBINARY;
extern const int q_Memcached_SERIALIZER_JSON;
extern const int q_Memcached_OPT_PREFIX_KEY;
extern const int q_Memcached_OPT_HASH;
extern const int q_Memcached_HASH_DEFAULT;
extern const int q_Memcached_HASH_MD5;
extern const int q_Memcached_HASH_CRC;
extern const int q_Memcached_HASH_FNV1_64;
extern const int q_Memcached_HASH_FNV1A_64;
extern const int q_Memcached_HASH_FNV1_32;
extern const int q_Memcached_HASH_FNV1A_32;
extern const int q_Memcached_HASH_HSIEH;
extern const int q_Memcached_HASH_MURMUR;
extern const int q_Memcached_OPT_DISTRIBUTION;
extern const int q_Memcached_DISTRIBUTION_MODULA;
extern const int q_Memcached_DISTRIBUTION_CONSISTENT;
extern const int q_Memcached_OPT_LIBKETAMA_COMPATIBLE;
extern const int q_Memcached_OPT_BUFFER_WRITES;
extern const int q_Memcached_OPT_BINARY_PROTOCOL;
extern const int q_Memcached_OPT_NO_BLOCK;
extern const int q_Memcached_OPT_TCP_NODELAY;
extern const int q_Memcached_OPT_SOCKET_SEND_SIZE;
extern const int q_Memcached_OPT_SOCKET_RECV_SIZE;
extern const int q_Memcached_OPT_CONNECT_TIMEOUT;
extern const int q_Memcached_OPT_RETRY_TIMEOUT;
extern const int q_Memcached_OPT_SEND_TIMEOUT;
extern const int q_Memcached_OPT_RECV_TIMEOUT;
extern const int q_Memcached_OPT_POLL_TIMEOUT;
extern const int q_Memcached_OPT_CACHE_LOOKUPS;
extern const int q_Memcached_OPT_SERVER_FAILURE_LIMIT;
extern const bool q_Memcached_HAVE_IGBINARY;
extern const bool q_Memcached_HAVE_JSON;
extern const int q_Memcached_GET_PRESERVE_ORDER;
extern const int q_Memcached_RES_SUCCESS;
extern const int q_Memcached_RES_FAILURE;
extern const int q_Memcached_RES_HOST_LOOKUP_FAILURE;
extern const int q_Memcached_RES_UNKNOWN_READ_FAILURE;
extern const int q_Memcached_RES_PROTOCOL_ERROR;
extern const int q_Memcached_RES_CLIENT_ERROR;
extern const int q_Memcached_RES_SERVER_ERROR;
extern const int q_Memcached_RES_WRITE_FAILURE;
extern const int q_Memcached_RES_DATA_EXISTS;
extern const int q_Memcached_RES_NOTSTORED;
extern const int q_Memcached_RES_NOTFOUND;
extern const int q_Memcached_RES_PARTIAL_READ;
extern const int q_Memcached_RES_SOME_ERRORS;
extern const int q_Memcached_RES_NO_SERVERS;
extern const int q_Memcached_RES_END;
extern const int q_Memcached_RES_ERRNO;
extern const int q_Memcached_RES_BUFFERED;
extern const int q_Memcached_RES_TIMEOUT;
extern const int q_Memcached_RES_BAD_KEY_PROVIDED;
extern const int q_Memcached_RES_CONNECTION_SOCKET_CREATE_FAILURE;
extern const int q_Memcached_RES_PAYLOAD_FAILURE;

///////////////////////////////////////////////////////////////////////////////
// class Memcached

FORWARD_DECLARE_CLASS(Memcached);
class c_Memcached : public ExtObjectData, public Sweepable {
 public:
  BEGIN_CLASS_MAP(Memcached)
  END_CLASS_MAP(Memcached)
  DECLARE_CLASS(Memcached, Memcached, ObjectData)

  // need to implement
  public: c_Memcached();
  public: ~c_Memcached();
  public: void t___construct(CStrRef persistent_id = null_string);
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  public: bool t_add(CStrRef key, CVarRef value, int expiration = 0);
  DECLARE_METHOD_INVOKE_HELPERS(add);
  public: bool t_addbykey(CStrRef server_key, CStrRef key, CVarRef value, int expiration = 0);
  DECLARE_METHOD_INVOKE_HELPERS(addbykey);
  public: bool t_addserver(CStrRef host, int port, int weight = 0);
  DECLARE_METHOD_INVOKE_HELPERS(addserver);
  public: bool t_addservers(CArrRef servers);
  DECLARE_METHOD_INVOKE_HELPERS(addservers);
  public: bool t_append(CStrRef key, CStrRef value);
  DECLARE_METHOD_INVOKE_HELPERS(append);
  public: bool t_appendbykey(CStrRef server_key, CStrRef key, CStrRef value);
  DECLARE_METHOD_INVOKE_HELPERS(appendbykey);
  public: bool t_cas(double cas_token, CStrRef key, CVarRef value, int expiration = 0);
  DECLARE_METHOD_INVOKE_HELPERS(cas);
  public: bool t_casbykey(double cas_token, CStrRef server_key, CStrRef key, CVarRef value, int expiration = 0);
  DECLARE_METHOD_INVOKE_HELPERS(casbykey);
  public: Variant t_decrement(CStrRef key, int64 offset = 1);
  DECLARE_METHOD_INVOKE_HELPERS(decrement);
  public: bool t_delete(CStrRef key, int time = 0);
  DECLARE_METHOD_INVOKE_HELPERS(delete);
  public: bool t_deletebykey(CStrRef server_key, CStrRef key, int time = 0);
  DECLARE_METHOD_INVOKE_HELPERS(deletebykey);
  public: Variant t_fetch();
  DECLARE_METHOD_INVOKE_HELPERS(fetch);
  public: Variant t_fetchall();
  DECLARE_METHOD_INVOKE_HELPERS(fetchall);
  public: bool t_flush(int delay = 0);
  DECLARE_METHOD_INVOKE_HELPERS(flush);
  public: Variant t_get(CStrRef key, CVarRef cache_cb = null_variant, VRefParam cas_token = null_variant);
  DECLARE_METHOD_INVOKE_HELPERS(get);
  public: Variant t_getbykey(CStrRef server_key, CStrRef key, CVarRef cache_cb = null_variant, VRefParam cas_token = null_variant);
  DECLARE_METHOD_INVOKE_HELPERS(getbykey);
  public: bool t_getdelayed(CArrRef keys, bool with_cas = false, CVarRef value_cb = null_variant);
  DECLARE_METHOD_INVOKE_HELPERS(getdelayed);
  public: bool t_getdelayedbykey(CStrRef server_key, CArrRef keys, bool with_cas = false, CVarRef value_cb = null_variant);
  DECLARE_METHOD_INVOKE_HELPERS(getdelayedbykey);
  public: Variant t_getmulti(CArrRef keys, VRefParam cas_tokens = null_variant, int flags = 0);
  DECLARE_METHOD_INVOKE_HELPERS(getmulti);
  public: Variant t_getmultibykey(CStrRef server_key, CArrRef keys, VRefParam cas_tokens = null_variant, int flags = 0);
  DECLARE_METHOD_INVOKE_HELPERS(getmultibykey);
  public: Variant t_getoption(int option);
  DECLARE_METHOD_INVOKE_HELPERS(getoption);
  public: int t_getresultcode();
  DECLARE_METHOD_INVOKE_HELPERS(getresultcode);
  public: String t_getresultmessage();
  DECLARE_METHOD_INVOKE_HELPERS(getresultmessage);
  public: Variant t_getserverbykey(CStrRef server_key);
  DECLARE_METHOD_INVOKE_HELPERS(getserverbykey);
  public: Array t_getserverlist();
  DECLARE_METHOD_INVOKE_HELPERS(getserverlist);
  public: Variant t_getstats();
  DECLARE_METHOD_INVOKE_HELPERS(getstats);
  public: Variant t_getversion();
  DECLARE_METHOD_INVOKE_HELPERS(getversion);
  public: Variant t_increment(CStrRef key, int64 offset = 1);
  DECLARE_METHOD_INVOKE_HELPERS(increment);
  public: bool t_prepend(CStrRef key, CStrRef value);
  DECLARE_METHOD_INVOKE_HELPERS(prepend);
  public: bool t_prependbykey(CStrRef server_key, CStrRef key, CStrRef value);
  DECLARE_METHOD_INVOKE_HELPERS(prependbykey);
  public: bool t_replace(CStrRef key, CVarRef value, int expiration = 0);
  DECLARE_METHOD_INVOKE_HELPERS(replace);
  public: bool t_replacebykey(CStrRef server_key, CStrRef key, CVarRef value, int expiration = 0);
  DECLARE_METHOD_INVOKE_HELPERS(replacebykey);
  public: bool t_set(CStrRef key, CVarRef value, int expiration = 0);
  DECLARE_METHOD_INVOKE_HELPERS(set);
  public: bool t_setbykey(CStrRef server_key, CStrRef key, CVarRef value, int expiration = 0);
  DECLARE_METHOD_INVOKE_HELPERS(setbykey);
  public: bool t_setmulti(CArrRef items, int expiration = 0);
  DECLARE_METHOD_INVOKE_HELPERS(setmulti);
  public: bool t_setmultibykey(CStrRef server_key, CArrRef items, int expiration = 0);
  DECLARE_METHOD_INVOKE_HELPERS(setmultibykey);
  public: bool t_setoption(int option, CVarRef value);
  DECLARE_METHOD_INVOKE_HELPERS(setoption);
  public: Variant t___destruct();
  DECLARE_METHOD_INVOKE_HELPERS(__destruct);

  // implemented by HPHP
  public: c_Memcached *create(String persistent_id = null_string);
  public: void dynConstruct(CArrRef Params);
  public: void getConstructor(MethodCallPackage &mcp);
  public: virtual void destruct();

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
  void toPayload(CVarRef value, std::vector<char> &payload, uint32 &flags);
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
  Variant incDecOperationImpl(IncDecOperation op, CStrRef key, int64 offset);

  typedef std::map<std::string, ImplPtr> ImplMap;
  static DECLARE_THREAD_LOCAL(ImplMap, s_persistentMap);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_MEMCACHED_H__
