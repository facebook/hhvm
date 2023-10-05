(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let to_range (pos : Pos.t) : Lsp.range =
  let (first_line, first_col) = Pos.line_column pos in
  let (last_line, last_col) = Pos.end_line_column pos in
  {
    Lsp.start = { Lsp.line = first_line - 1; character = first_col };
    end_ = { Lsp.line = last_line - 1; character = last_col };
  }

let text_edits (classish_starts : Pos.t SMap.t) (quickfix : Pos.t Quickfix.t) :
    Lsp.TextEdit.t list =
  let edits = Quickfix.get_edits ~classish_starts quickfix in
  List.map edits ~f:(fun (new_text, pos) ->
      { Lsp.TextEdit.range = to_range pos; newText = new_text })

let fix_action
    path (classish_starts : Pos.t SMap.t) (quickfix : Pos.t Quickfix.t) :
    Code_action_types.Quickfix.t =
  let open Lsp in
  let edit =
    lazy
      (let changes =
         SMap.singleton
           (Relative_path.to_absolute path)
           (text_edits classish_starts quickfix)
       in
       WorkspaceEdit.{ changes })
  in
  Code_action_types.Quickfix.{ title = Quickfix.get_title quickfix; edit }

let actions_for_errors
    (errors : Errors.t)
    (path : Relative_path.t)
    (classish_starts : Pos.t SMap.t)
    (selection : Pos.t) : Code_action_types.Quickfix.t list =
  let errors = Errors.get_error_list ~drop_fixmed:false errors in
  let errors_here =
    List.filter errors ~f:(fun e ->
        let e_pos = User_error.get_pos e in
        Pos.contains selection e_pos)
  in
  let quickfixes =
    List.map ~f:User_error.quickfixes errors_here |> List.concat
  in
  List.map quickfixes ~f:(fun qf -> fix_action path classish_starts qf)

let find ~ctx ~entry ~(range : Lsp.range) : Code_action_types.Quickfix.t list =
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
  let selection =
    let source_text = Ast_provider.compute_source_text ~entry in
    let line_to_offset line =
      Full_fidelity_source_text.position_to_offset source_text (line, 0)
    in
    Lsp_helpers.lsp_range_to_pos ~line_to_offset path range
  in
  actions_for_errors errors path classish_starts selection
