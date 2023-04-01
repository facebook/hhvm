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

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | (_, p, Binop { bop = (Eqeqeq | Diff2) as bop; lhs; rhs }) ->
        Tast_env.assert_nontrivial p bop env (get_type lhs) (get_type rhs)
      | _ -> ()
  end
