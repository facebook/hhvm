(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val indexing : ServerEnv.genv -> Relative_path.t list Bucket.next * float

val parsing :
  lazy_parse:bool ->
  ServerEnv.genv ->
  ServerEnv.env ->
  get_next:Relative_path.t list Bucket.next ->
  ?count:int ->
  float ->
  trace:bool ->
  ServerEnv.env * float

val update_files : ServerEnv.genv -> Naming_table.t -> float -> float

val naming : ServerEnv.env -> float -> ServerEnv.env * float

val type_check :
  ServerEnv.genv ->
  ServerEnv.env ->
  Relative_path.t list ->
  float ->
  ServerEnv.env * float
