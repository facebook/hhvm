(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Reordered_argument_collections

type key = string * Decl_defs.linearization_kind

let key_to_local_key (key : key) :
    Decl_defs.linearization Provider_backend.Linearization_cache_entry.t =
  let (name, kind) = key in
  match kind with
  | Decl_defs.Member_resolution ->
    Provider_backend.Linearization_cache_entry.Member_resolution_linearization
      name
  | Decl_defs.Ancestor_types ->
    Provider_backend.Linearization_cache_entry.Ancestor_types_linearization name

module CacheKey = struct
  type t = string * Decl_defs.linearization_kind [@@deriving show, ord]

  let to_string = show
end

module CacheKeySet = Reordered_argument_set (Caml.Set.Make (CacheKey))

module Cache =
  SharedMem.WithCache (SharedMem.ProfiledImmediate) (CacheKey)
    (struct
      type t = Decl_defs.mro_element list

      let prefix = Prefix.make ()

      let description = "Decl_Linearization"
    end)
    (struct
      let capacity = 1000
    end)

let local_changes_push_sharedmem_stack () : unit =
  Cache.LocalChanges.push_stack ()

let local_changes_pop_sharedmem_stack () : unit =
  Cache.LocalChanges.pop_stack ()

let remove_batch (_ctx : Provider_context.t) (classes : SSet.t) : unit =
  let keys =
    SSet.fold classes ~init:CacheKeySet.empty ~f:(fun class_name acc ->
        let acc =
          CacheKeySet.add acc (class_name, Decl_defs.Member_resolution)
        in
        let acc = CacheKeySet.add acc (class_name, Decl_defs.Ancestor_types) in
        acc)
  in
  Cache.remove_batch keys

module LocalCache =
  SharedMem.LocalCache
    (CacheKey)
    (struct
      type t = Decl_defs.linearization

      let prefix = Prefix.make ()

      let description = "Decl_LazyLinearization"
    end)
    (struct
      let capacity = 1000
    end)

let add (ctx : Provider_context.t) (key : key) (value : Decl_defs.linearization)
    : unit =
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory -> LocalCache.add key value
  | Provider_backend.Local_memory { Provider_backend.linearization_cache; _ } ->
    let key = key_to_local_key key in
    Provider_backend.Linearization_cache.add linearization_cache ~key ~value
  | Provider_backend.Decl_service _ ->
    (* TODO: Consider storing these in the provider context or decl-service
       instead of a global cache in the hh_worker process *)
    LocalCache.add key value

let complete
    (ctx : Provider_context.t) (key : key) (value : Decl_defs.mro_element list)
    : unit =
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory ->
    Cache.add key value;
    LocalCache.remove key
  | Provider_backend.Local_memory _ -> ()
  | Provider_backend.Decl_service _ -> ()

let get (ctx : Provider_context.t) (key : key) : Decl_defs.linearization option
    =
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory ->
    begin
      match Cache.get key with
      | Some lin -> Some (Sequence.of_list lin)
      | None -> LocalCache.get key
    end
  | Provider_backend.Local_memory { Provider_backend.linearization_cache; _ } ->
    let key = key_to_local_key key in
    Provider_backend.Linearization_cache.find_or_add
      linearization_cache
      ~key
      ~default:(fun () -> None)
  | Provider_backend.Decl_service _ ->
    (* TODO: Consider storing these in the provider context or decl-service
       instead of a global cache in the hh_worker process *)
    LocalCache.get key
