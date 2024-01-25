/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// @nolint
/**
 * Basic stats/information
 */
#define GROUP basic_stats | detailed_stats
STSS(version, MCROUTER_PACKAGE_STRING, 0)
STSS(commandargs, "", 0)
STSI(pid, 0, 0)
STSI(parent_pid, 0, 0)
STUI(time, 0, 0)
#undef GROUP
#define GROUP ods_stats | basic_stats | detailed_stats
// how long ago we started up libmcrouter instance
STUI(uptime, 0, 0)
#undef GROUP

/**
 * Standalone mcrouter stats (not applicable to libmcrouter)
 */
#define GROUP ods_stats | detailed_stats
STUI(num_client_connections, 0, 1)
#undef GROUP

/**
 * Stats related to connections to servers
 */
#define GROUP ods_stats | basic_stats | detailed_stats
STUI(num_servers, 0, 1) // num of ProxyDestinations possible connections.
STUI(num_servers_new, 0, 1) // num of new connections (not open yet).
STUI(num_servers_up, 0, 1) // num of opened connections.
STUI(num_servers_down, 0, 1) // num of conns that weren't explicitly closed.
STUI(num_servers_closed, 0, 1) // num of connections closed (were open before).
// same as above, but for ssl.
STUI(num_ssl_servers, 0, 1)
STUI(num_ssl_servers_new, 0, 1)
STUI(num_ssl_servers_up, 0, 1)
STUI(num_ssl_servers_down, 0, 1)
STUI(num_ssl_servers_closed, 0, 1)
// number of servers marked-down/suspect
STUI(num_suspect_servers, 0, 1)
// Running total of connection opens/closes
STUI(num_connections_opened, 0, 1)
STUI(num_connections_closed, 0, 1)
// Running total of connection not authorized
STUI(num_authorization_failures, 0, 1)
// Running total of connection authorized
STUI(num_authorization_successes, 0, 1)
// Running total of ssl connection opens/closes.
STUI(num_ssl_connections_opened, 0, 1)
STUI(num_ssl_connections_closed, 0, 1)
// Running total of successful SSL connection attempts/successes
STUI(num_ssl_resumption_attempts, 0, 1)
STUI(num_ssl_resumption_successes, 0, 1)
// Running total of tls to plain connections and resumption stats
STUI(num_tls_to_plain_fallback_failures, 0, 1)
STUI(num_tls_to_plain_connections_opened, 0, 1)
STUI(num_tls_to_plain_connections_closed, 0, 1)
STUI(num_tls_to_plain_resumption_attempts, 0, 1)
STUI(num_tls_to_plain_resumption_successes, 0, 1)
// Running total of ktls connections and resumption stats
STUI(num_ktls_fallback_failures, 0, 1)
STUI(num_ktls_connections_opened, 0, 1)
STUI(num_ktls_connections_closed, 0, 1)
STUI(num_ktls_resumption_attempts, 0, 1)
STUI(num_ktls_resumption_successes, 0, 1)
// time between closing an inactive connection and opening it again.
STAT(inactive_connection_closed_interval_sec, stat_double, 0, .dbl = 0.0)
// Information about connect retries
STUI(num_connect_success_after_retrying, 0, 1)
STUI(num_connect_retries, 0, 1)
STUI(num_soft_tko_count, 0, 1)
STUI(num_hard_tko_count, 0, 1)
STUI(num_fail_open_state_entered, 0, 1)
STUI(num_fail_open_state_exited, 0, 1)
// Connections closed due to retransmits
STUI(retrans_closed_connections, 0, 1)
#undef GROUP

/**
 * OutstandingLimitRoute (OLR) queue-related stats, broken down by request type
 * (get-like and update-like).
 */
