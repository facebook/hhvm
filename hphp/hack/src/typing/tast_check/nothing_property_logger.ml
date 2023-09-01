(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module A = Aast_defs

let create_handler _ctx =
  object
    inherit Tast_visitor.handler_base

    method! at_class_ env A.{ c_vars; _ } =
      let print_if_nothing_hint A.{ cv_span; cv_type = (locl_ty, _); _ } =
        let tenv = Tast_env.tast_env_as_typing_env env in
        let (_, locl_ty, _) =
          Typing_tdef.force_expand_typedef
            ~ety_env:Typing_defs.empty_expand_env
            tenv
            locl_ty
        in
        if
          Tast_env.is_sub_type
            env
            locl_ty
            (Typing_make_type.nothing Typing_reason.Rnone)
        then
          let kind =
            match Typing_defs.get_node locl_ty with
            | Typing_defs.Tany _ -> "tany"
            | _ -> "nothing"
          in
          let pos = cv_span |> Pos.to_relative_string in
          let json =
            Hh_json.(
              JSON_Object [("pos", Pos.json pos); ("kind", JSON_String kind)])
          in
          Hh_logger.log
            "[Nothing_property_logger] %s"
            (Hh_json.json_to_string json)
      in
      List.iter ~f:print_if_nothing_hint c_vars
  end
