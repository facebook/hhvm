open Typing_defs

module Env = Typing_env

(** For all type constant T of type variable, make its type equal to `ty`::T *)
val make_all_type_consts_equal:
  Env.env ->
  Ident.t ->
  locl ty ->
  as_tyvar_with_cnstr:bool ->
  Env.env

(** Get the type of a type constant of a type variable by looking it up in the
environment.
If that type constant is not present, make a fresh invariant
type variable and add it as the type of the type constant in the environment.
*)
val get_tyvar_type_const:
  Env.env ->
  Ident.t ->
  Nast.sid ->
  Env.env * locl ty
