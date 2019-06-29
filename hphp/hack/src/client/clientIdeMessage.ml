(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Hover_param = struct
  type t = {
    file_path: Path.t;
    file_input: ServerCommandTypes.file_input;
    line: int;
    char: int;
  }
end

(* Handles "textDocument/completion" LSP messages *)
module Lsp_autocomplete = struct
  type request = {
    filename: string;
    line: int;
    column: int;

    (* Contents of the file reflecting unsaved changes in the IDE *)
    file_content: string;

    (* TODO: Remove this variable everywhere *)
    delimit_on_namespaces: bool;
    is_manually_invoked: bool;
  }
  type result = AutocompleteTypes.ide_result
end

module Go_to_definition = struct
  type request = {
    file_path: Path.t;
    file_input: ServerCommandTypes.file_input;
    line: int;
    char: int;
  }
  type result = ServerCommandTypes.Go_to_definition.result
end

(* GADT for request/response types. See [ServerCommandTypes] for a discussion on
   using GADTs in this way. *)
type _ t =
  | Initialize_from_saved_state: Path.t -> unit t
  | Shutdown: unit -> unit t
  | File_changed: Path.t -> unit t
  | Hover:
    Hover_param.t ->
    HoverService.result t
  | Completion:
    Lsp_autocomplete.request ->
    Lsp_autocomplete.result t
  | Go_to_definition:
    Go_to_definition.request ->
    Go_to_definition.result t
