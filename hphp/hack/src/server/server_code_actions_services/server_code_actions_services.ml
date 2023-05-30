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

let find
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(path : Relative_path.t)
    ~(range : Ide_api_types.range) : resolvable_command_or_action list =
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
    Quickfixes.find ~ctx ~entry ~path ~range
    |> List.map ~f:(fun Code_action_types.Quickfix.{ title; edit } ->
           to_action ~title ~edit ~kind:Lsp.CodeActionKind.quickfix)
  in
  let refactors =
    Refactors.find ~ctx ~entry ~path ~range
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
    ~(path : Relative_path.t)
    ~(range : Ide_api_types.range) : Lsp.CodeAction.command_or_action list =
  let strip = update_edit ~f:(fun _ -> Lsp.CodeAction.UnresolvedEdit ()) in
  find ~ctx ~entry ~path ~range |> List.map ~f:strip

let resolve
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(path : Relative_path.t)
    ~(range : Ide_api_types.range)
    ~(resolve_title : string) : Lsp.CodeAction.resolved_command_or_action =
  let resolve_command_or_action =
    update_edit ~f:(fun lazy_edit ->
        Lsp.CodeAction.EditOnly (Lazy.force lazy_edit))
  in

  find ~ctx ~entry ~path ~range
  |> List.find ~f:(fun command_or_action ->
         let title = Lsp_helpers.title_of_command_or_action command_or_action in
         String.equal title resolve_title)
  (* TODO(T153638678): better error handling, see also https://github.com/microsoft/language-server-protocol/issues/1738 *)
  |> Option.value_exn
       ~message:
         {|Expected the code action requested with codeAction/resolve to be findable.
Note: This error message may be caused by the source text changing between
when the code action menu pops up and when the user selects the code action.
In such cases we may not be able to find a code action at the same location with
the same title, so cannot resolve the code action.
|}
  |> resolve_command_or_action