#define GROUP ods_stats | basic_stats | rate_stats
// Number of requests/second that couldn't be processed immediately in OLR
STUI(outstanding_route_get_reqs_queued, 0, 1)
STUI(outstanding_route_update_reqs_queued, 0, 1)
// number of requests that were attempted to be spooled to disk
STUI(asynclog_requests_rate, 0, 1)
// number of requests that were spooled successfully
STUI(asynclog_spool_success_rate, 0, 1)
// number of requests sending to Axon proxy service
STUI(axon_proxy_request_success_rate, 0, 1)
// number of requests sending to Axon proxy service
STUI(axon_proxy_request_fail_rate, 0, 1)
#undef GROUP
#define GROUP ods_stats | basic_stats
// Average number of requests waiting in OLR at any given time
STAT(outstanding_route_get_avg_queue_size, stat_double, 0, .dbl = 0.0)
STAT(outstanding_route_update_avg_queue_size, stat_double, 0, .dbl = 0.0)
// Average time a request had to wait in OLR
STAT(outstanding_route_get_avg_wait_time_sec, stat_double, 0, .dbl = 0.0)
STAT(outstanding_route_update_avg_wait_time_sec, stat_double, 0, .dbl = 0.0)
#undef GROUP
#define GROUP rate_stats
// OutstandingLimitRoute queue-related helper stats
STUI(outstanding_route_get_reqs_queued_helper, 0, 0)
STUI(outstanding_route_update_reqs_queued_helper, 0, 0)
STUI(outstanding_route_get_wait_time_sum_us, 0, 0)
STUI(outstanding_route_update_wait_time_sum_us, 0, 0)
// Retransmission per byte helper stats
STUI(retrans_per_kbyte_sum, 0, 0)
STUI(retrans_num_total, 0, 0)
#undef GROUP

/**
 * Stats about the process
 */
#define GROUP ods_stats | basic_stats
STUI(ps_rss, 0, 0)
#undef GROUP
#define GROUP basic_stats | detailed_stats
STAT(rusage_system, stat_double, 0, .dbl = 0.0)
STAT(rusage_user, stat_double, 0, .dbl = 0.0)
STUI(ps_num_minor_faults, 0, 0)
STUI(ps_num_major_faults, 0, 0)
STAT(ps_user_time_sec, stat_double, 0, .dbl = 0.0)
STAT(ps_system_time_sec, stat_double, 0, .dbl = 0.0)
STUI(ps_vsize, 0, 0)
#undef GROUP

/**
 * Stats about fibers
 */
#define GROUP ods_stats | basic_stats
STUI(fibers_allocated, 0, 0)
STUI(fibers_pool_size, 0, 0)
STUI(fibers_stack_high_watermark, 0, 0)
#undef GROUP

/**
 * Stats about routing
 */
