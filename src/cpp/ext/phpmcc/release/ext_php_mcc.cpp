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

#include <netinet/in.h>
#include <cpp/ext/ext_function.h>
#include <cpp/ext/ext_error.h>
#include <cpp/ext/ext_variable.h>
#include <cpp/ext/ext_facebook.h>
#include <zlib.h>
#include <cpp/base/runtime_option.h>
#include <cpp/base/util/request_local.h>
#include <cpp/base/variable_unserializer.h>
#include <cpp/base/server/server_stats.h>
#include <sys/time.h>
#include <time.h>

#include "ext_php_mcc.h"
#include "ext_php_mcc_impl.h"

using namespace HPHP;
using namespace boost;
using namespace std;

#define PHPMCC_LOG_MAX 1024

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

class MccExtension : public RequestEventHandler {
public:
  void registerPersistentObject(const std::string &name) {
    m_persistentObjects.insert(name);
  }

  virtual void requestInit() {}
  virtual void requestShutdown() {
    // listeners have "context" that are request specific
    for (set<string>::const_iterator iter = m_persistentObjects.begin();
         iter != m_persistentObjects.end(); ++iter) {
      ResourceData *res = g_persistentObjects->get("php_mcc::resources",
                                                   iter->c_str());
      if (res) {
        MccResource *mcc = dynamic_cast<MccResource*>(res);
        ASSERT(mcc);
        if (mcc) {
          mcc->m_nreqs++;
          mcc->removeListeners();
          mcc_clear_err(mcc->m_mcc, NULL);
        }
      }
    }
    m_persistentObjects.clear();
  }

public:
  set<string> m_persistentObjects;
};

static RequestLocal<MccExtension> s_mcc_extension;

///////////////////////////////////////////////////////////////////////////////
// constants

const int64 k_MCC_IPPROTO_TCP = IPPROTO_TCP;
const int64 k_MCC_IPPROTO_UDP = IPPROTO_UDP;
const int64 k_MCC_SERVER_UP = MCC_SERVER_UP;
const int64 k_MCC_SERVER_DOWN = MCC_SERVER_DOWN;
const int64 k_MCC_SERVER_DISABLED = MCC_SERVER_DISABLED;
const int64 k_MCC_SERVER_RETRY_TMO_MS = MCC_SERVER_RETRY_TMO_MS_DEFAULT;
const int64 k_MCC_DGRAM_TMO_THRESHOLD = MCC_DGRAM_TMO_THRESHOLD_DEFAULT;
const int64 k_MCC_PORT_DEFAULT = MCC_PORT_DEFAULT;
const int64 k_MCC_POOLPREFIX_LEN = MCC_POOLPREFIX_LEN_DEFAULT;
const int64 k_MCC_MTU = MCC_MTU_DEFAULT;
const int64 k_MCC_RXDGRAM_MAX = MCC_RXDGRAM_MAX_DEFAULT;
const int64 k_MCC_CONN_TMO_MS = MCC_CONN_TMO_MS_DEFAULT;
const int64 k_MCC_CONN_NTRIES = MCC_CONN_NTRIES_DEFAULT;
const int64 k_MCC_DGRAM_NTRIES = MCC_DGRAM_NTRIES_DEFAULT;
const double k_MCC_DGRAM_TMO_WEIGHT = MCC_DGRAM_TMO_WEIGHT_DEFAULT;
const int64 k_MCC_NODELAY = MCC_NODELAY_DEFAULT;
const int64 k_MCC_POLL_TMO_US = MCC_POLL_TMO_US_DEFAULT;
const int64 k_MCC_PROXY_DELETE_OP = mcc_proxy_delete_op; // HPHP wut
const int64 k_MCC_PROXY_UPDATE_OP = mcc_proxy_update_op;
const int64 k_MCC_PROXY_ARITH_OP = mcc_proxy_arith_op;
const int64 k_MCC_PROXY_GET_OP = mcc_proxy_get_op;
const int64 k_MCC_TMO_MS = MCC_TMO_MS_DEFAULT;
const int64 k_MCC_UDP_REPLY_PORTS = MCC_UDP_REPLY_PORTS_DEFAULT;
const int64 k_MCC_WINDOW_MAX = MCC_WINDOW_MAX_DEFAULT;
const int64 k_MCC_HAVE_FB_SERIALIZATION = 1;
const StaticString k_MCC_ARG_FB_SERIALIZE_ENABLED =
  PHPMCC_ARG_FB_SERIALIZE_ENABLED;
const StaticString k_MCC_ARG_CONSISTENT_HASHING_PREFIXES =
  PHPMCC_ARG_CONSISTENT_HASHING_PREFIXES;
#if defined(HAVE_DEBUG_LOG)
const int64 k_MCC_HAVE_DEBUG_LOG = 1;
#else /* defined(HAVE_DEBUG_LOG) */
const int64 k_MCC_HAVE_DEBUG_LOG = 0;
#endif /* defined(HAVE_DEBUG_LOG) */
const StaticString k_MCC_ARG_DEBUG =
  PHPMCC_ARG_DEBUG;
const StaticString k_MCC_ARG_DEBUG_LOGFILE =
  PHPMCC_ARG_DEBUG_LOGFILE;
const int64 k_MCC_HAVE_ZLIB_COMPRESSION = 1;
const int64 k_MCC_COMPRESSION_THRESHHOLD =
  PHPMCC_COMPRESSION_THRESHOLD_DEFAULT;
/* configuration arg members */
const StaticString k_MCC_ARG_SERVERS = PHPMCC_ARG_SERVERS;
const StaticString k_MCC_ARG_MIRROR_CFG = PHPMCC_ARG_MIRROR_CFG;
const StaticString k_MCC_ARG_MIRROR_CFG_NAME = PHPMCC_ARG_MIRROR_CFG_NAME;
const StaticString k_MCC_ARG_MIRROR_CFG_MODEL = PHPMCC_ARG_MIRROR_CFG_MODEL;
const StaticString k_MCC_ARG_MIRROR_CFG_SERVERPOOLS =
  PHPMCC_ARG_MIRROR_CFG_SERVERPOOLS;
const StaticString k_MCC_ARG_COMPRESSION_THRESHOLD =
  PHPMCC_ARG_COMPRESSION_THRESHOLD;
const StaticString k_MCC_ARG_NZLIB_COMPRESSION = PHPMCC_ARG_NZLIB_COMPRESSION;
const StaticString k_MCC_ARG_CONN_TMO = PHPMCC_ARG_CONN_TMO;
const StaticString k_MCC_ARG_CONN_NTRIES = PHPMCC_ARG_CONN_NTRIES;
const StaticString k_MCC_ARG_DEFAULT_PREFIX = PHPMCC_ARG_DEFAULT_PREFIX;
/* Alias args delete_proxy key to proxy so old clients still see the expected
   key. */
const StaticString k_MCC_ARG_DELETE_PROXY = PHPMCC_ARG_PROXY;
const StaticString k_MCC_ARG_DGRAM_NTRIES = PHPMCC_ARG_DGRAM_NTRIES;
const StaticString k_MCC_ARG_DGRAM_TMO_WEIGHT = PHPMCC_ARG_DGRAM_TMO_WEIGHT;
const StaticString k_MCC_ARG_NODELAY = PHPMCC_ARG_NODELAY;
const StaticString k_MCC_ARG_PERSISTENT = PHPMCC_ARG_PERSISTENT;
const StaticString k_MCC_ARG_POLL_TMO = PHPMCC_ARG_POLL_TMO;
const StaticString k_MCC_ARG_PROXY = PHPMCC_ARG_PROXY;
const StaticString k_MCC_ARG_PROXY_OPS = PHPMCC_ARG_PROXY_OPS;
const StaticString k_MCC_ARG_TMO = PHPMCC_ARG_TMO;
const StaticString k_MCC_ARG_TCP_INACTIVITY_TIME =
  PHPMCC_ARG_TCP_INACTIVITY_TIME;
const StaticString k_MCC_ARG_NPOOLPREFIX = PHPMCC_ARG_NPOOLPREFIX;
const int64 k_MCC_TCP_INACTIVITY_TMO_DEFAULT =
  MCC_TCP_INACTIVITY_TMO_MS_DEFAULT;
const StaticString k_MCC_ARG_UDP_REPLY_PORTS = PHPMCC_ARG_UDP_REPLY_PORTS;
const StaticString k_MCC_ARG_WINDOW_MAX = PHPMCC_ARG_WINDOW_MAX;
const int64 k_MCC_CONSISTENCY_IGNORE =
  MccMirrorMcc::CONSISTENCY_IGNORE;
const int64 k_MCC_CONSISTENCY_MATCH_ALL =
  MccMirrorMcc::CONSISTENCY_MATCH_ALL;
const int64 k_MCC_CONSISTENCY_MATCH_HITS =
  MccMirrorMcc::CONSISTENCY_MATCH_HITS;
const int64 k_MCC_CONSISTENCY_MATCH_HITS_SUPERCEDES =
  MccMirrorMcc::CONSISTENCY_MATCH_HITS_SUPERCEDES;
const StaticString k_MCC_ARG_SERVER_RETRY_TMO_MS =
  PHPMCC_ARG_SERVER_RETRY_TMO_MS;
const StaticString k_MCC_ARG_DGRAM_TMO_THRESHOLD =
  PHPMCC_ARG_DGRAM_TMO_THRESHOLD;
const int64 k_MCC_GET_RECORD_ERRORS = PHPMCC_GET_RECORD_ERRORS;
const int64 k_MCC_DELETE_DELETED = PHPMCC_DELETE_DELETED;
const int64 k_MCC_DELETE_NOTFOUND = PHPMCC_DELETE_NOTFOUND;
const int64 k_MCC_DELETE_ERROR_LOG = PHPMCC_DELETE_ERROR_LOG;
const int64 k_MCC_DELETE_ERROR_NOLOG = PHPMCC_DELETE_ERROR_NOLOG;

const int64 k_PHPMCC_NEW_HANDLE = (int64)MccResource::PHPMCC_NEW_HANDLE;
const int64 k_PHPMCC_USED_FAST_PATH = (int64)MccResource::PHPMCC_USED_FAST_PATH;
const int64 k_PHPMCC_USED_SLOW_PATH = (int64)MccResource::PHPMCC_USED_SLOW_PATH;

const StaticString k_PHPMCC_VERSION = "1.6.2";
const int64 q_phpmcc_IPPROTO_TCP = IPPROTO_TCP;
const int64 q_phpmcc_IPPROTO_UDP = IPPROTO_UDP;

///////////////////////////////////////////////////////////////////////////////
// crc32

/* The crc32 functions and data was originally written by Spencer
 * Garrett <srg@quick.com> and was cleaned from the PostgreSQL source
 * tree via the files contrib/ltree/crc32.[ch].  No license was
 * included, therefore it is assumed that this code is public
 * domain.  Attribution still noted. */

