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
    ((Ast.pos * string) * Typing_heap.Classes.t) option

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

val is_sub_type :
  Env.env ->
  locl ty ->
  locl ty ->
  bool

(** Non-side-effecting test for subtypes.
Result is
   result = Some true implies ty1 <: ty2
   result = Some false implies NOT ty1 <: ty2
   result = None, we don't know
*)
val is_sub_type_alt :
  Env.env ->
  no_top_bottom:bool ->
  locl ty ->
  locl ty ->
  bool option

(**
  Checks that ty_sub is a subtype of ty_super, and returns an env.

  E.g.
    sub_type env ?int int   => env
    sub_type env int alpha  => env where alpha==int
    sub_type env ?int alpha => env where alpha==?int
    sub_type env int string => error
 *)
val sub_type :
  ?error:(Env.env -> locl ty -> locl ty -> unit) option ->
  Env.env ->
  locl ty ->
  locl ty ->
  Env.env

(** Make a type a subtype of string. *)
val sub_string :
  ?allow_mixed:bool ->
  Pos.t ->
  Env.env ->
  locl ty ->
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
  Ast.constraint_kind ->
  locl ty ->
  locl ty ->
  Env.env

(* Force solve all remaining unsolved type variables *)
val solve_all_unsolved_tyvars :
  Env.env ->
  Env.env

val expand_type_and_solve :
  Env.env ->
  description_of_expected:string ->
  Pos.t ->
  locl ty ->
  Env.env * locl ty

val expand_type_and_narrow :
  Env.env ->
  description_of_expected:string ->
  (Env.env -> locl ty -> Env.env * locl ty option) ->
  Pos.t ->
  locl ty ->
  Env.env * locl ty

val close_tyvars_and_solve :
  Env.env ->
  Env.env

val log_prop :
  Env.env ->
  unit
