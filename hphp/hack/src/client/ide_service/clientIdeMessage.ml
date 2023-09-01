(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Initialize_from_saved_state = struct
  type t = {
    root: Path.t;
    naming_table_load_info: naming_table_load_info option;
    config: (string * string) list;
    ignore_hh_version: bool;
    open_files: Path.t list;
  }

  and naming_table_load_info = {
    path: Path.t;
    test_delay: float;  (** artificial delay in seconds, for test purposes *)
  }
end

module Completion_resolve = struct
  type request = {
    fullname: string;
    kind: SearchTypes.si_kind;
  }

  type result = {
    docblock: DocblockService.result;
    signature: string option;
  }
end

type document = {
  file_path: Path.t;
  file_contents: string;
}

type location = {
  line: int;
  column: int;
}

type find_refs_result =
  | Invalid_symbol
  | Find_refs_success of {
      full_name: string;  (** from [SymbolDefinition.full_name] *)
      action: ServerCommandTypes.Find_refs.action option;
          (** if true, then clientLsp should shell out to hh_server to collect more positions;
          this action will specifiy what hh_server shoud look for. *)
      hint_suffixes: string list;
          (** in case of a shell-out, we will suggest hh_server to look in these root-relative paths first. *)
      open_file_results: (string * Pos.absolute) list Lsp.UriMap.t;
          (** All references that were found in all open files in clientIdeDaemon. *)
    }

type rename_result =
  | Not_renameable_position
  | Rename_success of {
      shellout:
        (Relative_path.t SymbolDefinition.t
        * ServerCommandTypes.Find_refs.action)
        option;
      local: ServerRenameTypes.patch list;
    }

type go_to_impl_result =
  | Invalid_symbol_impl
  | Go_to_impl_success of
      (string
      * ServerCommandTypes.Find_refs.action
      * Pos.absolute list Lsp.UriMap.t)

type completion_request = { is_manually_invoked: bool }

type should_calculate_errors = { should_calculate_errors: bool }

(* GADT for request/response types. See [ServerCommandTypes] for a discussion on
   using GADTs in this way. *)
type _ t =
  | Initialize_from_saved_state : Initialize_from_saved_state.t -> unit t
      (** Invariant: the client sends no messages to the daemon before
      Initialize_from_state. And the daemon sends no messages before
      it has responded. *)
  | Shutdown : unit -> unit t
  | Did_change_watched_files : Relative_path.Set.t -> unit t
      (** This might include deleted files. The caller is responsible for filtering,
      and resolving symlinks (even resolving root symlink for a deleted file...) *)
  | Did_open_or_change :
      document * should_calculate_errors
      -> Errors.finalized_error list option t
      (** if the bool is true, it will send back an error-list;
      if false, it will return None. *)
  | Did_close : Path.t -> Errors.finalized_error list t
      (** This returns diagnostics for the file as it is on disk.
      This is to serve the following scenario: (1) file was open with
      modified contents and squiggles appropriate to the modified contents,
      (2) user closes file without saving. In this scenario we must
      restore squiggles to what would be appropriate for the file on disk.

      It'd be possible to return an [Errors.t option], and only return [Some]
      if the file had been closed while modified, if perf here is ever a concern. *)
  | Verbose_to_file : bool -> unit t
  | Hover : document * location -> HoverService.result t
  | Definition :
      document * location
      -> ServerCommandTypes.Go_to_definition.result t
  | Completion :
      document * location * completion_request
      -> AutocompleteTypes.ide_result t
      (** Handles "textDocument/completion" LSP messages *)
  | Completion_resolve_location :
      Path.t * location * SearchTypes.si_kind
      -> Completion_resolve.result t
      (** "completionItem/resolve" LSP messages - if we have file/line/column.
      The scenario is that VSCode requests textDocument/completion in A.PHP line 5 col 6,
      and we responded with a completion item that points to a class defined in B.PHP line 10 col 5,
      and now VSCode requests completionItem/resolve [(B.PHP, {line=10; column=5;}, class)],
      and this method will look up B.PHP to find the docblock for that class and return it.
      Typically B.PHP won't even be open. That's why we only provide it as Path.t *)
  | Completion_resolve :
      Completion_resolve.request
      -> Completion_resolve.result t
      (** "completionItem/resolve" LSP messages - if we have symbol name, and [Completion_resolve_location] failed *)
  | Document_highlight : document * location -> Ide_api_types.range list t
      (** Handles "textDocument/documentHighlight" LSP messages *)
  | Document_symbol : document -> FileOutline.outline t
      (** Handles "textDocument/documentSymbol" LSP messages *)
  | Workspace_symbol : string -> SearchUtils.result t
  | Go_to_implementation :
      document * location * document list
      -> go_to_impl_result t
  | Find_references : document * location * document list -> find_refs_result t
  | Rename : document * location * string * document list -> rename_result t
      (** The result of Rename is one of:
       - Not_renameable_position, indicating an attempt to rename something that isn't a valid symbol
       - Rename_success, where we return a record containing two fields:
           - [shellout], an optional tuple of (SymbolDefinition.full_name * Find_refs.action)
             to indicate that ClientIdeDaemon could not satisfy all of the rename (the non-localvar case)
           - [local], a [ServerRenameTypes.patch list], a list of rename patches for each open file supplied
            in the input document list
          *)
  | Type_definition :
      document * location
      -> ServerCommandTypes.Go_to_type_definition.result t
      (** Handles "textDocument/typeDefinition" LSP messages *)
  | Signature_help : document * location -> Lsp.SignatureHelp.result t
      (** Handles "textDocument/signatureHelp" LSP messages *)
  | Code_action :
      document * Ide_api_types.range
      -> (Lsp.CodeAction.command_or_action list
         * Errors.finalized_error list option)
         t
      (** Handles "textDocument/codeActions" LSP messages.

      Also, we take this as a handy opportunity to send updated [Errors.t]
      to clientLsp so it can publish them. We return [None] if the TAST+errors
      haven't changed since they were last sent to clientLsp (through didOpen,
      didChange, didClose or an earlier codeAction), and [Some] if they have
      been changed.

      Why send errors in code-action? It's for the scenario where file A.PHP
      is open, file B.PHP changes on disk (e.g. due to user saving it), which
      invalidates the TAST+errors of A.PHP, and we might therefore need to send
      updated errors. For efficiency we chose to do this lazily when the user
      interacts with A.PHP or switches tab to it, rather than doing every single
      open file up-front. We could have sent updated errors on every single action,
      e.g. hover and go-to-def. But settled for now on only doing it for codeAction
      because this is called so frequently by VSCode (when you switch tab, and every
      time the caret moves) in the hope that this will be a good balance of simple
      code and decent experience. *)
  | Code_action_resolve : {
      document: document;
      range: Ide_api_types.range;
      resolve_title: string;
      use_snippet_edits: bool;
    }
      -> Lsp.CodeActionResolve.result t
  | Type_Hierarchy : document * location -> ServerTypeHierarchyTypes.result t
      (** Handles "textDocument/typeHierarchy" LSP messages *)

