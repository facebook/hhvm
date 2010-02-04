#if !defined(__EXT_PHPMCC_CONSTANTS_H__)
#define __EXT_PHPMCC_CONSTANTS_H__

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

#endif /* #if !defined(__EXT_PHPMCC_CONSTANTS_H__) */
