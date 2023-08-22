(*
 * Copyright (c) Meta, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Sym_hash = Symbol_sym_hash
open Hh_prelude

type t = {
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

let create writeopt ~out_dir =
  let open GlobalOptions in
  (* Ensure we are writing to fresh files *)
  let is_invalid =
    try
      if not (Sys.is_directory out_dir) then
        true
      else
        Array.length (Sys.readdir out_dir) > 0
    with
    | _ ->
      Sys_utils.mkdir_p out_dir;
      false
  in
  if is_invalid then failwith "JSON write directory is invalid or non-empty";
  {
    referenced_file = writeopt.symbol_write_referenced_out;
    reindexed_file = writeopt.symbol_write_reindexed_out;
    gen_sym_hash = writeopt.symbol_write_sym_hash_out;
    ownership = writeopt.symbol_write_ownership;
    out_dir;
    root_path = writeopt.symbol_write_root_path;
    hhi_path = writeopt.symbol_write_hhi_path;
    incremental =
      Option.map
        ~f:(fun path -> Symbol_sym_hash.read ~path)
        writeopt.symbol_write_sym_hash_in;
  }

let default ~out_dir =
  {
    referenced_file = None;
    reindexed_file = None;
    ownership = false;
    out_dir;
    root_path = "www";
    hhi_path = "hhi";
    gen_sym_hash = false;
    incremental = None;
  }

let log opts =
  Hh_logger.log "Ownership mode: %b" opts.ownership;
  Hh_logger.log "Gen_sym_hash: %b" opts.gen_sym_hash;
  Hh_logger.log "Writing JSON to: %s" opts.out_dir
