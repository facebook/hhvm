// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::gen::global_options::GlobalOptions;
use crate::i_set;
use crate::s_map;
use crate::s_set;

const DEFAULT: GlobalOptions<'_> = GlobalOptions {
    tco_experimental_features: s_set::SSet::empty(),
    tco_migration_flags: s_set::SSet::empty(),
    tco_num_local_workers: None,
    tco_parallel_type_checking_threshold: 10,
    tco_max_typechecker_worker_memory_mb: None,
    tco_defer_class_declaration_threshold: None,
    tco_prefetch_deferred_files: false,
    tco_remote_type_check_threshold: 1_000_000,
    tco_remote_type_check: false,
    tco_remote_worker_key: None,
    tco_remote_check_id: None,
    tco_remote_max_batch_size: 8000,
    tco_remote_min_batch_size: 5000,
    tco_num_remote_workers: 0,
    so_remote_version_specifier: None,
    so_remote_worker_vfs_checkout_threshold: 0,
    so_naming_sqlite_path: None,
    po_auto_namespace_map: &[],
    po_codegen: false,
    po_deregister_php_stdlib: false,
    po_disallow_toplevel_requires: false,
    po_allow_unstable_features: false,
    tco_log_large_fanouts_threshold: None,
    tco_log_inference_constraints: false,
    tco_language_feature_logging: false,
    tco_timeout: 0,
    tco_disallow_invalid_arraykey: false,
    tco_disallow_byref_dynamic_calls: false,
    tco_disallow_byref_calls: true,
    allowed_fixme_codes_strict: i_set::ISet::empty(),
    log_levels: s_map::SMap::empty(),
    po_disable_lval_as_an_expression: false,
    tco_shallow_class_decl: false,
    tco_force_shallow_decl_fanout: false,
    tco_remote_old_decls_no_limit: false,
    tco_force_load_hot_shallow_decls: false,
    tco_fetch_remote_old_decls: false,
    tco_populate_member_heaps: true,
    tco_skip_hierarchy_checks: false,
    tco_skip_tast_checks: false,
    tco_like_type_hints: false,
    tco_union_intersection_type_hints: false,
    tco_coeffects: true,
    tco_coeffects_local: true,
    tco_strict_contexts: true,
    tco_like_casts: false,
    tco_simple_pessimize: 0.0,
    tco_complex_coercion: false,
    tco_check_xhp_attribute: false,
    tco_check_redundant_generics: false,
    tco_disallow_unresolved_type_variables: false,
    po_enable_class_level_where_clauses: false,
    po_disable_legacy_soft_typehints: true,
    po_allowed_decl_fixme_codes: i_set::ISet::empty(),
    po_allow_new_attribute_syntax: false,
    tco_global_inference: false,
    tco_gi_reinfer_types: &[],
    tco_ordered_solving: false,
    tco_const_static_props: false,
    po_disable_legacy_attribute_syntax: false,
    tco_const_attribute: false,
    po_const_default_func_args: false,
    po_const_default_lambda_args: false,
    po_disallow_silence: false,
    po_abstract_static_props: false,
    po_parser_errors_only: false,
    tco_check_attribute_locations: true,
    po_disallow_func_ptrs_in_constants: false,
    tco_error_php_lambdas: false,
    tco_disallow_discarded_nullable_awaitables: false,
    po_enable_xhp_class_modifier: false,
    po_disable_xhp_element_mangling: false,
    po_disable_xhp_children_declarations: false,
    glean_service: "",
    glean_hostname: "",
    glean_port: 0,
    glean_reponame: "",
    symbol_write_ownership: false,
    symbol_write_root_path: "",
    symbol_write_hhi_path: "",
    symbol_write_ignore_paths: &[],
    symbol_write_index_paths: &[],
    symbol_write_index_paths_file: None,
    symbol_write_include_hhi: true,
    symbol_write_index_paths_file_output: None,
    po_enable_enum_classes: true,
    po_disable_hh_ignore_error: false,
    tco_enable_systemlib_annotations: false,
    tco_higher_kinded_types: false,
    tco_method_call_inference: false,
    tco_report_pos_from_reason: false,
    tco_typecheck_sample_rate: 1.0,
    tco_enable_sound_dynamic: false,
    po_disallow_fun_and_cls_meth_pseudo_funcs: false,
    po_disallow_inst_meth: false,
    tco_use_direct_decl_parser: true,
    tco_ifc_enabled: &[],
    tco_global_write_check_enabled: &[],
    tco_global_write_check_functions_enabled: s_set::SSet::empty(),
    po_enable_enum_supertyping: true,
    po_interpret_soft_types_as_like_types: false,
    tco_enable_strict_string_concat_interp: false,
    tco_ignore_unsafe_cast: false,
    tco_no_parser_readonly_check: false,
    tco_enable_expression_trees: false,
    tco_enable_modules: false,
    tco_allowed_expression_tree_visitors: &[],
    tco_math_new_code: false,
    tco_typeconst_concrete_concrete_error: false,
    tco_enable_strict_const_semantics: 0,
    tco_strict_wellformedness: 0,
    tco_meth_caller_only_public_visibility: true,
    tco_require_extends_implements_ancestors: false,
    tco_strict_value_equality: false,
    tco_enforce_sealed_subclasses: false,
    tco_everything_sdt: false,
    tco_pessimise_builtins: false,
    tco_enable_disk_heap: false,
    tco_explicit_consistent_constructors: 0,
    tco_require_types_class_consts: 0,
    tco_type_printer_fuel: 100,
    tco_log_saved_state_age_and_distance: false,
    tco_specify_manifold_api_key: false,
    tco_saved_state_manifold_api_key: None,
    tco_profile_top_level_definitions: false,
    tco_allow_all_files_for_module_declarations: false,
    tco_allowed_files_for_module_declarations: &[],
    tco_use_manifold_cython_client: false,
    tco_record_fine_grained_dependencies: false,
    tco_loop_iteration_upper_bound: None,
    tco_expression_tree_virtualize_functions: false,
    tco_substitution_mutation: false,
};