#define GROUP ods_stats | basic_stats
// avg time spent for asynclog spooling
STAT(asynclog_duration_us, stat_double, 0, .dbl = 0.0)
// avg time spent for appending to axon proxy
STAT(axon_proxy_duration_us, stat_double, 0, .dbl = 0.0)
// Number of proxy threads
STUI(num_proxies, 0, 1)
// Proxy requests that are currently being routed.
STUI(proxy_reqs_processing, 0, 1)
// Proxy requests queued up and not routed yet
STUI(proxy_reqs_waiting, 0, 1)
// At least one MPMC queue between clients and proxy is full
STUI(proxy_queue_full, 0, 1)
// All MPMC queues between clients and proxies are full
STUI(proxy_queues_all_full, 0, 1)
// number of request routed using McBucketRoute
STUI(bucketized_routing, 0, 1)
// distribution stats
STUI(distribution_axon_write_success, 0, 1)
STUI(distribution_axon_write_failed, 0, 1)
STUI(distribution_async_spool_failed, 0, 1)
STUI(distribution_replay_no_source, 0, 1)
STUI(distribution_replay_xregion_directed, 0, 1)
STUI(distribution_replay_xregion_directed_no_prefix_error, 0, 1)
STUI(distribution_replay_xregion_broadcast, 0, 1)
STUI(distribution_replay_other, 0, 1)
STAT(client_queue_notify_period, stat_double, 0, .dbl = 0.0)
#undef GROUP
#define GROUP ods_stats | detailed_stats
STUI(proxy_request_num_outstanding, 0, 1)
#undef GROUP
#define GROUP detailed_stats
STUI(dev_null_requests, 0, 1)
#undef GROUP
#define GROUP ods_stats | detailed_stats | count_stats
STUI(rate_limited_log_count, 0, 1)
STUI(load_balancer_load_reset_count, 0, 1)
#undef GROUP
#define GROUP count_stats
STUI(request_sent_count, 0, 1)
STUI(request_error_count, 0, 1)
STUI(request_success_count, 0, 1)
STUI(request_replied_count, 0, 1)
#undef GROUP
#define GROUP ods_stats | detailed_stats | rate_stats
STUI(request_sent, 0, 1)
STUI(request_has_crypto_auth_token, 0, 1)
STUI(request_error, 0, 1)
STUI(request_success, 0, 1)
STUI(request_replied, 0, 1)
STUI(client_queue_notifications, 0, 1)
STUI(failover_all, 0, 1)
STUI(failover_all_failed, 0, 1)
STUI(failover_conditional, 0, 1)
STUI(failover_rate_limited, 0, 1)
STUI(failover_inorder_policy, 0, 1)
STUI(failover_inorder_policy_failed, 0, 1)
STUI(failover_least_failures_policy, 0, 1)
STUI(failover_least_failures_policy_failed, 0, 1)
STUI(failover_deterministic_order_policy, 0, 1)
STUI(failover_deterministic_order_policy_failed, 0, 1)
STUI(failover_rendezvous_policy, 0, 1)
STUI(failover_rendezvous_policy_failed, 0, 1)
STUI(custom_policy_attempts, 0, 1)
STUI(failover_custom_policy, 0, 1)
STUI(failover_custom_policy_failed, 0, 1)
STUI(failover_custom_policy_attempts, 0, 1)
STUI(failover_custom_deadline_exceeded, 0, 1)
STUI(failover_custom_limit_reached, 0, 1)
STUI(failover_custom_master_region, 0, 1)
STUI(failover_custom_master_region_skipped, 0, 1)
STUI(failover_custom_master_region_invalid, 0, 1)
STUI(failover_custom_db_enabled_region, 0, 1)
STUI(failover_custom_db_enabled_regions_missing, 0, 1)
STUI(failover_custom_db_enabled_regions_empty, 0, 1)
STUI(failover_custom_db_disabled_all_regions, 0, 1)
STUI(before_request_latency_injected, 0, 1)
STUI(after_request_latency_injected, 0, 1)
STUI(before_latency_injected, 0, 1)
STUI(after_latency_injected, 0, 1)
STUI(total_latency_injected, 0, 1)
#undef GROUP
#define GROUP ods_stats | count_stats
STUI(result_error_count, 0, 1)
STUI(result_error_all_count, 0, 1)
STUI(result_connect_error_count, 0, 1)
STUI(result_connect_error_all_count, 0, 1)
STUI(result_connect_timeout_count, 0, 1)
STUI(result_connect_timeout_all_count, 0, 1)
STUI(result_data_timeout_count, 0, 1)
STUI(result_data_timeout_all_count, 0, 1)
STUI(result_busy_count, 0, 1)
STUI(result_busy_all_count, 0, 1)
STUI(result_tko_count, 0, 1)
STUI(result_tko_all_count, 0, 1)
STUI(result_client_error_count, 0, 1)
STUI(result_client_error_all_count, 0, 1)
STUI(result_local_error_count, 0, 1)
STUI(result_local_error_all_count, 0, 1)
STUI(result_remote_error_count, 0, 1)
STUI(result_remote_error_all_count, 0, 1)
STUI(result_deadline_exceeded_error_count, 0, 1)
STUI(result_deadline_exceeded_error_all_count, 0, 1)
STUI(failover_num_collisions, 0, 1)
STUI(failover_num_failed_domain_collisions, 0, 1)
STUI(failover_same_failure_domain, 0, 1)
STUI(failover_conditional_count, 0, 1)
STUI(failover_policy_result_error, 0, 1)
STUI(failover_all_failed_count, 0, 1)
STUI(failover_policy_tko_error, 0, 1)
STUI(custom_policy_attempts_count, 0, 1)
STUI(failover_custom_deadline_exceeded_count, 0, 1)
STUI(failover_custom_limit_reached_count, 0, 1)
STUI(failover_custom_master_region_count, 0, 1)
STUI(failover_custom_master_region_skipped_count, 0, 1)
STUI(dest_with_no_failure_domain_count, 0, 1)
STUI(failover_custom_db_enabled_region_count, 0, 1)
STUI(failover_custom_db_disabled_all_regions_count, 0, 1)
STUI(request_deadline_num_copy, 0, 1)
#undef GROUP
#define GROUP ods_stats | detailed_stats | rate_stats
STUI(final_result_error, 0, 1)
STUI(result_error, 0, 1)
STUI(result_error_all, 0, 1)
STUI(result_connect_error, 0, 1)
STUI(result_connect_error_all, 0, 1)
STUI(result_connect_timeout, 0, 1)
STUI(result_connect_timeout_all, 0, 1)
STUI(result_data_timeout, 0, 1)
STUI(result_data_timeout_all, 0, 1)
STUI(result_busy, 0, 1)
STUI(result_busy_all, 0, 1)
STUI(result_tko, 0, 1)
STUI(result_tko_all, 0, 1)
STUI(result_client_error, 0, 1)
STUI(result_client_error_all, 0, 1)
STUI(result_local_error, 0, 1)
STUI(result_local_error_all, 0, 1)
STUI(result_remote_error, 0, 1)
STUI(result_remote_error_all, 0, 1)
STUI(result_deadline_exceeded_error, 0, 1)
STUI(result_deadline_exceeded_error_all, 0, 1)
#undef GROUP

