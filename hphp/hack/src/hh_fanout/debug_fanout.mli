(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type result

val go :
  ctx:Provider_context.t ->
  workers:MultiWorker.worker list ->
  old_naming_table:Naming_table.t ->
  new_naming_table:Naming_table.t ->
  file_deltas:Naming_sqlite.file_deltas ->
  path:Relative_path.t ->
  result

val result_to_json : result -> Hh_json.json
