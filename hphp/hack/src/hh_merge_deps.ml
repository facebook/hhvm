(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let sharedmem_config =
  let gig = 1024 * 1024 * 1024 in
  {
    SharedMem.global_size = gig;
    heap_size = 40 * gig;
    dep_table_pow = 28;
    (* 1 << 28 *)
    hash_table_pow = 26;
    (* 1 << 26 *)
    shm_dirs = [GlobalConfig.shm_dir; GlobalConfig.tmp_dir];
    shm_min_avail = gig / 2;
    (* Half a gig by default *)
    log_level = 0;
    sample_rate = 0.0;
    compression = 0;
  }

let merge_worker ((filename : string), (ignore_hh_version : bool)) : unit =
  try
    Hh_logger.log "Merging '%s'" filename;
    let (num_rows : int) =
      SharedMem.load_dep_table_blob filename ignore_hh_version
    in
    Hh_logger.log "Rows read: %d" num_rows
  with e ->
    let stack = Printexc.get_backtrace () in
    Hh_logger.exc ~prefix:(Printf.sprintf "Error in '%s'" filename) ~stack e

let merge_input
    ~(filenames : string list) ~(ignore_hh_version : bool) ~(db_name : string) :
    unit =
  let (input : (string * bool) list) =
    List.map (fun filename -> (filename, ignore_hh_version)) filenames
  in
  let () = List.iter merge_worker input in
  let (edges_added : int) =
    SharedMem.save_dep_table_sqlite db_name Build_id.build_revision false
  in
  Hh_logger.log "Edges added: %d" edges_added

let () =
  let (_handle : SharedMem.handle) =
    SharedMem.init ~num_workers:0 sharedmem_config
  in
  let (filenames : string list ref) = ref [] in
  let (ignore_hh_version : bool ref) = ref true in
  let (db_name : string ref) = ref "" in
  let options =
    Arg.align
      [
        ( "--ignore-hh-version",
          Arg.Bool (fun b -> ignore_hh_version := b),
          " Indicates whether to ignore the Hack server version" );
        ( "--output-filename",
          Arg.String (fun s -> db_name := s),
          " The name of the SQLite file to save the merged dependency table into"
        );
      ]
  in
  let usage =
    "Usage: "
    ^ Sys.executable_name
    ^ " [dep_table_fragment_1 ... dep_table_fragment_N]"
  in
  Arg.parse options (fun filename -> filenames := filename :: !filenames) usage;

  merge_input
    ~filenames:!filenames
    ~ignore_hh_version:!ignore_hh_version
    ~db_name:!db_name