impl GlobalOptions<'static> {
    pub const DEFAULT: &'static Self = &DEFAULT;

    pub const fn default_ref() -> &'static Self {
        Self::DEFAULT
    }
}

impl Default for &GlobalOptions<'_> {
    fn default() -> Self {
        GlobalOptions::default_ref()
    }
}

impl Eq for GlobalOptions<'_> {}

impl std::hash::Hash for GlobalOptions<'_> {
    fn hash<H>(&self, _: &mut H) {
        unimplemented!()
    }
}

impl no_pos_hash::NoPosHash for GlobalOptions<'_> {
    fn hash<H>(&self, _: &mut H) {
        unimplemented!()
    }
}

impl Ord for GlobalOptions<'_> {
    fn cmp(&self, _: &Self) -> std::cmp::Ordering {
        unimplemented!()
    }
}

impl GlobalOptions<'_> {
    pub fn clone_in<'a>(&self, arena: &'a bumpalo::Bump) -> GlobalOptions<'a> {
        let tco_experimental_features: s_set::SSet<'a> = s_set::SSet::from(
            arena,
            self.tco_experimental_features
                .into_iter()
                .map(|s| &*arena.alloc_str(s)),
        );
        let tco_migration_flags: s_set::SSet<'a> = s_set::SSet::from(
            arena,
            self.tco_migration_flags
                .into_iter()
                .map(|s| &*arena.alloc_str(s)),
        );
        let tco_num_local_workers = self.tco_num_local_workers.clone();
        let tco_parallel_type_checking_threshold = self.tco_parallel_type_checking_threshold;
        let tco_max_typechecker_worker_memory_mb = self.tco_max_typechecker_worker_memory_mb;
        let tco_defer_class_declaration_threshold = self.tco_defer_class_declaration_threshold;
        let tco_prefetch_deferred_files = self.tco_prefetch_deferred_files;
        let tco_remote_type_check_threshold = self.tco_remote_type_check_threshold;
        let tco_remote_type_check = self.tco_remote_type_check;
        let tco_remote_worker_key: Option<&'a str> =
            self.tco_remote_worker_key.map(|s| &*arena.alloc_str(s));
        let tco_remote_check_id: Option<&'a str> =
            self.tco_remote_check_id.map(|s| &*arena.alloc_str(s));
        let tco_remote_max_batch_size = self.tco_remote_max_batch_size;
        let tco_remote_min_batch_size = self.tco_remote_min_batch_size;
        let tco_num_remote_workers = self.tco_num_remote_workers;
        let so_remote_version_specifier: Option<&'a str> = self
            .so_remote_version_specifier
            .map(|s| &*arena.alloc_str(s));
        let so_remote_worker_vfs_checkout_threshold = self.so_remote_worker_vfs_checkout_threshold;
        let so_naming_sqlite_path: Option<&'a str> =
            self.so_naming_sqlite_path.map(|s| &*arena.alloc_str(s));
        let mut po_ns_map =
            bumpalo::collections::Vec::with_capacity_in(self.po_auto_namespace_map.len(), arena);
        po_ns_map.extend(
            self.po_auto_namespace_map
                .iter()
                .map(|(a, b)| (&*arena.alloc_str(a), &*arena.alloc_str(b))),
        );
        let po_auto_namespace_map: &'a [(&'a str, &'a str)] = po_ns_map.into_bump_slice();
        let po_codegen = self.po_codegen;
        let po_deregister_php_stdlib = self.po_deregister_php_stdlib;
        let po_disallow_toplevel_requires = self.po_disallow_toplevel_requires;
        let po_allow_unstable_features = self.po_allow_unstable_features;
        let tco_log_large_fanouts_threshold = self.tco_log_large_fanouts_threshold;
        let tco_log_inference_constraints = self.tco_log_inference_constraints;
        let tco_language_feature_logging = self.tco_language_feature_logging;
        let tco_timeout = self.tco_timeout;
        let tco_disallow_invalid_arraykey = self.tco_disallow_invalid_arraykey;
        let tco_disallow_byref_dynamic_calls = self.tco_disallow_byref_dynamic_calls;
        let tco_disallow_byref_calls = self.tco_disallow_byref_calls;
        let allowed_fixme_codes_strict: i_set::ISet<'a> =
            i_set::ISet::from(arena, self.allowed_fixme_codes_strict.into_iter().copied());
        let log_levels: s_map::SMap<'a, isize> = s_map::SMap::from(
            arena,
            self.log_levels
                .into_iter()
                .map(|(&k, &v)| (&*arena.alloc_str(k), v)),
        );
        let po_disable_lval_as_an_expression = self.po_disable_lval_as_an_expression;
        let tco_shallow_class_decl = self.tco_shallow_class_decl;
        let tco_remote_old_decls_no_limit = self.tco_remote_old_decls_no_limit;
        let tco_force_shallow_decl_fanout = self.tco_force_shallow_decl_fanout;
        let tco_fetch_remote_old_decls = self.tco_fetch_remote_old_decls;
        let tco_force_load_hot_shallow_decls = self.tco_force_load_hot_shallow_decls;
        let tco_populate_member_heaps = self.tco_populate_member_heaps;
        let tco_skip_hierarchy_checks = self.tco_skip_hierarchy_checks;
        let tco_skip_tast_checks = self.tco_skip_tast_checks;
        let tco_like_type_hints = self.tco_like_type_hints;
        let tco_union_intersection_type_hints = self.tco_union_intersection_type_hints;
        let tco_coeffects = self.tco_coeffects;
        let tco_coeffects_local = self.tco_coeffects_local;
        let tco_strict_contexts = self.tco_strict_contexts;
        let tco_like_casts = self.tco_like_casts;
        let tco_simple_pessimize = self.tco_simple_pessimize;
        let tco_complex_coercion = self.tco_complex_coercion;
        let tco_check_xhp_attribute = self.tco_check_xhp_attribute;
        let tco_check_redundant_generics = self.tco_check_redundant_generics;
        let tco_disallow_unresolved_type_variables = self.tco_disallow_unresolved_type_variables;
        let po_enable_class_level_where_clauses = self.po_enable_class_level_where_clauses;
        let po_disable_legacy_soft_typehints = self.po_disable_legacy_soft_typehints;
        let po_allowed_decl_fixme_codes: i_set::ISet<'a> =
            i_set::ISet::from(arena, self.po_allowed_decl_fixme_codes.into_iter().copied());
        let po_allow_new_attribute_syntax = self.po_allow_new_attribute_syntax;
        let tco_global_inference = self.tco_global_inference;
        let mut gi_reinfer_types =
            bumpalo::collections::Vec::with_capacity_in(self.tco_gi_reinfer_types.len(), arena);
        gi_reinfer_types.extend(
            self.tco_gi_reinfer_types
                .iter()
                .map(|s| &*arena.alloc_str(s)),
        );
        let tco_gi_reinfer_types: &'a [&'a str] = gi_reinfer_types.into_bump_slice();
        let tco_ordered_solving = self.tco_ordered_solving;
        let tco_const_static_props = self.tco_const_static_props;
        let po_disable_legacy_attribute_syntax = self.po_disable_legacy_attribute_syntax;
        let tco_const_attribute = self.tco_const_attribute;
        let po_const_default_func_args = self.po_const_default_func_args;
        let po_const_default_lambda_args = self.po_const_default_lambda_args;
        let po_disallow_silence = self.po_disallow_silence;
        let po_abstract_static_props = self.po_abstract_static_props;
        let po_parser_errors_only = self.po_parser_errors_only;
        let tco_check_attribute_locations = self.tco_check_attribute_locations;
        let glean_service: &'a str = &*arena.alloc_str(self.glean_service);
        let glean_hostname: &'a str = &*arena.alloc_str(self.glean_hostname);
        let glean_port = self.glean_port;
        let glean_reponame: &'a str = &*arena.alloc_str(self.glean_reponame);
        let symbol_write_ownership = self.symbol_write_ownership;
        let symbol_write_root_path: &'a str = &*arena.alloc_str(self.symbol_write_root_path);
        let symbol_write_hhi_path: &'a str = &*arena.alloc_str(self.symbol_write_hhi_path);
        let mut write_ignore_paths = bumpalo::collections::Vec::with_capacity_in(
            self.symbol_write_ignore_paths.len(),
            arena,
        );
        write_ignore_paths.extend(
            self.symbol_write_ignore_paths
                .iter()
                .map(|s| &*arena.alloc_str(s)),
        );
        let symbol_write_ignore_paths: &'a [&'a str] = write_ignore_paths.into_bump_slice();
        let mut write_index_paths =
            bumpalo::collections::Vec::with_capacity_in(self.symbol_write_index_paths.len(), arena);
        write_index_paths.extend(
            self.symbol_write_ignore_paths
                .iter()
                .map(|s| &*arena.alloc_str(s)),
        );
        let symbol_write_index_paths: &'a [&'a str] = write_index_paths.into_bump_slice();
        let symbol_write_index_paths_file: Option<&'a str> = self
            .symbol_write_index_paths_file
            .map(|s| &*arena.alloc_str(s));
        let symbol_write_index_paths_file_output: Option<&'a str> = self
            .symbol_write_index_paths_file_output
            .map(|s| &*arena.alloc_str(s));
        let symbol_write_include_hhi = self.symbol_write_include_hhi;
        let po_disallow_func_ptrs_in_constants = self.po_disallow_func_ptrs_in_constants;
        let tco_error_php_lambdas = self.tco_error_php_lambdas;
        let tco_disallow_discarded_nullable_awaitables =
            self.tco_disallow_discarded_nullable_awaitables;
        let po_enable_xhp_class_modifier = self.po_enable_xhp_class_modifier;
        let po_disable_xhp_element_mangling = self.po_disable_xhp_element_mangling;
        let po_disable_xhp_children_declarations = self.po_disable_xhp_children_declarations;
        let po_enable_enum_classes = self.po_enable_enum_classes;
        let po_disable_hh_ignore_error = self.po_disable_hh_ignore_error;
        let tco_enable_systemlib_annotations = self.tco_enable_systemlib_annotations;
        let tco_higher_kinded_types = self.tco_higher_kinded_types;
        let tco_method_call_inference = self.tco_method_call_inference;
        let tco_report_pos_from_reason = self.tco_report_pos_from_reason;
        let tco_typecheck_sample_rate = self.tco_typecheck_sample_rate;
        let tco_enable_sound_dynamic = self.tco_enable_sound_dynamic;
        let po_disallow_fun_and_cls_meth_pseudo_funcs =
            self.po_disallow_fun_and_cls_meth_pseudo_funcs;
        let po_disallow_inst_meth = self.po_disallow_inst_meth;
        let tco_use_direct_decl_parser = self.tco_use_direct_decl_parser;
        let mut ifc_enabled =
            bumpalo::collections::Vec::with_capacity_in(self.tco_ifc_enabled.len(), arena);
        ifc_enabled.extend(self.tco_ifc_enabled.iter().map(|s| &*arena.alloc_str(s)));
        let tco_ifc_enabled: &'a [&'a str] = ifc_enabled.into_bump_slice();
        let mut global_write_check_enabled = bumpalo::collections::Vec::with_capacity_in(
            self.tco_global_write_check_enabled.len(),
            arena,
        );
        global_write_check_enabled.extend(
            self.tco_global_write_check_enabled
                .iter()
                .map(|s| &*arena.alloc_str(s)),
        );
        let tco_global_write_check_enabled: &'a [&'a str] =
            global_write_check_enabled.into_bump_slice();
        let tco_global_write_check_functions_enabled: s_set::SSet<'a> = s_set::SSet::from(
            arena,
            self.tco_global_write_check_functions_enabled
                .into_iter()
                .map(|s| &*arena.alloc_str(s)),
        );
        let po_enable_enum_supertyping = self.po_enable_enum_supertyping;
        let po_interpret_soft_types_as_like_types = self.po_interpret_soft_types_as_like_types;
        let tco_enable_strict_string_concat_interp = self.tco_enable_strict_string_concat_interp;
        let tco_ignore_unsafe_cast = self.tco_ignore_unsafe_cast;
        let tco_no_parser_readonly_check = self.tco_no_parser_readonly_check;
        let tco_enable_expression_trees = self.tco_enable_expression_trees;
        let tco_enable_modules = self.tco_enable_modules;
        let mut tco_allowed_expression_tree_visitors = bumpalo::collections::Vec::with_capacity_in(
            self.tco_allowed_expression_tree_visitors.len(),
            arena,
        );
        tco_allowed_expression_tree_visitors.extend(
            self.tco_allowed_expression_tree_visitors
                .iter()
                .map(|s| &*arena.alloc_str(s)),
        );
        let tco_allowed_expression_tree_visitors: &'a [&'a str] =
            tco_allowed_expression_tree_visitors.into_bump_slice();
        let tco_math_new_code = self.tco_math_new_code;
        let tco_typeconst_concrete_concrete_error = self.tco_typeconst_concrete_concrete_error;
        let tco_enable_strict_const_semantics = self.tco_enable_strict_const_semantics;
        let tco_strict_wellformedness = self.tco_strict_wellformedness;
        let tco_meth_caller_only_public_visibility = self.tco_meth_caller_only_public_visibility;
        let tco_require_extends_implements_ancestors =
            self.tco_require_extends_implements_ancestors;
        let tco_strict_value_equality = self.tco_strict_value_equality;
        let tco_enforce_sealed_subclasses = self.tco_enforce_sealed_subclasses;
        let tco_everything_sdt = self.tco_everything_sdt;
        let tco_pessimise_builtins = self.tco_pessimise_builtins;
        let tco_enable_disk_heap = self.tco_enable_disk_heap;
        let tco_explicit_consistent_constructors = self.tco_explicit_consistent_constructors;
        let tco_require_types_class_consts = self.tco_require_types_class_consts;
        let tco_type_printer_fuel = self.tco_type_printer_fuel;
        let tco_log_saved_state_age_and_distance = self.tco_log_saved_state_age_and_distance;
        let tco_specify_manifold_api_key = self.tco_specify_manifold_api_key;
        let tco_saved_state_manifold_api_key: Option<&'a str> = self
            .tco_saved_state_manifold_api_key
            .map(|s| &*arena.alloc_str(s));
        let tco_profile_top_level_definitions = self.tco_profile_top_level_definitions;
        let tco_allow_all_files_for_module_declarations =
            self.tco_allow_all_files_for_module_declarations;
        let mut tco_allowed_files_for_module_declarations =
            bumpalo::collections::Vec::with_capacity_in(
                self.tco_allowed_files_for_module_declarations.len(),
                arena,
            );
        tco_allowed_files_for_module_declarations.extend(
            self.tco_allowed_files_for_module_declarations
                .iter()
                .map(|s| &*arena.alloc_str(s)),
        );
        let tco_allowed_files_for_module_declarations: &'a [&'a str] =
            tco_allowed_files_for_module_declarations.into_bump_slice();
        let tco_use_manifold_cython_client = self.tco_use_manifold_cython_client;
        let tco_record_fine_grained_dependencies = self.tco_record_fine_grained_dependencies;
        let tco_loop_iteration_upper_bound = self.tco_loop_iteration_upper_bound;
        let tco_expression_tree_virtualize_functions =
            self.tco_expression_tree_virtualize_functions;
        let tco_substitution_mutation = self.tco_substitution_mutation;

        GlobalOptions::<'a> {
            tco_experimental_features,
            tco_migration_flags,
            tco_num_local_workers,
            tco_parallel_type_checking_threshold,
            tco_max_typechecker_worker_memory_mb,
            tco_defer_class_declaration_threshold,
            tco_prefetch_deferred_files,
            tco_remote_type_check_threshold,
            tco_remote_type_check,
            tco_remote_worker_key,
            tco_remote_check_id,
            tco_remote_max_batch_size,
            tco_remote_min_batch_size,
            tco_num_remote_workers,
            so_remote_version_specifier,
            so_remote_worker_vfs_checkout_threshold,
            so_naming_sqlite_path,
            po_auto_namespace_map,
            po_codegen,
            po_deregister_php_stdlib,
            po_disallow_toplevel_requires,
            po_allow_unstable_features,
            tco_log_large_fanouts_threshold,
            tco_log_inference_constraints,
            tco_language_feature_logging,
            tco_timeout,
            tco_disallow_invalid_arraykey,
            tco_disallow_byref_dynamic_calls,
            tco_disallow_byref_calls,
            allowed_fixme_codes_strict,
            log_levels,
            po_disable_lval_as_an_expression,
            tco_shallow_class_decl,
            tco_force_shallow_decl_fanout,
            tco_remote_old_decls_no_limit,
            tco_fetch_remote_old_decls,
            tco_force_load_hot_shallow_decls,
            tco_populate_member_heaps,
            tco_skip_hierarchy_checks,
            tco_skip_tast_checks,
            tco_like_type_hints,
            tco_union_intersection_type_hints,
            tco_coeffects,
            tco_coeffects_local,
            tco_strict_contexts,
            tco_like_casts,
            tco_simple_pessimize,
            tco_complex_coercion,
            tco_check_xhp_attribute,
            tco_check_redundant_generics,
            tco_disallow_unresolved_type_variables,
            po_enable_class_level_where_clauses,
            po_disable_legacy_soft_typehints,
            po_allowed_decl_fixme_codes,
            po_allow_new_attribute_syntax,
            tco_global_inference,
            tco_gi_reinfer_types,
            tco_ordered_solving,
            tco_const_static_props,
            po_disable_legacy_attribute_syntax,
            tco_const_attribute,
            po_const_default_func_args,
            po_const_default_lambda_args,
            po_disallow_silence,
            po_abstract_static_props,
            po_parser_errors_only,
            tco_check_attribute_locations,
            glean_service,
            glean_hostname,
            glean_port,
            glean_reponame,
            symbol_write_ownership,
            symbol_write_root_path,
            symbol_write_hhi_path,
            symbol_write_ignore_paths,
            symbol_write_index_paths,
            symbol_write_index_paths_file,
            symbol_write_index_paths_file_output,
            symbol_write_include_hhi,
            po_disallow_func_ptrs_in_constants,
            tco_error_php_lambdas,
            tco_disallow_discarded_nullable_awaitables,
            po_enable_xhp_class_modifier,
            po_disable_xhp_element_mangling,
            po_disable_xhp_children_declarations,
            po_enable_enum_classes,
            po_disable_hh_ignore_error,
            tco_enable_systemlib_annotations,
            tco_higher_kinded_types,
            tco_method_call_inference,
            tco_report_pos_from_reason,
            tco_typecheck_sample_rate,
            tco_enable_sound_dynamic,
            po_disallow_fun_and_cls_meth_pseudo_funcs,
            po_disallow_inst_meth,
            tco_use_direct_decl_parser,
            tco_ifc_enabled,
            tco_global_write_check_enabled,
            tco_global_write_check_functions_enabled,
            po_enable_enum_supertyping,
            po_interpret_soft_types_as_like_types,
            tco_enable_strict_string_concat_interp,
            tco_ignore_unsafe_cast,
            tco_no_parser_readonly_check,
            tco_enable_expression_trees,
            tco_enable_modules,
            tco_allowed_expression_tree_visitors,
            tco_math_new_code,
            tco_typeconst_concrete_concrete_error,
            tco_enable_strict_const_semantics,
            tco_strict_wellformedness,
            tco_meth_caller_only_public_visibility,
            tco_require_extends_implements_ancestors,
            tco_strict_value_equality,
            tco_enforce_sealed_subclasses,
            tco_everything_sdt,
            tco_pessimise_builtins,
            tco_enable_disk_heap,
            tco_explicit_consistent_constructors,
            tco_require_types_class_consts,
            tco_type_printer_fuel,
            tco_log_saved_state_age_and_distance,
            tco_specify_manifold_api_key,
            tco_saved_state_manifold_api_key,
            tco_profile_top_level_definitions,
            tco_allow_all_files_for_module_declarations,
            tco_allowed_files_for_module_declarations,
            tco_use_manifold_cython_client,
            tco_record_fine_grained_dependencies,
            tco_loop_iteration_upper_bound,
            tco_expression_tree_virtualize_functions,
            tco_substitution_mutation,
        }
    }
}
