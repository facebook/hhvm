(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** for when we encountered an internal bugs and we want to write to
the logfile, and optionally also record telemetry. *)
val log_bug :
  ?data:Hh_json.json option ->
  ?exn:Exception.t ->
  telemetry:bool ->
  string ->
  unit

(** for when we encountered an internal bug and want to return an Lsp error.
These errors are never user-facing. *)
val make_bug_error :
  ?data:Hh_json.json option -> ?exn:Exception.t -> string -> Lsp.Error.t

(** for when we encountered an internal bug and want a user-facing problem report. *)
val make_bug_reason :
  ?data:Hh_json.json option ->
  ?exn:Exception.t ->
  string ->
  ClientIdeMessage.stopped_reason
