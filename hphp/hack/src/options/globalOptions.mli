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

val default_saved_state : saved_state

val with_saved_state_manifold_api_key :
  string option -> saved_state -> saved_state

val with_use_manifold_cython_client : bool -> saved_state -> saved_state

val with_log_saved_state_age_and_distance : bool -> saved_state -> saved_state

val with_zstd_decompress_by_file : bool -> saved_state -> saved_state

(** Naming conventions for fieds:
  - tco_<feature/flag/setting> - type checker option
  - po_<feature/flag/setting> - parser option
  - so_<feature/flag/setting> - server option *)
type t = {
  tco_saved_state: saved_state;
  tco_experimental_features: SSet.t;
      (** Set of experimental features, in lowercase. *)
  tco_migration_flags: SSet.t;
      (** Set of opt-in migration behavior flags, in lowercase. *)
  tco_num_local_workers: int option;
      (** If set to 0, only the type check delegate's logic will be used.
        If the delegate fails to type check, the typing check service as a whole
        will fail. *)
  tco_max_typechecker_worker_memory_mb: int option;
      (** If set, typechecker workers will quit after they exceed this limit *)
  tco_defer_class_declaration_threshold: int option;
      (** If set, defers class declarations after N lazy declarations; if not set,
        always lazily declares classes not already in cache. *)
  tco_locl_cache_capacity: int;
      (** The capacity of the localization cache for large types *)
  tco_locl_cache_node_threshold: int;
      (** The number of nodes a type has to exceed to enter the localization cache *)
  so_naming_sqlite_path: string option;
      (** Enables the reverse naming table to fall back to SQLite for queries. *)
  po_auto_namespace_map: (string * string) list;  (** Namespace aliasing map *)
  po_codegen: bool;  (** Are we emitting bytecode? *)
  po_deregister_php_stdlib: bool;
      (** Flag for disabling functions in HHI files with the __PHPStdLib attribute *)
  po_disallow_toplevel_requires: bool;
      (** Flag to disallow `require`, `require_once` etc as toplevel statements *)
  po_allow_unstable_features: bool;
      (** Allows enabling unstable features via the __EnableUnstableFeatures attribute *)
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
  tco_disallow_byref_dynamic_calls: bool;
      (** Produces an error if an arguments is passed by reference to dynamically
        called function [e.g. $foo(&$bar)]. *)
  tco_disallow_byref_calls: bool;
      (** Produces an error if an arguments is passed by reference in any form
        [e.g. foo(&$bar)].*)
  code_agnostic_fixme: bool;
      (** HH_FIXME should silence *any* error, not just the one specified by code *)
  allowed_fixme_codes_strict: ISet.t;
      (** Error codes for which we allow HH_FIXMEs in strict mode *)
  log_levels: int SMap.t;  (** Initial hh_log_level settings *)
  po_disable_lval_as_an_expression: bool;
      (** Flag to disable using lvals as expressions. *)
  tco_remote_old_decls_no_limit: bool;
      (** Flag to fetch old decls from remote decl store *)
  tco_use_old_decls_from_cas: bool;
      (** Don't limit amount of remote old decls fetched *)
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
  tco_like_type_hints: bool;  (** Enables like type hints *)
  tco_union_intersection_type_hints: bool;
      (** Enables union and intersection type hints *)
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
  po_enable_class_level_where_clauses: bool;
      (** Enable class-level where clauses, i.e.
        class base<T> where T = int {} *)
  po_disable_legacy_soft_typehints: bool;
      (** Disable legacy soft typehint syntax (@int) and only allow the __Soft attribute. *)
  po_allowed_decl_fixme_codes: ISet.t;
      (** Set of error codes disallowed in decl positions *)
  tco_const_static_props: bool;  (** Enable const static properties *)
  po_disable_legacy_attribute_syntax: bool;
      (** Disable <<...>> attribute syntax *)
  tco_const_attribute: bool;  (** Allow <<__Const>> attribute *)
  po_const_default_func_args: bool;
      (** Statically check default function arguments *)
  po_const_default_lambda_args: bool;
      (** Statically check default lambda arguments. Subset of default_func_args *)
  po_disallow_silence: bool;
      (** Flag to disable the error suppression operator *)
  po_abstract_static_props: bool;  (** Static properties can be abstract *)
  po_parser_errors_only: bool;
      (** Ignore all errors except those that can influence the shape of syntax tree
         (skipping post parse error checks) *)
  tco_check_attribute_locations: bool;
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
  po_disallow_func_ptrs_in_constants: bool;
      (** Flag to disallow HH\fun and HH\class_meth in constants and constant initializers *)
  tco_error_php_lambdas: bool;
      (** Flag to report an error on php style anonymous functions *)
  tco_disallow_discarded_nullable_awaitables: bool;
      (** Flag to error on using discarded nullable awaitables *)
  po_enable_xhp_class_modifier: bool;
      (** Enable the new style xhp class.
         Old style: class :name {}
         New style: xhp class name {} *)
  po_disable_xhp_element_mangling: bool;
      (** Flag to disable the old stype xhp element mangling. `<something/>` would otherwise be resolved as `xhp_something`
         The new style `xhp class something {}` does not do this style of mangling, thus we need a way to disable it on the
         'lookup side'. *)
  po_disable_xhp_children_declarations: bool;
      (** Disable `children (foo|bar+|pcdata)` declarations as they can be implemented without special syntax *)
  po_disable_hh_ignore_error: int;
      (** Disable HH_IGNORE_ERROR comments, either raising an error if 1 or treating them as normal comments if 2. *)
  po_keep_user_attributes: bool;
      (** Parse all user attributes rather than only the ones needed for typing *)
  tco_is_systemlib: bool;  (** Enable features used to typecheck systemlib *)
  tco_higher_kinded_types: bool;
      (** Controls if higher-kinded types are supported *)
  tco_method_call_inference: bool;
      (** Controls if method-call inference is supported *)
  tco_report_pos_from_reason: bool;
      (** If set, then positions derived from reason information are tainted, and primary errors
         with such positions are flagged *)
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
  po_interpret_soft_types_as_like_types: bool;  (** <<__Soft>> T -> ~T *)
  tco_enable_strict_string_concat_interp: bool;
      (** Restricts string concatenation and interpolation to arraykeys *)
  tco_ignore_unsafe_cast: bool;
      (** Ignores unsafe_cast and retains the original type of the expression *)
  tco_no_parser_readonly_check: bool;
      (** Disable parser-based readonly checking *)
  tco_enable_expression_trees: bool;
      (** Enable expression trees via unstable features flag *)
  tco_enable_function_references: bool;
      (** Enable unstable feature: function references *)
  tco_allowed_expression_tree_visitors: string list;
      (** Allowed expression tree visitors when not enabled via unstable features flag *)
  tco_math_new_code: bool;
      (** Use a new error code for math operations: addition, subtraction,
        division, multiplication, exponentiation *)
  tco_typeconst_concrete_concrete_error: bool;
      (** Raise an error when a concrete type constant is overridden by a concrete type constant
         in a child class. *)
  tco_enable_strict_const_semantics: int;
      (** When the value is 1, raises a 4734 error when an inherited constant comes from a conflicting
         hierarchy, but not if the constant is locally defined. When the value is 2, raises a conflict
         any time two parents declare concrete constants with the same name, matching HHVM
         -vEval.TraitConstantInterfaceBehavior=1 *)
  tco_strict_wellformedness: int;
      (** Different levels here raise previously missing well-formedness errors (see Typing_type_wellformedness) *)
  tco_meth_caller_only_public_visibility: bool;
      (** meth_caller can only reference public methods *)
  tco_require_extends_implements_ancestors: bool;
      (** Consider `require extends` and `require implements` as ancestors when checking a class *)
  tco_strict_value_equality: bool;
      (** Emit an error when "==" or "!=" is used to compare values that are incompatible types *)
  tco_enforce_sealed_subclasses: bool;
      (** All member of the __Sealed whitelist should be subclasses*)
  tco_everything_sdt: bool;
      (** All classes are implcitly marked <<__SupportDynamicType>> *)
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
  tco_expression_tree_virtualize_functions: bool;
      (** When enabled, wrap function types in Expression Trees in user defined virtual function types *)
  tco_use_type_alias_heap: bool;
  tco_populate_dead_unsafe_cast_heap: bool;
      (** Dead UNSAFE_CAST codemod stashes patches through a TAST visitor in shared
         heap. This is only needed in dead UNSAFE_CAST removal mode. This option
         controls whether the heap will be populated or not. *)
  po_disallow_static_constants_in_default_func_args: bool;
  tco_rust_elab: bool;
      (** Use the Rust implementation of naming elaboration and NAST checks. *)
  dump_tast_hashes: bool;  (** Dump tast hashes in /tmp/hh_server/tast_hashes *)
  dump_tasts: string list;
      (** List of paths whose TASTs to be dumped in /tmp/hh_server/tasts *)
  tco_autocomplete_mode: bool;  (** Are we running in autocomplete mode ? *)
  tco_package_info: PackageInfo.t;
      (** Information used to determine which package a module belongs to during typechecking. *)
  po_unwrap_concurrent: bool;
      (** Replace concurrent blocks with their bodies in the AST *)
  tco_log_exhaustivity_check: bool;
      (** Instrument the existing exhaustivity lint (for strict switch statements) *)
  po_disallow_direct_superglobals_refs: bool;
      (** block accessing superglobals via their variable names *)
  tco_sticky_quarantine: bool;
      (** Controls behavior of [Provider_utils.respect_but_quarantine_unsaved_changes] *)
  tco_lsp_invalidation: bool;
      (** Controls how [Provicer_utils.respect_but_quarantine_unsaved_changes] invalidates folded decls *)
  tco_prefetch_decls: bool;
      (** Controls behavior of [Decl_provider.prefetch_and_lock_decls_needed_for_entry] *)
  tco_autocomplete_sort_text: bool;
  po_nameof_precedence: bool;  (** Make nameof bind tighter *)
  po_strict_utf8: bool;  (** Require utf8 in source files *)
}
[@@deriving eq, show]

val set :
  ?tco_saved_state:saved_state ->
  ?po_deregister_php_stdlib:bool ->
  ?po_disallow_toplevel_requires:bool ->
  ?tco_log_large_fanouts_threshold:int ->
  ?tco_log_inference_constraints:bool ->
  ?tco_experimental_features:SSet.t ->
  ?tco_migration_flags:SSet.t ->
  ?tco_num_local_workers:int ->
  ?tco_max_typechecker_worker_memory_mb:int ->
  ?tco_defer_class_declaration_threshold:int ->
  ?tco_locl_cache_capacity:int ->
  ?tco_locl_cache_node_threshold:int ->
  ?so_naming_sqlite_path:string ->
  ?po_auto_namespace_map:(string * string) list ->
  ?po_codegen:bool ->
  ?tco_language_feature_logging:bool ->
  ?tco_timeout:int ->
  ?tco_disallow_invalid_arraykey:bool ->
  ?tco_disallow_byref_dynamic_calls:bool ->
  ?tco_disallow_byref_calls:bool ->
  ?code_agnostic_fixme:bool ->
  ?allowed_fixme_codes_strict:ISet.t ->
  ?log_levels:int SMap.t ->
  ?po_disable_lval_as_an_expression:bool ->
  ?tco_remote_old_decls_no_limit:bool ->
  ?tco_use_old_decls_from_cas:bool ->
  ?tco_fetch_remote_old_decls:bool ->
  ?tco_populate_member_heaps:bool ->
  ?tco_skip_hierarchy_checks:bool ->
  ?tco_skip_tast_checks:bool ->
  ?tco_like_type_hints:bool ->
  ?tco_union_intersection_type_hints:bool ->
  ?tco_coeffects:bool ->
  ?tco_coeffects_local:bool ->
  ?tco_strict_contexts:bool ->
  ?tco_like_casts:bool ->
  ?tco_check_xhp_attribute:bool ->
  ?tco_check_redundant_generics:bool ->
  ?tco_disallow_unresolved_type_variables:bool ->
  ?tco_custom_error_config:Custom_error_config.t ->
  ?po_enable_class_level_where_clauses:bool ->
  ?po_disable_legacy_soft_typehints:bool ->
  ?po_allowed_decl_fixme_codes:ISet.t ->
  ?tco_const_static_props:bool ->
  ?po_disable_legacy_attribute_syntax:bool ->
  ?tco_const_attribute:bool ->
  ?po_const_default_func_args:bool ->
  ?po_const_default_lambda_args:bool ->
  ?po_disallow_silence:bool ->
  ?po_abstract_static_props:bool ->
  ?po_parser_errors_only:bool ->
  ?tco_check_attribute_locations:bool ->
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
  ?po_disallow_func_ptrs_in_constants:bool ->
  ?tco_error_php_lambdas:bool ->
  ?tco_disallow_discarded_nullable_awaitables:bool ->
  ?po_enable_xhp_class_modifier:bool ->
  ?po_disable_xhp_element_mangling:bool ->
  ?po_disable_xhp_children_declarations:bool ->
  ?po_disable_hh_ignore_error:int ->
  ?po_keep_user_attributes:bool ->
  ?po_allow_unstable_features:bool ->
  ?tco_is_systemlib:bool ->
  ?tco_higher_kinded_types:bool ->
  ?tco_method_call_inference:bool ->
  ?tco_report_pos_from_reason:bool ->
  ?tco_typecheck_sample_rate:float ->
  ?tco_enable_sound_dynamic:bool ->
  ?tco_pessimise_builtins:bool ->
  ?tco_enable_no_auto_dynamic:bool ->
  ?tco_skip_check_under_dynamic:bool ->
  ?tco_global_access_check_enabled:bool ->
  ?po_interpret_soft_types_as_like_types:bool ->
  ?tco_enable_strict_string_concat_interp:bool ->
  ?tco_ignore_unsafe_cast:bool ->
  ?tco_no_parser_readonly_check:bool ->
  ?tco_enable_expression_trees:bool ->
  ?tco_enable_function_references:bool ->
  ?tco_allowed_expression_tree_visitors:string list ->
  ?tco_math_new_code:bool ->
  ?tco_typeconst_concrete_concrete_error:bool ->
  ?tco_enable_strict_const_semantics:int ->
  ?tco_strict_wellformedness:int ->
  ?tco_meth_caller_only_public_visibility:bool ->
  ?tco_require_extends_implements_ancestors:bool ->
  ?tco_strict_value_equality:bool ->
  ?tco_enforce_sealed_subclasses:bool ->
  ?tco_everything_sdt:bool ->
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
  ?tco_expression_tree_virtualize_functions:bool ->
  ?tco_use_type_alias_heap:bool ->
  ?tco_populate_dead_unsafe_cast_heap:bool ->
  ?po_disallow_static_constants_in_default_func_args:bool ->
  ?tco_rust_elab:bool ->
  ?dump_tast_hashes:bool ->
  ?dump_tasts:string list ->
  ?tco_autocomplete_mode:bool ->
  ?tco_package_info:PackageInfo.t ->
  ?po_unwrap_concurrent:bool ->
  ?tco_log_exhaustivity_check:bool ->
  ?po_disallow_direct_superglobals_refs:bool ->
  ?tco_sticky_quarantine:bool ->
  ?tco_lsp_invalidation:bool ->
  ?tco_prefetch_decls:bool ->
  ?tco_autocomplete_sort_text:bool ->
  ?po_nameof_precedence:bool ->
  ?po_strict_utf8:bool ->
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
