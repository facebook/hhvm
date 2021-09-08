(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
module Cls = Decl_provider.Class
module SN = Naming_special_names

let has_via_label_attribute param =
  Naming_attributes.mem SN.UserAttributes.uaViaLabel param.param_user_attributes

let method_has_via_label m =
  match m.m_params with
  | [] -> false
  | method_param :: _ -> has_via_label_attribute method_param

let method_ty_has_via_label mty =
  match get_node mty with
  | Tfun ft ->
    let params = ft.ft_params in
    begin
      match params with
      | [] -> false
      | param :: _ ->
        Typing_defs_flags.(is_set fp_flags_via_label param.fp_flags)
    end
  | _ -> false

(* Return true only if 1) the given method is public or protected, or 2) the
   given ancestor is a trait (since a user of the trait will inherit private
   trait methods) *)
let should_check_ancestor_method ancestor_class ancestor_method =
  if Ast_defs.is_c_trait (Cls.kind ancestor_class) then
    true
  else
    match ancestor_method.ce_visibility with
    | Vpublic
    | Vprotected _
    | Vinternal _ ->
      true
    | Vprivate _ -> false

let check_methods ctx c cls ~static =
  let ancestor_names = Cls.all_ancestor_names cls in
  let reqs = Cls.all_ancestor_req_names cls in
  let ancestor_names = ancestor_names @ reqs in
  let get_method =
    if static then
      Cls.get_smethod
    else
      Cls.get_method
  in
  (* For each method, *)
  let (_, static_methods, c_methods) = split_methods c.c_methods in
  (if static then
    static_methods
  else
    c_methods)
  |> Sequence.of_list
  (* which doesn't have the __ViaLabel attribute, *)
  |> Sequence.filter ~f:(fun m -> not (method_has_via_label m))
  |> Sequence.iter ~f:(fun m ->
         let (p, mid) = m.m_name in
         let matching_ancestor =
           ancestor_names
           (* inspect each ancestor, *)
           |> List.filter_map ~f:(Decl_provider.get_class ctx)
           (* and if it has a method with the same name, and either that method
              is non-private or the ancestor is a trait, *)
           |> List.filter_map ~f:(fun ancestor ->
                  match get_method ancestor mid with
                  | None -> None
                  | Some ancestor_method ->
                    if should_check_ancestor_method ancestor ancestor_method
                    then
                      Some ancestor_method
                    else
                      None)
           (* if the ancestor has the attribute, get the class which defined that method *)
           |> List.filter_map ~f:(fun m ->
                  let (lazy fty_parent) = m.ce_type in
                  if method_ty_has_via_label fty_parent then
                    Decl_provider.get_class ctx m.ce_origin
                  else
                    None)
           (* If such a class exists... *)
           |> List.hd
         in
         match matching_ancestor with
         | Some ancestor ->
           (* ...then this method first argument should have had the
            * __ViaLabel attribute. *)
           Lints_errors.missing_via_label_attribute
             p
             ~class_name:(Cls.name ancestor)
             ~method_name:mid
         | None -> ())

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_class_ env c =
      let cid = snd c.c_name in
      let ctx = Tast_env.get_ctx env in
      match Decl_provider.get_class ctx cid with
      | None -> ()
      | Some cls ->
        check_methods ctx c cls ~static:false;
        check_methods ctx c cls ~static:true
  end