static const uint32 crc32tab[256] = {
  0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
  0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
  0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
  0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
  0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
  0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
  0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
  0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
  0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
  0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
  0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
  0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
  0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
  0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
  0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
  0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
  0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
  0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
  0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
  0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
  0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
  0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
  0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
  0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
  0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
  0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
  0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
  0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
  0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
  0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
  0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
  0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
  0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
  0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
  0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
  0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
  0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
  0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
  0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
  0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
  0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
  0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
  0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
  0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
  0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
  0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
  0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
  0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
  0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
  0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
  0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
  0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
  0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
  0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
  0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
  0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
  0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
  0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
  0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
  0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
  0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
  0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
  0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
  0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

static inline uint32 phpmcc_crc32(const char* data, const size_t ndata) {
  uint32 result;
  size_t ix;

  result = ~0;
  for (ix = 0; ix < ndata; ix++)
    result = (result >> 8) ^ crc32tab[(result ^ data[ix]) & 0xff];

  //result = (~result >> 16) & 0x7fff;
  result = (~result) & 0x7fffffff;

  return result == 0 ? 1 : result;
}

///////////////////////////////////////////////////////////////////////////////
// nstring

static inline void phpstring_to_nstring(nstring_t &nstring, std::string s) {
  nstring.str = const_cast<char*>(s.data());
  nstring.len = s.size();
}

static inline void phpstring_to_nstring(nstring_t &nstring, CStrRef s) {
  nstring.str = const_cast<char*>(s.data());
  nstring.len = s.size();
}

static inline void phpstring_to_nstring(nstring_t &nstring, Variant var) {
  if (var.isString()) {
    String s = var.toString();
    nstring.str = const_cast<char*>(s.data());
    nstring.len = s.size();
  } else {
    nstring.str = NULL;
    nstring.len = 0;
  }
}

/** Converts PHP assoc array of string to a malloced array of nstrings. Note,
    this method references the string data in the PHP array.  It does not make
    a separate copy.  */
static inline nstring_t* nstring_array_new(nstring_t* &resultp,
                                           size_t &countp, CVarRef values) {
  nstring_t* result;
  size_t count;
  size_t ix = 0;

  if (values.is(KindOfArray)) {
    Array ht = values.toArray();
    count = ht.size();

    if ((result = (nstring_t*)malloc(sizeof(nstring_t) * count)) == NULL) {
      return result;
    }

    for (ArrayIter it = ht.begin(); !it.end(); ++it) {
      Variant key = it.first();
      Variant value = it.second();
      if (key.isString()) {
        String s = key.toString();
        result[ix].str = const_cast<char*>(s.data());
        result[ix].len = s.size();
        ix++;
      } else if (value.isString()) {
        String s = value.toString();
        result[ix].str = const_cast<char*>(s.data());
        result[ix].len = s.size();
        ix++;
      }
    }
  } else if (values.isString()) {
    if ((result = (nstring_t*)malloc(sizeof(nstring_t))) == NULL) {
      return result;
    }
    String s = values.toString();
    result->str = const_cast<char*>(s.data());
    result->len = s.size();
    ix = 1;
  } else {
    result = NULL;
  }

  resultp = result;
  countp = ix;
  return result;
}

static inline void nstring_array_del(nstring_t* nstring_array) {
  free(nstring_array);
}

/** Converts PHP assoc array of string to a malloced array of nstrings. Note,
    this method references the string data in the PHP array.  It does not make
    a separate copy.  */
static inline uint32 nstring_dict_new(MccResourcePtr &phpmcc,
                                      CArrRef input,
                                      nstring_t*& ret_keys,
                                      nstring_t*& ret_values) {
  nstring_t* keys = NULL, * values = NULL;
  size_t count;
  count = input.size();
  uint32 ix = 0;

  if ((keys = (nstring_t*)malloc(sizeof(nstring_t) * count)) == NULL) {
    goto epilogue;
  }
  if ((values = (nstring_t*)malloc(sizeof(nstring_t) * count)) == NULL) {
    goto epilogue;
  }

  for (ArrayIter it = input.begin(); !it.end(); ++it) {
    CVarRef key = it.first();
    ASSERT(key.isString());
    String skey = key.toString();
    keys[ix].str = const_cast<char*>(skey.data());
    keys[ix].len = skey.size();
    Variant value = it.second();
    ASSERT(value.isString());
    String sval = value.toString();
    values[ix].str = const_cast<char*>(sval.data());
    values[ix].len = sval.size();

    // at this point, if we've made it here, keys[ix] points to the key and
    // values[ix] points to the string-ified version of the value.
    ix++;
  }

  ret_keys = keys;
  ret_values = values;

 epilogue:
  if (ix == 0) {
    if (keys != NULL) {
      free(keys);
    }
    if (values != NULL) {
      free(values);
    }
  }
  return ix;
}

///////////////////////////////////////////////////////////////////////////////
// errors

static const char *error_types[MCC_UNKNOWN_ERROR + 1] = {
  /* [MCC_NO_ERROR] = */ "ok",
  /* [MCC_SYS_ERROR] = */ "system_error",
  /* [MCC_APP_ERROR] = */ "application_error",
  /* [MCC_REMOTE_ERROR] = */ "remote_error",
  /* [MCC_UNKNOWN_ERROR] = */ "unknown_error"
};

static inline const char* phpmcc_errtype_to_string(mcc_errtype_t type) {
  return error_types[type <= MCC_UNKNOWN_ERROR ? type : MCC_UNKNOWN_ERROR];
}

static void phpmcc_call_error_listeners(MccResourcePtr &phpmcc,
                                        CVarRef error_list) {
  ListenerIt listener;
  Array params;
  params.set(0, null);
  params.set(1, error_list);

  for (listener = phpmcc->m_error_listeners.begin();
       listener != phpmcc->m_error_listeners.end(); ++listener) {
    CStrRef function = (*listener)->m_function;
    params.set(0, ref((*listener)->m_context));
    if (f_function_exists(function)) {
      f_call_user_func_array(function, params);
    } else {
      Logger::Warning("[php_mcc] in %s on line %d error listener call back "
                      "failed for %s %p", __FILE__,  __LINE__,
                      (*listener)->m_function.data(), &(*listener)->m_context);
    }
  }
}

static Array phpmcc_err_to_phparray(const mcc_errtype_t type,
                                    const timeval_t timestamp, const int code,
                                    const char* source, const int lineno,
                                    const nstring_t message) {
  Array phperr;
  phperr.set("type", String(phpmcc_errtype_to_string(type), AttachLiteral));
  phperr.set("timestamp", timestamp.tv_sec * 1000 + timestamp.tv_usec / 1000);
  phperr.set("code", code);
  phperr.set("source", String(source, CopyString));
  phperr.set("line", lineno);
  if (message.len > 1) {
    phperr.set("message", String(message.str, message.len, CopyString));
  } else {
    phperr.set("message", String(""));
  }
  return phperr;
}

static Array phpmcc_errlist_to_phparray(const mcc_handle_t memcache) {
  Array errlist;
  const mcc_err_t* err = mcc_get_last_err(memcache);
  while ((err = mcc_get_last_err(memcache)) != NULL) {
    Array phperr = phpmcc_err_to_phparray
      (err->type, err->timestamp, err->code, err->source, err->lineno,
       err->message);
    errlist.append(phperr);
    mcc_clear_err(memcache, err);
  }
  return errlist;
}

static void phpmcc_log(MccResourcePtr &phpmcc, const mcc_errtype_t type,
                       const int code, const char* source, const int lineno,
                       const char* format, ...) {
  va_list ap;
  va_start(ap, format);

  timeval_t timestamp;
  gettimeofday(&timestamp, NULL);

  nstring_t message;
  char buf[PHPMCC_LOG_MAX];
  message.str = buf;
  if ((message.len = vsnprintf(buf, PHPMCC_LOG_MAX, format, ap)) >
      PHPMCC_LOG_MAX) {
    /* truncated message, just set length to everything but the null
       terminator */
    message.len = PHPMCC_LOG_MAX - 1;
  }

  Array err = phpmcc_err_to_phparray(type, timestamp, code,
                                     source, lineno, message);
  Array errlist;
  errlist.append(err);

  phpmcc_call_error_listeners(phpmcc, errlist);
}

static inline int mcc_log_if_error(MccResourcePtr &phpmcc) {
  const mcc_err_t* err = mcc_get_last_err(phpmcc->m_mcc);
  if (err == NULL) {
    return 0;
  }

  Array errlist = phpmcc_errlist_to_phparray(phpmcc->m_mcc);
  phpmcc_call_error_listeners(phpmcc, errlist);
  return 1;
}

///////////////////////////////////////////////////////////////////////////////
// serialization

static bool phpmcc_fb_serialize_value(MccResourcePtr &phpmcc,
                                      CStrRef key,
                                      phpmcc_flags_t& flags,
                                      CVarRef unserialized_value,
                                      String& serialized_value) {
  bool result = false;

  /* Quick check for object - which are not handled by fb serialization. */
  if (unserialized_value.is(KindOfObject)) {
    return result;
  }

  bool use_fb = phpmcc->m_fb_serialize_enabled &&
    phpmcc->m_fb_serialize_available;

  if (use_fb) {
    Variant sval = f_fb_thrift_serialize(unserialized_value);
    if (sval.isNull()) {
      result = false;
    } else {
      serialized_value = sval;
      flags = (phpmcc_flags_t)(flags | phpmcc_fb_serialized);
      result = true;
    }
  }
  return result;
}

static inline bool phpmcc_php_serialize_value(MccResourcePtr &phpmcc,
                                              CStrRef key,
                                              phpmcc_flags_t& flags,
                                              CVarRef unserialized_value,
                                              String& serialized_value) {
  Variant v = f_serialize(unserialized_value);
  if (v.isString()) {
    serialized_value = v.toString();
    flags = (phpmcc_flags_t) (flags | phpmcc_serialized);
    return true;
  }
  return false;
}

static inline int phpmcc_unserialize_value(MccResourcePtr &phpmcc,
                                           const phpmcc_flags_t flags,
                                           const char *serialized_value,
                                           int serialized_len,
                                           Variant& unserialized_value) {
  // 0 is fail, 1 is success
  if (serialized_len == 0) {
    unserialized_value = null;
    return 0;
  }

  if (flags & phpmcc_fb_serialized) {
    int pos = 0;
    if ((fb_unserialize_from_buffer(unserialized_value,
                                    serialized_value,
                                    serialized_len,
                                    &pos))) {
      return 0;
    }
    return 1;
  }

  if (flags & phpmcc_serialized) {
    std::istringstream in(std::string(serialized_value, serialized_len));
    VariableUnserializer vu(in);
    try {
      unserialized_value = vu.unserialize();
    } catch (Exception &e) {
      return 0;
    }
    return 1;
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// compression

#define NZLIB_MAGIC 0x6e7a6c69 /* nzli */
/* The new compression format stores a magic number and the size
   of the uncompressed object.  The magic number is stored to make sure
   bad values do not cause us to allocate bogus or extremely large amounts
   of memory when encountering an object with the new format. */
typedef struct _nzlib_format {
    uint32_t magic;
    uint32_t uncompressed_sz;
    Bytef buf[0];
} nzlib_format;

static int phpmcc_zlib_compress_value(MccResourcePtr &phpmcc,
                                      phpmcc_flags_t& flags,
                                      CStrRef uncompressed_value,
                                      String& compressed_value) {
  int result = 0;

  if (phpmcc->m_zlib_compression_enabled) {
    nzlib_format *compressed_object = NULL;
    uLong compressed_len;
    uLong internal_compressed_len;
    int status;
    char* compressed_buf;

    /* room for \0 and our header. taken from php_zlib */
    compressed_len = uncompressed_value.size() +
      (uncompressed_value.size() / 1000) + 15 + 5;
    compressed_object = (nzlib_format *)malloc(compressed_len);
    if (!compressed_object)
      goto epilogue;

    /* We want to steal the allocated compressed_value buf for the returned
       compressed_value zval, so compress into the compressed_object buf for
       nzlib compression or at the beginning of compressed_object itself if
       we aren't going to prepend a header. */

    if (phpmcc->m_nzlib_compression) {
      compressed_object->magic = htonl(NZLIB_MAGIC);
      compressed_object->uncompressed_sz = htonl(uncompressed_value.size());
      compressed_buf = (char*)compressed_object->buf;
      compressed_len -= sizeof(nzlib_format);
    } else {
      compressed_buf = (char*)compressed_object;
    }

    status = compress((Bytef*)compressed_buf, (uLongf *)&compressed_len,
                      (Bytef*)uncompressed_value.data(),
                      uncompressed_value.size());
    if (status != Z_OK) {
      goto epilogue;
    }

    /* XXX: Should we check for error? php_zlib does not. */
    internal_compressed_len = compressed_len +
      (phpmcc->m_nzlib_compression ? sizeof(*compressed_object) : 0);
    compressed_object = (nzlib_format*)realloc(compressed_object,
                                               internal_compressed_len + 1);
    ((char*)compressed_object)[internal_compressed_len] = '\0';
    compressed_value = String((char*)compressed_object,
                              internal_compressed_len, AttachString);
    flags =
      (phpmcc_flags_t)(flags |
                       ((phpmcc->m_nzlib_compression ? phpmcc_nzlib_compressed
                         : phpmcc_compressed)));

    /* We stole the buf from compressed object for compressed_value,
       so null it out so we don't attempt to free it. */

    compressed_object = NULL;

    result = 1;

  epilogue:

    if (!result) {
      if (compressed_object != NULL) {
        free(compressed_object);
      }
    }
  }

  return result;
}

static inline int phpmcc_zlib_uncompress_value(MccResourcePtr &phpmcc,
                                               uint32_t flags,
                                               const char *compressed_value,
                                               int compressed_len,
                                               String& uncompressed_value) {
  int result = 1;
  nzlib_format *nzlib_object;
  Bytef *compressed = NULL;
  Bytef *uncompressed = NULL;
  uint32_t factor = 1;
  uint32_t maxfactor = 16;
  uLong compressed_sz;
  uLong uncompressed_sz = 0;
  uLong length;
  int status = 0;

  if (flags & phpmcc_nzlib_compressed) {
    nzlib_object  = (nzlib_format *) compressed_value;

    if (ntohl(nzlib_object->magic) != NZLIB_MAGIC) {
      result = 0;
      goto epilogue;
    }
    compressed = nzlib_object->buf;
    compressed_sz = compressed_len - sizeof(*nzlib_object);
    uncompressed_sz = ntohl(nzlib_object->uncompressed_sz);
  } else {
    compressed = (Bytef *)compressed_value;
    compressed_sz = compressed_len;
  }

  /* Taken from php/ext/zlib/zlib.c
     zlib::uncompress() wants to know the output data length
     if none was given as a parameter
     we try from input length * 2 up to input length   2^15
     doubling it whenever it wasn't big enough
     that should be enough for all real life cases */
  do {
    length = uncompressed_sz ? uncompressed_sz :
      compressed_sz * (1 << factor++);
    uncompressed = (Bytef*)realloc(uncompressed, length);
    status = uncompress(uncompressed, &length, compressed, compressed_sz);
  } while (status == Z_BUF_ERROR && !uncompressed_sz && factor < maxfactor);
  if (status != Z_OK) {
    result = 0;
    goto epilogue;
  }
  /* Make sure the uncompressed_sz matches the length returned by zlib */
  if (uncompressed_sz != 0 && uncompressed_sz != length) {
    result = 0;
    goto epilogue;
  }

  /* XXX: Should we check for error? php_zlib does not. */
  uncompressed = (Bytef*)realloc(uncompressed, length + 1); /* space for \0 */
  uncompressed[length] = '\0';

  if (uncompressed != NULL) {
    uncompressed_value = String((char *)uncompressed, length, AttachString);
    uncompressed = NULL;
  }

 epilogue:
  if (!result) {
    /* Free the uncompressed if it exists */
    if (uncompressed != NULL) {
      free(uncompressed);
    }
  }
  return result;
}

///////////////////////////////////////////////////////////////////////////////
// transformation

/** Converts a PHP value into a string, performing all the necessary
    transformations.

    @param phpmcc          - php mcc handle.
    @param key             - a pointer to the key.
    @param object          - the object to transform.
    @param value           - where to store the resulting object.
    @param flags           - flags indicating the transformations completed is
    written to this address.
    @param compress        - if nonzero, compression is applied to the object.
    @param proxy_replicate - if nonzero, the proxy-replicate flag is set.
    @param async_set       - if nonzero, the async-set flag is set.
    additional php objects (zvals), the address of the
    zvals is written to this address.  the caller must
    free it when it is done with the value pointer.

    @return 0 if no error occured, nonzero otherwise.
*/
static int phpmcc_transform_value(MccResourcePtr &phpmcc,
                                  CStrRef key,
                                  CVarRef object,
                                  String& value,
                                  phpmcc_flags_t& flags,
                                  const bool compress,
                                  int64 proxy_replicate,
                                  int64 async_set) {
  String serialized, compressed;

  flags = (phpmcc_flags_t)0;
  if (!object.isString()) {
    if (!phpmcc_fb_serialize_value(phpmcc, key, flags, object, serialized) &&
        !phpmcc_php_serialize_value(phpmcc, key, flags, object, serialized)) {
      return -1;
    }
  } else {
    serialized = object.toString();
  }

  /* If compression fails or if result is greater than original, just send
     the uncompressed original. */
  if (compress && phpmcc->m_compression_threshold != 0 &&
      (size_t)serialized.size() >= phpmcc->m_compression_threshold) {

    if (!phpmcc_zlib_compress_value(phpmcc, flags, serialized, compressed) ||
        serialized.size() <= compressed.size()) {
      flags = (phpmcc_flags_t)
        (flags & ~(phpmcc_compressed | phpmcc_nzlib_compressed));
    } else {
      serialized = compressed;
    }
  }

  value = serialized;

  if (async_set) {
    flags = (phpmcc_flags_t)(flags | phpmcc_async_set);
  }
  if (proxy_replicate) {
    flags = (phpmcc_flags_t)(flags | phpmcc_proxy_replicate);
  }
  return 0;
}

static inline uint32 phpmcc_transform_values(MccResourcePtr &phpmcc,
                                             CArrRef input,
                                             Array& result,
                                             phpmcc_flags_t*& ret_flagss,
                                             bool compress,
                                             int64 proxy_replicate,
                                             int64 async_set) {
  phpmcc_flags_t* flagss = NULL;
  size_t count = input.size();
  uint32 ix = 0;

  if ((flagss =
       (phpmcc_flags_t*) malloc(sizeof(phpmcc_flags_t) * count)) == NULL) {
    return 0;
  }

  for (ArrayIter it = input.begin(); !it.end(); ++it) {
    CVarRef key = it.first();
    if (!key.isString()) continue;
    Variant value = it.second();
    // now examine the value.
    if (value.isNull()) {
      // no value?
      continue;
    }
    String res;
    if (phpmcc_transform_value(phpmcc, key.toString(), value, res, flagss[ix],
                               compress, proxy_replicate, async_set) != 0) {
      /* error has occured */
      continue;
    }
    result.set(key, res);

    ix++;
  }
  ret_flagss = flagss;
  return ix;
}

///////////////////////////////////////////////////////////////////////////////
// servers

static Array get_accesspoints(mcc_handle_t mcc, const nstring_t* server) {
  size_t naccesspoint;
  const nstring_t* accesspoints =
    mcc_server_get_accesspoints(mcc, server, &naccesspoint);

  Array aplist;
  if (accesspoints != NULL) {
    for (uint aix = 0; aix < naccesspoint; aix++) {
      aplist.append(String(accesspoints[aix].str, accesspoints[aix].len,
                           CopyString));
    }
    mcc_server_free_accesspoint_list(mcc, accesspoints, naccesspoint);
  }
  return aplist;
}

static Array get_serverpool_servers(mcc_handle_t mcc,
                                    const nstring_t* serverpool,
                                    bool is_mirror) {
  size_t nserver;
  const nstring_t* servers =
    mcc_serverpool_get_servers(mcc, serverpool, &nserver);

  Array slist = Array::Create();
  if (servers != NULL) {
    for (uint six = 0; six < nserver; six++) {
      Variant aplist = get_accesspoints(mcc, &servers[six]);
      if (is_mirror && aplist.isNull()) {
        continue;
      }
      slist.set(String(servers[six].str, servers[six].len, CopyString),
                aplist);
    }
    mcc_serverpool_free_server_list(mcc, servers, nserver);
  }
  return slist;
}

static Array get_serverpools(mcc_handle_t mcc, bool is_mirror) {
  size_t nserverpool;
  const nstring_t* serverpools = mcc_get_serverpools(mcc, &nserverpool);

  Array splist;
  if (serverpools != NULL) {
    for (uint spix = 0; spix < nserverpool; spix++) {
      Array slist = get_serverpool_servers(mcc, &serverpools[spix], is_mirror);
      splist.set(String(serverpools[spix].str, serverpools[spix].len,
                        CopyString), slist);
    }
    mcc_free_serverpool_list(mcc, serverpools, nserverpool);
  }
  return splist;
}

static Array get_mirror_cfg(MccResourcePtr &phpmcc) {
  Array mlist = Array::Create();
  for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
       mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
    Array mirror_cfg;
    mirror_cfg.set(PHPMCC_ARG_MIRROR_CFG_NAME, String((*mirror)->m_name));
    mirror_cfg.set(PHPMCC_ARG_MIRROR_CFG_MODEL, (*mirror)->m_model);
    Variant serverpools;
    if (!(serverpools = get_serverpools((*mirror)->m_mcc, 1)).isNull()) {
      mirror_cfg.set(PHPMCC_ARG_MIRROR_CFG_SERVERPOOLS, serverpools);
    }
    mlist.append(mirror_cfg);
  }
  return mlist;
}

/* XXX revert to bool once support for libch 2.0.0 is dropped. */
/**
 * Presently, the consistent hashing prefixes array stores a flag corresponding
 * to the last digit of the port on which the corresponding memcached instance
 * is running. This flag is decoded as:
 *   0: inconsistent hashing
 *   1: consistent hashing v2
 *   2: consistent hashing v3
 */
static Array get_consistent_hashing_prefixes(MccResourcePtr &phpmcc) {
  size_t nserverpool;
  const nstring_t* serverpools =
    mcc_get_serverpools(phpmcc->m_mcc, &nserverpool);

  Array splist = Array::Create();
  if (serverpools != NULL) {
    for (uint spix = 0; spix < nserverpool; spix++) {
      const nstring_t* const pool_name = &serverpools[spix];
      uint8_t version = mcc_serverpool_get_consistent_hashing_version
        (phpmcc->m_mcc, pool_name);
      if (version != 0) {
        /* Translates from version to version flag. */
        splist.set(String(serverpools[spix].str, serverpools[spix].len,
                          CopyString), (int64)(version - 1));
      }
    }
    mcc_free_serverpool_list(phpmcc->m_mcc, serverpools, nserverpool);
  }
  return splist;
}

static int64 phpmcc_addremove_server_helper(MccResourcePtr &phpmcc,
                                            CStrRef server,
                                            CStrRef mirrorname,
                                            bool is_add) {
  nstring_t nserver;
  phpstring_to_nstring(nserver, server);

  int64 retval;
  if (is_add) {
    retval = (long) mcc_add_server(phpmcc->m_mcc, &nserver);
    if (retval != 0) {
      phpmcc->m_server_count ++;
    }
  } else {
    retval = mcc_remove_server(phpmcc->m_mcc, &nserver);
    if (retval == 0) {
      phpmcc->m_server_count --;
    }
  }

  for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
       mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
    if (is_add) {
      mcc_add_server((*mirror)->m_mcc, &nserver);
    } else {
      mcc_remove_server((*mirror)->m_mcc, &nserver);
    }
  }

  return retval;
}

static int64 phpmcc_add_mirror_helper(MccResourcePtr &phpmcc,
                                      CStrRef mirrorname,
                                      MccMirrorMcc::ConsistencyModel model) {
  if (phpmcc->m_serverpool_count != 0) {
    phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_MIRROR_EXISTS,
               __FILE__, __LINE__,
               "cannot add mirror when serverpools exist.");
    return -1;
  }

  if (phpmcc->m_server_count != 0) {
    phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_MIRROR_EXISTS,
               __FILE__, __LINE__,
               "cannot add mirror when servers exist.");
    return -1;
  }

  for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
       mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
    if ((*mirror)->m_name == mirrorname.data()) {
      phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_MIRROR_EXISTS,
                 __FILE__, __LINE__,
                 "attempted to add mirror %.*s that already exists",
                 mirrorname.size(), mirrorname.data());
      return -2;
    }
  }
  MccMirrorMcc* mirr = new MccMirrorMcc(mirrorname, model, phpmcc);
  if (!mirr || mirr->m_mcc == NULL) {
    phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_MIRROR_CREATE,
               __FILE__, __LINE__,
               "failed to create mirror mcc object");
    if (mirr && mirr->m_mcc == NULL) {
      delete mirr;
    }
    return 3;
  }

  phpmcc->m_mirror_mccs.push_back(mirr);
  return 0;
}

