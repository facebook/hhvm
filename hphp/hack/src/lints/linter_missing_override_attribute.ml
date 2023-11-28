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
module Cls = Decl_provider.Class
module SN = Naming_special_names

let has_override_attribute m =
  List.exists m.m_user_attributes ~f:(fun ua ->
      String.equal (snd ua.ua_name) SN.UserAttributes.uaOverride)

let is_interface c = Ast_defs.is_c_interface (Cls.kind c)

(* Do not emit an error for classes implementing an interface method, but do
   emit errors for interfaces overriding interface methods, classes overriding
   class methods, classes overriding trait methods, etc. *)
let both_are_or_are_not_interfaces c1 c2 =
  Bool.equal (is_interface c1) (is_interface c2)

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
  (* filter out interfaces *)
  let reqs =
    List.filter
      ~f:(fun class_name ->
        match Decl_provider.get_class ctx class_name with
        | Decl_entry.DoesNotExist
        | Decl_entry.NotYetAvailable ->
          false
        | Decl_entry.Found cls -> not (Ast_defs.is_c_interface (Cls.kind cls)))
      reqs
  in
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
  (* which doesn't have the override attribute, *)
  |> Sequence.filter ~f:(fun m -> not (has_override_attribute m))
  (* and is not the constructor, *)
  |> Sequence.filter ~f:(fun m ->
         not (String.equal (snd m.m_name) SN.Members.__construct))
  |> Sequence.iter ~f:(fun m ->
         let (p, mid) = m.m_name in
         let matching_ancestor =
           ancestor_names
           (* inspect each ancestor, *)
           |> List.filter_map ~f:(fun c ->
                  Decl_provider.get_class ctx c |> Decl_entry.to_option)
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
           (* get the class which defined that method, *)
           |> List.filter_map ~f:(fun m ->
                  Decl_provider.get_class ctx m.ce_origin
                  |> Decl_entry.to_option)
           (* as long as it and this class are of the same kind. *)
           |> List.filter ~f:(both_are_or_are_not_interfaces cls)
           (* If such a class exists... *)
           |> List.hd
         in
         match matching_ancestor with
         | Some ancestor ->
           (* ...then this method should have had the override attribute. *)
           let first_attr_pos =
             Option.map
               ~f:(fun ua -> fst ua.ua_name)
               (List.hd m.m_user_attributes)
           in

           Lints_errors.missing_override_attribute
             ~meth_pos:m.m_span
             ~name_pos:p
             ~first_attr_pos
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
      | Decl_entry.DoesNotExist
      | Decl_entry.NotYetAvailable ->
        ()
      | Decl_entry.Found cls ->
        check_methods ctx c cls ~static:false;
        check_methods ctx c cls ~static:true
  end
