(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let ( >>= ) res f =
  Future.Promise.bind res (function
      | Ok r -> f r
      | Error e -> Future.Promise.return (Error e))

let return_ok x = Future.Promise.return (Ok x)

let return_err x = Future.Promise.return (Error x)

let get_hh_config_version ~(root : Path.t) :
    (string, string) result Future.Promise.t =
  let hhconfig_path =
    Path.to_string
      (Path.concat root Config_file.file_path_relative_to_repo_root)
  in
  (if Disk.file_exists hhconfig_path then
    return_ok (Config_file.parse_hhconfig hhconfig_path)
  else
    let error =
      "Attempted to parse .hhconfig.\nBut it doesn't exist at this path:\n"
      ^ hhconfig_path
    in
    return_err error)
  >>= fun (_, config) ->
  let version = Config_file.Getters.string_opt "version" config in
  begin
    match version with
    | None -> failwith "Failed to parse hh version"
    | Some version ->
      let version = "v" ^ String_utils.lstrip version "^" in
      return_ok version
  end
  >>= fun hh_config_version -> return_ok hh_config_version

let go (genv : ServerEnv.genv) : unit =
  let root = Wwwroot.get None in
  let hh_config_version =
    match Future.get @@ get_hh_config_version ~root with
    | Ok (Ok result) -> result
    | Ok (Error e) -> failwith (Printf.sprintf "%s" e)
    | Error e -> failwith (Printf.sprintf "%s" (Future.error_to_string e))
  in

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
            Marshal.to_string (filename, file_contents) [] )
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
