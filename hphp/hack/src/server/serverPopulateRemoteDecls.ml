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
    (string, string) result Future.Promise.t =
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
  >>= fun (_, config) ->
  let version = Config_file.Getters.string_opt "version" config in
  begin
    match version with
    | None -> failwith "Failed to parse hh version"
    | Some version ->
      let version = "v" ^ String_utils.lstrip version "^" in
      return_ok version
  end
  >>= fun hhconfig_version -> return_ok hhconfig_version

let get_version ~repo =
  if Build_id.is_dev_build then
    Remote_old_decl_client.Utils.get_dev_build_version ()
  else
    match Future.get @@ get_hhconfig_version ~repo with
    | Ok (Ok result) -> result
    | Ok (Error e) -> failwith (Printf.sprintf "%s" e)
    | Error e -> failwith (Printf.sprintf "%s" (Future.error_to_string e))

let go
    (env : ServerEnv.env)
    (genv : ServerEnv.genv)
    (workers : MultiWorker.worker list option) : unit =
  let ctx = Provider_utils.ctx_from_server_env env in
  (* TODO: the following is a bug! *)
  let repo = Wwwroot.interpret_command_line_root_parameter [] in
  let version = get_version ~repo in
  let manifold_dir = Remote_old_decl_client.Utils.make_manifold_path ~version in
  Hh_logger.log "Will upload to manifold directory %s" manifold_dir;
  let cmd = Printf.sprintf "manifold mkdirs %s" manifold_dir in
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
        Hh_logger.log
          "Saving decls for prefetching: %s"
          (Relative_path.suffix fn);
        match Direct_decl_utils.direct_decl_parse ctx fn with
        | None -> acc
        | Some parsed_file ->
          let class_decls = parsed_file.Direct_decl_parser.pfh_decls in
          let decls_to_upload =
            List.map class_decls ~f:(fun (name, decl, decl_hash) ->
                let decl_hash_64 = Int64.to_string decl_hash in
                let symbol_to_shallow_decl = SMap.singleton name decl in
                let marshalled_symbol_to_shallow_decl =
                  Marshal.to_string symbol_to_shallow_decl []
                in
                (decl_hash_64, marshalled_symbol_to_shallow_decl))
          in
          decls_to_upload @ acc)
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

  let _ = Remote_old_decls_ffi.put_decls ~silent:false version results in
  Hh_logger.log "Processed %d decls" (List.length results);
  ()
