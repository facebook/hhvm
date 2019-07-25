(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Initialize_from_saved_state = struct
  type t = {
    root: Path.t;
    naming_table_saved_state_path: Path.t option;
  }
end

module Hover = struct
  type request = {
    file_path: Path.t;
    file_input: ServerCommandTypes.file_input;
    line: int;
    char: int;
  }
  type result = HoverService.result
end

module Definition = struct
  type request = {
    file_path: Path.t;
    file_input: ServerCommandTypes.file_input;
    line: int;
    char: int;
  }
  type result = ServerCommandTypes.Go_to_definition.result
end

(* Handles "textDocument/completion" LSP messages *)
module Completion = struct
  type request = {
    filename: string;
    line: int;
    column: int;

    (* Contents of the file reflecting unsaved changes in the IDE *)
    file_content: string;
    is_manually_invoked: bool;
  }
  type result = AutocompleteTypes.ide_result
end

(* Handles "completionItem/resolve" LSP messages *)
module Completion_resolve = struct
  type request = {
    symbol: string;
    kind: SearchUtils.si_kind;
  }
  type result = DocblockService.result
end

(* Handles "textDocument/documentHighlight" LSP messages *)
module Document_highlight = struct
  type request = {
    file_path: Path.t;
    file_input: ServerCommandTypes.file_input;
    line: int;
    column: int;
  }
  type result = Ide_api_types.range list
end

(* GADT for request/response types. See [ServerCommandTypes] for a discussion on
   using GADTs in this way. *)
type _ t =
  | Initialize_from_saved_state: Initialize_from_saved_state.t -> unit t
  | Shutdown: unit -> unit t
  | File_changed: Path.t -> unit t
  | Hover:
    Hover.request ->
    Hover.result t
  | Definition:
    Definition.request ->
    Definition.result t
  | Completion:
    Completion.request ->
    Completion.result t
  | Completion_resolve:
    Completion_resolve.request ->
    Completion_resolve.result t
  | Document_highlight:
    Document_highlight.request ->
    Document_highlight.result t
