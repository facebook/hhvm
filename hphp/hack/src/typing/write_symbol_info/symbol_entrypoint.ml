(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Hh_json
module File_info = Symbol_file_info

let log_elapsed s elapsed =
  let { Unix.tm_min; tm_sec; _ } = Unix.gmtime elapsed in
  Hh_logger.log "%s %dm%ds" s tm_min tm_sec

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
    (ownership : bool)
    (file_dir : string)
    (files_info : File_info.t list)
    (start_time : float) : float =
  (* Large file may lead to large json files which timeout when sent
     to the server. Not an issue currently, but if it is, we can index
     xrefs/decls separately, or split in batches according to files size *)
  (List.iter files_info ~f:(fun File_info.{ tast; path; _ } ->
       if List.length tast > 2000 then Hh_logger.log "Large file: %s" path);
   try
     let json_chunks =
       Symbol_index_batch.build_json ctx files_info ~ownership
     in
     write_file file_dir (List.length files_info) json_chunks
   with
   | WorkerCancel.Worker_should_exit as exn ->
     (* Cancellation requests must be re-raised *)
     let e = Exception.wrap exn in
     Exception.reraise e
   | e ->
     Printf.eprintf "WARNING: symbol write failure: \n%s\n" (Exn.to_string e));
  let elapsed = Unix.gettimeofday () -. start_time in
  log_elapsed "Processed batch in" elapsed;
  elapsed

let recheck_job
    (ctx : Provider_context.t)
    (out_dir : string)
    (root_path : string)
    (hhi_path : string)
    (ownership : bool)
    (_ : float)
    (progress : Relative_path.t list) : float =
  let start_time = Unix.gettimeofday () in
  let files_info =
    List.map progress ~f:(File_info.create ctx ~root_path ~hhi_path)
  in
  write_json ctx ownership out_dir files_info start_time

let index_files ctx ~out_dir ~files =
  recheck_job ctx out_dir "www" "hhi" false 0.0 files |> ignore

let go
    (workers : MultiWorker.worker list option)
    (ctx : Provider_context.t)
    ~(ownership : bool)
    ~(out_dir : string)
    ~(root_path : string)
    ~(hhi_path : string)
    ~(files : Relative_path.t list) : unit =
  let num_workers =
    match workers with
    | Some w -> List.length w
    | None -> 1
  in
  let start_time = Unix.gettimeofday () in
  let cumulated_elapsed =
    MultiWorker.call
      workers
      ~job:(recheck_job ctx out_dir root_path hhi_path ownership)
      ~merge:(fun f1 f2 -> f1 +. f2)
      ~next:(Bucket.make ~num_workers ~max_size:115 files)
      ~neutral:0.
  in
  log_elapsed "Processed all batches (cumulated time) in " cumulated_elapsed;
  let elapsed = Unix.gettimeofday () -. start_time in
  log_elapsed "Processed all batches in " elapsed
