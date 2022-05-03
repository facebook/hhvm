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
let names_defined_by_cls_t cls =
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
    ~init:SSet.empty
    ~f:(fun set (accessor, condition, sanitize) ->
      List.fold_left
        ~init:set
        ~f:(fun set el ->
          if condition el then
            SSet.add (sanitize el) set
          else
            set)
        (accessor cls))
    accessors

(* collect the names of properties and methods of a class_ *)
let names_defined_by_class_ class_ =
  let method_names =
    List.fold_left
      ~init:SSet.empty
      ~f:(fun set m -> SSet.add (snd m.m_name) set)
      class_.c_methods
  in
  List.fold_left
    ~init:method_names
    ~f:(fun set cv -> SSet.add (snd cv.cv_id) set)
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
    | Some decl ->
      (match Cls.kind decl with
      | Ast_defs.Cinterface -> true
      | _ -> false)
    | None ->
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
      let (pos, cid) = c.c_name in
      let ctx = Tast_env.get_ctx env in

      let class_names = names_defined_by_class_ c in

      let used_trait_names =
        List.filter_map ~f:(fun u -> trait_name_from_hint (snd u)) c.c_uses
      in

      List.iter
        ~f:(fun tid ->
          match Decl_provider.get_class ctx tid with
          | None -> ()
          | Some t_cls ->
            let trait_names = names_defined_by_cls_t t_cls in
            if
              (not (SSet.is_empty trait_names))
              && SSet.subset trait_names class_names
              && not (trait_implements_interfaces ctx t_cls)
            then
              Lints_errors.class_overrides_all_trait_methods
                pos
                cid
                (Cls.name t_cls)
            else
              ())
        used_trait_names
  end
