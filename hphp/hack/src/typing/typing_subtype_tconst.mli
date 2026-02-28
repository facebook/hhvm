open Typing_defs
open Typing_defs_constraints
open Typing_env_types
module Env = Typing_env

(** [make_all_type_consts_equal env v ty] makes the types of
  all type constants T of type variable v equal to `ty`::T *)
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
  (env * Typing_error.t option * Type_expansions.cycle_reporter list) * locl_ty
