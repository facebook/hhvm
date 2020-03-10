(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open RemoteWorker
open Typing_service_types

val make_local_server_api :
  Naming_table.t ->
  root:string ->
  ignore_hh_version:bool ->
  (module LocalServerApi)

val make_remote_server_api :
  Provider_context.t ->
  MultiWorker.worker list option ->
  (module RemoteServerApi with type naming_table = Naming_table.t option)
