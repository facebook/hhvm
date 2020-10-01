(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val go :
  ?quick:bool ->
  ?show_all_errors:bool ->
  MultiWorker.worker list option ->
  Relative_path.Set.t ->
  get_next:Relative_path.t list Bucket.next ->
  ParserOptions.t ->
  trace:bool ->
  FileInfo.t Relative_path.Map.t * Errors.t * Relative_path.Set.t
