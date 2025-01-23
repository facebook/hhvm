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

  TODO(T168350458): remove all uses of types from `Lsp` from this library and instead use internal types
*)

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

let workspace_edit_of_code_action_edits
    (code_action_edit_map : Code_action_types.edits) : Lsp.WorkspaceEdit.t =
  let changes : Lsp.TextEdit.t list Lsp.DocumentUri.Map.t =
    code_action_edit_map
    |> Relative_path.Map.to_seq
    |> Seq.map (fun (path, code_action_edits) ->
           let uri = Lsp_helpers.path_to_lsp_uri path in
           let lsp_edits =
             List.map
               code_action_edits
               ~f:(fun Code_action_types.{ pos; text } ->
                 let range =
                   Lsp_helpers.hack_pos_to_lsp_range
                     ~equal:Relative_path.equal
                     pos
                 in
                 Lsp.TextEdit.{ range; newText = text })
           in
           (uri, lsp_edits))
    |> Lsp.DocumentUri.Map.of_seq
  in
  Lsp.WorkspaceEdit.{ changes }

let edit_to_code_action
    Code_action_types.{ title; edits; selection; trigger_inline_suggest } ~kind
    =
  let workspace_edit = Lazy.map edits ~f:workspace_edit_of_code_action_edits in
  let trigger_inline_suggest_command =
    if trigger_inline_suggest then
      Some Lsp_extra_commands.trigger_inline_suggest
    else
      None
  in
  let set_selection_command =
    match selection with
    | None -> None
    | Some selection ->
      let range =
        Lsp_helpers.hack_pos_to_lsp_range ~equal:Relative_path.equal selection
      in
      Some
        (Lsp_extra_commands.set_selection
           range
           ~command:trigger_inline_suggest_command)
  in
  let command =
    Option.first_some set_selection_command trigger_inline_suggest_command
  in
  let action =
    match command with
    | None -> lazy (Lsp.CodeAction.EditOnly (Lazy.force workspace_edit))
    | Some command ->
      lazy
        (Lsp.CodeAction.BothEditThenCommand (Lazy.force workspace_edit, command))
  in
  Lsp.CodeAction.Action
    {
      Lsp.CodeAction.title;
      kind;
      diagnostics = [];
      action = Lsp.CodeAction.UnresolvedEdit action;
      isAI = None;
    }

let to_action code_action =
  match code_action with
  | Code_action_types.Refactor_action edit ->
    edit_to_code_action edit ~kind:Lsp.CodeActionKind.refactor
  | Code_action_types.Quickfix_action edit ->
    edit_to_code_action edit ~kind:Lsp.CodeActionKind.quickfix

let find
    ~(ctx : Provider_context.t)
    ~error_filter
    ~(entry : Provider_context.entry)
    ~(range : Lsp.range) =
  let pos =
    let source_text = Ast_provider.compute_source_text ~entry in
    let line_to_offset line =
      Full_fidelity_source_text.position_to_offset source_text (line, 0)
    in
    let path = entry.Provider_context.path in
    Lsp_helpers.lsp_range_to_pos ~line_to_offset path range
  in
  let quickfixes = Code_actions_quickfixes.find ~entry pos ctx ~error_filter in
  let (quickfix_titles, quickfixes) =
    List.fold_map
      quickfixes
      ~init:SSet.empty
      ~f:(fun acc (Code_action_types.Quickfix edit) ->
        ( SSet.add edit.Code_action_types.title acc,
          Code_action_types.Quickfix_action edit ))
  in
  (* Accumulate refactors then reverse the entire list so that quickfixes come first *)
  List.rev
  @@ List.fold_left
       (Code_actions_refactors.find ~entry pos ctx)
       ~init:quickfixes
       ~f:(fun acc Code_action_types.(Refactor edit) ->
         (* Ensure no duplicates with quickfixes generated from Quickfixes_to_refactors_config. *)
         if SSet.mem edit.Code_action_types.title quickfix_titles then
           acc
         else
           Code_action_types.Refactor_action edit :: acc)

let map_edit_and_or_command ~f :
    Lsp.CodeAction.resolved_marker Lsp.CodeAction.edit_and_or_command ->
    Lsp.CodeAction.resolved_marker Lsp.CodeAction.edit_and_or_command =
  Lsp.CodeAction.(
    function
    | EditOnly e -> EditOnly (f e)
    | CommandOnly c -> CommandOnly c
    | BothEditThenCommand (e, c) -> BothEditThenCommand (f e, c)
    | UnresolvedEdit _ -> .)

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
    ~error_filter
    ~(entry : Provider_context.entry)
    ~(range : Ide_api_types.range) =
  let strip = update_edit ~f:(fun _ -> Lsp.CodeAction.UnresolvedEdit ()) in
  find ~ctx ~error_filter ~entry ~range:(lsp_range_of_ide_range range)
  |> List.map ~f:to_action
  |> List.map ~f:strip

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

let code_action_title t =
  let open Code_action_types in
  match t with
  | Refactor_action { title; _ }
  | Quickfix_action { title; _ } ->
    title

let resolve
    ~(ctx : Provider_context.t)
    ~error_filter
    ~(entry : Provider_context.entry)
    ~(range : Ide_api_types.range)
    ~(resolve_title : string)
    ~(use_snippet_edits : bool) : Lsp.CodeActionResolve.result =
  let transform_command_or_action :
      Lsp.CodeAction.resolved_marker Lsp.CodeAction.edit_and_or_command Lazy.t
      Lsp.CodeAction.command_or_action_ ->
      Lsp.CodeAction.resolved_command_or_action =
    update_edit ~f:(fun edit_and_or_command ->
        map_edit_and_or_command (Lazy.force edit_and_or_command) ~f:(fun edit ->
            if use_snippet_edits then
              edit
            else
              remove_snippets edit))
  in
  find ~ctx ~entry ~range:(lsp_range_of_ide_range range) ~error_filter
  |> List.find ~f:(fun code_action ->
         String.equal (code_action_title code_action) resolve_title)
  (* When we can't find a matching code action, ContentModified is the right error
     per https://github.com/microsoft/language-server-protocol/issues/1738 *)
  |> Result.of_option ~error:content_modified
  |> Result.map ~f:(fun ca -> transform_command_or_action (to_action ca))