/**
 * Stats about RPC (AsyncMcClient)
 */
#define GROUP basic_stats | rate_stats
STUI(destination_batches_sum, 0, 1)
STUI(destination_requests_sum, 0, 1)
#undef GROUP
#define GROUP basic_stats | detailed_stats
// count the dirty requests (i.e. requests that needed reserialization)
STUI(destination_reqs_dirty_buffer_sum, 0, 1)
STUI(destination_reqs_total_sum, 0, 1)
#undef GROUP
#define GROUP ods_stats | basic_stats | max_max_stats
STUI(retrans_per_kbyte_max, 0, 1)
STUI(max_num_tko, 0, 1)
#undef GROUP
#define GROUP ods_stats | basic_stats
// Total reqs in AsyncMcClient yet to be sent to memcache.
STUI(destination_pending_reqs, 0, 1)
// Total reqs waiting for reply from server.
STUI(destination_inflight_reqs, 0, 1)
STUI(destination_inflight_shadow_reqs, 0, 1)
STAT(destination_batch_size, stat_double, 0, .dbl = 0.0)
STAT(destination_reqs_dirty_buffer_ratio, stat_double, 0, .dbl = 0.0)
// duration of the rpc call
STAT(duration_us, stat_double, 0, .dbl = 0.0)
// Duration microseconds, broken down by request type (get-like and update-like)
STAT(duration_get_us, stat_double, 0, .dbl = 0.0)
STAT(duration_update_us, stat_double, 0, .dbl = 0.0)
#undef GROUP
#define GROUP ods_stats | basic_stats | max_stats
STUI(destination_max_pending_reqs, 0, 1)
STUI(destination_max_inflight_reqs, 0, 1)
STUI(destination_max_inflight_shadow_reqs, 0, 1)
#undef GROUP
#define GROUP ods_stats | count_stats
// stats about sending lease-set to where the lease-get came from
STUI(redirected_lease_set_count, 0, 1)
#undef GROUP
#define GROUP ods_stats | detailed_stats | rate_stats
// This is the total number of times we called write to a socket.
STUI(num_socket_writes, 0, 1)
// This is the number of times we couldn't complete a full write.
// Note: this can be larger than num_socket_writes. E.g. if we called write
// on a huge buffer, we would partially write it many times.
STUI(num_socket_partial_writes, 0, 1)
#undef GROUP
#define GROUP detailed_stats | rate_stats
STUI(replies_compressed, 0, 1)
STUI(replies_not_compressed, 0, 1)
STUI(reply_traffic_before_compression, 0, 1)
STUI(reply_traffic_after_compression, 0, 1)
#undef GROUP
#define GROUP ods_stats | detailed_stats
STAT(retrans_per_kbyte_avg, stat_double, 0, .dbl = 0.0)
#undef GROUP

