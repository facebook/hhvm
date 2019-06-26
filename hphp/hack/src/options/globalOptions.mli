(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
 (**
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
 tco_safe_array : bool;

 (**
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
 tco_safe_vector_array : bool;

 (* Set of experimental features, in lowercase. *)
 tco_experimental_features : SSet.t;

 (* Set of opt-in migration behavior flags, in lowercase. *)
 tco_migration_flags : SSet.t;

 (* Whether to treat Tany as  Tdynamic *)
 tco_dynamic_view : bool;

 tco_defer_class_declaration_threshold : int option;

  (*
   * Flag to disallow subtyping of untyped arrays and tuples (both ways)
  *)
 tco_disallow_array_as_tuple : bool;

 (* Namespace aliasing map *)
 po_auto_namespace_map : (string * string) list;

 (* Are we emitting bytecode? *)
 po_codegen : bool;

 (* Flag for disabling functions in HHI files with the __PHPStdLib attribute *)
 po_deregister_php_stdlib : bool;

 (* Flag to disable the backticks execution operator *)
 po_disallow_execution_operator : bool;

 (* Flag to disable PHP's non-top-level declarations *)
 po_disable_nontoplevel_declarations : bool;

 (* Flag to disable PHP's static closures *)
 po_disable_static_closures : bool;

 (* Flag to enable PHP's `goto` operator *)
 po_allow_goto: bool;

 (** Print types of size bigger than 1000 after performing a type union. *)
 tco_log_inference_constraints : bool;

 (*
  * Flag to disallow any lambda that has to be checked using the legacy
  * per-use technique
  *)
 tco_disallow_ambiguous_lambda : bool;

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
 tco_language_feature_logging : bool;

 (*
  * Flag to disable enforcement of requirements for reactive Hack.
  *
  * Currently defaults to true as Reactive Hack is experimental and
  * undocumented; the HSL is compatible with it, but we don't want to
  * raise errors that can't be fully understood without knowledge of
  * undocumented features.
  *)
 tco_unsafe_rx : bool;

 (*
  * Flag to disallow implicit and expressionless returns in non-void functions.
  *)
 tco_disallow_implicit_returns_in_non_void_functions: bool;

 (*
  * Flag to disable unsetting on varray / varray_or_darray.
  *)
 tco_disallow_unset_on_varray : bool;

 (*
  * When enabled, mismatches between the types of the scrutinee and case value
  * of a switch expression are reported as type errors.
  *)
 tco_disallow_scrutinee_case_value_type_mismatch: bool;

 (*
  * Flag to disallow (string) casting non-Stringish values
  *)
 tco_disallow_stringish_magic : bool;

 (*
  * Constraint-based type inference for lambdas
  *)
 tco_new_inference_lambda : bool;

 (*
  * If non-zero, give up type checking a class or function after this many seconds
  *)
 tco_timeout : int;

 (*
  * Flag to disallow using values that get casted to array keys at runtime;
  * like bools, floats, or null; as array keys.
  *)
 tco_disallow_invalid_arraykey : bool;

 (* Turn off type refinement via the `instanceof` operator. *)
 tco_disable_instanceof_refinement : bool;

 (*
  * Produces an error if an arguments is passed by reference to dynamically
  * called function [e.g. $foo(&$bar)].
  *)
 tco_disallow_byref_dynamic_calls : bool;

 (* Make usage of the `instanceof` operator a parse error. *)
 po_disable_instanceof : bool;

 (* Error codes for which we do not allow HH_FIXMEs *)
 ignored_fixme_codes : ISet.t;

 (*
  * Regular expression controlling which HH_FIXMEs are to be ignored by the
  * parser.
  *)
 ignored_fixme_regex : string option;

 (* Initial hh_log_level settings *)
 log_levels : int SMap.t;

 (* Flag to disable using lvals as expressions. *)
 po_disable_lval_as_an_expression : bool;

 (* Flag to typecheck xhp code *)
 tco_typecheck_xhp_cvars : bool;

 (* Flag to ignore the string in vec<string>[...] *)
 tco_ignore_collection_expr_type_arguments : bool;

 (* Look up class members lazily from shallow declarations instead of eagerly
    computing folded declarations representing the entire class type. *)
 tco_shallow_class_decl : bool;

 (* Use Rust parser *)
 po_rust : bool;

 (* Enables deeper like types features *)
 tco_like_types : bool;

 (* This tells the type checker to rewrite unenforceable as like types
    i.e. vec<string> => vec<~string> *)
 tco_pessimize_types : bool;

 (* Enables coercion from dynamic and like types to enforceable types
    i.e. dynamic ~> int, ~string ~> string *)
 tco_coercion_from_dynamic : bool;

 (* Treat partially abstract typeconsts like concrete typeconsts, disable overriding type *)
 tco_disable_partially_abstract_typeconsts : bool;

 (* Set of codes to be treated as if they were in strict mode files *)
 error_codes_treated_strictly : ISet.t;

 (* static check xhp required attribute *)
 tco_check_xhp_attribute : bool;

 (*
  * Flag to produce an error whenever the TAST contains unresolved type variables
  *)
 tco_disallow_unresolved_type_variables : bool;

 (* Disallow using non-string, non-int types as array key type constraints. *)
 tco_disallow_invalid_arraykey_constraint : bool;

 (* This tells the type checker to try to invalidate files in HHVM's hhbc cache *)
 tico_invalidate_files : bool;

 (* When invalidating files, the type checker will use hh_server's calculated
    dependencies as opposed to file level dependencies *)
 tico_invalidate_smart : bool;

 (* Enable constants to have visibility modifiers *)
 po_enable_constant_visibility_modifiers : bool;
} [@@deriving show]

val make :
  ?tco_safe_array: bool ->
  ?tco_safe_vector_array: bool ->
  ?po_deregister_php_stdlib: bool ->
  ?po_disallow_execution_operator: bool ->
  ?po_disable_nontoplevel_declarations: bool ->
  ?po_disable_static_closures: bool ->
  ?po_allow_goto: bool ->
  ?tco_log_inference_constraints : bool ->
  ?tco_experimental_features: SSet.t ->
  ?tco_migration_flags: SSet.t ->
  ?tco_dynamic_view: bool ->
  ?tco_defer_class_declaration_threshold: int ->
  ?tco_disallow_array_as_tuple: bool ->
  ?po_auto_namespace_map: (string * string) list ->
  ?tco_disallow_ambiguous_lambda: bool ->
  ?tco_disallow_array_typehint: bool ->
  ?tco_disallow_array_literal: bool ->
  ?tco_language_feature_logging: bool ->
  ?tco_unsafe_rx: bool ->
  ?tco_disallow_implicit_returns_in_non_void_functions: bool ->
  ?tco_disallow_unset_on_varray: bool ->
  ?tco_disallow_scrutinee_case_value_type_mismatch: bool ->
  ?tco_disallow_stringish_magic: bool ->
  ?tco_new_inference_lambda: bool ->
  ?tco_timeout: int ->
  ?tco_disallow_invalid_arraykey: bool ->
  ?tco_disable_instanceof_refinement: bool ->
  ?tco_disallow_byref_dynamic_calls: bool ->
  ?po_disable_instanceof: bool ->
  ?ignored_fixme_codes: ISet.t ->
  ?ignored_fixme_regex: string ->
  ?log_levels: int SMap.t ->
  ?po_disable_lval_as_an_expression: bool ->
  ?tco_typecheck_xhp_cvars: bool ->
  ?tco_ignore_collection_expr_type_arguments: bool ->
  ?tco_shallow_class_decl: bool ->
  ?po_rust: bool ->
  ?tco_like_types: bool ->
  ?tco_pessimize_types: bool ->
  ?tco_coercion_from_dynamic: bool ->
  ?tco_disable_partially_abstract_typeconsts: bool ->
  ?error_codes_treated_strictly: ISet.t ->
  ?tco_check_xhp_attribute: bool ->
  ?tco_disallow_unresolved_type_variables: bool ->
  ?tco_disallow_invalid_arraykey_constraint: bool ->
  ?tico_invalidate_files: bool ->
  ?tico_invalidate_smart: bool ->
  ?po_enable_constant_visibility_modifiers: bool ->
  unit ->
  t

val tco_safe_array : t -> bool
val tco_safe_vector_array : t -> bool
val tco_experimental_feature_enabled : t -> SSet.elt -> bool
val tco_migration_flag_enabled : t -> SSet.elt -> bool
val tco_dynamic_view : t -> bool
val tco_defer_class_declaration_threshold : t -> int option
val tco_disallow_array_as_tuple : t -> bool
val po_auto_namespace_map : t -> (string * string) list
val po_deregister_php_stdlib : t -> bool
val po_disallow_execution_operator : t -> bool
val po_disable_nontoplevel_declarations : t -> bool
val po_disable_static_closures : t -> bool
val po_allow_goto : t -> bool
val po_codegen : t -> bool
val tco_log_inference_constraints : t -> bool
val tco_disallow_ambiguous_lambda : t -> bool
val tco_disallow_array_typehint : t -> bool
val tco_disallow_array_literal : t -> bool
val tco_language_feature_logging : t -> bool
val tco_unsafe_rx : t -> bool
val tco_disallow_implicit_returns_in_non_void_functions : t -> bool
val tco_disallow_unset_on_varray : t -> bool
val tco_disallow_scrutinee_case_value_type_mismatch : t -> bool
val tco_disallow_stringish_magic : t -> bool
val tco_new_inference_lambda : t -> bool
val tco_timeout : t -> int
val tco_disallow_invalid_arraykey : t -> bool
val tco_disable_instanceof_refinement : t -> bool
val tco_disallow_byref_dynamic_calls : t -> bool
val po_disable_instanceof : t -> bool
val default : t
val tco_experimental_instanceof : string
val tco_experimental_isarray : string
val tco_experimental_goto : string
val tco_experimental_disable_shape_and_tuple_arrays : string
val tco_experimental_stronger_shape_idx_ret : string
val tco_experimental_unresolved_fix : string
val tco_experimental_generics_arity : string
val tco_experimental_annotate_function_calls : string
val tco_experimental_forbid_nullable_cast : string
val tco_experimental_coroutines: string
val tco_experimental_disallow_static_memoized : string
val tco_experimental_no_trait_reuse : string
val tco_experimental_reified_generics : string
val tco_experimental_type_param_shadowing : string
val tco_experimental_trait_method_redeclarations : string
val tco_experimental_type_const_attributes : string
val tco_experimental_decl_linearization : string
val tco_experimental_track_subtype_prop : string
val tco_experimental_pocket_universes : string
val tco_experimental_abstract_type_const_with_default : string
val tco_experimental_all : SSet.t
val tco_migration_flags_all : SSet.t
val ignored_fixme_codes : t -> ISet.t
val ignored_fixme_regex : t -> string option
val log_levels : t -> int SMap.t
val po_disable_lval_as_an_expression : t -> bool
val setup_pocket_universes : t -> bool -> t
val tco_typecheck_xhp_cvars : t -> bool
val tco_ignore_collection_expr_type_arguments : t -> bool
val tco_shallow_class_decl : t -> bool
val po_rust : t -> bool
val tco_like_types : t -> bool
val tco_pessimize_types : t -> bool
val tco_coercion_from_dynamic : t -> bool
val tco_disable_partially_abstract_typeconsts : t -> bool
val error_codes_treated_strictly : t -> ISet.t
val tco_check_xhp_attribute : t -> bool
val tco_disallow_unresolved_type_variables : t -> bool
val tco_disallow_invalid_arraykey_constraint : t -> bool
val tico_invalidate_files : t -> bool
val tico_invalidate_smart : t -> bool
val po_enable_constant_visibility_modifiers : t -> bool
