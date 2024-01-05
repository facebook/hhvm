(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** The command-line arguments for "hh_client lsp" *)
type args = {
  from: string;
      (** The source where the client was spawned from, i.e. nuclide, vim, emacs, etc. *)
  config: (string * string) list;  (** --config overrides at the command-line *)
  ignore_hh_version: bool;
  naming_table: string option;
  notebook_mode: bool;
      (** Notebook mode. The *only* thing this does is filter out error messages that are not relevant for notebooks.
      * Important:
      * 1. The *key* difference between notebooks and IDEs is that top-level statements are allowed
      * 2. To preserve (1) and keep our testing surface area small: do not propagate this option outside of ClientLsp:
      * parsing, elab, type-checking, etc. should not know about notebooks.
   *)
  verbose: bool;
      (** Extra logging, including logs per LSP message (voluminous!) *)
  root_from_cli: Path.t;
      (** clientLsp only ever uses the root path provided to us in the initialize request.
      This field here isn't that! it's a record of what root was derived upon launch
      (well before the initialize request), either from an argument if supplied, or
      otherwise by searching from the CWD. We use this field solely to validate
      whether there's a mismatch between the project root implied by how we launched,
      and the project root supplied by initialize, in case we want to warn about
      any difference. *)
}

(** This is the main loop for processing incoming Lsp client requests,
    and incoming server notifications. Never returns. *)
val main :
  args ->
  init_id:string ->
  local_config:ServerLocalConfig.t ->
  init_proc_stack:string list option ->
  Exit_status.t Lwt.t
