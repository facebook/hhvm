open Typing_defs
open Typing_env_types
module Env = Typing_env

val make_all_type_consts_equal :
  env -> Ident.t -> locl_ty -> as_tyvar_with_cnstr:bool -> env
(** For all type constant T of type variable, make its type equal to `ty`::T *)

val get_tyvar_type_const : env -> Ident.t -> Aast.sid -> env * locl_ty
(** Get the type of a type constant of a type variable by looking it up in the
environment.
If that type constant is not present, make a fresh invariant
type variable and add it as the type of the type constant in the environment.
*)
