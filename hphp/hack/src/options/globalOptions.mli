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

(** Naming conventions for fieds:
  - tco_<feature/flag/setting> - type checker option
  - po_<feature/flag/setting> - parser option
  - so_<feature/flag/setting> - server option *)
type t = {
  tco_saved_state: saved_state;
  (* Set of experimental features, in lowercase. *)
  tco_experimental_features: SSet.t;
  (* Set of opt-in migration behavior flags, in lowercase. *)
  tco_migration_flags: SSet.t;
  (* If set to 0, only the type check delegate's logic will be used.
     If the delegate fails to type check, the typing check service as a whole
     will fail. *)
  tco_num_local_workers: int option;
  (* If set, typechecker workers will quit after they exceed this limit *)
  tco_max_typechecker_worker_memory_mb: int option;
  (* If set, defers class declarations after N lazy declarations; if not set,
     always lazily declares classes not already in cache. *)
  tco_defer_class_declaration_threshold: int option;
  (* Whether the Eden prefetch hook should be invoked *)
  (* The capacity of the localization cache for large types *)
  tco_locl_cache_capacity: int;
  (* The number of nodes a type has to exceed to enter the localization cache *)
  tco_locl_cache_node_threshold: int;
  (* Enables the reverse naming table to fall back to SQLite for queries. *)
  so_naming_sqlite_path: string option;
  (* Namespace aliasing map *)
  po_auto_namespace_map: (string * string) list;
  (* Are we emitting bytecode? *)
  po_codegen: bool;
  (* Flag for disabling functions in HHI files with the __PHPStdLib attribute *)
  po_deregister_php_stdlib: bool;
  (* Flag to disallow `require`, `require_once` etc as toplevel statements *)
  po_disallow_toplevel_requires: bool;
  (* Allows enabling unstable features via the __EnableUnstableFeatures attribute *)
  po_allow_unstable_features: bool;
  tco_log_large_fanouts_threshold: int option;
  (* Print types of size bigger than 1000 after performing a type union. *)
  tco_log_inference_constraints: bool;
  (*
   * Flag to enable logging of statistics regarding use of language features.
   * Currently used for lambdas.
   *)
  tco_language_feature_logging: bool;
  (*
   * If non-zero, give up type checking a class or function after this many seconds
   *)
  tco_timeout: int;
  (*
   * Flag to disallow using values that get casted to array keys at runtime;
   * like bools, floats, or null; as array keys.
   *)
  tco_disallow_invalid_arraykey: bool;
  (*
   * Produces an error if an arguments is passed by reference to dynamically
   * called function [e.g. $foo(&$bar)].
   *)
  tco_disallow_byref_dynamic_calls: bool;
  (*
   * Produces an error if an arguments is passed by reference in any form
   * [e.g. foo(&$bar)].
   *)
  tco_disallow_byref_calls: bool;
  (* HH_FIXME should silence *any* error, not just the one specified by code *)
  code_agnostic_fixme: bool;
  (* Error codes for which we allow HH_FIXMEs in strict mode *)
  allowed_fixme_codes_strict: ISet.t;
  (* Initial hh_log_level settings *)
  log_levels: int SMap.t;
  (* Flag to disable using lvals as expressions. *)
  po_disable_lval_as_an_expression: bool;
  (* Flag to fetch old decls from remote decl store *)
  tco_remote_old_decls_no_limit: bool;
  (* Don't limit amount of remote old decls fetched *)
  tco_use_old_decls_from_cas: bool;
  (* Fetch old decls from CAS instead of memcache/manifold *)
  tco_fetch_remote_old_decls: bool;
  (* Populate the member signature heaps.

     If disabled, instead load lazily from shallow classes. *)
  tco_populate_member_heaps: bool;
  (* Skip checks on hierarchy e.g. overrides, require extend, etc.
     Set to true only for debugging purposes! *)
  tco_skip_hierarchy_checks: bool;
  (* Skip checks implemented with TAST visitors.
     Set to true only for debugging purposes! *)
  tco_skip_tast_checks: bool;
  (* Enables like type hints *)
  tco_like_type_hints: bool;
  (* Enables union and intersection type hints *)
  tco_union_intersection_type_hints: bool;
  (* Enables checking of coeffects *)
  tco_coeffects: bool;
  (* Enables checking of coeffects for local operations (not calls) *)
  tco_coeffects_local: bool;
  (* Internal (for tests-only): whether any type can appear in a context list
   * or only types defined in the appropriate Context namespace *)
  tco_strict_contexts: bool;
  (* Enables like casts *)
  tco_like_casts: bool;
  (* static check xhp required attribute *)
  tco_check_xhp_attribute: bool;
  (* Check redundant generics in return types *)
  tco_check_redundant_generics: bool;
  (*
   * Flag to produce an error whenever the TAST contains unresolved type variables
   *)
  tco_disallow_unresolved_type_variables: bool;
  (*
   * Allows additional error messages to be added to typing errors via config
   *)
  tco_custom_error_config: Custom_error_config.t;
  (* Enable class-level where clauses, i.e.
     class base<T> where T = int {} *)
  po_enable_class_level_where_clauses: bool;
  (* Disable legacy soft typehint syntax (@int) and only allow the __Soft attribute. *)
  po_disable_legacy_soft_typehints: bool;
  (* Set of error codes disallowed in decl positions *)
  po_allowed_decl_fixme_codes: ISet.t;
  (* Enable @ attribute syntax *)
  po_allow_new_attribute_syntax: bool;
  (* Enable const static properties *)
  tco_const_static_props: bool;
  (* Disable <<...>> attribute syntax *)
  po_disable_legacy_attribute_syntax: bool;
  (* Allow <<__Const>> attribute *)
  tco_const_attribute: bool;
  (* Statically check default function arguments *)
  po_const_default_func_args: bool;
  (* Statically check default lambda arguments. Subset of default_func_args *)
  po_const_default_lambda_args: bool;
  (* Flag to disable the error suppression operator *)
  po_disallow_silence: bool;
  (* Static properties can be abstract *)
  po_abstract_static_props: bool;
  (* Ignore all errors except those that can influence the shape of syntax tree
   * (skipping post parse error checks) *)
  po_parser_errors_only: bool;
  tco_check_attribute_locations: bool;
  (* Reponame used for glean connection, default to "www.autocomplete" *)
  glean_reponame: string;
  (* Path prefix to use for files relative to the repository root when writing symbol info to JSON *)
  autocomplete_cache: bool;
  symbol_write_index_inherited_members: bool;
  (* Index inherited members information from folded decls *)
  symbol_write_ownership: bool;
  (* include fact ownership information, for creating incremental databases. *)
  symbol_write_root_path: string;
  (* Path prefix to use for hhi files when writing symbol info to JSON *)
  symbol_write_hhi_path: string;
  (* Filepaths to ignore when writing symbol info to JSON, relative to path prefix, eg: root|foo.php *)
  symbol_write_ignore_paths: string list;
  (* When set, write indexing data for these filepaths only. Relative to repository root, eg: bar.php for root|bar.php *)
  symbol_write_index_paths: string list;
  (* A file which contains a list of Relative_path.t (one per line) to index *)
  symbol_write_index_paths_file: string option;
  (* Write the list of Relative_path.t to this file instead of indexing. Useful for sharding *)
  symbol_write_index_paths_file_output: string option;
  (* Write symbol indexing data for hhi files *)
  symbol_write_include_hhi: bool;
  (* Use symbols hash table for incremental indexing *)
  symbol_write_sym_hash_in: string option;
  (* Path to file containing units to exclude *)
  symbol_write_exclude_out: string option;
  (* Path to file containing referenced files *)
  symbol_write_referenced_out: string option;
  (* Generate symbols hash table *)
  symbol_write_reindexed_out: string option;
  (* Generate reindexed files *)
  symbol_write_sym_hash_out: bool;
  (* Flag to disallow HH\fun and HH\class_meth in constants and constant initializers *)
  po_disallow_func_ptrs_in_constants: bool;
  (* Flag to report an error on php style anonymous functions *)
  tco_error_php_lambdas: bool;
  (* Flag to error on using discarded nullable awaitables *)
  tco_disallow_discarded_nullable_awaitables: bool;
  (* Enable the new style xhp class.
   * Old style: class :name {}
   * New style: xhp class name {}
   *)
  po_enable_xhp_class_modifier: bool;
  (*
   * Flag to disable the old stype xhp element mangling. `<something/>` would otherwise be resolved as `xhp_something`
   * The new style `xhp class something {}` does not do this style of mangling, thus we need a way to disable it on the
   * 'lookup side'.
   *)
  po_disable_xhp_element_mangling: bool;
  (* Disable `children (foo|bar+|pcdata)` declarations as they can be implemented without special syntax *)
  po_disable_xhp_children_declarations: bool;
  (* Disable HH_IGNORE_ERROR comments, either raising an error if 1 or treating them as normal comments if 2. *)
  po_disable_hh_ignore_error: int;
  (* Parse all user attributes rather than only the ones needed for typing *)
  po_keep_user_attributes: bool;
  (* Enable features used to typecheck systemlib *)
  tco_is_systemlib: bool;
  (* Controls if higher-kinded types are supported *)
  tco_higher_kinded_types: bool;
  (* Controls if method-call inference is supported *)
  tco_method_call_inference: bool;
  (* If set, then positions derived from reason information are tainted, and primary errors
   * with such positions are flagged
   *)
  tco_report_pos_from_reason: bool;
  (* Type check this proportion of all files. Default is 1.0.
   * DO NOT set to any other value except for testing purposes.
   *)
  tco_typecheck_sample_rate: float;
  (* Experimental implementation of a "sound" dynamic type *)
  tco_enable_sound_dynamic: bool;
  (* Under sound dynamic, introduce like-types for built-in operations e.g. on Vector.
     This is done anyway if everything_sdt=true
  *)
  tco_pessimise_builtins: bool;
  (* Allow use of attribute <<__NoAutoDynamic>> *)
  tco_enable_no_auto_dynamic: bool;
  (* Skip second check of method under dynamic assumptions *)
  tco_skip_check_under_dynamic: bool;
  (* Enable ifc on the specified list of path prefixes
     (a list containing the empty string would denote all files,
     an empty list denotes no files) *)
  tco_ifc_enabled: string list;
  (* Enable global access checker to check global writes and reads *)
  tco_global_access_check_enabled: bool;
  (* <<__Soft>> T -> ~T *)
  po_interpret_soft_types_as_like_types: bool;
  (* Restricts string concatenation and interpolation to arraykeys *)
  tco_enable_strict_string_concat_interp: bool;
  (* Ignores unsafe_cast and retains the original type of the expression *)
  tco_ignore_unsafe_cast: bool;
  (* Disable parser-based readonly checking *)
  tco_no_parser_readonly_check: bool;
  (* Enable expression trees via unstable features flag *)
  tco_enable_expression_trees: bool;
  (* Enable unstable feature: modules *)
  tco_enable_modules: bool;
  (* Allowed expression tree visitors when not enabled via unstable features flag *)
  tco_allowed_expression_tree_visitors: string list;
  (* Use a new error code for math operations: addition, subtraction,
     division, multiplication, exponentiation *)
  tco_math_new_code: bool;
  (* Raise an error when a concrete type constant is overridden by a concrete type constant
     in a child class. *)
  tco_typeconst_concrete_concrete_error: bool;
  (* When the value is 1, raises a 4734 error when an inherited constant comes from a conflicting
   * hierarchy, but not if the constant is locally defined. When the value is 2, raises a conflict
   * any time two parents declare concrete constants with the same name, matching HHVM
   * -vEval.TraitConstantInterfaceBehavior=1 *)
  tco_enable_strict_const_semantics: int;
  (* Different levels here raise previously missing well-formedness errors (see Typing_type_wellformedness) *)
  tco_strict_wellformedness: int;
  (* meth_caller can only reference public methods *)
  tco_meth_caller_only_public_visibility: bool;
  (* Consider `require extends` and `require implements` as ancestors when checking a class *)
  tco_require_extends_implements_ancestors: bool;
  (* Emit an error when "==" or "!=" is used to compare values that are incompatible types *)
  tco_strict_value_equality: bool;
  (* All member of the __Sealed whitelist should be subclasses*)
  tco_enforce_sealed_subclasses: bool;
  (* All classes are implcitly marked <<__SupportDynamicType>> *)
  tco_everything_sdt: bool;
  (* Raises an error when a classish is declared <<__ConsistentConstruct>> but lacks an
   * explicit constructor declaration. 0 does not raise, 1 raises for traits, 2 raises
   * for all classish *)
  tco_explicit_consistent_constructors: int;
  (* Raises an error when a class constant is missing a type. 0 does not raise, 1 raises
   * for abstract class constants, 2 raises for all. *)
  tco_require_types_class_consts: int;
  (* Sets the amount of fuel that the type printer can use to display an
   * individual type. More of a type is printed as the value increases. *)
  tco_type_printer_fuel: int;
  (* allows saved_state_loader to shell out to hg to find globalrev and timestamp of revisions *)
  tco_specify_manifold_api_key: bool;
  (* Measures and reports the time it takes to typecheck each top-level
     definition. *)
  tco_profile_top_level_definitions: bool;
  tco_allow_all_files_for_module_declarations: bool;
  tco_allowed_files_for_module_declarations: string list;
  (* If enabled, the type checker records more fine-grained dependencies than usual,
     for example between individual methods. *)
  tco_record_fine_grained_dependencies: bool;
  (* When set, uses the given number of iterations while typechecking loops *)
  tco_loop_iteration_upper_bound: int option;
  (* When enabled, wrap function types in Expression Trees in user defined virtual function types *)
  tco_expression_tree_virtualize_functions: bool;
  (* When set, mutates generic entities by substituting type parameters and
     typechecks the mutated program *)
  tco_substitution_mutation: bool;
  tco_use_type_alias_heap: bool;
  (* Dead UNSAFE_CAST codemod stashes patches through a TAST visitor in shared
     heap. This is only needed in dead UNSAFE_CAST removal mode. This option
     controls whether the heap will be populated or not. *)
  tco_populate_dead_unsafe_cast_heap: bool;
  po_disallow_static_constants_in_default_func_args: bool;
  tco_load_hack_64_distc_saved_state: bool;
  (* Produce variations of methods and functions in TASTs that are cheked under
     dynamic assumptions. *)
  (* Use the Rust implementation of naming elaboration and NAST checks. *)
  tco_rust_elab: bool;
  dump_tast_hashes: bool;  (** Dump tast hashes in /tmp/hh_server/tast_hashes *)
  tco_autocomplete_mode: bool;  (** Are we running in autocomplete mode ? *)
  tco_package_info: PackageInfo.t;
      (** Information used to determine which package a module belongs to during typechecking. *)
  po_unwrap_concurrent: bool;
      (** Replace concurrent blocks with their bodies in the AST *)
  tco_log_exhaustivity_check: bool;
      (** Instrument the existing exhaustivity lint (for strict switch statements) *)
  po_disallow_direct_superglobals_refs: bool;
      (** block accessing superglobals via their variable names *)
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
  ?po_allow_new_attribute_syntax:bool ->
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
  ?autocomplete_cache:bool ->
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
  ?tco_ifc_enabled:string list ->
  ?tco_global_access_check_enabled:bool ->
  ?po_interpret_soft_types_as_like_types:bool ->
  ?tco_enable_strict_string_concat_interp:bool ->
  ?tco_ignore_unsafe_cast:bool ->
  ?tco_no_parser_readonly_check:bool ->
  ?tco_enable_expression_trees:bool ->
  ?tco_enable_modules:bool ->
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
  ?tco_substitution_mutation:bool ->
  ?tco_use_type_alias_heap:bool ->
  ?tco_populate_dead_unsafe_cast_heap:bool ->
  ?po_disallow_static_constants_in_default_func_args:bool ->
  ?tco_load_hack_64_distc_saved_state:bool ->
  ?tco_rust_elab:bool ->
  ?dump_tast_hashes:bool ->
  ?tco_autocomplete_mode:bool ->
  ?tco_package_info:PackageInfo.t ->
  ?po_unwrap_concurrent:bool ->
  ?tco_log_exhaustivity_check:bool ->
  ?po_disallow_direct_superglobals_refs:bool ->
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
