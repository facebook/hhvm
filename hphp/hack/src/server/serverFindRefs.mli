(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open ServerCommandTypes.Find_refs

val to_json: result -> Hh_json.json

val go : action -> bool -> ServerEnv.genv -> ServerEnv.env -> ServerEnv.env * server_result_or_retry

val go_from_file:
  ServerCommandTypes.labelled_file * int * int ->
  ServerEnv.env ->
  (string * action) option

val to_absolute: server_result -> result

val to_ide: string -> server_result -> ide_result
