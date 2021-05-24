(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* The command-line arguments for "hh_client lsp" *)
type args = {
  from: string;
      (** The source where the client was spawned from, i.e. nuclide, vim, emacs, etc. *)
  config: (string * string) list;  (** --config overrides at the command-line *)
  verbose: bool;
      (** Extra logging, including logs per LSP message (voluminous!) *)
}

(* main: this is the main loop for processing incoming Lsp client requests,
   and incoming server notifications. Never returns. *)
val main : args -> init_id:string -> Exit_status.t Lwt.t
