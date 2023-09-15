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
 * Format same as in mcrouter_options_list.h
 */

MCROUTER_OPTION_GROUP("Standalone mcrouter options")

MCROUTER_OPTION_STRING(log_file, "", "log-path", 'L', "Log file path")

MCROUTER_OPTION_STRING(
    carbon_router_name,
    "Memcache",
    "carbon-router-name",
    no_short,
    "Name of the carbon router to use")

MCROUTER_OPTION_OTHER(
    std::vector<std::string>,
    listen_addresses,
    ,
    "listen-addresses",
    no_short,
    "Address(es) to listen on (comma separated)")

MCROUTER_OPTION_OTHER(
    std::vector<uint16_t>,
    ports,
    ,
    "port",
    'p',
    "Port(s) to listen on (comma separated)")

MCROUTER_OPTION_OTHER(
    std::vector<uint16_t>,
    ssl_ports,
    ,
    "ssl-port",
    no_short,
    "SSL Port(s) to listen on (comma separated)")

MCROUTER_OPTION_STRING(
  tls_ticket_key_seed_path, "",
  "tls-ticket-key-seed-path", no_short,
  "Path to file containing JSON object for old, current, and new seeds"
  " used to generate TLS ticket keys")

MCROUTER_OPTION_TOGGLE(
    ssl_require_peer_certs,
    false,
    "ssl-require-peer-certs",
    no_short,
    "If enabled, clients must present valid certificates when using ssl")

MCROUTER_OPTION_STRING(
    server_pem_cert_path,
    "", // this may get overwritten by finalizeOptions
    "server-pem-cert-path",
    no_short,
    "Path of pem-style server certificate for ssl.")

MCROUTER_OPTION_STRING(
    server_pem_key_path,
    "", // this may get overwritten by finalizeOptions
    "server-pem-key-path",
    no_short,
    "Path of pem-style server key for ssl.")

MCROUTER_OPTION_STRING(
    server_pem_ca_path,
    MCROUTER_DEFAULT_CA_PATH,
    "server-pem-ca-path",
    no_short,
    "Path of pem-style CA cert for ssl to verify clients against")

MCROUTER_OPTION_TOGGLE(
    ssl_use_ktls12,
    false,
    "ssl-use-ktls12",
    no_short,
    "Use KTLS for all TLS 1.2 connections")

MCROUTER_OPTION_INTEGER(
    int,
    listen_sock_fd,
    -1,
    "listen-sock-fd",
    no_short,
    "Listen socket to take over")

MCROUTER_OPTION_STRING(
    unix_domain_sock,
    "",
    "unix-domain-sock",
    no_short,
    "Unix domain socket path")

MCROUTER_OPTION_INTEGER(
    size_t,
    max_conns,
    0,
    "max-conns",
    no_short,
    "Maximum number of connections maintained by server. Special values: "
    "0 - disable connection eviction logic; 1 - calculate number of maximum "
    "connections based on rlimits. Eviction logic is disabled by default.")

MCROUTER_OPTION_INTEGER(
    int,
    tcp_listen_backlog,
    SOMAXCONN,
    "tcp-listen-backlog",
    no_short,
    "TCP listen backlog size")

MCROUTER_OPTION_INTEGER(
    uint32_t,
    max_client_outstanding_reqs,
    DEFAULT_MAX_CLIENT_OUTSTANDING_REQS,
    "max-client-outstanding-reqs",
    no_short,
    "Maximum requests outstanding per client (0 to disable)")

MCROUTER_OPTION_TOGGLE(
    retain_source_ip,
    false,
    "retain-source-ip",
    no_short,
    "Look up the source IP address for inbound requests and expose it to"
    " routing logic.")

MCROUTER_OPTION_TOGGLE(
    enable_server_compression,
    false,
    "enable-server-compression",
    no_short,
    "Enable compression on the AsyncMcServer")

MCROUTER_OPTION_INTEGER(
    unsigned int,
    client_timeout_ms,
    1000,
    "client-timeout",
    no_short,
    "Timeout for sending replies back to clients, in milliseconds. "
    "(0 to disable)")

MCROUTER_OPTION_INTEGER(
    uint64_t,
    server_load_interval_ms,
    0,
    "server-load-interval-ms",
    no_short,
    "How often to collect server load data. "
    "(0 to disable exposing server load)")

