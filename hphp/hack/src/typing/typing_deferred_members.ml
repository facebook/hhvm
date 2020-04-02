(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Shallow_decl_defs
open Typing_defs
module Attrs = Naming_attributes
module Cls = Decl_provider.Class
module Env = Typing_env
module SN = Naming_special_names

let parent_init_prop = "parent::" ^ SN.Members.__construct

(* If we need to call parent::__construct, we treat it as if it were
 * a class variable that needs to be initialized. It's a bit hacky
 * but it works. The idea here is that if the parent needs to be
 * initialized, we add a phony class variable. *)
let add_parent_construct env c props parent_ty =
  match get_node parent_ty with
  | Tapply ((_, parent), _) ->
    begin
      match Env.get_class_dep env parent with
      | Some class_ when Cls.need_init class_ && Option.is_some c.sc_constructor
        ->
        SSet.add parent_init_prop props
      | _ -> props
    end
  | _ -> props

let parent env c acc =
  if FileInfo.(equal_mode c.sc_mode Mdecl) then
    acc
  else if Ast_defs.(equal_class_kind c.sc_kind Ctrait) then
    List.fold_left c.sc_req_extends ~f:(add_parent_construct env c) ~init:acc
  else
    match c.sc_extends with
    | [] -> acc
    | parent_ty :: _ -> add_parent_construct env c acc parent_ty

let is_lateinit cv =
  Attrs.mem SN.UserAttributes.uaLateInit cv.cv_user_attributes

let prop_needs_init sp =
  if Option.is_some sp.sp_xhp_attr then
    false
  else if sp.sp_lateinit then
    false
  else
    match sp.sp_type with
    | None -> false
    | Some ty ->
      (match get_node ty with
      | Tprim Tnull
      | Toption _
      | Tmixed ->
        false
      | _ -> sp.sp_needs_init)

let own_props c props =
  List.fold_left
    c.sc_props
    ~f:
      begin
        fun acc sp ->
        if prop_needs_init sp then
          SSet.add (snd sp.sp_name) acc
        else
          acc
      end
    ~init:props

let initialized_props c props =
  List.fold_left
    c.sc_props
    ~f:
      begin
        fun acc sp ->
        if (not (prop_needs_init sp)) && not sp.sp_lateinit then
          SSet.add (snd sp.sp_name) acc
        else
          acc
      end
    ~init:props

let rec parent_props env c props =
  List.fold_left
    c.sc_extends
    ~f:
      begin
        fun acc parent ->
        match get_node parent with
        | Tapply ((_, parent), _) ->
          begin
            match Shallow_classes_provider.get (Env.get_ctx env) parent with
            | None -> acc
            | Some sc ->
              let members = class_ env sc in
              SSet.union members acc
          end
        | _ -> acc
      end
    ~init:props

and trait_props env c props =
  List.fold_left
    c.sc_uses
    ~f:
      begin
        fun acc ty ->
        match get_node ty with
        | Tapply ((_, trait), _) ->
          let cls = Env.get_class_dep env trait in
          let shallow_class =
            Shallow_classes_provider.get (Env.get_ctx env) trait
          in
          (match (cls, shallow_class) with
          | (Some cls, Some sc) ->
            let cstr = Cls.construct cls in
            let members = class_ env sc in
            (* If our current class defines its own constructor, completely ignore
             * the fact that the trait may have had one defined and merge in all of
             * its members.
             * If the curr. class does not have its own constructor, only fold in
             * the trait members if it would not have had its own constructor when
             * defining `dc_deferred_init_members`. See logic in `class_` for
             * Ast_defs.Cabstract to see where this deviated for traits.
             *)
            begin
              match fst cstr with
              | None -> SSet.union members acc
              | Some cstr
                when String.( <> ) cstr.ce_origin trait || get_ce_abstract cstr
                ->
                SSet.union members acc
              | _ when Option.is_some c.sc_constructor -> SSet.union members acc
              | _ -> acc
            end
          | _ -> acc)
        | _ -> acc
      end
    ~init:props

(* return a tuple of the private init-requiring props of the class
 * and all init-requiring props of the class and its ancestors *)
and get_deferred_init_props env c =
  let (priv_props, props) =
    List.fold_left
      ~f:(fun (priv_props, props) sp ->
        let name = snd sp.sp_name in
        let visibility = sp.sp_visibility in
        if not (prop_needs_init sp) then
          (priv_props, props)
        else if Aast_defs.equal_visibility visibility Private then
          (SSet.add name priv_props, SSet.add name props)
        else
          (priv_props, SSet.add name props))
      ~init:(SSet.empty, SSet.empty)
      c.sc_props
  in
  let props = parent_props env c props in
  let props = parent env c props in
  (priv_props, props)

and class_ env c =
  match Decl_provider.get_class (Typing_env.get_ctx env) (snd c.sc_name) with
  | None -> SSet.empty
  | Some cls ->
    let has_concrete_cstr = Cls.need_init cls in
    let has_own_cstr = has_concrete_cstr && Option.is_some c.sc_constructor in
    (match c.sc_kind with
    | Ast_defs.Cabstract when not has_own_cstr ->
      let (priv_props, props) = get_deferred_init_props env c in
      if not (SSet.is_empty priv_props) then
        (* XXX: should priv_props be checked for a trait?
         * see chown_privates in typing_inherit *)
        Errors.constructor_required c.sc_name priv_props;
      props
    | Ast_defs.Ctrait -> snd (get_deferred_init_props env c)
    | _ -> SSet.empty)