static int64 phpmcc_remove_mirror_helper(MccResourcePtr &phpmcc,
                                         CStrRef mirrorname) {
  MirrorIt mirror;
  for (mirror = phpmcc->m_mirror_mccs.begin();
       mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
    if ((*mirror)->m_name == mirrorname.data()) {
      break;
    }
  }

  if (mirror != phpmcc->m_mirror_mccs.end()) {
    /* clean up mirror */
    MccMirrorMcc* mirr = *mirror;
    phpmcc->m_mirror_mccs.erase(mirror);
    delete mirr;
    return 0;
  } else {
    /* trying to remove a mirror that doesn't exist! */
    phpmcc_log(phpmcc, MCC_APP_ERROR,  PHPMCC_ERR_MIRROR_NOT_FOUND,
               __FILE__, __LINE__,
               "attempted to remove mirror %.*s that doesn't exist",
               mirrorname.size(), mirrorname.data());
    return 1;
  }
}

static int64
phpmcc_addremove_mirror_accesspoint_helper(MccResourcePtr &phpmcc,
                                           CStrRef mirrorname,
                                           CStrRef server,
                                           CStrRef host,
                                           CStrRef port,
                                           int32 protocol,
                                           bool is_add) {
  for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
       mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
    if ((*mirror)->m_name == mirrorname.data()) {
      nstring_t nhost, nserver, nport;
      phpstring_to_nstring(nhost, host);
      phpstring_to_nstring(nserver, server);
      phpstring_to_nstring(nport, port);
      if (is_add) {
        return (long) mcc_add_accesspoint((*mirror)->m_mcc, &nserver,
                                          &nhost, &nport,
                                          protocol,
                                          MCC_ASCII_PROTOCOL);
      } else {
        mcc_remove_accesspoint((*mirror)->m_mcc, &nserver,
                               &nhost, &nport,
                               protocol,
                               MCC_ASCII_PROTOCOL);
        return 1;
      }
    }
  }

  // mirror not found.
  phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_MIRROR_NOT_FOUND,
             __FILE__, __LINE__,
             "failed to find mirror %s", mirrorname.data());

  return 0;
}

static int64 phpmcc_add_serverpool_helper(MccResourcePtr &phpmcc,
                                          CStrRef sp_name,
                                          int64 consistent_hashing_enabled,
                                          int64 consistent_hashing_version) {
  mcc_sphandle_t sp_handle;
  nstring_t nsp;
  phpstring_to_nstring(nsp, sp_name);
  sp_handle = mcc_add_serverpool(phpmcc->m_mcc, &nsp,
                                 consistent_hashing_enabled,
                                 consistent_hashing_version);
  if (sp_handle != NULL) {
    phpmcc->m_serverpool_count ++;

    /* propagate to mirrors */
    for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
         mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
      mcc_handle_t mcch = (*mirror)->m_mcc;
      mcc_sphandle_t mirror_sp_handle;
      mirror_sp_handle = mcc_add_serverpool(mcch, &nsp,
                                            consistent_hashing_enabled,
                                            consistent_hashing_version);
      ASSERT(mirror_sp_handle != NULL);
    }
  }

  return (long) sp_handle;
}

static void phpmcc_remove_serverpool_helper(MccResourcePtr &phpmcc,
                                            CStrRef sp_name) {
  nstring_t nsp;
  phpstring_to_nstring(nsp, sp_name);
  if (mcc_remove_serverpool(phpmcc->m_mcc, &nsp) == 0) {
    ASSERT(phpmcc->m_serverpool_count > 0);
    phpmcc->m_serverpool_count--;

    /* propagate to mirrors */
    for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
         mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
      int retval;
      retval = mcc_remove_serverpool((*mirror)->m_mcc, &nsp);
      ASSERT(retval == 0);
    }
  }
}

static int64
phpmcc_serverpool_addremove_server_helper(MccResourcePtr &phpmcc,
                                          CStrRef sp_name,
                                          CStrRef server_name,
                                          CStrRef mirrorname,
                                          int is_add) {
  int64 retval;
  typedef int (*add_remove_funcptr_t) (mcc_handle_t mcc,
                                       const nstring_t* serverpool,
                                       const nstring_t* server);

  add_remove_funcptr_t op;

  /* adding or removing? */
  if (is_add) {
    op = mcc_serverpool_add_server;
  } else {
    op = mcc_serverpool_remove_server;
  }
  nstring_t nsp, nsn;
  phpstring_to_nstring(nsp, sp_name);
  phpstring_to_nstring(nsn, server_name);
  retval = op(phpmcc->m_mcc, &nsp, &nsn);
  if (retval) {
    /* propagate to mirrors */
    for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
         mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
      int retval;
      retval = op((*mirror)->m_mcc, &nsp, &nsn);
      ASSERT(retval);
    }
  }
  return retval;
}

static void phpmcc_apevent_dispatcher(MccResourcePtr &phpmcc) {
  ApEventIt event;
  ListenerIt listener;

  for (event = phpmcc->m_events.begin();
       event != phpmcc->m_events.end(); ++event) {
    Array params;
    params.append(null);
    params.append(String(event->m_server));
    params.append(String(event->m_host));
    params.append(String(event->m_port));
    params.append(event->m_protocol);
    params.append(event->m_presentation_protocol);
    params.append(event->m_id);

    Array se_params;
    se_params.append(null);
    ostringstream ss;
    ss << event->m_server << "[" <<
      (event->m_protocol == IPPROTO_TCP ? "tcp" : "udp") << ":" <<
      event->m_host << ":" << event->m_port << "]";
    se_params.append(String(ss.str().c_str(), CopyString));
    params.append(event->m_id == MCC_AP_UP ? MCC_SERVER_UP : MCC_SERVER_DOWN);

    for (listener = phpmcc->m_ap_listeners.begin();
         listener != phpmcc->m_ap_listeners.end(); ++listener) {
      CStrRef function = (*listener)->m_function;
      Variant & con = (*listener)->m_context;
      params.set(0, ref(con));
      if (f_function_exists(function)) {
        f_call_user_func_array(function, params);
      } else {
        Logger::Warning("[php_mcc] in %s on line %d accesspoint listener "
                        "call back failed for %s %p", __FILE__, __LINE__,
                        (*listener)->m_function.data(),
                        &(*listener)->m_context);
      }
    }

    for (listener = phpmcc->m_server_listeners.begin();
         listener != phpmcc->m_server_listeners.end(); ++listener) {
      CStrRef function = (*listener)->m_function;
      Variant & con = (*listener)->m_context;
      se_params.set(0, ref(con));
      if (f_function_exists(function)) {
        f_call_user_func_array(function, se_params);
      } else {
        Logger::Warning("[php_mcc] in %s on line %d server listener call back "
                        "failed for %s %p", __FILE__, __LINE__,
                        (*listener)->m_function.data(),
                        &(*listener)->m_context);
      }
    }
  }
  phpmcc->m_events.clear();
}

///////////////////////////////////////////////////////////////////////////////
// listeners

static bool same_context(CVarRef context1, CVarRef context2) {
  if (context1.isArray() && context2.isArray()) {
    return (context1.getArrayData() == context2.getArrayData());
  }
  if (context1.isObject() && context2.isObject()) {
    return (context1.getObjectData() == context2.getObjectData());
  }
  if (context1.getType() == KindOfString &&
      context2.getType() == KindOfString) {
    return (context1.getStringData() == context2.getStringData());
  }
  return (context1.same(context2));
}

static bool zim_phpmcc_add_listener(MccResourcePtr &phpmcc,
                                    CStrRef function,
                                    CVarRef context,
                                    MccListener::ListenerType type) {
  if (!phpmcc.get()) {
    return false;
  }

  ListenerIt listener;
  deque<MccListener*>* listeners =
    type == MccListener::AccessPointListener ? &phpmcc->m_ap_listeners :
    type == MccListener::ServerListener ? &phpmcc->m_server_listeners :
    &phpmcc->m_error_listeners;
  for (listener = listeners->begin(); listener != listeners->end();
       ++listener) {
    if (same_context(context, (*listener)->m_context)) {
      Logger::Warning("[php_mcc] in %s on line %d "
                      "server listener %p already exists",
                      __FILE__, __LINE__, &context);
      return false;
    }
  }

  MccListener* list = new MccListener(type, function, context);
  if (!list) return false;
  listeners->push_back(list);
  return true;
}

static bool zim_phpmcc_remove_listener(MccResourcePtr &phpmcc,
                                       CStrRef function,
                                       CVarRef context,
                                       MccListener::ListenerType type) {
  if (!phpmcc.get()) {
    return false;
  }

  ListenerIt listener;
  bool found = false;

  deque<MccListener*>* listeners =
    type == MccListener::AccessPointListener ? &phpmcc->m_ap_listeners :
    type == MccListener::ServerListener ? &phpmcc->m_server_listeners :
    &phpmcc->m_error_listeners;
  for (listener = listeners->begin(); listener != listeners->end();
       ++listener) {
    if (same_context(context, (*listener)->m_context)) {
      found = true;
      break;
    }
  }

  if (found) {
    MccListener* l = *listener;
    listeners->erase(listener);
    delete l;
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// updates

static Variant phpmcc_update(MccResourcePtr &phpmcc,
                             CStrRef key,
                             CVarRef object,
                             int64 exptime,
                             bool compress,
                             int64 proxy_replicate,
                             int64 async_set,
                             mcc_cmdtype_t cmd) {
  String value;
  phpmcc_flags_t flags = (phpmcc_flags_t)0;
  mcc_res_t result = mcc_res_local_error;
  mcc_req_t* req = NULL;
  typedef mcc_req_t* (*update_funcptr_t) (mcc_handle_t, const nstring_t* const,
                                          const uint32_t, const uint32_t,
                                          const nstring_t* const,
                                          const size_t);
  update_funcptr_t update_op;

  if (phpmcc_transform_value(phpmcc, key, object, value, flags,
                             compress, proxy_replicate, async_set) == 0) {
    if (RuntimeOption::MemcacheReadOnly) {
      return true;
    }
    /* get the function pointer for the command type so we don't have to keep
     * on switching on cmd.  this is in theory taken care of by CSE, but we'll
     * do it also for readability. */
    bool ok = true;
    switch (cmd) {
    case mcc_set_cmd:
      update_op = mcc_set;
      break;
    case mcc_add_cmd:
      update_op = mcc_add;
      break;
    case mcc_replace_cmd:
      update_op = mcc_replace;
      break;
    default:
      result = mcc_res_local_error;
      ASSERT(0);
      ok = false;
    }
    if (ok) {
      /* make the request on the primary. */
      nstring_t nvalue, nkey;
      phpstring_to_nstring(nvalue, value);
      phpstring_to_nstring(nkey, key);
      req = update_op(phpmcc->m_mcc, &nkey, flags, exptime, &nvalue, 1);
      /* propagation to mirrors for update might be handled by mcproxy.  if a
       * update proxy has not been set, then propagate it ourselves. */
      if ((phpmcc->m_proxy_ops & mcc_proxy_update_op) == 0) {
        for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
             mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
          mcc_req_t* mirror_req =
            update_op((*mirror)->m_mcc, &nkey, flags, exptime, &nvalue, 1);
          if (mirror_req != NULL) {
            mcc_req_del(mirror_req);
          }
          mcc_clear_err(phpmcc->m_mcc, NULL);
        }
      }

      if (req != NULL) {
        result = req->result;
        mcc_req_del(req);
      }
    }
  }

  Variant retval;
  if (result == mcc_res_stored) {
    retval = true;
  } else if (result == mcc_res_notstored) {
    retval = false;
  } else {
    mcc_log_if_error(phpmcc);
    retval = null;
  }
  if (phpmcc->m_events.size() > 0) {
    phpmcc_apevent_dispatcher(phpmcc);
  }
  return retval;
}

/* todo work out these optional args */
static Variant zim_phpmcc_update(MccResourcePtr &phpmcc,
                                 CVarRef phpkey,
                                 CVarRef value,
                                 int64 exptime /* = 0 */,
                                 bool compress /* = 1 */,
                                 int64 proxy_replicate /* = 0 */,
                                 int64 async_set /* = 0 */,
                                 mcc_cmdtype_t cmd) {
  if (!phpmcc.get()) {
    return null;
  }

  if (!phpkey.isString()) {
    phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_MISSINGS_REQS,
               __FILE__, __LINE__, "key must be a string");
    return null;
  }

  return phpmcc_update(phpmcc, phpkey.toString(), value,
                       exptime, compress, proxy_replicate, async_set, cmd);
}