MCROUTER_OPTION_INTEGER(
    uint32_t,
    tfo_queue_size,
    100000,
    "tfo-queue-size",
    no_short,
    "TFO queue size for SSL connections.  "
    "(only matters if ssl tfo is enabled)")

MCROUTER_OPTION_TOGGLE(
    enable_pass_through_mode,
    false,
    "enable-pass-through-mode",
    no_short,
    "If enabled, mcrouter will avoid reserializing requests if the request"
    " is not modified during routing.")

MCROUTER_OPTION_INTEGER(
    size_t,
    tcp_zero_copy_threshold,
    0,
    "tcp-zero-copy-threshold",
    no_short,
    "TCP packets with payload >= tcp-zero-copy-threshold bytes will"
    " use the zero copy optimization on TX."
    " If 0, the tcp zero copy optimization will not be applied.")

MCROUTER_OPTION_TOGGLE(
    acl_checker_enable,
    false,
    "acl-checker-enable",
    no_short,
    "If true, incoming connections are checked against the ACL.")

MCROUTER_OPTION_TOGGLE(
    acl_checker_enforce,
    false,
    "acl-checker-enforce",
    no_short,
    "If true, enforces the result of the ACL check.")

MCROUTER_OPTION_TOGGLE(
    prefix_acl_checker_enable,
    false,
    "prefix-acl-checker-enable",
    no_short,
    "If true, incoming requests are checked against the Prefix ACL.")

MCROUTER_OPTION_STRING(
    server_ssl_service_identity,
    "memcache",
    "server-ssl-service-identity",
    no_short,
    "If true, enforces the result of the ACL check.")

MCROUTER_OPTION_INTEGER(
    size_t,
    num_listening_sockets,
    kListeningSocketsDefault,
    "num-listening-sockets",
    no_short,
    "adjust how many listening sockets to use. Must be <= num_proxies")

MCROUTER_OPTION_TOGGLE(
    remote_thread,
    false,
    "remote-thread",
    no_short,
    "If true, mcrouter client threads will be separate from the server threads."
    " This option will cause mcrouter to use more CPU, but it will reduce the"
    " number of connections if running with >1 proxies/threads, and used"
    " together with thread-affinity option.")

MCROUTER_OPTION_TOGGLE(
    enable_qos,
    false,
    "enable-qos",
    no_short,
    "If enabled, sets the DSCP field in IP header of accepted connections according "
    "to the specified qos class / qos path.")

MCROUTER_OPTION_INTEGER(
    unsigned int,
    default_qos_class,
    0,
    "default-qos-class",
    no_short,
    "Default qos class to use on accepted connections. The classes go from "
    "0 (lowest priority) to 4 (highest priority) and act on the hightest-order "
    "bits of DSCP.")

MCROUTER_OPTION_INTEGER(
    unsigned int,
    default_qos_path,
    0,
    "default-qos-path",
    no_short,
    "Default qos path to use on accepted connections. The classes go from "
    "0 (lowest priority) to 3 (highest priority) and act on the lowest-order "
    "bits of DSCP.")

MCROUTER_OPTION_TOGGLE(
    use_thrift,
    false,
    "use-thrift",
    no_short,
    "If true, mcrouter will start both AsyncMcServer and ThriftServer. "
    "They will share the same threads and evbs but bind to different ports, "
    "as specified by ports, ssl-port and thrift-port")

MCROUTER_OPTION_INTEGER(
    uint16_t,
    thrift_port,
    0,
    "thrift-port",
    no_short,
    "Thrift Port to listen on")

MCROUTER_OPTION_DOUBLE(
    double,
    core_multiplier,
    1.0,
    "core-multiplier",
    no_short,
    "When > 0, number of proxies used will be std::hardware_concurrency * core-multiplier")

MCROUTER_OPTION_INTEGER(
    size_t,
    core_multiplier_threshold,
    0,
    "core-multiplier-threshold",
    no_short,
    "core-mutiplier logic is applied if the number of cores  >= core_multiplier_threshold")

#ifdef ADDITIONAL_STANDALONE_OPTIONS_FILE
#include ADDITIONAL_STANDALONE_OPTIONS_FILE
#endif

#undef no_short
#undef no_long
#undef MCROUTER_OPTION_GROUP
#undef MCROUTER_OPTION
