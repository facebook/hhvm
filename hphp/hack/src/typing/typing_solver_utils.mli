(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Typing_defs
open Typing_env_types

val filter_locl_types : Internal_type_set.t -> Typing_set.t

val remove_tyvar_from_lower_bound :
  env -> Ident.t -> internal_type -> env * internal_type

val remove_tyvar_from_lower_bounds :
  env -> Ident.t -> Internal_type_set.t -> env * Internal_type_set.t

val remove_tyvar_from_upper_bound :
  env -> Ident.t -> internal_type -> env * internal_type

val remove_tyvar_from_upper_bounds :
  env -> Ident.t -> Internal_type_set.t -> env * Internal_type_set.t

val remove_tyvar_from_bounds : env -> Ident.t -> env

val err_if_var_in_ty : env -> Ident.t -> locl_ty -> locl_ty
