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
  ?e:Exception.t ->
  telemetry:bool ->
  string ->
  unit

(** for when we encountered an internal bug and want a user-facing problem report. *)
val make_rich_error :
  ?data:Hh_json.json option ->
  ?e:Exception.t ->
  string ->
  ClientIdeMessage.rich_error

val to_lsp_error : ClientIdeMessage.rich_error -> Lsp.Error.t
