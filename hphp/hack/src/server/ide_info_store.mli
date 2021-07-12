(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Module to track the current connected IDE. *)

type client_id = int

(** The information we track about an IDE. *)
type t = {
  id: client_id;  (** A unique ID assigned to this IDE when it's created. *)
  client: ClientProvider.client;
      (** Used to communicated with the IDE client. *)
  open_files: Relative_path.Set.t;  (** Files which are opened in this IDE. *)
}

(** Make this client persistent and track a new IDE client. *)
val make_persistent_and_track_new_ide :
  ClientProvider.client -> ClientProvider.client

(** Notify that the IDE has disconnected and stop tracking it. *)
val ide_disconnect : unit -> unit

(** Notify that a file has been opened in the IDE. *)
val open_file : Relative_path.t -> unit

(** Notify that a file has been closed in the IDE. *)
val close_file : Relative_path.t -> unit

(** Get current IDE tracked information. *)
val get : unit -> t option
