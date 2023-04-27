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

let go
    (env : ServerEnv.env)
    (genv : ServerEnv.genv)
    (workers : MultiWorker.worker list option)
    (dir : string) : unit =
  let ctx = Provider_utils.ctx_from_server_env env in
  let shallow_decl_file_name = "shallow_decls.bin" in
  let local_file = dir ^ "/" ^ shallow_decl_file_name in
  (* TODO: the following is a bug! *)
  let repo = Wwwroot.interpret_command_line_root_parameter [] in
  let hhconfig_version =
    match Future.get @@ get_hhconfig_version ~repo with
    | Ok (Ok result) -> result
    | Ok (Error e) -> failwith (Printf.sprintf "%s" e)
    | Error e -> failwith (Printf.sprintf "%s" (Future.error_to_string e))
  in
  (* ensure manifold path exists when we run the Remote_old_decls_ffi *)
  let cmd =
    "manifold mkdirs hack_decl_prefetching/tree/prefetch/"
    ^ hhconfig_version
    ^ "/shallow_decls"
  in
  ignore (Sys.command cmd);

  let get_next =
    ServerUtils.make_next
      ~hhi_filter:(fun _ -> true)
      ~indexer:(genv.ServerEnv.indexer FindUtils.file_filter)
      ~extra_roots:(ServerConfig.extra_paths genv.ServerEnv.config)
  in

  (* acc contains two lists
     one is (string * string) representing the data set we send to Manifold
     for the shallow decl service. This is a tuple of decl_hash, marshalled SMap of symbol to shallow decl

     One is Int64 * string) representing  the data set we write to disk for the shallow decl saved state.
     This is a tuple of (decl hash, marshalled shallow decl)
  *)
  let job
      (acc : (string * string) list * (Int64.t * string) list)
      (fnl : Relative_path.t list) =
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
          let (prev_decls_to_upload, prev_decls_to_write) = acc in
          let decls_to_upload =
            List.map decls ~f:(fun (name, decl, decl_hash) ->
                let decl_hash_64 = Int64.to_string decl_hash in
                let symbol_to_shallow_decl = SMap.singleton name decl in
                let marshalled_symbol_to_shallow_decl =
                  Marshal.to_string symbol_to_shallow_decl []
                in
                (decl_hash_64, marshalled_symbol_to_shallow_decl))
          in
          let decls_to_write =
            List.map decls ~f:(fun (name, decl, decl_hash) ->
                (decl_hash, Marshal.to_string (name, decl) []))
          in
          ( prev_decls_to_upload @ decls_to_upload,
            prev_decls_to_write @ decls_to_write ))
      fnl
  in

  let tuple_list_merge
      (arg_fst : (string * string) list * (Int64.t * string) list)
      (arg_snd : (string * string) list * (Int64.t * string) list) =
    let (manifold_fst, disk_fst) = (fst arg_fst, snd arg_fst) in
    let (manifold_snd, disk_snd) = (fst arg_snd, snd arg_snd) in
    (manifold_fst @ manifold_snd, disk_fst @ disk_snd)
  in

  (* results is a list of (decl_hash, marshalled { symbol: decl }) pairs *)
  let (manifold_results, disk_results) =
    MultiWorker.call
      workers
      ~job
      ~neutral:([], [])
      ~merge:tuple_list_merge
      ~next:get_next
  in
  let disk_results = I64Map.of_list disk_results in
  let _ = Hh_logger.log "Processed %d decls" (I64Map.cardinal disk_results) in
  (* save_contents local_file (I64Map.of_list results); *)
  save_contents local_file disk_results;
  Hh_logger.log "Saved to local file %s" local_file;
  (* ensure we save decls to shallow decls service as well
     invariant: all decls are successfully uploaded or we fail *)
  match
    Remote_old_decls_ffi.put_decls
      ~silent:false
      hhconfig_version
      manifold_results
  with
  | Error msg ->
    failwith (Printf.sprintf "Failed to upload decls with error: %s" msg)
  | Ok _ ->
    Hh_logger.log "Processed %d results" (List.length manifold_results);
    ()
