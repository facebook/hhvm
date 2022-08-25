(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Indexing a Hack file consists in three distinct passes
  1. Indexing source text (regex matching for extracting gencode info from comments)
  2. Indexing declarations, done in [Symbol_index_decls]
  3. Indexing xrefs and filecalls, done in [Symbol_index_refs] *)

open Hh_prelude
module File_info = Symbol_file_info
module Gencode = Symbol_gencode
module Add_fact = Symbol_add_fact
module Fact_acc = Symbol_predicate.Fact_acc

let process_source_text _ctx prog File_info.{ path; source_text; _ } =
  let text = Full_fidelity_source_text.text source_text in
  match Gencode.get_gencode_status text with
  | Gencode.
      {
        is_generated = true;
        fully_generated;
        source;
        command;
        class_;
        signature;
      } ->
    Add_fact.gen_code
      ~path
      ~fully_generated
      ~signature
      ~source
      ~command
      ~class_
      prog
    |> snd
  | _ -> prog

let process_xrefs_all_files ctx files_info progress =
  List.fold
    files_info
    ~init:progress
    ~f:(Symbol_index_xrefs.process_xrefs_and_calls ctx)

let process_decls_all_files ctx files_info progress =
  List.fold files_info ~init:progress ~f:(Symbol_index_decls.process_decls ctx)

let process_source_text_all_files ctx files_info progress =
  List.fold files_info ~init:progress ~f:(process_source_text ctx)

(* This function processes declarations, starting with an
empty fact cache. *)
let build_decls_json ctx files_info ~ownership =
  Fact_acc.init ~ownership
  |> process_decls_all_files ctx files_info
  |> Fact_acc.to_json

(* This function processes cross-references, starting with an
empty fact cache. *)
let build_xrefs_json ctx files_info ~ownership =
  Fact_acc.init ~ownership
  |> process_xrefs_all_files ctx files_info
  |> Fact_acc.to_json

(* This function processes both declarations and cross-references,
sharing the declaration fact cache between them. *)
let build_json ctx files_info ~ownership =
  Fact_acc.init ~ownership
  |> process_source_text_all_files ctx files_info
  |> process_decls_all_files ctx files_info
  |> process_xrefs_all_files ctx files_info
  |> Fact_acc.to_json
