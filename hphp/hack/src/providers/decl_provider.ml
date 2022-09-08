(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Class = Typing_classes_heap.Api

type fun_key = string

type type_key = string

type gconst_key = string

type module_key = string

type fun_decl = Typing_defs.fun_elt

type class_decl = Typing_classes_heap.Api.t

type typedef_decl = Typing_defs.typedef_type

type gconst_decl = Typing_defs.const_decl

type module_decl = Typing_defs.module_def_type

let ( let* ) = Caml.Option.bind

let err_not_found = Typedef_provider.err_not_found

let find_in_direct_decl_parse = Typedef_provider.find_in_direct_decl_parse

let use_direct_decl_parser ctx =
  TypecheckerOptions.use_direct_decl_parser (Provider_context.get_tcopt ctx)

(** This cache caches the result of full class computations
      (the class merged with all its inherited members.)  *)
module Cache =
  SharedMem.FreqCache
    (StringKey)
    (struct
      type t = Typing_classes_heap.class_t

      let description = "Decl_Typing_ClassType"
    end)
    (struct
      let capacity = 1000
    end)

let declare_folded_class_in_file
    (ctx : Provider_context.t) (file : Relative_path.t) (name : type_key) :
    Decl_defs.decl_class_type * Decl_store.class_members option =
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis -> failwith "invalid"
  | _ ->
    (match
       Errors.run_in_decl_mode file (fun () ->
           Decl_folded_class.class_decl_if_missing ~sh:SharedMem.Uses ctx name)
     with
    | None -> err_not_found file name
    | Some decl_and_members -> decl_and_members)

let lookup_or_populate_class_cache class_name populate =
  match Cache.get class_name with
  | Some _ as result -> result
  | None ->
    begin
      match populate class_name with
      | None -> None
      | Some v as result ->
        Cache.add class_name v;
        result
    end

let get_class
    ?(tracing_info : Decl_counters.tracing_info option)
    (ctx : Provider_context.t)
    (class_name : type_key) : class_decl option =
  Decl_counters.count_decl ?tracing_info Decl_counters.Class class_name
  @@ fun counter ->
  (* There's a confusing matrix of possibilities:
     SHALLOW - in this case, the Typing_classes_heap.class_t we get back is
       just a small shim that does memoization; further accessors on it
       like "get_method" will lazily call Linearization_provider and Shallow_classes_provider
       to get more information
     EAGER - in this case, the Typing_classes_heap.class_t we get back is
       an "folded" object which keeps an intire index of all members, although
       those members are fetched lazily via Lazy.t.

     and

     LOCAL BACKEND - the class_t is cached in the local backend.
     SHAREDMEM BACKEND - the class_t is cached in the worker-local 'Cache' heap.
       Note that in the case of eager, the class_t is really just a fairly simple
       derivation of the decl_class_type that lives in shmem.
     DECL BACKEND - the class_t is cached in the worker-local 'Cache' heap *)
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis ->
    begin
      match
        lookup_or_populate_class_cache class_name (fun class_name ->
            Decl_store.((get ()).get_class class_name)
            |> Option.map ~f:Typing_classes_heap.make_eager_class_decl)
      with
      | None -> None
      | Some v -> Some (counter, v, Some ctx)
    end
  | Provider_backend.Pessimised_shared_memory _ ->
    (* No pessimisation needs to be done here directly. All pessimisation is
     * done on the shallow classes within [Shallow_classes_provider] that the
     * [Typing_classes_heap.Api.t] returned here is constructed from
     * Crucially, we do not use the [Cache] here, which would contain
     * outdated member types once we update its members during
     * pessimisation. *)
    begin
      match
        Typing_classes_heap.get ctx class_name declare_folded_class_in_file
      with
      | None -> None
      | Some v -> Some (counter, v, Some ctx)
    end
  | Provider_backend.Shared_memory
  | Provider_backend.Decl_service _ ->
    begin
      match
        lookup_or_populate_class_cache class_name (fun class_name ->
            Typing_classes_heap.get ctx class_name declare_folded_class_in_file)
      with
      | None -> None
      | Some v -> Some (counter, v, Some ctx)
    end
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } ->
    let open Option.Monad_infix in
    Typing_classes_heap.get_class_with_cache
      ctx
      class_name
      decl_cache
      declare_folded_class_in_file
    >>| fun cls -> (counter, cls, Some ctx)
  | Provider_backend.Rust_provider_backend backend ->
    begin
      match
        lookup_or_populate_class_cache class_name (fun class_name ->
            Rust_provider_backend.Decl.get_folded_class backend class_name
            |> Option.map ~f:Typing_classes_heap.make_eager_class_decl)
      with
      | None -> None
      | Some v -> Some (counter, v, Some ctx)
    end

