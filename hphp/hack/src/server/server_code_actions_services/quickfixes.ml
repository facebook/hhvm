(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let text_edits (classish_starts : Pos.t SMap.t) (quickfix : Pos.t Quickfix.t) :
    Code_action_types.edit list =
  let edits = Quickfix.get_edits ~classish_starts quickfix in
  List.map edits ~f:(fun (text, pos) -> Code_action_types.{ pos; text })

let convert_quickfix
    path (classish_starts : Pos.t SMap.t) (quickfix : Pos.t Quickfix.t) :
    Code_action_types.Quickfix.t =
  let edits =
    lazy
      (Relative_path.Map.singleton path (text_edits classish_starts quickfix))
  in

  Code_action_types.Quickfix.{ title = Quickfix.get_title quickfix; edits }

let errors_to_quickfixes
    (errors : Errors.t)
    (path : Relative_path.t)
    (classish_starts : Pos.t SMap.t)
    (selection : Pos.t) : Code_action_types.Quickfix.t list =
  let errors = Errors.get_error_list ~drop_fixmed:false errors in
  let errors_here =
    List.filter errors ~f:(fun e ->
        let e_pos = User_error.get_pos e in
        Pos.contains selection e_pos || Pos.contains e_pos selection)
  in
  let quickfixes = List.bind ~f:User_error.quickfixes errors_here in
  List.map quickfixes ~f:(convert_quickfix path classish_starts)

let find ~ctx ~entry pos : Code_action_types.Quickfix.t list =
  let cst = Ast_provider.compute_cst ~ctx ~entry in
  let tree = Provider_context.PositionedSyntaxTree.root cst in

  let classish_starts =
    match entry.Provider_context.source_text with
    | Some source_text ->
      Quickfix_ffp.classish_starts tree source_text entry.Provider_context.path
    | None -> SMap.empty
  in

  let { Tast_provider.Compute_tast_and_errors.errors; _ } =
    Tast_provider.compute_tast_and_errors_quarantined ~ctx ~entry
  in
  let path = entry.Provider_context.path in
  errors_to_quickfixes errors path classish_starts pos
