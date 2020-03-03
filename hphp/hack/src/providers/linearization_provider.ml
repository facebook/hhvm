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

      let description = "Linearization"
    end)

let push_local_changes (_ctx : Provider_context.t) : unit =
  Cache.LocalChanges.push_stack ()

let pop_local_changes (_ctx : Provider_context.t) : unit =
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

      let description = "LazyLinearization"
    end)

let add
    (_ctx : Provider_context.t) (key : key) (value : Decl_defs.linearization) :
    unit =
  LocalCache.add key value

let complete
    (_ctx : Provider_context.t) (key : key) (value : Decl_defs.mro_element list)
    : unit =
  Cache.add key value;
  LocalCache.remove key;
  ()

let get (_ctx : Provider_context.t) (key : key) : Decl_defs.linearization option
    =
  match Cache.get key with
  | Some lin -> Some (Sequence.of_list lin)
  | None -> LocalCache.get key
