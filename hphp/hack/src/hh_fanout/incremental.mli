(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** An identifier representing one of the users of `hh_fanout`. Clients with
different options will have different IDs. *)
type client_id = Client_id of string

(** An identifier representing a `cursor`. *)
type cursor_id = Cursor_id of string

type dep_graph_delta = (Typing_deps.Dep.t * Typing_deps.Dep.t) HashSet.t

(** Any options which would affect the results of the returned fanouts. *)
type client_config = {
  client_id: string;
  ignore_hh_version: bool;
  dep_table_saved_state_path: Path.t;
  dep_table_errors_saved_state_path: Path.t;
  naming_table_saved_state_path: Naming_sqlite.db_path;
}

(** A "cursor" represents a pointer to a state in time of the repository with
respect to the dependency graph and typechecking errors.

`hh_fanout` operations take a cursor representing the previous state of the
world, and return a new cursor representing the current state of the world.
*)
class type cursor =
  object
    (** Get the cumulative file deltas that have occurred since the
    saved-state for the point in time represented by this cursor. *)
    method get_file_deltas : Naming_sqlite.file_deltas

    (** Get the fanout calculated for the files changed in this cursor.
    Returns `None` if inapplicable for this type of cursor. *)
    method get_calculate_fanout_result : Calculate_fanout.result option

    (** Get the fanout that needs to be re-typechecked (i.e. the fanout of
    the files changed since the last typecheck).

    There is one element in the returned list per cursor-advance since the
    last typecheck. The newest elements are first. *)
    method get_calculate_fanout_results_since_last_typecheck :
      Calculate_fanout.result list

    (** Get the client ID that owns this cursor. *)
    method get_client_id : client_id

    (** Get the client configuration associated with this cursor.

    In particular, this contains information about the saved-state that this
    cursor was initialized with. *)
    method get_client_config : client_config

    (** Process the provided set of changed files and advance the cursor.

    This involves typechecking the changed files and updating the dependency
    graph.

    The resulting cursor is NOT persisted to disk. The caller is responsible
    for persisting it using `State.add_cursor`. *)
    method advance :
      detail_level:Calculate_fanout.Detail_level.t ->
      Provider_context.t ->
      MultiWorker.worker list ->
      Relative_path.Set.t ->
      cursor

    (** Typecheck the files in the fanout for this cursor.

    Returns a cursor with the typechecking errors cached. The resulting
    cursor is NOT persisted to disk. The caller is responsible for persisting
    it using `State.add_cursor`.

    If no new cursor was created (because no additional work needed to be
    performed, i.e. the typechecking errors were already cached for this
    cursor), then returns `None` instead of `Some cursor`.
    *)
    method calculate_errors :
      Provider_context.t -> MultiWorker.worker list -> Errors.t * cursor option
  end

class type state =
  object
    (** Look up the ID of the client corresponding to the provided
    `client_config`.

    If no such client already exists, one is created. This operation is
    idempotent. *)
    method make_client_id : client_config -> client_id

    (** Construct the cursor corresponding to the saved-state.

    The caller should immediately advance it with any files that have changed
    since the saved-state.

    Returns `Error` if the given `client_id` doesn't exist. *)
    method make_default_cursor : client_id -> (cursor, string) result

    (** Look up the given cursor.

    If `client_id` is provided, it is used as a sanity-check, and an
    exception is thrown if the provided cursor does not belong to the
    provided client.

    * Returns `Error` if the cursor does not exist.
    * Returns `Error` if the cursor exists, but doesn't belong to the
    provided client. *)
    method look_up_cursor :
      client_id:client_id option -> cursor_id:string -> (cursor, string) result

    (** Add the given cursor to the state, committing any changes to disk. *)
    method add_cursor : cursor -> cursor_id
  end

(** Reference implementation using OCaml blobs to store state. Loads the
state from the given path on disk.

If the path does not exist, it is atomically created and loaded.

Currently not production-usable -- it is not safe for concurrent consumers.
*)
val make_reference_implementation : Path.t -> state
