(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* The environment for hh_client with LSP *)
type env = {
  from: string;
      (** The source where the client was spawned from, i.e. nuclide, vim, emacs, etc. *)
  config: (string * string) list;  (** --config overrides at the command-line *)
  use_ffp_autocomplete: bool;
      (** Flag to turn on the (experimental) FFP based autocomplete *)
  use_ranked_autocomplete: bool;
      (** Flag to turn on ranked autocompletion results *)
  use_serverless_ide: bool;
      (** Flag to provide IDE services from `hh_client` *)
  verbose: bool;
      (** Extra logging, including logs per LSP message (voluminous!) *)
  init_id: string;  (** telemetry uses this per-instance init_id *)
}

(* main: this is the main loop for processing incoming Lsp client requests,
   and incoming server notifications. Never returns. *)
val main : env -> Exit_status.t Lwt.t
