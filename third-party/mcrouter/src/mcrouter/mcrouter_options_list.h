/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// @nolint
#ifndef MCROUTER_OPTION_GROUP
#define MCROUTER_OPTION_GROUP(_sep)
#endif

#define no_long ""
#define no_short '\0'

/**
 * Format:
 *
 * mcrouter_option_<string, integer, or toggle>(
 *  [type (integers only), ] name of field in the struct, default value,
 *  long option (or no_long), short option char (or no_short),
 *  docstring)
 *
 * A long option is a requirement for options that can be set from command line.
 *
 * Short options are optional and in short supply (pun overload).
 *
 * A toggle option doesn't accept a command line argument, and specifying
 * it on the command line will set it to the opposite of the default value.
 *
 * MCROUTER_OPTION_GROUP(name) starts a new option group (for nicer display)
 */

MCROUTER_OPTION_GROUP("Startup")

MCROUTER_OPTION_STRING(
    service_name,
    "unknown",
    no_long,
    no_short,
    "Name of the service using this libmcrouter instance")

MCROUTER_OPTION_STRING(
    router_name,
    "unknown",
    no_long,
    no_short,
    "Name for this router instance (should reflect the configuration,"
    " the flavor name is usually a good choice)")

MCROUTER_OPTION_STRING(
    flavor_name,
    "unknown",
    no_long,
    no_short,
    "Name of the flavor used to configure this router instance.")

MCROUTER_OPTION_TOGGLE(
    asynclog_disable,
    false,
    "asynclog-disable",
    no_short,
    "disable async log file spooling")

MCROUTER_OPTION_STRING(
    async_spool,
    "/var/spool/mcrouter",
    "async-dir",
    'a',
    "container directory for async storage spools")

MCROUTER_OPTION_TOGGLE(
    use_asynclog_version2,
    false,
    "use-asynclog-version2",
    no_short,
    "Enable using the asynclog version 2.0")

MCROUTER_OPTION_INTEGER(
    size_t,
    num_proxies,
    DEFAULT_NUM_PROXIES,
    "num-proxies",
    no_short,
    "adjust how many proxy threads to run")

MCROUTER_OPTION_INTEGER(
    size_t,
    client_queue_size,
    1024,
    no_long,
    no_short,
    "McrouterClient -> ProxyThread queue size.")

MCROUTER_OPTION_INTEGER(
    size_t,
    client_queue_no_notify_rate,
    0,
    no_long,
    no_short,
    "Each client will only notify on every Nth request."
    "  If 0, normal notification logic is used - i.e. notify on every request,"
    " best effort avoid notifying twice.  Higher values decrease CPU utilization,"
    " but increase average latency.")

MCROUTER_OPTION_INTEGER(
    size_t,
    client_queue_wait_threshold_us,
    0,
    no_long,
    no_short,
    "Force client queue notification if last drain was at least this long ago."
    "  If 0, this logic is disabled.")

MCROUTER_OPTION_INTEGER(
    size_t,
    big_value_split_threshold,
    0,
    "big-value-split-threshold",
    no_short,
    "If 0, big value route handle is not part of route handle tree,"
    "else used as threshold for splitting big values internally")

MCROUTER_OPTION_INTEGER(
    size_t,
    big_value_batch_size,
    10,
    "big-value-batch-size",
    no_short,
    "If nonzero, big value chunks are written/read in batches of at most"
    " this size.  Used to prevent queue build up with really large values")

MCROUTER_OPTION_TOGGLE(
    big_value_hide_reply_flag,
    false,
    "big-value-hide-reply-flag",
    no_short,
    "If enabled, the reply flags will not contain MC_MSG_FLAG_BIG_VALUE in"
    " case of a big value.")

MCROUTER_OPTION_INTEGER(
    size_t,
    fibers_max_pool_size,
    1000,
    "fibers-max-pool-size",
    no_short,
    "Maximum number of preallocated free fibers to keep around")

