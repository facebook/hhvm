(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open ServerCommandTypes.Find_refs

val to_json : result -> Hh_json.json

val add_ns : String.t -> String.t

val handle_prechecked_files :
  ServerEnv.genv ->
  ServerEnv.env ->
  Typing_deps.DepSet.elt ->
  (unit -> 'a) ->
  ServerEnv.env * 'a ServerCommandTypes.Done_or_retry.t

val go :
  Provider_context.t ->
  action ->
  bool ->
  ServerEnv.genv ->
  ServerEnv.env ->
  ServerEnv.env * server_result_or_retry

val go_from_file_ctx :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  line:int ->
  column:int ->
  (string * action) option

val to_absolute : server_result -> result

val to_ide : string -> server_result -> ide_result
