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

let save_contents (output_filename : string) (contents : 'a) : unit =
  let chan = Stdlib.open_out_bin output_filename in
  Marshal.to_channel chan contents [];
  Stdlib.close_out chan

let load_contents (input_filename : string) : 'a =
  let chan = Stdlib.open_in_bin input_filename in
  let contents = Marshal.from_channel chan in
  Stdlib.close_in chan;
  contents

let get_name_and_decl_hashes_from_decls decls : (string * Int64.t) list =
  List.filter_map decls ~f:(fun (name, decl, decl_hash) ->
      match decl with
      | Shallow_decl_defs.Class _ -> Some (name, decl_hash)
      | _ -> None)

let get_hh_config_version ~(repo : Path.t) :
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
  >>= fun hh_config_version -> return_ok hh_config_version

let go
    (env : ServerEnv.env)
    (genv : ServerEnv.genv)
    (workers : MultiWorker.worker list option)
    (dir : string) : unit =
  let ctx = Provider_utils.ctx_from_server_env env in
  let shallow_decl_file_name = "shallow_decls.bin" in
  let local_file = dir ^ "/" ^ shallow_decl_file_name in

  let get_next =
    ServerUtils.make_next
      ~hhi_filter:(fun _ -> true)
      ~indexer:(genv.ServerEnv.indexer FindUtils.file_filter)
      ~extra_roots:(ServerConfig.extra_paths genv.ServerEnv.config)
  in

  let job acc (fnl : Relative_path.t list) =
    let acc =
      List.fold_left
        ~init:acc
        ~f:(fun acc fn ->
          Hh_logger.log
            "Saving decls for prefetching: %s"
            (Relative_path.suffix fn);
          match Direct_decl_utils.direct_decl_parse ctx fn with
          | None -> acc
          | Some parsed_file ->
            let decls = parsed_file.Direct_decl_parser.pfh_decls in
            Direct_decl_utils.cache_decls ctx fn decls;
            let names_and_decl_hashes =
              get_name_and_decl_hashes_from_decls decls
            in
            List.fold_left
              ~init:acc
              ~f:(fun acc (name, decl_hash) ->
                let shallow_decl_opt = Shallow_classes_provider.get ctx name in
                if Option.is_some shallow_decl_opt then
                  let shallow_decl = Option.value_exn shallow_decl_opt in
                  List.cons
                    (* marshal here to avoid OOM *)
                    (decl_hash, Marshal.to_string shallow_decl [])
                    acc
                else
                  acc)
              names_and_decl_hashes)
        fnl
    in
    acc
  in

  (* results is a list of (decl_hash, marshalled { symbol: decl }) pairs *)
  let results =
    I64Map.of_list
      (MultiWorker.call
         workers
         ~job
         ~neutral:[]
         ~merge:List.append
         ~next:get_next)
  in
  let _ = Hh_logger.log "Processed %d decls" (I64Map.cardinal results) in

  save_contents local_file results;
  (* save_contents local_file (I64Map.of_list results); *)
  let _ = Hh_logger.log "Saved to local file %s" local_file in
  ()
