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

module InferMissing : sig
  type t =
    | Deactivated
    | Infer_return
    | Infer_params
    | Infer_global
  [@@deriving show]

  val can_infer_return : t -> bool

  val can_infer_params : t -> bool

  val global_inference : t -> bool

  val is_on : t -> bool

  val from_string : string -> t

  val from_string_opt : string option -> t
end

type t = {
  (*
   * Enforces array subtyping relationships.
   *
   * There are a few kinds of arrays in Hack:
   *   1. array: This is a type parameter-less array, that behaves like a PHP
   *      array, but may be used in place of the arrays listed below.
   *   2. array<T>: This is a vector-like array. It may be implemented with a
   *      compact representation. Keys are integer indices. Values are of type T.
   *   3. array<Tk, Tv>: This is a dictionary-like array. It is generally
   *      implemented as a hash table. Keys are of type Tk. Values are of type
   *      Tv.
   *
   * Unfortunately, there is no consistent subtyping relationship between these
   * types:
   *   1. An array<T> may be provided where an array is required.
   *   2. An array may be provided where an array<T> is required.
   *   3. An array<Tk, Tv> may be provided where an array is required.
   *   4. An array may be provided where an array<Tk, Tv> is required.
   *
   * This option enforces a stricter subtyping relationship within these types.
   * In particular, when enabled, points 2. and 4. from the above list become
   * typing errors.
   *)
  tco_safe_array: bool;
  (*
   * Enforces that a vector-like array may not be used where a hashtable-like
   * array is required.
   *
   * When disabled, Hack assumes the following:
   *
   *   array<T> <: array<int, T>
   *
   * When enabled, there is no subtyping relationship between array<T> and
   * array<int, T>.
   *)
  tco_safe_vector_array: bool;
  (* Set of experimental features, in lowercase. *)
  tco_experimental_features: SSet.t;
  (* Set of opt-in migration behavior flags, in lowercase. *)
  tco_migration_flags: SSet.t;
  (* Whether to treat Tany as  Tdynamic *)
  tco_dynamic_view: bool;
  (* If set, defers class declarations after N lazy declarations; if not set,
    always lazily declares classes not already in cache. *)
  tco_defer_class_declaration_threshold: int option;
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
  (* Dictates the number of remote type checking workers *)
  tco_num_remote_workers: int;
  (* The version specifier that is used to identify the remote worker package version to install *)
  so_remote_version_specifier: string option;
  (* Above this threshold of files to check, the remote type checking worker will not use Eden *)
  so_remote_worker_eden_checkout_threshold: int;
  (* Enables the reverse naming table to fall back to SQLite for queries. *)
  so_naming_sqlite_path: string option;
  (* Flag to disallow subtyping of untyped arrays and tuples (both ways) *)
  tco_disallow_array_as_tuple: bool;
  (* Namespace aliasing map *)
  po_auto_namespace_map: (string * string) list;
  (* Are we emitting bytecode? *)
  po_codegen: bool;
  (* Flag for disabling functions in HHI files with the __PHPStdLib attribute *)
  po_deregister_php_stdlib: bool;
  (* Flag to disable the backticks execution operator *)
  po_disallow_execution_operator: bool;
  (* Flag to disallow `require`, `require_once` etc as toplevel statements *)
  po_disallow_toplevel_requires: bool;
  (* Flag to disable PHP's non-top-level declarations *)
  po_disable_nontoplevel_declarations: bool;
  (* Flag to disable PHP's static closures *)
  po_disable_static_closures: bool;
  (* Flag to disable PHP's __halt_compiler() function *)
  po_disable_halt_compiler: bool;
  (* Flag to enable PHP's `goto` operator *)
  po_allow_goto: bool;
  (* Print types of size bigger than 1000 after performing a type union. *)
  tco_log_inference_constraints: bool;
  (*
   * Flag to disallow any lambda that has to be checked using the legacy
   * per-use technique
   *)
  tco_disallow_ambiguous_lambda: bool;
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
   * Flag to disable enforcement of requirements for reactive Hack.
   *
   * Currently defaults to true as Reactive Hack is experimental and
   * undocumented; the HSL is compatible with it, but we don't want to
   * raise errors that can't be fully understood without knowledge of
   * undocumented features.
   *)
  tco_unsafe_rx: bool;
  (*
   * Flag to disable unsetting on varray / varray_or_darray.
   *)
  tco_disallow_unset_on_varray: bool;
  (*
   * When enabled, mismatches between the types of the scrutinee and case value
   * of a switch expression are reported as type errors.
   *)
  tco_disallow_scrutinee_case_value_type_mismatch: bool;
  (*
   * Constraint-based type inference for lambdas
   *)
  tco_new_inference_lambda: bool;
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
  (* Error codes for which we do not allow HH_FIXMEs *)
  ignored_fixme_codes: ISet.t;
  (*
   * Regular expression controlling which HH_FIXMEs are to be ignored by the
   * parser.
   *)
  ignored_fixme_regex: string option;
  (* Initial hh_log_level settings *)
  log_levels: int SMap.t;
  (* Flag to disable using lvals as expressions. *)
  po_disable_lval_as_an_expression: bool;
  (* Flag to typecheck xhp code *)
  tco_typecheck_xhp_cvars: bool;
  (* Flag to ignore the string in vec<string>[...] *)
  tco_ignore_collection_expr_type_arguments: bool;
  (* Look up class members lazily from shallow declarations instead of eagerly
     computing folded declarations representing the entire class type. *)
  tco_shallow_class_decl: bool;
  (* Use Rust parser errors *)
  po_rust_parser_errors: bool;
  (* The threshold (in seconds) that determines whether a file's type checking time
      should be logged. It's only in effect if we're profiling type checking to begin
      with. To profile, pass --profile-log to hh_server. *)
  profile_type_check_duration_threshold: float;
  (* Enables like type hints *)
  tco_like_type_hints: bool;
  (* Enables like casts *)
  tco_like_casts: bool;
  (* This tells the type checker to rewrite unenforceable as like types
     i.e. vec<string> => vec<~string> *)
  tco_pessimize_types: bool;
  (* A simpler form of pessimization, only wraps the outermost type in like
   * if the type is not enforceable *)
  tco_simple_pessimize: float;
  (* Enables coercion from dynamic to enforceable types
     i.e. dynamic ~> int *)
  tco_coercion_from_dynamic: bool;
  (* Enables coercion from union types *)
  tco_coercion_from_union: bool;
  (* Enables complex coercion interactions that involve like types *)
  tco_complex_coercion: bool;
  (* Treat partially abstract typeconsts like concrete typeconsts, disable overriding type *)
  tco_disable_partially_abstract_typeconsts: bool;
  (* Set of codes to be treated as if they were in strict mode files *)
  error_codes_treated_strictly: ISet.t;
  (* static check xhp required attribute *)
  tco_check_xhp_attribute: bool;
  (*
   * Flag to produce an error whenever the TAST contains unresolved type variables
   *)
  tco_disallow_unresolved_type_variables: bool;
  (* Disallow using non-string, non-int types as array key type constraints. *)
  tco_disallow_invalid_arraykey_constraint: bool;
  (* Enable constants to have visibility modifiers *)
  po_enable_constant_visibility_modifiers: bool;
  (* Enable class-level where clauses, i.e.
     class base<T> where T = int {} *)
  po_enable_class_level_where_clauses: bool;
  (* Disable legacy soft typehint syntax (@int) and only allow the __Soft attribute. *)
  po_disable_legacy_soft_typehints: bool;
  (* Use shared_lru workers instead of MultiWorker workers *)
  tco_use_lru_workers: bool;
  (* Set of error codes disallowed in decl positions *)
  po_disallowed_decl_fixmes: ISet.t;
  (* Enable @ attribute syntax *)
  po_allow_new_attribute_syntax: bool;
  (* Trigger the use of inferred types in the tast for non annotated
    arguments and return types *)
  tco_infer_missing: InferMissing.t;
  (* Enable const static properties *)
  tco_const_static_props: bool;
  (* Disable <<...>> attribute syntax *)
  po_disable_legacy_attribute_syntax: bool;
  (* Allow <<__Const>> attribute *)
  tco_const_attribute: bool;
  (* Statically check default function arguments *)
  po_const_default_func_args: bool;
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
}
[@@deriving show]

val make :
  ?tco_safe_array:bool ->
  ?tco_safe_vector_array:bool ->
  ?po_deregister_php_stdlib:bool ->
  ?po_disallow_execution_operator:bool ->
  ?po_disallow_toplevel_requires:bool ->
  ?po_disable_nontoplevel_declarations:bool ->
  ?po_disable_static_closures:bool ->
  ?po_disable_halt_compiler:bool ->
  ?po_allow_goto:bool ->
  ?tco_log_inference_constraints:bool ->
  ?tco_experimental_features:SSet.t ->
  ?tco_migration_flags:SSet.t ->
  ?tco_dynamic_view:bool ->
  ?tco_defer_class_declaration_threshold:int ->
  ?tco_max_times_to_defer_type_checking:int ->
  ?tco_prefetch_deferred_files:bool ->
  ?tco_remote_type_check_threshold:int ->
  ?tco_remote_type_check:bool ->
  ?tco_remote_worker_key:string ->
  ?tco_remote_check_id:string ->
  ?tco_num_remote_workers:int ->
  ?so_remote_version_specifier:string ->
  ?so_remote_worker_eden_checkout_threshold:int ->
  ?so_naming_sqlite_path:string ->
  ?tco_disallow_array_as_tuple:bool ->
  ?po_auto_namespace_map:(string * string) list ->
  ?tco_disallow_ambiguous_lambda:bool ->
  ?tco_disallow_array_typehint:bool ->
  ?tco_disallow_array_literal:bool ->
  ?tco_language_feature_logging:bool ->
  ?tco_unsafe_rx:bool ->
  ?tco_disallow_unset_on_varray:bool ->
  ?tco_disallow_scrutinee_case_value_type_mismatch:bool ->
  ?tco_new_inference_lambda:bool ->
  ?tco_timeout:int ->
  ?tco_disallow_invalid_arraykey:bool ->
  ?tco_disallow_byref_dynamic_calls:bool ->
  ?tco_disallow_byref_calls:bool ->
  ?ignored_fixme_codes:ISet.t ->
  ?ignored_fixme_regex:string ->
  ?log_levels:int SMap.t ->
  ?po_disable_lval_as_an_expression:bool ->
  ?tco_typecheck_xhp_cvars:bool ->
  ?tco_ignore_collection_expr_type_arguments:bool ->
  ?tco_shallow_class_decl:bool ->
  ?po_rust_parser_errors:bool ->
  ?profile_type_check_duration_threshold:float ->
  ?tco_like_type_hints:bool ->
  ?tco_like_casts:bool ->
  ?tco_pessimize_types:bool ->
  ?tco_simple_pessimize:float ->
  ?tco_coercion_from_dynamic:bool ->
  ?tco_coercion_from_union:bool ->
  ?tco_complex_coercion:bool ->
  ?tco_disable_partially_abstract_typeconsts:bool ->
  ?error_codes_treated_strictly:ISet.t ->
  ?tco_check_xhp_attribute:bool ->
  ?tco_disallow_unresolved_type_variables:bool ->
  ?tco_disallow_invalid_arraykey_constraint:bool ->
  ?po_enable_constant_visibility_modifiers:bool ->
  ?po_enable_class_level_where_clauses:bool ->
  ?po_disable_legacy_soft_typehints:bool ->
  ?tco_use_lru_workers:bool ->
  ?po_disallowed_decl_fixmes:ISet.t ->
  ?po_allow_new_attribute_syntax:bool ->
  ?tco_infer_missing:InferMissing.t ->
  ?tco_const_static_props:bool ->
  ?po_disable_legacy_attribute_syntax:bool ->
  ?tco_const_attribute:bool ->
  ?po_const_default_func_args:bool ->
  ?po_disallow_silence:bool ->
  ?po_abstract_static_props:bool ->
  ?po_disable_unset_class_const:bool ->
  ?po_parser_errors_only:bool ->
  ?tco_check_attribute_locations:bool ->
  unit ->
  t

val tco_safe_array : t -> bool

val tco_safe_vector_array : t -> bool

val tco_experimental_feature_enabled : t -> SSet.elt -> bool

val tco_migration_flag_enabled : t -> SSet.elt -> bool

val tco_dynamic_view : t -> bool

val tco_defer_class_declaration_threshold : t -> int option

val tco_max_times_to_defer_type_checking : t -> int option

val tco_prefetch_deferred_files : t -> bool

val tco_remote_type_check_threshold : t -> int option

val tco_remote_type_check : t -> bool

val tco_remote_worker_key : t -> string option

val tco_remote_check_id : t -> string option

val tco_num_remote_workers : t -> int

val so_remote_version_specifier : t -> string option

val so_remote_worker_eden_checkout_threshold : t -> int

val so_naming_sqlite_path : t -> string option

val tco_disallow_array_as_tuple : t -> bool

val po_auto_namespace_map : t -> (string * string) list

val po_deregister_php_stdlib : t -> bool

val po_disallow_execution_operator : t -> bool

val po_disallow_toplevel_requires : t -> bool

val po_disable_nontoplevel_declarations : t -> bool

val po_disable_static_closures : t -> bool

val po_disable_halt_compiler : t -> bool

val po_allow_goto : t -> bool

val po_codegen : t -> bool

val tco_log_inference_constraints : t -> bool

val tco_disallow_ambiguous_lambda : t -> bool

val tco_disallow_array_typehint : t -> bool

val tco_disallow_array_literal : t -> bool

val tco_language_feature_logging : t -> bool

val tco_unsafe_rx : t -> bool

val tco_disallow_unset_on_varray : t -> bool

val tco_disallow_scrutinee_case_value_type_mismatch : t -> bool

val tco_new_inference_lambda : t -> bool

val tco_timeout : t -> int

val tco_disallow_invalid_arraykey : t -> bool

val tco_disallow_byref_dynamic_calls : t -> bool

val tco_disallow_byref_calls : t -> bool

val default : t

val tco_experimental_isarray : string

val tco_experimental_stronger_shape_idx_ret : string

val tco_experimental_generics_arity : string

val tco_experimental_forbid_nullable_cast : string

val tco_experimental_coroutines : string

val tco_experimental_disallow_static_memoized : string

val tco_experimental_no_trait_reuse : string

val tco_experimental_type_param_shadowing : string

val tco_experimental_trait_method_redeclarations : string

val tco_experimental_abstract_type_const_with_default : string

val tco_experimental_all : SSet.t

val tco_migration_flags_all : SSet.t

val ignored_fixme_codes : t -> ISet.t

val ignored_fixme_regex : t -> string option

val log_levels : t -> int SMap.t

val po_disable_lval_as_an_expression : t -> bool

val tco_typecheck_xhp_cvars : t -> bool

val tco_ignore_collection_expr_type_arguments : t -> bool

val tco_shallow_class_decl : t -> bool

val po_rust_parser_errors : t -> bool

val profile_type_check_duration_threshold : t -> float

val tco_like_type_hints : t -> bool

val tco_like_casts : t -> bool

val tco_pessimize_types : t -> bool

val tco_simple_pessimize : t -> float

val tco_coercion_from_dynamic : t -> bool

val tco_coercion_from_union : t -> bool

val tco_complex_coercion : t -> bool

val tco_disable_partially_abstract_typeconsts : t -> bool

val error_codes_treated_strictly : t -> ISet.t

val tco_check_xhp_attribute : t -> bool

val tco_disallow_unresolved_type_variables : t -> bool

val tco_disallow_invalid_arraykey_constraint : t -> bool

val po_enable_constant_visibility_modifiers : t -> bool

val po_enable_class_level_where_clauses : t -> bool

val po_disable_legacy_soft_typehints : t -> bool

val tco_use_lru_workers : t -> bool

val po_disallowed_decl_fixmes : t -> ISet.t

val po_allow_new_attribute_syntax : t -> bool

val tco_infer_missing : t -> InferMissing.t

val tco_const_static_props : t -> bool

val po_disable_legacy_attribute_syntax : t -> bool

val tco_const_attribute : t -> bool

val po_const_default_func_args : t -> bool

val po_disallow_silence : t -> bool

val po_abstract_static_props : t -> bool

val po_disable_unset_class_const : t -> bool

val set_infer_missing : t -> InferMissing.t -> t

val po_parser_errors_only : t -> bool

val tco_check_attribute_locations : t -> bool
