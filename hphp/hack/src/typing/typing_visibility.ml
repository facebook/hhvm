(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 * *)
open Hh_prelude
open Aast
open Typing_defs
open Utils
module Env = Typing_env
module TUtils = Typing_utils
module Cls = Decl_provider.Class

let is_private_visible env x self_id =
  if String.equal x self_id then
    None
  else
    let my_class = Env.get_class env self_id in
    let their_class = Env.get_class env x in
    match (my_class, their_class) with
    | (Some my_class, Some their_class) ->
      let make_tany _ = mk (Reason.Rnone, Typing_defs.make_tany ()) in
      let my_class_ty =
        Typing_make_type.class_type
          Reason.Rnone
          self_id
          (List.map (Cls.tparams my_class) make_tany)
      in
      let their_class_ty =
        Typing_make_type.class_type
          Reason.Rnone
          x
          (List.map (Cls.tparams their_class) make_tany)
      in
      if
        Typing_utils.is_sub_type env my_class_ty their_class_ty
        && Typing_utils.is_sub_type env their_class_ty my_class_ty
      then
        None
      else
        Some "You cannot access this member"
    | (_, _) -> Some "You cannot access this member"

let is_protected_visible env x self_id =
  if String.equal x self_id then
    None
  else
    let my_class = Env.get_class env self_id in
    let their_class = Env.get_class env x in
    match (my_class, their_class) with
    | (Some my_class, Some their_class) ->
      (* Children can call parent's protected methods and
       * parents can call children's protected methods (like a
       * constructor) *)
      let make_tany _ = mk (Reason.Rnone, Typing_defs.make_tany ()) in
      let my_class_ty =
        Typing_make_type.class_type
          Reason.Rnone
          self_id
          (List.map (Cls.tparams my_class) make_tany)
      in
      let their_class_ty =
        Typing_make_type.class_type
          Reason.Rnone
          x
          (List.map (Cls.tparams their_class) make_tany)
      in
      if
        Typing_utils.is_sub_type env my_class_ty their_class_ty
        || Cls.extends their_class self_id
        || (not (Cls.members_fully_known my_class))
        (* This is for the case where type generics are emitted *)
        || Cls.has_ancestor my_class x
      then
        None
      else
        Some
          ("Cannot access this protected member, you don't extend " ^ strip_ns x)
    | (_, _) -> None

let is_private_visible_for_class env x self_id cid class_ =
  match cid with
  | CIstatic ->
    let my_class = Env.get_class env self_id in
    (match my_class with
    | Some cls when Cls.final cls -> None
    | _ ->
      Some
        "Private members cannot be accessed with static:: since a child class may also have an identically named private member")
  | CIparent -> Some "You cannot access a private member with parent::"
  | CIself -> is_private_visible env x self_id
  | CI (_, called_ci) ->
    (match is_private_visible env x self_id with
    | None -> None
    | Some _ ->
      begin
        match Env.get_class env called_ci with
        | Some cls when Ast_defs.(equal_class_kind (Cls.kind cls) Ctrait) ->
          Some
            "You cannot access private members using the trait's name (did you mean to use self::?)"
        | _ -> Some "You cannot access this member"
      end)
  | CIexpr _ ->
    if Cls.final class_ then
      None
    else
      Some
        "Private members cannot be accessed dynamically. Did you mean to use 'self::'?"

let is_visible_for_obj env vis =
  match vis with
  | Vpublic -> None
  | Vprivate x ->
    (match Env.get_self_id env with
    | None -> Some "You cannot access this member"
    | Some self_id -> is_private_visible env x self_id)
  | Vprotected x ->
    (match Env.get_self_id env with
    | None -> Some "You cannot access this member"
    | Some self_id -> is_protected_visible env x self_id)

(* The only permitted way to access an LSB property is via
   static::, ClassName::, or $class_name:: *)
let is_lsb_permitted cid =
  match cid with
  | CIself -> Some "__LSB properties cannot be accessed with self::"
  | CIparent -> Some "__LSB properties cannot be accessed with parent::"
  | _ -> None

(* LSB property accessibility is relative to the defining class *)
let is_lsb_accessible env vis =
  match vis with
  | Vpublic -> None
  | Vprivate x ->
    (match Env.get_self_id env with
    | None -> Some "You cannot access this member"
    | Some self_id -> is_private_visible env x self_id)
  | Vprotected x ->
    (match Env.get_self_id env with
    | None -> Some "You cannot access this member"
    | Some self_id -> is_protected_visible env x self_id)

let is_lsb_visible_for_class env vis cid =
  match is_lsb_permitted cid with
  | Some x -> Some x
  | None -> is_lsb_accessible env vis

let is_visible_for_class env (vis, lsb) cid cty =
  if lsb then
    is_lsb_visible_for_class env vis cid
  else
    match vis with
    | Vpublic -> None
    | Vprivate x ->
      (match Env.get_self_id env with
      | None -> Some "You cannot access this member"
      | Some self_id -> is_private_visible_for_class env x self_id cid cty)
    | Vprotected x ->
      (match Env.get_self_id env with
      | None -> Some "You cannot access this member"
      | Some self_id ->
        let their_class = Env.get_class env x in
        (match (cid, their_class) with
        | (CI _, Some cls)
          when Ast_defs.(equal_class_kind (Cls.kind cls) Ctrait) ->
          Some
            "You cannot access protected members using the trait's name (did you mean to use static:: or self::?)"
        | _ -> is_protected_visible env x self_id))

let is_visible env (vis, lsb) cid class_ =
  let msg_opt =
    match cid with
    | Some cid -> is_visible_for_class env (vis, lsb) cid class_
    | None -> is_visible_for_obj env vis
  in
  Option.is_none msg_opt

let visibility_error p msg (p_vis, vis) =
  let s = TUtils.string_of_visibility vis in
  let msg_vis = "This member is " ^ s in
  Errors.visibility p msg p_vis msg_vis

let check_obj_access ~use_pos ~def_pos env vis =
  match is_visible_for_obj env vis with
  | None -> ()
  | Some msg -> visibility_error use_pos msg (def_pos, vis)

let check_inst_meth_access ~use_pos ~def_pos vis =
  match vis with
  | Vprivate _ -> Errors.private_inst_meth ~def_pos ~use_pos
  | Vprotected _ -> Errors.protected_inst_meth ~def_pos ~use_pos
  | _ -> ()

let check_meth_caller_access ~use_pos ~def_pos vis =
  match vis with
  | Vprivate _ -> Errors.private_meth_caller ~def_pos ~use_pos
  | Vprotected _ -> Errors.protected_meth_caller ~def_pos ~use_pos
  | _ -> ()

let check_class_access ~use_pos ~def_pos env (vis, lsb) cid class_ =
  match is_visible_for_class env (vis, lsb) cid class_ with
  | None -> ()
  | Some msg -> visibility_error use_pos msg (def_pos, vis)

let check_deprecated ~use_pos ~def_pos deprecated =
  match deprecated with
  | Some s -> Errors.deprecated_use use_pos ~pos_def:(Some def_pos) s
  | None -> ()
