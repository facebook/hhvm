(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Module which checks class members against the full set of members it
    inherited, including those which were overridden. *)

open Core_kernel
open Decl_defs
open Reordered_argument_collections
open Shallow_decl_defs
open Typing_defs
module Cls = Decl_provider.Class
module Env = Typing_env

(** Ensure that all methods which have the override annotation and were
    shallowly defined within [cls] do in fact override some inherited
    definition. *)
let check_override_annotations cls ~static =
  let methods =
    let sc = Cls.shallow_decl cls in
    if static then
      sc.sc_static_methods
    else
      sc.sc_methods
  in
  List.iter methods (fun meth ->
      if not meth.sm_override then
        ()
      else
        let (get_method, all_methods_named) =
          if static then
            (Cls.get_smethod, Cls.all_inherited_smethods)
          else
            (Cls.get_method, Cls.all_inherited_methods)
        in
        let (pos, id) = meth.sm_name in
        match get_method cls id with
        | None -> ()
        | Some meth ->
          let parent_method_exists =
            List.exists (all_methods_named cls id) (fun parent_meth ->
                meth.ce_origin <> parent_meth.ce_origin)
          in
          if not parent_method_exists then
            Errors.should_be_override pos (Cls.name cls) id)

(** Ensure that all methods which have the override annotation, were inherited
    by [cls], and were originally defined in a trait do in fact override some
    other inherited definition. *)
let check_trait_override_annotations env cls ~static =
  let (methods, all_methods_named) =
    if static then
      (Cls.smethods, Cls.all_inherited_smethods)
    else
      (Cls.methods, Cls.all_inherited_methods)
  in
  Sequence.iter (methods cls) (fun (id, meth) ->
      if not meth.ce_override then
        ()
      else if meth.ce_origin = Cls.name cls then
        ()
      else
        match Env.get_class env meth.ce_origin with
        | None -> ()
        | Some parent_class ->
          if Cls.kind parent_class <> Ast_defs.Ctrait then
            ()
          else (
            match meth with
            | { ce_type = (lazy (_, Tfun { ft_pos; _ })); _ } ->
              let parent_method_exists =
                List.exists (all_methods_named cls id) (fun parent_meth ->
                    meth.ce_origin <> parent_meth.ce_origin)
              in
              if not parent_method_exists then
                Errors.override_per_trait (Cls.pos cls, Cls.name cls) id ft_pos
            | _ -> ()
          ))

let check_if_cyclic cls =
  let cyclic_classes =
    Decl_linearize.get_linearization (Cls.name cls)
    |> Sequence.find_map ~f:(fun mro -> mro.mro_cyclic)
  in
  match cyclic_classes with
  | None -> ()
  | Some classes ->
    let classes = SSet.add classes (Cls.name cls) in
    Errors.cyclic_class_def classes (Cls.pos cls)

let check_extend_kind parent_pos parent_kind child_pos child_kind =
  Ast_defs.(
    match (parent_kind, child_kind) with
    | ((Cabstract | Cnormal), (Cabstract | Cnormal))
    | (Cabstract, Cenum) (* enums extend BuiltinEnum under the hood *)
    | (Ctrait, Ctrait)
    | (Cinterface, Cinterface) ->
      ()
    | _ ->
      let parent = Ast_defs.string_of_class_kind parent_kind in
      let child = Ast_defs.string_of_class_kind child_kind in
      Errors.wrong_extend_kind child_pos child parent_pos parent)

let check_extend_kinds shallow_class =
  let class_pos = fst shallow_class.sc_name in
  let class_kind = shallow_class.sc_kind in
  List.iter shallow_class.sc_extends ~f:(fun ty ->
      let (_, (parent_pos, parent_name), _) =
        Decl_utils.unwrap_class_type ty
      in
      match Shallow_classes_heap.get parent_name with
      | None -> ()
      | Some parent ->
        check_extend_kind parent_pos parent.sc_kind class_pos class_kind)

let no_trait_reuse_enabled env =
  TypecheckerOptions.experimental_feature_enabled
    (Env.get_tcopt env)
    TypecheckerOptions.experimental_no_trait_reuse

let check_trait_reuse shallow_class =
  let class_name = snd shallow_class.sc_name in
  Decl_linearize.(get_linearization ~kind:Ancestor_types) class_name
  |> Sequence.iter ~f:(fun mro ->
         match mro.mro_trait_reuse with
         | None -> ()
         | Some parent_name ->
           let parent_pos =
             Shallow_classes_heap.get parent_name
             |> Option.value_map ~default:Pos.none ~f:(fun p -> fst p.sc_name)
           in
           Errors.trait_reuse
             parent_pos
             parent_name
             shallow_class.sc_name
             mro.mro_name)

let check_class env cls =
  check_if_cyclic cls;
  let shallow_class = Cls.shallow_decl cls in
  check_extend_kinds shallow_class;
  if no_trait_reuse_enabled env then check_trait_reuse shallow_class;
  if Cls.kind cls <> Ast_defs.Ctrait then (
    check_override_annotations cls ~static:false;
    check_override_annotations cls ~static:true
  );
  if Cls.kind cls = Ast_defs.Cnormal then (
    check_trait_override_annotations env cls ~static:false;
    check_trait_override_annotations env cls ~static:true
  );
  ()