static Variant phpmcc_multi_update(MccResourcePtr &phpmcc,
                                   mcc_cmdtype_t cmd, CArrRef keys_values,
                                   uint32_t exptime, bool compress,
                                   int64 proxy_replicate, int64 async_set) {
  nstring_t* keys = NULL, * values = NULL;
  phpmcc_flags_t* flagss = NULL;
  unsigned npairs;
  mcc_req_t* reqs, * current;
  int log_errors = 0;

  Array transformed;
  if (!phpmcc_transform_values(phpmcc, keys_values, transformed, flagss,
                               compress, proxy_replicate, async_set)) {
    return null;
  }
  ASSERT(flagss);

  if ((npairs = nstring_dict_new(phpmcc, transformed, keys, values)) == 0) {
    return null;
  }

  current = reqs = mcc_update_generic
    (phpmcc->m_mcc, cmd, keys, (uint32_t*)flagss,
     &exptime, values, npairs, 1 /* unique flags */,
     0 /* shared expiration times */);
  Array retval;
  while (current != NULL) {
    nstring_t* key = &current->u.getreq.key;
    Variant phpvalue;

    if (current->result == mcc_res_stored) {
      phpvalue = true;
    } else if (current->result == mcc_res_notstored) {
      phpvalue = false;
    } else {
      log_errors = 1;
      phpvalue = null;
    }
    retval.set(String(key->str, key->len, CopyString), phpvalue);

    current = current->item.tqe_next;
  }

  if (reqs != NULL) {
    mcc_req_del(reqs);
  }

  /* propagation to mirrors for update might be handled by mcproxy.  if a
   * update proxy has not been set, then propagate it ourselves. */
  if ((phpmcc->m_proxy_ops & mcc_proxy_update_op) == 0) {
    for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
         mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
      mcc_req_t* mirror_reqs =
        mcc_update_generic((*mirror)->m_mcc, cmd, keys, (uint32_t*)flagss,
                           &exptime, values, npairs,
                           1 /* unique flags */,
                           0 /* shared expiration times */);

      if (mirror_reqs != NULL) {
        mcc_req_del(mirror_reqs);
      }
      mcc_clear_err(phpmcc->m_mcc, NULL);
    }
  }

  if (keys != NULL) {
    nstring_array_del(keys);
  }
  if (values != NULL) {
    nstring_array_del(values);
  }
  if (flagss != NULL) {
    free(flagss);
  }
  if (log_errors == 1) {
    mcc_log_if_error(phpmcc);
  }

  /* If we didn't get any results, check for errors */
  if (phpmcc->m_events.size() > 0) {
    phpmcc_apevent_dispatcher(phpmcc);
  }
  return retval;
}

static Variant zim_phpmcc_multi_update(MccResourcePtr &phpmcc,
                                       CArrRef keys_values,
                                       int64 exptime /* = 0 */,
                                       int64 compress /* = 1 */,
                                       int64 proxy_replicate /* = 0 */,
                                       int64 async_set /* = 0 */,
                                       mcc_cmdtype_t cmd) {
  if (!phpmcc.get()) {
    return null;
  }
  return phpmcc_multi_update(phpmcc, cmd, keys_values, exptime,
                             compress, proxy_replicate, async_set);
}

static Variant zim_phpmcc_arith_update(MccResourcePtr &phpmcc,
                                       CStrRef key,
                                       int64 value /* = 1 */,
                                       mcc_cmdtype_t cmd) {
  if (!phpmcc.get()) {
    return null;
  }

  Variant retval;
  int64 result = 0;
  mcc_res_t status;
  typedef uint32_t (*arith_funcptr_t) (mcc_handle_t, const nstring_t* const,
                                       const int, mcc_res_t*);
  arith_funcptr_t op;

  switch(cmd) {
  case mcc_incr_cmd:
    op = mcc_incr_ex;
    break;
  case mcc_decr_cmd:
    op = mcc_decr_ex;
    break;
  default:
    ASSERT(0);
    return 0;
  }
  nstring_t nkey;
  phpstring_to_nstring(nkey, key);
  result = op(phpmcc->m_mcc, &nkey, value, &status);

  if (status != mcc_res_stored) {
    mcc_log_if_error(phpmcc);
    retval = false;
  } else {
    retval = result;
  }

  /* propagation to mirrors for increment/decrement might be handled by
   * mcproxy.  if a delete proxy has not been set, then propagate it
   * ourselves. */
  if ((phpmcc->m_proxy_ops & mcc_proxy_arith_op) == 0) {
    for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
         mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
      result = op((*mirror)->m_mcc, &nkey, value, &status);
    }
  }

  if (phpmcc->m_events.size() > 0) {
    phpmcc_apevent_dispatcher(phpmcc);
  }
  return retval;
}

static Variant phpmcc_delete(MccResourcePtr &phpmcc, CVarRef phpkeys,
                             int64 exptime, bool with_details) {
  nstring_t* keys;
  size_t nkeys;
  if (nstring_array_new(keys, nkeys, phpkeys) == NULL) {
    return null;
  }

  mcc_req_t* reqs = mcc_delete(phpmcc->m_mcc, keys, nkeys, exptime);
  mcc_req_t* current = reqs;

  Variant retval;
  if (phpkeys.is(KindOfArray)) {
    while (current != NULL) {
      nstring_t* key = &current->u.getreq.key;
      String skey(key->str, key->len, CopyString);
      switch(current->result) {
      case mcc_res_deleted:
      case mcc_res_notfound:
        if (with_details) {
          retval.set(skey, (current->result == mcc_res_deleted) ?
                     PHPMCC_DELETE_DELETED : PHPMCC_DELETE_NOTFOUND);
        } else {
          retval.set(skey, current->result == mcc_res_deleted);
        }
        break;
      case mcc_res_bad_key:
        if (with_details) {
          retval.set(skey, PHPMCC_DELETE_ERROR_NOLOG);
        } else {
          retval.set(skey, null);
        }
        break;
      default:
        if (with_details) {
          retval.set(skey, PHPMCC_DELETE_ERROR_LOG);
        } else {
          retval.set(skey, null);
        }
        break;
      }

      current = current->item.tqe_next;
    }
  } else if (phpkeys.isString()) {
    if (current != NULL) {
      switch(current->result) {
      case mcc_res_deleted:
      case mcc_res_notfound:
        if (with_details) {
          retval = (current->result == mcc_res_deleted) ?
            PHPMCC_DELETE_DELETED : PHPMCC_DELETE_NOTFOUND;
        } else {
          retval = current->result == mcc_res_deleted;
        }
        break;
      case mcc_res_bad_key:
        if (with_details) {
          retval = PHPMCC_DELETE_ERROR_NOLOG;
        } else {
          retval = null;
        }
        break;
      default:
        Logger::Error("php_mcc: Failed to delete key %s with result %d",
                      phpkeys.toString().data(), (int)current->result);
        if (with_details) {
          retval = PHPMCC_DELETE_ERROR_LOG;
        } else {
          retval = null;
        }
        break;
      }
    }
  }

  mcc_req_del(reqs);

  /* propagation to mirrors for delete might be handled by mcproxy.  if a
   * delete proxy has not been set, then propagate it ourselves. */
  if ((phpmcc->m_proxy_ops & mcc_proxy_delete_op) == 0) {
    for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
         mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
      reqs = mcc_delete((*mirror)->m_mcc, keys, nkeys, exptime);
      mcc_req_del(reqs);
    }
  }

  nstring_array_del(keys);
  if (phpmcc->m_events.size() > 0) {
    phpmcc_apevent_dispatcher(phpmcc);
  }
  return retval;
}

///////////////////////////////////////////////////////////////////////////////
// processors

/*
  phpmcc_get works with a callback setup.  phpmcc_get(..) sets up the keys and
  the callback context structure.  Then it calls phpmcc_get_helper(..), which
  retrieves the values from the primary group and the mirrors.  Once it has
  resolved the return values from the primary group and the mirrors, it
  post-processes the values into php zvals and calls the callback functions.

  There are two callback functions currently defined.  One handles multiget
  requests, and places each zval into a dictionary.  The other handles single
  get requests, and places the zval into the return value.
*/
static void
phpmcc_multiget_processor(const nstring_t search_key,
                          mcc_res_t result, CVarRef value,
                          phpmcc_get_processor_context_t* context) {
  Variant& hashtable = *(context->results);
  if (result == mcc_res_found) {
    context->hits ++;
    String skey(search_key.str, search_key.len, CopyString);
    hashtable.set(skey, value);
  } else if (mcc_res_is_err(result)) {
    context->errors ++;
  }
}

static void
phpmcc_multiget_record_errors_processor(const nstring_t search_key,
                                        mcc_res_t result,
                                        CVarRef value,
                                        phpmcc_get_processor_context_t* context) {
  Variant& hashtable = *(context->results);
  Variant& error_keys = *(context->additional_context);
  String skey(search_key.str, search_key.len, CopyString);
  if (result == mcc_res_found) {
    context->hits ++;
    if (!value.isNull()) {
      hashtable.set(skey, value);
    }
  } else {
    if (mcc_res_is_err(result)) {
      error_keys.set(skey, true);
      context->errors ++;
    }
  }
}

static void phpmcc_get_processor(const nstring_t search_key,
                                 mcc_res_t result,
                                 CVarRef value,
                                 phpmcc_get_processor_context_t* context) {
  Variant& retval = *(context->results);

  if (result == mcc_res_found) {
    context->hits ++;
    if (!value.isNull()) {
      retval = value;
      return;
    }
  }

  if (mcc_res_is_err(result)) {
    context->errors ++;
  }

  retval = null;
}

///////////////////////////////////////////////////////////////////////////////
// results

static void process_mirror_results(MccResourcePtr &phpmcc,
                                   const nstring_t& key,
                                   mcc_res_t* const result_ptr,
                                   nstring_t* const value_ptr,
                                   uint32_t* const flags_ptr,
                                   MccMirrorMcc::ConsistencyModel model,
                                   const mcc_req_t* mirror_result) {
  switch (model) {
  case MccMirrorMcc::CONSISTENCY_IGNORE:
    return;

  case MccMirrorMcc::CONSISTENCY_MATCH_ALL:
    /* check for a mismatch of return status (hit vs miss).  then
     * fallthrough and check the actual return values if they are both
     * hits. */
    if (*result_ptr != mirror_result->result) {
      /* the only case we permit the status to mismatch is
       * mcc_res_local_error, because that's how libmcc signals that
       * the server has no access points.  this is unfortunately
       * rather crude, as mcc_res_local_error is also returned when
       * the server has access points but they are all down. */
      if (mirror_result->result == mcc_res_local_error) {
        return;
      }

      phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_MIRROR_CONSISTENCY,
                 __FILE__, __LINE__,
                 "CONSISTENCY_MATCH_ALL result comparison failed "
                 "(key = \"%.*s\").", key.len, key.str);
      *result_ptr = mcc_res_remote_error;
    }

    if (*result_ptr != mcc_res_found) {
      return;
    }
    break;

  case MccMirrorMcc::CONSISTENCY_MATCH_HITS:
    /* if either is a miss, don't compare. */
    if (*result_ptr != mcc_res_found ||
        mirror_result->result != mcc_res_found) {
      return;
    }
    break;

  case MccMirrorMcc::CONSISTENCY_MATCH_HITS_SUPERCEDES:
    /* if we miss in the primary and hit in the mirror, replace the
     * result with the mirror's result.  immediately return because
     * there's no need to do any checking. */
    if (*result_ptr != mcc_res_found &&
        mirror_result->result == mcc_res_found) {
      *result_ptr = mcc_res_found;
      *value_ptr = mirror_result->u.getreq.value;
      *flags_ptr = mirror_result->u.getreq.flags;
      return;
    }

    /* if either is a miss, don't compare. */
    if (*result_ptr != mcc_res_found ||
        mirror_result->result != mcc_res_found) {
      return;
    }
    break;
  }

  /* both the primary and the mirror hit.  compare the results. */
  ASSERT(*result_ptr == mcc_res_found);
  ASSERT(mirror_result->result == mcc_res_found);

  if (*flags_ptr != mirror_result->u.getreq.flags ||
      nstring_cmp(value_ptr, &mirror_result->u.getreq.value) != 0) {
    /* the results mismatched in some way, flag the error. */
    phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_MIRROR_CONSISTENCY,
               __FILE__, __LINE__,
               "result comparison failed (key = \"%.*s\").",
               key.len, key.str);

    /* squash the hit */
    *result_ptr = mcc_res_remote_error;
  }
}

/* given a key, and the result from the primary server, determine the final
 * value to return to the client.  this must take into account the mirrors and
 * their results as well as deserializing and decompressing. */
static mcc_res_t resolve_value(MccResourcePtr &phpmcc,
                               const nstring_t& key, mcc_res_t result,
                               nstring_t value, uint32 flags,
                               mcc_req_t* const * const mirror_reqs,
                               Variant& php_value) {
  /* first, evaluate the mirrors. */
  int i = 0;
  for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
       mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
    mcc_req_t* iterator = mirror_reqs[i];
    if (iterator == NULL) {
      continue;
    }

    ASSERT(nstring_cmp(&key, &iterator->u.getreq.key) == 0);

    /* found our key */
    process_mirror_results(phpmcc, key, &result, &value, &flags,
                           (*mirror)->m_model, iterator);
    i++;
  }

  if (result == mcc_res_found) {
    int valid = 1;
    const char *val = value.str;
    int len = value.len;
    String php_uncompressedvalue;
    if (flags & (phpmcc_compressed | phpmcc_nzlib_compressed)) {
      if ((valid = phpmcc_zlib_uncompress_value(phpmcc, flags, val, len,
                                                php_uncompressedvalue))) {
        val = php_uncompressedvalue.data();
        len = php_uncompressedvalue.size();
      } else {
        phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_UNCOMPRESS,
                   __FILE__, __LINE__, "failed to uncompress key %s",
                   key.str);
      }
    }

    if (valid) {
      if (flags & (phpmcc_serialized | phpmcc_fb_serialized)) {
        if (!(valid = phpmcc_unserialize_value(phpmcc, (phpmcc_flags_t)flags,
                                               val, len, php_value))) {
          phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_UNSERIALIZE,
                     __FILE__, __LINE__, "failed to unserialize key %s",
                     key.str);
        }
      } else {
        if (val == value.str) {
          php_value = String(val, len, CopyString);
        } else {
          ASSERT(val == php_uncompressedvalue.data());
          php_value = php_uncompressedvalue;
        }
      }
    }

    /* Only add value if we successful */
    if (!valid) {
      php_value = null;
      phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_BAD_VALUE,
                 __FILE__, __LINE__, "invalid value for %s %u %u",
                 key.str, flags, len);
    }
  }
  return result;
}

