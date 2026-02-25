(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* used for validation below *)
let registered_keys : SSet.t ref = ref SSet.empty

let key (name : string) : string =
  registered_keys := SSet.add name !registered_keys;
  name

(* parser options *)

let is_systemlib = key "is_systemlib"

let disable_lval_as_an_expression = key "disable_lval_as_an_expression"

let disable_legacy_soft_typehints = key "disable_legacy_soft_typehints"

let const_default_func_args = key "const_default_func_args"

let const_default_lambda_args = key "const_default_lambda_args"

let const_static_props = key "const_static_props"

let abstract_static_props = key "abstract_static_props"

let disallow_func_ptrs_in_constants = key "disallow_func_ptrs_in_constants"

let disable_xhp_element_mangling = key "disable_xhp_element_mangling"

let disable_xhp_children_declarations = key "disable_xhp_children_declarations"

let enable_xhp_class_modifier = key "enable_xhp_class_modifier"

let interpret_soft_types_as_like_types =
  key "interpret_soft_types_as_like_types"

let disallow_static_constants_in_default_func_args =
  key "disallow_static_constants_in_default_func_args"

let auto_namespace_map = key "auto_namespace_map"

let everything_sdt = key "everything_sdt"

let keep_user_attributes = key "keep_user_attributes"

let deregister_php_stdlib = key "deregister_php_stdlib"

let union_intersection_type_hints = key "union_intersection_type_hints"

let disallow_silence = key "disallow_silence"

let allowed_decl_fixme_codes = key "allowed_decl_fixme_codes"

let disable_hh_ignore_error = key "disable_hh_ignore_error"

let enable_experimental_stx_features = key "enable_experimental_stx_features"

let package_support_multifile_tests = key "package_support_multifile_tests"

let enable_class_pointer_hint = key "enable_class_pointer_hint"

let disallow_non_annotated_memoize = key "disallow_non_annotated_memoize"

let treat_non_annotated_memoize_as_kbic =
  key "treat_non_annotated_memoize_as_kbic"

let ignore_string_methods = key "ignore_string_methods"

(* global options and typechecker options *)

let language_feature_logging = key "language_feature_logging"

let timeout = key "timeout"

let constraint_array_index_assign = key "constraint_array_index_assign"

let constraint_method_call = key "constraint_method_call"

let code_agnostic_fixme = key "code_agnostic_fixme"

let allowed_fixme_codes_strict = key "allowed_fixme_codes_strict"

let enable_experimental_tc_features = key "enable_experimental_tc_features"

let enable_tc_migration_flags = key "enable_tc_migration_flags"

let call_coeffects = key "call_coeffects"

let local_coeffects = key "local_coeffects"

let like_casts = key "like_casts"

let check_xhp_attribute = key "check_xhp_attribute"

let check_redundant_generics = key "check_redundant_generics"

let disallow_unresolved_type_variables =
  key "disallow_unresolved_type_variables"

let locl_cache_capacity = key "locl_cache_capacity"

let locl_cache_node_threshold = key "locl_cache_node_threshold"

let disallow_toplevel_requires = key "disallow_toplevel_requires"

let const_attribute = key "const_attribute"

let check_attribute_locations = key "check_attribute_locations"

let glean_reponame = key "glean_reponame"

let symbol_write_index_inherited_members =
  key "symbol_write_index_inherited_members"

let symbol_write_ownership = key "symbol_write_ownership"

let symbol_write_root_path = key "symbol_write_root_path"

let symbol_write_hhi_path = key "symbol_write_hhi_path"

let symbol_write_ignore_paths = key "symbol_write_ignore_paths"

let symbol_write_index_paths = key "symbol_write_index_paths"

let symbol_write_index_paths_file = key "symbol_write_index_paths_file"

let symbol_write_index_paths_file_output =
  key "symbol_write_index_paths_file_output"

let symbol_write_include_hhi = key "symbol_write_include_hhi"

let symbol_write_sym_hash_in = key "symbol_write_sym_hash_in"

let symbol_write_exclude_out = key "symbol_write_exclude_out"

let symbol_write_referenced_out = key "symbol_write_referenced_out"

let symbol_write_reindexed_out = key "symbol_write_reindexed_out"

let symbol_write_sym_hash_out = key "symbol_write_sym_hash_out"

let error_php_lambdas = key "error_php_lambdas"

let disallow_discarded_nullable_awaitables =
  key "disallow_discarded_nullable_awaitables"

let typecheck_sample_rate = key "typecheck_sample_rate"

let pessimise_builtins = key "pessimise_builtins"

let enable_no_auto_dynamic = key "enable_no_auto_dynamic"

let skip_check_under_dynamic = key "skip_check_under_dynamic"

let enable_function_references = key "enable_function_references"

let ignore_unsafe_cast = key "ignore_unsafe_cast"

let allowed_expression_tree_visitors = key "allowed_expression_tree_visitors"

let typeconst_concrete_concrete_error = key "typeconst_concrete_concrete_error"

let meth_caller_only_public_visibility =
  key "meth_caller_only_public_visibility"

let require_extends_implements_ancestors =
  key "require_extends_implements_ancestors"

