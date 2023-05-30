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
    Lsp.CodeAction.resolvable_command_or_action =
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
  CodeAction.Action
    {
      CodeAction.title = Quickfix.get_title quickfix;
      kind = CodeActionKind.quickfix;
      (* We can tell LSP which error this fixed, but we'd have to
         recompute the diagnostic from the error and there's no clear
         benefit. *)
      diagnostics = [];
      action = CodeAction.UnresolvedEdit edit;
    }

let actions_for_errors
    (errors : Errors.t)
    (path : Relative_path.t)
    (classish_starts : Pos.t SMap.t)
    ~(start_line : int)
    ~(start_col : int) : Lsp.CodeAction.resolvable_command_or_action list =
  let errors = Errors.get_error_list ~drop_fixmed:false errors in
  let errors_here =
    List.filter errors ~f:(fun e ->
        let e_pos = User_error.get_pos e in
        Pos.inside e_pos start_line start_col)
  in
  let quickfixes =
    List.map ~f:User_error.quickfixes errors_here |> List.concat
  in
  List.map quickfixes ~f:(fun qf -> fix_action path classish_starts qf)

let find
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(path : Relative_path.t)
    ~(range : Ide_api_types.range) :
    Lsp.CodeAction.resolvable_command_or_action list =
  let open Ide_api_types in
  let start_line = range.st.line in
  let start_col = range.st.column in
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

  actions_for_errors errors path classish_starts ~start_line ~start_col
