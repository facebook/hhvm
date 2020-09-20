(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(** This sharedmem is used only for the Shared_memory backend *)
module Shared_db_settings =
  SharedMem.NoCache (SharedMem.ProfiledImmediate) (StringKey)
    (struct
      type t = Naming_sqlite.db_path

      let prefix = Prefix.make ()

      let description = "NamingTableDatabaseSettings"
    end)

(** SharedMem doesn't cache absences. So we write our own cache. *)
let naming_db_path_cache :
    [ `Shmem_not_yet_cached_path
    | `Shmem_cached_path of Naming_sqlite.db_path option
    ]
    ref =
  ref `Shmem_not_yet_cached_path

let get_naming_db_path (backend : Provider_backend.t) :
    Naming_sqlite.db_path option =
  match (backend, !naming_db_path_cache) with
  | (Provider_backend.Shared_memory, `Shmem_cached_path path_opt) -> path_opt
  | (Provider_backend.Shared_memory, `Shmem_not_yet_cached_path) ->
    let path_opt = Shared_db_settings.get "database_path" in
    naming_db_path_cache := `Shmem_cached_path path_opt;
    path_opt
  | (Provider_backend.Local_memory { Provider_backend.naming_db_path_ref; _ }, _)
    ->
    !naming_db_path_ref
  | (Provider_backend.Decl_service _, _) ->
    failwith "decl provider doesn't expose naming db path"

let set_naming_db_path
    (backend : Provider_backend.t)
    (naming_db_path : Naming_sqlite.db_path option) : unit =
  match backend with
  | Provider_backend.Shared_memory ->
    Shared_db_settings.remove_batch (SSet.singleton "database_path");
    Option.iter naming_db_path ~f:(Shared_db_settings.add "database_path");
    naming_db_path_cache := `Shmem_cached_path naming_db_path
  | Provider_backend.Local_memory { Provider_backend.naming_db_path_ref; _ } ->
    naming_db_path_ref := naming_db_path
  | Provider_backend.Decl_service _ ->
    failwith "decl provider doesn't expose naming db path"
