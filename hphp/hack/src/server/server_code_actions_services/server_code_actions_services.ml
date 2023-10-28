(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(**
  For LSP "textDocument/codeAction" response, we do not compute the edit for the action.
  For LSP "codeAction/resolve" response, we compute the edit.
  We never use commands on the server side of the code action flow: afaict that's a legacy technique
  from before "codeAction/resolve" was introduced.

  See [CodeAction.edit_or_command] in lsp.ml for more on the code action flow.
*)
type resolvable_command_or_action =
  Lsp.WorkspaceEdit.t Lazy.t Lsp.CodeAction.command_or_action_

let lsp_range_of_ide_range (ide_range : Ide_api_types.range) : Lsp.range =
  let lsp_pos_of_ide_pos ide_pos =
    Lsp.
      {
        line = ide_pos.Ide_api_types.line;
        character = ide_pos.Ide_api_types.column;
      }
  in
  Lsp.
    {
      start = lsp_pos_of_ide_pos ide_range.Ide_api_types.st;
      end_ = lsp_pos_of_ide_pos ide_range.Ide_api_types.ed;
    }

(* Example: in "${0:placeholder}" extracts "placeholder" *)
let snippet_regexp = Str.regexp {|\${[0-9]+:\([^}]*\)}|}

let remove_snippets Lsp.WorkspaceEdit.{ changes } =
  let un_snippet_string = Str.global_replace snippet_regexp {|\1|} in
  let un_snippet_text_edit text_edit =
    Lsp.TextEdit.
      { text_edit with newText = un_snippet_string text_edit.newText }
  in
  let changes =
    Lsp.DocumentUri.Map.map (List.map ~f:un_snippet_text_edit) changes
  in
  Lsp.WorkspaceEdit.{ changes }

let find
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(range : Lsp.range) : resolvable_command_or_action list =
  let to_action ~title ~edit ~kind =
    Lsp.CodeAction.Action
      {
        Lsp.CodeAction.title;
        kind;
        diagnostics = [];
        action = Lsp.CodeAction.UnresolvedEdit edit;
      }
  in
  let quickfixes =
    Quickfixes.find ~ctx ~entry ~range
    |> List.map ~f:(fun Code_action_types.Quickfix.{ title; edit } ->
           to_action ~title ~edit ~kind:Lsp.CodeActionKind.quickfix)
  in
  let refactors =
    Refactors.find ~entry ~range ctx
    |> List.map ~f:(fun Code_action_types.Refactor.{ title; edit } ->
           to_action ~title ~edit ~kind:Lsp.CodeActionKind.refactor)
  in
  quickfixes @ refactors

let update_edit ~f =
  Lsp.CodeAction.(
    function
    | Command _ as c -> c
    | Action ({ action; _ } as a) ->
      let action =
        match action with
        (* Currently only [UnresolvedEdit] is used, since all code actions involve lazy edits *)
        | UnresolvedEdit lazy_edit -> f lazy_edit
        | EditOnly e -> EditOnly e
        | CommandOnly c -> CommandOnly c
        | BothEditThenCommand ca -> BothEditThenCommand ca
      in
      Action { a with action })

let go
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(range : Ide_api_types.range) =
  let strip = update_edit ~f:(fun _ -> Lsp.CodeAction.UnresolvedEdit ()) in
  find ~ctx ~entry ~range:(lsp_range_of_ide_range range) |> List.map ~f:strip

let content_modified =
  Lsp.Error.
    {
      code = ContentModified;
      message =
        {|Expected the code action requested with codeAction/resolve to be findable.
Note: This error message may be caused by the source text changing between
when the code action menu pops up and when the user selects the code action.
In such cases we may not be able to find a code action at the same location with
the same title, so cannot resolve the code action.
        |};
      data = None;
    }

let resolve
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(range : Ide_api_types.range)
    ~(resolve_title : string)
    ~(use_snippet_edits : bool) : Lsp.CodeActionResolve.result =
  let transform_command_or_action :
      Lsp.WorkspaceEdit.t Lazy.t Lsp.CodeAction.command_or_action_ ->
      Lsp.CodeAction.resolved_command_or_action =
    update_edit ~f:(fun lazy_edit ->
        let edit = Lazy.force lazy_edit in
        let edit =
          if use_snippet_edits then
            edit
          else
            remove_snippets edit
        in
        Lsp.CodeAction.EditOnly edit)
  in
  find ~ctx ~entry ~range:(lsp_range_of_ide_range range)
  |> List.find ~f:(fun command_or_action ->
         let title = Lsp_helpers.title_of_command_or_action command_or_action in
         String.equal title resolve_title)
  (* When we can't find a matching code action, ContentModified is the right error
     per https://github.com/microsoft/language-server-protocol/issues/1738 *)
  |> Result.of_option ~error:content_modified
  |> Result.map ~f:transform_command_or_action
