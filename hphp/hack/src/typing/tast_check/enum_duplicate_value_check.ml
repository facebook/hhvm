(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
module Env = Tast_env
module Cls = Folded_class

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_class_ env c =
      if Ast_defs.is_c_enum c.c_kind then
        let (enum_pos, enum_name) = c.c_name in
        match Env.get_class env enum_name with
        | Decl_entry.DoesNotExist
        | Decl_entry.NotYetAvailable ->
          ()
        | Decl_entry.Found cls ->
          (* The folded consts merge `use`-included members, so one pass over
             them catches own, included, and cross-include clashes. The error is
             always anchored at the enum itself, with both clashing members as
             secondary positions. *)
          let enum_decl_name = Cls.name cls in
          let is_local cc = String.equal cc.cc_origin enum_decl_name in
          (* Maps each value to the first member that defines it; a later member
             with the same value is a clash. *)
          let seen : (string, string * class_const) Hashtbl.t =
            String.Table.create ()
          in
          List.iter (Tast_env.consts env cls) ~f:(fun (member_name, cc) ->
              if Enum_member_value.is_absent cc.cc_enum_value then
                ()
              else
                let value =
                  Enum_member_value.value_repr ~member_name cc.cc_enum_value
                in
                match Hashtbl.find seen value with
                | None -> Hashtbl.set seen ~key:value ~data:(member_name, cc)
                | Some (prev_name, prev_cc) ->
                  (* Skip a clash between two members inherited from the *same*
                     included enum: it is reported when that enum is checked, not
                     again on every enum that uses it. *)
                  let both_inherited_same_origin =
                    (not (is_local cc))
                    && (not (is_local prev_cc))
                    && String.equal cc.cc_origin prev_cc.cc_origin
                  in
                  if not both_inherited_same_origin then
                    Tast_env.add_typing_error
                      ~env
                      Typing_error.(
                        primary
                        @@ Primary.Enum_duplicate_value
                             {
                               pos = enum_pos;
                               value;
                               member_name;
                               member_pos = cc.cc_pos;
                               prev_name;
                               prev_pos = prev_cc.cc_pos;
                             }))
  end
