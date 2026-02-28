(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module SN = Naming_special_names

module Env = struct
  let consistent_ctor_level Naming_phase_env.{ consistent_ctor_level; _ } =
    consistent_ctor_level
end

let on_class_
    on_error (Aast.{ c_methods; c_user_attributes; c_kind; _ } as c) ~ctx =
  let err_opt =
    if Env.consistent_ctor_level ctx > 0 then
      let attr_pos_opt =
        Naming_attributes.mem_pos
          SN.UserAttributes.uaConsistentConstruct
          c_user_attributes
      in
      let ctor_opt =
        List.find c_methods ~f:(fun Aast.{ m_name = (_, nm); _ } ->
            if String.equal nm "__construct" then
              true
            else
              false)
      in
      match (attr_pos_opt, ctor_opt) with
      | (Some pos, None)
        when Ast_defs.is_c_trait c_kind || Env.consistent_ctor_level ctx > 1 ->
        if Option.is_none ctor_opt then
          Some
            (Naming_phase_error.naming
            @@ Naming_error.Explicit_consistent_constructor
                 { classish_kind = c_kind; pos })
        else
          None
      | _ -> None
    else
      None
  in
  Option.iter ~f:on_error err_opt;
  (ctx, Ok c)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      {
        id with
        on_ty_class_ = Some (fun elem ~ctx -> on_class_ on_error elem ~ctx);
      }