MCROUTER_OPTION_INTEGER(
    size_t,
    fibers_stack_size,
    24 * 1024,
    "fibers-stack-size",
    no_short,
    "Size of stack in bytes to allocate per fiber."
    " 0 means use fibers library default.")

MCROUTER_OPTION_INTEGER(
    size_t,
    fibers_record_stack_size_every,
    100000,
    "fibers-record-stack-size-every",
    no_short,
    "Record exact amount of fibers stacks used for every N fiber. "
    "0 disables stack recording.")

MCROUTER_OPTION_TOGGLE(
    fibers_use_guard_pages,
    true,
    "disable-fibers-use-guard-pages",
    no_short,
    "If enabled, protect limited amount of fiber stacks with guard pages")

MCROUTER_OPTION_STRING(
    runtime_vars_file,
    MCROUTER_RUNTIME_VARS_DEFAULT,
    "runtime-vars-file",
    no_short,
    "Path to the runtime variables file.")

MCROUTER_OPTION_INTEGER(
    uint32_t,
    file_observer_poll_period_ms,
    100,
    "file-observer-poll-period-ms",
    no_short,
    "How often to check inotify for updates on the tracked files.")

MCROUTER_OPTION_INTEGER(
    uint32_t,
    file_observer_sleep_before_update_ms,
    1000,
    "file-observer-sleep-before-update-ms",
    no_short,
    "How long to sleep for after an update occured"
    " (a hack to avoid partial writes).")

MCROUTER_OPTION_INTEGER(
    uint32_t,
    fibers_pool_resize_period_ms,
    60000,
    "fibers-pool-resize-period-ms",
    no_short,
    "Free unnecessary fibers in the fibers pool every"
    " fibers-pool-resize-period-ms milliseconds.  If value is 0, periodic"
    " resizing of the free pool is disabled.")

MCROUTER_OPTION_GROUP("Network")

MCROUTER_OPTION_INTEGER(
    int,
    keepalive_cnt,
    0,
    "keepalive-count",
    'K',
    "set TCP KEEPALIVE count, 0 to disable")

MCROUTER_OPTION_INTEGER(
    int,
    keepalive_interval_s,
    60,
    "keepalive-interval",
    'i',
    "set TCP KEEPALIVE interval parameter in seconds")

MCROUTER_OPTION_INTEGER(
    int,
    keepalive_idle_s,
    300,
    "keepalive-idle",
    'I',
    "set TCP KEEPALIVE idle parameter in seconds")

MCROUTER_OPTION_INTEGER(
    int,
    max_no_flush_event_loops,
    5,
    "max-no-flush-event-loops",
    no_short,
    "Maximum number of non-blocking event loops before we flush batched "
    "requests")

MCROUTER_OPTION_INTEGER(
    unsigned int,
    reset_inactive_connection_interval,
    60000,
    "reset-inactive-connection-interval",
    no_short,
    "Will close open connections without any activity after at most 2 * interval"
    " ms. If value is 0, connections won't be closed.")

MCROUTER_OPTION_INTEGER(
    int,
    tcp_rto_min,
    -1,
    "tcp-rto-min",
    no_short,
    "adjust the minimum TCP retransmit timeout (ms) to memcached")

MCROUTER_OPTION_INTEGER(
    uint64_t,
    target_max_inflight_requests,
    0,
    "target-max-inflight-requests",
    no_short,
    "Maximum inflight requests allowed per target per thread"
    " (0 means no throttling)")

MCROUTER_OPTION_INTEGER(
    uint64_t,
    target_max_pending_requests,
    100000,
    "target-max-pending-requests",
    no_short,
    "Only active if target-max-inflight-requests is nonzero."
    " Hard limit on the number of requests allowed in the queue"
    " per target per thread.  Requests that would exceed this limit are dropped"
    " immediately.")

MCROUTER_OPTION_INTEGER(
    size_t,
    target_max_shadow_requests,
    1000,
    "target-max-shadow-requests",
    no_short,
    "Hard limit on the number of shadow requests allowed in the queue"
    " per target per thread.  Requests that would exceed this limit are dropped"
    " immediately.")

