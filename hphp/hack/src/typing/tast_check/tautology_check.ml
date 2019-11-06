(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

open Ast_defs
open Aast
open Tast

let trivial_equality_check
    p bop env (((_, ty1), _) as te1 : expr) (((_, ty2), _) as te2 : expr) =
  begin
    match (te1, te2) with
    | ((_, Null), ((_, ty), _))
    | (((_, ty), _), (_, Null)) ->
      Tast_env.assert_nullable p bop env ty
    | _ -> ()
  end;
  Tast_env.assert_nontrivial p bop env ty1 ty2

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | ((p, _), Binop (((Eqeqeq | Diff2) as bop), te1, te2)) ->
        trivial_equality_check p bop env te1 te2
      | _ -> ()
  end
