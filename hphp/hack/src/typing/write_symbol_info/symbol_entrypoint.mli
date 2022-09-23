(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* entry point to the hack indexer *)

val index_files :
  Provider_context.t -> out_dir:string -> files:Relative_path.t list -> unit

val go :
  MultiWorker.worker list option ->
  Provider_context.t ->
  namespace_map:(string * string) list ->
  ownership:bool ->
  out_dir:string ->
  root_path:string ->
  hhi_path:string ->
  files:Relative_path.t list ->
  unit