let declare_fun_in_file_DEPRECATED
    (ctx : Provider_context.t) (file : Relative_path.t) (name : fun_key) :
    Typing_defs.fun_elt =
  match Ast_provider.find_fun_in_file ctx file name with
  | Some f ->
    let (_name, decl) = Decl_nast.fun_naming_and_decl_DEPRECATED ctx f in
    decl
  | None -> err_not_found file name

let get_fun
    ?(tracing_info : Decl_counters.tracing_info option)
    (ctx : Provider_context.t)
    (fun_name : fun_key) : fun_decl option =
  Decl_counters.count_decl Decl_counters.Fun ?tracing_info fun_name
  @@ fun _counter ->
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis -> Decl_store.((get ()).get_fun fun_name)
  | Provider_backend.Pessimised_shared_memory info ->
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
    (match Decl_store.((get ()).get_fun fun_name) with
    | Some c -> Some c
    | None ->
      (match Naming_provider.get_fun_path ctx fun_name with
      | Some filename ->
        if use_direct_decl_parser ctx then
          find_in_direct_decl_parse
            ~cache_results:true
            ctx
            filename
            fun_name
            Shallow_decl_defs.to_fun_decl_opt
        else
          let ft =
            Errors.run_in_decl_mode filename (fun () ->
                declare_fun_in_file_DEPRECATED ctx filename fun_name)
          in
          Decl_store.((get ()).add_fun fun_name ft);
          Some ft
      | None -> None))
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } ->
    Provider_backend.Decl_cache.find_or_add
      decl_cache
      ~key:(Provider_backend.Decl_cache_entry.Fun_decl fun_name)
      ~default:(fun () ->
        match Naming_provider.get_fun_path ctx fun_name with
        | Some filename ->
          if use_direct_decl_parser ctx then
            find_in_direct_decl_parse
              ~cache_results:true
              ctx
              filename
              fun_name
              Shallow_decl_defs.to_fun_decl_opt
          else
            let ft =
              Errors.run_in_decl_mode filename (fun () ->
                  declare_fun_in_file_DEPRECATED ctx filename fun_name)
            in
            Some ft
        | None -> None)
  | Provider_backend.Decl_service { decl; _ } ->
    Decl_service_client.rpc_get_fun decl fun_name
  | Provider_backend.Rust_provider_backend backend ->
    Rust_provider_backend.Decl.get_fun backend fun_name

let get_typedef
    ?(tracing_info : Decl_counters.tracing_info option)
    (ctx : Provider_context.t)
    (typedef_name : type_key) : typedef_decl option =
  Decl_counters.count_decl Decl_counters.Typedef ?tracing_info typedef_name
  @@ fun _counter ->
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis -> Decl_store.((get ()).get_typedef typedef_name)
  | Provider_backend.Shared_memory ->
    Typedef_provider.get_typedef ctx typedef_name
  | Provider_backend.Pessimised_shared_memory info ->
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
    Provider_backend.Decl_cache.find_or_add
      decl_cache
      ~key:(Provider_backend.Decl_cache_entry.Typedef_decl typedef_name)
      ~default:(fun () ->
        match Naming_provider.get_typedef_path ctx typedef_name with
        | Some filename ->
          if use_direct_decl_parser ctx then
            find_in_direct_decl_parse
              ~cache_results:true
              ctx
              filename
              typedef_name
              Shallow_decl_defs.to_typedef_decl_opt
          else
            let tdecl =
              Errors.run_in_decl_mode filename (fun () ->
                  Typedef_provider.declare_typedef_in_file_DEPRECATED
                    ctx
                    filename
                    typedef_name)
            in
            Some tdecl
        | None -> None)
  | Provider_backend.Decl_service { decl; _ } ->
    Decl_service_client.rpc_get_typedef decl typedef_name
  | Provider_backend.Rust_provider_backend backend ->
    Rust_provider_backend.Decl.get_typedef backend typedef_name

let declare_const_in_file_DEPRECATED
    (ctx : Provider_context.t) (file : Relative_path.t) (name : gconst_key) :
    gconst_decl =
  match Ast_provider.find_gconst_in_file ctx file name with
  | Some cst ->
    let (_name, decl) = Decl_nast.const_naming_and_decl_DEPRECATED ctx cst in
    decl
  | None -> err_not_found file name