let t_to_string : type a. a t -> string = function
  | Initialize_from_saved_state _ -> "Initialize_from_saved_state"
  | Shutdown () -> "Shutdown"
  | Did_change_watched_files paths ->
    let paths =
      paths |> Relative_path.Set.elements |> List.map ~f:Relative_path.suffix
    in
    let (files, remainder) = List.split_n paths 10 in
    let remainder =
      if List.is_empty remainder then
        ""
      else
        ",..."
    in
    Printf.sprintf
      "Did_change_watched_files(%s%s)"
      (String.concat files)
      remainder
  | Did_open_or_change ({ file_path; _ }, { should_calculate_errors }) ->
    Printf.sprintf
      "Did_open_or_change(%s,%b)"
      (Path.to_string file_path)
      should_calculate_errors
  | Did_close file_path ->
    Printf.sprintf "Ide_file_closed(%s)" (Path.to_string file_path)
  | Verbose_to_file verbose -> Printf.sprintf "Verbose_to_file(%b)" verbose
  | Hover ({ file_path; _ }, _) ->
    Printf.sprintf "Hover(%s)" (Path.to_string file_path)
  | Definition ({ file_path; _ }, _) ->
    Printf.sprintf "Definition(%s)" (Path.to_string file_path)
  | Completion ({ file_path; _ }, _, _) ->
    Printf.sprintf "Completion(%s)" (Path.to_string file_path)
  | Completion_resolve Completion_resolve.{ fullname; kind = _ } ->
    Printf.sprintf "Completion_resolve(%s)" fullname
  | Completion_resolve_location (file_path, _, _) ->
    Printf.sprintf "Completion_resolve_location(%s)" (Path.to_string file_path)
  | Document_highlight ({ file_path; _ }, _) ->
    Printf.sprintf "Document_highlight(%s)" (Path.to_string file_path)
  | Document_symbol { file_path; _ } ->
    Printf.sprintf "Document_symbol(%s)" (Path.to_string file_path)
  | Workspace_symbol query -> Printf.sprintf "Workspace_symbol(%s)" query
  | Type_definition ({ file_path; _ }, _) ->
    Printf.sprintf "Type_definition(%s)" (Path.to_string file_path)
  | Signature_help ({ file_path; _ }, _) ->
    Printf.sprintf "Signature_help(%s)" (Path.to_string file_path)
  | Code_action ({ file_path; _ }, _) ->
    Printf.sprintf "Code_action(%s)" (Path.to_string file_path)
  | Code_action_resolve { document = { file_path; _ }; resolve_title; _ } ->
    Printf.sprintf
      "Code_action_resolve(%s, %s)"
      (Path.to_string file_path)
      resolve_title
  | Go_to_implementation ({ file_path; _ }, _, _) ->
    Printf.sprintf "Go_to_implementation(%s)" (Path.to_string file_path)
  | Find_references ({ file_path; _ }, _, _) ->
    Printf.sprintf "Find_references(%s)" (Path.to_string file_path)
  | Rename ({ file_path; _ }, _, _, _) ->
    Printf.sprintf "Rename(%s)" (Path.to_string file_path)
  | Type_Hierarchy ({ file_path; _ }, _) ->
    Printf.sprintf "Type_Hierarchy(%s)" (Path.to_string file_path)

