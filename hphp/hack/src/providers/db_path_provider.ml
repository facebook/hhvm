(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(** This sharedmem is used only for the Shared_memory and Analysis backends *)
module Shared_db_settings =
  SharedMem.Heap
    (SharedMem.ImmediateBackend (SharedMem.NonEvictable)) (StringKey)
    (struct
      type t = Naming_sqlite.db_path

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
  | ( ( Provider_backend.Analysis | Provider_backend.Rust_provider_backend _
      | Provider_backend.Shared_memory
      | Provider_backend.Pessimised_shared_memory _ ),
      `Shmem_cached_path path_opt ) ->
    path_opt
  | ( ( Provider_backend.Analysis | Provider_backend.Shared_memory
      | Provider_backend.Pessimised_shared_memory _ ),
      `Shmem_not_yet_cached_path ) ->
    let path_opt = Shared_db_settings.get "database_path" in
    naming_db_path_cache := `Shmem_cached_path path_opt;
    path_opt
  | (Provider_backend.Rust_provider_backend backend, `Shmem_not_yet_cached_path)
    ->
    let path_opt = Rust_provider_backend.Naming.get_db_path backend in
    naming_db_path_cache := `Shmem_cached_path path_opt;
    path_opt
  | (Provider_backend.Local_memory { Provider_backend.naming_db_path_ref; _ }, _)
    ->
    !naming_db_path_ref

let set_naming_db_path
    (backend : Provider_backend.t)
    (naming_db_path : Naming_sqlite.db_path option) : unit =
  match backend with
  | Provider_backend.Analysis
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Shared_memory ->
    Shared_db_settings.remove_batch (SSet.singleton "database_path");
    Option.iter naming_db_path ~f:(Shared_db_settings.add "database_path");
    naming_db_path_cache := `Shmem_cached_path naming_db_path
  | Provider_backend.Rust_provider_backend backend ->
    Option.iter
      naming_db_path
      ~f:(Rust_provider_backend.Naming.set_db_path backend);
    naming_db_path_cache := `Shmem_cached_path naming_db_path
  | Provider_backend.Local_memory { Provider_backend.naming_db_path_ref; _ } ->
    naming_db_path_ref := naming_db_path
