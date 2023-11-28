(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Aast
open Typing_defs
open Utils
module Cls = Decl_provider.Class
module Env = Tast_env

let collect_attrs_from_ty_sid ?(include_optional = false) env add bag sid =
  match Env.get_class env sid with
  | Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable ->
    bag
  | Decl_entry.Found c ->
    let should_collect ce =
      match Typing_defs.get_ce_xhp_attr ce with
      | Some { Xhp_attribute.xa_has_default; xa_tag = None; _ }
        when include_optional ->
        xa_has_default
      | Some { Xhp_attribute.xa_tag = Some Xhp_attribute.Required; _ } -> true
      | _ -> false
    in
    let required_attrs =
      List.filter (Cls.props c) ~f:(compose should_collect snd)
    in
    List.fold required_attrs ~init:bag ~f:(fun s (n, elt) ->
        add (n, elt.ce_origin) s)

let rec collect_attrs_from_ty env set ty =
  let (_, ty) = Env.expand_type env ty in
  let tenv = Tast_env.tast_env_as_typing_env env in
  let ty = Typing_utils.strip_dynamic tenv ty in
  match get_node ty with
  | Tunion tys ->
    (* Filter out dynamic, as we conservatively assume that anything dynamic
     * has the appropriate required attrs *)
    let tys =
      List.filter tys ~f:(fun ty -> not (Typing_utils.is_dynamic tenv ty))
    in
    begin
      match tys with
      | [] -> set
      | ty :: tys ->
        let collect = collect_attrs_from_ty env SSet.empty in
        List.fold (List.map tys ~f:collect) ~init:(collect ty) ~f:SSet.inter
    end
  | Tclass ((_, sid), _, _) ->
    collect_attrs_from_ty_sid
      ~include_optional:true
      env
      (compose SSet.add fst)
      set
      sid
  | _ -> set

let collect_attrs env attrs =
  let collect_attr set attr =
    match attr with
    | Xhp_simple { xs_name = (_, n); _ } -> SSet.add (":" ^ n) set
    | Xhp_spread (ty, _, _) -> collect_attrs_from_ty env set ty
  in
  List.fold attrs ~init:SSet.empty ~f:collect_attr

let check_attrs pos env sid attrs =
  let collect_with_ty =
    collect_attrs_from_ty_sid env (fun (n, c) -> SMap.add n c)
  in
  let required_attrs = collect_with_ty SMap.empty sid in
  let supplied_attrs = collect_attrs env attrs in
  let missing_attrs = SSet.fold SMap.remove supplied_attrs required_attrs in
  if SMap.is_empty missing_attrs then
    ()
  else
    SMap.iter
      (fun attr origin_sid ->
        let attr_name = Utils.strip_xhp_ns attr in
        let ty_reason_msg =
          lazy
            (match Env.get_class env origin_sid with
            | Decl_entry.DoesNotExist
            | Decl_entry.NotYetAvailable ->
              []
            | Decl_entry.Found ty ->
              let pos =
                match Cls.get_prop ty attr with
                | Some attr_decl -> Lazy.force attr_decl.ce_pos
                | None -> Cls.pos ty
              in
              Reason.to_string
                ("The attribute `" ^ attr_name ^ "` is declared here.")
                (Reason.Rwitness_from_decl pos))
        in
        Typing_error_utils.add_typing_error
          ~env:(Tast_env.tast_env_as_typing_env env)
          Typing_error.(
            xhp
            @@ Primary.Xhp.Missing_xhp_required_attr
                 { pos; attr = attr_name; ty_reason_msg }))
      missing_attrs

let create_handler ctx =
  let handler =
    object
      inherit Tast_visitor.handler_base

      method! at_expr env =
        function
        | (_, pos, Xml ((_, sid), attrs, _)) -> check_attrs pos env sid attrs
        | _ -> ()
    end
  in
  if TypecheckerOptions.check_xhp_attribute (Provider_context.get_tcopt ctx)
  then
    Some handler
  else
    None
