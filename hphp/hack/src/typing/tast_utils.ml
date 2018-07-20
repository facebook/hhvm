(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Typing_defs

module Env = Tast_env

(** Return true if ty definitely does not contain null.  I.e., the
    return value false can mean two things: ty does contain null, e.g.,
    it is an option type; or we cannot tell, e.g., it is an opaque
    newtype. *)
let rec type_non_nullable env ty =
  let _, ty = Env.expand_type env ty in
  match ty with
  | _, (Tprim Nast.(Tint | Tbool | Tfloat | Tstring | Tresource | Tnum
                    | Tarraykey | Tnoreturn)
        | Tnonnull | Tfun _ | Ttuple _ | Tshape _ | Tanon _ | Tobject
        | Tclass _ | Tarraykind _ | Tabstract (AKenum _, _)) -> true
  | _, Tabstract (_, Some ty) when type_non_nullable env ty -> true
  | _, Tunresolved tyl when not (List.is_empty tyl) ->
    List.for_all tyl (type_non_nullable env)
  | _ -> false
