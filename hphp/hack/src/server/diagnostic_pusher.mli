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
type t

val init : t

(** Push a new batch of errors to the LSP client. [rechecked] is the full
    set of files that have been rechecked to produce this new batch of errors. *)
val push_new_errors :
  t -> rechecked:Relative_path.Set.t -> Errors.t -> phase:Errors.phase -> t

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
