(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
module SN = Naming_special_names

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_stmt _env stmt =
      match stmt with
      | (_, If (cond, then_block, _)) ->
        (match (cond, then_block) with
        (* `invariant(cond, msg)` would have been transformed into
           `if (!cond) { invariant_violation(msg) }` *)
        | ( (_, _, Unop (Ast_defs.Unot, (_, _, Package _))),
            [(pos, Expr (_, _, Call { func = (_, _, Id (_, name)); _ }))] )
          when String.equal name SN.AutoimportedFunctions.invariant_violation ->
          Errors.add_error
            Nast_check_error.(to_user_error @@ Package_expr_in_invariant pos)
        | _ -> ())
      | _ -> ()
  end
