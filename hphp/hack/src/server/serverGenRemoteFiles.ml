(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

(*
 * This module is used for sub1m testing. Usage:
 *   1) Run hh --gen-remote-files
 *   2) This will populate a remote file store at hack_sub1m_file_store/tree/v0.0.1/files
 * The sub1m remote workers will prefetch files from the above manifold directory
 *)
open Hh_prelude

let go (genv : ServerEnv.genv) : unit =
  (* Keep the hhconfig version (e.g. v0.0.1) consistent between decl and file prefetching *)
  let hh_config_version = "v0.0.1" in
  let manifold_dir =
    "hack_sub1m_file_store/tree/" ^ hh_config_version ^ "/files"
  in
  let cmd = "manifold mkdirs " ^ manifold_dir in
  ignore (Sys.command cmd);

  let get_next =
    ServerUtils.make_next
      ~hhi_filter:(fun _ -> true)
      ~indexer:(genv.ServerEnv.indexer FindUtils.file_filter)
      ~extra_roots:(ServerConfig.extra_paths genv.ServerEnv.config)
  in

  let job (acc : (string * string) list) (fnl : Relative_path.t list) :
      (string * string) list =
    List.fold_left
      ~init:acc
      ~f:(fun acc fn ->
        let filename = Relative_path.to_absolute fn in
        if Disk.file_exists filename then
          let file_contents = Sys_utils.cat filename in
          let file_hash = SharedMemHash.hash_string filename in
          let content_sha1 = Sha1.digest file_contents in
          ( Int64.to_string file_hash ^ content_sha1,
            Marshal.to_string (fn, file_contents) [] )
          :: acc
        else (
          Hh_logger.log "%s doesn't exist" filename;
          acc
        ))
      fnl
  in

  let results =
    MultiWorker.call
      genv.ServerEnv.workers
      ~job
      ~neutral:[]
      ~merge:List.append
      ~next:get_next
  in

  Hh_logger.log "Uploading %d files to %s" (List.length results) manifold_dir;
  let _ = Remote_files_ffi.put_files hh_config_version results in
  ()
