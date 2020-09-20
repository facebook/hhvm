(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Gets rid of all the type variables,
 * this is only useful when declaring class constants.
 * The thing is, we don't want any type variable left in
 * type declarations, (it would force us to maintain a global
 * substitution, which would be way too big).
 *)
(*****************************************************************************)

val fully_expand :
  Typing_env_types.env -> Typing_defs.locl_ty -> Typing_defs.locl_ty

val fully_expand_i :
  Typing_env_types.env -> Typing_defs.internal_type -> Typing_defs.internal_type