let strict_value_equality = key "strict_value_equality"

let enforce_sealed_subclasses = key "enforce_sealed_subclasses"

let implicit_inherit_sdt = key "implicit_inherit_sdt"

let repo_stdlib_path = key "repo_stdlib_path"

let explicit_consistent_constructors = key "explicit_consistent_constructors"

let require_types_tco_require_types_class_consts =
  key "require_types_tco_require_types_class_consts"

let check_bool_for_condition = key "check_bool_for_condition"

let type_printer_fuel = key "type_printer_fuel"

let profile_top_level_definitions = key "profile_top_level_definitions"

let typecheck_if_name_matches_regexp = key "typecheck_if_name_matches_regexp"

let log_levels = key "log_levels"

let allowed_files_for_module_declarations =
  key "allowed_files_for_module_declarations"

let allow_all_files_for_module_declarations =
  key "allow_all_files_for_module_declarations"

let populate_dead_unsafe_cast_heap = key "populate_dead_unsafe_cast_heap"

let dump_tast_hashes = key "dump_tast_hashes"

let warnings_default_all = key "warnings_default_all"

let allowed_files_for_ignore_readonly = key "allowed_files_for_ignore_readonly"

let package_allow_typedef_violations = key "package_allow_typedef_violations"

let package_allow_classconst_violations =
  key "package_allow_classconst_violations"

let package_allow_reifiable_tconst_violations =
  key "package_allow_reifiable_tconst_violations"

let package_allow_reified_generics_violations =
  key "package_allow_reified_generics_violations"

let package_allow_all_tconst_violations =
  key "package_allow_all_tconst_violations"

let package_allow_all_generics_violations =
  key "package_allow_all_generics_violations"

let package_exclude_patterns = key "package_exclude_patterns"

let extended_reasons = key "extended_reasons"

let disable_physical_equality = key "disable_physical_equality"

let re_no_cache = key "re_no_cache"

let hh_distc_should_disable_trace_store =
  key "hh_distc_should_disable_trace_store"

let hh_distc_exponential_backoff_num_retries =
  key "hh_distc_exponential_backoff_num_retries"

let enable_abstract_method_optional_parameters =
  key "enable_abstract_method_optional_parameters"

let hack_warnings = key "hack_warnings"

let recursive_case_types = key "recursive_case_types"

let class_sub_classname = key "class_sub_classname"

let class_class_type = key "class_class_type"

let needs_concrete = key "needs_concrete"

let needs_concrete_override_check = key "needs_concrete_override_check"

let allow_class_string_cast = key "allow_class_string_cast"

let class_pointer_ban_classname_new = key "class_pointer_ban_classname_new"

let class_pointer_ban_classname_type_structure =
  key "class_pointer_ban_classname_type_structure"

let class_pointer_ban_classname_static_meth =
  key "class_pointer_ban_classname_static_meth"

let class_pointer_ban_classname_class_const =
  key "class_pointer_ban_classname_class_const"

let class_pointer_ban_class_array_key = key "class_pointer_ban_class_array_key"

let poly_function_pointers = key "poly_function_pointers"

let check_packages = key "check_packages"

let package_config_disable_transitivity_check =
  key "package_config_disable_transitivity_check"

let allow_require_package_on_interface_methods =
  key "allow_require_package_on_interface_methods"

(* server options *)

let version = key "version"

let packages_config_path = key "packages_config_path"

let gc_minor_heap_size = key "gc_minor_heap_size"

let gc_space_overhead = key "gc_space_overhead"

let sharedmem_global_size = key "sharedmem_global_size"

let sharedmem_heap_size = key "sharedmem_heap_size"

let sharedmem_hash_table_pow = key "sharedmem_hash_table_pow"

let sharedmem_log_level = key "sharedmem_log_level"

let sharedmem_sample_rate = key "sharedmem_sample_rate"

let sharedmem_compression = key "sharedmem_compression"

let sharedmem_dirs = key "sharedmem_dirs"

let shm_use_sharded_hashtbl = key "shm_use_sharded_hashtbl"

let shm_cache_size = key "shm_cache_size"

let sharedmem_minimum_available = key "sharedmem_minimum_available"

let ignored_paths = key "ignored_paths"

let extra_paths = key "extra_paths"

let untrusted_mode = key "untrusted_mode"

let formatter_override = key "formatter_override"

let load_script_timeout = key "load_script_timeout"

let warn_on_non_opt_build = key "warn_on_non_opt_build"

let ide_fall_back_to_full_index = key "ide_fall_back_to_full_index"

let naming_table_compression_level = key "naming_table_compression_level"

let naming_table_compression_threads = key "naming_table_compression_threads"

let warnings_generated_files = key "warnings_generated_files"

let disabled_warnings = key "disabled_warnings"

let current_saved_state_rollout_flag_index =
  key "current_saved_state_rollout_flag_index"

let deactivate_saved_state_rollout = key "deactivate_saved_state_rollout"

let override_hhconfig_hash = key "override_hhconfig_hash"

let silence_errors_under_dynamic = key "silence_errors_under_dynamic"

(* validation *)

type did_you_mean = Did_you_mean of string option

let all_keys = !registered_keys

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
