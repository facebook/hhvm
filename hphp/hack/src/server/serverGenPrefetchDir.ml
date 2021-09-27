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

let ( >|= ) = Future.Promise.bind

let return_ok x = Future.Promise.return (Ok x)

let return_err x = Future.Promise.return (Error x)

let get_shallow_decls_filename (filename : string) : string =
  filename ^ ".shallow.bin"

let get_folded_decls_filename (filename : string) : string =
  filename ^ ".folded.bin"

let save_contents (output_filename : string) (contents : 'a) : unit =
  let chan = Stdlib.open_out_bin output_filename in
  Marshal.to_channel chan contents [];
  Stdlib.close_out chan

let get_decls_from_file (file : Relative_path.t) (ctx : Provider_context.t) :
    (string * Int64.t) list =
  match
    Direct_decl_utils.direct_decl_parse_and_cache
      ~file_decl_hash:true
      ~symbol_decl_hashes:true
      ctx
      file
  with
  | None -> []
  | Some (decls, _, _, decl_hashes) ->
    List.map
      ~f:(fun ((name, _decl), decl_hash) -> (name, Option.value_exn decl_hash))
      (List.zip_exn decls decl_hashes)

let dump_folded_decls
    (ctx : Provider_context.t) (dir : string) (path : Relative_path.t) : unit =
  let decls_in_file = get_decls_from_file path ctx in
  List.iter
    ~f:(fun (name, decl_hash) ->
      let folded_decls_in_file =
        Decl_export.collect_legacy_decls ctx (SSet.singleton name)
      in
      let folded_decls_dir = Filename.concat dir "folded_decls" in
      let file =
        Filename.concat folded_decls_dir @@ Int64.to_string decl_hash
      in
      Sys_utils.mkdir_p (Filename.dirname file);
      save_contents (get_folded_decls_filename file) folded_decls_in_file)
    decls_in_file

let dump_shallow_decls
    (ctx : Provider_context.t)
    (genv : ServerEnv.genv)
    (dir : string)
    (path : Relative_path.t) : unit =
  let decls_in_file = get_decls_from_file path ctx in
  List.iter
    ~f:(fun (name, decl_hash) ->
      let shallow_decls_in_file =
        Decl_export.collect_shallow_decls
          ctx
          genv.ServerEnv.workers
          (SSet.singleton name)
      in
      let shallow_decls_dir = Filename.concat dir "shallow_decls" in
      let file =
        Int64.to_string decl_hash |> Filename.concat shallow_decls_dir
      in
      Sys_utils.mkdir_p (Filename.dirname file);
      save_contents (get_shallow_decls_filename file) shallow_decls_in_file)
    decls_in_file

let get_project_metadata ~(repo : Path.t) ~ignore_hh_version :
    (string, string) result Future.Promise.t =
  let hhconfig_path =
    Path.to_string
      (Path.concat repo Config_file.file_path_relative_to_repo_root)
  in
  (if Disk.file_exists hhconfig_path then
    return_ok (Config_file.parse_hhconfig hhconfig_path ~silent:true)
  else
    let error =
      "Attempted to parse .hhconfig.\nBut it doesn't exist at this path:\n"
      ^ hhconfig_path
    in
    return_err error)
  >>= fun (hhconfig_hash, config) ->
  (* Determine hh_server_hash. *)
  begin
    let derived_ignore_hh_version =
      (* Explicitly told to ignore hh_server_hash!
         NOTE: this enables downloading of saved states on unreleased versions
         that do have server hashes *)
      ignore_hh_version
      (* Buck development build hashes are empty. *)
      || String.equal Build_id.build_revision ""
      (* Dune build hashes are short. *)
      || String.length Build_id.build_revision <= 16
    in
    if not derived_ignore_hh_version then
      return_ok Build_id.build_revision
    else
      let version = Config_file.Getters.string_opt "version" config in
      begin
        match version with
        | None ->
          let error =
            Printf.sprintf
              "Couldn't find hh_server version hash.\nbuild_revision=%s\nignore_hh_version=%b\nderived_ignore_hh_version=%b\n'version=' absent from .hhconfig"
              Build_id.build_revision
              ignore_hh_version
              derived_ignore_hh_version
          in
          return_err error
        | Some version ->
          let version = "v" ^ String_utils.lstrip version "^" in
          return_ok version
      end
      >>= fun hh_version ->
      let hh_server_path =
        Printf.sprintf
          "/usr/local/fbprojects/packages/hack/%s/hh_server"
          hh_version
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
  end
  >>= fun hh_server_hash ->
  let project_metadata = Printf.sprintf "%s-%s" hhconfig_hash hh_server_hash in
  return_ok project_metadata

let get_changed_files_since_last_saved_state ~(ignore_hh_version : bool) :
    (Relative_path.t list, string) result Future.Promise.t =
  let saved_state_type =
    Saved_state_loader.Naming_and_dep_table { is_64bit = true }
  in
  let root = Wwwroot.get None in
  let project_name = Saved_state_loader.get_project_name saved_state_type in
  get_project_metadata ~repo:root ~ignore_hh_version >>= fun project_metadata ->
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

let go (env : ServerEnv.env) (genv : ServerEnv.genv) (dir : string) : unit =
  let ctx = Provider_utils.ctx_from_server_env env in
  (* TODO: Make this more robust. I should be able to get the root from somewhere... *)
  let root = Wwwroot.get None in
  let mergebase_rev =
    match Future.get @@ Hg.current_mergebase_hg_rev (Path.to_string root) with
    | Ok hash -> hash
    | Error _ -> failwith "Exception getting the current mergebase revision"
  in
  let dir = Filename.concat dir mergebase_rev in
  let changed_files_since_last_saved_state =
    get_changed_files_since_last_saved_state
      ~ignore_hh_version:(ServerArgs.ignore_hh_version genv.ServerEnv.options)
  in
  let changed_files : Relative_path.t list =
    match Future.get changed_files_since_last_saved_state with
    | Ok (Ok files) -> files
    | Ok (Error e) -> failwith (Printf.sprintf "%s" e)
    | Error e -> failwith (Printf.sprintf "%s" (Future.error_to_string e))
  in
  let changed_files =
    List.filter_map
      ~f:(fun path ->
        if Filename.check_suffix (Relative_path.suffix path) ".php" then
          Some path
        else
          None)
      changed_files
  in
  List.iter
    ~f:(fun path ->
      dump_shallow_decls ctx genv dir path;
      dump_folded_decls ctx dir path)
    changed_files
