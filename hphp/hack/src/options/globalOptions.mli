(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
 (* When we encounter an unknown class|function|constant name outside
  * of strict mode, is that an error? *)
 tco_assume_php : bool;

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

 (* List of <<UserAttribute>> names expected in the codebase *)
 tco_user_attrs : SSet.t option;

 (* Set of experimental features, in lowercase. *)
 tco_experimental_features : SSet.t;

 (* Set of opt-in migration behavior flags, in lowercase. *)
 tco_migration_flags : SSet.t;

 (* Whether to treat Tany as  Tdynamic *)
 tco_dynamic_view : bool;

  (*
   * Flag to disallow subtyping of untyped arrays and tuples (both ways)
  *)
 tco_disallow_array_as_tuple : bool;

 (* Namespace aliasing map *)
 po_auto_namespace_map : (string * string) list;

 (* Should we auto import into the HH namespace? *)
 po_enable_hh_syntax_for_hhvm : bool;

 (* Flag for disabling functions in HHI files with the __PHPStdLib attribute *)
 po_deregister_php_stdlib : bool;

 (* Flag to signal whether parsing via FFP or legacy parser *)
 po_use_full_fidelity : bool;

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
  * Flag to disallow returning references from functions
  *)
 tco_disallow_return_by_ref: bool;

 (*
  * Flag to disallow binding array cells by reference as function arguments
  *)
 tco_disallow_array_cell_pass_by_ref: bool;

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

 (* Error codes for which we do not allow HH_FIXMEs *)
 ignored_fixme_codes : ISet.t;

 (* What version of Hack the current codebase was designed for *)
 forward_compatibility_level : ForwardCompatibilityLevel.t;
} [@@deriving show]
val make :
  tco_assume_php: bool ->
  tco_safe_array: bool ->
  tco_safe_vector_array: bool ->
  po_deregister_php_stdlib: bool ->
  po_use_full_fidelity: bool ->
  tco_user_attrs: SSet.t option ->
  tco_experimental_features: SSet.t ->
  tco_migration_flags: SSet.t ->
  tco_dynamic_view: bool ->
  tco_disallow_array_as_tuple: bool ->
  po_auto_namespace_map: (string * string) list ->
  tco_disallow_ambiguous_lambda: bool ->
  tco_disallow_array_typehint: bool ->
  tco_disallow_array_literal: bool ->
  tco_disallow_return_by_ref: bool ->
  tco_disallow_array_cell_pass_by_ref: bool ->
  tco_language_feature_logging: bool ->
  tco_unsafe_rx: bool ->
  tco_disallow_implicit_returns_in_non_void_functions: bool ->
  ignored_fixme_codes: ISet.t ->
  forward_compatibility_level: ForwardCompatibilityLevel.t ->
  t
val tco_assume_php : t -> bool
val tco_safe_array : t -> bool
val tco_safe_vector_array : t -> bool
val tco_user_attrs : t -> SSet.t option
val tco_experimental_feature_enabled : t -> SSet.elt -> bool
val tco_migration_flag_enabled : t -> SSet.elt -> bool
val tco_dynamic_view : t -> bool
val tco_disallow_array_as_tuple : t -> bool
val tco_allowed_attribute : t -> SSet.elt -> bool
val po_auto_namespace_map : t -> (string * string) list
val po_deregister_php_stdlib : t -> bool
val po_use_full_fidelity : t -> bool
val po_enable_hh_syntax_for_hhvm : t -> bool
val tco_disallow_ambiguous_lambda : t -> bool
val tco_disallow_array_typehint : t -> bool
val tco_disallow_array_literal : t -> bool
val tco_disallow_return_by_ref : t -> bool
val tco_disallow_array_cell_pass_by_ref : t -> bool
val tco_language_feature_logging : t -> bool
val tco_unsafe_rx : t -> bool
val tco_disallow_implicit_returns_in_non_void_functions : t -> bool
val default : t
val make_permissive : t -> t
val tco_experimental_instanceof : string
val tco_experimental_isarray : string
val tco_experimental_goto : string
val tco_experimental_tconst_on_generics : string
val tco_experimental_disable_shape_and_tuple_arrays : string
val tco_experimental_stronger_shape_idx_ret : string
val tco_experimental_unresolved_fix : string
val tco_experimental_generics_arity : string
val tco_experimental_annotate_function_calls : string
val tco_experimental_forbid_nullable_cast : string
val tco_experimental_coroutines: string
val tco_experimental_disallow_static_memoized : string
val tco_experimental_disable_optional_and_unknown_shape_fields : string
val tco_experimental_no_trait_reuse : string
val tco_experimental_void_is_type_of_null : string
val tco_experimental_null_coalesce_assignment : string
val tco_experimental_reified_generics : string
val tco_experimental_re_prefixed_strings : string
val tco_experimental_all : SSet.t
val tco_migration_flags_all : SSet.t
val ignored_fixme_codes : t -> ISet.t
val forward_compatibility_level : t -> ForwardCompatibilityLevel.t
