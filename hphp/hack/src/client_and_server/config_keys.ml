(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* used for validation below *)
let registered_keys : SSet.t ref = ref SSet.empty

let registered_hhconf_keys : SSet.t ref = ref SSet.empty

let key (name : string) : string =
  registered_keys := SSet.add name !registered_keys;
  name

(** Register an hh.conf config key (ServerLocalConfig). *)
let hhconf_key (name : string) : string =
  registered_hhconf_keys := SSet.add name !registered_hhconf_keys;
  name

(** Keys valid in .hhconfig files. *)
module Hhconfig = struct
  (* parser options *)

  let abstract_static_props = key "abstract_static_props"

  let allowed_decl_fixme_codes = key "allowed_decl_fixme_codes"

  let auto_namespace_map = key "auto_namespace_map"

  let const_default_func_args = key "const_default_func_args"

  let const_default_lambda_args = key "const_default_lambda_args"

  let const_static_props = key "const_static_props"

  let deregister_php_stdlib = key "deregister_php_stdlib"

  let disable_hh_ignore_error = key "disable_hh_ignore_error"

  let disable_lval_as_an_expression = key "disable_lval_as_an_expression"

  let disable_xhp_children_declarations =
    key "disable_xhp_children_declarations"

  let disable_xhp_element_mangling = key "disable_xhp_element_mangling"

  let disallow_bool_cast = key "disallow_bool_cast"

  let disallow_func_ptrs_in_constants = key "disallow_func_ptrs_in_constants"

  let disallow_non_annotated_memoize = key "disallow_non_annotated_memoize"

  let disallow_silence = key "disallow_silence"

  let disallow_static_constants_in_default_func_args =
    key "disallow_static_constants_in_default_func_args"

  let enable_class_pointer_hint = key "enable_class_pointer_hint"

  let enable_experimental_stx_features = key "enable_experimental_stx_features"

  let enable_xhp_class_modifier = key "enable_xhp_class_modifier"

  let everything_sdt = key "everything_sdt"

  let ignore_string_methods = key "ignore_string_methods"

  let interpret_soft_types_as_like_types =
    key "interpret_soft_types_as_like_types"

  let is_systemlib = key "is_systemlib"

  let keep_user_attributes = key "keep_user_attributes"

  let package_support_multifile_tests = key "package_support_multifile_tests"

  let treat_non_annotated_memoize_as_kbic =
    key "treat_non_annotated_memoize_as_kbic"

  let union_intersection_type_hints = key "union_intersection_type_hints"

  (* global options and typechecker options *)

  let allow_all_files_for_module_declarations =
    key "allow_all_files_for_module_declarations"

  let allow_class_string_cast = key "allow_class_string_cast"

  let allowed_expression_tree_visitors = key "allowed_expression_tree_visitors"

  let allowed_files_for_ignore_readonly =
    key "allowed_files_for_ignore_readonly"

  let allowed_files_for_module_declarations =
    key "allowed_files_for_module_declarations"

  let allowed_fixme_codes_strict = key "allowed_fixme_codes_strict"

  let call_coeffects = key "call_coeffects"

  let check_bool_for_condition = key "check_bool_for_condition"

  let check_redundant_generics = key "check_redundant_generics"

  let check_xhp_attribute = key "check_xhp_attribute"

  let class_class_type = key "class_class_type"

  let class_pointer_ban_class_array_key =
    key "class_pointer_ban_class_array_key"

  let class_pointer_ban_classname_class_const =
    key "class_pointer_ban_classname_class_const"

  let class_pointer_ban_classname_new = key "class_pointer_ban_classname_new"

  let class_pointer_ban_classname_static_meth =
    key "class_pointer_ban_classname_static_meth"

  let class_pointer_ban_classname_type_structure =
    key "class_pointer_ban_classname_type_structure"

  let class_sub_classname = key "class_sub_classname"

  let code_agnostic_fixme = key "code_agnostic_fixme"

  let const_attribute = key "const_attribute"

  let constraint_array_index_assign = key "constraint_array_index_assign"

  let constraint_method_call = key "constraint_method_call"

  let disable_physical_equality = key "disable_physical_equality"

  let disallow_discarded_nullable_awaitables =
    key "disallow_discarded_nullable_awaitables"

  let disallow_toplevel_requires = key "disallow_toplevel_requires"

  let disallow_unresolved_type_variables =
    key "disallow_unresolved_type_variables"

  let dump_tast_hashes = key "dump_tast_hashes"

  let enable_experimental_tc_features = key "enable_experimental_tc_features"

  let enable_no_auto_dynamic = key "enable_no_auto_dynamic"

  let enable_tc_migration_flags = key "enable_tc_migration_flags"

  let explicit_consistent_constructors = key "explicit_consistent_constructors"

  let extended_reasons = key "extended_reasons"

  let glean_reponame = key "glean_reponame"

  let hack_warnings = key "hack_warnings"

  let hh_distc_exponential_backoff_num_retries =
    key "hh_distc_exponential_backoff_num_retries"

  let hh_distc_should_disable_trace_store =
    key "hh_distc_should_disable_trace_store"

  let ignore_unsafe_cast = key "ignore_unsafe_cast"

  let implicit_inherit_sdt = key "implicit_inherit_sdt"

  let language_feature_logging = key "language_feature_logging"

  let like_casts = key "like_casts"

  let local_coeffects = key "local_coeffects"

  let locl_cache_capacity = key "locl_cache_capacity"

  let locl_cache_node_threshold = key "locl_cache_node_threshold"

  let log_levels = key "log_levels"

  let meth_caller_only_public_visibility =
    key "meth_caller_only_public_visibility"

  let needs_concrete = key "needs_concrete"

  let needs_concrete_override_check = key "needs_concrete_override_check"

  let package_allow_all_generics_violations =
    key "package_allow_all_generics_violations"

  let package_allow_as_expression_violations =
    key "package_allow_as_expression_violations"

  let package_allow_all_tconst_violations =
    key "package_allow_all_tconst_violations"

  let package_allow_classconst_violations =
    key "package_allow_classconst_violations"

  let package_allow_reifiable_tconst_violations =
    key "package_allow_reifiable_tconst_violations"

  let package_allow_typedef_violations = key "package_allow_typedef_violations"

  let package_exclude_patterns = key "package_exclude_patterns"

  let pessimise_builtins = key "pessimise_builtins"

  let poly_function_pointers = key "poly_function_pointers"

  let populate_dead_unsafe_cast_heap = key "populate_dead_unsafe_cast_heap"

  let profile_top_level_definitions = key "profile_top_level_definitions"

  let re_no_cache = key "re_no_cache"

  let recursive_case_types = key "recursive_case_types"

  let repo_stdlib_path = key "repo_stdlib_path"

  let require_extends_implements_ancestors =
    key "require_extends_implements_ancestors"

  let require_types_tco_require_types_class_consts =
    key "require_types_tco_require_types_class_consts"

  let silence_errors_under_dynamic = key "silence_errors_under_dynamic"

  let skip_check_under_dynamic = key "skip_check_under_dynamic"

  let strict_consistent_construct = key "strict_consistent_construct"

  let symbol_write_exclude_out = key "symbol_write_exclude_out"

  let symbol_write_hhi_path = key "symbol_write_hhi_path"

  let symbol_write_ignore_paths = key "symbol_write_ignore_paths"

  let symbol_write_include_hhi = key "symbol_write_include_hhi"

  let symbol_write_index_inherited_members =
    key "symbol_write_index_inherited_members"

  let symbol_write_index_paths = key "symbol_write_index_paths"

  let symbol_write_index_paths_file = key "symbol_write_index_paths_file"

  let symbol_write_index_paths_file_output =
    key "symbol_write_index_paths_file_output"

  let symbol_write_ownership = key "symbol_write_ownership"

  let symbol_write_referenced_out = key "symbol_write_referenced_out"

  let symbol_write_reindexed_out = key "symbol_write_reindexed_out"

  let symbol_write_root_path = key "symbol_write_root_path"

  let symbol_write_sym_hash_in = key "symbol_write_sym_hash_in"

  let symbol_write_sym_hash_out = key "symbol_write_sym_hash_out"

  let timeout = key "timeout"

  let typecheck_if_name_matches_regexp = key "typecheck_if_name_matches_regexp"

  let typecheck_sample_rate = key "typecheck_sample_rate"

  let typeconst_concrete_concrete_error =
    key "typeconst_concrete_concrete_error"

  let type_printer_fuel = key "type_printer_fuel"

  let warnings_default_all = key "warnings_default_all"

  let warnings_in_sandcastle = key "warnings_in_sandcastle"

  (* server options *)

  let current_saved_state_rollout_flag_index =
    key "current_saved_state_rollout_flag_index"

  let deactivate_saved_state_rollout = key "deactivate_saved_state_rollout"

  let disabled_warnings = key "disabled_warnings"

  let extra_paths = key "extra_paths"

  let formatter_override = key "formatter_override"

  let gc_minor_heap_size = key "gc_minor_heap_size"

  let gc_space_overhead = key "gc_space_overhead"

  let ide_fall_back_to_full_index = key "ide_fall_back_to_full_index"

  let ignored_paths = key "ignored_paths"

  let load_script_timeout = key "load_script_timeout"

  let naming_table_compression_level = key "naming_table_compression_level"

  let naming_table_compression_threads = key "naming_table_compression_threads"

  let packages_config_path = key "packages_config_path"

  let sharedmem_compression = key "sharedmem_compression"

  let sharedmem_dirs = key "sharedmem_dirs"

  let sharedmem_global_size = key "sharedmem_global_size"

  let sharedmem_hash_table_pow = key "sharedmem_hash_table_pow"

  let sharedmem_heap_size = key "sharedmem_heap_size"

  let sharedmem_log_level = key "sharedmem_log_level"

  let sharedmem_minimum_available = key "sharedmem_minimum_available"

  let sharedmem_sample_rate = key "sharedmem_sample_rate"

  let shm_cache_size = key "shm_cache_size"

  let shm_use_sharded_hashtbl = key "shm_use_sharded_hashtbl"

  let untrusted_mode = key "untrusted_mode"

  let version = key "version"

  let warn_on_non_opt_build = key "warn_on_non_opt_build"

  let warnings_generated_files = key "warnings_generated_files"
