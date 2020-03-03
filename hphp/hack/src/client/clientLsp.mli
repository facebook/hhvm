(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* The environment for hh_client with LSP *)
type env = {
  (* The source where the client was spawned from, i.e. nuclide, vim, emacs, etc. *)
  from: string;
  (* --config overrides at the command-line *)
  config: (string * string) list;
  (* Flag to turn on the (experimental) FFP based autocomplete *)
  use_ffp_autocomplete: bool;
  (* Flag to turn on ranked autocompletion results *)
  use_ranked_autocomplete: bool;
  (* Flag to provide IDE services from `hh_client` *)
  use_serverless_ide: bool;
  (* Extra logging, including logs per LSP message (voluminous!) *)
  verbose: bool;
}

(* main: this is the main loop for processing incoming Lsp client requests,
   and incoming server notifications. Never returns. *)
val main : string -> env -> Exit_status.t Lwt.t
