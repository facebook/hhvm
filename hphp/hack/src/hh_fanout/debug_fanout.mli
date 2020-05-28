(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type result

val go :
  naming_table:Naming_table.t ->
  ctx:Provider_context.t ->
  workers:MultiWorker.worker list ->
  path:Path.t ->
  result

val result_to_json : result -> Hh_json.json
