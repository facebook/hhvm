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

let is_int ty = Typing_defs.is_prim Aast.Tint ty

let is_igid_of_meta_ent_instagram_user env ty =
  let bound = TM.mixed TR.none in
  let meta_ent_instagram_user =
    TM.class_type TR.none "\\IMETAEntInstagramUser" []
  in
  let in_igid_range = TM.class_type TR.none "\\InIGIDRange" [] in
  let base_igid =
    T.mk
      ( TR.none,
        T.Tnewtype
          ("\\BaseIGID", [meta_ent_instagram_user; in_igid_range], bound) )
  in
  Tast_env.is_sub_type env ty base_igid

let is_fbid_of_meta_ent_instagram_user env ty =
  let bound = TM.mixed TR.none in
  let meta_ent_instagram_user =
    TM.class_type TR.none "\\IMETAEntInstagramUser" []
  in
  let in_fbid_range = TM.class_type TR.none "\\IN_FBID_RANGE" [] in
  let fbid_forbids_zero = TM.class_type TR.none "\\FBID_FORBIDS_ZERO" [] in
  let base_fbid =
    T.mk
      ( TR.none,
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
  let bound = TM.mixed TR.none in
  let media_ent = TM.class_type TR.none "\\IGEntMedia" [] in
  let in_oid_range = TM.class_type TR.none "\\IN_OID_RANGE" [] in
  let fbid_forbids_zero = TM.class_type TR.none "\\FBID_FORBIDS_ZERO" [] in
  let base_media_id =
    T.mk
      ( TR.none,
        T.Tnewtype
          ("\\BaseFBID", [media_ent; in_oid_range; fbid_forbids_zero], bound) )
  in
  Tast_env.is_sub_type env ty base_media_id

let log_json env kind message arg_ty param_ty param_name pos =
  let json =
    `Assoc
      [
        ("message", `String message);
        ("kind", `String kind);
        ("arg_type", Tast_env.ty_to_json env arg_ty);
        ( "full_param_text",
          `String
            (Printf.sprintf
               "%s %s"
               (name param_ty)
               (Option.value param_name ~default:"None")) );
        ("param_type", Tast_env.ty_to_json env param_ty);
        ("pos", Pos.json pos);
      ]
  in
  (* unsorted: emitted in argument order *)
  Hh_logger.log "[FBID_IGID_type_logger] %s" (Yojson.Safe.to_string json)

let check_one_arg
    (env : Tast_env.env) (fp : T.locl_ty T.fun_param) (arg_ty : T.locl_ty) pos =
  let Equal = Tast_env.eq_typing_env in
  if Typing_utils.is_dynamic env arg_ty then
    (* Skip dynamic types as they're likely false positives *)
    ()
  else
    let param_name = fp.T.fp_name in
    let param_ty = fp.T.fp_type in
    if is_int param_ty && is_igid_of_meta_ent_instagram_user env arg_ty then
      log_json
        env
        "IGID"
        "IGID type erasure detected, is this an IGID or an FBID? How will you know?"
        arg_ty
        param_ty
        param_name
        pos
    else if is_int param_ty && is_fbid_of_meta_ent_instagram_user env arg_ty
    then
      log_json
        env
        "FBID"
        "FBID type erasure detected, is this an IGID or an FBID? How will you know?"
        arg_ty
        param_ty
        param_name
        pos
    else if is_int param_ty && is_fbid_or_igid_of_media_id env arg_ty then
      log_json
        env
        "MediaID"
        "MediaID type weakening detected"
        arg_ty
        param_ty
        param_name
        pos
    else
      ()

let check_args (env : Tast_env.env) (fty : T.locl_ty T.fun_type) args pos =
  let named_variadic = T.ft_named_variadic_param fty in
  let (named_params, positional_params) =
    T.ft_params_without_named_variadic fty
    |> List.partition_tf ~f:T.get_fp_is_named
  in
  let named_params =
    List.fold named_params ~init:S_map.empty ~f:(fun params param ->
        match T.Named_params.name_of_named_param param with
        | Some name -> S_map.add name param params
        | None -> params)
  in
  let (positional_params, positional_variadic) =
    if T.get_ft_variadic fty then
      (List.drop_last_exn positional_params, List.last positional_params)
    else
      (positional_params, None)
  in
  let rec loop args positional_params named_params =
    match args with
    | [] -> ()
    | arg :: rest ->
      let arg_ty = Tast.get_type (Aast_utils.arg_to_expr arg) in
      (match T.Named_params.name_of_arg arg with
      | None ->
        (* Positional argument: the next positional formal, else the
           positional variadic. Anything unmatched is an arity error
           reported elsewhere. *)
        (match positional_params with
        | fp :: rest_pos ->
          check_one_arg env fp arg_ty pos;
          loop rest rest_pos named_params
        | [] ->
          (match positional_variadic with
          | Some fp ->
            check_one_arg env fp arg_ty pos;
            loop rest [] named_params
          | None -> loop rest [] named_params))
      | Some name ->
        (* Named argument: the formal with that name, else the named
           variadic, which absorbs any number of them. Anything
           unmatched is an unexpected-name error reported elsewhere. *)
        (match S_map.find_opt name named_params with
        | Some fp ->
          check_one_arg env fp arg_ty pos;
          loop rest positional_params (S_map.remove name named_params)
        | None ->
          (match named_variadic with
          | Some fp ->
            check_one_arg env fp arg_ty pos;
            loop rest positional_params named_params
          | None -> loop rest positional_params named_params)))
  in
  loop args positional_params named_params

let create_handler _ctx =
  object
    inherit Tast_visitor.handler_base

    method! at_Call env { func = (fx_ty, pos, _); args; _ } =
      let check (_, ft) =
        check_args env ft args (pos |> Pos.to_relative_string)
      in
      Option.iter (Tast_env.get_underlying_function_type env fx_ty) ~f:check
  end