let get_gconst
    ?(tracing_info : Decl_counters.tracing_info option)
    (ctx : Provider_context.t)
    (gconst_name : gconst_key) : gconst_decl option =
  Decl_counters.count_decl Decl_counters.GConst ?tracing_info gconst_name
  @@ fun _counter ->
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
        if use_direct_decl_parser ctx then
          find_in_direct_decl_parse
            ~cache_results:true
            ctx
            filename
            gconst_name
            Shallow_decl_defs.to_const_decl_opt
        else
          let gconst =
            Errors.run_in_decl_mode filename (fun () ->
                declare_const_in_file_DEPRECATED ctx filename gconst_name)
          in
          Decl_store.((get ()).add_gconst gconst_name gconst);
          Some gconst
      | None -> None))
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } ->
    Provider_backend.Decl_cache.find_or_add
      decl_cache
      ~key:(Provider_backend.Decl_cache_entry.Gconst_decl gconst_name)
      ~default:(fun () ->
        match Naming_provider.get_const_path ctx gconst_name with
        | Some filename ->
          if use_direct_decl_parser ctx then
            find_in_direct_decl_parse
              ~cache_results:true
              ctx
              filename
              gconst_name
              Shallow_decl_defs.to_const_decl_opt
          else
            let gconst =
              Errors.run_in_decl_mode filename (fun () ->
                  declare_const_in_file_DEPRECATED ctx filename gconst_name)
            in
            Some gconst
        | None -> None)
  | Provider_backend.Decl_service { decl; _ } ->
    Decl_service_client.rpc_get_gconst decl gconst_name
  | Provider_backend.Rust_provider_backend backend ->
    Rust_provider_backend.Decl.get_gconst backend gconst_name

let prepare_for_typecheck
    (ctx : Provider_context.t) (path : Relative_path.t) (content : string) :
    unit =
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis
  | Provider_backend.Rust_provider_backend _
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Shared_memory
  | Provider_backend.Local_memory _ ->
    ()
  (* When using the decl service, before typechecking the file, populate our
     decl caches with the symbols declared within that file. If we leave this to
     the decl service, then in longer files, the decls declared later in the
     file may be evicted by the time we attempt to typecheck them, forcing the
     decl service to re-parse the file. This can lead to many re-parses in
     extreme cases. *)
  | Provider_backend.Decl_service { decl; _ } ->
    Decl_service_client.parse_and_cache_decls_in decl path content

let declare_module_in_file_DEPRECATED
    (ctx : Provider_context.t) (file : Relative_path.t) (name : module_key) :
    module_decl =
  match Ast_provider.find_module_in_file ctx file name with
  | Some md ->
    let (_name, decl) = Decl_nast.module_naming_and_decl_DEPRECATED ctx md in
    decl
  | None -> err_not_found file name

let get_module
    ?(tracing_info : Decl_counters.tracing_info option)
    (ctx : Provider_context.t)
    (module_name : module_key) : module_decl option =
  Decl_counters.count_decl Decl_counters.Module_decl ?tracing_info module_name
  @@ fun _counter ->
  let fetch_from_backing_store () =
    Naming_provider.get_module_path ctx module_name
    |> Option.bind ~f:(fun filename ->
           if use_direct_decl_parser ctx then
             find_in_direct_decl_parse
               ~cache_results:true
               ctx
               filename
               module_name
               Shallow_decl_defs.to_module_decl_opt
           else
             let module_ =
               Errors.run_in_decl_mode filename (fun () ->
                   declare_module_in_file_DEPRECATED ctx filename module_name)
             in
             Decl_store.((get ()).add_module module_name module_);
             Some module_)
  in
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis -> Decl_store.((get ()).get_module module_name)
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Shared_memory ->
    Option.first_some
      Decl_store.((get ()).get_module module_name)
      (fetch_from_backing_store ())
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } ->
    Provider_backend.Decl_cache.find_or_add
      decl_cache
      ~key:(Provider_backend.Decl_cache_entry.Module_decl module_name)
      ~default:fetch_from_backing_store
  | Provider_backend.Decl_service { decl; _ } ->
    Decl_service_client.rpc_get_module decl module_name
  | Provider_backend.Rust_provider_backend backend ->
    Rust_provider_backend.Decl.get_module backend module_name

let get_overridden_method ctx ~class_name ~method_name ~is_static :
    Typing_defs.class_elt option =
  let open Option.Monad_infix in
  get_class ctx class_name >>= fun cls ->
  Class.overridden_method cls ~method_name ~is_static ~get_class

let local_changes_push_sharedmem_stack () =
  Decl_store.((get ()).push_local_changes ())

let local_changes_pop_sharedmem_stack () =
  Decl_store.((get ()).pop_local_changes ())

let declare_folded_class_in_file_FOR_TESTS_ONLY ctx fn cid =
  fst (declare_folded_class_in_file ctx fn cid)
