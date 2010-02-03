<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

p(
<<<END
#include "ext_php_mcc_resource.h"
END
);

k("MCC_IPPROTO_TCP", Int64);
k("MCC_IPPROTO_UDP", Int64);
k("MCC_SERVER_UP", Int64);
k("MCC_SERVER_DOWN", Int64);
k("MCC_SERVER_DISABLED", Int64);
k("MCC_SERVER_RETRY_TMO_MS", Int64);
k("MCC_DGRAM_TMO_THRESHOLD", Int64);
k("MCC_PORT_DEFAULT", Int64);
k("MCC_POOLPREFIX_LEN", Int64);
k("MCC_MTU", Int64);
k("MCC_RXDGRAM_MAX", Int64);
k("MCC_CONN_TMO_MS", Int64);
k("MCC_CONN_NTRIES", Int64);
k("MCC_DGRAM_NTRIES", Int64);
k("MCC_DGRAM_TMO_WEIGHT", Double);
k("MCC_NODELAY", Int64);
k("MCC_POLL_TMO_US", Int64);
k("MCC_PROXY_DELETE_OP", Int64);
k("MCC_PROXY_UPDATE_OP", Int64);
k("MCC_PROXY_ARITH_OP", Int64);
k("MCC_PROXY_GET_OP", Int64);
k("MCC_TMO_MS", Int64);
k("MCC_UDP_REPLY_PORTS", Int64);
k("MCC_WINDOW_MAX", Int64);
k("MCC_HAVE_FB_SERIALIZATION", Int64);
k("MCC_ARG_FB_SERIALIZE_ENABLED", String);
k("MCC_ARG_CONSISTENT_HASHING_PREFIXES", String);
k("MCC_HAVE_DEBUG_LOG", Int64);
k("MCC_ARG_DEBUG", String);
k("MCC_ARG_DEBUG_LOGFILE", String);
k("MCC_HAVE_ZLIB_COMPRESSION", Int64);
k("MCC_COMPRESSION_THRESHHOLD", Int64);
k("MCC_ARG_SERVERS", String);
k("MCC_ARG_MIRROR_CFG", String);
k("MCC_ARG_MIRROR_CFG_NAME", String);
k("MCC_ARG_MIRROR_CFG_MODEL", String);
k("MCC_ARG_MIRROR_CFG_SERVERPOOLS", String);
k("MCC_ARG_COMPRESSION_THRESHOLD", String);
k("MCC_ARG_NZLIB_COMPRESSION", String);
k("MCC_ARG_CONN_TMO", String);
k("MCC_ARG_CONN_NTRIES", String);
k("MCC_ARG_DEFAULT_PREFIX", String);
k("MCC_ARG_DELETE_PROXY", String);
k("MCC_ARG_DGRAM_NTRIES", String);
k("MCC_ARG_DGRAM_TMO_WEIGHT", String);
k("MCC_ARG_NODELAY", String);
k("MCC_ARG_PERSISTENT", String);
k("MCC_ARG_POLL_TMO", String);
k("MCC_ARG_PROXY", String);
k("MCC_ARG_PROXY_OPS", String);
k("MCC_ARG_TMO", String);
k("MCC_ARG_TCP_INACTIVITY_TIME", String);
k("MCC_ARG_NPOOLPREFIX", String);
k("MCC_TCP_INACTIVITY_TMO_DEFAULT", Int64);
k("MCC_ARG_UDP_REPLY_PORTS", String);
k("MCC_ARG_WINDOW_MAX", String);
k("MCC_CONSISTENCY_IGNORE", Int64);
k("MCC_CONSISTENCY_MATCH_ALL", Int64);
k("MCC_CONSISTENCY_MATCH_HITS", Int64);
k("MCC_CONSISTENCY_MATCH_HITS_SUPERCEDES", Int64);
k("MCC_ARG_SERVER_RETRY_TMO_MS", String);
k("MCC_ARG_DGRAM_TMO_THRESHOLD", String);
k("MCC_GET_RECORD_ERRORS", Int64);
k("MCC_DELETE_DELETED", Int64);
k("MCC_DELETE_NOTFOUND", Int64);
k("MCC_DELETE_ERROR_LOG", Int64);
k("MCC_DELETE_ERROR_NOLOG", Int64);

k("PHPMCC_NEW_HANDLE", Int64);
k("PHPMCC_USED_FAST_PATH", Int64);
k("PHPMCC_USED_SLOW_PATH", Int64);


