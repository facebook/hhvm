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

type document_location = {
  file_path: Path.t;
  file_contents: string option; (* if absent, should read from file on disk *)
  line: int;
  column: int;
} [@@deriving show]
(** Denotes a location of the cursor in a document at which an IDE request is
being executed (e.g. hover). *)

module Hover = struct
  type request = document_location
  type result = HoverService.result
end

module Definition = struct
  type request = document_location
  type result = ServerCommandTypes.Go_to_definition.result
end

(* Handles "textDocument/completion" LSP messages *)
module Completion = struct
  type request = {
    document_location: document_location;
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
  type request = document_location
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

type notification =
  | Done_processing

type message_from_daemon =
  | Notification of notification
  | Response: ('a, string) result -> message_from_daemon
