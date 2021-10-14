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
open Option.Monad_infix

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

let remote_decl_store ~(decl_hash : string) : string =
  Printf.sprintf
    "prefetch/%s/shallow_decls/%s.shallow.bin"
    (get_hh_version ())
    decl_hash
  |> Filename.concat Sys_utils.temp_dir_name

let extract_decl ~(decl_hash : string) :
    Shallow_decl_defs.shallow_class SMap.t option =
  try
    let decl_store = remote_decl_store ~decl_hash in
    if Path.file_exists (Path.make decl_store) then
      Hh_logger.log "Found remote decl store: %s" decl_store
    else
      Hh_logger.log "Cannot find remote decl store: %s" decl_store;
    let ic = Stdlib.open_in_bin decl_store in
    let contents : Decl_export.saved_shallow_decls = Marshal.from_channel ic in
    Stdlib.close_in ic;
    Some contents.Decl_export.classes
  with
  | _ -> None

let db_path_of_ctx (ctx : Provider_context.t) : Naming_sqlite.db_path option =
  ctx |> Provider_context.get_backend |> Db_path_provider.get_naming_db_path

let merge_decls
    (decls_opt : Shallow_decl_defs.shallow_class option SMap.t)
    (decls : Shallow_decl_defs.shallow_class SMap.t) :
    Shallow_decl_defs.shallow_class option SMap.t =
  decls
  |> SMap.map Option.some
  |> SMap.merge
       (fun _ v1 v2 ->
         if Option.is_some v1 then
           v1
         else
           v2)
       decls_opt

let get_old_decl ~(ctx : Provider_context.t) (name : string) :
    Shallow_decl_defs.shallow_class option SMap.t option =
  let mode = Provider_context.get_deps_mode ctx in
  Hh_logger.log "Attempt to fetch old decl for %s from remote decl store" name;
  let dep = Typing_deps.(Dep.make (hash_mode mode) (Dep.Type name)) in
  db_path_of_ctx ctx >>= fun db_path ->
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
        let path = Filename.concat Sys_utils.temp_dir_name "naming.sql" in
        let db_path = Naming_sqlite.Db_path path in
        ```
  *)
  Naming_sqlite.get_decl_hash_by_64bit_dep db_path dep >>= fun decl_hash ->
  extract_decl ~decl_hash >>| fun decls -> merge_decls SMap.empty decls