MCROUTER_OPTION_INTEGER(
    size_t,
    proxy_max_inflight_requests,
    0,
    "proxy-max-inflight-requests",
    no_short,
    "If non-zero, sets the limit on maximum incoming requests that will be routed"
    " in parallel by each proxy thread.  Requests over limit will be queued up"
    " until the number of inflight requests drops.")

MCROUTER_OPTION_INTEGER(
    size_t,
    proxy_max_inflight_shadow_requests,
    0,
    "proxy-max-inflight-shadow-requests",
    no_short,
    "If non-zero, sets the limit on maximum shadow requests that can be inflight"
    " on each proxy thread.  Shadow requests over the limit will be dropped and"
    " an error reply sent.")

MCROUTER_OPTION_INTEGER(
    size_t,
    proxy_max_throttled_requests,
    0,
    "proxy-max-throttled-requests",
    no_short,
    "Only active if proxy-max-inflight-requests is non-zero. "
    "Hard limit on the number of requests to queue per proxy after "
    "there are already proxy-max-inflight-requests requests in flight for the "
    "proxy. Further requests will be rejected with an error immediately. 0 means "
    "disabled.")

MCROUTER_OPTION_STRING(
    pem_cert_path,
    "", // this may get overwritten by finalizeOptions
    "pem-cert-path",
    no_short,
    "Path of pem-style client certificate for ssl.")

MCROUTER_OPTION_STRING(
    pem_key_path,
    "", // this may get overwritten by finalizeOptions
    "pem-key-path",
    no_short,
    "Path of pem-style client key for ssl.")

MCROUTER_OPTION_STRING(
    pem_ca_path,
    MCROUTER_DEFAULT_CA_PATH,
    "pem-ca-path",
    no_short,
    "Path of pem-style CA cert for ssl")

MCROUTER_OPTION_STRING(
    ssl_service_identity,
    "",
    "ssl-service-identity",
    no_short,
    "The service identity of the destination service when SSL is used")

MCROUTER_OPTION_OTHER(
    std::vector<std::string>,
    additional_ssl_service_identities,
    ,
    "additional-ssl-service-identities",
    no_short,
    "Additional service identities of the destination service when SSL is used (comma separated)")

MCROUTER_OPTION_TOGGLE(
    ssl_service_identity_authorization_log,
    false,
    "ssl-service-identity-authorization-log",
    no_short,
    "The configured service identity of the client is compared against the "
    "service identity of the server in the peer certificate. Log if they "
    "do not match.")

MCROUTER_OPTION_TOGGLE(
    ssl_service_identity_authorization_enforce,
    false,
    "ssl-service-identity-authorization-enforce",
    no_short,
    "The configured service identity of the client is compared against the "
    "service identity of the server in the peer certificate. Fail to "
    "connect if they do not match.")

MCROUTER_OPTION_TOGGLE(
    enable_qos,
    false,
    "enable-qos",
    no_short,
    "If enabled, sets the DSCP field in IP header according "
    "to the specified qos class.")

MCROUTER_OPTION_INTEGER(
    unsigned int,
    default_qos_class,
    0,
    "default-qos-class",
    no_short,
    "Default qos class to use if qos is enabled and the class is not specified "
    "in pool/server config. The classes go from 0 (lowest priority) to "
    "4 (highest priority) and act on the hightest-order bits of DSCP.")

MCROUTER_OPTION_INTEGER(
    unsigned int,
    default_qos_path,
    0,
    "default-qos-path",
    no_short,
    "Default qos path priority class to use if qos is enabled and it is not "
    "specified in the pool/server config. The path priority classes go from "
    "0 (lowest priority) to 3 (highest priority) and act on the lowest-order "
    "bits of DSCP.")

MCROUTER_OPTION_TOGGLE(
    ssl_connection_cache,
    false,
    "ssl-connection-cache",
    no_short,
    "If enabled, limited number of SSL sessions will be cached")

