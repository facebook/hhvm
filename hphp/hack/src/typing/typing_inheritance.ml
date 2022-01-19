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
  List.iter methods ~f:(fun meth ->
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
            List.exists (all_methods_named cls id) ~f:(fun parent_meth ->
                String.( <> ) meth.ce_origin parent_meth.ce_origin)
          in
          if not parent_method_exists then
            Errors.add_typing_error
              Typing_error.(
                assert_in_current_decl ~ctx:(Env.get_current_decl_and_file env)
                @@ Secondary.Should_be_override
                     { pos; class_id = Cls.name cls; id }))

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
  List.iter (methods cls) ~f:(fun (id, meth) ->
      if not (get_ce_override meth) then
        ()
      else if String.equal meth.ce_origin (Cls.name cls) then
        ()
      else
        match Env.get_class env meth.ce_origin with
        | None -> ()
        | Some parent_class ->
          if not Ast_defs.(is_c_trait (Cls.kind parent_class)) then
            ()
          else (
            match meth with
            | { ce_type = (lazy ty); _ } ->
              let parent_method_exists =
                List.exists (all_methods_named cls id) ~f:(fun parent_meth ->
                    String.( <> ) meth.ce_origin parent_meth.ce_origin)
              in
              if not parent_method_exists then
                Errors.add_typing_error
                  Typing_error.(
                    primary
                    @@ Primary.Override_per_trait
                         {
                           pos = class_pos;
                           class_name = Cls.name cls;
                           meth_name = id;
                           trait_name = meth.ce_origin;
                           meth_pos = get_pos ty;
                         })
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
    Errors.add_typing_error
      Typing_error.(
        primary @@ Primary.Cyclic_class_def { stack = classes; pos = class_pos })

let check_extend_kind
    parent_pos parent_kind parent_name child_pos child_kind child_name =
  Ast_defs.(
    match (parent_kind, child_kind) with
    | (Cclass _, Cclass _)
    | (Ctrait, Ctrait)
    | (Cinterface, Cinterface) ->
      ()
    (* enums extend BuiltinEnum under the hood *)
    | (Cclass k, (Cenum | Cenum_class _)) when is_abstract k -> ()
    | (Cenum_class _, Cenum_class _) -> ()
    | ((Cenum | Cenum_class _), (Cenum | Cenum_class _)) ->
      Errors.add_typing_error
        Typing_error.(
          primary
          @@ Primary.Wrong_extend_kind
               {
                 parent_pos;
                 parent_kind;
                 parent_name;
                 pos = child_pos;
                 kind = child_kind;
                 name = child_name;
               })
    | _ ->
      Errors.add_typing_error
        Typing_error.(
          primary
          @@ Primary.Wrong_extend_kind
               {
                 parent_pos;
                 parent_kind;
                 parent_name;
                 pos = child_pos;
                 kind = child_kind;
                 name = child_name;
               }))

(** Check that the [extends] keyword is between two classes or two interfaces, but not
    between a class and an interface or vice-versa. *)
let check_extend_kinds ctx class_pos shallow_class =
  let classish_kind = shallow_class.sc_kind in
  let class_name = snd shallow_class.sc_name in
  List.iter shallow_class.sc_extends ~f:(fun ty ->
      let (_, (parent_pos, parent_name), _) = Decl_utils.unwrap_class_type ty in
      match Shallow_classes_provider.get ctx parent_name with
      | None -> ()
      | Some parent ->
        check_extend_kind
          parent_pos
          parent.sc_kind
          (snd parent.sc_name)
          class_pos
          classish_kind
          class_name)

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
           Errors.add_typing_error
             Typing_error.(
               primary
               @@ Primary.Trait_reuse
                    {
                      parent_pos;
                      parent_name;
                      pos = class_pos;
                      class_name = Cls.name cls;
                      trait_name = mro.mro_name;
                    }))

let check_class env class_pos cls =
  check_if_cyclic class_pos cls;
  let shallow_class = Cls.shallow_decl cls in
  check_extend_kinds (Env.get_ctx env) class_pos shallow_class;
  if disallow_trait_reuse env then
    check_trait_reuse (Env.get_ctx env) class_pos cls;
  if not Ast_defs.(is_c_trait (Cls.kind cls)) then (
    check_override_annotations env cls ~static:false;
    check_override_annotations env cls ~static:true
  );
  if Ast_defs.is_c_normal (Cls.kind cls) then (
    check_trait_override_annotations env class_pos cls ~static:false;
    check_trait_override_annotations env class_pos cls ~static:true
  );
  ()
