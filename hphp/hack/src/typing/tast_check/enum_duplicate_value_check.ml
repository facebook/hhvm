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

(* Cap on how many reasons we render in an "uncheckable" error. An enum that
   `use`s many uncheckable enums could otherwise produce an unbounded list. *)
let max_reasons = 5

(* The enum can't be checked as a whole (one of its own values is unevaluable, or
   it `use`s an enum that opted out), so emit a single Enum_uncheckable_value and
   do NOT report duplicates -- a partial report would give a false sense of
   safety. The human-readable reasons are built lazily only if the error is
   rendered. [reasons] pairs the (capped) list of reasons with a count of how
   many further reasons were omitted. *)
let report_uncheckable_enum
    env
    ~enum_pos
    ~enum_decl_name
    ~is_own_uncheckable
    ~is_from_opted_out_origin
    consts =
  let reasons =
    lazy
      (let own =
         List.filter_map consts ~f:(fun (member_name, cc) ->
             Option.some_if
               (is_own_uncheckable cc)
               (`Own (cc.cc_pos, member_name)))
       in
       let used =
         List.filter_map consts ~f:(fun (_, cc) ->
             Option.some_if (is_from_opted_out_origin cc) cc.cc_origin)
         |> List.dedup_and_sort ~compare:String.compare
         |> List.map ~f:(fun origin -> `Used origin)
       in
       let all = own @ used in
       let render = function
         | `Own (pos, member_name) ->
           (pos, Printf.sprintf "`%s` cannot be checked" member_name)
         | `Used origin ->
           (* Point at the used enum's own declaration, not the enum being
              checked, so the reason directs the reader to the enum they need to
              fix or opt out. *)
           let origin_pos =
             match Env.get_class env origin with
             | Decl_entry.Found ocls -> Cls.pos ocls
             | Decl_entry.DoesNotExist
             | Decl_entry.NotYetAvailable ->
               Pos_or_decl.of_raw_pos enum_pos
           in
           ( origin_pos,
             Printf.sprintf
               "it uses `%s`, which cannot be checked for duplicate values"
               (Utils.strip_ns origin) )
       in
       ( List.map (List.take all max_reasons) ~f:render,
         Int.max 0 (List.length all - max_reasons) ))
  in
  Env.add_typing_error
    ~env
    Typing_error.(
      primary
      @@ Primary.Enum_uncheckable_value
           { pos = enum_pos; name = enum_decl_name; reasons })

(* The enum is fully checkable: flag duplicate values. The folded consts merge
   `use`-included members, so one pass catches own, included, and cross-include
   clashes. Maps each value to the first member that defines it; a later member
   with the same value is a clash. *)
let report_duplicate_values env ~enum_pos ~is_local consts =
  let seen : (string, string * class_const) Hashtbl.t =
    String.Table.create ()
  in
  List.iter consts ~f:(fun (member_name, cc) ->
      if Enum_member_value.is_absent cc.cc_enum_value then
        ()
      else
        let value =
          Enum_member_value.value_repr ~member_name cc.cc_enum_value
        in
        match Hashtbl.find seen value with
        | None -> Hashtbl.set seen ~key:value ~data:(member_name, cc)
        | Some (prev_name, prev_cc) ->
          (* A clash between two members inherited from the *same* enum is
             reported when that enum is checked, not again on every enum that
             uses it. *)
          let both_inherited_same_origin =
            (not (is_local cc))
            && (not (is_local prev_cc))
            && String.equal cc.cc_origin prev_cc.cc_origin
          in
          if not both_inherited_same_origin then
            Env.add_typing_error
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
        (* An enum carrying the opt-out attribute is exempt from the whole check
           (read from the decl, so included enums are visible below); an opted-out
           enum falls through to the no-op arm below. *)
        | Decl_entry.Found cls when not (Cls.allow_unchecked_enum_values cls) ->
          let enum_decl_name = Cls.name cls in
          let is_local cc = String.equal cc.cc_origin enum_decl_name in
          let consts = Env.consts env cls in
          (* Memoized opt-out check for an included enum. *)
          let origin_opts_out =
            let cache : (string, bool) Hashtbl.t = String.Table.create () in
            fun origin ->
              Hashtbl.find_or_add cache origin ~default:(fun () ->
                  match Env.get_class env origin with
                  | Decl_entry.Found ocls ->
                    Cls.allow_unchecked_enum_values ocls
                  | Decl_entry.DoesNotExist
                  | Decl_entry.NotYetAvailable ->
                    false)
          in
          (* A local member whose value can't be statically evaluated. *)
          let is_own_uncheckable cc =
            is_local cc
            && (not cc.cc_synthesized)
            && Enum_member_value.is_absent cc.cc_enum_value
          in
          (* A member inherited from an enum that opted out of the check. An
             inherited enum whose own values are uncheckable is flagged on its
             own definition and, once it opts out, is caught here -- so the
             opt-out check alone is sufficient for inherited members. *)
          let is_from_opted_out_origin cc =
            (not (is_local cc)) && origin_opts_out cc.cc_origin
          in
          let uncheckable =
            List.exists consts ~f:(fun (_, cc) ->
                is_own_uncheckable cc || is_from_opted_out_origin cc)
          in
          if uncheckable then
            report_uncheckable_enum
              env
              ~enum_pos
              ~enum_decl_name
              ~is_own_uncheckable
              ~is_from_opted_out_origin
              consts
          else
            report_duplicate_values env ~enum_pos ~is_local consts
        | Decl_entry.Found _ -> ()
  end