type 'a tracked_t = {
  tracking_id: string;
  message: 'a t;
}

let tracked_t_to_string : type a. a tracked_t -> string =
 fun { tracking_id; message } ->
  Printf.sprintf "#%s: %s" tracking_id (t_to_string message)

module Processing_files = struct
  type t = {
    processed: int;
    total: int;
  }

  let to_string (t : t) : string = Printf.sprintf "%d/%d" t.processed t.total
end

(** This can be used for user-facing messages as well as LSP error responses. *)
type rich_error = {
  short_user_message: string;
      (** max 20 chars, for status bar. Will be prepended by "Hack:" *)
  medium_user_message: string;
      (** max 10 words, for tooltip and alert. Will be postpended by " See <log>" *)
  is_actionable: bool;
      (** used to decide if we hould show window/showMessage, i.e. an alert *)
  long_user_message: string;
      (** max 5 lines, for window/logMessage, i.e. Output>Hack window. Will be postpended by "\nDetails: <url>" *)
  category: string;  (** used in LSP Error message and telemetry *)
  data: Hh_json.json option;  (** used in LSP Error message and telemetry *)
}

type notification =
  | Done_init of (Processing_files.t, rich_error) result
  | Processing_files of Processing_files.t
  | Done_processing

let notification_to_string (n : notification) : string =
  match n with
  | Done_init (Ok p) ->
    Printf.sprintf "Done_init(%s)" (Processing_files.to_string p)
  | Done_init (Error edata) ->
    Printf.sprintf "Done_init(%s)" edata.medium_user_message
  | Processing_files p ->
    Printf.sprintf "Processing_file(%s)" (Processing_files.to_string p)
  | Done_processing -> "Done_processing"

type 'a timed_response = {
  unblocked_time: float;
  tracking_id: string;
  response: ('a, Lsp.Error.t) result;
}

type message_from_daemon =
  | Notification of notification
  | Response : 'a timed_response -> message_from_daemon

let message_from_daemon_to_string (m : message_from_daemon) : string =
  match m with
  | Notification n -> notification_to_string n
  | Response { response = Error { Lsp.Error.message; _ }; tracking_id; _ } ->
    Printf.sprintf "#%s: Response_error(%s)" tracking_id message
  | Response { response = Ok _; tracking_id; _ } ->
    Printf.sprintf "#%s: Response_ok" tracking_id

type daemon_args = {
  init_id: string;
  verbose_to_stderr: bool;
  verbose_to_file: bool;
}
