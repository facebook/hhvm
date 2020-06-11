(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val init :
  Provider_context.t ->
  MultiWorker.worker list option ->
  worker_key:string ->
  check_id:string ->
  recli_version:string ->
  transport_channel:string option ->
  file_system_mode:ArtifactStore.file_system_mode ->
  ci_info:Ci_util.info option Future.t option ->
  init_id:string ->
  init_start_t:float ->
  bin_root:Path.t ->
  root:Path.t ->
  unit
