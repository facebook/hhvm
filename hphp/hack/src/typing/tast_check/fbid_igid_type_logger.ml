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
module TM = Typing_make_type
module TR = Typing_reason
module SN = Naming_special_names

let name ty =
  match T.get_node ty with
  | T.Tnewtype (s, _, _) -> s
  | T.Tprim tp -> A.string_of_tprim tp
  | _ -> ""

let rec find_ft ty =
  match T.get_node ty with
  | T.Tnewtype (name, _, ty) when String.equal name SN.Classes.cSupportDyn ->
    find_ft ty
  | T.Tfun ft -> Some ft
  | _ -> None

let is_int ty = Typing_defs.is_prim Aast.Tint ty

let is_igid_of_meta_ent_instagram_user env ty =
  let bound = TM.mixed TR.Rnone in
  let meta_ent_instagram_user =
    TM.class_type TR.Rnone "\\IMETAEntInstagramUser" []
  in
  let in_igid_range = TM.class_type TR.Rnone "\\InIGIDRange" [] in
  let base_igid =
    T.mk
      ( TR.Rnone,
        T.Tnewtype
          ("\\BaseIGID", [meta_ent_instagram_user; in_igid_range], bound) )
  in
  Tast_env.is_sub_type env ty base_igid

let is_fbid_of_meta_ent_instagram_user env ty =
  let bound = TM.mixed TR.Rnone in
  let meta_ent_instagram_user =
    TM.class_type TR.Rnone "\\IMETAEntInstagramUser" []
  in
  let in_fbid_range = TM.class_type TR.Rnone "\\IN_FBID_RANGE" [] in
  let fbid_forbids_zero = TM.class_type TR.Rnone "\\FBID_FORBIDS_ZERO" [] in
  let base_fbid =
    T.mk
      ( TR.Rnone,
        T.Tnewtype
          ( "\\BaseFBID",
            [meta_ent_instagram_user; in_fbid_range; fbid_forbids_zero],
            bound ) )
  in
  Tast_env.is_sub_type env ty base_fbid

(* BaseFBID<
     IGEntMedia,
     IN_OID_RANGE,
     FBID_FORBIDS_ZERO
   > *)
let is_fbid_or_igid_of_media_id env ty =
  let bound = TM.mixed TR.Rnone in
  let media_ent = TM.class_type TR.Rnone "\\IGEntMedia" [] in
  let in_oid_range = TM.class_type TR.Rnone "\\IN_OID_RANGE" [] in
  let fbid_forbids_zero = TM.class_type TR.Rnone "\\FBID_FORBIDS_ZERO" [] in
  let base_media_id =
    T.mk
      ( TR.Rnone,
        T.Tnewtype
          ("\\BaseFBID", [media_ent; in_oid_range; fbid_forbids_zero], bound) )
  in
  Tast_env.is_sub_type env ty base_media_id

let log_json env kind message arg_ty param_ty param_name pos =
  let json =
    Hh_json.(
      JSON_Object
        [
          ("message", JSON_String message);
          ("kind", JSON_String kind);
          ("arg_type", Tast_env.ty_to_json env arg_ty);
          ( "full_param_text",
            JSON_String
              (Printf.sprintf
                 "%s %s"
                 (name param_ty)
                 (Option.value param_name ~default:"None")) );
          ("param_type", Tast_env.ty_to_json env param_ty);
          ("pos", Pos.json pos);
        ])
  in
  Hh_logger.log "[FBID_IGID_type_logger] %s" (Hh_json.json_to_string json)

let check_args env params args pos =
  let tenv = Tast_env.tast_env_as_typing_env env in
  let rec loop p a =
    match (p, a) with
    | ([], _) -> ()
    | (_, []) -> ()
    | (_ :: param_tail, arg_ty :: arg_tail)
      when Typing_utils.is_dynamic tenv arg_ty ->
      (* Skip dynamic types as they're likely false positives *)
      loop param_tail arg_tail
    | ((param_name, param_ty) :: param_tail, arg_ty :: arg_tail)
      when is_int param_ty && is_igid_of_meta_ent_instagram_user env arg_ty ->
      let () =
        log_json
          env
          "IGID"
          "IGID type erasure detected, is this an IGID or an FBID? How will you know?"
          arg_ty
          param_ty
          param_name
          pos
      in
      loop param_tail arg_tail
    | ((param_name, param_ty) :: param_tail, arg_ty :: arg_tail)
      when is_int param_ty && is_fbid_of_meta_ent_instagram_user env arg_ty ->
      let () =
        log_json
          env
          "FBID"
          "FBID type erasure detected, is this an IGID or an FBID? How will you know?"
          arg_ty
          param_ty
          param_name
          pos
      in
      loop param_tail arg_tail
    | ((param_name, param_ty) :: param_tail, arg_ty :: arg_tail)
      when is_int param_ty && is_fbid_or_igid_of_media_id env arg_ty ->
      let () =
        log_json
          env
          "MediaID"
          "MediaID type weakening detected"
          arg_ty
          param_ty
          param_name
          pos
      in
      loop param_tail arg_tail
    | (_ :: param_tail, _ :: arg_tail) -> loop param_tail arg_tail
  in
  loop params args

let create_handler _ctx =
  object
    inherit Tast_visitor.handler_base

    method! at_Call env { func = (fx_ty, pos, _); args; _ } =
      let extract_param_types param : T.locl_ty = param.T.fp_type in
      let extract_arg_types (_, arg_exp) : T.locl_ty = Tast.get_type arg_exp in
      let check ft =
        let param_names_and_types =
          List.map ft.T.ft_params ~f:(fun p ->
              (p.T.fp_name, extract_param_types p))
        in
        let arg_types = List.map args ~f:(fun a -> extract_arg_types a) in

        check_args
          env
          param_names_and_types
          arg_types
          (pos |> Pos.to_relative_string)
      in
      Option.iter (find_ft fx_ty) ~f:check
  end
