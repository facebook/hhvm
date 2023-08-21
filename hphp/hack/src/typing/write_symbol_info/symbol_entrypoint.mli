(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* entry point to the hack indexer *)

open Hh_prelude
module Indexable = Symbol_indexable
module Sym_hash = Symbol_sym_hash

val index_files :
  Provider_context.t -> out_dir:string -> files:Relative_path.t list -> unit

val sym_hashes :
  Provider_context.t -> files:Relative_path.t list -> (string * Md5.t) list

val go :
  MultiWorker.worker list option ->
  Provider_context.t ->
  referenced_file:string option ->
  reindexed_file:string option ->
  namespace_map:(string * string) list ->
  gen_sym_hash:bool ->
  ownership:bool ->
  out_dir:string ->
  root_path:string ->
  hhi_path:string ->
  incremental:Sym_hash.t option ->
  files:Indexable.t list ->
  unit
