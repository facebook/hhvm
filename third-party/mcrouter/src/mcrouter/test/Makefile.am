# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

SUBDIRS = cpp_unit_tests .

TESTS = \
  ../lib/config/test/mcrouter_config_test \
  ../lib/fbi/cpp/test/mcrouter_fbi_cpp_test \
  ../lib/fbi/test/mcrouter_fbi_test \
  ../lib/test/mcrouter_lib_test \
  ../routes/test/mcrouter_routes_test \
  cpp_unit_tests/mcrouter_test

# TODO: fix the test
# ../lib/network/test/mcrouter_network_test

if HAVE_PYTHON
TESTS += \
  test_allow_only_gets.py \
  test_ascii_error.py \
  test_ascii_multiget_mock.py \
  test_async_files.py \
  test_bad_params.py \
  test_config_params.py \
  test_const_shard_hash.py \
  test_custom_failover.py \
  test_empty_pool.py \
  test_flush_all.py \
  test_largeobj.py \
  test_logical_routing_policies.py \
  test_max_shadow_requests.py \
  test_mcpiper.py \
  test_mcrouter.py \
  test_mcrouter_basic.py \
  test_mcrouter_errors.py \
  test_mcrouter_sanity_mock.py \
  test_mcrouter_to_mcrouter_tko.py \
  test_memcache_router.py \
  test_migrated_failover.py \
  test_miss_on_error_arith_ops.py \
  test_modify_exptime.py \
  test_modify_key.py \
  test_named_handles.py \
  test_noreply.py \
  test_probe_timeout.py \
  test_rates.py \
  test_routing_prefixes.py \
  test_send_to_all_hosts.py \
  test_server_stats.py \
  test_service_info.py \
  test_shadow.py \
  test_shadow_route.py \
  test_shadow_with_file.py \
  test_shard_splits.py \
  test_slow_warmup.py \
  test_tko_inactive.py \
  test_tko_reconfigure.py \
  test_validate_config.py \
  test_warmup.py \
  test_wch3.py
endif

# TODO fix the test
# test_warmup2.py

TEST_EXTENSIONS = .py
PY_LOG_COMPILER = ./run_python_test.sh
AM_PY_LOG_FLAGS = $(PYTHON)
LOG_COMPILER = ./run_cpp_test.sh
