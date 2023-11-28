(*
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* This linter warns if a class overrides all instance/static methods
   and properties of a trait the class uses *)

open Hh_prelude
open Aast
module Cls = Decl_provider.Class

let trait_name_from_hint th =
  match th with
  | Happly ((_, tid), _) -> Some tid
  | _ -> None

(* collect the names of properties and methods of a Cls.t *)
let names_and_origins_defined_by_cls_t cls =
  (* static properties in decls have a $ prefix, removing it *)
  let remove_trailing_dollar s =
    match String.chop_prefix s ~prefix:"$" with
    | None -> s
    | Some s -> s
  in
  let accessors =
    [
      (Cls.methods, (fun m -> not (Typing_defs.get_ce_abstract (snd m))), fst);
      (Cls.smethods, (fun m -> not (Typing_defs.get_ce_abstract (snd m))), fst);
      (Cls.props, (fun _ -> true), fst);
      (Cls.sprops, (fun _ -> true), (fun p -> remove_trailing_dollar (fst p)));
    ]
  in
  List.fold_left
    ~init:(SSet.empty, SMap.empty)
    ~f:(fun set (accessor, condition, sanitize) ->
      List.fold_left
        ~init:set
        ~f:(fun (names, origins) el ->
          if condition el then
            let name = sanitize el in
            let element = snd el in
            ( SSet.add name names,
              SMap.add name element.Typing_defs.ce_origin origins )
          else
            (names, origins))
        (accessor cls))
    accessors

(* collect the names of properties and methods of a class_ *)
let names_and_pos_defined_by_class_ class_ =
  let method_names_pos =
    List.fold_left
      ~init:(SSet.empty, SMap.empty)
      ~f:(fun (names, pos) m ->
        ( SSet.add (snd m.m_name) names,
          SMap.add (snd m.m_name) (fst m.m_name) pos ))
      class_.c_methods
  in
  List.fold_left
    ~init:method_names_pos
    ~f:(fun (names, pos) cv ->
      (SSet.add (snd cv.cv_id) names, SMap.add (snd cv.cv_id) (fst cv.cv_id) pos))
    class_.c_vars

(* Does this [trait] implement any interfaces?

   This looks for traits of the form:

   trait X implements I {} // true

   It does not include traits with requirements.

   trait X { require implements I; } // false
*)
let trait_implements_interfaces ctx (trait : Cls.t) : bool =
  let is_interface name : bool =
    match Decl_provider.get_class ctx name with
    | Decl_entry.Found decl ->
      (match Cls.kind decl with
      | Ast_defs.Cinterface -> true
      | _ -> false)
    | Decl_entry.DoesNotExist
    | Decl_entry.NotYetAvailable ->
      (* If we can't find this type (e.g. the user hasn't finished
         writing it), conservatively assume it's an interface. *)
      true
  in

  let interface_ancestors =
    List.filter ~f:is_interface (Cls.all_ancestor_names trait)
  in
  not (List.is_empty interface_ancestors)

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_class_ env c =
      let (pos, bid) = c.c_name in
      let ctx = Tast_env.get_ctx env in

      let (base_names, base_pos_map) = names_and_pos_defined_by_class_ c in

      let used_trait_names =
        List.filter_map ~f:(fun u -> trait_name_from_hint (snd u)) c.c_uses
      in

      let required_classes =
        List.filter_map
          ~f:(fun (u, k) ->
            match k with
            | RequireClass -> trait_name_from_hint (snd u)
            | RequireExtends
            | RequireImplements ->
              None)
          c.c_reqs
      in

      (* if the class or trait uses a trait that does not have require class constraints,
       * and does not implement an interface, then ensure that at least one method or
       * property of the trait is not overridden by the using class.
       *)
      List.iter
        ~f:(fun tid ->
          match Decl_provider.get_class ctx tid with
          | Decl_entry.DoesNotExist
          | Decl_entry.NotYetAvailable ->
            ()
          | Decl_entry.Found t_cls ->
            let trait_names = fst (names_and_origins_defined_by_cls_t t_cls) in
            if
              List.is_empty (Cls.all_ancestor_req_class_requirements t_cls)
              && (not (SSet.is_empty trait_names))
              && SSet.subset trait_names base_names
              && not (trait_implements_interfaces ctx t_cls)
            then
              Lints_errors.class_overrides_all_trait_methods
                pos
                bid
                (Cls.name t_cls)
            else
              ())
        used_trait_names;
      (* if the trait has a require class C constraint, ensure that the class C does
       * not override any property or method of the trait
       *)
      List.iter
        ~f:(fun cid ->
          match Decl_provider.get_class ctx cid with
          | Decl_entry.DoesNotExist
          | Decl_entry.NotYetAvailable ->
            ()
          | Decl_entry.Found c_cls ->
            let (class_names, origins) =
              names_and_origins_defined_by_cls_t c_cls
            in
            let dead_names = SSet.inter class_names base_names in
            SSet.iter
              (fun n ->
                let origin = SMap.find n origins in
                if not (String.equal origin bid) then
                  Lints_errors.trait_requires_class_that_overrides_method
                    (SMap.find n base_pos_map)
                    cid
                    bid
                    n)
              dead_names)
        required_classes
  end
