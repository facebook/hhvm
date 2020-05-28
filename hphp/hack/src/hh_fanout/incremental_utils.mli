(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Atomically initialize the provided directory using `populate_dir`. *)
val init_state_dir : Path.t -> populate_dir:(Path.t -> unit) -> unit

(** Construct the client ID exposed to the user.

The client ID contains strictly less information than the cursor ID, as a
single client may have multiple cursors associated with it. *)
val make_client_id : Incremental.client_config -> Incremental.client_id

(** Construct the cursor ID exposed to the user. *)
val make_cursor_id : int -> Incremental.client_config -> Incremental.cursor_id
