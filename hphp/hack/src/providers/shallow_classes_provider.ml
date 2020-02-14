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
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory ->
    if use_cache then
      Shallow_classes_heap.class_decl_if_missing ctx class_
    else
      Shallow_classes_heap.class_naming_and_decl ctx class_
  | Provider_backend.Local_memory _ ->
    Shallow_classes_heap.class_naming_and_decl ctx class_
  | Provider_backend.Decl_service _ ->
    failwith "shallow class decl not implemented for Decl_service"

let get_class_filename x =
  match Naming_table.Types.get_filename_and_kind x with
  | Some (fn, Naming_table.TClass) -> Some fn
  | _ -> None

let get (ctx : Provider_context.t) (name : string) : shallow_class option =
  if not (TypecheckerOptions.shallow_class_decl ctx.Provider_context.tcopt) then
    failwith "shallow_class_decl not enabled"
  else
    match ctx.Provider_context.backend with
    | Provider_backend.Shared_memory -> Shallow_classes_heap.get ctx name
    | Provider_backend.Local_memory { shallow_decl_cache; _ } ->
      Provider_backend.Shallow_decl_cache.find_or_add
        shallow_decl_cache
        ~key:(Provider_backend.Shallow_decl_cache_entry.Shallow_class_decl name)
        ~default:(fun () ->
          let open Option.Monad_infix in
          get_class_filename name >>= fun path ->
          Ast_provider.find_class_in_file ctx path name >>| fun class_ ->
          Shallow_classes_heap.class_naming_and_decl ctx class_)
    | Provider_backend.Decl_service _ ->
      failwith "shallow class cache lookup not implemented for Decl_service"

let get_batch (names : SSet.t) : shallow_class option SMap.t =
  let ctx = Provider_context.get_global_context_or_empty_FOR_MIGRATION () in
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory -> Shallow_classes_heap.get_batch names
  | Provider_backend.Local_memory _ ->
    failwith "get_batch not implemented for Local_memory"
  | Provider_backend.Decl_service _ ->
    failwith "get_batch not implemented for Decl_service"

let get_old_batch (names : SSet.t) : shallow_class option SMap.t =
  let ctx = Provider_context.get_global_context_or_empty_FOR_MIGRATION () in
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory -> Shallow_classes_heap.get_old_batch names
  | Provider_backend.Local_memory _ ->
    failwith "get_old_batch not implemented for Local_memory"
  | Provider_backend.Decl_service _ ->
    failwith "get_old_batch not implemented for Decl_service"

let oldify_batch (names : SSet.t) : unit =
  let ctx = Provider_context.get_global_context_or_empty_FOR_MIGRATION () in
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory -> Shallow_classes_heap.oldify_batch names
  | Provider_backend.Local_memory _ ->
    failwith "oldify_batch not implemented for Local_memory"
  | Provider_backend.Decl_service _ ->
    failwith "oldify_batch not implemented for Decl_service"

let remove_old_batch (names : SSet.t) : unit =
  let ctx = Provider_context.get_global_context_or_empty_FOR_MIGRATION () in
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory ->
    Shallow_classes_heap.remove_old_batch names
  | Provider_backend.Local_memory _ ->
    failwith "remove_old_batch not implemented for Local_memory"
  | Provider_backend.Decl_service _ ->
    failwith "remove_old_batch not implemented for Decl_service"

let remove_batch (names : SSet.t) : unit =
  let ctx = Provider_context.get_global_context_or_empty_FOR_MIGRATION () in
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory -> Shallow_classes_heap.remove_batch names
  | Provider_backend.Local_memory _ ->
    failwith "remove_batch not implemented for Local_memory"
  | Provider_backend.Decl_service _ ->
    failwith "remove_batch not implemented for Decl_service"

let invalidate_class (ctx : Provider_context.t) (class_name : string) : unit =
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory -> remove_batch (SSet.singleton class_name)
  | Provider_backend.Local_memory { decl_cache = _; shallow_decl_cache } ->
    Provider_backend.Shallow_decl_cache.remove
      shallow_decl_cache
      ~key:
        (Provider_backend.Shallow_decl_cache_entry.Shallow_class_decl class_name)
  | Provider_backend.Decl_service _ ->
    failwith
      "Decl_provider.invalidate_class not yet impl. for decl memory provider"

let invalidate_context_decls ~(ctx : Provider_context.t) : unit =
  Relative_path.Map.iter ctx.Provider_context.entries ~f:(fun _ entry ->
      let (_funs, classes, _record_defs, _typedefs, _gconsts) =
        Nast.get_defs entry.Provider_context.ast
      in
      List.iter classes ~f:(fun (_, class_name) ->
          invalidate_class ctx class_name))

let push_local_changes () : unit =
  let ctx = Provider_context.get_global_context_or_empty_FOR_MIGRATION () in
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory -> Shallow_classes_heap.push_local_changes ()
  | Provider_backend.Local_memory _ -> invalidate_context_decls ctx
  | Provider_backend.Decl_service _ ->
    failwith "push_local_changes not implemented for Decl_service"

let pop_local_changes () : unit =
  let ctx = Provider_context.get_global_context_or_empty_FOR_MIGRATION () in
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory -> Shallow_classes_heap.pop_local_changes ()
  | Provider_backend.Local_memory _ -> invalidate_context_decls ctx
  | Provider_backend.Decl_service _ ->
    failwith "pop_local_changes not implemented for Decl_service"
