(*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

let quickfix_action path (quickfix : Pos.t Errors.quickfix) :
    Lsp.CodeAction.command_or_action =
  let open Lsp in
  let text_edit =
    {
      TextEdit.range = to_range quickfix.Errors.pos;
      newText = quickfix.Errors.new_text;
    }
  in
  let action =
    {
      CodeAction.title = quickfix.Errors.title;
      kind = CodeActionKind.quickfix;
      (* We can tell LSP which error this fixed, but we'd have to
         recompute the diagnostic from the error and there's no clear
         benefit. *)
      diagnostics = [];
      action =
        CodeAction.EditOnly
          WorkspaceEdit.{ changes = SMap.singleton path [text_edit] };
    }
  in
  CodeAction.Action action

let actions_for_errors
    (errors : Errors.t) (path : string) ~(start_line : int) ~(start_col : int) :
    Lsp.CodeAction.command_or_action list =
  let errors = Errors.get_error_list errors in
  let errors_here =
    List.filter errors ~f:(fun e ->
        let e_pos = Errors.get_pos e in
        Pos.inside e_pos start_line start_col)
  in
  let quickfixes = List.map ~f:Errors.quickfixes errors_here |> List.concat in
  List.map quickfixes ~f:(fun qf -> quickfix_action path qf)

let go
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(path : string)
    ~(range : Ide_api_types.range) : Lsp.CodeAction.command_or_action list =
  let open Ide_api_types in
  let start_line = range.st.line in
  let start_col = range.st.column in
  let { Tast_provider.Compute_tast_and_errors.errors; _ } =
    Tast_provider.compute_tast_and_errors_quarantined ~ctx ~entry
  in
  actions_for_errors errors path ~start_line ~start_col
