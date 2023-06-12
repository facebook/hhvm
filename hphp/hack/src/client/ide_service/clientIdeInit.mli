(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Error case for [init]. [stopped_reason] is a human-facing response,
and [Lsp.Error.t] contains structured telemetry data. *)
type init_error = ClientIdeMessage.stopped_reason * Lsp.Error.t

(** Success case for [init]. *)
type init_ok = {
  naming_table: Naming_table.t;
      (** An open connection to sqlite naming-table file. (Naming_table.t is able to store an in-memory
      delta, but in our case the result of [init] always has an empty delta.) *)
  sienv: SearchUtils.si_env;
      (** search+autocomplete index, either full (in case we built) or delta
      plus a connection to whatever symbolindex is specified in hh.conf and .hhconfig
      (in other cases) *)
  changed_files: Saved_state_loader.changed_files;
      (** files changed between saved-state and "now", for some arbitrary value of "now"
      during the execution of [init]. For race-freedom, the caller will need to set up
      some subscription for change-notifications prior to calling [init]. *)
}

type init_result = (init_ok, init_error) result

(** "Initialization" for clinetIdeDaemon means setting up naming-table and search-index
and figuring out what files have changed and need to be indexed.
We uses a strategy of "find/init/fetch/build":
1. [FIND] (if ide_load_naming_table_on_disk) look for any saved-state already on disk
in the canonical location /tmp/hh_server that's within an age threshold,
and if found then use it, ask mercurial for files changed since that saved-state,
and initialize sienv empty
2. [INIT] Otherwise, if the LSP initialize request contains the path of a naming-table
then use it, trust that there are no files-changed since that saved-state, and initialize sienv empty
3. [FETCH] Otherwise, try [State_loader_lwt.load] which uses manifold to look for
the best saved-state. If one is found then download it, ask mercurial for
files changed since that saved-state, and initialize sienv empty
4. [BUILD] Otherwise, if .hhconfig allows fall_back_to_full_index (default true) then do a fast
multi-core build of the naming-table-sqlite, initialize the naming-table with that,
deem that there are no files changed since the build, and initialize sienv with all the symbols that
we parsed while building the full naming table
5. Otherwise, error. *)
val init :
  config:ServerConfig.t ->
  local_config:ServerLocalConfig.t ->
  param:ClientIdeMessage.Initialize_from_saved_state.t ->
  hhi_root:Path.t ->
  local_memory:Provider_backend.local_memory ->
  notify_callback_in_case_of_fallback_to_full_init:(unit -> unit Lwt.t) ->
  init_result Lwt.t