MCROUTER_OPTION_TOGGLE(
    ssl_handshake_offload,
    false,
    "ssl-handshake-offload",
    no_short,
    "If enabled, SSL handshakes are offloaded to a separate threadpool")

MCROUTER_OPTION_TOGGLE(
    ssl_verify_peers,
    false,
    "ssl-verify-peers",
    no_short,
    "If enabled, clients will verify server certificates.")

MCROUTER_OPTION_TOGGLE(
    enable_compression,
    false,
    "enable-compression",
    no_short,
    "If enabled, mcrouter replies will be compressed according to the "
    "compression algorithms/dictionaries supported by the client. Only "
    "compresses caret protocol replies.")

MCROUTER_OPTION_GROUP("Routing configuration")

MCROUTER_OPTION_TOGGLE(
    constantly_reload_configs,
    false,
    "constantly-reload-configs",
    no_short,
    "")

MCROUTER_OPTION_TOGGLE(
    disable_reload_configs,
    false,
    "disable-reload-configs",
    no_short,
    "")

MCROUTER_OPTION_STRING(
    config,
    "",
    "config",
    no_short,
    "Configuration to use. The provided string must be of one of two forms:"
    " file:<path-to-config-file> OR <JSON-config-string>. If provided,"
    " this option supersedes the deprecated config-file and config-str options.")

MCROUTER_OPTION_STRING(
    config_dump_root,
    CONFIG_DUMP_ROOT_DEFAULT,
    "config-dump-root",
    no_short,
    "Directory where the last valid config will be saved. "
    "Empty string to disable.")

MCROUTER_OPTION_INTEGER(
    int,
    max_dumped_config_age,
    12 * 60 * 60 /* 12 hours */,
    "max-dumped-config-age",
    no_short,
    "Max age of backup config files that mcrouter is allowed to use"
    " (in seconds). 0 to disable using dumped configs.")

MCROUTER_OPTION_STRING(
    config_file,
    "",
    "config-file",
    'f',
    "DEPRECATED. Load configuration from file. This option has no effect if"
    " --config option is used.")

MCROUTER_OPTION_STRING(
    pool_stats_config_file,
    "",
    "pool-stats-config-file",
    no_short,
    "File containing stats enabled pool names.")

MCROUTER_OPTION_STRING(
    config_str,
    "",
    "config-str",
    no_short,
    "DEPRECATED. Configuration string provided as a command line argument."
    " This option has no effect if --config option is used.")

MCROUTER_OPTION(
    facebook::memcache::mcrouter::RoutingPrefix,
    default_route,
    "/././",
    "route-prefix",
    'R',
    "default routing prefix (ex. /oregon/prn1c16/)",
    routing_prefix)

MCROUTER_OPTION_TOGGLE(
    miss_on_get_errors,
    true,
    "disable-miss-on-get-errors",
    no_short,
    "Disable reporting get errors as misses")

MCROUTER_OPTION_TOGGLE(
    disable_miss_on_arith_errors,
    false,
    "disable-miss-on-arith-errors",
    no_short,
    "Disable reporting arithmetic operation errors as misses")

MCROUTER_OPTION_TOGGLE(
    group_remote_errors,
    false,
    "group-remote-errors",
    no_short,
    "Groups all remote (i.e. non-local) errors together, returning a single "
    "result for all of them: REMOTE_ERROR")

MCROUTER_OPTION_TOGGLE(
    send_invalid_route_to_default,
    false,
    "send-invalid-route-to-default",
    no_short,
    "Send request to default route if routing prefix is not present in config")

MCROUTER_OPTION_TOGGLE(
    enable_flush_cmd,
    false,
    "enable-flush-cmd",
    no_short,
    "Enable flush_all command")

MCROUTER_OPTION_TOGGLE(
    disable_request_deadline_check,
    false,
    "disable-request-deadline-check",
    no_short,
    "Disable request deadline functionality")