end

(** Keys valid in hh.conf (ServerLocalConfig).
    These keys are accepted by hh_server and hh_client via --config overrides,
    but NOT by hh_single_type_check. *)
module Hhconf = struct
  (* distc *)

  let distc_avoid_unnecessary_saved_state_work =
    hhconf_key "distc_avoid_unnecessary_saved_state_work"

  let enable_fanout_aware_distc = hhconf_key "enable_fanout_aware_distc"

  let hh_distc_fanout_full_init_threshold =
    hhconf_key "hh_distc_fanout_full_init_threshold"

  let hh_distc_fanout_threshold = hhconf_key "hh_distc_fanout_threshold"

  let use_compressed_dep_graph = hhconf_key "use_compressed_dep_graph"

  let use_distc = hhconf_key "use_distc"

  let use_distc_crawl_dircache = hhconf_key "use_distc_crawl_dircache"

  (* edenfs file watcher *)

  let edenfs_file_watcher_debug_logging =
    hhconf_key "edenfs_file_watcher_debug_logging"

  let edenfs_file_watcher_enabled = hhconf_key "edenfs_file_watcher_enabled"

  let edenfs_file_watcher_report_telemetry =
    hhconf_key "edenfs_file_watcher_report_telemetry"

  let edenfs_file_watcher_state_tracking =
    hhconf_key "edenfs_file_watcher_state_tracking"

  let edenfs_file_watcher_sync_queries_obey_deferral =
    hhconf_key "edenfs_file_watcher_sync_queries_obey_deferral"

  let edenfs_file_watcher_throttle_time_ms =
    hhconf_key "edenfs_file_watcher_throttle_time_ms"

  let edenfs_file_watcher_timeout_secs =
    hhconf_key "edenfs_file_watcher_timeout_secs"

  let edenfs_file_watcher_tracked_states =
    hhconf_key "edenfs_file_watcher_tracked_states"

  (* experiments / infrastructure *)

  let experiments = hhconf_key "experiments"

  let experiments_config_enabled = hhconf_key "experiments_config_enabled"

  let experiments_config_path = hhconf_key "experiments_config_path"

  let experiments_config_source = hhconf_key "experiments_config_source"

  let experiments_config_ttl_seconds =
    hhconf_key "experiments_config_ttl_seconds"

  let experiments_config_update = hhconf_key "experiments_config_update"

  let log_categories = hhconf_key "log_categories"

  let log_inference_constraints = hhconf_key "log_inference_constraints"

  let log_init_proc_stack_also_on_absent_from =
    hhconf_key "log_init_proc_stack_also_on_absent_from"

  let log_large_fanouts_threshold = hhconf_key "log_large_fanouts_threshold"

  let min_log_level = hhconf_key "min_log_level"

  let use_justknobs = hhconf_key "use_justknobs"

  (* IDE *)

  let autocomplete_sort_text = hhconf_key "autocomplete_sort_text"

  let go_to_implementation = hhconf_key "go_to_implementation"

  let ide_load_naming_table_on_disk = hhconf_key "ide_load_naming_table_on_disk"

  let ide_naming_table_update_threshold =
    hhconf_key "ide_naming_table_update_threshold"

  let ide_parser_cache = hhconf_key "ide_parser_cache"

  let ide_symbolindex_search_provider =
    hhconf_key "ide_symbolindex_search_provider"

  let lsp_invalidation = hhconf_key "lsp_invalidation"

  let lsp_sticky_quarantine = hhconf_key "lsp_sticky_quarantine"

  let symbolindex_quiet = hhconf_key "symbolindex_quiet"

  (* misc *)

  let dump_tasts = hhconf_key "dump_tasts"

  let eden_fetch_parallelism = hhconf_key "eden_fetch_parallelism"

  let package_config_strict_validation =
    hhconf_key "package_config_strict_validation"

  let rollout_group = hhconf_key "rollout_group"

  let workload_quantile = hhconf_key "workload_quantile"

  let zstd_decompress_by_file = hhconf_key "zstd_decompress_by_file"

  (* profiling *)

  let memtrace_dir = hhconf_key "memtrace_dir"

  let profile_decling = hhconf_key "profile_decling"

  let profile_desc = hhconf_key "profile_desc"

  let profile_log = hhconf_key "profile_log"

  let profile_owner = hhconf_key "profile_owner"

  let profile_slow_threshold = hhconf_key "profile_slow_threshold"

  let profile_type_check_duration_threshold =
    hhconf_key "profile_type_check_duration_threshold"

  let profile_type_check_memory_threshold_mb =
    hhconf_key "profile_type_check_memory_threshold_mb"

  let profile_type_check_twice = hhconf_key "profile_type_check_twice"

  (* provider backend *)

  let allow_unstable_features = hhconf_key "allow_unstable_features"

  let cache_remote_decls = hhconf_key "cache_remote_decls"

  let rust_elab = hhconf_key "rust_elab"

  let rust_provider_backend = hhconf_key "rust_provider_backend"

  (* saved state *)

  let load_state_natively_dirty_files_timeout =
    hhconf_key "load_state_natively_dirty_files_timeout"

  let load_state_natively_download_timeout =
    hhconf_key "load_state_natively_download_timeout"

  let load_state_natively_v4 = hhconf_key "load_state_natively_v4"

  let log_saved_state_age_and_distance =
    hhconf_key "log_saved_state_age_and_distance"

  let project_metadata_w_flags = hhconf_key "project_metadata_w_flags"

  let require_saved_state = hhconf_key "require_saved_state"

  let saved_state_cache_limit = hhconf_key "saved_state_cache_limit"

  let saved_state_manifold_api_key = hhconf_key "saved_state_manifold_api_key"

  let specify_manifold_api_key = hhconf_key "specify_manifold_api_key"

  let ss_force = hhconf_key "ss_force"

  let store_decls_in_saved_state = hhconf_key "store_decls_in_saved_state"

  let use_manifold_cython_client = hhconf_key "use_manifold_cython_client"

  let use_mini_state = hhconf_key "use_mini_state"

  let use_mini_state_when_indexing = hhconf_key "use_mini_state_when_indexing"

  (* server operations *)

  let attempt_fix_credentials = hhconf_key "attempt_fix_credentials"

  let consume_streaming_errors = hhconf_key "consume_streaming_errors"

  let cpu_priority = hhconf_key "cpu_priority"

  let defer_class_declaration_threshold =
    hhconf_key "defer_class_declaration_threshold"

  let disable_naming_table_fallback_loading =
    hhconf_key "disable_naming_table_fallback_loading"

  let enable_fuzzy_search = hhconf_key "enable_fuzzy_search"

  let enable_global_access_check = hhconf_key "enable_global_access_check"

  let enable_naming_table_fallback = hhconf_key "enable_naming_table_fallback"

  let enable_on_nfs = hhconf_key "enable_on_nfs"

  let enable_type_check_filter_files =
    hhconf_key "enable_type_check_filter_files"

  let extend_defs_per_file_bucket_size =
    hhconf_key "extend_defs_per_file_bucket_size"

  let fetch_remote_old_decls = hhconf_key "fetch_remote_old_decls"

  let heartbeat_interval = hhconf_key "heartbeat_interval"

  let hg_aware = hhconf_key "hg_aware"

  let hg_aware_parsing_restart_threshold =
    hhconf_key "hg_aware_parsing_restart_threshold"

  let hg_aware_redecl_restart_threshold =
    hhconf_key "hg_aware_redecl_restart_threshold"

  let hg_aware_recheck_restart_threshold =
    hhconf_key "hg_aware_recheck_restart_threshold"

  let idle_gc_slice = hhconf_key "idle_gc_slice"

  let informant_min_distance_restart =
    hhconf_key "informant_min_distance_restart"

  let interrupt_on_client = hhconf_key "interrupt_on_client"

  let interrupt_on_file_changes = hhconf_key "interrupt_on_file_changes"

  let interrupt_on_watchman = hhconf_key "interrupt_on_watchman"

  let io_priority = hhconf_key "io_priority"

  let longlived_workers = hhconf_key "longlived_workers"

  let max_purgatory_clients = hhconf_key "max_purgatory_clients"

  let max_workers = hhconf_key "max_workers"

  let naming_sqlite_path = hhconf_key "naming_sqlite_path"

  let num_local_workers = hhconf_key "num_local_workers"

  let populate_member_heaps = hhconf_key "populate_member_heaps"

  let prechecked_files = hhconf_key "prechecked_files"

  let predeclare_ide = hhconf_key "predeclare_ide"

  let produce_streaming_errors = hhconf_key "produce_streaming_errors"

  let remote_old_decls_no_limit = hhconf_key "remote_old_decls_no_limit"

  let search_chunk_size = hhconf_key "search_chunk_size"

  let shm_dirs = hhconf_key "shm_dirs"

  let skip_hierarchy_checks = hhconf_key "skip_hierarchy_checks"

  let skip_tast_checks = hhconf_key "skip_tast_checks"

  let tico_invalidate_files = hhconf_key "tico_invalidate_files"

  let tico_invalidate_smart = hhconf_key "tico_invalidate_smart"

  let trace_parsing = hhconf_key "trace_parsing"

  let type_decl_bucket_size = hhconf_key "type_decl_bucket_size"

  let use_dummy_informant = hhconf_key "use_dummy_informant"

  let use_full_fidelity_parser = hhconf_key "use_full_fidelity_parser"

  (* watchman *)

  let use_watchman = hhconf_key "use_watchman"

  let watchman_debug_logging = hhconf_key "watchman_debug_logging"

  let watchman_enabled = hhconf_key "watchman_enabled"

  let watchman_init_timeout = hhconf_key "watchman_init_timeout"

  let watchman_sockname = hhconf_key "watchman_sockname"

  let watchman_subscribe_v2 = hhconf_key "watchman_subscribe_v2"

  let watchman_synchronous_timeout = hhconf_key "watchman_synchronous_timeout"
