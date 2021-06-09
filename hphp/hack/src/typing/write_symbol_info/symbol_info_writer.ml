(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Hack_bucket = Bucket
open Hh_prelude
open Hh_json
open Provider_context
open Unix
module Bucket = Hack_bucket

let write_file file_dir num_tasts json_chunks =
  let (_out_file, channel) =
    Caml.Filename.open_temp_file
      ~temp_dir:file_dir
      "glean_symbol_info_chunk_"
      ".json"
  in
  let json_string = json_to_string (JSON_Array json_chunks) in
  let json_length = String.length json_string in
  Out_channel.output_string channel json_string;
  Out_channel.close channel;
  Hh_logger.log
    "Wrote %s of JSON facts on %i files"
    (Byte_units.to_string_hum (Byte_units.of_bytes_int json_length))
    num_tasts

let write_json
    (ctx : Provider_context.t)
    (file_dir : string)
    (files_info : Symbol_builder_types.file_info list)
    (start_time : float) : unit =
  try
    let (small, large) =
      List.partition_tf files_info ~f:(fun (_, tast, _) ->
          List.length tast <= 2000)
    in
    if List.is_empty large then
      let json_chunks = Symbol_json_builder.build_json ctx files_info in
      write_file file_dir (List.length files_info) json_chunks
    else
      let json_chunks = Symbol_json_builder.build_json ctx small in
      write_file file_dir (List.length small) json_chunks;
      List.iter large ~f:(fun (fp, tast, st) ->
          let decl_json_chunks =
            Symbol_json_builder.build_decls_json ctx [(fp, tast, st)]
          in
          write_file file_dir 1 decl_json_chunks;
          let xref_json_chunks =
            Symbol_json_builder.build_xrefs_json ctx [tast]
          in
          write_file file_dir 1 xref_json_chunks);
      let elapsed = Unix.gettimeofday () -. start_time in
      let time = Unix.gmtime elapsed in
      Hh_logger.log "Processed batch in %dm%ds" time.tm_min time.tm_sec
  with
  | WorkerCancel.Worker_should_exit as e ->
    (* Cancellation requests must be re-raised *)
    raise e
  | e ->
    Printf.eprintf "WARNING: symbol write failure: \n%s\n" (Exn.to_string e)

let recheck_job
    (ctx : Provider_context.t)
    (out_dir : string)
    (root_path : string)
    (hhi_path : string)
    ()
    (progress : Relative_path.t list) : unit =
  let start_time = Unix.gettimeofday () in
  let files_info =
    List.map progress ~f:(fun path ->
        let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
        let { Tast_provider.Compute_tast.tast; _ } =
          Tast_provider.compute_tast_unquarantined ~ctx ~entry
        in
        (path, tast, entry.source_text))
  in
  Relative_path.set_path_prefix Relative_path.Root (Path.make root_path);
  Relative_path.set_path_prefix Relative_path.Hhi (Path.make hhi_path);
  write_json ctx out_dir files_info start_time

let go
    (workers : MultiWorker.worker list option)
    (ctx : Provider_context.t)
    (out_dir : string)
    (root_path : string)
    (hhi_path : string)
    (filepaths : Relative_path.t list) : unit =
  let num_workers =
    match workers with
    | Some w -> List.length w
    | None -> 1
  in
  MultiWorker.call
    workers
    ~job:(recheck_job ctx out_dir root_path hhi_path)
    ~merge:(fun () () -> ())
    ~next:(Bucket.make ~num_workers ~max_size:150 filepaths)
    ~neutral:()
