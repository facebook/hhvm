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
    (env, (Aast.{ c_methods; c_user_attributes; c_kind; _ } as c), err_acc) =
  let err =
    if Env.consistent_ctor_level env > 0 then
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
        when Ast_defs.is_c_trait c_kind || Env.consistent_ctor_level env > 1 ->
        if Option.is_none ctor_opt then
          (Naming_phase_error.naming
          @@ Naming_error.Explicit_consistent_constructor
               { classish_kind = c_kind; pos })
          :: err_acc
        else
          err_acc
      | _ -> err_acc
    else
      err_acc
  in
  Ok (env, c, err)

let pass =
  Naming_phase_pass.(top_down { identity with on_class_ = Some on_class_ })
