module Env = Typing_env

open Typing_defs

(** Non-side-effecting test for subtypes.
    result = true implies ty1 <: ty2
    result = false implies NOT ty1 <: ty2 OR we don't know
*)
val is_sub_type :
  Env.env ->
  locl ty ->
  locl ty ->
  bool

val push_option_out :
  Pos.t ->
  Env.env ->
  locl ty ->
  Env.env * locl ty

val non_null :
  Env.env ->
  Pos.t ->
  locl ty ->
  Env.env * locl ty

(* Force solve all remaining unsolved type variables *)
val solve_all_unsolved_tyvars :
  Env.env ->
  Errors.typing_error_callback ->
  Env.env

val expand_type_and_solve :
  Env.env ->
  description_of_expected:string ->
  Pos.t ->
  locl ty ->
  Errors.typing_error_callback ->
  Env.env * locl ty

val expand_type_and_narrow :
  Env.env ->
  ?default:locl ty ->
  description_of_expected:string ->
  (Env.env -> locl ty -> Env.env * locl ty option) ->
  Pos.t ->
  locl ty ->
  Errors.typing_error_callback ->
  Env.env * locl ty

val close_tyvars_and_solve :
  Env.env ->
  Errors.typing_error_callback ->
  Env.env
