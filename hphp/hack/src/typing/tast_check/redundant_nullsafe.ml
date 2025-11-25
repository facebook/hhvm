(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Typing_defs
module Env = Tast_env
module Utils = Tast_utils

let warning_kind = Typing_warning.Redundant_nullsafe_operation

let error_codes = Typing_warning_utils.codes warning_kind

[@@@warning "-27"]

let handler ~as_lint =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | (_, pos, Obj_get ((ty, _, _), _, Nullsafe, _))
        when Utils.type_non_nullable env ty && not (Env.is_in_expr_tree env) ->
        let open Typing_warning.Redundant_nullsafe_operation in
        Env.add_warning
          env
          ( pos,
            Redundant_nullsafe_operation,
            {
              kind = Redundant_nullsafe_member_select;
              ty = Env.print_ty env ty;
            } )
      | (_, pos, Obj_get ((ty, _, _), _, Nullsafe, _))
        when not (Env.is_in_expr_tree env) ->
        let (_, ty) = Tast_env.expand_type env ty in
        begin
          match get_node ty with
          | Tprim Tnull ->
            let open Typing_warning.Redundant_nullsafe_operation in
            Env.add_warning
              env
              ( pos,
                Redundant_nullsafe_operation,
                {
                  kind = Nullsafe_member_select_on_null;
                  ty = Env.print_ty env ty;
                } )
          | _ -> ()
        end
      | _ -> ()
  end