/* phpmcc_get_helper retrieves the values from memcache.*/
static int phpmcc_get_helper(MccResourcePtr &phpmcc, nstring_t* keys,
                             unsigned int nkeys,
                             phpmcc_get_processor_funcptr_t func,
                             phpmcc_get_processor_context_t* context) {
  mcc_req_t** mirror_reqs = NULL, ** mirror_current = NULL;
  mcc_req_t* reqs = NULL, * current;
  unsigned int mirror_index;
  unsigned int reqs_returned = 0;

  reqs = mcc_get(phpmcc->m_mcc, keys, nkeys);
  size_t mirr_ct = phpmcc->m_mirror_mccs.size();
  /* propagate to mirrors */
  if (mirr_ct > 0) {
    mirror_reqs = (mcc_req_t**)malloc(sizeof(mcc_req_t*) * mirr_ct * 2);
    if (mirror_reqs == NULL) {
      free(mirror_reqs);
      return -1;
    }
    mirror_current = &mirror_reqs[mirr_ct];

    mirror_index = 0;
    for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
         mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
      mirror_current[mirror_index] = mirror_reqs[mirror_index] =
        mcc_get((*mirror)->m_mcc, keys, nkeys);

      mcc_clear_err((*mirror)->m_mcc, NULL);
      mirror_index ++;
    }
  }

  for (current = reqs; current != NULL; current = TAILQ_NEXT(current, item),
         reqs_returned ++) {
    const nstring_t search_key = current->u.getreq.key;
    mcc_res_t primary_result = current->result;
    nstring_t primary_value = current->u.getreq.value;
    uint32_t primary_flags = current->u.getreq.flags;
    mcc_res_t final_result;
    Variant result;

    for (mirror_index = 0; mirror_index < mirr_ct; mirror_index ++) {
      for (mirror_current[mirror_index] = mirror_reqs[mirror_index];
           mirror_current[mirror_index] != NULL;
           mirror_current[mirror_index] =
             TAILQ_NEXT(mirror_current[mirror_index], item)) {
        if (nstring_cmp(&search_key,
                        &(mirror_current[mirror_index]->u.getreq.key)) == 0) {
          break;
        }
      }
    }

    final_result = resolve_value(phpmcc, search_key,
                                 primary_result, primary_value, primary_flags,
                                 mirror_current, result);
    func(search_key, final_result, result, context);
  }

  if (reqs_returned != nkeys) {
    phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_MISSINGS_REQS,
               __FILE__, __LINE__,
               "executed gets on %d keys but only received %d reqs",
               nkeys, reqs_returned);
  }

  if (reqs != NULL) {
    mcc_req_del(reqs);
  }

  for (mirror_index = 0; mirror_index < mirr_ct; mirror_index ++) {
    if (mirror_reqs[mirror_index] != NULL) {
      mcc_req_del(mirror_reqs[mirror_index]);
    }
  }
  if (mirror_reqs != NULL) {
    free(mirror_reqs);
  }

  return 0;
}

static void phpmcc_get(MccResourcePtr &phpmcc, Variant& retval,
                       CVarRef phpkeys,
                       phpmcc_get_details_t detailed_info_mode,
                       Variant& detailed_info) {
  nstring_t* keys = NULL;
  size_t nkeys = 0;

  phpmcc_get_processor_context_t context;
  phpmcc_get_processor_funcptr_t processor;

  if (nstring_array_new(keys, nkeys, phpkeys) == NULL) {
    goto epilogue;
  }

  context.hits = 0;
  context.errors = 0;
  if (phpkeys.is(KindOfArray)) {
    retval = Array::Create();
    context.results = &retval;

    // check to see if we have what we need to proceed.  if the arguments
    // do not check out, revert to normal multiget.
    switch (detailed_info_mode) {
    case PHPMCC_GET_RECORD_ERRORS:
      // if the variable is a null, then let's make it a value.
      if (detailed_info.isNull()) {
        detailed_info = Array::Create();
      }

      if (!detailed_info.is(KindOfArray)) {
        phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_DETAILED_BAD_ARG,
                   __FILE__, __LINE__,
                   "detailed info argument must be an array");
        detailed_info_mode = PHPMCC_GET_DEFAULT;
        break;
      }
      break;

    case PHPMCC_GET_DEFAULT:
      /* no special condition */
      break;

    default:
      phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_DETAILED_BAD_ARG,
                 __FILE__, __LINE__,
                 "detailed info mode is unknown (%d)", detailed_info_mode);
      detailed_info_mode = PHPMCC_GET_DEFAULT;
      break;
    }

    // if there was a problem with the arguments, we revert to a normal
    // multiget.  actually do the work here.
    switch (detailed_info_mode) {
    case PHPMCC_GET_RECORD_ERRORS:
      processor = phpmcc_multiget_record_errors_processor;
      context.additional_context = &detailed_info;
      break;

    case PHPMCC_GET_DEFAULT:
    default:
      processor = phpmcc_multiget_processor;
      context.additional_context = NULL;
      break;
    }
  } else {
    context.results = &retval;
    processor = phpmcc_get_processor;
  }

  if (phpmcc_get_helper(phpmcc, keys, nkeys, processor, &context) != 0) {
    goto epilogue;
  }

 epilogue:
  if (keys != NULL) {
    nstring_array_del(keys);
  }

  /* If we didn't get any results, check for errors */
  if ((context.errors > 0) || (context.hits == 0 && nkeys != 0)) {
    mcc_log_if_error(phpmcc);
  }

  if (phpmcc->m_events.size() > 0) {
    phpmcc_apevent_dispatcher(phpmcc);
  }
}

///////////////////////////////////////////////////////////////////////////////
// stats

typedef struct stat_map_s {
  const char* name;
  size_t off;
} stat_map_t;

static stat_map_t stat_map[] = {
  {"ntx_msgs",      offsetof(mcc_stats_t, ntx_msgs)},
  {"ntx_dgmsgs",    offsetof(mcc_stats_t, ntx_dgmsgs)},
  {"ntx_errs",      offsetof(mcc_stats_t, ntx_errs)},
  {"ntx_dgerrs",    offsetof(mcc_stats_t, ntx_dgerrs)},
  {"ntx_dgtmos",    offsetof(mcc_stats_t, ntx_dgtmos)},
  {"ntx_tmos",      offsetof(mcc_stats_t, ntx_tmos)},

  {"nrx_msgs",      offsetof(mcc_stats_t, nrx_msgs)},
  {"nrx_dgmsgs",    offsetof(mcc_stats_t, nrx_dgmsgs)},
  {"nrx_errs",      offsetof(mcc_stats_t, nrx_errs)},
  {"nrx_dgerrs",    offsetof(mcc_stats_t, nrx_dgerrs)},
  {"nrx_dgooos",    offsetof(mcc_stats_t, nrx_dgooos)},
  {"nrx_dgtmos",    offsetof(mcc_stats_t, nrx_dgtmos)},

  {"nrx_tmos",      offsetof(mcc_stats_t, nrx_tmos)},
  {"nrx_aborts",    offsetof(mcc_stats_t, nrx_aborts)},

  {"ntx_gets",      offsetof(mcc_stats_t, ntx_gets)},
  {"nrx_hits",      offsetof(mcc_stats_t, nrx_hits)},
  {"nrx_misses",    offsetof(mcc_stats_t, nrx_misses)},

  {"ntx_deletes",   offsetof(mcc_stats_t, ntx_deletes)},
  {"nrx_deletes",   offsetof(mcc_stats_t, nrx_deletes)},

  {"ntx_adds",      offsetof(mcc_stats_t, ntx_adds)},
  {"nrx_adds",      offsetof(mcc_stats_t, nrx_adds)},

  {"ntx_replaces",  offsetof(mcc_stats_t, ntx_replaces)},
  {"nrx_replaces",  offsetof(mcc_stats_t, nrx_replaces)},

  {"ntx_sets",      offsetof(mcc_stats_t, ntx_sets)},
  {"nrx_sets",      offsetof(mcc_stats_t, nrx_sets)},

  {"ntx_decrs",     offsetof(mcc_stats_t, ntx_decrs)},
  {"nrx_decrs",     offsetof(mcc_stats_t, nrx_decrs)},

  {"ntx_incrs",     offsetof(mcc_stats_t, ntx_incrs)},
  {"nrx_incrs",     offsetof(mcc_stats_t, nrx_incrs)},

  {"nrx_nostores",  offsetof(mcc_stats_t, nrx_nostores)},

  {"nrx_notfounds", offsetof(mcc_stats_t, nrx_notfounds)},

  {"nrx_failures",  offsetof(mcc_stats_t, nrx_failures)},

  {"ntx_flushalls", offsetof(mcc_stats_t, ntx_flushalls)},
  {"nrx_flushalls", offsetof(mcc_stats_t, nrx_flushalls)},

  {"nopens",        offsetof(mcc_stats_t, nopens)},
  {"ncloses",       offsetof(mcc_stats_t, ncloses)},
  {"nconn_tmos",    offsetof(mcc_stats_t, nconn_tmos)},
  {"nconn_errs",    offsetof(mcc_stats_t, nconn_errs)},

  {"rtt",           offsetof(mcc_stats_t, rtt)},

  {"nsys_errs",     offsetof(mcc_stats_t, nsys_errs)},
  {"napp_errs",     offsetof(mcc_stats_t, napp_errs)},
  {"nremote_errs",  offsetof(mcc_stats_t, nremote_errs)},
  {"nunknown_errs", offsetof(mcc_stats_t, nunknown_errs)},
};

static Array phpmcc_stats(MccResourcePtr &phpmcc, int clear) {
  mcc_stats_t stats;
  mcc_get_stats(phpmcc->m_mcc, &stats, clear);

  char* sp = (char*)&stats;
  Array retval;
  for (uint ix = 0; ix < sizeof(stat_map)/sizeof(stat_map_t); ix++) {
    retval.set(String(stat_map[ix].name, AttachLiteral),
               (int64)(*((size_t*)(sp+stat_map[ix].off))));
  }
  return retval;
}

///////////////////////////////////////////////////////////////////////////////
// MccMirrorMcc

MccMirrorMcc::MccMirrorMcc(CStrRef name, ConsistencyModel model,
                           MccResourcePtr &phpmcc)
  : m_name(name.data(), name.size()), m_model(model) {
  nstring_t nname;
  phpstring_to_nstring(nname, m_name);
  m_mcc = mcc_new_ex(&nname,
                     mcc_get_npoolprefix(phpmcc->m_mcc),
                     phpmcc->m_mtu,
                     phpmcc->m_rxdgram_max,
                     phpmcc->m_nodelay,
                     mcc_get_conn_tmo(phpmcc->m_mcc),
                     mcc_get_conn_ntries(phpmcc->m_mcc),
                     mcc_get_tmo(phpmcc->m_mcc),
                     mcc_get_dgram_ntries(phpmcc->m_mcc),
                     mcc_get_dgram_tmo_weight(phpmcc->m_mcc),
                     mcc_get_window_max(phpmcc->m_mcc),
                     mcc_get_server_retry_tmo(phpmcc->m_mcc),
                     mcc_get_dgram_tmo_threshold(phpmcc->m_mcc));
}

MccMirrorMcc::~MccMirrorMcc() {
  if (m_mcc) {
    mcc_del(m_mcc);
  }
}

///////////////////////////////////////////////////////////////////////////////
// MccResource

MccResourcePtr MccResource::GetPersistent(CStrRef name) {
  ResourceData *mcc = g_persistentObjects->get("php_mcc::resources",
                                               name.data());
  if (mcc) {
    s_mcc_extension->registerPersistentObject(name.data());
    return MccResourcePtr(dynamic_cast<MccResource*>(mcc));
  }
  return MccResourcePtr();
}

MccResource::MccResource(CStrRef name, bool persistent, int64 mtu,
                         int64 rxdgram_max, int64 nodelay)
  : m_name(name.data(), name.size()), m_persistent(persistent), m_mcc(NULL),
    m_compressed(0), m_nreqs(0),
    m_mtu(mtu), m_rxdgram_max(rxdgram_max), m_nodelay(nodelay),
    m_server_count(0), m_serverpool_count(0), m_handle_status(PHPMCC_NEW_HANDLE),
    m_fast_path_eligible(true), m_sitevar_version(0), m_proxy_ops(0) {
  struct timeval creation_time;
  if (gettimeofday(&creation_time, NULL) == 0){
    m_creation_time = creation_time.tv_sec;
  } else {
    m_creation_time = 0;
  }
  if (persistent) {
    g_persistentObjects->set("php_mcc::resources", name.c_str(), this);
    s_mcc_extension->registerPersistentObject(m_name);
  }
  m_zlib_compression_enabled = 1;
  m_nzlib_compression = 0;
  m_compression_threshold = PHPMCC_COMPRESSION_THRESHOLD_DEFAULT;

  m_fb_serialize_enabled = true;
  m_fb_serialize_available = true;
}

static void phpmcc_accesspoint_listener(void* context,
                                        const nstring_t* server,
                                        const nstring_t* host,
                                        const nstring_t* port,
                                        const int protocol,
                                        const mcc_presentation_protocol_t pres,
                                        const mcc_apstate_t event_id) {
  MccResource* resource = (MccResource*)context;
  MccApEvent event(string(server->str, server->len),
                   string(host->str, host->len),
                   string(port->str, port->len), protocol, pres, event_id);
  resource->m_events.push_back(event);
}

bool MccResource::init_mcc(int64 npoolprefix, int64 conn_tmo,
                           int64 conn_ntries, int64 tmo, int64 dgram_ntries,
                           double dgram_tmo_weight, int64 window_max,
                           int64 server_retry_tmo, int64 dgram_tmo_threshold) {
  nstring_t nname;
  phpstring_to_nstring(nname, m_name);
  m_mcc = mcc_new_ex(&nname, npoolprefix, m_mtu, m_rxdgram_max, m_nodelay,
                     conn_tmo, conn_ntries, tmo, dgram_ntries,
                     dgram_tmo_weight, window_max, server_retry_tmo,
                     dgram_tmo_threshold);
  if (!m_mcc) return false;
  mcc_add_accesspoint_listener(m_mcc, phpmcc_accesspoint_listener, this);
  return true;
}

MccResource::~MccResource() {
  for (MirrorIt mirror = m_mirror_mccs.begin();
       mirror != m_mirror_mccs.end(); ++mirror) {
    delete *mirror;
  }
  removeListeners();
  if (m_mcc) mcc_del(m_mcc);
}

void MccResource::sweep() {
  for (ListenerIt listener = m_error_listeners.begin();
       listener != m_error_listeners.end(); ++listener) {
    (*listener)->onSweep();
  }
  for (ListenerIt listener = m_server_listeners.begin();
       listener != m_server_listeners.end(); ++listener) {
    (*listener)->onSweep();
  }
  for (ListenerIt listener = m_ap_listeners.begin();
       listener != m_ap_listeners.end(); ++listener) {
    (*listener)->onSweep();
  }
  delete this;
}

void MccResource::removeListeners() {
  for (ListenerIt listener = m_error_listeners.begin();
       listener != m_error_listeners.end(); ++listener) {
    delete *listener;
  }
  for (ListenerIt listener = m_server_listeners.begin();
       listener != m_server_listeners.end(); ++listener) {
    delete *listener;
  }
  for (ListenerIt listener = m_ap_listeners.begin();
       listener != m_ap_listeners.end(); ++listener) {
    delete *listener;
  }
  m_error_listeners.clear();
  m_server_listeners.clear();
  m_ap_listeners.clear();
}

bool MccResource::close() {
  if (m_persistent) {
    g_persistentObjects->remove("php_mcc::resources", m_name.c_str());
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// properties

static void phpmcc_get_compression_threshold(MccResourcePtr &phpmcc,
                                             Variant& retval) {
  retval = (int64)phpmcc->m_compression_threshold;
}

static void phpmcc_set_compression_threshold(MccResourcePtr &phpmcc,
                                             CVarRef value) {
  if (value.isInteger()) {
    phpmcc->m_compression_threshold = (size_t)value.toInt64();
  } else if (value.is(KindOfBoolean)) {
    phpmcc->m_compression_threshold = value ? 1 : 0;
  }
}

static void phpmcc_get_nzlib_compression(MccResourcePtr &phpmcc,
                                         Variant& retval) {
  retval = (int64)phpmcc->m_nzlib_compression;
}

static void phpmcc_set_nzlib_compression(MccResourcePtr &phpmcc,
                                         CVarRef value) {
  if (value.isInteger()) {
    phpmcc->m_nzlib_compression = value;
  } else if (value.is(KindOfBoolean)) {
    phpmcc->m_nzlib_compression = value.toBoolean() ? 1 : 0 ;
  }
}

static void phpmcc_get_conn_tmo(MccResourcePtr &phpmcc, Variant& retval) {
  retval = (int64)mcc_get_conn_tmo(phpmcc->m_mcc);
}

static void phpmcc_set_conn_tmo(MccResourcePtr &phpmcc, CVarRef value) {
  if (value.isInteger()) {
    int64 tmo = value;

    mcc_set_conn_tmo(phpmcc->m_mcc, tmo);

    /* propagate to mirrors */
    for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
         mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
      mcc_set_conn_tmo((*mirror)->m_mcc, tmo);
    }
  }
}

static void phpmcc_get_conn_ntries(MccResourcePtr &phpmcc, Variant& retval) {
  retval = (int64)mcc_get_conn_ntries(phpmcc->m_mcc);
}

static void phpmcc_set_conn_ntries(MccResourcePtr &phpmcc, CVarRef value) {
  if (value.isInteger()) {
    int64 ntries = value;

    mcc_set_conn_ntries(phpmcc->m_mcc, ntries);

    /* propagate to mirrors */
    for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
         mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
      mcc_set_conn_ntries((*mirror)->m_mcc, ntries);
    }
  }
}

