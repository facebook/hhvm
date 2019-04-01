(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Base
open Nast

let handler = object
  inherit Nast_visitor.handler_base

  method! at_class_ _env c =
    (* generic test *)
    let check err seen name =
      if SSet.mem name seen
      then (
        err name;
        seen
      ) else SSet.add name seen in
    (* Check that `case type` declaration have no duplicates *)
    let check_case_types seen (p, name) =
      let err name = Errors.pu_duplication p name "case type" in
      check err seen name in
    (* Check that `case` declaration have no duplicates *)
    let check_case_expr seen ((p, name), _) =
      let err name = Errors.pu_duplication p name "case expression" in
      check err seen name in
    (* In a member definition, check that type/expr have no duplicates *)
    let check_member_item kind seen ((p, name), _) =
      let err name = Errors.pu_duplication p name kind in
      check err seen name in
    (* Check that member declarations have no duplicates and are well-formed *)
    let check_member seen member =
      let (p, name) = member.pum_atom in
      let err name = Errors.pu_duplication p name "member" in
      let _ = List.fold ~init:SSet.empty
          ~f:(check_member_item (name ^ " member type")) member.pum_types in
      let _ = List.fold ~init:SSet.empty
          ~f:(check_member_item (name ^ " member expr")) member.pum_exprs in
      check err seen name in
    let check_enums seen enum  =
      let (p, name) = enum.pu_name in
      let err name = Errors.pu_duplication p name "enum" in
      let _ = List.fold ~init:SSet.empty ~f:check_case_types enum.pu_case_types in
      let _ = List.fold ~init:SSet.empty ~f:check_case_expr enum.pu_case_values in
      let _ = List.fold ~init:SSet.empty ~f:check_member enum.pu_members in
      check err seen name in
    (* check that all enums have distinct names *)
    let _ = List.fold ~init:SSet.empty ~f:check_enums c.c_pu_enums in ()

end