MCROUTER_OPTION_INTEGER(
    int,
    reconfiguration_delay_ms,
    1000,
    "reconfiguration-delay-ms",
    no_short,
    "Delay between config files change and mcrouter reconfiguration.")

MCROUTER_OPTION_INTEGER(
    int,
    reconfiguration_jitter_ms,
    0,
    "reconfiguration-jitter-ms",
    no_short,
    "Random jitter from [0,reconfiguration_jitter_ms) applied after "
    "config files change and before mcrouter reconfiguration.")

MCROUTER_OPTION_INTEGER(
    int,
    post_reconfiguration_delay_ms,
    0,
    "post-reconfiguration-delay-ms",
    no_short,
    "Delay after a reconfiguration is complete.")

MCROUTER_OPTION_STRING_MAP(
    config_params,
    "config-params",
    no_short,
    "Params for config preprocessor in format 'name1:value1,name2:value2'. "
    "All values will be passed as strings.")

MCROUTER_OPTION_GROUP("TKO probes")

MCROUTER_OPTION_TOGGLE(
    disable_tko_tracking,
    false,
    "disable-tko-tracking",
    no_short,
    "Disable TKO tracker (marking a host down for fast failover after"
    " a number of failures, and sending probes to check if the server"
    " came back up).")

MCROUTER_OPTION_INTEGER(
    int,
    probe_delay_initial_ms,
    10000,
    "probe-timeout-initial",
    'r',
    "TKO probe retry initial timeout in ms")

MCROUTER_OPTION_INTEGER(
    int,
    probe_delay_max_ms,
    60000,
    "probe-timeout-max",
    no_short,
    "TKO probe retry max timeout in ms")

MCROUTER_OPTION_INTEGER(
    int,
    failures_until_tko,
    3,
    "timeouts-until-tko",
    no_short,
    "Mark as TKO after this many failures")

MCROUTER_OPTION_TOGGLE(
    allow_only_gets,
    false,
    "allow-only-gets",
    no_short,
    "Testing only. Allow only get-like operations: get, metaget, lease get. "
    "For any other operation return a default reply (not stored/not found).")

MCROUTER_OPTION_GROUP("Timeouts")

MCROUTER_OPTION_INTEGER(
    unsigned int,
    server_timeout_ms,
    1000,
    "server-timeout",
    't',
    "Timeout for talking to destination servers (e.g. memcached), "
    "in milliseconds. Must be greater than 0.")

MCROUTER_OPTION_INTEGER(
    unsigned int,
    cross_region_timeout_ms,
    0,
    "cross-region-timeout-ms",
    no_short,
    "Timeouts for talking to cross region pool. "
    "If specified (non 0) takes precedence over every other timeout.")

MCROUTER_OPTION_INTEGER(
    unsigned int,
    cross_cluster_timeout_ms,
    0,
    "cross-cluster-timeout-ms",
    no_short,
    "Timeouts for talking to pools within same region but different cluster. "
    "If specified (non 0) takes precedence over every other timeout.")

MCROUTER_OPTION_INTEGER(
    unsigned int,
    within_cluster_timeout_ms,
    0,
    "within-cluster-timeout-ms",
    no_short,
    "Timeouts for talking to pools within same cluster. "
    "If specified (non 0) takes precedence over every other timeout.")

MCROUTER_OPTION_INTEGER(
    unsigned int,
    waiting_request_timeout_ms,
    0,
    "waiting-request-timeout-ms",
    no_short,
    "Maximum time in ms that a new request can wait in the queue before being"
    " discarded. Enabled only if value is non-zero and"
    " if proxy-max-throttled-requests is enabled.")

MCROUTER_OPTION_INTEGER(
    unsigned int,
    connect_timeout_retries,
    0,
    "connect-timeout-retries",
    no_short,
    "The number of times to retry establishing a connection in case of a"
    " connect timeout. We will just return the result back to the client after"
    " either the connection is esblished, or we exhausted all retries.")

MCROUTER_OPTION_GROUP("Custom Memory Allocation")