#if defined(HAVE_DEBUG_LOG)
static void phpmcc_get_debug(MccResourcePtr &phpmcc, Variant& retval) {
  retval = mcc_get_debug(phpmcc->m_mcc));
}

static void phpmcc_set_debug(MccResourcePtr &phpmcc, CVarRef value) {
  if (value.isInteger()) {
    mcc_set_debug(phpmcc->m_mcc, value);
  }
}

static void phpmcc_get_debug_logfile(MccResourcePtr &phpmcc, Variant& retval) {
  const nstring_t* result = mcc_get_debug_logfile(phpmcc->m_mcc);
  if (result != NULL) {
    retval = String(result->str, result->len, CopyString);
  } else {
    retval = null;
  }
}

static void phpmcc_set_debug_logfile(MccResourcePtr &phpmcc, CVarRef value) {
  nstring_t svalue;
  phpstring_to_nstring(svalue, value);
  mcc_set_debug_logfile(phpmcc->m_mcc, &svalue);
}
#endif /* defined(HAVE_DEBUG_LOG) */

static void phpmcc_get_delproxy(MccResourcePtr &phpmcc, Variant& retval) {
  const nstring_t* proxy;
  if ((proxy = mcc_get_delproxy(phpmcc->m_mcc)) == NULL) {
    retval = null;
  } else {
    Variant accesspoints;
    String prox(proxy->str, proxy->len, CopyString);
    accesspoints = get_accesspoints(phpmcc->m_mcc,
                                    proxy);
    if (!accesspoints.isNull()) {
      retval.set(prox, accesspoints);
    } else {
      retval.set(prox, null);
    }
  }
}

static void phpmcc_set_delproxy(MccResourcePtr &phpmcc, CVarRef value) {
  nstring_t host;
  nstring_t default_port =
    NSTRING_CONST(const_cast<char*>("PHPMCC_PROXY_PORT_DEFAULT"));
  nstring_t port;

  phpstring_to_nstring(host, value.toString());

  port.len = 0;
  port.str = (char*)memchr(host.str, ':', host.len);

  if (port.str != NULL) {
    port.str++;
    port.len = host.str + host.len - port.str;
    host.len-= port.len + 1;
  }
  if (port.len == 0) {
    port = default_port;
  }
  if (mcc_set_delproxy(phpmcc->m_mcc, &host, &port) != NULL) {
    phpmcc->m_proxy_ops = mcc_get_proxy_ops(phpmcc->m_mcc);
  } else {
    phpmcc->m_proxy_ops = 0;
  }
}

static void phpmcc_get_default_serverpool(MccResourcePtr &phpmcc,
                                          Variant& retval) {
  const nstring_t* name = mcc_get_default_serverpool(phpmcc->m_mcc);
  if (name != NULL) {
    retval = String(name->str, name->len, CopyString);
  } else {
    retval = null;
  }
}

static void phpmcc_set_default_serverpool(MccResourcePtr &phpmcc,
                                          CVarRef value) {
  nstring_t name;
  phpstring_to_nstring(name, value);

  mcc_set_default_serverpool(phpmcc->m_mcc, &name);
  for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
       mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
    mcc_set_default_serverpool((*mirror)->m_mcc, &name);
  }
}

static void phpmcc_get_dgram_ntries(MccResourcePtr &phpmcc, Variant& retval) {
  retval = (int64)mcc_get_dgram_ntries(phpmcc->m_mcc);
}

static void phpmcc_set_dgram_ntries(MccResourcePtr &phpmcc, CVarRef value) {
  if (value.isInteger()) {
    int64 ntries = value;

    mcc_set_dgram_ntries(phpmcc->m_mcc, ntries);

    /* propagate to mirrors */
    for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
         mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
      mcc_set_dgram_ntries((*mirror)->m_mcc, ntries);
    }
  }
}

static void phpmcc_get_dgram_tmo_weight(MccResourcePtr &phpmcc,
                                        Variant& retval) {
  retval = mcc_get_dgram_tmo_weight(phpmcc->m_mcc);
}

static void phpmcc_set_dgram_tmo_weight(MccResourcePtr &phpmcc,
                                        CVarRef value) {
  double tmo_weight = 0;

  if (value.is(KindOfDouble)) {
    tmo_weight = value.toDouble();
  } else if (value.isInteger()) {
    tmo_weight = (double)value.toInt64();
  } else {
    return;
  }

  mcc_set_dgram_tmo_weight(phpmcc->m_mcc, tmo_weight);
  /* propagate to mirrors */
  for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
       mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
    mcc_set_dgram_tmo_weight((*mirror)->m_mcc, tmo_weight);
  }
}

static void phpmcc_get_fb_serialize_enabled(MccResourcePtr &phpmcc,
                                            Variant& retval) {
  retval = phpmcc->m_fb_serialize_enabled;
}

static void phpmcc_set_fb_serialize_enabled(MccResourcePtr &phpmcc,
                                            CVarRef value) {
  if (value.is(KindOfBoolean)) {
    phpmcc->m_fb_serialize_enabled = value.toBoolean();
  } else if (value.isInteger()) {
    phpmcc->m_fb_serialize_enabled = value.toInt64() != 0;
  }
}

static void phpmcc_get_name(MccResourcePtr &phpmcc, Variant& retval) {
  retval = String(phpmcc->m_name);
}

static void phpmcc_get_nodelay(MccResourcePtr &phpmcc, Variant& retval) {
  retval = mcc_get_nodelay(phpmcc->m_mcc);
}

static void phpmcc_set_nodelay(MccResourcePtr &phpmcc, CVarRef value) {
  if (value.is(KindOfBoolean)) {
    mcc_set_nodelay(phpmcc->m_mcc, value.toBoolean());
  } else if (value.isInteger()) {
    mcc_set_nodelay(phpmcc->m_mcc, value.toInt64() != 0);
  }
}

static void phpmcc_get_persistent(MccResourcePtr &phpmcc,
                                  Variant& retval) {
  retval = phpmcc->m_persistent;
}

static void phpmcc_get_poll_tmo(MccResourcePtr &phpmcc,
                                Variant& retval) {
  retval = (int64)mcc_get_poll_tmo(phpmcc->m_mcc);
}

static void phpmcc_set_poll_tmo(MccResourcePtr &phpmcc, CVarRef value) {
  if (value.isInteger()) {
    mcc_set_poll_tmo(phpmcc->m_mcc, value.toInt64());
  }
}

static void phpmcc_get_proxy(MccResourcePtr &phpmcc, Variant& retval) {
  const nstring_t* proxy;

  if ((proxy = mcc_get_proxy(phpmcc->m_mcc)) == NULL) {
    retval = null;
  } else {
    Array accesspoints;
    accesspoints = get_accesspoints(phpmcc->m_mcc, proxy);
    String s(proxy->str, proxy->len, CopyString);
    if (!accesspoints.empty()) {
      retval.set(s, accesspoints);
    } else {
      retval.set(s, null);
    }
  }
}

static void phpmcc_set_proxy(MccResourcePtr &phpmcc, CVarRef value) {
  nstring_t host;
  nstring_t default_port =
    NSTRING_CONST(const_cast<char*>("PHPMCC_PROXY_PORT_DEFAULT"));
  nstring_t port;

  phpstring_to_nstring(host, value);

  port.len = 0;
  port.str = (char*)memchr(host.str, ':', host.len);

  if (port.str != NULL) {
    port.str++;
    port.len = host.str + host.len - port.str;
    host.len-= port.len + 1;
  }
  if (port.len == 0) {
    port = default_port;
  }
  if (mcc_set_proxy(phpmcc->m_mcc, &host, &port,
                    (mcc_proxy_op_t)(mcc_proxy_delete_op | mcc_proxy_arith_op))
      != NULL) {
    phpmcc->m_proxy_ops = mcc_get_proxy_ops(phpmcc->m_mcc);
  } else {
    phpmcc->m_proxy_ops = 0;
  }
}

static void phpmcc_get_proxy_ops(MccResourcePtr &phpmcc, Variant& retval) {
  retval = mcc_get_proxy_ops(phpmcc->m_mcc);
}

static void phpmcc_set_proxy_ops(MccResourcePtr &phpmcc, CVarRef value) {
  if (value.isInteger()) {
    phpmcc->m_proxy_ops = (mcc_proxy_op_t)value.toInt64();
    mcc_set_proxy_ops(phpmcc->m_mcc, (mcc_proxy_op_t)value.toInt64());
  }
}

static void phpmcc_get_server_retry_tmo(MccResourcePtr &phpmcc,
                                        Variant& retval) {
  retval = (int64)mcc_get_server_retry_tmo(phpmcc->m_mcc);
}

static void phpmcc_set_server_retry_tmo(MccResourcePtr &phpmcc,
                                        CVarRef value) {
  if (value.isInteger()) {
    int64 tmo = value.toInt64();
    mcc_set_server_retry_tmo(phpmcc->m_mcc, tmo);

    /* propagate to mirrors */
    for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
         mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
      mcc_set_server_retry_tmo((*mirror)->m_mcc, tmo);
    }
  }
}

static void phpmcc_get_dgram_tmo_threshold(MccResourcePtr &phpmcc,
                                           Variant& retval) {
  retval = (int64)mcc_get_dgram_tmo_threshold(phpmcc->m_mcc);
}

static void phpmcc_set_dgram_tmo_threshold(MccResourcePtr &phpmcc,
                                           CVarRef value) {
  if (value.isInteger()) {
    int64 tmo = value.toInt64();
    mcc_set_dgram_tmo_threshold(phpmcc->m_mcc, tmo);

    /* propagate to mirrors */
    for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
         mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
      mcc_set_dgram_tmo_threshold((*mirror)->m_mcc, tmo);
    }
  }
}

static void phpmcc_get_tcp_inactivity_time(MccResourcePtr &phpmcc,
                                           Variant& retval) {
  retval = (int64)mcc_get_tcp_inactivity_time(phpmcc->m_mcc);
}

static void phpmcc_set_tcp_inactivity_time(MccResourcePtr &phpmcc,
                                           CVarRef value) {
  if (value.isInteger()) {
    mcc_set_tcp_inactivity_time(phpmcc->m_mcc, value.toInt64());
  }
}

static void phpmcc_get_serverpools(MccResourcePtr &phpmcc, Variant& retval) {
  retval = get_serverpools(phpmcc->m_mcc, false);
}

static void phpmcc_get_tmo(MccResourcePtr &phpmcc, Variant& retval) {
  retval = (int64)mcc_get_tmo(phpmcc->m_mcc);
}

static void phpmcc_set_tmo(MccResourcePtr &phpmcc, CVarRef value) {
  if (value.isInteger()) {
    int64 tmo = value.toInt64();
    mcc_set_tmo(phpmcc->m_mcc, tmo);

    /* propagate to mirrors */
    for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
         mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
      mcc_set_tmo((*mirror)->m_mcc, tmo);
    }
  }
}

static void phpmcc_get_nphpreqs(MccResourcePtr &phpmcc, Variant& retval) {
  retval = (int64)phpmcc->m_nreqs;
}

static void phpmcc_get_udp_reply_ports(MccResourcePtr &phpmcc,
                                       Variant& retval) {
  retval = (int64)mcc_get_udp_reply_ports(phpmcc->m_mcc);
}

static void phpmcc_set_udp_reply_ports(MccResourcePtr &phpmcc,
                                       CVarRef value) {
  if (value.isInteger()) {
    int64 tmo = value.toInt64();
    mcc_set_udp_reply_ports(phpmcc->m_mcc, value.toInt64());

    /* propagate to mirrors */
    for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
         mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
      mcc_set_udp_reply_ports((*mirror)->m_mcc, tmo);
    }
  }
}

static void phpmcc_get_window_max(MccResourcePtr &phpmcc, Variant& retval) {
  retval = (int64)mcc_get_window_max(phpmcc->m_mcc);
}

static void phpmcc_set_window_max(MccResourcePtr &phpmcc, CVarRef value) {
  if (value.isInteger()) {
    int64 tmo = value.toInt64();
    mcc_set_window_max(phpmcc->m_mcc, value.toInt64());

    /* propagate to mirrors */
    for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
         mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
      mcc_set_window_max((*mirror)->m_mcc, tmo);
    }
  }
}

/* XXX deleteme once support for libch 2.0.0 is dropped. */
static void phpmcc_get_consistent_hashing_maximum_pool_size(MccResourcePtr &phpmcc,
                                                            Variant& retval) {
  retval = (int64)mcc_get_consistent_hashing_maximum_pool_size(2);
}

static void phpmcc_get_handle_status(MccResourcePtr &phpmcc, Variant& retval) {
  retval = (int64)phpmcc->m_handle_status;
}

static void phpmcc_set_handle_status(MccResourcePtr &phpmcc, CVarRef value) {
  if(value.isInteger()) {
    phpmcc->m_handle_status = (MccResource::HandleStatus)value.toInt64();
  }
}

static void phpmcc_get_fast_path_eligible(MccResourcePtr &phpmcc, Variant& retval) {
  retval = phpmcc->m_fast_path_eligible;
}

static void phpmcc_set_fast_path_eligible(MccResourcePtr &phpmcc, CVarRef value) {
  if(value.is(KindOfBoolean)) {
    phpmcc->m_fast_path_eligible = value.toBoolean();
  }
}

static void phpmcc_get_sitevar_version(MccResourcePtr &phpmcc, Variant& retval) {
  retval = phpmcc->m_sitevar_version;
}

static void phpmcc_set_sitevar_version(MccResourcePtr &phpmcc, CVarRef value) {
  if(value.isInteger()) {
    phpmcc->m_sitevar_version = value.toInt64();
  }
}

static void phpmcc_get_creation_time(MccResourcePtr &phpmcc, Variant& retval) {
  retval = (int64)phpmcc->m_creation_time;
}

