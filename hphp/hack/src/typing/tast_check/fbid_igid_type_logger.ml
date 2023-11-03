(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
module A = Aast_defs
module T = Typing_defs
module Tc = Typing_defs_core
module TM = Typing_make_type
module TR = Typing_reason
module SN = Naming_special_names

let rec find_ft ty =
  match T.get_node ty with
  | T.Tnewtype (name, _, ty) when String.equal name SN.Classes.cSupportDyn ->
    find_ft ty
  | T.Tfun ft -> Some ft
  | _ -> None

let check_args env params args pos =
  let is_int ty =
    let int_ty = TM.int TR.Rnone in
    Tast_env.is_sub_type env ty int_ty
  in
  let is_igid_of_meta_ent_instagram_user ty =
    let bound = TM.mixed TR.Rnone in
    let meta_ent_instagram_user =
      TM.class_type TR.Rnone "\\IMETAEntInstagramUser" []
    in
    let in_igid_range = TM.class_type TR.Rnone "\\InIGIDRange" [] in
    let base_igid =
      Tc.mk
        ( TR.Rnone,
          Tc.Tnewtype
            ("\\BaseIGID", [meta_ent_instagram_user; in_igid_range], bound) )
    in
    Tast_env.is_sub_type env ty base_igid
  in
  let is_fbid_of_meta_ent_instagram_user ty =
    let bound = TM.mixed TR.Rnone in
    let meta_ent_instagram_user =
      TM.class_type TR.Rnone "\\IMETAEntInstagramUser" []
    in
    let in_fbid_range = TM.class_type TR.Rnone "\\IN_FBID_RANGE" [] in
    let fbid_forbids_zero = TM.class_type TR.Rnone "\\FBID_FORBIDS_ZERO" [] in
    let base_fbid =
      Tc.mk
        ( TR.Rnone,
          Tc.Tnewtype
            ( "\\BaseFBID",
              [meta_ent_instagram_user; in_fbid_range; fbid_forbids_zero],
              bound ) )
    in
    Tast_env.is_sub_type env ty base_fbid
  in
  let rec loop p a =
    match (p, a) with
    | ([], _) -> ()
    | (_, []) -> ()
    | (param_ty :: param_tail, arg_ty :: arg_tail)
      when is_int param_ty
           && (is_igid_of_meta_ent_instagram_user arg_ty
              || is_fbid_of_meta_ent_instagram_user arg_ty) ->
      let json =
        Hh_json.(
          JSON_Object
            [
              ( "message",
                JSON_String
                  "FBID or IGID type erasure detected, is this an IGID or an FBID? How will you know?"
              );
              ("type", JSON_String "call");
              ("pos", Pos.json pos);
            ])
      in
      let () =
        Hh_logger.log "[FBID_IGID_type_logger] %s" (Hh_json.json_to_string json)
      in
      loop param_tail arg_tail
    | (_ :: param_tail, _ :: arg_tail) -> loop param_tail arg_tail
  in
  loop params args

let create_handler _ctx =
  object
    inherit Tast_visitor.handler_base

    method! at_Call env { func = (fx_ty, pos, _); args; _ } =
      let extract_param_types param : T.locl_ty = param.T.fp_type.T.et_type in
      let extract_arg_types (_, arg_exp) : T.locl_ty = Tast.get_type arg_exp in
      let check ft =
        let param_types = List.map ft.T.ft_params ~f:extract_param_types in
        let arg_types = List.map args ~f:extract_arg_types in
        check_args env param_types arg_types (pos |> Pos.to_relative_string)
      in
      Option.iter (find_ft fx_ty) ~f:check
  end