k("PHPMCC_VERSION", String);

c('phpmcc', null, array(),
  array(
        m(PublicMethod, "__construct", null,
          array('name' => String,
                'persistent' => array(Boolean, 'true'),
                'npoolprefix' => array(Int64, 'k_MCC_POOLPREFIX_LEN'),
                'mtu' => array(Int64, 'k_MCC_MTU'),
                'rxdgram_max' => array(Int64, 'k_MCC_NODELAY'),
                'nodelay' => array(Int64, 'k_MCC_CONN_TMO_MS'),
                'conn_tmo' => array(Int64, 'k_MCC_CONN_TMO_MS'),
                'conn_ntries' => array(Int64, 'k_MCC_CONN_NTRIES'),
                'tmo' => array(Int64, 'k_MCC_TMO_MS'),
                'dgram_ntries' => array(Int64, 'k_MCC_DGRAM_NTRIES'),
                'dgram_tmo_weight' => array(Double, 'k_MCC_DGRAM_TMO_WEIGHT'),
                'server_retry_tmo' => array(Int64, 'k_MCC_SERVER_RETRY_TMO_MS'),
                'dgram_tmo_threshold' => array(Int64, 'k_MCC_DGRAM_TMO_THRESHOLD'),
                'window_max' => array(Int64, 'k_MCC_WINDOW_MAX'))),
        m(PublicMethod, "__destruct", Variant),
        m(PublicMethod, "__toString", String),
        m(PublicMethod, "__set", Variant,
          array('name' => Variant,
                'val' => Variant)),
        m(PublicMethod, "__get", Variant,
          array('name' => Variant)),
        m(PublicMethod, "close", Boolean),
        m(PublicMethod, "del", Boolean),
        m(PublicMethod, "add_accesspoint", Int64,
          array('server' => String,
                'host' => String,
                'port' => array(String, '"11211"'),
                'protocol' => array(Int64, 'k_MCC_IPPROTO_TCP'))),
        m(PublicMethod, "remove_accesspoint", null,
          array('server' => String,
                'host' => String,
                'port' => array(String, '"11211"'),
                'protocol' => array(Int64, 'k_MCC_IPPROTO_TCP'))),
        m(PublicMethod, "get_accesspoints", Variant,
          array('server' => String)),
        m(PublicMethod, "get_server", Variant,
          array('server' => String)),
        m(PublicMethod, "add_mirror_accesspoint", Variant,
          array('mirrorname' => String,
                'server' => String,
                'host' => String,
                'port' => array(String, '"11211"'),
                'protocol' => array(Int64, 'k_MCC_IPPROTO_TCP'))),
        m(PublicMethod, "remove_mirror_accesspoint", null,
          array('mirrorname' => String,
                'server' => String,
                'host' => String,
                'port' => array(String, '"11211"'),
                'protocol' => array(Int64, 'k_MCC_IPPROTO_TCP'))),
        m(PublicMethod, "add_server", Int64,
          array('server' => String,
                'mirror' => array(String, '""'))),
        m(PublicMethod, "remove_server", null,
          array('server' => String,
                'mirror' => array(String, '""'))),
        m(PublicMethod, "server_flush", Boolean,
          array('server' => String,
                'exptime' => array(Int64, '0'))),
        m(PublicMethod, "server_version", Variant,
          array('server' => String)),
        m(PublicMethod, "server_is_alive", Boolean,
          array('server' => array(String, '""'))),
        m(PublicMethod, "test_proxy", Boolean,
          array('server' => array(String, '""'))),
        m(PublicMethod, "add_mirror", Variant,
          array('mirrorname' => String,
                'model' => Int64)),
        m(PublicMethod, "remove_mirror", Variant,
          array('mirrorname' => String)),
        m(PublicMethod, "add_serverpool", Variant,
          array('serverpool' => String,
                'consistent_hashing_enabled' => array(Boolean, 'false'))),
        m(PublicMethod, "add_serverpool_ex", Variant,
          array('serverpool' => String,
                'version_flag' => Int64)),
        m(PublicMethod, "remove_serverpool", null,
          array('serverpool' => String)),
        m(PublicMethod, "add_accesspoint_listener", Boolean,
          array('function' => String,
                'context' => Variant | Reference)),
        m(PublicMethod, "remove_accesspoint_listener", Boolean,
          array('function' => String,
                'context' => Variant | Reference)),
        m(PublicMethod, "add_server_listener", Boolean,
          array('function' => String,
                'context' => Variant | Reference)),
        m(PublicMethod, "remove_server_listener", Boolean,
          array('function' => String,
                'context' => Variant | Reference)),
        m(PublicMethod, "add_error_listener", Boolean,
          array('function' => String,
                'context' => Variant | Reference)),
        m(PublicMethod, "remove_error_listener", Boolean,
          array('function' => String,
                'context' => Variant | Reference)),
        m(PublicMethod, "get_server_by_key", Variant,
          array('key' => String)),
        m(PublicMethod, "get_host", Variant,
          array('key' => String)),
        m(PublicMethod, "get_serverpool_by_key", Variant,
          array('key' => String)),
        m(PublicMethod, "serverpool_add_server", Variant,
          array('serverpool' => String,
                'server' => String,
                'mirrorname' => array(String, '""'))),
        m(PublicMethod, "serverpool_remove_server", Variant,
          array('serverpool' => String,
                'server' => String,
                'mirrorname' => array(String, '""'))),
        m(PublicMethod, "serverpool_get_servers", Variant,
          array('serverpool' => String)),
        m(PublicMethod, "serverpool_get_consistent_hashing_enabled", Variant,
          array('serverpool' => String)),
        m(PublicMethod, "serverpool_get_consistent_hashing_version", Variant,
          array('serverpool' => String)),
        m(PublicMethod, "multi_add", Variant,
          array('keys_values' => StringMap,
                'exptime' => array(Int64, '0'),
                'compress' => array(Int64, '1'),
                'proxy_replicate' => array(Int64, '0'),
                'async_set' => array(Int64, '0'))),
        m(PublicMethod, "multi_replace", Variant,
          array('keys_values' => StringMap,
                'exptime' => array(Int64, '0'),
                'compress' => array(Int64, '1'),
                'proxy_replicate' => array(Int64, '0'),
                'async_set' => array(Int64, '0'))),
        m(PublicMethod, "multi_set", Variant,
          array('keys_values' => StringMap,
                'exptime' => array(Int64, '0'),
                'compress' => array(Int64, '1'),
                'proxy_replicate' => array(Int64, '0'),
                'async_set' => array(Int64, '0'))),
        m(PublicMethod, "add", Variant,
          array('key' => Variant,
                'value' => Variant,
                'exptime' => array(Int64, '0'),
                'compress' => array(Boolean, 'true'),
                'proxy_replicate' => array(Int64, '0'),
                'async_set' => array(Int64, '0'))),
        m(PublicMethod, "decr", Variant,
          array('key' => String,
                'value' => array(Int64, '1'))),
        m(PublicMethod, "incr", Variant,
          array('key' => String,
                'value' => array(Int64, '1'))),
        m(PublicMethod, "delete", Variant,
          array('keys' => Variant,
                'exptime' => array(Int64, '0'))),
        m(PublicMethod, "delete_details", Variant,
          array('keys' => Variant,
                'exptime' => array(Int64, '0'))),
        m(PublicMethod, "get", Variant,
          array('keys' => Variant,
                'detailed_info_mode' => array(Int64, '0'),
                'detailed_info' => array(Variant | Reference, 'null'))),
        m(PublicMethod, "get_multi", Variant,
          array('keys' => Variant,
                'detailed_info_mode' => array(Int64, '0'),
                'detailed_info' => array(Variant | Reference, 'null'))),
        m(PublicMethod, "replace", Variant,
          array('key' => Variant,
                'value' => Variant,
                'exptime' => array(Int64, '0'),
                'compress' => array(Boolean, 'true'),
                'proxy_replicate' => array(Int64, '0'),
                'async_set' => array(Int64, '0'))),
        m(PublicMethod, "set", Variant,
          array('key' => Variant,
                'value' => Variant,
                'exptime' => array(Int64, '0'),
                'compress' => array(Boolean, 'true'),
                'proxy_replicate' => array(Int64, '0'),
                'async_set' => array(Int64, '0'))),
        m(PublicMethod, "stats", Variant,
          array('clear' => array(Int64, '0')))),
  array(ck("IPPROTO_TCP", Int64),
        ck("IPPROTO_UDP", Int64)),
  "\n  public: MccResourcePtr m_mcc;");
