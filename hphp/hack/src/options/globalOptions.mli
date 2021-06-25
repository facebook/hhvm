(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Naming conventions in this file:
  - tco_<feature/flag/setting> - type checker option
  - po_<feature/flag/setting> - parser option
  - so_<feature/flag/setting> - server option
*)

type t = {
  (* Set of experimental features, in lowercase. *)
  tco_experimental_features: SSet.t;
  (* Set of opt-in migration behavior flags, in lowercase. *)
  tco_migration_flags: SSet.t;
  (* Whether to treat Tany as Tdynamic *)
  tco_dynamic_view: bool;
  (* If set to 0, only the type check delegate's logic will be used.
    If the delegate fails to type check, the typing check service as a whole
    will fail. *)
  tco_num_local_workers: int option;
  (* If the number of files to type check is fewer than this value, the files
    will be type checked sequentially (in the master process). Otherwise,
    the files will be type checked in parallel (in MultiWorker workers). *)
  tco_parallel_type_checking_threshold: int;
  (* If set, typechecker workers will quit after they exceed this limit *)
  tco_max_typechecker_worker_memory_mb: int option;
  (* If set, defers class declarations after N lazy declarations; if not set,
    always lazily declares classes not already in cache. *)
  tco_defer_class_declaration_threshold: int option;
  (* If set, defers class declarations if worker memory exceeds threshold.
    This prevents OOMs due to a single file fetching a lot of decls, which would
    not be prevented by [tco_max_typechecker_worker_memory_mb] which is checked
    only after each file. It doesn't make sense to set this higher
    than [tco_max_typechecker_worker_memory_mb]. *)
  tco_defer_class_memory_mb_threshold: int option;
  (* If set, prevents type checking of files from being deferred more than
    the number of times greater than or equal to the threshold. If not set,
    defers class declarations indefinitely. *)
  tco_max_times_to_defer_type_checking: int option;
  (* Whether the Eden prefetch hook should be invoked *)
  tco_prefetch_deferred_files: bool;
  (* If set, distributes type checking to remote workers if the number of files to
   type check exceeds the threshold. If not set, then always checks everything locally. *)
  tco_remote_type_check_threshold: int option;
  (* Turns on remote type checking *)
  tco_remote_type_check: bool;
  (* If set, uses the key to fetch type checking jobs *)
  tco_remote_worker_key: string option;
  (* If set, uses the check ID when logging events in the context of remove init/work *)
  tco_remote_check_id: string option;
  (* The max batch size that a remote worker can receive to type check *)
  tco_remote_max_batch_size: int;
  (* The min batch size that a remote worker can receive to type check *)
  tco_remote_min_batch_size: int;
  (* Dictates the number of remote type checking workers *)
  tco_num_remote_workers: int;
  tco_stream_errors: bool;
      (** Whether to send errors to the IDE as soon as they are discovered. *)
  (* The version specifier that is used to identify the remote worker package version to install *)
  so_remote_version_specifier: string option;
  (* Above this threshold of files to check, the remote type checking worker will not use Eden *)
  so_remote_worker_vfs_checkout_threshold: int;
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
  (* Flag to disable PHP's non-top-level declarations *)
  po_disable_nontoplevel_declarations: bool;
  (* Allows enabling unstable features via the __EnableUnstableFeatures attribute *)
  po_allow_unstable_features: bool;
  (* Print types of size bigger than 1000 after performing a type union. *)
  tco_log_inference_constraints: bool;
  (*
   * Flag to disallow array typehints
   *)
  tco_disallow_array_typehint: bool;
  (*
   * Flag to disallow array literal expressions
   *)
  tco_disallow_array_literal: bool;
  (*
   * Flag to enable logging of statistics regarding use of language features.
   * Currently used for lambdas.
   *)
  tco_language_feature_logging: bool;
  (*
   * When enabled, mismatches between the types of the scrutinee and case value
   * of a switch expression are reported as type errors.
   *)
  tco_disallow_scrutinee_case_value_type_mismatch: bool;
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
  (* Error codes for which we allow HH_FIXMEs in strict mode *)
  allowed_fixme_codes_strict: ISet.t;
  allowed_fixme_codes_partial: ISet.t;
  codes_not_raised_partial: ISet.t;
  (* Initial hh_log_level settings *)
  log_levels: int SMap.t;
  (* Flag to disable using lvals as expressions. *)
  po_disable_lval_as_an_expression: bool;
  (* Flag to ignore the string in vec<string>[...] *)
  (* Look up class members lazily from shallow declarations instead of eagerly
     computing folded declarations representing the entire class type. *)
  tco_shallow_class_decl: bool;
  (* Use Rust parser errors *)
  po_rust_parser_errors: bool;
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
  (* A simpler form of pessimization, only wraps the outermost type in like
   * if the type is not enforceable *)
  tco_simple_pessimize: float;
  (* Enables complex coercion interactions that involve like types *)
  tco_complex_coercion: bool;
  (* Treat partially abstract typeconsts like concrete typeconsts, disable overriding type *)
  tco_disable_partially_abstract_typeconsts: bool;
  (* Ban definitions of partially abstract typeconsts *)
  tco_disallow_partially_abstract_typeconst_definitions: bool;
  (* Set of codes to be treated as if they were in strict mode files *)
  error_codes_treated_strictly: ISet.t;
  (* static check xhp required attribute *)
  tco_check_xhp_attribute: bool;
  (* Check redundant generics in return types *)
  tco_check_redundant_generics: bool;
  (*
   * Flag to produce an error whenever the TAST contains unresolved type variables
   *)
  tco_disallow_unresolved_type_variables: bool;
  (* Ban use of traits that are already used in parent classes. *)
  tco_disallow_trait_reuse: bool;
  (* Disallow using non-string, non-int types as array key type constraints. *)
  tco_disallow_invalid_arraykey_constraint: bool;
  (* Enable class-level where clauses, i.e.
     class base<T> where T = int {} *)
  po_enable_class_level_where_clauses: bool;
  (* Disable legacy soft typehint syntax (@int) and only allow the __Soft attribute. *)
  po_disable_legacy_soft_typehints: bool;
  (* Set of error codes disallowed in decl positions *)
  po_allowed_decl_fixme_codes: ISet.t;
  (* Enable @ attribute syntax *)
  po_allow_new_attribute_syntax: bool;
  (* Perform global inference globally on the code base to infer missing type annotations. *)
  tco_global_inference: bool;
  tco_gi_reinfer_types: string list;
      (** Types we want to remove and replace by infered types during global inference. *)
  tco_ordered_solving: bool;
      (** Whether to solve typing inference constraints using ordered solving or transitive closure. *)
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
  (* Make unsetting a class constant a parse error *)
  po_disable_unset_class_const: bool;
  (* Ignore all errors except those that can influence the shape of syntax tree
   * (skipping post parse error checks) *)
  po_parser_errors_only: bool;
  tco_check_attribute_locations: bool;
  (* Service name for glean connection; default "" to autoselect server *)
  glean_service: string;
  (* Hostname for glean connection; default "" to autoselect server *)
  glean_hostname: string;
  (* Port number for glean connection; default 0 to autoselect server *)
  glean_port: int;
  (* Reponame used for glean connection, default to "www.autocomplete" *)
  glean_reponame: string;
  (* Path prefix to use for files relative to the repository root when writing symbol info to JSON *)
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
  (* Enable enum class syntax *)
  po_enable_enum_classes: bool;
  (* Treats partial files as strict *)
  po_disable_modes: bool;
  po_disable_hh_ignore_error: bool;
  (* Disable array(...) *)
  po_disable_array: bool;
  po_disable_array_typehint: bool;
  (* Enable features used to typecheck systemlib *)
  tco_enable_systemlib_annotations: bool;
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
  (* Disallow #-style comments, except hashbangs(#!) *)
  po_disallow_hash_comments: bool;
  (* Disable parsing of fun() and class_meth() *)
  po_disallow_fun_and_cls_meth_pseudo_funcs: bool;
  (* Disable parsing of inst_meth() *)
  po_disallow_inst_meth: bool;
  (* Escape brace in \{$x} *)
  po_escape_brace: bool;
  (* Enable use of the direct decl parser for parsing type signatures. *)
  tco_use_direct_decl_parser: bool;
  (* Enable ifc on the specified list of path prefixes
    (a list containing the empty string would denote all files,
    an empty list denotes no files) *)
  tco_ifc_enabled: string list;
  (* Enables the enum supertyping extension *)
  po_enable_enum_supertyping: bool;
  (* <<__Soft>> T -> ~T *)
  po_interpret_soft_types_as_like_types: bool;
  (* Restricts string concatenation and interpolation to arraykeys *)
  tco_enable_strict_string_concat_interp: bool;
  (* Ignores unsafe_cast and retains the original type of the expression *)
  tco_ignore_unsafe_cast: bool;
  (* Enable Unstable feature readonly tast check *)
  tco_readonly: bool;
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
  (* meth_caller can only reference public methods *)
  tco_meth_caller_only_public_visibility: bool;
  (* Consider `require extends` and `require implements` as ancestors when checking a class *)
  tco_require_extends_implements_ancestors: bool;
  (* Emit an error when "==" or "!=" is used to compare values that are incompatible types *)
  tco_strict_value_equality: bool;
  (* All member of the __Sealed whitelist should be subclasses*)
  tco_enforce_sealed_subclasses: bool;
}
[@@deriving eq, show]

val make :
  ?po_deregister_php_stdlib:bool ->
  ?po_disallow_toplevel_requires:bool ->
  ?po_disable_nontoplevel_declarations:bool ->
  ?tco_log_inference_constraints:bool ->
  ?tco_experimental_features:SSet.t ->
  ?tco_migration_flags:SSet.t ->
  ?tco_dynamic_view:bool ->
  ?tco_num_local_workers:int ->
  ?tco_parallel_type_checking_threshold:int ->
  ?tco_max_typechecker_worker_memory_mb:int ->
  ?tco_defer_class_declaration_threshold:int ->
  ?tco_defer_class_memory_mb_threshold:int ->
  ?tco_max_times_to_defer_type_checking:int ->
  ?tco_prefetch_deferred_files:bool ->
  ?tco_remote_type_check_threshold:int ->
  ?tco_remote_type_check:bool ->
  ?tco_remote_worker_key:string ->
  ?tco_remote_check_id:string ->
  ?tco_remote_max_batch_size:int ->
  ?tco_remote_min_batch_size:int ->
  ?tco_num_remote_workers:int ->
  ?tco_stream_errors:bool ->
  ?so_remote_version_specifier:string ->
  ?so_remote_worker_vfs_checkout_threshold:int ->
  ?so_naming_sqlite_path:string ->
  ?po_auto_namespace_map:(string * string) list ->
  ?tco_disallow_array_typehint:bool ->
  ?tco_disallow_array_literal:bool ->
  ?tco_language_feature_logging:bool ->
  ?tco_disallow_scrutinee_case_value_type_mismatch:bool ->
  ?tco_timeout:int ->
  ?tco_disallow_invalid_arraykey:bool ->
  ?tco_disallow_byref_dynamic_calls:bool ->
  ?tco_disallow_byref_calls:bool ->
  ?allowed_fixme_codes_strict:ISet.t ->
  ?allowed_fixme_codes_partial:ISet.t ->
  ?codes_not_raised_partial:ISet.t ->
  ?log_levels:int SMap.t ->
  ?po_disable_lval_as_an_expression:bool ->
  ?tco_shallow_class_decl:bool ->
  ?po_rust_parser_errors:bool ->
  ?tco_like_type_hints:bool ->
  ?tco_union_intersection_type_hints:bool ->
  ?tco_coeffects:bool ->
  ?tco_coeffects_local:bool ->
  ?tco_strict_contexts:bool ->
  ?tco_like_casts:bool ->
  ?tco_simple_pessimize:float ->
  ?tco_complex_coercion:bool ->
  ?tco_disable_partially_abstract_typeconsts:bool ->
  ?tco_disallow_partially_abstract_typeconst_definitions:bool ->
  ?error_codes_treated_strictly:ISet.t ->
  ?tco_check_xhp_attribute:bool ->
  ?tco_check_redundant_generics:bool ->
  ?tco_disallow_unresolved_type_variables:bool ->
  ?tco_disallow_trait_reuse:bool ->
  ?tco_disallow_invalid_arraykey_constraint:bool ->
  ?po_enable_class_level_where_clauses:bool ->
  ?po_disable_legacy_soft_typehints:bool ->
  ?po_allowed_decl_fixme_codes:ISet.t ->
  ?po_allow_new_attribute_syntax:bool ->
  ?tco_global_inference:bool ->
  ?tco_gi_reinfer_types:string list ->
  ?tco_ordered_solving:bool ->
  ?tco_const_static_props:bool ->
  ?po_disable_legacy_attribute_syntax:bool ->
  ?tco_const_attribute:bool ->
  ?po_const_default_func_args:bool ->
  ?po_const_default_lambda_args:bool ->
  ?po_disallow_silence:bool ->
  ?po_abstract_static_props:bool ->
  ?po_disable_unset_class_const:bool ->
  ?po_parser_errors_only:bool ->
  ?tco_check_attribute_locations:bool ->
  ?glean_service:string ->
  ?glean_hostname:string ->
  ?glean_port:int ->
  ?glean_reponame:string ->
  ?symbol_write_root_path:string ->
  ?symbol_write_hhi_path:string ->
  ?symbol_write_ignore_paths:string list ->
  ?symbol_write_index_paths:string list ->
  ?symbol_write_index_paths_file:string ->
  ?symbol_write_index_paths_file_output:string ->
  ?symbol_write_include_hhi:bool ->
  ?po_disallow_func_ptrs_in_constants:bool ->
  ?tco_error_php_lambdas:bool ->
  ?tco_disallow_discarded_nullable_awaitables:bool ->
  ?po_enable_xhp_class_modifier:bool ->
  ?po_disable_xhp_element_mangling:bool ->
  ?po_disable_xhp_children_declarations:bool ->
  ?po_enable_enum_classes:bool ->
  ?po_disable_modes:bool ->
  ?po_disable_hh_ignore_error:bool ->
  ?po_disable_array:bool ->
  ?po_disable_array_typehint:bool ->
  ?po_allow_unstable_features:bool ->
  ?tco_enable_systemlib_annotations:bool ->
  ?tco_higher_kinded_types:bool ->
  ?tco_method_call_inference:bool ->
  ?tco_report_pos_from_reason:bool ->
  ?tco_typecheck_sample_rate:float ->
  ?tco_enable_sound_dynamic:bool ->
  ?po_disallow_hash_comments:bool ->
  ?po_disallow_fun_and_cls_meth_pseudo_funcs:bool ->
  ?po_disallow_inst_meth:bool ->
  ?po_escape_brace:bool ->
  ?tco_use_direct_decl_parser:bool ->
  ?tco_ifc_enabled:string list ->
  ?po_enable_enum_supertyping:bool ->
  ?po_interpret_soft_types_as_like_types:bool ->
  ?tco_enable_strict_string_concat_interp:bool ->
  ?tco_ignore_unsafe_cast:bool ->
  ?tco_readonly:bool ->
  ?tco_enable_expression_trees:bool ->
  ?tco_enable_modules:bool ->
  ?tco_allowed_expression_tree_visitors:string list ->
  ?tco_math_new_code:bool ->
  ?tco_typeconst_concrete_concrete_error:bool ->
  ?tco_meth_caller_only_public_visibility:bool ->
  ?tco_require_extends_implements_ancestors:bool ->
  ?tco_strict_value_equality:bool ->
  ?tco_enforce_sealed_subclasses:bool ->
  unit ->
  t

val tco_experimental_feature_enabled : t -> SSet.elt -> bool

val tco_migration_flag_enabled : t -> SSet.elt -> bool

val tco_dynamic_view : t -> bool

val tco_num_local_workers : t -> int option

val tco_parallel_type_checking_threshold : t -> int

val tco_max_typechecker_worker_memory_mb : t -> int option

val tco_defer_class_declaration_threshold : t -> int option

val tco_defer_class_memory_mb_threshold : t -> int option

val tco_max_times_to_defer_type_checking : t -> int option

val tco_prefetch_deferred_files : t -> bool

val tco_remote_type_check_threshold : t -> int option

val tco_remote_type_check : t -> bool

val tco_remote_worker_key : t -> string option

val tco_remote_check_id : t -> string option

val tco_remote_max_batch_size : t -> int

val tco_remote_min_batch_size : t -> int

val tco_num_remote_workers : t -> int

val tco_stream_errors : t -> bool

val so_remote_version_specifier : t -> string option

val so_remote_worker_vfs_checkout_threshold : t -> int

val so_naming_sqlite_path : t -> string option

val po_auto_namespace_map : t -> (string * string) list

val po_deregister_php_stdlib : t -> bool

val po_disallow_toplevel_requires : t -> bool

val po_disable_nontoplevel_declarations : t -> bool

val po_codegen : t -> bool

val tco_log_inference_constraints : t -> bool

val tco_disallow_array_typehint : t -> bool

val tco_disallow_array_literal : t -> bool

val tco_language_feature_logging : t -> bool

val tco_disallow_scrutinee_case_value_type_mismatch : t -> bool

val tco_timeout : t -> int

val tco_disallow_invalid_arraykey : t -> bool

val tco_disallow_byref_dynamic_calls : t -> bool

val tco_disallow_byref_calls : t -> bool

val default : t

val tco_experimental_generics_arity : string

val tco_experimental_forbid_nullable_cast : string

val tco_experimental_disallow_static_memoized : string

val tco_experimental_type_param_shadowing : string

val tco_experimental_abstract_type_const_with_default : string

val tco_experimental_infer_flows : string

val tco_experimental_case_sensitive_inheritance : string

val tco_experimental_all : SSet.t

val tco_migration_flags_all : SSet.t

val allowed_fixme_codes_strict : t -> ISet.t

val allowed_fixme_codes_partial : t -> ISet.t

val codes_not_raised_partial : t -> ISet.t

val log_levels : t -> int SMap.t

val po_disable_lval_as_an_expression : t -> bool

val tco_shallow_class_decl : t -> bool

val po_rust_parser_errors : t -> bool

val tco_like_type_hints : t -> bool

val tco_union_intersection_type_hints : t -> bool

val tco_call_coeffects : t -> bool

val tco_local_coeffects : t -> bool

val tco_strict_contexts : t -> bool

val ifc_enabled : t -> string list

val enable_ifc : t -> t

val tco_like_casts : t -> bool

val tco_simple_pessimize : t -> float

val tco_complex_coercion : t -> bool

val tco_disable_partially_abstract_typeconsts : t -> bool

val tco_disallow_partially_abstract_typeconst_definitions : t -> bool

val error_codes_treated_strictly : t -> ISet.t

val tco_check_xhp_attribute : t -> bool

val tco_check_redundant_generics : t -> bool

val tco_disallow_unresolved_type_variables : t -> bool

val tco_disallow_trait_reuse : t -> bool

val tco_disallow_invalid_arraykey_constraint : t -> bool

val po_enable_class_level_where_clauses : t -> bool

val po_disable_legacy_soft_typehints : t -> bool

val po_allowed_decl_fixme_codes : t -> ISet.t

val po_allow_new_attribute_syntax : t -> bool

val tco_global_inference : t -> bool

val tco_gi_reinfer_types : t -> string list

val tco_ordered_solving : t -> bool

val tco_const_static_props : t -> bool

val po_disable_legacy_attribute_syntax : t -> bool

val tco_const_attribute : t -> bool

val po_const_default_func_args : t -> bool

val po_const_default_lambda_args : t -> bool

val po_disallow_silence : t -> bool

val po_abstract_static_props : t -> bool

val po_allow_unstable_features : t -> bool

val po_disable_unset_class_const : t -> bool

val set_global_inference : t -> t

val set_ordered_solving : t -> bool -> t

val po_parser_errors_only : t -> bool

val tco_check_attribute_locations : t -> bool

val glean_service : t -> string

val glean_hostname : t -> string

val glean_port : t -> int

val glean_reponame : t -> string

val symbol_write_root_path : t -> string

val symbol_write_hhi_path : t -> string

val symbol_write_ignore_paths : t -> string list

val symbol_write_index_paths : t -> string list

val symbol_write_index_paths_file : t -> string option

val symbol_write_index_paths_file_output : t -> string option

val symbol_write_include_hhi : t -> bool

val po_disallow_func_ptrs_in_constants : t -> bool

val tco_error_php_lambdas : t -> bool

val tco_disallow_discarded_nullable_awaitables : t -> bool

val po_enable_xhp_class_modifier : t -> bool

val po_disable_xhp_element_mangling : t -> bool

val po_disable_xhp_children_declarations : t -> bool

val po_enable_enum_classes : t -> bool

val po_disable_modes : t -> bool

val po_disable_hh_ignore_error : t -> bool

val po_disable_array : t -> bool

val po_disable_array_typehint : t -> bool

val tco_enable_systemlib_annotations : t -> bool

val tco_higher_kinded_types : t -> bool

val tco_method_call_inference : t -> bool

val tco_report_pos_from_reason : t -> bool

val tco_typecheck_sample_rate : t -> float

val tco_enable_sound_dynamic : t -> bool

val po_disallow_hash_comments : t -> bool

val po_disallow_fun_and_cls_meth_pseudo_funcs : t -> bool

val po_disallow_inst_meth : t -> bool

val po_escape_brace : t -> bool

val tco_use_direct_decl_parser : t -> bool

val po_enable_enum_supertyping : t -> bool

val po_interpret_soft_types_as_like_types : t -> bool

val tco_enable_strict_string_concat_interp : t -> bool

val tco_ignore_unsafe_cast : t -> bool

val tco_readonly : t -> bool

val set_tco_readonly : t -> bool -> t

val set_tco_enable_expression_trees : t -> bool -> t

val tco_enable_modules : t -> bool

val set_tco_enable_modules : t -> bool -> t

val expression_trees_enabled : t -> bool

val allowed_expression_tree_visitors : t -> string list

val tco_math_new_code : t -> bool

val tco_typeconst_concrete_concrete_error : t -> bool

val tco_meth_caller_only_public_visibility : t -> bool

val tco_require_extends_implements_ancestors : t -> bool

val tco_strict_value_equality : t -> bool

val tco_enforce_sealed_subclasses : t -> bool
