(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type saved_state_loading = {
  saved_state_manifold_api_key: string option;
      (** A string from hh.conf. The API key is used for saved state downloads
        when we call out to manifold *)
  log_saved_state_age_and_distance: bool;
      (** Collects the age of a saved state (in seconds) and distance (in globalrevs) for telemetry *)
  use_manifold_cython_client: bool;
      (** Required for Hedwig support for saved state downloads *)
  zstd_decompress_by_file: bool;
      (** When decompressing a saved state folder, pass specific files into zstd *)
  use_compressed_dep_graph: bool;
      (** When unpacking a saved state, decompress the dep graph from a .zhhdg file to a .hhdg file  *)
}

val default_saved_state_loading : saved_state_loading

type saved_state = {
  loading: saved_state_loading;
  rollouts: Saved_state_rollouts.t;
  project_metadata_w_flags: bool;
}
[@@deriving show, eq]

type 'a all_or_some =
  | All
  | ASome of 'a list
[@@deriving eq, show]

type 'a none_or_all_except =
  | NNone
  | All_except of 'a list
[@@deriving eq, show]

val default_saved_state : saved_state

val with_saved_state_manifold_api_key :
  string option -> saved_state -> saved_state

val with_use_manifold_cython_client : bool -> saved_state -> saved_state

val with_log_saved_state_age_and_distance : bool -> saved_state -> saved_state

val with_zstd_decompress_by_file : bool -> saved_state -> saved_state

type extended_reasons_config =
  | Extended of int
  | Legacy
  | Debug
[@@deriving eq, show]

(** Naming conventions for fieds:
  - tco_<feature/flag/setting> - type checker option
  - po_<feature/flag/setting> - parser option
  - so_<feature/flag/setting> - server option *)
type t = {
  po: ParserOptions.t;
  tco_saved_state: saved_state;
  tco_experimental_features: SSet.t;
      (** Set of typechecker-only experimental features, in lowercase.
          NB, *not* the experimental features that are controlled by the
          file level <<__EnableUnstableFeatures('feature_name')>> attribute.
          Those are in ParserOptions. *)
  tco_migration_flags: SSet.t;
      (** Set of opt-in migration behavior flags, in lowercase. *)
  tco_num_local_workers: int option;
      (** If set to 0, only the type check delegate's logic will be used.
        If the delegate fails to type check, the typing check service as a whole
        will fail. *)
  tco_defer_class_declaration_threshold: int option;
      (** If set, defers class declarations after N lazy declarations; if not set,
        always lazily declares classes not already in cache. *)
  tco_locl_cache_capacity: int;
      (** The capacity of the localization cache for large types *)
  tco_locl_cache_node_threshold: int;
      (** The number of nodes a type has to exceed to enter the localization cache *)
  so_naming_sqlite_path: string option;
      (** Enables the reverse naming table to fall back to SQLite for queries. *)
  po_disallow_toplevel_requires: bool;
      (** Flag to disallow `require`, `require_once` etc as toplevel statements *)
  tco_log_large_fanouts_threshold: int option;
  tco_log_inference_constraints: bool;
      (** Print types of size bigger than 1000 after performing a type union. *)
  tco_language_feature_logging: bool;
      (** Flag to enable logging of statistics regarding use of language features.
        Currently used for lambdas. *)
  tco_timeout: int;
      (** If non-zero, give up type checking a class or function after this many seconds *)
  tco_disallow_invalid_arraykey: bool;
      (** Flag to disallow using values that get casted to array keys at runtime;
        like bools, floats, or null; as array keys. *)
  code_agnostic_fixme: bool;
      (** HH_FIXME should silence *any* error, not just the one specified by code *)
  allowed_fixme_codes_strict: ISet.t;
      (** Error codes for which we allow HH_FIXMEs in strict mode *)
  log_levels: int SMap.t;  (** Initial hh_log_level settings *)
  class_pointer_levels: int SMap.t;
      (** Map of restriction levels for class pointer migration *)
  tco_remote_old_decls_no_limit: bool;
      (** Flag to fetch old decls from remote decl store *)
  tco_fetch_remote_old_decls: bool;
      (** Fetch old decls from CAS instead of memcache/manifold *)
  tco_populate_member_heaps: bool;
      (** Populate the member signature heaps.

        If disabled, instead load lazily from shallow classes. *)
  tco_skip_hierarchy_checks: bool;
      (** Skip checks on hierarchy e.g. overrides, require extend, etc.
        Set to true only for debugging purposes! *)
  tco_skip_tast_checks: bool;
      (** Skip checks implemented with TAST visitors.
        Set to true only for debugging purposes! *)
  tco_coeffects: bool;  (** Enables checking of coeffects *)
  tco_coeffects_local: bool;
      (** Enables checking of coeffects for local operations (not calls) *)
  tco_strict_contexts: bool;
      (** Internal (for tests-only): whether any type can appear in a context list
         or only types defined in the appropriate Context namespace *)
  tco_like_casts: bool;  (** Enables like casts *)
  tco_check_xhp_attribute: bool;  (** static check xhp required attribute *)
  tco_check_redundant_generics: bool;
      (** Check redundant generics in return types *)
  tco_disallow_unresolved_type_variables: bool;
      (** Flag to produce an error whenever the TAST contains unresolved type variables *)
  tco_custom_error_config: Custom_error_config.t;
      (** Allows additional error messages to be added to typing errors via config *)
  tco_const_attribute: bool;  (** Allow <<__Const>> attribute *)
  tco_check_attribute_locations: bool;
  tco_type_refinement_partition_shapes: bool;
      (** Use new type splitting logic for shape refinement *)
  glean_reponame: string;
      (** Reponame used for glean connection, default to "www.autocomplete" *)
  symbol_write_index_inherited_members: bool;
  symbol_write_ownership: bool;
      (** Index inherited members information from folded decls *)
  symbol_write_root_path: string;
      (** include fact ownership information, for creating incremental databases. *)
  symbol_write_hhi_path: string;
      (** Path prefix to use for hhi files when writing symbol info to JSON *)
  symbol_write_ignore_paths: string list;
      (** Filepaths to ignore when writing symbol info to JSON, relative to path prefix, eg: root|foo.php *)
  symbol_write_index_paths: string list;
      (** When set, write indexing data for these filepaths only. Relative to repository root, eg: bar.php for root|bar.php *)
  symbol_write_index_paths_file: string option;
      (** A file which contains a list of Relative_path.t (one per line) to index *)
  symbol_write_index_paths_file_output: string option;
      (** Write the list of Relative_path.t to this file instead of indexing. Useful for sharding *)
  symbol_write_include_hhi: bool;
      (** Write symbol indexing data for hhi files *)
  symbol_write_sym_hash_in: string option;
      (** Use symbols hash table for incremental indexing *)
  symbol_write_exclude_out: string option;
      (** Path to file containing units to exclude *)
  symbol_write_referenced_out: string option;
      (** Path to file containing referenced files *)
  symbol_write_reindexed_out: string option;  (** Generate symbols hash table *)
  symbol_write_sym_hash_out: bool;  (** Generate reindexed files *)
  tco_error_php_lambdas: bool;
      (** Flag to report an error on php style anonymous functions *)
  tco_disallow_discarded_nullable_awaitables: bool;
      (** Flag to error on using discarded nullable awaitables *)
  tco_higher_kinded_types: bool;
      (** Controls if higher-kinded types are supported *)
  tco_typecheck_sample_rate: float;
      (** Type check this proportion of all files. Default is 1.0.
        DO NOT set to any other value except for testing purposes. *)
  tco_enable_sound_dynamic: bool;
      (** Experimental implementation of a "sound" dynamic type *)
  tco_pessimise_builtins: bool;
      (** Under sound dynamic, introduce like-types for built-in operations e.g. on Vector.
         This is done anyway if everything_sdt=true *)
  tco_enable_no_auto_dynamic: bool;
      (** Allow use of attribute <<__NoAutoDynamic>> *)
  tco_skip_check_under_dynamic: bool;
      (** Skip second check of method under dynamic assumptions *)
  tco_global_access_check_enabled: bool;
  tco_enable_strict_string_concat_interp: bool;
      (** Restricts string concatenation and interpolation to arraykeys *)
  tco_ignore_unsafe_cast: bool;
      (** Ignores unsafe_cast and retains the original type of the expression *)
  tco_enable_expression_trees: bool;
      (** Enable expression trees via unstable features flag *)
  tco_enable_function_references: bool;
      (** Enable unstable feature: function references *)
  tco_allowed_expression_tree_visitors: string list;
      (** Allowed expression tree visitors when not enabled via unstable features flag *)
  tco_typeconst_concrete_concrete_error: bool;
      (** Raise an error when a concrete type constant is overridden by a concrete type constant
         in a child class. *)
  tco_enable_strict_const_semantics: int;
      (** When the value is 1, raises a 4734 error when an inherited constant comes from a conflicting
         hierarchy, but not if the constant is locally defined. When the value is 2, raises a conflict
         any time two parents declare concrete constants with the same name, matching HHVM
         -vEval.TraitConstantInterfaceBehavior=1 *)
  tco_meth_caller_only_public_visibility: bool;
      (** meth_caller can only reference public methods *)
  tco_require_extends_implements_ancestors: bool;
      (** Consider `require extends` and `require implements` as ancestors when checking a class *)
  tco_strict_value_equality: bool;
      (** Emit an error when "==" or "!=" is used to compare values that are incompatible types *)
  tco_enforce_sealed_subclasses: bool;
      (** All member of the __Sealed whitelist should be subclasses*)
  tco_implicit_inherit_sdt: bool;
      (** Inherit SDT from parents, without writing <<__SupportDynamicType>> *)
  tco_explicit_consistent_constructors: int;
      (** Raises an error when a classish is declared <<__ConsistentConstruct>> but lacks an
         explicit constructor declaration. 0 does not raise, 1 raises for traits, 2 raises
         for all classish *)
  tco_require_types_class_consts: int;
      (** Raises an error when a class constant is missing a type. 0 does not raise, 1 raises
        for abstract class constants, 2 raises for all. *)
  tco_type_printer_fuel: int;
      (** Sets the amount of fuel that the type printer can use to display an
        individual type. More of a type is printed as the value increases. *)
  tco_specify_manifold_api_key: bool;
      (** allows saved_state_loader to shell out to hg to find globalrev and timestamp of revisions *)
  tco_profile_top_level_definitions: bool;
      (** Measures and reports the time it takes to typecheck each top-level
         definition. *)
  tco_allow_all_files_for_module_declarations: bool;
  tco_allowed_files_for_module_declarations: string list;
  tco_record_fine_grained_dependencies: bool;
      (** If enabled, the type checker records more fine-grained dependencies than usual,
         for example between individual methods. *)
  tco_loop_iteration_upper_bound: int option;
      (** When set, uses the given number of iterations while typechecking loops *)
  tco_populate_dead_unsafe_cast_heap: bool;
      (** Dead UNSAFE_CAST codemod stashes patches through a TAST visitor in shared
         heap. This is only needed in dead UNSAFE_CAST removal mode. This option
         controls whether the heap will be populated or not. *)
  dump_tast_hashes: bool;  (** Dump tast hashes in /tmp/hh_server/tast_hashes *)
  dump_tasts: string list;
      (** List of paths whose TASTs to be dumped in /tmp/hh_server/tasts *)
  tco_autocomplete_mode: bool;  (** Are we running in autocomplete mode ? *)
  tco_log_exhaustivity_check: bool;
      (** Instrument the existing exhaustivity lint (for strict switch statements) *)
  tco_sticky_quarantine: bool;
      (** Controls behavior of [Provider_utils.respect_but_quarantine_unsaved_changes] *)
  tco_lsp_invalidation: bool;
      (** Controls how [Provicer_utils.respect_but_quarantine_unsaved_changes] invalidates folded decls *)
  tco_autocomplete_sort_text: bool;
  tco_extended_reasons: extended_reasons_config option;
      (** Controls whether we retain the full path for reasons or only simple witnesses *)
  tco_disable_physical_equality: bool;
      (** If set to true, this disables the use of physical equality in subtyping *)
  hack_warnings: int none_or_all_except;  (** turn on hack warnings *)
  warnings_default_all: bool;
  warnings_in_sandcastle: bool;
  tco_strict_switch: bool;
      (** Enable strict case checking in switch statements *)
  tco_allowed_files_for_ignore_readonly: string list;
  tco_package_v2_exclude_patterns: string list;
      (** Patterns for files excluded from the package boundary check. *)
  tco_package_v2_bypass_package_check_for_classptr_migration: bool;
      (** Option for package v2 to bypass package boundary violation errors on ::class during
          the ::class to nameof migration to unblock V0 of intern-prod separation *)
  re_no_cache: bool;
      (** Disable RE cache when calling hh_distc. Useful for performance testing.
        Corresponds to the `--no-cache` options of hh_distc. *)
  hh_distc_should_disable_trace_store: bool;
      (** Disable trace store when calling hh_distc. Useful for performance testing.
        Corresponds to the `--trace-store-mode local` options of hh_distc.*)
  hh_distc_exponential_backoff_num_retries: int;
      (** Number of retries when uploading/download/executing with hh_distc *)
  tco_enable_abstract_method_optional_parameters: bool;
      (** Enable use of optional on parameters in abstract methods *)
  recursive_case_types: bool;  (** Enable recursive case types *)
  class_sub_classname: bool;  (** Whether class<T> <: classname<T> *)
  class_class_type: bool;  (** When true, C::class : class<C> *)
  safe_abstract: bool;
      (** Enable Safe Abstract features https://fburl.com/hack-safe-abstract *)
  needs_concrete: bool;
      (** Enable __NeedsConcrete checking https://fburl.com/hack-needs-concrete *)
  allow_class_string_cast: bool;  (** Admits (string)$c when $c: class<T>  *)
  tco_new_exhaustivity_check: bool;
      (** Enables strict exhaustivity checks on switch statements and disables
          the legacy ones *)
}
[@@deriving eq, show]

val set :
  ?po:ParserOptions.t ->
  ?tco_saved_state:saved_state ->
  ?po_disallow_toplevel_requires:bool ->
  ?tco_log_large_fanouts_threshold:int ->
  ?tco_log_inference_constraints:bool ->
  ?tco_experimental_features:SSet.t ->
  ?tco_migration_flags:SSet.t ->
  ?tco_num_local_workers:int ->
  ?tco_defer_class_declaration_threshold:int ->
  ?tco_locl_cache_capacity:int ->
  ?tco_locl_cache_node_threshold:int ->
  ?so_naming_sqlite_path:string ->
  ?tco_language_feature_logging:bool ->
  ?tco_timeout:int ->
  ?tco_disallow_invalid_arraykey:bool ->
  ?code_agnostic_fixme:bool ->
  ?allowed_fixme_codes_strict:ISet.t ->
  ?log_levels:int SMap.t ->
  ?class_pointer_levels:int SMap.t ->
  ?tco_remote_old_decls_no_limit:bool ->
  ?tco_fetch_remote_old_decls:bool ->
  ?tco_populate_member_heaps:bool ->
  ?tco_skip_hierarchy_checks:bool ->
  ?tco_skip_tast_checks:bool ->
  ?tco_coeffects:bool ->
  ?tco_coeffects_local:bool ->
  ?tco_strict_contexts:bool ->
  ?tco_like_casts:bool ->
  ?tco_check_xhp_attribute:bool ->
  ?tco_check_redundant_generics:bool ->
  ?tco_disallow_unresolved_type_variables:bool ->
  ?tco_custom_error_config:Custom_error_config.t ->
  ?tco_const_attribute:bool ->
  ?tco_check_attribute_locations:bool ->
  ?tco_type_refinement_partition_shapes:bool ->
  ?glean_reponame:string ->
  ?symbol_write_index_inherited_members:bool ->
  ?symbol_write_ownership:bool ->
  ?symbol_write_root_path:string ->
  ?symbol_write_hhi_path:string ->
  ?symbol_write_ignore_paths:string list ->
  ?symbol_write_index_paths:string list ->
  ?symbol_write_index_paths_file:string ->
  ?symbol_write_index_paths_file_output:string ->
  ?symbol_write_include_hhi:bool ->
  ?symbol_write_sym_hash_in:string ->
  ?symbol_write_exclude_out:string ->
  ?symbol_write_referenced_out:string ->
  ?symbol_write_reindexed_out:string ->
  ?symbol_write_sym_hash_out:bool ->
  ?tco_error_php_lambdas:bool ->
  ?tco_disallow_discarded_nullable_awaitables:bool ->
  ?tco_higher_kinded_types:bool ->
  ?tco_typecheck_sample_rate:float ->
  ?tco_enable_sound_dynamic:bool ->
  ?tco_pessimise_builtins:bool ->
  ?tco_enable_no_auto_dynamic:bool ->
  ?tco_skip_check_under_dynamic:bool ->
  ?tco_global_access_check_enabled:bool ->
  ?tco_enable_strict_string_concat_interp:bool ->
  ?tco_ignore_unsafe_cast:bool ->
  ?tco_enable_expression_trees:bool ->
  ?tco_enable_function_references:bool ->
  ?tco_allowed_expression_tree_visitors:string list ->
  ?tco_typeconst_concrete_concrete_error:bool ->
  ?tco_enable_strict_const_semantics:int ->
  ?tco_meth_caller_only_public_visibility:bool ->
  ?tco_require_extends_implements_ancestors:bool ->
  ?tco_strict_value_equality:bool ->
  ?tco_enforce_sealed_subclasses:bool ->
  ?tco_implicit_inherit_sdt:bool ->
  ?tco_explicit_consistent_constructors:int ->
  ?tco_require_types_class_consts:int ->
  ?tco_type_printer_fuel:int ->
  ?tco_specify_manifold_api_key:bool ->
  ?tco_profile_top_level_definitions:bool ->
  ?tco_allow_all_files_for_module_declarations:bool ->
  ?tco_allowed_files_for_module_declarations:string list ->
  ?tco_record_fine_grained_dependencies:bool ->
  ?tco_loop_iteration_upper_bound:int option ->
  ?tco_populate_dead_unsafe_cast_heap:bool ->
  ?dump_tast_hashes:bool ->
  ?dump_tasts:string list ->
  ?tco_autocomplete_mode:bool ->
  ?tco_log_exhaustivity_check:bool ->
  ?tco_sticky_quarantine:bool ->
  ?tco_lsp_invalidation:bool ->
  ?tco_autocomplete_sort_text:bool ->
  ?tco_extended_reasons:extended_reasons_config ->
  ?tco_disable_physical_equality:bool ->
  ?hack_warnings:int none_or_all_except ->
  ?warnings_default_all:bool ->
  ?warnings_in_sandcastle:bool ->
  ?tco_strict_switch:bool ->
  ?tco_allowed_files_for_ignore_readonly:string list ->
  ?tco_package_v2_exclude_patterns:string list ->
  ?tco_package_v2_bypass_package_check_for_classptr_migration:bool ->
  ?re_no_cache:bool ->
  ?hh_distc_should_disable_trace_store:bool ->
  ?hh_distc_exponential_backoff_num_retries:int ->
  ?tco_enable_abstract_method_optional_parameters:bool ->
  ?recursive_case_types:bool ->
  ?class_sub_classname:bool ->
  ?class_class_type:bool ->
  ?safe_abstract:bool ->
  ?needs_concrete:bool ->
  ?allow_class_string_cast:bool ->
  ?tco_new_exhaustivity_check:bool ->
  t ->
  t

val default : t

(* NOTE: set/getters for tco_* options moved to TypecheckerOptions *)
(* NOTE: set/getters for po_* options moved to ParserOptions *)

val so_naming_sqlite_path : t -> string option

val allowed_fixme_codes_strict : t -> ISet.t

val code_agnostic_fixme : t -> bool

(* NOTE: set/getters for tco_* options moved to TypecheckerOptions *)
(* NOTE: set/getters for po_* options moved to ParserOptions *)