MCROUTER_OPTION_TOGGLE(
    jemalloc_nodump_buffers,
    false,
    "jemalloc-nodump-buffers",
    no_short,
    "Use the JemallocNodumpAllocator custom allocator. "
    "As the name suggests the memory allocated by this allocator will not be"
    " part of any core dump. This is achieved by setting MADV_DONTDUMP on"
    " explicitly created jemalloc arenas. The default value is false.")

MCROUTER_OPTION_GROUP("Logging")

MCROUTER_OPTION_STRING(
    stats_root,
    MCROUTER_STATS_ROOT_DEFAULT,
    "stats-root",
    no_short,
    "Root directory for stats files")

MCROUTER_OPTION_STRING(
    debug_fifo_root,
    DEBUG_FIFO_ROOT_DEFAULT,
    "debug-fifo-root",
    no_short,
    "Root directory for debug fifos. If empty, debug fifos are disabled.")

MCROUTER_OPTION_INTEGER(
    unsigned int,
    stats_logging_interval,
    10000,
    "stats-logging-interval",
    no_short,
    "Time in ms between stats reports, or 0 for no logging")

MCROUTER_OPTION_INTEGER(
    unsigned int,
    logging_rtt_outlier_threshold_us,
    0,
    "logging-rtt-outlier-threshold-us",
    no_short,
    "surpassing this threshold rtt time means we will log it as an outlier. "
    "0 (the default) means that we will do no logging of outliers.")

MCROUTER_OPTION_INTEGER(
    unsigned int,
    stats_async_queue_length,
    50,
    "stats-async-queue-length",
    no_short,
    "Asynchronous queue size for logging.")

MCROUTER_OPTION_TOGGLE(
    enable_failure_logging,
    true,
    "disable-failure-logging",
    no_short,
    "Disable failure logging.")

MCROUTER_OPTION_TOGGLE(
    test_mode,
    false,
    "test-mode",
    no_short,
    "Starts mcrouter in test mode - with logging disabled.")

MCROUTER_OPTION_TOGGLE(
    enable_logging_route,
    false,
    "enable-logging-route",
    no_short,
    "Log every request via LoggingRoute.")

MCROUTER_OPTION_INTEGER(
    uint64_t,
    collect_rxmit_stats_every_hz,
    0,
    "collect-rxmit-stats-every-hz",
    no_short,
    "Will calculate retransmits per kB after every set cycles whenever a "
    "timeout or deviation from average latency occurs."
    " If value is 0, calculation won't be done.")

MCROUTER_OPTION_INTEGER(
    uint64_t,
    rxmit_latency_deviation_us,
    0,
    "rxmit-latency-deviation-us",
    no_short,
    "Latency deviation of request in microseconds from the average latency on "
    "the connection will trigger recalculation of retransmits per kB. "
    "If value is 0, calculation won't be done.")

MCROUTER_OPTION_INTEGER(
    uint64_t,
    min_rxmit_reconnect_threshold,
    0,
    "min-rxmit-reconnect-threshold",
    no_short,
    "If value is non-zero, mcrouter will reconnect to a target after hitting"
    " min-rxmit-reconnect-threshold retransmits per kb for the first time."
    " Subsequently, the reconnection threshold for the same target server is"
    " dynamically adjusted, always remaining at least"
    " min-rxmit-reconnect-threshold rxmits/kb. If value is 0,"
    " this feature is disabled.")

MCROUTER_OPTION_INTEGER(
    uint64_t,
    max_rxmit_reconnect_threshold,
    0,
    "max-rxmit-reconnect-threshold",
    no_short,
    "Has no effect if min-rxmit-reconnect-threshold is 0."
    " If max-rxmit-reconnect-threshold is also non-zero, the dynamic reconnection"
    " threshold is always at most max-rxmit-reconnect-threshold rxmits/kb."
    " If max-rxmit-reconnect-threshold is 0, the dynamic threshold is unbounded.")

MCROUTER_OPTION_INTEGER(
    int,
    asynclog_port_override,
    0,
    no_long,
    no_short,
    "If non-zero use this port while logging to async log")

