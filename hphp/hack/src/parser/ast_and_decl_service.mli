(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val go :
  Provider_context.t ->
  ?quick:bool ->
  ?show_all_errors:bool ->
  MultiWorker.worker list option ->
  get_next:Relative_path.t list Bucket.next ->
  ParserOptions.t ->
  trace:bool ->
  cache_decls:bool ->
  FileInfo.t Relative_path.Map.t * Errors.t * Relative_path.Set.t
