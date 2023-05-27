(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Module to push diagnostics to the LSP client. *)

(** This needs to persist for the whole lifetime of the server to properly
    keep track of diagnostics. *)
type t [@@deriving show]

type seconds_since_epoch = float

val init : t

(** Push a new batch of errors to the LSP client. [rechecked] is the full
    set of files that have been rechecked to produce this new batch of errors.
    Return the updated pusher and the timestamp of the push if anything was pushed,
    or None if not, e.g. there was nothing to push or no persistent client or the push failed. *)
val push_new_errors :
  t ->
  rechecked:Relative_path.Set.t ->
  Errors.t ->
  t * seconds_since_epoch option

(** If any error remains to be pushed, for example because the previous push failed or
    because there was no IDE connected at the time, push them.
    Return the updated pusher and the timestamp of the push if anything was pushed,
    or None if not, e.g. there was nothing to push or no persistent client or the push failed. *)
val push_whats_left : t -> t * seconds_since_epoch option

val get_files_with_diagnostics : t -> Relative_path.t list

(** Module to export internal functions for unit testing. Do not use in production code. *)
module TestExporter : sig
  module FileMap : module type of Relative_path.Map

  module ErrorTracker : sig
    type t [@@deriving eq, show]

    val init : t

    val get_errors_to_push :
      t ->
      rechecked:Relative_path.Set.t ->
      new_errors:Errors.t ->
      phase:Errors.phase ->
      priority_files:Relative_path.Set.t option ->
      t * Errors.finalized_error list SMap.t

    val commit_pushed_errors : t -> t

    module TestExporter : sig
      val make :
        errors_in_ide:Errors.error list Errors.PhaseMap.t FileMap.t ->
        to_push:Errors.error list Errors.PhaseMap.t FileMap.t ->
        errors_beyond_limit:Errors.error list Errors.PhaseMap.t FileMap.t ->
        t

      val with_error_limit : int -> (unit -> 'result) -> 'result
    end
  end
end