MCROUTER_OPTION_TOGGLE(
    enable_send_to_main_shard_split,
    true,
    "disable-send-to-main-shard-split",
    no_short,
    "DEPRECATED. No longer supported/needed")

MCROUTER_OPTION_INTEGER(
    size_t,
    max_shadow_token_map_size,
    1024,
    "max-shadow-token-map-size",
    no_short,
    "Maximum size of LRU cache mapping normal lease tokens to shadow lease"
    " tokens. High rates of shadowing of lease operations may require a limit"
    " higher than the default. 0 disables limiting of map size.")

MCROUTER_OPTION_TOGGLE(
    enable_ssl_tfo,
    false,
    "enable-ssl-tfo",
    no_short,
    "enable TFO when connecting/accepting via SSL")

MCROUTER_OPTION_TOGGLE(
    tls_prefer_ocb_cipher,
    false,
    "tls-prefer-ocb-cipher",
    no_short,
    "Prefer AES-OCB cipher for TLSv1.3 connections if available")

MCROUTER_OPTION_TOGGLE(
    force_same_thread,
    false,
    "force-same-thread",
    no_short,
    "Route requests in the same thread as the caller.")

MCROUTER_OPTION_TOGGLE(
    thread_affinity,
    false,
    "thread-affinity",
    no_short,
    "Enable deterministic selection of the proxy thread to lower the number of"
    "connections between client and server.")

MCROUTER_OPTION_TOGGLE(
    disable_shard_split_route,
    false,
    "disable-shard-split-route",
    no_short,
    "Disable shard split route. Ignore shard_splits field in routing config.")

MCROUTER_OPTION_TOGGLE(
    enable_service_router,
    false,
    "enable-service-router",
    no_short,
    "Enable service router for pool level routing.")

MCROUTER_OPTION_TOGGLE(
    enable_partial_reconfigure,
    false,
    "enable-partial-reconfigure",
    no_short,
    "Incrementally update routing tree with simple config source changes. For "
    " complicated config source change, Mcrouter will resort to build a new "
    "routing tree as if this flag is disabled.")

MCROUTER_OPTION_INTEGER(
    size_t,
    thrift_compression_threshold,
    0,
    "thrift-compression-threshold",
    no_short,
    "Payloads >= thriftCompressionTreshold will be compressed "
    "iff thriftCompression is enabled.")

MCROUTER_OPTION_TOGGLE(
    enable_axonlog,
    false,
    "enable-axonlog",
    no_short,
    "Enable Axon log features")

MCROUTER_OPTION_TOGGLE(
    external_carbon_connection_logging_enabled,
    false,
    "external-carbon-connection-logging-enabled",
    no_short,
    "Enables logging for external carbon connections")

MCROUTER_OPTION_INTEGER(
    uint64_t,
    external_carbon_connection_log_rate_per_hour,
    3600,
    "external-carbon-connection-log-rate-per-hour",
    no_short,
    "Number of carbon connection samples to write on average per hour")

MCROUTER_OPTION_INTEGER(
    uint32_t,
    external_carbon_connection_log_max_burst,
    500,
    "external-carbon-connection-log-max-burst",
    no_short,
    "maximum instantaneous number of request logs per server")

MCROUTER_OPTION_INTEGER(
    uint32_t,
    external_carbon_connection_log_sample_rate,
    10000,
    "external-carbon-connection-log-sample-rate",
    no_short,
    "1 in S non-error connection samples will be logged")

MCROUTER_OPTION_INTEGER(
    uint32_t,
    proxy_cpu_interval_s,
    0,
    "proxy-cpu-interval-s",
    no_short,
    "Measure proxy CPU utilization every proxy_cpu_interval_s seconds. "
    "0 means disabled.")

#ifdef ADDITIONAL_OPTIONS_FILE
#include ADDITIONAL_OPTIONS_FILE
#endif

#undef no_short
#undef no_long
#undef MCROUTER_OPTION_GROUP
