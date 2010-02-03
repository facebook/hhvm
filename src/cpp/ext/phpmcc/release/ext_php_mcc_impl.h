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

#ifndef __EXT_PHP_MCC_IMPL_H__
#define __EXT_PHP_MCC_IMPL_H__

#include <util/base.h>

#include "ext_php_mcc.h"
#include "ext_php_mcc_resource.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// defines

#define PHPMCC_RESOURCE_HANDLE_NAME "phpmcc"
#define PHPMCC_ARG_COMPRESSION_THRESHOLD "compression_threshold"
#define PHPMCC_ARG_CONN_TMO "conn_tmo"
#define PHPMCC_ARG_CONN_NTRIES "conn_ntries"
#define PHPMCC_ARG_DEBUG "debug"
#define PHPMCC_ARG_DEBUG_LOGFILE "debug_logfile"
#define PHPMCC_ARG_DEFAULT_PREFIX "default_prefix"
#define PHPMCC_ARG_DELETE_PROXY "delete_proxy"
#define PHPMCC_ARG_DGRAM_TMO "dgram_tmo"
#define PHPMCC_ARG_DGRAM_NTRIES "dgram_ntries"
#define PHPMCC_ARG_DGRAM_TMO_WEIGHT "dgram_tmo_weight"
#define PHPMCC_ARG_FB_SERIALIZE_ENABLED "fb_serialize_enabled"
#define PHPMCC_ARG_CONSISTENT_HASHING_PREFIXES "consistent_hashing_prefixes"
#define PHPMCC_ARG_NODELAY "nodelay"
#define PHPMCC_ARG_PERSISTENT "persistent"
#define PHPMCC_ARG_POLL_TMO "poll_tmo"
#define PHPMCC_ARG_PROXY "proxy"
#define PHPMCC_ARG_PROXY_OPS "proxy_ops"
#define PHPMCC_ARG_SERVERS "servers"
#define PHPMCC_ARG_MIRROR_CFG "mirror_cfg"
#define PHPMCC_ARG_MIRROR_CFG_NAME "name"
#define PHPMCC_ARG_MIRROR_CFG_MODEL "model"
#define PHPMCC_ARG_MIRROR_CFG_SERVERPOOLS "serverpools"
#define PHPMCC_ARG_TMO "tmo"
#define PHPMCC_ARG_TCP_INACTIVITY_TIME "tcp_inactivity_time"
#define PHPMCC_ARG_UDP_REPLY_PORTS "udp_reply_ports"
#define PHPMCC_ARG_WINDOW_MAX "window_max"
#define PHPMCC_ARG_NZLIB_COMPRESSION "nzlib_compression"
#define PHPMCC_ARG_SERVER_RETRY_TMO_MS "server_retry_tmo"
#define PHPMCC_ARG_DGRAM_TMO_THRESHOLD "dgram_tmo_threshold"
#define PHPMCC_ARG_NPOOLPREFIX "npoolprefix"

#define PHPMCC_COMPRESSION_THRESHOLD_DEFAULT 100 * 1024

#define PHPMCC_ERR_UNCOMPRESS 0x1001
#define PHPMCC_ERR_UNSERIALIZE 0x1002
#define PHPMCC_ERR_FB_UNSERIALIZE 0x1003
#define PHPMCC_ERR_BAD_VALUE 0x1004
#define PHPMCC_ERR_MIRROR_CREATE 0x1010
#define PHPMCC_ERR_MIRROR_EXISTS 0x1011
#define PHPMCC_ERR_MIRROR_NOT_FOUND 0x1012
#define PHPMCC_ERR_MIRROR_CONSISTENCY 0x1013
#define PHPMCC_ERR_MISSINGS_REQS 0x1014
#define PHPMCC_ERR_DETAILED_BAD_ARG 0x1015

///////////////////////////////////////////////////////////////////////////////
// types

enum phpmcc_flags_t {
  phpmcc_serialized = 0x1,
  phpmcc_compressed = 0x2,
  phpmcc_fb_serialized = 0x4,
  phpmcc_proxy_replicate = 0x400,
  phpmcc_nzlib_compressed = 0x800,
  phpmcc_async_set = 0x1000,
};

enum phpmcc_get_details_t {
  PHPMCC_GET_DEFAULT,                 /* default mode of operation. */
  PHPMCC_GET_RECORD_ERRORS,           /* record the keys that caused errors */
};

enum phpmcc_delete_details_t {
  PHPMCC_DELETE_DELETED,              /* deleted from cache */
  PHPMCC_DELETE_NOTFOUND,             /* not found in cache */
  PHPMCC_DELETE_ERROR_LOG,            /* delete error should be logged */
  PHPMCC_DELETE_ERROR_NOLOG,          /* delete error should not be logged */
};

typedef std::deque<MccListener*>::iterator ListenerIt;
typedef std::deque<MccApEvent>::iterator ApEventIt;
typedef std::deque<MccMirrorMcc*>::iterator MirrorIt;

/*
  The context structure used to communicate between phpmcc_get(..) and the
  callback functions.
*/
typedef struct phpmcc_get_processor_context_s {
  Variant* results;
  Variant* additional_context;
  int hits;
  int errors;
} phpmcc_get_processor_context_t;

/*
  The type definition for the callbacks and the actual callback functions.
*/
typedef void (*phpmcc_get_processor_funcptr_t)
  (const nstring_t search_key,
   mcc_res_t final_result,
   CVarRef result,
   phpmcc_get_processor_context_t* context);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_PHP_MCC_IMPL_H__
