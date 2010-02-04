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
#include <zlib.h>
#include <cpp/base/runtime_option.h>
#include <cpp/base/util/request_local.h>
#include <cpp/base/server/server_stats.h>
#include <sys/time.h>
#include <time.h>

#include "ext_php_mcc.h"
#include "accessors.h"
#include "constants.h"
#include "types.h"

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

void phpmcc_log(MccResourcePtr &phpmcc, const mcc_errtype_t type,
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

int mcc_log_if_error(MccResourcePtr &phpmcc) {
  const mcc_err_t* err = mcc_get_last_err(phpmcc->m_mcc);
  if (err == NULL) {
    return 0;
  }

  Array errlist = phpmcc_errlist_to_phparray(phpmcc->m_mcc);
  phpmcc_call_error_listeners(phpmcc, errlist);
  return 1;
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

void phpmcc_apevent_dispatcher(MccResourcePtr &phpmcc) {
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
