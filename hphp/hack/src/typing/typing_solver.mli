module Env = Typing_env

open Typing_defs
open Typing_env_types

(** Non-side-effecting test for subtypes.
    result = true implies ty1 <: ty2
    result = false implies NOT ty1 <: ty2 OR we don't know
*)
val is_sub_type :
  env ->
  locl ty ->
  locl ty ->
  bool

val push_option_out :
  Pos.t ->
  env ->
  locl ty ->
  env * locl ty

val non_null :
  env ->
  Pos.t ->
  locl ty ->
  env * locl ty

(* Force solve all remaining unsolved type variables *)
val solve_all_unsolved_tyvars :
  env ->
  Errors.typing_error_callback ->
  env

val expand_type_and_solve :
  env ->
  description_of_expected:string ->
  Pos.t ->
  locl ty ->
  Errors.typing_error_callback ->
  env * locl ty

val expand_type_and_narrow :
  env ->
  ?default:locl ty ->
  description_of_expected:string ->
  (env -> locl ty -> env * locl ty option) ->
  Pos.t ->
  locl ty ->
  Errors.typing_error_callback ->
  env * locl ty

val close_tyvars_and_solve :
  env ->
  Errors.typing_error_callback ->
  env
