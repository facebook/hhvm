open Typing_defs
open Typing_env_types
module Env = Typing_env

(** For all type constant T of type variable, make its type equal to `ty`::T *)
val make_all_type_consts_equal :
  env ->
  Tvid.t ->
  internal_type ->
  on_error:Typing_error.Reasons_callback.t option ->
  as_tyvar_with_cnstr:bool ->
  env * Typing_error.t option

(** Get the type of a type constant of a type variable by looking it up in the
environment.
If that type constant is not present, make a fresh invariant
type variable and add it as the type of the type constant in the environment.
*)
val get_tyvar_type_const :
  env ->
  Tvid.t ->
  pos_id ->
  on_error:Typing_error.Reasons_callback.t option ->
  (env * Typing_error.t option) * locl_ty
