(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Decl_defs
open Nast
open Typing_defs

let parent_init_prop = "parent::" ^ SN.Members.__construct

(* If we need to call parent::__construct, we treat it as if it were
 * a class variable that needs to be initialized. It's a bit hacky
 * but it works. The idea here is that if the parent needs to be
 * initialized, we add a phony class variable. *)
let add_parent_construct decl_env c add_prop acc parent_hint =
  match parent_hint with
    | (_, Happly ((_, parent), _)) ->
      let class_ = Decl_env.get_class_dep decl_env parent in
      (match class_ with
        | Some class_ when
            class_.dc_need_init && c.c_constructor <> None
            -> add_prop parent_init_prop acc
        | _ -> acc
      )
    | _ -> acc

let parent decl_env c add_prop acc =
  if c.c_mode = FileInfo.Mdecl then acc
  else
    if c.c_kind = Ast.Ctrait
    then List.fold_left c.c_req_extends
      ~f:(add_parent_construct decl_env c add_prop) ~init:acc
    else match c.c_extends with
    | [] -> acc
    | parent_hint :: _ -> add_parent_construct decl_env c add_prop acc parent_hint

let prop_needs_init cv =
  if cv.cv_is_xhp then false
  else match cv.cv_type with
    | None
    | Some (_, Hoption _)
    | Some (_, Hmixed) -> false
    | Some _ -> cv.cv_expr = None

let own_props c add_prop acc =
  List.fold_left c.c_vars ~f:begin fun acc cv ->
    if prop_needs_init cv
    then add_prop (snd cv.cv_id) acc
    else acc
  end ~init:acc

let parent_props decl_env c add_prop acc =
  List.fold_left c.c_extends ~f:begin fun acc parent ->
    match parent with
    | _, Happly ((_, parent), _) ->
      let tc = Decl_env.get_class_dep decl_env parent in
      (match tc with
        | None -> acc
        | Some { dc_deferred_init_members = members; _ } ->
          SSet.fold add_prop members acc)
    | _ -> acc
  end ~init:acc

let trait_props decl_env c add_prop acc =
  List.fold_left c.c_uses ~f:begin fun acc -> function
    | _, Happly ((_, trait), _) -> begin
      let class_ = Decl_env.get_class_dep decl_env trait in
      match class_ with
      | None -> acc
      | Some { dc_construct = cstr; dc_deferred_init_members = members; _ } -> begin
        (* If our current class defines its own constructor, completely ignore
         * the fact that the trait may have had one defined and merge in all of
         * its members.
         * If the curr. class does not have its own constructor, only fold in
         * the trait members if it would not have had its own constructor when
         * defining `dc_deferred_init_members`. See logic in `class_` for
         * Ast.Cabstract to see where this deviated for traits.
         *)
        let add_props members acc = SSet.fold add_prop members acc in
        match fst cstr with
          | None -> add_props members acc
          | Some cstr when cstr.elt_origin <> trait || cstr.elt_abstract ->
              add_props members acc
          | _ when c.c_constructor <> None -> add_props members acc
          | _ -> acc
      end
    end
    | _ -> acc
  end ~init:acc

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
  let props = parent_props decl_env c SSet.add props in
  let props = parent decl_env c SSet.add props in
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
