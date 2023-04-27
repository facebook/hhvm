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
module XRefs = Symbol_xrefs
module Fact_id = Symbol_fact_id

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

let build_json ctx files_info ~ownership =
  let index_file progress file_info =
    let path = file_info.File_info.path in
    Fact_acc.set_ownership_unit progress (Some path);
    let progress = process_source_text ctx progress file_info in
    let (progress, xrefs) =
      Symbol_index_xrefs.process_xrefs_and_calls ctx progress file_info
    in
    Fact_acc.set_pos_map progress xrefs.XRefs.pos_map;
    let (mod_xrefs, progress) =
      Symbol_index_decls.process_decls ctx progress file_info
    in
    let fact_map_xrefs = xrefs.XRefs.fact_map in
    let fact_map_module_xrefs = mod_xrefs.XRefs.fact_map in
    let merge _ x _ = Some x in
    let all_xrefs =
      Fact_id.Map.union merge fact_map_module_xrefs fact_map_xrefs
    in
    if Fact_id.Map.is_empty all_xrefs then
      progress
    else
      Add_fact.file_xrefs ~path all_xrefs progress |> snd
  in
  let progress = Fact_acc.init ~ownership in
  List.fold files_info ~init:progress ~f:index_file |> Fact_acc.to_json
