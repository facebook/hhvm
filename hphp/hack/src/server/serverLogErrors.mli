(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Type check files and generate Telemetry with error diagnostics *)
val go :
  MultiWorker.worker list option ->
  ServerEnv.env ->
  string list ->
  Filter_diagnostics.Filter.t ->
  bool ->
  Telemetry.t
