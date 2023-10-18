(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shallow_decl_defs

let direct_decl_parse ctx filename name =
  match Direct_decl_utils.direct_decl_parse ctx filename with
  | None -> Decl_defs.raise_decl_not_found (Some filename) name
  | Some parsed_file -> parsed_file.Direct_decl_utils.pfh_decls

let direct_decl_parse_and_cache ctx filename name =
  match Direct_decl_utils.direct_decl_parse_and_cache ctx filename with
  | None -> Decl_defs.raise_decl_not_found (Some filename) name
  | Some parsed_file -> parsed_file.Direct_decl_utils.pfh_decls

let get (ctx : Provider_context.t) (name : string) : shallow_class option =
  let find_in_direct_decl_parse ~fill_caches path =
    let f =
      if fill_caches then
        direct_decl_parse_and_cache
      else
        direct_decl_parse
    in
    f ctx path name
    |> List.find_map ~f:(function
           | (n, Shallow_decl_defs.Class decl, _) when String.equal name n ->
             Some decl
           | _ -> None)
  in

  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis ->
    (match Shallow_classes_heap.Classes.get name with
    | Some _ as decl_opt -> decl_opt
    | None -> failwith (Printf.sprintf "failed to get shallow class %S" name))
  | Provider_backend.Rust_provider_backend backend ->
    Rust_provider_backend.Decl.get_shallow_class
      backend
      (Naming_provider.rust_backend_ctx_proxy ctx)
      name
  | Provider_backend.Pessimised_shared_memory info ->
    (match Shallow_classes_heap.Classes.get name with
    | Some _ as decl_opt -> decl_opt
    | None ->
      (match Naming_provider.get_class_path ctx name with
      | None -> None
      | Some path ->
        let open Option.Let_syntax in
        let* original_sc = find_in_direct_decl_parse ~fill_caches:false path in
        let sc =
          info.Provider_backend.pessimise_shallow_class path ~name original_sc
        in
        if info.Provider_backend.store_pessimised_result then
          Shallow_classes_heap.Classes.add name sc;
        Some sc))
  | Provider_backend.Shared_memory ->
    (match Shallow_classes_heap.Classes.get name with
    | Some _ as decl_opt -> decl_opt
    | None ->
      (match Naming_provider.get_class_path ctx name with
      | None -> None
      | Some path -> find_in_direct_decl_parse ~fill_caches:true path))
  | Provider_backend.Local_memory { Provider_backend.shallow_decl_cache; _ } ->
    Provider_backend.Shallow_decl_cache.find_or_add
      shallow_decl_cache
      ~key:(Provider_backend.Shallow_decl_cache_entry.Shallow_class_decl name)
      ~default:(fun () ->
        match Naming_provider.get_class_path ctx name with
        | None -> None
        | Some path -> find_in_direct_decl_parse ~fill_caches:true path)
  | Provider_backend.Decl_service { decl; _ } ->
    Decl_service_client.rpc_get_class decl name

let get_batch (ctx : Provider_context.t) (names : SSet.t) :
    shallow_class option SMap.t =
  match Provider_context.get_backend ctx with
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Analysis ->
    failwith "invalid"
  | Provider_backend.Rust_provider_backend be ->
    SSet.fold
      (fun name acc ->
        SMap.add
          name
          (Rust_provider_backend.Decl.get_shallow_class
             be
             (Naming_provider.rust_backend_ctx_proxy ctx)
             name)
          acc)
      names
      SMap.empty
  | Provider_backend.Shared_memory ->
    Shallow_classes_heap.Classes.get_batch names
  | Provider_backend.Local_memory _ ->
    failwith "get_batch not implemented for Local_memory"
  | Provider_backend.Decl_service _ ->
    failwith "get_batch not implemented for Decl_service"

let remove_batch (ctx : Provider_context.t) (names : SSet.t) : unit =
  match Provider_context.get_backend ctx with
  | Provider_backend.Pessimised_shared_memory _ -> failwith "invalid"
  | Provider_backend.Analysis
  | Provider_backend.Rust_provider_backend _
  | Provider_backend.Shared_memory ->
    Shallow_classes_heap.Classes.remove_batch names
  | Provider_backend.Local_memory _ ->
    failwith "remove_batch not implemented for Local_memory"
  | Provider_backend.Decl_service _ ->
    failwith "remove_batch not implemented for Decl_service"

let local_changes_push_sharedmem_stack () : unit =
  Shallow_classes_heap.Classes.LocalChanges.push_stack ()

let local_changes_pop_sharedmem_stack () : unit =
  Shallow_classes_heap.Classes.LocalChanges.pop_stack ()
