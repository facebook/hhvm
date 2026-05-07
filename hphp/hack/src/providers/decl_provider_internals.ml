(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let find_in_direct_decl_parse ~cache_results ctx filename name extract_decl_opt
    =
  let parse_result =
    if cache_results then
      Direct_decl_utils.direct_decl_parse_and_cache ctx filename
    else
      Direct_decl_utils.direct_decl_parse ctx filename
  in
  match parse_result with
  | None -> Decl_defs.raise_decl_not_found (Some filename) name
  | Some parsed_file ->
    let decls = parsed_file.Direct_decl_utils.pfh_decls in
    List.find_map decls ~f:(function
        | (decl_name, decl, _) when String.equal decl_name name ->
          extract_decl_opt decl
        | _ -> None)

let get_fun_without_pessimise (ctx : Provider_context.t) (fun_name : string) :
    Typing_defs.fun_elt Decl_entry.t =
  let open Option.Let_syntax in
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis ->
    Decl_entry.of_option_or_does_not_exist
    @@ Decl_store.((get ()).get_fun fun_name)
  | Provider_backend.Pessimised_shared_memory info ->
    Decl_entry.of_option_or_does_not_exist
    @@
    (match Decl_store.((get ()).get_fun fun_name) with
    | Some c -> Some c
    | None ->
      (match Naming_provider.get_fun_path ctx fun_name with
      | Some filename ->
        let* original_ft =
          find_in_direct_decl_parse
            ~cache_results:false
            ctx
            filename
            fun_name
            Shallow_decl_defs.to_fun_decl_opt
        in
        let ft =
          info.Provider_backend.pessimise_fun
            filename
            ~name:fun_name
            original_ft
        in
        if info.Provider_backend.store_pessimised_result then
          Decl_store.((get ()).add_fun) fun_name ft;
        Some ft
      | None -> None))
  | Provider_backend.Shared_memory ->
    Decl_entry.of_option_or_does_not_exist
    @@
    (match Decl_store.((get ()).get_fun fun_name) with
    | Some c -> Some c
    | None ->
      (match Naming_provider.get_fun_path ctx fun_name with
      | Some filename ->
        find_in_direct_decl_parse
          ~cache_results:true
          ctx
          filename
          fun_name
          Shallow_decl_defs.to_fun_decl_opt
      | None -> None))
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } ->
    Decl_entry.of_option_or_does_not_exist
    @@ Provider_backend.Decl_cache.find_or_add
         decl_cache
         ~key:(Provider_backend.Decl_cache_entry.Fun_decl fun_name)
         ~default:(fun () ->
           match Naming_provider.get_fun_path ctx fun_name with
           | Some filename ->
             find_in_direct_decl_parse
               ~cache_results:true
               ctx
               filename
               fun_name
               Shallow_decl_defs.to_fun_decl_opt
           | None -> None)
  | Provider_backend.Rust_provider_backend backend ->
    Decl_entry.of_option_or_does_not_exist
    @@ Rust_provider_backend.Decl.get_fun
         backend
         (Naming_provider.rust_backend_ctx_proxy ctx)
         fun_name

let get_typedef_without_pessimise
    (ctx : Provider_context.t) (typedef_name : string) :
    Typing_defs.typedef_type Decl_entry.t =
  let open Option.Let_syntax in
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis ->
    Decl_store.((get ()).get_typedef typedef_name)
    |> Decl_entry.of_option_or_does_not_exist
  | Provider_backend.Shared_memory ->
    Decl_entry.of_option_or_does_not_exist
    @@
    (match Decl_store.((get ()).get_typedef typedef_name) with
    | Some c -> Some c
    | None ->
      (match Naming_provider.get_typedef_path ctx typedef_name with
      | Some filename ->
        find_in_direct_decl_parse
          ~cache_results:true
          ctx
          filename
          typedef_name
          Shallow_decl_defs.to_typedef_decl_opt
      | None -> None))
  | Provider_backend.Pessimised_shared_memory info ->
    Decl_entry.of_option_or_does_not_exist
    @@
    (match Decl_store.((get ()).get_typedef typedef_name) with
    | Some c -> Some c
    | None ->
      (match Naming_provider.get_typedef_path ctx typedef_name with
      | Some filename ->
        let* original_typedef =
          find_in_direct_decl_parse
            ~cache_results:false
            ctx
            filename
            typedef_name
            Shallow_decl_defs.to_typedef_decl_opt
        in
        let typedef =
          info.Provider_backend.pessimise_typedef
            filename
            ~name:typedef_name
            original_typedef
        in
        if info.Provider_backend.store_pessimised_result then
          Decl_store.((get ()).add_typedef) typedef_name typedef;
        Some typedef
      | None -> None))
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } ->
    Decl_entry.of_option_or_does_not_exist
    @@ Provider_backend.Decl_cache.find_or_add
         decl_cache
         ~key:(Provider_backend.Decl_cache_entry.Typedef_decl typedef_name)
         ~default:(fun () ->
           match Naming_provider.get_typedef_path ctx typedef_name with
           | Some filename ->
             find_in_direct_decl_parse
               ~cache_results:true
               ctx
               filename
               typedef_name
               Shallow_decl_defs.to_typedef_decl_opt
           | None -> None)
  | Provider_backend.Rust_provider_backend backend ->
    Decl_entry.of_option_or_does_not_exist
    @@ Rust_provider_backend.Decl.get_typedef
         backend
         (Naming_provider.rust_backend_ctx_proxy ctx)
         typedef_name

