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
  nonce:Int64.t ->
  check_id:string ->
  recli_version:string ->
  transport_channel:string option ->
  remote_type_check_config:ServerLocalConfig.RemoteTypeCheck.t ->
  ci_info:Ci_util.info option Future.t option ->
  init_id:string ->
  init_start_t:float ->
  bin_root:Path.t ->
  root:Path.t ->
  mode:HulkStrategy.hulk_mode ->
  cache_remote_decls:bool ->
  use_shallow_decls_saved_state:bool ->
  saved_state_manifold_path:string option ->
  shallow_decls_manifold_path:string option ->
  unit
