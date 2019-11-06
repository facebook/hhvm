(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Hack_bucket = Bucket
open Core_kernel
open Hh_json
open Aast
open Typing_symbol_json_builder
open SymbolOccurrence
module Bucket = Hack_bucket

let get_localvars fn_def lv_acc =
  let process_lv symbol =
    { lv_name = symbol.name; lv_definition = symbol.pos; lvs = [symbol] }
  in
  let symbols = IdentifySymbolService.all_symbols [fn_def] in
  let new_lvs =
    List.fold symbols ~init:[] ~f:(fun acc symbol ->
        match symbol.type_ with
        | LocalVar ->
          if List.exists ~f:(fun x -> String.equal x.lv_name symbol.name) acc
          then
            (* if we found a localvar already, update it *)
            List.map acc ~f:(fun lv ->
                if String.equal lv.lv_name symbol.name then
                  { lv with lvs = symbol :: lv.lvs }
                else
                  lv)
          else
            (* found a new localvar - must be the definition *)
            process_lv symbol :: acc
        | _ -> acc)
  in
  new_lvs @ lv_acc

let get_decls (tast : (Relative_path.t * Tast.program) list) :
    symbol_occurrences =
  let (all_decls, all_defs, all_lvs) =
    List.fold
      tast
      ~init:([], [], [])
      ~f:(fun (decl_acc, def_acc, lvs_acc) (_, prog) ->
        List.fold
          prog
          ~init:(decl_acc, def_acc, lvs_acc)
          ~f:(fun (decls, defs, lvs) def ->
            match def with
            | Class _
            | Typedef _
            | Constant _ ->
              (def :: decls, def :: defs, lvs)
            | Fun _ -> (def :: decls, def :: defs, get_localvars def lvs)
            | _ -> (decls, def :: defs, lvs)))
  in
  let symbols = IdentifySymbolService.all_symbols all_defs in
  { decls = all_decls; occurrences = symbols; localvars = all_lvs }

let write_json
    (tcopt : TypecheckerOptions.t)
    (file_dir : string)
    (tast_lst : (Relative_path.t * Tast.program) list) : unit =
  let symbol_occurrences = get_decls tast_lst in
  let json_chunks =
    Typing_symbol_json_builder.build_json tcopt symbol_occurrences
  in
  let (_, channel) =
    Filename.open_temp_file
      ~temp_dir:file_dir
      "glean_symbol_info_chunk_"
      ".json"
  in
  let json = JSON_Array json_chunks in
  Out_channel.output_string channel (json_to_string ~pretty:true json);
  Out_channel.close channel

let recheck_job
    (tcopt : TypecheckerOptions.t)
    (out_dir : string)
    ()
    (progress : (Relative_path.t * FileInfo.t) list) : unit =
  let results = ServerIdeUtils.recheck tcopt progress in
  write_json tcopt out_dir results

let go
    (workers : MultiWorker.worker list option)
    (tcopt : TypecheckerOptions.t)
    (out_dir : string)
    (file_tuples : (Relative_path.t * FileInfo.t) list) : unit =
  let num_workers =
    match workers with
    | Some w -> List.length w
    | None -> 1
  in
  MultiWorker.call
    workers
    ~job:(recheck_job tcopt out_dir)
    ~merge:(fun () () -> ())
    ~next:(Bucket.make ~num_workers file_tuples)
    ~neutral:()