end

(* validation *)

type did_you_mean = Did_you_mean of string option

let all_hhconfig_keys = !registered_keys

let all_hhconf_keys = !registered_hhconf_keys

let all_keys = SSet.union all_hhconfig_keys all_hhconf_keys

let validate_hhconfig_key ~(config_key : string) : (unit, did_you_mean) result =
  let all_keys_list = lazy (SSet.elements all_hhconfig_keys) in
  if SSet.mem config_key all_hhconfig_keys then
    Ok ()
  else
    let suggestion =
      String_utils.most_similar
        ~max_edit_distance:3
        config_key
        (Lazy.force all_keys_list)
        Fun.id
    in
    Error (Did_you_mean suggestion)

let validate_hhconf_key ~(config_key : string) : (unit, did_you_mean) result =
  let all_keys_list = lazy (SSet.elements all_hhconf_keys) in
  if SSet.mem config_key all_hhconf_keys then
    Ok ()
  else
    let suggestion =
      String_utils.most_similar
        ~max_edit_distance:3
        config_key
        (Lazy.force all_keys_list)
        Fun.id
    in
    Error (Did_you_mean suggestion)

let validate ~(config_key : string) : (unit, did_you_mean) result =
  let all_keys_list = lazy (SSet.elements all_keys) in
  if SSet.mem config_key all_keys then
    Ok ()
  else
    let suggestion =
      String_utils.most_similar
        ~max_edit_distance:3
        config_key
        (Lazy.force all_keys_list)
        Fun.id
    in
    Error (Did_you_mean suggestion)
