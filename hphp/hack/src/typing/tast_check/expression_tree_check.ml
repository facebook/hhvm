(*
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env ((pos, _), e) =
      match e with
      | ExpressionTree et ->
        let tcopt = Tast_env.get_tcopt env in
        (* Unstable features file attribute ignores all restrictions *)
        if TypecheckerOptions.expression_trees_enabled tcopt then
          ()
        else
          (* Otherwise, only allow those visitors in hhconfig *)
          let allowed_expression_tree_visitors =
            TypecheckerOptions.allowed_expression_tree_visitors tcopt
          in
          let (_pos, hint) = et.et_hint in
          begin
            match hint with
            | Happly ((_pos, id), _) ->
              if
                List.exists allowed_expression_tree_visitors ~f:(fun s ->
                    String.equal s id)
              then
                ()
              else
                Errors.experimental_expression_trees pos
            | _ -> Errors.experimental_expression_trees pos
          end
      | _ -> ()
  end
