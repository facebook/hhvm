(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Shallow_decl_defs

(** Fetches the shallow decl, optionally keeping it in cache.
When might we want to keep it in cache? e.g. if we're using lazy (shallow)
decls, then all shallow decls should be kept in cache, so that subsequent
linearizations or searches can find them, and so that even if linearizations
get evicted then the shallow cache is still available.
Why might we not want to keep it in the cache? e.g. if we're using eager (folded)
decls, then we incidentally obtain shallow decls in the process of generating
a folded decl, but the folded decl contains everything that one could ever need,
and is never evicted, and nno one will ever go back to the shallow decl again,
so keeping it around would just be a waste of memory. *)
let decl (ctx : Provider_context.t) ~(use_cache : bool) (class_ : Nast.class_) :
    shallow_class =
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory ->
    if use_cache then
      Shallow_classes_heap.class_decl_if_missing ctx class_
    else
      Shallow_classes_heap.class_naming_and_decl ctx class_
  | Provider_backend.Local_memory _ ->
    Shallow_classes_heap.class_naming_and_decl ctx class_
  | Provider_backend.Decl_service _ ->
    failwith "shallow class decl not implemented for Decl_service"

let get (ctx : Provider_context.t) (name : string) : shallow_class option =
  if
    not (TypecheckerOptions.shallow_class_decl (Provider_context.get_tcopt ctx))
  then
    failwith "shallow_class_decl not enabled"
  else
    match Provider_context.get_backend ctx with
    | Provider_backend.Shared_memory -> Shallow_classes_heap.get ctx name
    | Provider_backend.Local_memory { Provider_backend.shallow_decl_cache; _ }
      ->
      Provider_backend.Shallow_decl_cache.find_or_add
        shallow_decl_cache
        ~key:(Provider_backend.Shallow_decl_cache_entry.Shallow_class_decl name)
        ~default:(fun () ->
          let open Option.Monad_infix in
          Naming_provider.get_class_path ctx name >>= fun path ->
          Ast_provider.find_class_in_file ctx path name >>| fun class_ ->
          Shallow_classes_heap.class_naming_and_decl ctx class_)
    | Provider_backend.Decl_service { decl; _ } ->
      Decl_service_client.rpc_get_class decl name

let get_batch (ctx : Provider_context.t) (names : SSet.t) :
    shallow_class option SMap.t =
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory -> Shallow_classes_heap.get_batch names
  | Provider_backend.Local_memory _ ->
    failwith "get_batch not implemented for Local_memory"
  | Provider_backend.Decl_service _ ->
    failwith "get_batch not implemented for Decl_service"

let get_old_batch (ctx : Provider_context.t) (names : SSet.t) :
    shallow_class option SMap.t =
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory -> Shallow_classes_heap.get_old_batch names
  | Provider_backend.Local_memory _ ->
    failwith "get_old_batch not implemented for Local_memory"
  | Provider_backend.Decl_service _ ->
    failwith "get_old_batch not implemented for Decl_service"

let oldify_batch (ctx : Provider_context.t) (names : SSet.t) : unit =
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory -> Shallow_classes_heap.oldify_batch names
  | Provider_backend.Local_memory _ ->
    failwith "oldify_batch not implemented for Local_memory"
  | Provider_backend.Decl_service _ ->
    failwith "oldify_batch not implemented for Decl_service"

let remove_old_batch (ctx : Provider_context.t) (names : SSet.t) : unit =
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory ->
    Shallow_classes_heap.remove_old_batch names
  | Provider_backend.Local_memory _ ->
    failwith "remove_old_batch not implemented for Local_memory"
  | Provider_backend.Decl_service _ ->
    failwith "remove_old_batch not implemented for Decl_service"

let remove_batch (ctx : Provider_context.t) (names : SSet.t) : unit =
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory -> Shallow_classes_heap.remove_batch names
  | Provider_backend.Local_memory _ ->
    failwith "remove_batch not implemented for Local_memory"
  | Provider_backend.Decl_service _ ->
    failwith "remove_batch not implemented for Decl_service"

let local_changes_push_sharedmem_stack () : unit =
  Shallow_classes_heap.push_local_changes ()

let local_changes_pop_sharedmem_stack () : unit =
  Shallow_classes_heap.pop_local_changes ()
