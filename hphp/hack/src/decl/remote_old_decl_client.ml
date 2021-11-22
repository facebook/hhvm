(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Module for getting old decls.
  - Currently we rely on hh --gen-prefetch-dir <remote decl store> to generate
    old decls stored on disk.
  - We fetch the old decls "remotely" from this directory.
  - Eventually we will fetch the decls from memcache/manifold.
 *)
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
      let (_, config) = Config_file.parse_hhconfig hhconfig_path ~silent:true in
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
      (Option.value_exn decl_hash);
  decl_hash

let fetch_old_decls ~(ctx : Provider_context.t) (names : string list) :
    Shallow_decl_defs.shallow_class option SMap.t =
  let db_path_opt =
    db_path_of_ctx ctx
    (*
    Note:
      Currently only the naming table builder generates naming tables with decl
      hashes. The saved-state naming table contains only zero-valued decl hashes.

      For testing this function,
      1. use hh_naming_table_builder to build a naming table at /tmp/naming.sql:
        ```
        buck run //hphp/hack/src/facebook/naming_table_builder:hh_naming_table_builder \
        -- --use-direct-decl-parser --overwrite --www ~/www --output /tmp/naming.sql
        ```
      2. uncomment the following lines to extract the decl hash from /tmp/naming.sql
        ```
        let _ = db_path_opt in
        let path = Filename.concat Sys_utils.temp_dir_name "naming.sql" in
        let db_path_opt = Some (Naming_sqlite.Db_path path) in
        ```
  *)
  in
  match db_path_opt with
  | None -> SMap.empty
  | Some db_path ->
    let decl_hashes =
      List.filter_map
        ~f:(fun name -> name_to_decl_hash_opt ~name ~db_path)
        names
    in
    let hh_config_version = get_hh_version () in
    let decl_blobs =
      Remote_old_decls_ffi.get_decls hh_config_version decl_hashes
    in
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
    decls
