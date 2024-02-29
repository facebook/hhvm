(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Indexing a Hack file consists in three distinct passes
  1. Indexing source text (regex matching for extracting gencode info from comments)
  2. Indexing declarations, done in [index_decls]
  3. Indexing xrefs and filecalls, done in [index_refs] *)

open Hh_prelude
module Fact_acc = Predicate.Fact_acc

let process_source_text _ctx fa File_info.{ path; source_text; _ } =
  let text = Full_fidelity_source_text.text source_text in
  match Gencode_utils.get_gencode_status text with
  | Gencode_utils.
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
      fa
    |> snd
  | _ -> fa

let build_json ctx files_info ~ownership =
  let index_file fa file_info =
    let path = file_info.File_info.path in
    Fact_acc.set_ownership_unit fa (Some path);
    let fa = process_source_text ctx fa file_info in
    let (fa, xrefs) = Index_xrefs.process_xrefs_and_calls ctx fa file_info in
    Fact_acc.set_pos_map fa xrefs.Xrefs.pos_map;
    let (mod_xrefs, fa) = Index_decls.process_decls fa file_info in
    let fact_map_xrefs = xrefs.Xrefs.fact_map in
    let fact_map_module_xrefs = mod_xrefs.Xrefs.fact_map in
    let merge _ x _ = Some x in
    let all_xrefs =
      Fact_id.Map.union merge fact_map_module_xrefs fact_map_xrefs
    in
    if Fact_id.Map.is_empty all_xrefs then
      fa
    else
      Add_fact.file_xrefs ~path all_xrefs fa |> snd
  in
  let fa = Fact_acc.init ~ownership in
  List.fold files_info ~init:fa ~f:index_file |> Fact_acc.to_json
