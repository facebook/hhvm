// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<62eb99fd42a633e69292002a5ff5eb3c>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (show, eq)")]
#[repr(C)]
pub struct SavedStateLoading {
    /// A string from hh.conf. The API key is used for saved state downloads
    /// when we call out to manifold
    pub saved_state_manifold_api_key: Option<String>,
    /// Collects the age of a saved state (in seconds) and distance (in globalrevs) for telemetry
    pub log_saved_state_age_and_distance: bool,
    /// Required for Hedwig support for saved state downloads
    pub use_manifold_cython_client: bool,
    /// When decompressing a saved state folder, pass specific files into zstd
    pub zstd_decompress_by_file: bool,
    /// When unpacking a saved state, decompress the dep graph from a .zhhdg file to a .hhdg file
    pub use_compressed_dep_graph: bool,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (show, eq)")]
#[repr(C)]
pub struct SavedState {
    pub loading: SavedStateLoading,
    pub rollouts: saved_state_rollouts::SavedStateRollouts,
    pub project_metadata_w_flags: bool,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C, u8)]
pub enum AllOrSome<A> {
    All,
    ASome(Vec<A>),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C, u8)]
pub enum NoneOrAllExcept<A> {
    NNone,
    #[rust_to_ocaml(name = "All_except")]
    AllExcept(Vec<A>),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C, u8)]
pub enum ExtendedReasonsConfig {
    Extended(isize),
    Legacy,
    Debug,
}

/// Naming conventions for fields in this struct:
/// - tco_<feature/flag/setting> - type checker option
/// - po_<feature/flag/setting> - parser option
/// - so_<feature/flag/setting> - server option
#[derive(
    Clone,
    Debug,
    Deserialize,
    FromOcamlRep,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C)]
