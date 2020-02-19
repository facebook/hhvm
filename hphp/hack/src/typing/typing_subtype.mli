module Env = Typing_env
open Typing_defs
open Typing_env_types

type reactivity_extra_info = {
  method_info: (* method_name *) (string * (* is_static *) bool) option;
  class_ty: phase_ty option;
  parent_class_ty: phase_ty option;
}

module ConditionTypes : sig
  val try_get_class_for_condition_type :
    env ->
    decl_ty ->
    ((Ast_defs.pos * string) * Decl_provider.class_decl) option

  val try_get_method_from_condition_type :
    env -> decl_ty -> bool -> string -> class_elt option

  val localize_condition_type : env -> decl_ty -> locl_ty
end

(** Non-side-effecting test for subtypes.
    result = true implies ty1 <: ty2
    result = false implies NOT ty1 <: ty2 OR we don't know
*)
val is_sub_type : env -> locl_ty -> locl_ty -> bool

val is_sub_type_for_coercion : env -> locl_ty -> locl_ty -> bool

val is_sub_type_ignore_generic_params : env -> locl_ty -> locl_ty -> bool

val is_sub_type_for_union : env -> locl_ty -> locl_ty -> bool

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
 *)
val sub_type : env -> locl_ty -> locl_ty -> Errors.typing_error_callback -> env

(**
 * As above, but with a simpler error handler that doesn't make use of the
 * code and message list provided by subtyping.
 *)
val sub_type_or_fail : env -> locl_ty -> locl_ty -> (unit -> unit) -> env

val sub_type_with_dynamic_as_bottom :
  env -> locl_ty -> locl_ty -> Errors.typing_error_callback -> env

val sub_type_i :
  env -> internal_type -> internal_type -> Errors.typing_error_callback -> env

(** Check that the method with signature ft_sub can be used to override
(is a subtype of) method with signature ft_super. *)
val subtype_method :
  check_return:bool ->
  extra_info:reactivity_extra_info ->
  env ->
  Reason.t ->
  locl_fun_type ->
  Reason.t ->
  locl_fun_type ->
  Errors.typing_error_callback ->
  env

val subtype_reactivity :
  ?extra_info:reactivity_extra_info ->
  ?is_call_site:bool ->
  env ->
  Pos.t ->
  reactivity ->
  Pos.t ->
  reactivity ->
  Errors.typing_error_callback ->
  env

val add_constraint :
  Pos.t -> env -> Ast_defs.constraint_kind -> locl_ty -> locl_ty -> env

val add_constraints :
  Pos.t -> env -> (locl_ty * Ast_defs.constraint_kind * locl_ty) list -> env

(** Hack to allow for circular dependencies between Ocaml modules. *)
val set_fun_refs : unit -> unit

val simplify_subtype_i :
  env ->
  internal_type ->
  internal_type ->
  on_error:Errors.typing_error_callback ->
  env * Typing_logic.subtype_prop