let get_gconst (ctx : Provider_context.t) (gconst_name : string) :
    Typing_defs.const_decl option =
  let open Option.Let_syntax in
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis -> Decl_store.((get ()).get_gconst gconst_name)
  | Provider_backend.Pessimised_shared_memory info ->
    (match Decl_store.((get ()).get_gconst gconst_name) with
    | Some c -> Some c
    | None ->
      (match Naming_provider.get_const_path ctx gconst_name with
      | Some filename ->
        let* original_gconst =
          find_in_direct_decl_parse
            ~cache_results:false
            ctx
            filename
            gconst_name
            Shallow_decl_defs.to_const_decl_opt
        in
        let gconst =
          info.Provider_backend.pessimise_gconst
            filename
            ~name:gconst_name
            original_gconst
        in
        (if info.Provider_backend.store_pessimised_result then
          Decl_store.((get ()).add_gconst gconst_name gconst));
        Some gconst
      | None -> None))
  | Provider_backend.Shared_memory ->
    (match Decl_store.((get ()).get_gconst gconst_name) with
    | Some c -> Some c
    | None ->
      (match Naming_provider.get_const_path ctx gconst_name with
      | Some filename ->
        find_in_direct_decl_parse
          ~cache_results:true
          ctx
          filename
          gconst_name
          Shallow_decl_defs.to_const_decl_opt
      | None -> None))
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } ->
    Provider_backend.Decl_cache.find_or_add
      decl_cache
      ~key:(Provider_backend.Decl_cache_entry.Gconst_decl gconst_name)
      ~default:(fun () ->
        match Naming_provider.get_const_path ctx gconst_name with
        | Some filename ->
          find_in_direct_decl_parse
            ~cache_results:true
            ctx
            filename
            gconst_name
            Shallow_decl_defs.to_const_decl_opt
        | None -> None)
  | Provider_backend.Rust_provider_backend backend ->
    Rust_provider_backend.Decl.get_gconst
      backend
      (Naming_provider.rust_backend_ctx_proxy ctx)
      gconst_name

let get_module (ctx : Provider_context.t) (module_name : string) :
    Typing_defs.module_def_type option =
  let fetch_from_backing_store () =
    Naming_provider.get_module_path ctx module_name
    |> Option.bind ~f:(fun filename ->
           find_in_direct_decl_parse
             ~cache_results:true
             ctx
             filename
             module_name
             Shallow_decl_defs.to_module_decl_opt)
  in
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis -> Decl_store.((get ()).get_module module_name)
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Shared_memory ->
    (match Decl_store.((get ()).get_module module_name) with
    | Some m -> Some m
    | None -> fetch_from_backing_store ())
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } ->
    Provider_backend.Decl_cache.find_or_add
      decl_cache
      ~key:(Provider_backend.Decl_cache_entry.Module_decl module_name)
      ~default:fetch_from_backing_store
  | Provider_backend.Rust_provider_backend backend ->
    Rust_provider_backend.Decl.get_module
      backend
      (Naming_provider.rust_backend_ctx_proxy ctx)
      module_name

let get_shallow_class (ctx : Provider_context.t) (name : string) :
    Shallow_decl_defs.shallow_class option =
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
        let* original_sc =
          find_in_direct_decl_parse
            ~cache_results:false
            ctx
            path
            name
            Shallow_decl_defs.to_class_decl_opt
        in
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
        find_in_direct_decl_parse
          ~cache_results:true
          ctx
          path
          name
          Shallow_decl_defs.to_class_decl_opt))
  | Provider_backend.Local_memory { Provider_backend.shallow_decl_cache; _ } ->
    Provider_backend.Shallow_decl_cache.find_or_add
      shallow_decl_cache
      ~key:(Provider_backend.Shallow_decl_cache_entry.Shallow_class_decl name)
      ~default:(fun () ->
        match Naming_provider.get_class_path ctx name with
        | None -> None
        | Some path ->
          find_in_direct_decl_parse
            ~cache_results:true
            ctx
            path
            name
            Shallow_decl_defs.to_class_decl_opt)
