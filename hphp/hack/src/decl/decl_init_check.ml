(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Decl_defs
open Aast
open Shallow_decl_defs
open Typing_defs
module Attrs = Naming_attributes
module SN = Naming_special_names

(** Helpers for checking initialization of class properties by looking at decls. *)

type get_class_add_dep =
  Decl_env.env -> string -> Decl_defs.decl_class_type option

let parent_init_prop = "parent::" ^ SN.Members.__construct

(* If we need to call parent::__construct, we treat it as if it were
 * a class variable that needs to be initialized. It's a bit hacky
 * but it works. The idea here is that if the parent needs to be
 * initialized, we add a phony class variable. *)
let add_parent_construct
    ~(get_class_add_dep : get_class_add_dep) decl_env c props parent_hint =
  match parent_hint with
  | (_, Happly ((_, parent), _)) ->
    (match get_class_add_dep decl_env parent with
    | Some class_ when class_.dc_need_init ->
      let (c_constructor, _, _) = split_methods c.c_methods in
      if Option.is_some c_constructor then
        SSet.add parent_init_prop props
      else
        props
    | _ -> props)
  | _ -> props

(* As above, but for shallow_class decls rather than class NASTs. *)
let add_parent_construct_from_shallow_decl
    ~get_class_add_dep decl_env sc props parent_ty =
  match get_node parent_ty with
  | Tapply ((_, parent), _) ->
    (match get_class_add_dep decl_env parent with
    | Some class_ when class_.dc_need_init && Option.is_some sc.sc_constructor
      ->
      SSet.add parent_init_prop props
    | _ -> props)
  | _ -> props

let parent ~(get_class_add_dep : get_class_add_dep) decl_env c acc =
  if FileInfo.is_hhi c.c_mode then
    acc
  else if Ast_defs.is_c_trait c.c_kind then
    let (req_extends, _req_implements, _req_class, req_this_as) =
      split_reqs c.c_reqs
    in
    List.fold_left
      (req_extends @ req_this_as)
      ~init:acc
      ~f:(add_parent_construct ~get_class_add_dep decl_env c)
  else
    match c.c_extends with
    | [] -> acc
    | parent_ty :: _ ->
      add_parent_construct ~get_class_add_dep decl_env c acc parent_ty

(* As above, but for shallow_class decls rather than class NASTs. *)
let parent_from_shallow_decl
    ~(get_class_add_dep : get_class_add_dep) decl_env sc acc =
  if FileInfo.is_hhi sc.sc_mode then
    acc
  else if Ast_defs.is_c_trait sc.sc_kind then
    List.fold_left
      sc.sc_req_extends
      ~init:acc
      ~f:(add_parent_construct_from_shallow_decl ~get_class_add_dep decl_env sc)
  else
    match sc.sc_extends with
    | [] -> acc
    | parent_ty :: _ ->
      add_parent_construct_from_shallow_decl
        ~get_class_add_dep
        decl_env
        sc
        acc
        parent_ty

let is_lateinit cv =
  Attrs.mem SN.UserAttributes.uaLateInit cv.cv_user_attributes

let prop_may_need_init cv =
  if Option.is_some cv.cv_xhp_attr then
    false
  else if is_lateinit cv then
    false
  else
    match snd cv.cv_type with
    | None
    | Some (_, Hoption _)
    | Some (_, Hmixed) ->
      false
    | Some _ -> Option.is_none cv.cv_expr

(* As above, but for shallow_class decls rather than class NASTs. *)
let shallow_prop_may_need_init sp =
  if Option.is_some sp.sp_xhp_attr then
    false
  else if sp_lateinit sp then
    false
  else
    sp_needs_init sp

let own_props c props =
  List.fold_left c.c_vars ~init:props ~f:(fun acc cv ->
      if prop_may_need_init cv then
        SSet.add (snd cv.cv_id) acc
      else
        acc)

let init_not_required_props c props =
  List.fold_left c.c_vars ~init:props ~f:(fun acc cv ->
      if prop_may_need_init cv then
        acc
      else
        SSet.add (snd cv.cv_id) acc)

let parent_props ~(get_class_add_dep : get_class_add_dep) decl_env c props =
  List.fold_left c.c_extends ~init:props ~f:(fun acc parent ->
      match parent with
      | (_, Happly ((_, parent), _)) ->
        let tc = get_class_add_dep decl_env parent in
        (match tc with
        | None -> acc
        | Some { dc_deferred_init_members = members; _ } ->
          SSet.union members acc)
      | _ -> acc)

(* As above, but for shallow_class decls rather than class NASTs. *)
let parent_props_from_shallow_decl
    ~(get_class_add_dep : get_class_add_dep) decl_env sc props =
  List.fold_left sc.sc_extends ~init:props ~f:(fun acc parent ->
      match get_node parent with
      | Tapply ((_, parent), _) ->
        let tc = get_class_add_dep decl_env parent in
        (match tc with
        | None -> acc
        | Some { dc_deferred_init_members = members; _ } ->
          SSet.union members acc)
      | _ -> acc)

let trait_props ~(get_class_add_dep : get_class_add_dep) decl_env c props =
  List.fold_left c.c_uses ~init:props ~f:(fun acc trait_hint ->
      match trait_hint with
      | (_, Happly ((_, trait), _)) ->
        let class_ = get_class_add_dep decl_env trait in
        (match class_ with
        | None -> acc
        | Some { dc_construct = cstr; dc_deferred_init_members = members; _ } ->
          (* If our current class defines its own constructor, completely ignore
           * the fact that the trait may have had one defined and merge in all of
           * its members.
           * If the curr. class does not have its own constructor, only fold in
           * the trait members if it would not have had its own constructor when
           * defining `dc_deferred_init_members`. See logic in `class_` for
           * Ast_defs.Cclass (Abstract) to see where this deviated for traits. *)
          (match fst cstr with
          | None -> SSet.union members acc
          | Some cstr
            when String.( <> ) cstr.elt_origin trait || get_elt_abstract cstr ->
            SSet.union members acc
          | _ ->
            let (c_constructor, _, _) = split_methods c.c_methods in
            if Option.is_some c_constructor then
              SSet.union members acc
            else
              acc))
      | _ -> acc)

(** return the private init-requiring props of the class from its NAST *)
let get_private_deferred_init_props c =
  List.fold_left c.c_vars ~init:SSet.empty ~f:(fun priv_props cv ->
      let name = snd cv.cv_id in
      let visibility = cv.cv_visibility in
      if prop_may_need_init cv && Aast.(equal_visibility visibility Private)
      then
        SSet.add name priv_props
      else
        priv_props)

(** return all init-requiring props of the class and its ancestors from the
    given shallow class decl and the ancestors' folded decls. *)
let get_nonprivate_deferred_init_props
    ~(get_class_add_dep : get_class_add_dep) decl_env sc =
  let props =
    List.fold_left sc.sc_props ~init:SSet.empty ~f:(fun props sp ->
        if shallow_prop_may_need_init sp then
          SSet.add (snd sp.sp_name) props
        else
          props)
  in
  let props =
    parent_props_from_shallow_decl ~get_class_add_dep decl_env sc props
  in
  let props = parent_from_shallow_decl ~get_class_add_dep decl_env sc props in
  props

let private_deferred_init_props ~has_own_cstr c =
  match c.c_kind with
  | Ast_defs.Cclass k when Ast_defs.is_abstract k && not has_own_cstr ->
    get_private_deferred_init_props c
  | Ast_defs.Ctrait -> get_private_deferred_init_props c
  | Ast_defs.(Cclass _ | Cinterface | Cenum | Cenum_class _) -> SSet.empty

let nonprivate_deferred_init_props
    ~has_own_cstr ~(get_class_add_dep : get_class_add_dep) decl_env sc =
  match sc.sc_kind with
  | Ast_defs.Cclass k when Ast_defs.is_abstract k && not has_own_cstr ->
    get_nonprivate_deferred_init_props ~get_class_add_dep decl_env sc
  | Ast_defs.Ctrait ->
    get_nonprivate_deferred_init_props ~get_class_add_dep decl_env sc
  | Ast_defs.(Cclass _ | Cinterface | Cenum | Cenum_class _) -> SSet.empty

(**
 * [parent_initialized_members decl_env c] returns all members initialized in
 * the parents of [c], s.t. they should not be readable within [c]'s
 * [__construct] method until _after_ [parent::__construct] has been called.
 *)
let parent_initialized_members ~get_class_add_dep decl_env c =
  let parent_initialized_members_helper = function
    | None -> SSet.empty
    | Some { dc_props; _ } ->
      dc_props
      |> SMap.filter (fun _ p -> Decl_defs.get_elt_needs_init p)
      |> SMap.keys
      |> SSet.of_list
  in
  List.fold_left c.c_extends ~init:SSet.empty ~f:(fun acc parent ->
      match parent with
      | (_, Happly ((_, parent), _)) ->
        get_class_add_dep decl_env parent
        |> parent_initialized_members_helper
        |> SSet.union acc
      | _ -> acc)
