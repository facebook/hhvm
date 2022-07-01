(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

(*
 * This module is used in saved state jobs to create two primary artifacts for
 * each changed file in a mergebase commit
 *  1) a marshalled ocaml blob representing the decls in said file
 *  2) a json blob representing the "fan in" or in other words the decls
 *     needed to typecheck said file
 *)

open Hh_prelude

let ( >>= ) res f =
  Future.Promise.bind res (function
      | Ok r -> f r
      | Error e -> Future.Promise.return (Error e))

let return_ok x = Future.Promise.return (Ok x)

let return_err x = Future.Promise.return (Error x)

let get_hhconfig_version ~(repo : Path.t) :
    (string * string, string) result Future.Promise.t =
  let hhconfig_path =
    Path.to_string
      (Path.concat repo Config_file.file_path_relative_to_repo_root)
  in
  (if Disk.file_exists hhconfig_path then
    return_ok (Config_file.parse_hhconfig hhconfig_path)
  else
    let error =
      "Attempted to parse .hhconfig.\nBut it doesn't exist at this path:\n"
      ^ hhconfig_path
    in
    return_err error)
  >>= fun (hhconfig_hash, config) ->
  let version = Config_file.Getters.string_opt "version" config in
  match version with
  | None -> failwith "Failed to parse hh version"
  | Some version ->
    let version = "v" ^ String_utils.lstrip version "^" in
    return_ok (hhconfig_hash, version)

let get_project_metadata ~hhconfig_hash ~hhconfig_version :
    (string, string) result Future.Promise.t =
  let hh_server_path =
    Printf.sprintf
      "/usr/local/fbprojects/packages/hack/%s/hh_server"
      hhconfig_version
  in
  let cmd = Printf.sprintf "%s --version" hh_server_path in
  let process =
    Process.exec (Exec_command.Hh_server hh_server_path) ["--version"]
  in
  let future = FutureProcess.make process (fun str -> str) in
  let process_result =
    Future.continue_and_map_err future (function
        | Ok stdout -> Ok stdout
        | Error e -> Error (Future.error_to_string e))
  in
  Future.Promise.bind process_result (function
      | Ok stdout ->
        let hash = String.slice stdout 0 40 in
        return_ok hash
      | Error process_failure ->
        let error =
          Printf.sprintf
            "Attempted to invoke hh_server to get version.\n`%s`\nRESPONSE:\n%s"
            cmd
            process_failure
        in
        return_err error)
  >>= fun hh_server_hash ->
  let project_metadata = Printf.sprintf "%s-%s" hhconfig_hash hh_server_hash in
  return_ok project_metadata

