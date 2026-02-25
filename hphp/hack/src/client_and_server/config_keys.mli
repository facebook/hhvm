(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * Configuration keys that can appear in:
 * - .hhconfig
 * - --config command-line options (which override .hhconfig values)
 *
 * To add a new config option:
 * 1. Add a new key definition in config_keys.ml using the [key] function
 * 2. Use the key constant in serverConfig.ml instead of a raw string literal
 *
 *)

(* parser options *)

val is_systemlib : string

val disable_lval_as_an_expression : string

val disable_legacy_soft_typehints : string

val const_default_func_args : string

val const_default_lambda_args : string

val const_static_props : string

val abstract_static_props : string

val disallow_func_ptrs_in_constants : string

val disable_xhp_element_mangling : string

val disable_xhp_children_declarations : string

val enable_xhp_class_modifier : string

val interpret_soft_types_as_like_types : string

val disallow_static_constants_in_default_func_args : string

val auto_namespace_map : string

val everything_sdt : string

val keep_user_attributes : string

val deregister_php_stdlib : string

val union_intersection_type_hints : string

val disallow_silence : string

val allowed_decl_fixme_codes : string

val disable_hh_ignore_error : string

val enable_experimental_stx_features : string

val package_support_multifile_tests : string

val enable_class_pointer_hint : string

val disallow_non_annotated_memoize : string

val treat_non_annotated_memoize_as_kbic : string

val ignore_string_methods : string

(* global options and typechecker options *)

val language_feature_logging : string

val timeout : string

val constraint_array_index_assign : string

val constraint_method_call : string

val code_agnostic_fixme : string

val allowed_fixme_codes_strict : string

val enable_experimental_tc_features : string

val enable_tc_migration_flags : string

val call_coeffects : string

val local_coeffects : string

val like_casts : string

val check_xhp_attribute : string

val check_redundant_generics : string

val disallow_unresolved_type_variables : string

val locl_cache_capacity : string

val locl_cache_node_threshold : string

val disallow_toplevel_requires : string

val const_attribute : string

val check_attribute_locations : string

val glean_reponame : string

val symbol_write_index_inherited_members : string

val symbol_write_ownership : string

val symbol_write_root_path : string

val symbol_write_hhi_path : string

val symbol_write_ignore_paths : string

val symbol_write_index_paths : string

val symbol_write_index_paths_file : string

val symbol_write_index_paths_file_output : string

val symbol_write_include_hhi : string

val symbol_write_sym_hash_in : string

val symbol_write_exclude_out : string

val symbol_write_referenced_out : string

val symbol_write_reindexed_out : string

val symbol_write_sym_hash_out : string

val error_php_lambdas : string

val disallow_discarded_nullable_awaitables : string

val typecheck_sample_rate : string

val pessimise_builtins : string

val enable_no_auto_dynamic : string

val skip_check_under_dynamic : string

val enable_function_references : string

val ignore_unsafe_cast : string

val allowed_expression_tree_visitors : string

val typeconst_concrete_concrete_error : string

val meth_caller_only_public_visibility : string

val require_extends_implements_ancestors : string

val strict_value_equality : string

val enforce_sealed_subclasses : string

val implicit_inherit_sdt : string

val repo_stdlib_path : string

val explicit_consistent_constructors : string

val require_types_tco_require_types_class_consts : string

val check_bool_for_condition : string

val type_printer_fuel : string

val profile_top_level_definitions : string

val typecheck_if_name_matches_regexp : string

val log_levels : string

val allowed_files_for_module_declarations : string

val allow_all_files_for_module_declarations : string

val populate_dead_unsafe_cast_heap : string

val dump_tast_hashes : string

val warnings_default_all : string

val allowed_files_for_ignore_readonly : string

val package_allow_typedef_violations : string

val package_allow_classconst_violations : string

val package_allow_reifiable_tconst_violations : string

val package_allow_reified_generics_violations : string

val package_allow_all_tconst_violations : string

val package_allow_all_generics_violations : string

val package_exclude_patterns : string

val extended_reasons : string

val disable_physical_equality : string

val re_no_cache : string

val hh_distc_should_disable_trace_store : string

val hh_distc_exponential_backoff_num_retries : string

val enable_abstract_method_optional_parameters : string

val hack_warnings : string

val recursive_case_types : string

val class_sub_classname : string

val class_class_type : string

val needs_concrete : string

val needs_concrete_override_check : string

val allow_class_string_cast : string

val class_pointer_ban_classname_new : string

val class_pointer_ban_classname_type_structure : string

val class_pointer_ban_classname_static_meth : string

val class_pointer_ban_classname_class_const : string

val class_pointer_ban_class_array_key : string

val poly_function_pointers : string

val check_packages : string

val package_config_disable_transitivity_check : string

val allow_require_package_on_interface_methods : string

(* server options *)

val version : string

val packages_config_path : string

val gc_minor_heap_size : string

val gc_space_overhead : string

val sharedmem_global_size : string

val sharedmem_heap_size : string

val sharedmem_hash_table_pow : string

val sharedmem_log_level : string

val sharedmem_sample_rate : string

val sharedmem_compression : string

val sharedmem_dirs : string

val shm_use_sharded_hashtbl : string

val shm_cache_size : string

val sharedmem_minimum_available : string

val ignored_paths : string

val extra_paths : string

val untrusted_mode : string

val formatter_override : string

val load_script_timeout : string

val warn_on_non_opt_build : string

val ide_fall_back_to_full_index : string

val naming_table_compression_level : string

val naming_table_compression_threads : string

val warnings_generated_files : string

val disabled_warnings : string

val current_saved_state_rollout_flag_index : string

val deactivate_saved_state_rollout : string

val override_hhconfig_hash : string

val silence_errors_under_dynamic : string

(* validation *)

(** When a configuration key looks like a typo, we make a suggestion *)
type did_you_mean = Did_you_mean of string option

val validate : config_key:string -> (unit, did_you_mean) result
