(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type direct_decl_mode =
  | Normal
  | Cached

val go :
  Provider_context.t ->
  trace:bool ->
  decl_mode:direct_decl_mode ->
  ?worker_call:Multi_worker.call_wrapper ->
  Multi_worker.worker list option ->
  get_next:Relative_path.t list Multi_worker.Hh_bucket.next ->
  FileInfo.t Relative_path.Map.t