pub struct GlobalOptions {
    pub po: parser_options::ParserOptions,
    pub tco_saved_state: SavedState,
    /// Set of typechecker-only experimental features, in lowercase.
    /// NB, *not* the experimental features that are controlled by the
    /// file level <<__EnableUnstableFeatures('feature_name')>> attribute.
    /// Those are in ParserOptions.
    pub tco_experimental_features: s_set::SSet,
    /// Set of opt-in migration behavior flags, in lowercase.
    pub tco_migration_flags: s_set::SSet,
    /// If set to 0, only the type check delegate's logic will be used.
    /// If the delegate fails to type check, the typing check service as a whole
    /// will fail.
    pub tco_num_local_workers: Option<isize>,
    /// If set, defers class declarations after N lazy declarations; if not set,
    /// always lazily declares classes not already in cache.
    pub tco_defer_class_declaration_threshold: Option<isize>,
    /// The capacity of the localization cache for large types
    pub tco_locl_cache_capacity: isize,
    /// The number of nodes a type has to exceed to enter the localization cache
    pub tco_locl_cache_node_threshold: isize,
    /// Enables the reverse naming table to fall back to SQLite for queries.
    pub so_naming_sqlite_path: Option<String>,
    /// Flag to disallow `require`, `require_once` etc as toplevel statements
    pub po_disallow_toplevel_requires: bool,
    pub tco_log_large_fanouts_threshold: Option<isize>,
    /// Print types of size bigger than 1000 after performing a type union.
    pub tco_log_inference_constraints: bool,
    /// Flag to enable logging of statistics regarding use of language features.
    /// Currently used for lambdas.
    pub tco_language_feature_logging: bool,
    /// If non-zero, give up type checking a class or function after this many seconds
    pub tco_timeout: isize,
    /// Flag to enable the constraint solver to infer that a type can be indexed then assigned
    pub tco_constraint_array_index_assign: bool,
    /// Flag to enable the constraint solver to infer that a type supports method calls
    pub tco_constraint_method_call: bool,
    /// HH_FIXME should silence *any* error, not just the one specified by code
    pub code_agnostic_fixme: bool,
    /// Error codes for which we allow HH_FIXMEs in strict mode
    pub allowed_fixme_codes_strict: i_set::ISet,
    /// Initial hh_log_level settings
    pub log_levels: s_map::SMap<isize>,
    /// Flag to fetch old decls from remote decl store
    pub tco_remote_old_decls_no_limit: bool,
    /// Fetch old decls from CAS instead of memcache/manifold
    pub tco_fetch_remote_old_decls: bool,
    /// Populate the member signature heaps.
    ///
    /// If disabled, instead load lazily from shallow classes.
    pub tco_populate_member_heaps: bool,
    /// Skip checks on hierarchy e.g. overrides, require extend, etc.
    /// Set to true only for debugging purposes!
    pub tco_skip_hierarchy_checks: bool,
    /// Skip checks implemented with TAST visitors.
    /// Set to true only for debugging purposes!
    pub tco_skip_tast_checks: bool,
    /// Enables checking of coeffects
    pub tco_coeffects: bool,
    /// Enables checking of coeffects for local operations (not calls)
    pub tco_coeffects_local: bool,
    /// Internal (for tests-only): whether any type can appear in a context list
    /// or only types defined in the appropriate Context namespace
    pub tco_strict_contexts: bool,
    /// Enables like casts
    pub tco_like_casts: bool,
    /// static check xhp required attribute
    pub tco_check_xhp_attribute: bool,
    /// Check redundant generics in return types
    pub tco_check_redundant_generics: bool,
    /// Flag to produce an error whenever the TAST contains unresolved type variables
    pub tco_disallow_unresolved_type_variables: bool,
    /// Allows additional error messages to be added to typing errors via config
    pub tco_custom_error_config: custom_error_config::CustomErrorConfig,
    /// Allow <<__Const>> attribute
    pub tco_const_attribute: bool,
    pub tco_check_attribute_locations: bool,
    /// Use new type splitting logic for shape refinement
    pub tco_type_refinement_partition_shapes: bool,
    /// Reponame used for glean connection, default to "www.autocomplete"
    pub glean_reponame: String,
    pub symbol_write_index_inherited_members: bool,
    /// Index inherited members information from folded decls
    pub symbol_write_ownership: bool,
    /// include fact ownership information, for creating incremental databases.
    pub symbol_write_root_path: String,
    /// Path prefix to use for hhi files when writing symbol info to JSON
    pub symbol_write_hhi_path: String,
    /// Filepaths to ignore when writing symbol info to JSON, relative to path prefix, eg: root|foo.php
    pub symbol_write_ignore_paths: Vec<String>,
    /// When set, write indexing data for these filepaths only. Relative to repository root, eg: bar.php for root|bar.php
    pub symbol_write_index_paths: Vec<String>,
    /// A file which contains a list of Relative_path.t (one per line) to index
    pub symbol_write_index_paths_file: Option<String>,
    /// Write the list of Relative_path.t to this file instead of indexing. Useful for sharding
    pub symbol_write_index_paths_file_output: Option<String>,
    /// Write symbol indexing data for hhi files
    pub symbol_write_include_hhi: bool,
    /// Use symbols hash table for incremental indexing
    pub symbol_write_sym_hash_in: Option<String>,
    /// Path to file containing units to exclude
    pub symbol_write_exclude_out: Option<String>,
    /// Path to file containing referenced files
    pub symbol_write_referenced_out: Option<String>,
    /// Generate symbols hash table
    pub symbol_write_reindexed_out: Option<String>,
    /// Generate reindexed files
    pub symbol_write_sym_hash_out: bool,
    /// Flag to report an error on php style anonymous functions
    pub tco_error_php_lambdas: bool,
    /// Flag to error on using discarded nullable awaitables
    pub tco_disallow_discarded_nullable_awaitables: bool,
    /// Type check this proportion of all files. Default is 1.0.
    /// DO NOT set to any other value except for testing purposes.
    pub tco_typecheck_sample_rate: f64,
    /// Under sound dynamic, introduce like-types for built-in operations e.g. on Vector.
    /// This is done anyway if everything_sdt=true
    pub tco_pessimise_builtins: bool,
    /// Allow use of attribute <<__NoAutoDynamic>>
    pub tco_enable_no_auto_dynamic: bool,
    /// Skip second check of method under dynamic assumptions
    pub tco_skip_check_under_dynamic: bool,
    pub tco_global_access_check_enabled: bool,
    /// Ignores unsafe_cast and retains the original type of the expression
    pub tco_ignore_unsafe_cast: bool,
    /// Enable expression trees via unstable features flag
    pub tco_enable_expression_trees: bool,
    /// Enable unstable feature: function references
    pub tco_enable_function_references: bool,
    /// Allowed expression tree visitors when not enabled via unstable features flag
    pub tco_allowed_expression_tree_visitors: Vec<String>,
    /// Raise an error when a concrete type constant is overridden by a concrete type constant
    /// in a child class.
    pub tco_typeconst_concrete_concrete_error: bool,
    /// meth_caller can only reference public methods
    pub tco_meth_caller_only_public_visibility: bool,
    /// Consider `require extends` and `require implements` as ancestors when checking a class
    pub tco_require_extends_implements_ancestors: bool,
    /// Emit an error when "==" or "!=" is used to compare values that are incompatible types
    pub tco_strict_value_equality: bool,
    /// All member of the __Sealed whitelist should be subclasses
    pub tco_enforce_sealed_subclasses: bool,
    /// Inherit SDT from parents, without writing <<__SupportDynamicType>>
    pub tco_implicit_inherit_sdt: bool,
    /// Directory of HSL wrappers defined in the repo, warns on unbound name.
    pub tco_repo_stdlib_path: Option<String>,
    /// Raises an error when a classish is declared <<__ConsistentConstruct>> but lacks an
    /// explicit constructor declaration. 0 does not raise, 1 raises for traits, 2 raises
    /// for all classish
    pub tco_explicit_consistent_constructors: isize,
    /// Raises an error when a class constant is missing a type. 0 does not raise, 1 raises
    /// for abstract class constants, 2 raises for all.
    pub tco_require_types_class_consts: isize,
    /// Controls reporting when a non-bool type is used in a condition. 0 does not report,
    /// 1 reports as a warning, 2 reports as an error.
    pub tco_check_bool_for_condition: isize,
    /// Sets the amount of fuel that the type printer can use to display an
    /// individual type. More of a type is printed as the value increases.
    pub tco_type_printer_fuel: isize,
    /// allows saved_state_loader to shell out to hg to find globalrev and timestamp of revisions
    pub tco_specify_manifold_api_key: bool,
    /// Measures and reports the time it takes to typecheck each top-level
    /// definition.
    pub tco_profile_top_level_definitions: bool,
    /// When set, it checks if the identifier for the definition matches the
    /// given regular expression and only then typechecks the definition.
    pub tco_typecheck_if_name_matches_regexp: Option<String>,
    pub tco_allow_all_files_for_module_declarations: bool,
    pub tco_allowed_files_for_module_declarations: Vec<String>,
    /// If enabled, the type checker records more fine-grained dependencies than usual,
    /// for example between individual methods.
    pub tco_record_fine_grained_dependencies: bool,
    /// When set, uses the given number of iterations while typechecking loops
    pub tco_loop_iteration_upper_bound: Option<isize>,
    /// Dead UNSAFE_CAST codemod stashes patches through a TAST visitor in shared
    /// heap. This is only needed in dead UNSAFE_CAST removal mode. This option
    /// controls whether the heap will be populated or not.
    pub tco_populate_dead_unsafe_cast_heap: bool,
    /// Dump tast hashes in /tmp/hh_server/tast_hashes
    pub dump_tast_hashes: bool,
    /// List of paths whose TASTs to be dumped in /tmp/hh_server/tasts
    pub dump_tasts: Vec<String>,
    /// Are we running in autocomplete mode ?
    pub tco_autocomplete_mode: bool,
    /// Controls behavior of [Provider_utils.respect_but_quarantine_unsaved_changes]
    pub tco_sticky_quarantine: bool,
    /// Controls how [Provicer_utils.respect_but_quarantine_unsaved_changes] invalidates folded decls
    pub tco_lsp_invalidation: bool,
    pub tco_autocomplete_sort_text: bool,
    /// Controls whether we retain the full path for reasons or only simple witnesses
    pub tco_extended_reasons: Option<ExtendedReasonsConfig>,
    /// If set to true, this disables the use of physical equality in subtyping
    pub tco_disable_physical_equality: bool,
    /// turn on hack warnings
    pub hack_warnings: NoneOrAllExcept<isize>,
    pub warnings_default_all: bool,
    pub warnings_in_sandcastle: bool,
    /// Matchers for file paths for which any warning will be ignored.
    /// Useful to ignore warnings from certain generated files.
    pub warnings_generated_files: Vec<String>,
    pub tco_allowed_files_for_ignore_readonly: Vec<String>,
    /// Patterns for files excluded from the package boundary check.
    pub tco_package_exclude_patterns: Vec<String>,
    /// Option for package support to bypass package boundary violation errors on typedefs to unblock V1 of
    /// intern-prod separation
    pub tco_package_allow_typedef_violations: bool,
    /// Option for package support to bypass package boundary violation errors on ::class during
    /// the ::class to nameof migration to unblock V1 of intern-prod separation
    pub tco_package_allow_classconst_violations: bool,
    /// Option for package support to bypass package boundary violation errors on definitions of
    /// reifiable abstract type constants to unblock V1 of intern-prod separation
    pub tco_package_allow_reifiable_tconst_violations: bool,
    /// Option for package support to bypass package boundary violation errors on definitions of
    /// all type constants to unblock V1 of intern-prod separation. This flag controls the
    /// superset of violations controlled by `tco_package_allow_reifiable_tconst_violations`
    /// and will be switched off as a step further in tightening the packgage boundary endforcement.
    pub tco_package_allow_all_tconst_violations: bool,
    /// Option for package support to bypass package boundary violation errors on reified generics
    /// to unblock V1 of intern-prod separation. This flag controls the
    /// superset of violations controlled by `tco_package_allow_reified_generics_violations`
    /// and will be switched off as a step further in tightening the packgage boundary endforcement.
    pub tco_package_allow_reified_generics_violations: bool,
    /// Option for package support to bypass package boundary violation errors on all generics
    /// to unblock V1 of intern-prod separation.
    pub tco_package_allow_all_generics_violations: bool,
    /// Option for package support to bypass package boundary violation errors on all function pointers
    /// to unblock V1 of intern-prod separation.
    pub tco_package_allow_function_pointers_violations: bool,
    /// Disable RE cache when calling hh_distc. Useful for performance testing.
    /// Corresponds to the `--no-cache` options of hh_distc.
    pub re_no_cache: bool,
    /// Disable trace store when calling hh_distc. Useful for performance testing.
    /// Corresponds to the `--trace-store-mode local` options of hh_distc.
    pub hh_distc_should_disable_trace_store: bool,
    /// Number of retries when uploading/download/executing with hh_distc
    pub hh_distc_exponential_backoff_num_retries: isize,
    /// Enable use of optional on parameters in abstract methods
    pub tco_enable_abstract_method_optional_parameters: bool,
    /// Enable recursive case types
    pub recursive_case_types: bool,
    /// Whether class<T> <: classname<T>
    pub class_sub_classname: bool,
    /// When true, C::class : class<C>
    pub class_class_type: bool,
    /// Enable __NeedsConcrete checking https://fburl.com/hack-needs-concrete.
    /// Excludes hierarchy/override check, which is covered by `needs_concrete_override_check`
    pub needs_concrete: bool,
    /// Enable override check for __NeedsConcrete methods https://fburl.com/hack-needs-concrete
    pub needs_concrete_override_check: bool,
    /// Admits (string)$c when $c: class<T>
    pub allow_class_string_cast: bool,
    /// Error on new $c() when $c: classname<T>
    pub class_pointer_ban_classname_new: isize,
    /// Error on type_structure($c, 'T') when $c: classname<T>
    pub class_pointer_ban_classname_type_structure: isize,
    /// Error on $c::foo() when $c: classname<T>
    pub class_pointer_ban_classname_static_meth: isize,
    /// Error on $c::FOO when $c: classname<T>
    pub class_pointer_ban_classname_class_const: isize,
    /// Error on dict[$c => 1] when $c: class<T>
    pub class_pointer_ban_class_array_key: bool,
    pub tco_poly_function_pointers: bool,
    /// enable static package enforcement
    pub tco_check_packages: bool,
    /// POC: @fzn, if true fanout strips class location when comparing shallow classes for minor changes
    pub fanout_strip_class_location: bool,
    /// POC: @fzn, if true, transitivity checks for package/deployment inclusion are disabled - for intern/prod rollout only
    pub tco_package_config_disable_transitivity_check: bool,
    /// POC: @fzn, when true (default), allows __RequirePackage on interface methods
    pub tco_allow_require_package_on_interface_methods: bool,
}