let get_changed_files_since_last_saved_state
    ~(hhconfig_hash : string) ~(hhconfig_version : string) :
    (Relative_path.t list, string) result Future.Promise.t =
  let saved_state_type =
    (* TODO: using shallow_decls_saved_state *)
    Saved_state_loader.Naming_and_dep_table { naming_sqlite = false }
  in
  let root = Wwwroot.get None in
  let project_name = Saved_state_loader.get_project_name saved_state_type in
  get_project_metadata ~hhconfig_hash ~hhconfig_version
  >>= fun project_metadata ->
  let query =
    Printf.sprintf
      {|
      [
        "query",
        "%s",
        {
          "fields": ["name"],
          "fail_if_no_saved_state": true,
          "since": {
            "scm": {
              "mergebase-with": "remote/master",
              "saved-state": {
                "storage": "manifold",
                "config": {
                  "project": "%s",
                  "project-metadata": "%s"
                }
              }
            }
          }
        }
      ]
    |}
      (Path.to_string root)
      project_name
      project_metadata
  in
  let process_result =
    let process = Process.exec ~input:query Exec_command.Watchman ["-j"] in
    let future = FutureProcess.make process (fun str -> str) in
    Future.continue_and_map_err future (function
        | Ok stdout -> Ok stdout
        | Error e -> Error (Future.error_to_string e))
  in
  Future.Promise.bind process_result (function
      | Ok response ->
        (* The text returned is huge, but let's try json parsing anyway *)
        return_ok (Hh_json.json_of_string response)
      | Error process_failure ->
        let process_failure = String_utils.truncate 2048 process_failure in
        let error =
          Printf.sprintf "QUERY:\n%s\n\nOUTPUT:\n%s" query process_failure
        in
        return_err error)
  >>= fun response_json ->
  let response_parsed =
    match Hh_json.Access.return response_json with
    | Ok response -> Hh_json.Access.get_array "files" response
    | Error _ as e -> e
  in
  begin
    match response_parsed with
    | Ok (files_json, _keytrace) ->
      files_json
      |> List.map ~f:(function
             | Hh_json.JSON_String s -> Ok s
             | _ -> Error "Non-string .files element")
      |> Result.all
      |> Future.Promise.return
    | Error access_failure ->
      "No files: " ^ Hh_json.Access.access_failure_to_string access_failure
      |> return_err
  end
  >>= fun files ->
  let changed_files =
    List.map files ~f:(fun suffix -> Relative_path.from_root ~suffix)
  in
  return_ok changed_files

let go
    (env : ServerEnv.env)
    (genv : ServerEnv.genv)
    (workers : MultiWorker.worker list option)
    ~(incremental : bool) : unit =
  let ctx = Provider_utils.ctx_from_server_env env in
  let repo = Wwwroot.get None in
  let (hhconfig_hash, hhconfig_version) =
    match Future.get @@ get_hhconfig_version ~repo with
    | Ok (Ok result) -> result
    | Ok (Error e) -> failwith (Printf.sprintf "%s" e)
    | Error e -> failwith (Printf.sprintf "%s" (Future.error_to_string e))
  in
  let cmd =
    "manifold mkdirs hack_decl_prefetching/tree/prefetch/"
    ^ hhconfig_version
    ^ "/shallow_decls"
  in
  ignore (Sys.command cmd);

  let get_next : Relative_path.t list Bucket.next =
    if incremental then (
      let changed_files_since_last_saved_state =
        get_changed_files_since_last_saved_state
          ~hhconfig_hash
          ~hhconfig_version
      in
      let changed_files : Relative_path.t list =
        match Future.get changed_files_since_last_saved_state with
        | Ok (Ok files) ->
          List.filter_map
            ~f:(fun path ->
              if Filename.check_suffix (Relative_path.suffix path) ".php" then
                Some path
              else
                None)
            files
        | Ok (Error e) -> failwith (Printf.sprintf "%s" e)
        | Error e -> failwith (Printf.sprintf "%s" (Future.error_to_string e))
      in
      let _ =
        Hh_logger.log
          "incrementally update %d files"
          (List.length changed_files)
      in
      (* created an iterator.
         The function returns `files` in the first call,
         and returns [] in the later calls indicating no more jobs *)
      let files = ref changed_files in
      fun () ->
        let fnl = !files in
        files := [];
        Bucket.of_list fnl
    ) else
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
        Hh_logger.log
          "Saving decls for prefetching: %s"
          (Relative_path.suffix fn);
        match Direct_decl_utils.direct_decl_parse ctx fn with
        | None -> acc
        | Some parsed_file ->
          let class_decls =
            List.filter_map
              parsed_file.Direct_decl_parser.pfh_decls
              ~f:(function
                | (_, Shallow_decl_defs.Class _, _) as decl -> Some decl
                | _ -> None)
          in
          let pf_hash_and_marshalled_declmap =
            List.map class_decls ~f:(fun (name, decl, decl_hash) ->
                let decl_hash_64 = Int64.to_string decl_hash in
                let symbol_to_shallow_decl = SMap.singleton name decl in
                let marshalled_symbol_to_shallow_decl =
                  Marshal.to_string symbol_to_shallow_decl []
                in
                (decl_hash_64, marshalled_symbol_to_shallow_decl))
          in
          acc @ pf_hash_and_marshalled_declmap)
      fnl
  in

  (* results is a list of (decl_hash, marshalled { symbol: decl }) pairs *)
  let results =
    MultiWorker.call
      workers
      ~job
      ~neutral:[]
      ~merge:List.rev_append
      ~next:get_next
  in

  let _ = Remote_old_decls_ffi.put_decls hhconfig_version results in
  Hh_logger.log "Processed %d decls" (List.length results);
  ()
