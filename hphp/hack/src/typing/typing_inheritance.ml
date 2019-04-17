(**
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
open Shallow_decl_defs
open Typing_defs

module Cls = Typing_classes_heap
module Env = Typing_env

(** Ensure that all methods which have the override annotation and were
    shallowly defined within [cls] do in fact override some inherited
    definition. *)
let check_override_annotations cls ~static =
  let methods =
    let sc = Cls.shallow_decl cls in
    if static then sc.sc_static_methods else sc.sc_methods
  in
  List.iter methods begin fun meth ->
    if not meth.sm_override then () else
    let get_method, all_methods_named =
      if static
      then Cls.get_smethod, Cls.all_inherited_smethods
      else Cls.get_method, Cls.all_inherited_methods
    in
    let pos, id = meth.sm_name in
    match get_method cls id with
    | None -> ()
    | Some meth ->
      let parent_method_exists =
        List.exists (all_methods_named cls id) begin fun parent_meth ->
          meth.ce_origin <> parent_meth.ce_origin
        end
      in
      if not parent_method_exists
      then Errors.should_be_override pos (Cls.name cls) id
  end

(** Ensure that all methods which have the override annotation, were inherited
    by [cls], and were originally defined in a trait do in fact override some
    other inherited definition. *)
let check_trait_override_annotations env cls ~static =
  let methods, all_methods_named =
    if static
    then Cls.smethods, Cls.all_inherited_smethods
    else Cls.methods, Cls.all_inherited_methods
  in
  Sequence.iter (methods cls) begin fun (id, meth) ->
    if not meth.ce_override then () else
    if meth.ce_origin = Cls.name cls then () else
    match Env.get_class env meth.ce_origin with
    | None -> ()
    | Some parent_class ->
      if Cls.kind parent_class <> Ast.Ctrait then () else
      match meth with
      | { ce_type = lazy (_, Tfun { ft_pos; _ }); _ } ->
        let parent_method_exists =
          List.exists (all_methods_named cls id) begin fun parent_meth ->
            meth.ce_origin <> parent_meth.ce_origin
          end
        in
        if not parent_method_exists
        then Errors.override_per_trait (Cls.pos cls, Cls.name cls) id ft_pos
      | _ -> ()
  end

let check_class env cls =
  if Cls.kind cls <> Ast.Ctrait then begin
    check_override_annotations cls ~static:false;
    check_override_annotations cls ~static:true;
  end;
  if Cls.kind cls = Ast.Cnormal then begin
    check_trait_override_annotations env cls ~static:false;
    check_trait_override_annotations env cls ~static:true;
  end;
  ()
