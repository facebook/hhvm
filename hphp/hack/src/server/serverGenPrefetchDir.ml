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

let save_contents (output_filename : string) (contents : 'a) : unit =
  let chan = Stdlib.open_out_bin output_filename in
  Marshal.to_channel chan contents [];
  Stdlib.close_out chan

let get_name_and_decl_hashes_from_decls
    ((decls, _, _, symbol_decl_hashes) :
      Direct_decl_parser.decls
      * FileInfo.mode option
      * Int64.t option
      * Int64.t option list) : (string * Int64.t) list =
  let add acc ((name, decl), decl_hash) =
    match decl with
    | Shallow_decl_defs.Class _ ->
      List.rev_append acc [(name, Option.value_exn decl_hash)]
    | _ -> acc
  in
  List.fold ~f:add ~init:[] (List.zip_exn decls symbol_decl_hashes)

let get_project_metadata ~(repo : Path.t) ~ignore_hh_version :
    (string * string, string) result Future.Promise.t =
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
      return_ok (Build_id.build_revision, Hh_version.version)
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
            return_ok (hash, hh_version)
          | Error process_failure ->
            let error =
              Printf.sprintf
                "Attempted to invoke hh_server to get version.\n`%s`\nRESPONSE:\n%s"
                cmd
                process_failure
            in
            return_err error)
  end
  >>= fun (hh_server_hash, hh_version) ->
  let project_metadata = Printf.sprintf "%s-%s" hhconfig_hash hh_server_hash in
  return_ok (project_metadata, hh_version)

let go
    (env : ServerEnv.env)
    (genv : ServerEnv.genv)
    (dir : string)
    (workers : MultiWorker.worker list option) : unit =
  let ctx = Provider_utils.ctx_from_server_env env in
  let repo = Wwwroot.get None in
  let ignore_hh_version = ServerArgs.ignore_hh_version genv.ServerEnv.options in
  let (_, hh_version) =
    match Future.get @@ get_project_metadata ~repo ~ignore_hh_version with
    | Ok (Ok result) -> result
    | Ok (Error e) -> failwith (Printf.sprintf "%s" e)
    | Error e -> failwith (Printf.sprintf "%s" (Future.error_to_string e))
  in
  let dir = Filename.concat dir hh_version in

  let get_next =
    ServerUtils.make_next
      ~hhi_filter:(fun _ -> true)
      ~indexer:(genv.ServerEnv.indexer FindUtils.file_filter)
      ~extra_roots:(ServerConfig.extra_paths genv.ServerEnv.config)
  in

  let job (acc : Int64.t list) (fnl : Relative_path.t list) : Int64.t list =
    List.fold_left
      ~init:acc
      ~f:(fun acc fn ->
        match
          Direct_decl_utils.direct_decl_parse
            ~file_decl_hash:true
            ~symbol_decl_hashes:true
            ctx
            fn
        with
        | None -> acc
        | Some ((decls, _, _, _) as decl_and_mode_and_hash) ->
          Direct_decl_utils.cache_decls ctx decls;
          let names_and_decl_hashes =
            get_name_and_decl_hashes_from_decls decl_and_mode_and_hash
          in
          List.fold_left
            ~init:acc
            ~f:(fun acc (name, decl_hash) ->
              let shallow_decl_opt = Shallow_classes_provider.get ctx name in
              if Option.is_some shallow_decl_opt then (
                let shallow_decls_in_file = Option.value_exn shallow_decl_opt in
                let shallow_decls_dir = Filename.concat dir "shallow_decls" in
                let file =
                  Int64.to_string decl_hash |> Filename.concat shallow_decls_dir
                in
                Sys_utils.mkdir_p (Filename.dirname file);
                save_contents
                  (get_shallow_decls_filename file)
                  shallow_decls_in_file;
                List.rev_append acc [decl_hash]
              ) else
                acc)
            names_and_decl_hashes)
      fnl
  in

  let results =
    MultiWorker.call
      workers
      ~job
      ~neutral:[]
      ~merge:List.rev_append
      ~next:get_next
  in
  Hh_logger.log "Processed %d decls" (List.length results);
  ()
