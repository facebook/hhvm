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

    method! at_expr env (_, pos, e) =
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
          let (_pos, id) = et.et_class in
          let err_opt =
            if
              List.exists allowed_expression_tree_visitors ~f:(fun s ->
                  String.equal s id)
            then
              None
            else
              Some
                Typing_error.(
                  expr_tree
                  @@ Primary.Expr_tree.Experimental_expression_trees pos)
          in
          let tenv = Tast_env.tast_env_as_typing_env env in
          Option.iter err_opt ~f:(Typing_error_utils.add_typing_error ~env:tenv)
      | _ -> ()
  end
