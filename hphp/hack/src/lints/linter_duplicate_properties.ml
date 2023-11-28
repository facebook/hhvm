(*
 * Copyright (c) 2020, Facebook, Inc.
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

(* efficient check for List.length l > 1 *)
let more_than_one l =
  match l with
  | _ :: _ :: _ -> true
  | _ -> false

(* All the trait names that appear in a use T statement inside class c *)
let traits c : (Pos.t * string) list =
  List.filter_map c.c_uses ~f:(fun (pos, trait_hint) ->
      match trait_hint with
      | Happly ((_, trait_name), _) -> Some (pos, trait_name)
      | _ -> None)

(* All the properites of class or trait [type name], flattened *)
let properties ctx (type_name : string) : (string * Typing_defs.class_elt) list
    =
  let decl = Decl_provider.get_class ctx type_name in
  match decl with
  | Decl_entry.Found decl -> Cls.props decl @ Cls.sprops decl
  | Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable ->
    []

let is_trait_name ctx (type_name : string) : bool =
  let decl = Decl_provider.get_class ctx type_name in
  match decl with
  | Decl_entry.Found decl -> Ast_defs.is_c_trait (Cls.kind decl)
  | Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable ->
    false

(* All the parent classes, used traits, and implemented interfaces of
   [type_name]. This is the flattened inheritance tree. *)
let all_ancestor_names ctx (type_name : string) : string list =
  let decl = Decl_provider.get_class ctx type_name in
  match decl with
  | Decl_entry.Found decl -> Decl_provider.Class.all_ancestor_names decl
  | Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable ->
    []

(* All the used traits of [type_name]. This is the flattened inheritance tree. *)
let all_trait_ancestors ctx (trait_name : string) : string list =
  let ancestors = all_ancestor_names ctx trait_name in
  (* Trait ancestors can be other traits or interfaces. *)
  let ancestor_traits = List.filter ancestors ~f:(is_trait_name ctx) in
  trait_name :: ancestor_traits

let get_class_nast ctx file_path c_name =
  let open Option in
  let ast = Ast_provider.find_class_in_file ~full:true ctx file_path c_name in
  ast >>| fun ast -> Naming.class_ ctx ast

let check_initialisation ast expr : bool =
  let checker =
    object (_ : 'self)
      val mutable result = false

      method result = result

      inherit [_] Aast.iter as super

      method! on_expr ast ((_, _, expr_) as expr) =
        super#on_expr ast expr;
        match expr_ with
        | Class_const (_, (_, s)) ->
          if String.( <> ) s Naming_special_names.Members.mClass then
            result <- true
        | _ -> ()
    end
  in
  checker#on_expr ast expr;
  checker#result

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_class_ env c =
      let ctx = Tast_env.get_ctx env in
      let (cls_pos, cls_name) = c.c_name in

      (* props_seen maps
       * - the property names of each trait used by cls to their origin
       * - the properties defined by cls itself to cls
       * If a property is inherited multiple times, props_seen will report multiple origins*)
      let props_seen = Hashtbl.create (module Base.String) in

      (* in decls static properties are stored with a leading dollar, in class_var without:
       * normalise removing leading dollars *)
      let strip_dollar s = String.lstrip s ~drop:(fun c -> Char.equal c '$') in

      (* add the properties defined in the class itself to props_seen *)
      List.iter c.c_vars ~f:(fun v ->
          Hashtbl.add_multi props_seen ~key:(snd v.cv_id) ~data:cls_name);

      (* for each used trait add the properties defined in the trait, mapped to their origin *)
      List.iter (traits c) ~f:(fun (_, type_name) ->
          let all_trait_ancestors =
            SSet.of_list (all_trait_ancestors ctx type_name)
          in
          List.iter (properties ctx type_name) ~f:(fun (prop_name, prop_elt) ->
              (* but do not add properties that are imported via require extends *)
              if SSet.mem prop_elt.ce_origin all_trait_ancestors then
                Hashtbl.add_multi
                  props_seen
                  ~key:(strip_dollar prop_name)
                  ~data:prop_elt.ce_origin));

      (* if the props_seen reports multiple origins for a property,
       * ensure that in none of the origins it is initialisedl
       * with an enum or class constant *)
      Hashtbl.iteri props_seen ~f:(fun ~key ~data ->
          if more_than_one data then
            (* property key is inherited multiple times, possibly via diamond inclusion
             * ensure that is never initialised with an enum or class constant *)
            let is_initialised_with_class_constant =
              List.fold data ~init:false ~f:(fun status origin ->
                  let open Option in
                  let res =
                    let opt_origin_file_path =
                      Naming_provider.get_class_path ctx origin
                    in
                    opt_origin_file_path >>= fun file_path ->
                    get_class_nast ctx file_path origin >>= fun ast ->
                    let opt_var =
                      List.find ast.c_vars ~f:(fun v ->
                          String.equal (snd v.cv_id) key)
                    in
                    opt_var >>= fun var ->
                    var.cv_expr >>| fun expr ->
                    status || check_initialisation ast expr
                  in
                  Option.value ~default:status res)
            in
            (* remove duplicate trait names arising from diamond inclusion from data *)
            let dedup_data = List.dedup_and_sort data ~compare:String.compare in

            if is_initialised_with_class_constant then
              (* HHVM will unconditinally fatal, so report a linter error *)
              Lints_errors.duplicate_property_class_constant_init
                cls_pos
                ~class_name:cls_name
                ~prop_name:key
                ~class_names:dedup_data
            else if more_than_one dedup_data then
              (* there is a duplicate property not arising from diamond inclusion, warn about it *)
              Lints_errors.duplicate_property
                cls_pos
                ~class_name:cls_name
                ~prop_name:key
                ~class_names:dedup_data)
  end
