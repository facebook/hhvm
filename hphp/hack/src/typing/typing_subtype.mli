open Typing_defs
open Typing_env_types

(** Non-side-effecting test for subtypes.
    result = true implies ty1 <: ty2
    result = false implies NOT ty1 <: ty2 OR we don't know
*)
val is_sub_type : env -> locl_ty -> locl_ty -> bool

val is_dynamic_aware_sub_type : env -> locl_ty -> locl_ty -> bool

val is_sub_type_ignore_generic_params : env -> locl_ty -> locl_ty -> bool

val is_sub_type_for_union : env -> locl_ty -> locl_ty -> bool

(** Determines whether the types are definitely disjoint, or whether they might
    overlap. *)
val is_type_disjoint : env -> locl_ty -> locl_ty -> bool

val can_sub_type : env -> locl_ty -> locl_ty -> bool

(**
 * [sub_type env t u on_error] asserts that [t] is a subtype of [u],
 * adding constraints to [env.tvenv] that are necessary to ensure this, or
 * calling [on_error ?code msgl] with (optional) error code and a list of
 * (position, message) pairs if the assertion is unsatisfiable.
 *
 * Note that the [on_error] callback must prefix this list with a top-level
 * position and message identifying the primary source of the error (e.g.
 * an expression or statement).
 * If the optional [coerce] argument indicates whether subtyping should allow
 * coercion to or from dynamic. For coercion to dynamic, types that implement
 * dynamic are considered sub-types of dynamic. For coercion from dynamic,
 * dynamic is treated as a sub-type of all types.
 *)
val sub_type :
  env ->
  ?coerce:Typing_logic.coercion_direction option ->
  ?is_coeffect:bool ->
  locl_ty ->
  locl_ty ->
  Typing_error.Reasons_callback.t option ->
  env * Typing_error.t option

(**
 * As above, but with a simpler error handler that doesn't make use of the
 * code and message list provided by subtyping.
 *)
val sub_type_or_fail :
  env ->
  locl_ty ->
  locl_ty ->
  Typing_error.Error.t option ->
  env * Typing_error.t option

val sub_type_with_dynamic_as_bottom :
  env ->
  locl_ty ->
  locl_ty ->
  Typing_error.Reasons_callback.t option ->
  env * Typing_error.t option

val sub_type_i :
  env ->
  ?is_coeffect:bool ->
  internal_type ->
  internal_type ->
  Typing_error.Reasons_callback.t option ->
  env * Typing_error.t option

val add_constraint :
  env ->
  Ast_defs.constraint_kind ->
  locl_ty ->
  locl_ty ->
  Typing_error.Reasons_callback.t option ->
  env

val add_constraints :
  Pos.t -> env -> (locl_ty * Ast_defs.constraint_kind * locl_ty) list -> env

(** Hack to allow for circular dependencies between Ocaml modules. *)
val set_fun_refs : unit -> unit

val simplify_subtype_i :
  ?is_coeffect:bool ->
  env ->
  internal_type ->
  internal_type ->
  on_error:Typing_error.Reasons_callback.t option ->
  env * Typing_logic.subtype_prop

val subtype_funs :
  check_return:bool ->
  for_override:bool ->
  on_error:Typing_error.Reasons_callback.t option ->
  Reason.t ->
  locl_fun_type ->
  Reason.t ->
  locl_fun_type ->
  env ->
  env * Typing_error.t option

val can_traverse_to_iface : can_traverse -> locl_ty
