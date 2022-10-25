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
module Add_fact = Symbol_add_fact
module Fact_acc = Symbol_predicate.Fact_acc
module Indexable = Symbol_indexable
module Sym_hash = Symbol_sym_hash

module JobReturn = struct
  type t = {
    elapsed: float;
    hashes: Md5.Set.t;
    reindexed: SSet.t;
  }

  let neutral =
    { elapsed = 0.0; hashes = Md5.Set.empty; reindexed = SSet.empty }

  let merge t1 t2 =
    {
      elapsed = t1.elapsed +. t2.elapsed;
      hashes = Md5.Set.union t1.hashes t2.hashes;
      reindexed = SSet.union t1.reindexed t2.reindexed;
    }
end

let log_elapsed s elapsed =
  let { Unix.tm_min; tm_sec; _ } = Unix.gmtime elapsed in
  Hh_logger.log "%s %dm%ds" s tm_min tm_sec

let write_file out_dir num_tasts json_chunks =
  let (_out_file, channel) =
    Caml.Filename.open_temp_file
      ~temp_dir:out_dir
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

let gen_shard_facts ns ~ownership ~shard_name all_hashes =
  let progress = Fact_acc.init ~ownership in
  let list_hashes = Md5.Set.to_list all_hashes in
  if ownership then Fact_acc.set_ownership_unit progress (Some ".hhconfig");
  List.fold ns ~init:progress ~f:(fun progress (from, to_) ->
      Add_fact.global_namespace_alias progress ~from ~to_ |> snd)
  |> Add_fact.indexerInputsHash shard_name list_hashes
  |> snd
  |> Fact_acc.to_json

let write_json
    (ctx : Provider_context.t)
    (ownership : bool)
    (out_dir : string)
    (files_info : File_info.t list)
    (start_time : float) : JobReturn.t =
  (* Large file may lead to large json files which timeout when sent
     to the server. Not an issue currently, but if it is, we can index
     xrefs/decls separately, or split in batches according to files size *)
  (List.iter files_info ~f:(fun File_info.{ tast; path; _ } ->
       if List.length tast > 2000 then Hh_logger.log "Large file: %s" path);
   try
     let json_chunks =
       Symbol_index_batch.build_json ctx files_info ~ownership
     in
     write_file out_dir (List.length files_info) json_chunks
   with
   | WorkerCancel.Worker_should_exit as exn ->
     (* Cancellation requests must be re-raised *)
     let e = Exception.wrap exn in
     Exception.reraise e
   | e ->
     Printf.eprintf "WARNING: symbol write failure: \n%s\n" (Exn.to_string e));
  let elapsed = Unix.gettimeofday () -. start_time in
  log_elapsed "Processed batch in" elapsed;
  let hashes =
    List.filter_map files_info ~f:(fun file_info ->
        file_info.File_info.sym_hash)
    |> Md5.Set.of_list
  in
  JobReturn.{ elapsed; hashes; reindexed = SSet.empty }

let recheck_job
    (ctx : Provider_context.t)
    (out_dir : string)
    (root_path : string)
    (hhi_path : string)
    ~(gen_sym_hash : bool)
    ~(incremental : Sym_hash.t option)
    (ownership : bool)
    (_ : JobReturn.t)
    (progress : Indexable.t list) : JobReturn.t =
  let start_time = Unix.gettimeofday () in
  let gen_sym_hash = gen_sym_hash || Option.is_some incremental in
  let files_info =
    List.map
      progress
      ~f:(File_info.create ctx ~root_path ~hhi_path ~gen_sym_hash)
  in
  let reindex f =
    match (f.File_info.fanout, incremental, f.File_info.sym_hash) with
    | (true, Some table, Some hash) when Sym_hash.mem table hash -> false
    | _ -> true
  in
  let to_reindex = List.filter ~f:reindex files_info in
  let res = write_json ctx ownership out_dir to_reindex start_time in
  let fanout_reindexed =
    let f File_info.{ path; fanout; _ } =
      if fanout then
        Some path
      else
        None
    in
    List.filter_map to_reindex ~f |> SSet.of_list
  in
  JobReturn.{ res with reindexed = fanout_reindexed }

let index_files ctx ~out_dir ~files =
  let idx = List.map files ~f:Indexable.from_file in
  recheck_job
    ctx
    out_dir
    "www"
    "hhi"
    ~gen_sym_hash:false
    ~incremental:None
    false
    JobReturn.neutral
    idx
  |> ignore

(* TODO create a type for all these options *)
let go
    (workers : MultiWorker.worker list option)
    (ctx : Provider_context.t)
    ~(namespace_map : (string * string) list)
    ~(gen_sym_hash : bool)
    ~(ownership : bool)
    ~(out_dir : string)
    ~(root_path : string)
    ~(hhi_path : string)
    ~(incremental : Sym_hash.t option)
    ~(files : Indexable.t list) : unit =
  let num_workers =
    match workers with
    | Some w -> List.length w
    | None -> 1
  in
  let start_time = Unix.gettimeofday () in
  let jobs =
    MultiWorker.call
      workers
      ~job:
        (recheck_job
           ctx
           out_dir
           root_path
           hhi_path
           ~gen_sym_hash
           ~incremental
           ownership)
      ~merge:JobReturn.merge
      ~next:(Bucket.make ~num_workers ~max_size:115 files)
      ~neutral:JobReturn.neutral
  in
  (* just a uid *)
  let shard_name =
    Md5.digest_string (Float.to_string start_time) |> Md5.to_hex
  in
  if gen_sym_hash then
    let shard_facts =
      gen_shard_facts namespace_map ~ownership ~shard_name jobs.JobReturn.hashes
    in
    write_file out_dir 1 shard_facts
  else
    ();
  SSet.iter (Hh_logger.log "Reindexed: %s") jobs.JobReturn.reindexed;
  let cumulated_elapsed = jobs.JobReturn.elapsed in
  log_elapsed "Processed all batches (cumulated time) in " cumulated_elapsed;
  let elapsed = Unix.gettimeofday () -. start_time in
  log_elapsed "Processed all batches in " elapsed
