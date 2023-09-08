(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val go :
  Provider_context.t ->
  trace:bool ->
  cache_decls:bool ->
  ?worker_call:MultiWorker.call_wrapper ->
  MultiWorker.worker list option ->
  get_next:Relative_path.t list MultiWorker.Hh_bucket.next ->
  FileInfo.t Relative_path.Map.t
