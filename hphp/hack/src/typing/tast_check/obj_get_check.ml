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
        Errors.nonsense_member_selection p "$this"
      | (_, _, Obj_get (_, (_, p, Lplaceholder _), _, _)) ->
        Errors.nonsense_member_selection p "$_"
      | (_, _, Obj_get ((ty, _, _), _, _, _))
        when Tast_env.is_sub_type_for_union
               env
               ty
               (MakeType.dynamic Reason.none)
             || Tast_env.is_sub_type_for_union
                  env
                  ty
                  (mk (Reason.none, Typing_defs.make_tany ()))
             || Tast_env.is_sub_type_for_union env ty (MakeType.err Reason.none)
        ->
        ()
      | (_, _, Obj_get (_, (_, p, Lvar _), _, _)) ->
        Errors.add_naming_error @@ Naming_error.Lvar_in_obj_get p
      | _ -> ()
  end
