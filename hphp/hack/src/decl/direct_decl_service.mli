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
  MultiWorker.worker list option ->
  (* IDE files are processed sequentially (in the master process), to match the
     behavior of the legacy Parsing_service. *)
  ide_files:Relative_path.Set.t ->
  (* Buckets the files which will be processed in parallel *)
  get_next:Relative_path.t list MultiWorker.Hh_bucket.next ->
  FileInfo.t Relative_path.Map.t
