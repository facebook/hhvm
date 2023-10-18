(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shallow_decl_defs

let fetch_remote_old_decl_flag (ctx : Provider_context.t) =
  TypecheckerOptions.fetch_remote_old_decls (Provider_context.get_tcopt ctx)

let fetch_remote_old_decls ctx ~during_init =
  fetch_remote_old_decl_flag ctx && during_init

let fetch_missing_old_classes_remotely ctx ~during_init old_classes =
  if fetch_remote_old_decls ctx ~during_init then
    let missing_old_classes =
      SMap.filter (fun _key -> Option.is_none) old_classes |> SMap.keys
    in
    let remote_old_classes =
      Remote_old_decl_client.fetch_old_decls ~ctx missing_old_classes
    in
    SMap.union old_classes remote_old_classes ~combine:(fun _key decl1 decl2 ->
        Some (Option.first_some decl1 decl2))
  else
    old_classes

let get_old_batch (ctx : Provider_context.t) ~during_init (names : SSet.t) :
    shallow_class option SMap.t =
  match Provider_context.get_backend ctx with
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Analysis ->
    failwith "invalid"
  (* TODO(sf, 2022-10-20): Reduce duplication between the following two cases. *)
  | Provider_backend.Rust_provider_backend be ->
    let (old_classes, _funs, _typedefs, _consts, _modules) =
      Rust_provider_backend.Decl.get_old_defs
        be
        FileInfo.{ empty_names with n_classes = names }
    in
    fetch_missing_old_classes_remotely ctx ~during_init old_classes
  | Provider_backend.Shared_memory ->
    let old_classes = Shallow_classes_heap.Classes.get_old_batch names in
    fetch_missing_old_classes_remotely ctx ~during_init old_classes
  | Provider_backend.Local_memory _ ->
    failwith "get_old_batch not implemented for Local_memory"
  | Provider_backend.Decl_service _ ->
    failwith "get_old_batch not implemented for Decl_service"

let oldify_batch (ctx : Provider_context.t) (names : SSet.t) : unit =
  match Provider_context.get_backend ctx with
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Analysis ->
    failwith "invalid"
  | Provider_backend.Rust_provider_backend _
  | Provider_backend.Shared_memory ->
    Shallow_classes_heap.Classes.oldify_batch names
  | Provider_backend.Local_memory _ ->
    failwith "oldify_batch not implemented for Local_memory"
  | Provider_backend.Decl_service _ ->
    failwith "oldify_batch not implemented for Decl_service"

let remove_old_batch (ctx : Provider_context.t) (names : SSet.t) : unit =
  match Provider_context.get_backend ctx with
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Analysis ->
    failwith "invalid"
  | Provider_backend.Rust_provider_backend _
  | Provider_backend.Shared_memory ->
    Shallow_classes_heap.Classes.remove_old_batch names
  | Provider_backend.Local_memory _ ->
    failwith "remove_old_batch not implemented for Local_memory"
  | Provider_backend.Decl_service _ ->
    failwith "remove_old_batch not implemented for Decl_service"
