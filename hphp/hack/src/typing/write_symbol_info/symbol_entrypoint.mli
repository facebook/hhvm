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
module Indexer_options = Symbol_indexer_options

(* simpler entry point, uses default options indexing options, doesn't
   use worker. This is used by hh_single_type_check. *)
val index_files :
  Provider_context.t -> out_dir:string -> files:Relative_path.t list -> unit

val sym_hashes :
  Provider_context.t -> files:Relative_path.t list -> (string * Md5.t) list

(* namespace_map is the aliases map, used to generate hack.GlobalNamespaceAlias *)
val go :
  MultiWorker.worker list option ->
  Provider_context.t ->
  Indexer_options.t ->
  namespace_map:(string * string) list ->
  files:Indexable.t list ->
  unit