static void phpmcc_get_args(MccResourcePtr &phpmcc, Variant& retval) {
  Array serverpools, mirror_cfg, consistent_hashing_prefixes;

  const nstring_t* name;

  serverpools = get_serverpools(phpmcc->m_mcc, false);

  if (!serverpools.empty()) {
    retval.set(PHPMCC_ARG_SERVERS, serverpools);
  }

  mirror_cfg = get_mirror_cfg(phpmcc);
  retval.set(PHPMCC_ARG_MIRROR_CFG, mirror_cfg);

  consistent_hashing_prefixes = get_consistent_hashing_prefixes(phpmcc);
  retval.set(PHPMCC_ARG_CONSISTENT_HASHING_PREFIXES,
             consistent_hashing_prefixes);

  retval.set(PHPMCC_ARG_COMPRESSION_THRESHOLD,
             (int64)phpmcc->m_compression_threshold);
  retval.set(PHPMCC_ARG_NZLIB_COMPRESSION, (int64)phpmcc->m_nzlib_compression);
  retval.set(PHPMCC_ARG_CONN_TMO, (int64)mcc_get_conn_tmo(phpmcc->m_mcc));
  retval.set(PHPMCC_ARG_CONN_NTRIES,
             (int64)mcc_get_conn_ntries(phpmcc->m_mcc));
  retval.set(PHPMCC_ARG_DGRAM_NTRIES,
             (int64)mcc_get_dgram_ntries(phpmcc->m_mcc));
  retval.set(PHPMCC_ARG_DGRAM_TMO_WEIGHT,
             mcc_get_dgram_tmo_weight(phpmcc->m_mcc));
  retval.set(PHPMCC_ARG_NODELAY, (bool)mcc_get_nodelay(phpmcc->m_mcc));
  retval.set(PHPMCC_ARG_TMO, (int64)mcc_get_tmo(phpmcc->m_mcc));
  retval.set(PHPMCC_ARG_TCP_INACTIVITY_TIME,
             (int64)mcc_get_tcp_inactivity_time(phpmcc->m_mcc));

  if ((name = mcc_get_default_serverpool(phpmcc->m_mcc)) != NULL) {
    retval.set(PHPMCC_ARG_DEFAULT_PREFIX,
               String(name->str, name->len, CopyString));
  }

  retval.set(PHPMCC_ARG_PERSISTENT, phpmcc->m_persistent);
  retval.set(PHPMCC_ARG_POLL_TMO, (int64)mcc_get_poll_tmo(phpmcc->m_mcc));

  retval.set(PHPMCC_ARG_NPOOLPREFIX, (int64)mcc_get_npoolprefix(phpmcc->m_mcc));

  if ((name = mcc_get_proxy(phpmcc->m_mcc)) != NULL) {
    size_t naccesspoint;
    mcc_proxy_op_t proxy_ops;
    const nstring_t* accesspoints;

    proxy_ops = mcc_get_proxy_ops(phpmcc->m_mcc);
    retval.set(PHPMCC_ARG_PROXY_OPS, proxy_ops);

    if ((accesspoints = mcc_server_get_accesspoints(phpmcc->m_mcc,
                                                    name,
                                                    &naccesspoint)) != NULL) {
      /* Verify there's at least one accesspoint, and verify it's a tcp ap.
         Strip off the 'tcp:' prefix it's not used for the delete-proxy */

      if (naccesspoint > 0 && memcmp(accesspoints[0].str, "tcp:", 4) == 0) {
        char* ap = accesspoints[0].str + 4;
        int aplen = accesspoints[0].len - 4;
        retval.set(PHPMCC_ARG_PROXY, String(ap, aplen, CopyString));
      }
      mcc_server_free_accesspoint_list(phpmcc->m_mcc, accesspoints,
                                       naccesspoint);
    }
  }

  retval.set(PHPMCC_ARG_FB_SERIALIZE_ENABLED,
             (bool)phpmcc->m_fb_serialize_enabled);

#if defined(HAVE_DEBUG_LOG)
  {
    const nstring_t* logfile = mcc_get_debug_logfile(phpmcc->m_mcc);
    retval.set(PHPMCC_ARG_DEBUG, mcc_get_debug(phpmcc->m_mcc));

    if (logfile != NULL && logfile->len > 0) {
      retval.set(PHPMCC_ARG_DEBUG_LOGFILE,
                 String(logfile->str, logfile->len, CopyString));

    } else {
      retval.set(PHPMCC_ARG_DEBUG_LOGFILE, null);
    }
  }
#endif /* defined(HAVE_DEBUG_LOG) */

  retval.set(PHPMCC_ARG_UDP_REPLY_PORTS,
             (int64)mcc_get_udp_reply_ports(phpmcc->m_mcc));
  retval.set(PHPMCC_ARG_WINDOW_MAX, (int64)mcc_get_window_max(phpmcc->m_mcc));
  retval.set(PHPMCC_ARG_SERVER_RETRY_TMO_MS,
             (int64)mcc_get_server_retry_tmo(phpmcc->m_mcc));
  retval.set(PHPMCC_ARG_DGRAM_TMO_THRESHOLD,
             (int64)mcc_get_dgram_tmo_threshold(phpmcc->m_mcc));
}

typedef struct member_accessor_s {
    nstring_t name;
    void (*getter)(MccResourcePtr&, Variant&);
    void (*setter)(MccResourcePtr&, CVarRef);
} member_accessor_t;


#define MEMBER_RO_ACCESSOR(public_name, getter)                         \
  {{(char*)public_name, sizeof(public_name) - 1},                       \
   getter, NULL},
#define MEMBER_RW_ACCESSOR(public_name, getter, setter)                 \
  {{(char*)public_name, sizeof(public_name) - 1},                       \
   getter, setter},

static member_accessor_t member_accessors[] = {
  MEMBER_RO_ACCESSOR("args", phpmcc_get_args)
  MEMBER_RW_ACCESSOR("conn_tmo", phpmcc_get_conn_tmo, phpmcc_set_conn_tmo)
  MEMBER_RW_ACCESSOR("conn_ntries", phpmcc_get_conn_ntries,
                     phpmcc_set_conn_ntries)
  MEMBER_RW_ACCESSOR("compression_threshold", phpmcc_get_compression_threshold,
                     phpmcc_set_compression_threshold)
  MEMBER_RW_ACCESSOR("nzlib_compression", phpmcc_get_nzlib_compression,
                     phpmcc_set_nzlib_compression)
  MEMBER_RW_ACCESSOR("default_serverpool", phpmcc_get_default_serverpool,
                     phpmcc_set_default_serverpool)
#if defined(HAVE_DEBUG_LOG)
  MEMBER_RW_ACCESSOR("debug", phpmcc_get_debug, phpmcc_set_debug)
  MEMBER_RW_ACCESSOR("debug_logfile", phpmcc_get_debug_logfile,
                     phpmcc_set_debug_logfile)
#endif /* defined(HAVE_DEBUG_LOG) */
  MEMBER_RW_ACCESSOR("delproxy", phpmcc_get_delproxy, phpmcc_set_delproxy)
  MEMBER_RW_ACCESSOR("dgram_ntries", phpmcc_get_dgram_ntries,
                     phpmcc_set_dgram_ntries)
  MEMBER_RW_ACCESSOR("dgram_tmo_weight", phpmcc_get_dgram_tmo_weight,
                     phpmcc_set_dgram_tmo_weight)
  MEMBER_RW_ACCESSOR("fb_serialize_enabled", phpmcc_get_fb_serialize_enabled,
                     phpmcc_set_fb_serialize_enabled)
  MEMBER_RO_ACCESSOR("name", phpmcc_get_name)
  MEMBER_RW_ACCESSOR("nodelay", phpmcc_get_nodelay, phpmcc_set_nodelay)
  MEMBER_RO_ACCESSOR("nphpreqs", phpmcc_get_nphpreqs)
  MEMBER_RO_ACCESSOR("persistent", phpmcc_get_persistent)
  MEMBER_RW_ACCESSOR("poll_tmo", phpmcc_get_poll_tmo, phpmcc_set_poll_tmo)
  MEMBER_RW_ACCESSOR("proxy", phpmcc_get_proxy, phpmcc_set_proxy)
  MEMBER_RW_ACCESSOR("proxy_ops", phpmcc_get_proxy_ops, phpmcc_set_proxy_ops)
  MEMBER_RW_ACCESSOR("server_retry_tmo", phpmcc_get_server_retry_tmo,
                     phpmcc_set_server_retry_tmo)
  MEMBER_RW_ACCESSOR("dgram_tmo_threshold", phpmcc_get_dgram_tmo_threshold,
                     phpmcc_set_dgram_tmo_threshold)
  MEMBER_RW_ACCESSOR("tcp_inactivity_time", phpmcc_get_tcp_inactivity_time,
                     phpmcc_set_tcp_inactivity_time)
  MEMBER_RO_ACCESSOR("serverpools", phpmcc_get_serverpools)
  MEMBER_RW_ACCESSOR("tmo", phpmcc_get_tmo, phpmcc_set_tmo)
  MEMBER_RW_ACCESSOR("udp_reply_ports", phpmcc_get_udp_reply_ports,
                     phpmcc_set_udp_reply_ports)
  MEMBER_RW_ACCESSOR("window_max", phpmcc_get_window_max, phpmcc_set_window_max)
  MEMBER_RO_ACCESSOR("consistent_hashing_maximum_pool_size",
                     phpmcc_get_consistent_hashing_maximum_pool_size)

  MEMBER_RW_ACCESSOR("handle_status",
                     phpmcc_get_handle_status,
                     phpmcc_set_handle_status)
  MEMBER_RW_ACCESSOR("fast_path_eligible",
                     phpmcc_get_fast_path_eligible,
                     phpmcc_set_fast_path_eligible)
  MEMBER_RW_ACCESSOR("sitevar_version",
                     phpmcc_get_sitevar_version,
                     phpmcc_set_sitevar_version)
  MEMBER_RO_ACCESSOR("creation_time",
                     phpmcc_get_creation_time)
};
#undef MEMBER_RO_ACCESSOR
#undef MEMBER_RW_ACCESSOR

static Variant phpmcc_read_property(MccResourcePtr &phpmcc, CVarRef member) {
  nstring_t member_name;
  member_accessor_t* accessor;
  int found = 0;
  Variant retval;

  if (!member.isString()) {
    goto epilogue;
  }
  if (!phpmcc.get()) {
    goto epilogue;
  }

  phpstring_to_nstring(member_name, member);

  for (accessor = &member_accessors[0];
       accessor < &member_accessors[sizeof(member_accessors)/
                                    sizeof(member_accessor_t)];
       accessor++) {
    if (nstring_cmp(&member_name, &accessor->name) == 0) {
      found = 1;
      break;
    }
  }

  if (!found || accessor->getter == NULL) {
    Logger::Error("Property %s of class mcc cannot be read", member_name.str);
    goto epilogue;
  }

  accessor->getter(phpmcc, retval);

 epilogue:
  return retval;
}

static void phpmcc_write_property(MccResourcePtr &phpmcc, CVarRef member,
                                  CVarRef value) {
  nstring_t member_name;
  member_accessor_t* accessor;
  int found = 0;

  if (!member.isString()) {
    return;
  }

  if (!phpmcc.get()) {
    return;
  }

  phpstring_to_nstring(member_name, member);

  for (accessor = &member_accessors[0];
       accessor < &member_accessors[sizeof(member_accessors)/
                                    sizeof(member_accessor_t)];
       accessor++) {
    if (nstring_cmp(&member_name, &accessor->name) == 0) {
      found = 1;
      break;
    }
  }

  if (!found || accessor->setter == NULL) {
    Logger::Error("Property %s of class mcc cannot be updated",
                  member_name.str);
    return;
  }

  accessor->setter(phpmcc, value);
}

///////////////////////////////////////////////////////////////////////////////
// methods

c_phpmcc::c_phpmcc() {
}
c_phpmcc::~c_phpmcc() {
}

void c_phpmcc::t___construct
(CStrRef name,
 bool persistent/* = true */,
 int64 npoolprefix/* = MCC_POOLPREFIX_LEN_DEFAULT */,
 int64 mtu/* = MCC_MTU_DEFAULT */,
 int64 rxdgram_max/* = MCC_RXDGRAM_MAX_DEFAULT */,
 int64 nodelay/* = MCC_NODELAY_DEFAULT */,
 int64 conn_tmo/* = MCC_CONN_TMO_MS_DEFAULT */,
 int64 conn_ntries/* = MCC_CONN_NTRIES_DEFAULT */,
 int64 tmo/* = MCC_TMO_MS_DEFAULT */,
 int64 dgram_ntries/* = MCC_DGRAM_NTRIES_DEFAULT */,
 double dgram_tmo_weight/* = MCC_DGRAM_TMO_WEIGHT_DEFAULT */,
 int64 server_retry_tmo/* = MCC_SERVER_RETRY_TMO_MS_DEFAULT */,
 int64 dgram_tmo_threshold/* = MCC_DGRAM_TMO_THRESHOLD_DEFAULT */,
 int64 window_max/* = MCC_WINDOW_MAX_DEFAULT */) {
  FUNCTION_INJECTION(phpmcc::__construct);

  bool success = false;
  MccResourcePtr mcc_r;
  if (persistent) {
    mcc_r = MccResource::GetPersistent(name);
    if (mcc_r.get() != NULL) {
      success = true;
    }
  }

  if (!success &&
      (mcc_r = MccResourcePtr(new MccResource
                              (name, persistent, mtu, rxdgram_max,
                               nodelay))).get() &&
      mcc_r->init_mcc(npoolprefix, conn_tmo, conn_ntries,
                      tmo, dgram_ntries, dgram_tmo_weight,
                      window_max, server_retry_tmo,
                      dgram_tmo_threshold)) {
    success = true;
  }

  if (success) {
    m_mcc = mcc_r;
  }
}

Variant c_phpmcc::t___destruct() {
  FUNCTION_INJECTION(phpmcc::__destruct);
  return null;
}

String c_phpmcc::t___tostring() {
  FUNCTION_INJECTION(phpmcc::__tostring);
  return m_mcc->m_name;
}

Variant c_phpmcc::t___set(Variant name, Variant val) {
  FUNCTION_INJECTION(phpmcc::__set);
  phpmcc_write_property(m_mcc, name, val);
  return null;
}

Variant c_phpmcc::t___get(Variant name) {
  FUNCTION_INJECTION(phpmcc::__get);
  return phpmcc_read_property(m_mcc, name);
}

bool c_phpmcc::t_close() {
  FUNCTION_INJECTION(phpmcc::close);
  return m_mcc->close();
}

bool c_phpmcc::t_del() {
  FUNCTION_INJECTION(phpmcc::del);
  return t_close();
}

int64 c_phpmcc::t_add_accesspoint(CStrRef server, CStrRef host,
                                  CStrRef port /* = 11211 */,
                                  int64 protocol /* = IPPROTO_TCP */) {
  FUNCTION_INJECTION(phpmcc::add_accesspoint);
  if (!m_mcc.get()) {
    return 0;
  }
  /* hmmm */
  nstring_t nserv, nhost, nport;
  String sport(port);
  phpstring_to_nstring(nserv, server);
  phpstring_to_nstring(nhost, host);
  phpstring_to_nstring(nport, sport);
  return (int64)mcc_add_accesspoint(m_mcc->m_mcc, &nserv, &nhost,
                                    &nport, protocol, MCC_ASCII_PROTOCOL);
}

void c_phpmcc::t_remove_accesspoint(CStrRef server, CStrRef host,
                                    CStrRef port /* = 11211 */,
                                    int64 protocol /* = IPPROTO_TCP */) {
  FUNCTION_INJECTION(phpmcc::remove_accesspoint);
  if (!m_mcc.get()) {
    return;
  }
  /* hmm */
  String sport(port);
  nstring_t nserv, nhost, nport;
  phpstring_to_nstring(nserv, server);
  phpstring_to_nstring(nhost, host);
  phpstring_to_nstring(nport, sport);
  mcc_remove_accesspoint(m_mcc->m_mcc, &nserv,
                         &nhost, &nport, protocol, MCC_ASCII_PROTOCOL);
}

Variant c_phpmcc::t_get_accesspoints(CStrRef server) {
  FUNCTION_INJECTION(phpmcc::get_accesspoint);
  if (!m_mcc.get()) {
    return null;
  }
  nstring_t nserv;
  phpstring_to_nstring(nserv, server);
  return get_accesspoints(m_mcc->m_mcc, &nserv);
}

Variant c_phpmcc::t_get_server(CStrRef server) {
  FUNCTION_INJECTION(phpmcc::get_server);
  return t_get_accesspoints(server);
}

Variant
c_phpmcc::t_add_mirror_accesspoint(CStrRef mirrorname,
                                   CStrRef server,
                                   CStrRef host,
                                   CStrRef port /* = 11211 */,
                                   int64 protocol /* = IPPROTO_TCP */) {
  FUNCTION_INJECTION(phpmcc::add_mirror_accesspoint);

  if (!m_mcc.get()) {
    return null;
  }

  // phpmcc_addremove_mirror_accesspoint_helper returns 0 on error, but we're
  // returning 0 on success.
  return (int64)(0 ==
                 phpmcc_addremove_mirror_accesspoint_helper(m_mcc,
                                                            mirrorname,
                                                            server,
                                                            host,
                                                            port,
                                                            protocol, true));
}

