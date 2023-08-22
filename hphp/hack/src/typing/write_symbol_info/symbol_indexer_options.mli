(*
 * Copyright (c) Meta, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Sym_hash = Symbol_sym_hash

type t = private {
  (* if provided, create a file containing all php files referenced by the indexed files *)
  referenced_file: string option;
  (* if provided, create a file containing all files actually reindexed within that run.  *)
  reindexed_file: string option;
  (* if true, will generate hack.IndexerInputsHash *)
  gen_sym_hash: bool;
  (* if true, generate ownership units *)
  ownership: bool;
  (* dir containing indexer facts *)
  out_dir: string;
  (* prefix for files in www, e.g. www  *)
  root_path: string;
  (* prefix for files in fbcode, e.g. fbsource  *)
  hhi_path: string;
  (* incremental runs (and only them) need an IndexerInputsHash table *)
  incremental: Sym_hash.t option;
}

val create : GlobalOptions.t -> out_dir:string -> t

val default : out_dir:string -> t

val log : t -> unit
