(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Module for getting old decls remotely. *)
(*****************************************************************************)
open Hh_prelude

let get_hh_version () =
  let repo = Wwwroot.get None in
  let hhconfig_path =
    Path.to_string
      (Path.concat repo Config_file.file_path_relative_to_repo_root)
  in
  let version =
    if Disk.file_exists hhconfig_path then
      let (_, config) = Config_file.parse_hhconfig hhconfig_path in
      Config_file.Getters.string_opt "version" config
    else
      None
  in
  match version with
  | None -> Hh_version.version
  | Some version ->
    let version = "v" ^ String_utils.lstrip version "^" in
    version

let db_path_of_ctx (ctx : Provider_context.t) : Naming_sqlite.db_path option =
  ctx |> Provider_context.get_backend |> Db_path_provider.get_naming_db_path

let name_to_decl_hash_opt ~(name : string) ~(db_path : Naming_sqlite.db_path) =
  let dep = Typing_deps.(Dep.make (Dep.Type name)) in
  let decl_hash = Naming_sqlite.get_decl_hash_by_64bit_dep db_path dep in
  if Option.is_some decl_hash then
    Hh_logger.log
      "Attempting to fetch old decl with decl hash %s remotely"
      (Option.value_exn decl_hash)
  else
    Hh_logger.log
      "Couldn't find decl hash for %s in sqlite naming table. Not fetching its old decl."
      name;
  decl_hash

module FetchAsync = struct
  (** The input record that gets passed from the main process to the daemon process *)
  type input = {
    hh_config_version: string;
    destination_path: string;
    no_limit: bool;
    decl_hashes: string list;
  }

  (** The main entry point of the daemon process that fetches the remote old decl blobs
      and writes them to a file. *)
  let fetch { hh_config_version; destination_path; no_limit; decl_hashes } :
      unit =
    begin
      match
        Remote_old_decls_ffi.get_decls hh_config_version no_limit decl_hashes
      with
      | Ok decl_blobs ->
        let chan = Stdlib.open_out_bin destination_path in
        Marshal.to_channel chan decl_blobs [];
        Stdlib.close_out chan
      | Error msg -> Hh_logger.log "Error fetching remote decls: %s" msg
    end;

    (* The intention here is to force the daemon process to exit with
       an failed exit code so that the main process can detect
       the condition and log the outcome. *)
    assert (Disk.file_exists destination_path)

  (** The daemon entry registration - used by the main process *)
  let fetch_entry =
    Process.register_entry_point "remote_old_decls_fetch_async_entry" fetch
end

let fetch_async ~hh_config_version ~destination_path ~no_limit decl_hashes =
  Hh_logger.log "Fetching remote old decls to %s" destination_path;
  let open FetchAsync in
  FutureProcess.make
    (Process.run_entry
       Process_types.Default
       fetch_entry
       { hh_config_version; destination_path; no_limit; decl_hashes })
    (fun _output -> Hh_logger.log "Finished fetching remote old decls")

let fetch_old_decls
    ~(telemetry_label : string)
    ~(ctx : Provider_context.t)
    (names : string list) : Shallow_decl_defs.shallow_class option SMap.t =
  let db_path_opt = db_path_of_ctx ctx in
  match db_path_opt with
  | None -> SMap.empty
  | Some db_path ->
    let decl_hashes =
      List.filter_map
        ~f:(fun name -> name_to_decl_hash_opt ~name ~db_path)
        names
    in
    let hh_config_version = get_hh_version () in
    let start_t = Unix.gettimeofday () in
    let no_limit =
      TypecheckerOptions.remote_old_decls_no_limit
        (Provider_context.get_tcopt ctx)
    in
    let tmp_dir = Tempfile.mkdtemp ~skip_mocking:false in
    let destination_path = Path.(to_string @@ concat tmp_dir "decl_blobs") in
    let decl_fetch_future =
      fetch_async ~hh_config_version ~destination_path ~no_limit decl_hashes
    in
    (match Future.get ~timeout:120 decl_fetch_future with
    | Error e ->
      Hh_logger.log
        "Failed to fetch decls from remote decl store: %s"
        (Future.error_to_string e);
      SMap.empty
    | Ok () ->
      let chan = Stdlib.open_in_bin destination_path in
      let decl_blobs : string list = Marshal.from_channel chan in
      Stdlib.close_in chan;
      let decls =
        List.fold
          ~init:SMap.empty
          ~f:(fun acc blob ->
            let contents : Shallow_decl_defs.shallow_class SMap.t =
              Marshal.from_string blob 0
            in
            SMap.fold
              (fun name cls acc -> SMap.add name (Some cls) acc)
              contents
              acc)
          decl_blobs
      in
      let telemetry = Telemetry.create () in
      let fetch_results =
        Telemetry.create ()
        |> Telemetry.int_ ~key:"to_fetch" ~value:(List.length names)
        |> Telemetry.int_ ~key:"fetched" ~value:(SMap.cardinal decls)
      in
      Hh_logger.log
        "Fetched %d/%d decls remotely"
        (SMap.cardinal decls)
        (List.length names);

      let telemetry =
        Telemetry.object_ telemetry ~key:telemetry_label ~value:fetch_results
      in
      HackEventLogger.remote_old_decl_end telemetry start_t;
      decls)
