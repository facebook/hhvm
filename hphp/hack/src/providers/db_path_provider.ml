(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

(** This sharedmem is used only for the Shared_memory backend *)
module Shared_db_settings =
  SharedMem.NoCache (SharedMem.ProfiledImmediate) (StringKey)
    (struct
      type t = Naming_sqlite.db_path

      let prefix = Prefix.make ()

      let description = "NamingTableDatabaseSettings"
    end)

let get_naming_db_path (ctx : Provider_context.t) : Naming_sqlite.db_path option
    =
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory -> Shared_db_settings.get "database_path"
  | Provider_backend.Local_memory { Provider_backend.naming_db_path_ref; _ } ->
    !naming_db_path_ref
  | Provider_backend.Decl_service _ ->
    failwith "decl provider doesn't expose naming db path"

let set_naming_db_path
    (ctx : Provider_context.t) (naming_db_path : Naming_sqlite.db_path option) :
    unit =
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory ->
    Shared_db_settings.remove_batch (SSet.singleton "database_path");
    Option.iter naming_db_path ~f:(Shared_db_settings.add "database_path")
  | Provider_backend.Local_memory { Provider_backend.naming_db_path_ref; _ } ->
    naming_db_path_ref := naming_db_path
  | Provider_backend.Decl_service _ ->
    failwith "decl provider doesn't expose naming db path"
