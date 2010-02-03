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

#ifndef __EXT_PHP_MCC_RESOURCE_H__
#define __EXT_PHP_MCC_RESOURCE_H__

#include <util/base.h>

#define HAVE_UDP_REPLY_PORTS 1
#include <mcc/mcc.h>

namespace HPHP {

class MccResource;
typedef SmartPtr<MccResource> MccResourcePtr;
///////////////////////////////////////////////////////////////////////////////
// MccListener

class MccListener {
public:
  enum ListenerType {
    ErrorListener  = 1,
    AccessPointListener = 2,
    ServerListener = 3
  };

public:
  MccListener(ListenerType type, CStrRef function, CVarRef context)
    : m_type(type), m_function(function) {
    m_context = ref(context);
  }

  ListenerType m_type;
  String m_function;
  Variant m_context;

  void onSweep() {
    m_function.reset();
    m_context.reset();
  }
};

///////////////////////////////////////////////////////////////////////////////
// MccApEvent

class MccApEvent {
public:
  MccApEvent(const std::string &server, const std::string &host,
             const std::string &port, int protocol,
             mcc_presentation_protocol_t presentation_pro,
             mcc_apstate_t id)
    : m_server(server), m_host(host), m_port(port), m_protocol(protocol),
      m_presentation_protocol(presentation_pro), m_id(id) {}
  std::string m_server;
  std::string m_host;
  std::string m_port;
  int m_protocol;
  mcc_presentation_protocol_t m_presentation_protocol;
  mcc_apstate_t m_id;
};

///////////////////////////////////////////////////////////////////////////////
// MccMirrorMcc

class MccMirrorMcc {
public:
  enum ConsistencyModel {
    /* return values from this mcc are ignored. */
    CONSISTENCY_IGNORE,

    /* return values from this mcc must match the return value from the
     * primary mcc.  if the primary mcc hits, this should hit as well.  if
     * the primary mcc misses, this should miss as well.  if there is a
     * mismatch, flag a warning and return a miss. */
    CONSISTENCY_MATCH_ALL,

    /* return values from this mcc are compared with the return values from
     * the primary mcc only when both hit. if there is a mismatch, flag a
     * warning and return a miss. */
    CONSISTENCY_MATCH_HITS,

    /* return values from this mcc are compared with the return values from
     * the primary mcc only when both hit. if there is a mismatch, flag a
     * warning and return a miss.  if the primary mcc misses and this mcc
     * hits, then this result supercedes the primary mcc's miss. */
    CONSISTENCY_MATCH_HITS_SUPERCEDES,
  };

public:
  MccMirrorMcc(CStrRef name, ConsistencyModel model, MccResourcePtr &mcc);
  ~MccMirrorMcc();

  std::string m_name;
  ConsistencyModel m_model;
  mcc_handle_t m_mcc;
};

///////////////////////////////////////////////////////////////////////////////
// MccResource

class MccResource : public SweepableResourceData {
public:
  static MccResourcePtr GetPersistent(CStrRef name);

public:
  MccResource(CStrRef name, bool persistent, int64 mtu,
              int64 rxdgram_max, int64 nodelay);
  ~MccResource();

  bool init_mcc(int64 npoolprefix, int64 conn_tmo, int64 conn_ntries,
                int64 tmo, int64 dgram_ntries, double dgram_tmo_weight,
                int64 window_max, int64 server_retry_tmo,
                int64 dgram_tmo_threshold);

  void removeListeners();
  bool close();

  // overriding ResourceData
  const char *o_getClassName() const { return "MccResource";}

  // overriding Sweepable
  virtual void sweep();

public:
  enum HandleStatus {
    PHPMCC_NEW_HANDLE,
    PHPMCC_USED_FAST_PATH,
    PHPMCC_USED_SLOW_PATH,
  };

  std::string m_name;
  bool m_persistent;
  mcc_handle_t m_mcc;

  int m_compressed;
  size_t m_nreqs;

  bool m_fb_serialize_available;
  bool m_fb_serialize_enabled;

  int m_zlib_compression_enabled;
  int m_nzlib_compression;
  size_t m_compression_threshold;

  std::deque<MccListener*> m_error_listeners;
  std::deque<MccListener*> m_server_listeners;
  std::deque<MccListener*> m_ap_listeners;

  std::deque<MccApEvent> m_events;
  std::deque<MccMirrorMcc*> m_mirror_mccs;

  /* these are not stored in the mcc object itself, or have no accessors, so
   * we have to copy them locally. */
  int64 m_mtu;
  int64 m_rxdgram_max;
  int32 m_nodelay;

  /* variables used by mirroring code: counts of mirrors and serverpools,
   * whether or not a proxy has been set. */
  uint32 m_server_count;
  uint32 m_serverpool_count;
  HandleStatus m_handle_status;
  bool m_fast_path_eligible;         /* if this handle was created with
                                    * different parameters, we have to
                                    * always do the full comparison. */
  int64 m_sitevar_version;           /* the sitevar version this handle was
                                    * created with */
  time_t m_creation_time;            /* time that this handle was created. */
  int64 m_proxy_ops;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_PHP_MCC_RESOURCE_H__
