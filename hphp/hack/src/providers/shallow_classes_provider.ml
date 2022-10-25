(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shallow_decl_defs

let err_not_found (file : Relative_path.t) (name : string) : 'a =
  let err_str =
    Printf.sprintf "%s not found in %s" name (Relative_path.to_absolute file)
  in
  raise (Decl_defs.Decl_not_found err_str)

let class_naming_and_decl_DEPRECATED ctx c =
  let c = Errors.ignore_ (fun () -> Naming.class_ ctx c) in
  Shallow_decl.class_DEPRECATED ctx c

let direct_decl_parse ctx filename name =
  match Direct_decl_utils.direct_decl_parse ctx filename with
  | None -> err_not_found filename name
  | Some parsed_file -> parsed_file.Direct_decl_utils.pfh_decls

let direct_decl_parse_and_cache ctx filename name =
  match Direct_decl_utils.direct_decl_parse_and_cache ctx filename with
  | None -> err_not_found filename name
  | Some parsed_file -> parsed_file.Direct_decl_utils.pfh_decls

let shallow_decl_enabled ctx =
  TypecheckerOptions.shallow_class_decl (Provider_context.get_tcopt ctx)

let force_shallow_decl_fanout_enabled (ctx : Provider_context.t) =
  TypecheckerOptions.force_shallow_decl_fanout (Provider_context.get_tcopt ctx)

let fetch_remote_old_decls (ctx : Provider_context.t) =
  TypecheckerOptions.fetch_remote_old_decls (Provider_context.get_tcopt ctx)

let use_direct_decl_parser ctx =
  TypecheckerOptions.use_direct_decl_parser (Provider_context.get_tcopt ctx)

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
let decl_DEPRECATED (ctx : Provider_context.t) (class_ : Nast.class_) :
    shallow_class =
  let (_, name) = class_.Aast.c_name in
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis -> failwith "invalid"
  | Provider_backend.Rust_provider_backend _ ->
    failwith
      "Rust_provider_backend: Shallow_classes_provider.decl_DEPRECATED not supported"
  | Provider_backend.Pessimised_shared_memory _ -> failwith "not supported"
  | Provider_backend.Shared_memory ->
    let decl = class_naming_and_decl_DEPRECATED ctx class_ in
    if shallow_decl_enabled ctx && not (Shallow_classes_heap.Classes.mem name)
    then (
      Shallow_classes_heap.Classes.add name decl;
      Shallow_classes_heap.MemberFilters.add decl
    ) else if
        force_shallow_decl_fanout_enabled ctx
        && not (Shallow_classes_heap.Classes.mem name)
      then
      Shallow_classes_heap.Classes.add name decl;
    decl
  | Provider_backend.Local_memory _ ->
    class_naming_and_decl_DEPRECATED ctx class_
  | Provider_backend.Decl_service _ ->
    failwith "shallow class decl not implemented for Decl_service"

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
    Rust_provider_backend.Decl.get_shallow_class backend name
  | Provider_backend.Pessimised_shared_memory info ->
    (match Shallow_classes_heap.Classes.get name with
    | Some _ as decl_opt -> decl_opt
    | None ->
      (match Naming_provider.get_class_path ctx name with
      | None -> None
      | Some path ->
        let ( let* ) = Caml.Option.bind in
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
      | Some path ->
        if use_direct_decl_parser ctx then
          find_in_direct_decl_parse ~fill_caches:true path
        else
          Some
            (match Ast_provider.find_class_in_file ctx path name with
            | None -> err_not_found path name
            | Some class_ ->
              let decl = class_naming_and_decl_DEPRECATED ctx class_ in
              if shallow_decl_enabled ctx then (
                Shallow_classes_heap.Classes.add name decl;
                Shallow_classes_heap.MemberFilters.add decl
              ) else if force_shallow_decl_fanout_enabled ctx then
                Shallow_classes_heap.Classes.add name decl;
              decl)))
  | Provider_backend.Local_memory { Provider_backend.shallow_decl_cache; _ } ->
    Provider_backend.Shallow_decl_cache.find_or_add
      shallow_decl_cache
      ~key:(Provider_backend.Shallow_decl_cache_entry.Shallow_class_decl name)
      ~default:(fun () ->
        match Naming_provider.get_class_path ctx name with
        | None -> None
        | Some path ->
          if use_direct_decl_parser ctx then
            find_in_direct_decl_parse ~fill_caches:true path
          else
            Some
              (match Ast_provider.find_class_in_file ctx path name with
              | None -> err_not_found path name
              | Some class_ -> class_naming_and_decl_DEPRECATED ctx class_))
  | Provider_backend.Decl_service { decl; _ } ->
    Decl_service_client.rpc_get_class decl name

let get_member_filter (ctx : Provider_context.t) (name : string) :
    BloomFilter.t option =
  match Provider_context.get_backend ctx with
  | Provider_backend.Pessimised_shared_memory _ -> failwith "invalid"
  | Provider_backend.Rust_provider_backend _
  | Provider_backend.Shared_memory ->
    Shallow_classes_heap.MemberFilters.get name
  | Provider_backend.Analysis
  | Provider_backend.Local_memory _
  | Provider_backend.Decl_service _ ->
    None

let get_batch (ctx : Provider_context.t) (names : SSet.t) :
    shallow_class option SMap.t =
  match Provider_context.get_backend ctx with
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Analysis ->
    failwith "invalid"
  | Provider_backend.Rust_provider_backend be ->
    SSet.fold
      (fun name acc ->
        SMap.add name (Rust_provider_backend.Decl.get_shallow_class be name) acc)
      names
      SMap.empty
  | Provider_backend.Shared_memory ->
    Shallow_classes_heap.Classes.get_batch names
  | Provider_backend.Local_memory _ ->
    failwith "get_batch not implemented for Local_memory"
  | Provider_backend.Decl_service _ ->
    failwith "get_batch not implemented for Decl_service"

let get_old_batch
    (ctx : Provider_context.t)
    (names : SSet.t)
    ~(fetch_old_decls :
       string list -> Shallow_decl_defs.shallow_class option SMap.t) :
    shallow_class option SMap.t =
  match Provider_context.get_backend ctx with
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Analysis ->
    failwith "invalid"
  (* TODO(sf, 2022-10-20): Reduce duplication between the following two cases. *)
  | Provider_backend.Rust_provider_backend be ->
    let old_classes =
      Rust_provider_backend.Decl.get_old_shallow_classes_batch
        be
        (SSet.elements names)
    in
    if fetch_remote_old_decls ctx then
      let missing_old_classes =
        SMap.fold
          (fun cid decl_opt missing_classes ->
            if Option.is_some decl_opt then
              missing_classes
            else
              cid :: missing_classes)
          old_classes
          []
      in
      let remote_old_classes = fetch_old_decls missing_old_classes in
      SMap.fold
        (fun a b acc ->
          if Option.is_some b || not (SMap.mem a remote_old_classes) then
            SMap.add a b acc
          else
            SMap.add a (SMap.find a remote_old_classes) acc)
        old_classes
        SMap.empty
    else
      old_classes
  | Provider_backend.Shared_memory ->
    let old_classes = Shallow_classes_heap.Classes.get_old_batch names in
    if fetch_remote_old_decls ctx then
      let missing_old_classes =
        SMap.fold
          (fun cid decl_opt missing_classes ->
            if Option.is_some decl_opt then
              missing_classes
            else
              cid :: missing_classes)
          old_classes
          []
      in
      let remote_old_classes = fetch_old_decls missing_old_classes in
      SMap.fold
        (fun a b acc ->
          if Option.is_some b || not (SMap.mem a remote_old_classes) then
            SMap.add a b acc
          else
            SMap.add a (SMap.find a remote_old_classes) acc)
        old_classes
        SMap.empty
    else
      old_classes
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
    Shallow_classes_heap.Classes.oldify_batch names;
    Shallow_classes_heap.MemberFilters.oldify_batch names
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
    Shallow_classes_heap.Classes.remove_old_batch names;
    Shallow_classes_heap.MemberFilters.remove_old_batch names
  | Provider_backend.Local_memory _ ->
    failwith "remove_old_batch not implemented for Local_memory"
  | Provider_backend.Decl_service _ ->
    failwith "remove_old_batch not implemented for Decl_service"

let remove_batch (ctx : Provider_context.t) (names : SSet.t) : unit =
  match Provider_context.get_backend ctx with
  | Provider_backend.Pessimised_shared_memory _ -> failwith "invalid"
  | Provider_backend.Analysis
  | Provider_backend.Rust_provider_backend _
  | Provider_backend.Shared_memory ->
    Shallow_classes_heap.Classes.remove_batch names;
    Shallow_classes_heap.MemberFilters.remove_batch names
  | Provider_backend.Local_memory _ ->
    failwith "remove_batch not implemented for Local_memory"
  | Provider_backend.Decl_service _ ->
    failwith "remove_batch not implemented for Decl_service"

let local_changes_push_sharedmem_stack () : unit =
  Shallow_classes_heap.Classes.LocalChanges.push_stack ();
  Shallow_classes_heap.MemberFilters.LocalChanges.push_stack ()

let local_changes_pop_sharedmem_stack () : unit =
  Shallow_classes_heap.Classes.LocalChanges.pop_stack ();
  Shallow_classes_heap.MemberFilters.LocalChanges.pop_stack ()