void
c_phpmcc::t_remove_mirror_accesspoint(CStrRef mirrorname,
                                      CStrRef server,
                                      CStrRef host,
                                      CStrRef port /* = 11211 */,
                                      int64 protocol /* = IPPROTO_TCP */) {
  FUNCTION_INJECTION(phpmcc::remove_mirror_accesspoint);
  if (m_mcc.get()) {
    phpmcc_addremove_mirror_accesspoint_helper
      (m_mcc, mirrorname, server, host, port, protocol, false);
  }
}

int64 c_phpmcc::t_add_server(CStrRef server, CStrRef mirror) {
  FUNCTION_INJECTION(phpmcc::add_server);
  String serverToAdd = server;
  if (RuntimeOption::LocalMemcache) {
    serverToAdd = "127.0.0.1";
  }
  if (!m_mcc.get()) {
    return 0;
  }
  return phpmcc_addremove_server_helper(m_mcc, serverToAdd, mirror, true);
}

void c_phpmcc::t_remove_server(CStrRef server, CStrRef mirror) {
  FUNCTION_INJECTION(phpmcc::remove_server);
  if (m_mcc.get()) {
    phpmcc_addremove_server_helper(m_mcc, server, mirror, false);
  }
}

bool c_phpmcc::t_server_flush(CStrRef server, int64 exptime /* = 0 */) {
  FUNCTION_INJECTION(phpmcc::server_flush);
  if (!m_mcc.get()) {
    return false;
  }
  nstring_t nserv;
  phpstring_to_nstring(nserv, server);
  int64 result = mcc_server_flush(m_mcc->m_mcc, &nserv, exptime);
  return result == 1;
}

Variant c_phpmcc::t_server_version(CStrRef server /* = "" */) {
  FUNCTION_INJECTION(phpmcc::server_version);
  const nstring_t* version;

  if (!m_mcc.get()) {
    return null;
  }
  nstring_t nserv;
  phpstring_to_nstring(nserv, server);
  version = mcc_server_get_version(m_mcc->m_mcc, &nserv);
  Variant return_value;
  if (version != NULL) {
    return_value = String(version->str, version->len, CopyString);
    mcc_free_version(m_mcc->m_mcc, version);
  } else {
    return_value = null;
  }
  return return_value;
}

bool c_phpmcc::t_server_is_alive(CStrRef server /* = "" */) {
  FUNCTION_INJECTION(phpmcc::server_is_alive);
  const nstring_t* version;

  if (!m_mcc.get()) {
    return null;
  }
  nstring_t nserv;
  phpstring_to_nstring(nserv, server);
  version = mcc_server_get_version(m_mcc->m_mcc, &nserv);

  bool return_value = version != NULL;

  if (version != NULL) {
    mcc_free_version(m_mcc->m_mcc, version);
  }
  return return_value;
}

bool c_phpmcc::t_test_proxy(CStrRef server /* = "" */) {
  FUNCTION_INJECTION(phpmcc::test_proxy);
  return t_server_is_alive(server);
}

Variant c_phpmcc::t_add_mirror(CStrRef mirrorname, int64 model) {
  FUNCTION_INJECTION(phpmcc::add_mirror);
  if (!m_mcc.get()) {
    return null;
  }
  return phpmcc_add_mirror_helper(m_mcc, mirrorname,
                                  (MccMirrorMcc::ConsistencyModel)model);
}

Variant c_phpmcc::t_remove_mirror(CStrRef mirrorname) {
  FUNCTION_INJECTION(phpmcc::remove_mirror);
  if (!m_mcc.get()) {
    return null;
  }
  return phpmcc_remove_mirror_helper(m_mcc, mirrorname);
}

Variant c_phpmcc::t_add_serverpool(CStrRef serverpool,
                                   bool consistent_hashing_enabled /* = 0 */) {
  FUNCTION_INJECTION(phpmcc::add_serverpool);
  if (!m_mcc.get()) {
    return null;
  }
  return phpmcc_add_serverpool_helper(m_mcc, serverpool,
                                      consistent_hashing_enabled, 2);
}

Variant c_phpmcc::t_add_serverpool_ex(CStrRef serverpool,
                                      int64 consistent_hashing_flag) {
  FUNCTION_INJECTION(phpmcc::add_serverpool);
  if (!m_mcc.get()) {
    return null;
  }
  return phpmcc_add_serverpool_helper(m_mcc, serverpool,
                                      consistent_hashing_flag != 0,
                                      consistent_hashing_flag + 1);
}

bool c_phpmcc::t_add_accesspoint_listener(CStrRef function, Variant context) {
  FUNCTION_INJECTION(phpmcc::add_accesspoint_listener);
  return zim_phpmcc_add_listener(m_mcc, function, context,
                                 MccListener::AccessPointListener);
}

bool c_phpmcc::t_remove_accesspoint_listener(CStrRef function, Variant context) {
  FUNCTION_INJECTION(phpmcc::remove_accesspoint_listener);
  return zim_phpmcc_remove_listener(m_mcc, function, context,
                                    MccListener::AccessPointListener);
}

bool c_phpmcc::t_add_server_listener(CStrRef function, Variant context) {
  FUNCTION_INJECTION(phpmcc::add_server_listener);
  return zim_phpmcc_add_listener(m_mcc, function, context,
                                 MccListener::ServerListener);
}

bool c_phpmcc::t_remove_server_listener(CStrRef function, Variant context) {
  FUNCTION_INJECTION(phpmcc::remove_server_listener);
  return zim_phpmcc_remove_listener(m_mcc, function, context,
                                    MccListener::ServerListener);
}

bool c_phpmcc::t_add_error_listener(CStrRef function, Variant context) {
  FUNCTION_INJECTION(phpmcc::add_error_listener);
  return zim_phpmcc_add_listener(m_mcc, function, context,
                                 MccListener::ErrorListener);
}

bool c_phpmcc::t_remove_error_listener(CStrRef function, Variant context) {
  FUNCTION_INJECTION(phpmcc::remove_error_listener);
  return zim_phpmcc_remove_listener(m_mcc, function, context,
                                    MccListener::ErrorListener);
}

void c_phpmcc::t_remove_serverpool(CStrRef serverpool) {
  FUNCTION_INJECTION(phpmcc::remove_serverpool);
  if (!m_mcc.get()) {
    return;
  }
  phpmcc_remove_serverpool_helper(m_mcc, serverpool);
}

Variant c_phpmcc::t_get_server_by_key(CStrRef key) {
  FUNCTION_INJECTION(phpmcc::get_server_by_key);
  if (!m_mcc.get()) {
    return null;
  }

  const nstring_t* server;

  nstring_t nkey;
  phpstring_to_nstring(nkey, key);
  if ((server = mcc_get_server_by_key(m_mcc->m_mcc, &nkey)) != NULL) {
    return String(server->str, server->len, CopyString);
  }
  return null;
}

Variant c_phpmcc::t_get_host(CStrRef key) {
  FUNCTION_INJECTION(phpmcc::get_host);
  return t_get_server_by_key(key);
}

Variant c_phpmcc::t_get_serverpool_by_key(CStrRef key) {
  FUNCTION_INJECTION(phpmcc::get_serverpool_by_key);
  if (!m_mcc.get()) {
    return null;
  }

  const nstring_t* serverpool;
  nstring_t nkey;
  phpstring_to_nstring(nkey, key);
  if ((serverpool = mcc_get_serverpool_by_key(m_mcc->m_mcc, &nkey)) != NULL) {
    return String(serverpool->str, serverpool->len, CopyString);
  }
  return null;
}

Variant c_phpmcc::t_serverpool_add_server(CStrRef serverpool,
                                          CStrRef server,
                                          CStrRef mirrorname /* = "" */) {
  FUNCTION_INJECTION(phpmcc::get_serverpool_add_server);
  String serverToAdd = server;
  if (RuntimeOption::LocalMemcache) {
    serverToAdd = "127.0.0.1";
  }

  if (!m_mcc.get()) {
    return null;
  }
  return phpmcc_serverpool_addremove_server_helper(m_mcc, serverpool,
                                                   serverToAdd, mirrorname,
                                                   true);
}

Variant c_phpmcc::t_serverpool_remove_server(CStrRef serverpool,
                                             CStrRef server,
                                             CStrRef mirrorname /* = "" */) {
  FUNCTION_INJECTION(phpmcc::get_serverpool_remove_server);
  if (!m_mcc.get()) {
    return null;
  }
  return phpmcc_serverpool_addremove_server_helper(m_mcc, serverpool,
                                                   server, mirrorname, false);
}

Variant c_phpmcc::t_serverpool_get_servers(CStrRef serverpool) {
  FUNCTION_INJECTION(phpmcc::get_serverpool_get_servers);
  if (!m_mcc.get()) {
    return null;
  }
  nstring_t nsp;
  phpstring_to_nstring(nsp, serverpool);
  return get_serverpool_servers(m_mcc->m_mcc, &nsp, false);
}

Variant
c_phpmcc::t_serverpool_get_consistent_hashing_enabled(CStrRef serverpool) {
  FUNCTION_INJECTION(phpmcc::get_serverpool_get_consistent_hashing_enabled);
  if (!m_mcc.get()) {
    return null;
  }
  nstring_t nsp;
  phpstring_to_nstring(nsp, serverpool);
  return (bool)mcc_serverpool_get_consistent_hashing_enabled(m_mcc->m_mcc,
                                                             &nsp);
}

Variant
c_phpmcc::t_serverpool_get_consistent_hashing_version(CStrRef serverpool) {
  FUNCTION_INJECTION(phpmcc::get_serverpool_get_consistent_hashing_enabled);
  if (!m_mcc.get()) {
    return null;
  }
  nstring_t nsp;
  phpstring_to_nstring(nsp, serverpool);
  return (int64)mcc_serverpool_get_consistent_hashing_version(m_mcc->m_mcc,
                                                              &nsp);
}

Variant c_phpmcc::t_multi_add(CArrRef keys_values,
                              int64 exptime /* = 0 */,
                              int64 compress /* = 1 */,
                              int64 proxy_replicate /* = 0 */,
                              int64 async_set /* = 0 */) {
  FUNCTION_INJECTION(phpmcc::multi_add);
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.madd", 1);
    ServerStats::Log("mcc.madd.count", keys_values.size());
  }
  return zim_phpmcc_multi_update(m_mcc, keys_values, exptime, compress,
                                 proxy_replicate, async_set, mcc_add_cmd);
}

Variant c_phpmcc::t_multi_replace(CArrRef keys_values,
                                  int64 exptime /* = 0 */,
                                  int64 compress /* = 1 */,
                                  int64 proxy_replicate /* = 0 */,
                                  int64 async_set /* = 0 */) {
  FUNCTION_INJECTION(phpmcc::multi_replace);
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.mreplace", 1);
    ServerStats::Log("mcc.mreplace.count", keys_values.size());
  }
  return zim_phpmcc_multi_update(m_mcc, keys_values, exptime, compress,
                                 proxy_replicate, async_set, mcc_replace_cmd);
}

Variant c_phpmcc::t_multi_set(CArrRef keys_values,
                              int64 exptime /* = 0 */,
                              int64 compress /* = 1 */,
                              int64 proxy_replicate /* = 0 */,
                              int64 async_set /* = 0 */) {
  FUNCTION_INJECTION(phpmcc::multi_set);
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.set", 1);
  }
  return zim_phpmcc_multi_update(m_mcc, keys_values, exptime, compress,
                                 proxy_replicate, async_set, mcc_set_cmd);
}

Variant c_phpmcc::t_add(CVarRef key,
                        CVarRef value,
                        int64 exptime /* = 0 */,
                        bool compress /* = 1 */,
                        int64 proxy_replicate /* = 0 */,
                        int64 async_set /* = 0 */) {
  FUNCTION_INJECTION(phpmcc::add);
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.add", 1);
  }
  return zim_phpmcc_update(m_mcc, key, value, exptime,
                           compress, proxy_replicate, async_set,
                           mcc_add_cmd);
}

Variant c_phpmcc::t_decr(CStrRef key, int64 value /* = 1 */) {
  FUNCTION_INJECTION(phpmcc::decr);
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.decr", 1);
  }
  return zim_phpmcc_arith_update(m_mcc, key, value, mcc_decr_cmd);
}

Variant c_phpmcc::t_incr(CStrRef key, int64 value /* = 1 */) {
  FUNCTION_INJECTION(phpmcc::incr);
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.incr", 1);
  }
  return zim_phpmcc_arith_update(m_mcc, key, value, mcc_incr_cmd);
}

Variant c_phpmcc::t_delete(CVarRef keys, int64 exptime /* = 0 */) {
  FUNCTION_INJECTION(phpmcc::delete);
  if (!m_mcc.get()) {
    return null;
  }
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.delete", 1);
  }
  return phpmcc_delete(m_mcc, keys, exptime, false);
}

Variant c_phpmcc::t_delete_details(CVarRef keys, int64 exptime /* = 0 */) {
  FUNCTION_INJECTION(phpmcc::delete_details);
  if (!m_mcc.get()) {
    return null;
  }
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.delete_details", 1);
  }
  return phpmcc_delete(m_mcc, keys, exptime, true);
}

Variant c_phpmcc::t_get(CVarRef keys, int64 detailed_info_mode /* = 0 */,
                        Variant detailed_info /* = null */) {
  FUNCTION_INJECTION(phpmcc::get);
  if (!m_mcc.get()) {
    return null;
  }
  if (detailed_info_mode == 0) detailed_info_mode = PHPMCC_GET_DEFAULT;
  Variant return_value;
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.get", 1);
  }
  phpmcc_get(m_mcc, return_value, keys,
             (phpmcc_get_details_t)detailed_info_mode, detailed_info);
  return return_value;
}

Variant c_phpmcc::t_get_multi(CVarRef keys, int64 detailed_info_mode /* = 0 */,
                              Variant detailed_info /* = null */) {
  FUNCTION_INJECTION(phpmcc::get_multi);
  if (!m_mcc.get()) {
    return null;
  }
  if (detailed_info_mode == 0) detailed_info_mode = PHPMCC_GET_DEFAULT;
  Variant return_value;
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.mget", 1);
    ServerStats::Log("mcc.mget.count", keys.toArray().size());
  }
  phpmcc_get(m_mcc, return_value, keys,
             (phpmcc_get_details_t)detailed_info_mode, detailed_info);
  return return_value;
}

Variant c_phpmcc::t_replace(CVarRef key,
                            CVarRef value,
                            int64 exptime /* = 0 */,
                            bool compress /* = 1 */,
                            int64 proxy_replicate /* = 0 */,
                            int64 async_set /* = 0 */) {
  FUNCTION_INJECTION(phpmcc::replace);
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.replace", 1);
  }
  return zim_phpmcc_update(m_mcc, key, value, exptime,
                           compress, proxy_replicate, async_set,
                           mcc_replace_cmd);
}

Variant c_phpmcc::t_set(CVarRef key,
                        CVarRef value,
                        int64 exptime /* = 0 */,
                        bool compress /* = 1 */,
                        int64 proxy_replicate /* = 0 */,
                        int64 async_set /* = 0 */) {
  FUNCTION_INJECTION(phpmcc::set);
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.set", 1);
  }
  return zim_phpmcc_update(m_mcc, key, value, exptime,
                           compress, proxy_replicate, async_set,
                           mcc_set_cmd);
}

Variant c_phpmcc::t_stats(int64 clear /* = 0 */) {
  FUNCTION_INJECTION(phpmcc::stats);
  if (!m_mcc.get()) {
    return null;
  }
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.stats", 1);
  }
  return phpmcc_stats(m_mcc, clear);
}

///////////////////////////////////////////////////////////////////////////////
}