/**
 * Stats about configuration
 */
#define GROUP ods_stats | detailed_stats
STUI(config_age, 0, 0)
STUI(config_last_attempt, 0, 0)
STUI(config_last_success, 0, 0)
STUI(config_failures, 0, 0)
STUI(configs_from_disk, 0, 0)
STUI(config_partial_reconfig_attempt, 0, 0)
STUI(config_partial_reconfig_success, 0, 0)
STUI(config_full_attempt, 0, 0)
#undef GROUP

/* Declaring external stats */

/* Stats for Web Fabrics (Memcache) Prefix ACL */
EXTERNAL_STAT(prefix_acl_active_entries_filter_hit)
EXTERNAL_STAT(prefix_acl_active_entries_filter_wrong_accessor)
EXTERNAL_STAT(prefix_acl_actual_accept)
EXTERNAL_STAT(prefix_acl_auth_accept)
EXTERNAL_STAT(prefix_acl_auth_deny)
EXTERNAL_STAT(prefix_acl_checker_create_failure)
EXTERNAL_STAT(prefix_acl_checker_create_success)
EXTERNAL_STAT(prefix_acl_checker_fetch_count)
EXTERNAL_STAT(prefix_acl_checks)
EXTERNAL_STAT(prefix_acl_critical_errors)
EXTERNAL_STAT(prefix_acl_dirty_hit)
EXTERNAL_STAT(prefix_acl_disabled_count)
EXTERNAL_STAT(prefix_acl_empty_accessor)
EXTERNAL_STAT(prefix_acl_invalid_count)
EXTERNAL_STAT(prefix_acl_match)
EXTERNAL_STAT(prefix_acl_permitted_to_all_hit)
EXTERNAL_STAT(prefix_acl_scuba_log_attempt)
EXTERNAL_STAT(prefix_acl_scuba_log_attempt_on_accept)
EXTERNAL_STAT(prefix_acl_scuba_log_attempt_on_deny)
EXTERNAL_STAT(prefix_acl_scuba_log_attempt_on_new_accessor_deny)
EXTERNAL_STAT(prefix_acl_scuba_log_ratelimit)
EXTERNAL_STAT(prefix_acl_scuba_log_success)
EXTERNAL_STAT(prefix_acl_timeout_fetch_count)
EXTERNAL_STAT(prefix_acl_underlying_acl_checker_calls)
EXTERNAL_STAT(prefix_acl_underlying_acl_checker_fail)
EXTERNAL_STAT(prefix_acl_underlying_acl_checker_pass)
EXTERNAL_STAT(prefix_acl_unenforced_deny)
EXTERNAL_STAT(prefix_acl_allowlist_acl_match)
EXTERNAL_STAT(prefix_acl_allowlist_acl_permitted_to_all)
EXTERNAL_STAT(prefix_acl_allowlist_acl_dirty)
EXTERNAL_STAT(prefix_acl_time_since_init_ms)
EXTERNAL_STAT(prefix_acl_init_duration_ms)
EXTERNAL_STAT(prefix_acl_fetch_count)
EXTERNAL_STAT(prefix_acl_category_count)
EXTERNAL_STAT(prefix_acl_acl_checker_failure_count)
EXTERNAL_STAT(prefix_acl_acl_checker_success_count)
EXTERNAL_STAT(prefix_acl_timeout_count)
EXTERNAL_STAT(prefix_acl_checker_count)
EXTERNAL_STAT(prefix_acl_count)
EXTERNAL_STAT(prefix_acl_refresh_attempts)
EXTERNAL_STAT(prefix_acl_concurrent_refresh_fail)
EXTERNAL_STAT(prefix_acl_refresh_success)
EXTERNAL_STAT(prefix_acl_refresh_timeout)
EXTERNAL_STAT(prefix_acl_refresh_error)
