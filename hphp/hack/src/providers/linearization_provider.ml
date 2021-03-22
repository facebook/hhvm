(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Reordered_argument_collections

let key_to_local_key (key : string) :
    Decl_defs.lin Provider_backend.Linearization_cache_entry.t =
  Provider_backend.Linearization_cache_entry.Linearization key

module Cache =
  SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey)
    (struct
      type t = Decl_defs.lin

      let prefix = Prefix.make ()

      let description = "Decl_Linearization"
    end)
    (struct
      let capacity = 500
    end)

module DeclServiceLocalCache =
  SharedMem.LocalCache
    (StringKey)
    (struct
      type t = Decl_defs.lin

      let prefix = Prefix.make ()

      let description = "Decl_LocalLinearization"
    end)
    (struct
      let capacity = 1000
    end)

let local_changes_push_sharedmem_stack () : unit =
  Cache.LocalChanges.push_stack ()

let local_changes_pop_sharedmem_stack () : unit =
  Cache.LocalChanges.pop_stack ()

let remove_batch (_ctx : Provider_context.t) (classes : SSet.t) : unit =
  Cache.remove_batch classes

let add (ctx : Provider_context.t) (key : string) (value : Decl_defs.lin) : unit
    =
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis
  | Provider_backend.Shared_memory ->
    Cache.add key value
  | Provider_backend.Local_memory { Provider_backend.linearization_cache; _ } ->
    let key = key_to_local_key key in
    Provider_backend.Linearization_cache.add linearization_cache ~key ~value
  | Provider_backend.Decl_service _ ->
    (* TODO: Consider storing these in the provider context or decl-service
       instead of a global cache in the hh_worker process *)
    DeclServiceLocalCache.add key value

let get (ctx : Provider_context.t) (key : string) : Decl_defs.lin option =
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis
  | Provider_backend.Shared_memory ->
    Cache.get key
  | Provider_backend.Local_memory { Provider_backend.linearization_cache; _ } ->
    let key = key_to_local_key key in
    Provider_backend.Linearization_cache.find_or_add
      linearization_cache
      ~key
      ~default:(fun () -> None)
  | Provider_backend.Decl_service _ ->
    (* TODO: Consider storing these in the provider context or decl-service
       instead of a global cache in the hh_worker process *)
    DeclServiceLocalCache.get key
