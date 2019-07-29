module Env = Typing_env

open Typing_defs

type reactivity_extra_info = {
  method_info: ((* method_name *) string * (* is_static *) bool) option;
  class_ty: phase_ty option;
  parent_class_ty: phase_ty option
}

module ConditionTypes : sig
  val try_get_class_for_condition_type :
    Env.env ->
    decl ty ->
    ((Ast_defs.pos * string) * Decl_provider.class_decl) option

  val try_get_method_from_condition_type :
    Env.env ->
    decl ty ->
    bool ->
    string ->
    class_elt option

  val localize_condition_type :
    Env.env ->
    decl ty ->
    locl ty
end

val is_sub_type_LEGACY_DEPRECATED :
  Env.env ->
  locl ty ->
  locl ty ->
  bool

(** Non-side-effecting test for subtypes.
    result = true implies ty1 <: ty2
    result = false implies NOT ty1 <: ty2 OR we don't know
*)
val is_sub_type :
  Env.env ->
  locl ty ->
  locl ty ->
  bool

val is_sub_type_ignore_generic_params :
  Env.env ->
  locl ty ->
  locl ty ->
  bool

val is_sub_type_for_union :
  Env.env ->
  locl ty ->
  locl ty ->
  bool

val can_sub_type :
  Env.env ->
  locl ty ->
  locl ty ->
  bool
(**
  Checks that ty_sub is a subtype of ty_super, and returns an env.

  E.g.
    sub_type env ?int int   => env
    sub_type env int alpha  => env where alpha==int
    sub_type env ?int alpha => env where alpha==?int
    sub_type env int string => error
 *)
val sub_type :
  Env.env ->
  locl ty ->
  locl ty ->
  Errors.typing_error_callback ->
  Env.env

(** Check that the method with signature ft_sub can be used to override
(is a subtype of) method with signature ft_super. *)
val subtype_method :
  check_return:bool ->
  extra_info:reactivity_extra_info ->
  Env.env ->
  Reason.t ->
  decl fun_type ->
  Reason.t ->
  decl fun_type ->
  Errors.typing_error_callback ->
  Env.env

val subtype_reactivity :
  ?extra_info:reactivity_extra_info ->
  ?is_call_site:bool ->
  Env.env ->
  reactivity ->
  reactivity ->
  bool

val add_constraint :
  Pos.t ->
  Env.env ->
  Ast_defs.constraint_kind ->
  locl ty ->
  locl ty ->
  Env.env

val log_prop :
  Env.env ->
  unit
