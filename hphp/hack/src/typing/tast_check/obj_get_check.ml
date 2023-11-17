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
module MakeType = Typing_make_type

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | (_, _, Obj_get (_, (_, p, This), _, _)) ->
        Typing_error_utils.add_typing_error
          ~env:(Tast_env.tast_env_as_typing_env env)
          Typing_error.(
            primary
            @@ Primary.Nonsense_member_selection { pos = p; kind = "$this" })
      | (_, _, Obj_get (_, (_, p, Lplaceholder _), _, _)) ->
        Typing_error_utils.add_typing_error
          ~env:(Tast_env.tast_env_as_typing_env env)
          Typing_error.(
            primary
            @@ Primary.Nonsense_member_selection { pos = p; kind = "$_" })
      | (_, _, Obj_get ((ty, _, _), _, _, _))
        when Tast_env.is_sub_type_for_union
               env
               ty
               (MakeType.dynamic Reason.none) ->
        (* TODO akenn: do we need to detect error tyvar too? *)
        ()
      | (_, _, Obj_get (_, (_, pos, Lvar (lvar_pos, lvar_lid)), _, _)) ->
        let lvar_name = Local_id.get_name lvar_lid in
        Errors.add_error
          Naming_error.(
            to_user_error @@ Lvar_in_obj_get { pos; lvar_pos; lvar_name })
      | _ -> ()
  end
