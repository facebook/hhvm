(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Decl_defs
open Nast

module Attrs = Attributes
module SN = Naming_special_names

let parent_init_prop = "parent::" ^ SN.Members.__construct

(* If we need to call parent::__construct, we treat it as if it were
 * a class variable that needs to be initialized. It's a bit hacky
 * but it works. The idea here is that if the parent needs to be
 * initialized, we add a phony class variable. *)
let add_parent_construct decl_env c props parent_hint =
  match parent_hint with
    | (_, Happly ((_, parent), _)) ->
      begin match Decl_env.get_class_dep decl_env parent with
        | Some class_ when class_.dc_need_init && c.c_constructor <> None ->
          SSet.add parent_init_prop props
        | _ -> props
      end
    | _ -> props

let parent decl_env c acc =
  if c.c_mode = FileInfo.Mdecl then acc
  else
    if c.c_kind = Ast.Ctrait
    then List.fold_left c.c_req_extends
      ~f:(add_parent_construct decl_env c) ~init:acc
    else match c.c_extends with
    | [] -> acc
    | parent_hint :: _ -> add_parent_construct decl_env c acc parent_hint

let is_lateinit cv =
  Attrs.mem SN.UserAttributes.uaLateInit cv.cv_user_attributes

let prop_needs_init cv =
  if cv.cv_is_xhp then false
  else if is_lateinit cv then false
  else match cv.cv_type with
    | None
    | Some (_, Hoption _)
    | Some (_, Hmixed) -> false
    | Some _ -> cv.cv_expr = None

let own_props c props =
  List.fold_left c.c_vars ~f:begin fun acc cv ->
    if prop_needs_init cv
    then SSet.add (snd cv.cv_id) acc
    else acc
  end ~init:props

let parent_props decl_env c props =
  List.fold_left c.c_extends ~f:begin fun acc parent ->
    match parent with
    | _, Happly ((_, parent), _) ->
      let tc = Decl_env.get_class_dep decl_env parent in
      begin match tc with
        | None -> acc
        | Some { dc_deferred_init_members = members; _ } ->
          SSet.union members acc
      end
    | _ -> acc
  end ~init:props

let trait_props decl_env c props =
  List.fold_left c.c_uses ~f:begin fun acc -> function
    | _, Happly ((_, trait), _) -> begin
      let class_ = Decl_env.get_class_dep decl_env trait in
      match class_ with
      | None -> acc
      | Some { dc_construct = cstr; dc_deferred_init_members = members; _ } ->
        (* If our current class defines its own constructor, completely ignore
         * the fact that the trait may have had one defined and merge in all of
         * its members.
         * If the curr. class does not have its own constructor, only fold in
         * the trait members if it would not have had its own constructor when
         * defining `dc_deferred_init_members`. See logic in `class_` for
         * Ast.Cabstract to see where this deviated for traits.
         *)
        begin match fst cstr with
          | None -> SSet.union members acc
          | Some cstr when cstr.elt_origin <> trait || cstr.elt_abstract ->
              SSet.union members acc
          | _ when c.c_constructor <> None -> SSet.union members acc
          | _ -> acc
        end
    end
    | _ -> acc
  end ~init:props

(* return a tuple of the private init-requiring props of the class
 * and all init-requiring props of the class and its ancestors *)
let get_deferred_init_props decl_env c =
  let priv_props, props = List.fold_left ~f:(fun (priv_props, props) cv ->
    let name = snd cv.cv_id in
    let visibility = cv.cv_visibility in
    if not (prop_needs_init cv) then
      priv_props, props
    else if visibility = Private then
      SSet.add name priv_props, SSet.add name props
    else
      priv_props, SSet.add name props
  ) ~init:(SSet.empty, SSet.empty) c.c_vars in
  let props = parent_props decl_env c props in
  let props = parent decl_env c props in
  priv_props, props

let class_ ~has_own_cstr decl_env c =
  match c.c_kind with
  | Ast.Cabstract when not has_own_cstr ->
    let priv_props, props = get_deferred_init_props decl_env c in
    if priv_props <> SSet.empty then
      (* XXX: should priv_props be checked for a trait?
       * see chown_privates in typing_inherit *)
      Errors.constructor_required c.c_name priv_props;
    props
  | Ast.Ctrait -> snd (get_deferred_init_props decl_env c)
  | _ -> SSet.empty
