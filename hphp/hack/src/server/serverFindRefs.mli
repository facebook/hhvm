(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerCommandTypes.Find_refs

val to_json : result -> Hh_json.json

val add_ns : String.t -> String.t

val handle_prechecked_files :
  ServerEnv.genv ->
  ServerEnv.env ->
  Typing_deps.DepSet.elt ->
  (unit -> 'a) ->
  ServerEnv.env * 'a ServerCommandTypes.Done_or_retry.t

val is_local : action -> bool

(**
 * Given a Find_refs action, returns an updated env
 * and optional list of all positions where this action exists.
 * Wrapped in a Done_or_retry for clientLsp.
 *)
val go :
  Provider_context.t ->
  action ->
  bool ->
  ServerEnv.genv ->
  ServerEnv.env ->
  ServerEnv.env * server_result_or_retry

(**
 * Given a position in a file, returns the name of the symbol
 * and data ("action") that represents if it's a class, method, etc.
 *)
val go_from_file_ctx :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  line:int ->
  column:int ->
  (string * action) option

(**
 * Like go, but only supports LocalVar actions. If supplied a localvar,
 * returns an optional list of all positions of that localvar. If action is not a localvar,
 * returns the same action back wrapped in an error.
 *)
val go_for_localvar :
  Provider_context.t ->
  action ->
  ((string * Pos.t) list, action) Hh_prelude.result

val to_absolute : server_result -> result

val to_ide : string -> server_result -> ide_result
