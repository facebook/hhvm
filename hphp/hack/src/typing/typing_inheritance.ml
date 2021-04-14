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

open Hh_prelude
open Decl_defs
open Reordered_argument_collections
open Shallow_decl_defs
open Typing_defs
module Cls = Decl_provider.Class
module Env = Typing_env

(** Ensure that all methods which have the override annotation and were
    shallowly defined within [cls] do in fact override some inherited
    definition. *)
let check_override_annotations env cls ~static =
  let methods =
    let sc = Cls.shallow_decl cls in
    if static then
      sc.sc_static_methods
    else
      sc.sc_methods
  in
  List.iter methods (fun meth ->
      if not (sm_override meth) then
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
                String.( <> ) meth.ce_origin parent_meth.ce_origin)
          in
          if not parent_method_exists then
            Errors.should_be_override
              pos
              (Cls.name cls)
              id
              ~current_decl_and_file:(Env.get_current_decl_and_file env))

(** Ensure that all methods which have the override annotation, were inherited
    by [cls], and were originally defined in a trait do in fact override some
    other inherited definition. *)
let check_trait_override_annotations env class_pos cls ~static =
  let (methods, all_methods_named) =
    if static then
      (Cls.smethods, Cls.all_inherited_smethods)
    else
      (Cls.methods, Cls.all_inherited_methods)
  in
  List.iter (methods cls) (fun (id, meth) ->
      if not (get_ce_override meth) then
        ()
      else if String.equal meth.ce_origin (Cls.name cls) then
        ()
      else
        match Env.get_class env meth.ce_origin with
        | None -> ()
        | Some parent_class ->
          if not Ast_defs.(equal_class_kind (Cls.kind parent_class) Ctrait) then
            ()
          else (
            match meth with
            | { ce_type = (lazy ty); _ } ->
              let parent_method_exists =
                List.exists (all_methods_named cls id) (fun parent_meth ->
                    String.( <> ) meth.ce_origin parent_meth.ce_origin)
              in
              if not parent_method_exists then
                Errors.override_per_trait
                  (class_pos, Cls.name cls)
                  id
                  meth.ce_origin
                  (get_pos ty)
          ))

(** Error if any member of the class was detected to be cyclic
    during linearization. *)
let check_if_cyclic class_pos cls =
  let cyclic_classes =
    Cls.linearization cls Decl_defs.Member_resolution
    |> List.find_map ~f:(fun mro -> mro.mro_cyclic)
  in
  match cyclic_classes with
  | None -> ()
  | Some classes ->
    let classes = SSet.add classes (Cls.name cls) in
    Errors.cyclic_class_def classes class_pos

let check_extend_kind
    parent_pos
    parent_kind
    parent_name
    parent_is_enum_class
    child_pos
    child_kind
    child_name
    child_is_enum_class =
  Ast_defs.(
    match (parent_kind, child_kind) with
    | ((Cabstract | Cnormal), (Cabstract | Cnormal))
    | (Cabstract, Cenum) (* enums extend BuiltinEnum under the hood *)
    | (Ctrait, Ctrait)
    | (Cinterface, Cinterface) ->
      ()
    | (Ast_defs.Cenum, Ast_defs.Cenum) ->
      if parent_is_enum_class && child_is_enum_class then
        ()
      else
        Errors.wrong_extend_kind
          ~parent_pos
          ~parent_kind
          ~parent_name
          ~parent_is_enum_class
          ~child_pos
          ~child_kind
          ~child_name
          ~child_is_enum_class
    | _ ->
      Errors.wrong_extend_kind
        ~parent_pos
        ~parent_kind
        ~parent_name
        ~parent_is_enum_class
        ~child_pos
        ~child_kind
        ~child_name
        ~child_is_enum_class)

(** Check the proper use of the [extends] keyword between two classes or two interfaces. *)
let check_extend_kinds ctx class_pos shallow_class =
  let class_kind = shallow_class.sc_kind in
  let class_name = snd shallow_class.sc_name in
  List.iter shallow_class.sc_extends ~f:(fun ty ->
      let (_, (parent_pos, parent_name), _) = Decl_utils.unwrap_class_type ty in
      match Shallow_classes_provider.get ctx parent_name with
      | None -> ()
      | Some parent ->
        let parent_is_enum_class = is_enum_class parent.sc_enum_type in
        let class_is_enum_class = is_enum_class shallow_class.sc_enum_type in
        check_extend_kind
          parent_pos
          parent.sc_kind
          (snd parent.sc_name)
          parent_is_enum_class
          class_pos
          class_kind
          class_name
          class_is_enum_class)

let disallow_trait_reuse env =
  TypecheckerOptions.disallow_trait_reuse (Env.get_tcopt env)

(** Check that a class does not reuse a trait in its hierarchy. *)
let check_trait_reuse ctx class_pos cls =
  Cls.linearization cls Decl_defs.Ancestor_types
  |> List.iter ~f:(fun mro ->
         match mro.mro_trait_reuse with
         | None -> ()
         | Some parent_name ->
           let parent_pos =
             Shallow_classes_provider.get ctx parent_name
             |> Option.value_map ~default:Pos_or_decl.none ~f:(fun p ->
                    fst p.sc_name)
           in
           Errors.trait_reuse
             parent_pos
             parent_name
             (class_pos, Cls.name cls)
             mro.mro_name)

let check_class env class_pos cls =
  check_if_cyclic class_pos cls;
  let shallow_class = Cls.shallow_decl cls in
  check_extend_kinds (Env.get_ctx env) class_pos shallow_class;
  if disallow_trait_reuse env then
    check_trait_reuse (Env.get_ctx env) class_pos cls;
  if not Ast_defs.(equal_class_kind (Cls.kind cls) Ctrait) then (
    check_override_annotations env cls ~static:false;
    check_override_annotations env cls ~static:true
  );
  if Ast_defs.(equal_class_kind (Cls.kind cls) Cnormal) then (
    check_trait_override_annotations env class_pos cls ~static:false;
    check_trait_override_annotations env class_pos cls ~static:true
  );
  ()
