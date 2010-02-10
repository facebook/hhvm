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
#include "constants.h"
#include "types.h"

using namespace HPHP;

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// properties

///////////////////////////////////////////////////////////////////////////////
// servers

Array get_accesspoints(mcc_handle_t mcc, const nstring_t* server) {
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

Array get_serverpool_servers(mcc_handle_t mcc,
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

Variant phpmcc_read_property(MccResourcePtr &phpmcc, CVarRef member) {
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

void phpmcc_write_property(MccResourcePtr &phpmcc, CVarRef member,
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
}
