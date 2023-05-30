(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let find
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(path : Relative_path.t)
    ~(range : Ide_api_types.range) :
    Lsp.CodeAction.resolvable_command_or_action list =
  let quickfixes = Quickfixes.find ~ctx ~entry ~path ~range in
  let refactors = Refactors.find ~ctx ~entry ~path ~range in
  quickfixes @ refactors

let go
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(path : Relative_path.t)
    ~(range : Ide_api_types.range) : Lsp.CodeAction.command_or_action list =
  let open Lsp.CodeAction in
  let strip : resolvable_command_or_action -> command_or_action = function
    | Command _ as c -> c
    | Action ({ action; _ } as a) ->
      let action =
        match action with
        | UnresolvedEdit _ -> UnresolvedEdit ()
        | EditOnly e -> EditOnly e
        | CommandOnly c -> CommandOnly c
        | BothEditThenCommand ca -> BothEditThenCommand ca
      in
      Action { a with action }
  in

  find ~ctx ~entry ~path ~range |> List.map ~f:strip

let resolve
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(path : Relative_path.t)
    ~(range : Ide_api_types.range)
    ~(resolve_title : string) : Lsp.CodeAction.resolved_command_or_action =
  let open Lsp.CodeAction in
  let resolve_command_or_action :
      resolvable_command_or_action -> resolved_command_or_action = function
    | Command _ as c -> c
    | Action ({ action; _ } as a) ->
      let action =
        match action with
        | UnresolvedEdit lazy_edit -> EditOnly (Lazy.force lazy_edit)
        | EditOnly e -> EditOnly e
        | CommandOnly c -> CommandOnly c
        | BothEditThenCommand ca -> BothEditThenCommand ca
      in
      Action { a with action }
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
