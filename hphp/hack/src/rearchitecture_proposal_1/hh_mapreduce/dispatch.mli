(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** These are the kinds of mapreduce orchestrators, and of their workers. *)
type kind =
  | Prototype
  | Typecheck

(** For each kind, says its string name, plus how to invoke its orchestrator/worker *)
type info = {
  name: string;
      (** String name is used (1) at CLI, (2) for initial orchestrator->worker json handshake *)
  kind: kind;
  run: unit -> unit;  (** run is used at CLI *)
  run_worker: Unix.file_descr -> Decl_service_client.t -> unit;
      (** run_worker is how prototype dispatches the worker *)
}

(** register must be called exactly once, prior to find_by_kind or find_by_name,
    to provide the full list of modes. The provided list must cover every single
    Dispatch.kind (this invariant isn't checked at moment of register, but will throw
    an exception down the line). The invariant that it's called no more than once
    is verified at moment of calling; the invariant that it's called at least
    once is verified by the find_by_xyz functions. *)
val register : info list -> unit

(** given a name, finds the Dispatch.info registered for it. Returns None if no
    info was associated with this name, e.g. in case of a typo. Throws exception
    if register hasn't been called. *)
val find_by_name : string -> info option

(** given a kind, finds the Dispatch.info registered for it. Throws a Not_found
    exception if registration failed to cover this kind. Throws exception if
    register hasn't yet been called. *)
val find_by_kind : kind -> info
