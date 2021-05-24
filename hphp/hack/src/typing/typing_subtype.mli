open Typing_defs
open Typing_env_types

(** Non-side-effecting test for subtypes.
    result = true implies ty1 <: ty2
    result = false implies NOT ty1 <: ty2 OR we don't know
*)
val is_sub_type : env -> locl_ty -> locl_ty -> bool

val is_sub_type_for_coercion : env -> locl_ty -> locl_ty -> bool

val is_sub_type_ignore_generic_params : env -> locl_ty -> locl_ty -> bool

(** If the optional [coerce] argument indicates whether subtyping should allow
 * coercion to or from dynamic. For coercion to dynamic, types that implement
 * dynamic are considered sub-types of dynamic. For coercion from dynamic,
 * dynamic is treated as a sub-type of all types.
*)
val is_sub_type_for_union :
  env ->
  ?coerce:Typing_logic.coercion_direction option ->
  locl_ty ->
  locl_ty ->
  bool

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
  Errors.error_from_reasons_callback ->
  env

(**
 * As above, but with a simpler error handler that doesn't make use of the
 * code and message list provided by subtyping.
 *)
val sub_type_or_fail : env -> locl_ty -> locl_ty -> (unit -> unit) -> env

(** As above but with a `result` type indicating if subtyping failed *)
val sub_type_or_fail_res :
  env -> locl_ty -> locl_ty -> (unit -> unit) -> (env, env) result

(**
 * As above but only return the modified environment as [Ok] when the assertion
 * is satisfiable and the original environment as [Err] otherwise.
*)
val sub_type_res :
  env ->
  ?coerce:Typing_logic.coercion_direction option ->
  locl_ty ->
  locl_ty ->
  Errors.error_from_reasons_callback ->
  (env, env) result

val sub_type_with_dynamic_as_bottom :
  env -> locl_ty -> locl_ty -> Errors.error_from_reasons_callback -> env

val sub_type_with_dynamic_as_bottom_res :
  env ->
  locl_ty ->
  locl_ty ->
  Errors.error_from_reasons_callback ->
  (env, env) result

val sub_type_i :
  env ->
  ?is_coeffect:bool ->
  internal_type ->
  internal_type ->
  Errors.error_from_reasons_callback ->
  env

val sub_type_i_res :
  env ->
  internal_type ->
  internal_type ->
  Errors.error_from_reasons_callback ->
  (env, env) result

val add_constraint :
  env ->
  Ast_defs.constraint_kind ->
  locl_ty ->
  locl_ty ->
  Errors.error_from_reasons_callback ->
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
  on_error:Errors.error_from_reasons_callback ->
  env * Typing_logic.subtype_prop

val subtype_funs :
  check_return:bool ->
  on_error:Errors.error_from_reasons_callback ->
  Reason.t ->
  locl_fun_type ->
  Reason.t ->
  locl_fun_type ->
  env ->
  env

(* Given a subtype proposition, resolve conjunctions of subtype assertions
 * of the form #v <: t or t <: #v by adding bounds to #v in env. Close env
 * wrt transitivity i.e. if t <: #v and #v <: u then resolve t <: u which
 * may in turn produce more bounds in env.
 * For disjunctions, arbitrarily pick the first disjunct that is not
 * unsatisfiable. If any unsatisfiable disjunct remains, return it.
 *)
val prop_to_env :
  env ->
  Typing_logic.subtype_prop ->
  Errors.error_from_reasons_callback ->
  env * Typing_logic.subtype_prop
