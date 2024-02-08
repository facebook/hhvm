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

module Utils = struct
  let db_path_of_ctx ~(ctx : Provider_context.t) : Naming_sqlite.db_path option
      =
    ctx |> Provider_context.get_backend |> Db_path_provider.get_naming_db_path

  let name_to_decl_hash_opt ~(name : string) ~(db_path : Naming_sqlite.db_path)
      =
    let dep = Typing_deps.(Dep.make (Dep.Type name)) in
    let decl_hash = Naming_sqlite.get_decl_hash_by_64bit_dep db_path dep in
    decl_hash

  let name_to_file_hash_opt ~(name : string) ~(db_path : Naming_sqlite.db_path)
      =
    let dep = Typing_deps.(Dep.make (Dep.Type name)) in
    let file_hash = Naming_sqlite.get_file_hash_by_64bit_dep db_path dep in
    file_hash
end

let fetch_old_decls_via_file_hashes
    ~(ctx : Provider_context.t)
    ~(db_path : Naming_sqlite.db_path)
    (names : string list) : Shallow_decl_defs.shallow_class option SMap.t =
  (* TODO(bobren): names should really be a list of deps *)
  let file_hashes =
    List.filter_map
      ~f:(fun name -> Utils.name_to_file_hash_opt ~name ~db_path)
      names
  in
  let popt = Provider_context.get_popt ctx in
  let opts = DeclParserOptions.from_parser_options popt in
  match Remote_old_decls_ffi.get_decls_via_file_hashes opts file_hashes with
  | Ok named_old_decls ->
    (* TODO(bobren) do funs typedefs consts and modules *)
    let old_decls =
      List.fold
        ~init:SMap.empty
        ~f:(fun acc ndecl ->
          match ndecl with
          | Shallow_decl_defs.NClass (name, cls) -> SMap.add name (Some cls) acc
          | _ -> acc)
        named_old_decls
    in
    old_decls
  | Error msg ->
    Hh_logger.log "Error fetching remote decls: %s" msg;
    SMap.empty

let fetch_old_decls ~(ctx : Provider_context.t) (names : string list) :
    Shallow_decl_defs.shallow_class option SMap.t =
  let db_path_opt = Utils.db_path_of_ctx ~ctx in
  match db_path_opt with
  | None -> SMap.empty
  | Some db_path ->
    let decl_hashes =
      List.filter_map
        ~f:(fun name -> Utils.name_to_decl_hash_opt ~name ~db_path)
        names
    in
    let start_t = Unix.gettimeofday () in
    let old_decls =
      let use_old_decls_from_cas =
        TypecheckerOptions.use_old_decls_from_cas
          (Provider_context.get_tcopt ctx)
      in
      Hh_logger.log "Using old decls from CAS? %b" use_old_decls_from_cas;
      fetch_old_decls_via_file_hashes ~ctx ~db_path names
    in
    let to_fetch = List.length decl_hashes in
    let telemetry =
      Telemetry.create ()
      |> Telemetry.int_ ~key:"to_fetch" ~value:to_fetch
      |> Telemetry.int_ ~key:"fetched" ~value:(SMap.cardinal old_decls)
    in
    HackEventLogger.remote_old_decl_end telemetry start_t;
    Hh_logger.log
      "Fetched %d/%d decls remotely"
      (SMap.cardinal old_decls)
      to_fetch;
    old_decls
