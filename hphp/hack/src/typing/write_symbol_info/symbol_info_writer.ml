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

let write_json
    (ctx : Provider_context.t)
    (file_dir : string)
    (tasts : Tast.program list)
    (files_info : Symbol_builder_types.file_info list)
    (start_time : float) : unit =
  try
    let json_chunks = Symbol_json_builder.build_json ctx files_info tasts in
    let (_out_file, channel) =
      Caml.Filename.open_temp_file
        ~temp_dir:file_dir
        "glean_symbol_info_chunk_"
        ".json"
    in
    let json_string = json_to_string ~pretty:true (JSON_Array json_chunks) in
    let json_length = String.length json_string in
    Out_channel.output_string channel json_string;
    Out_channel.close channel;
    let elapsed = Unix.gettimeofday () -. start_time in
    let time = Unix.gmtime elapsed in
    Hh_logger.log
      "Wrote %s of JSON facts on %i files in %dm%ds"
      (Byte_units.to_string_hum
         (Byte_units.create `Bytes (float_of_int json_length)))
      (List.length files_info)
      time.tm_min
      time.tm_sec;
    if Float.(elapsed > 600.0) then
      (* Write out files from batches that take more than 10 minutes *)
      let file_list =
        String.concat
          ~sep:","
          (List.map files_info (fun (fp, _) -> Relative_path.to_absolute fp))
      in
      Hh_logger.log "Files processed: %s" file_list
    else
      ()
  with e ->
    Printf.eprintf "WARNING: symbol write failure: \n%s\n" (Exn.to_string e)

let recheck_job
    (ctx : Provider_context.t)
    (out_dir : string)
    (root_path : string)
    (hhi_path : string)
    ()
    (progress : Relative_path.t list) : unit =
  let start_time = Unix.gettimeofday () in
  let info_lsts =
    List.map progress ~f:(fun path ->
        let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
        let { Tast_provider.Compute_tast.tast; _ } =
          Tast_provider.compute_tast_unquarantined ~ctx ~entry
        in
        (tast, (path, entry.source_text)))
  in
  let (tasts, files_info) = List.unzip info_lsts in
  Relative_path.set_path_prefix Relative_path.Root (Path.make root_path);
  Relative_path.set_path_prefix Relative_path.Hhi (Path.make hhi_path);
  write_json ctx out_dir tasts files_info start_time

let go
    (workers : MultiWorker.worker list option)
    (ctx : Provider_context.t)
    (out_dir : string)
    (root_path : string)
    (hhi_path : string)
    (file_tuples : Relative_path.t list) : unit =
  let num_workers =
    match workers with
    | Some w -> List.length w
    | None -> 1
  in
  MultiWorker.call
    workers
    ~job:(recheck_job ctx out_dir root_path hhi_path)
    ~merge:(fun () () -> ())
    ~next:(Bucket.make ~num_workers ~max_size:150 file_tuples)
    ~neutral:()
