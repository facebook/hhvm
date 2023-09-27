(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(*****************************************************************************)
(* Gets rid of all the type variables,
 * this is only useful when declaring class constants.
 * The thing is, we don't want any type variable left in
 * type declarations, (it would force us to maintain a global
 * substitution, which would be way too big).
 *)
(*****************************************************************************)

let visitor =
  object
    inherit Type_mapper.tvar_expanding_type_mapper
  end

(*****************************************************************************)
(* External API *)
(*****************************************************************************)

let fully_expand env ty = snd (visitor#on_type (Type_mapper.fresh_env env) ty)

let fully_expand_i env ty =
  Typing_defs.(
    match ty with
    | ConstraintType _ -> ty
    | LoclType ty -> LoclType (fully_expand env ty))
