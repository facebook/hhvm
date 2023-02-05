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

let declare_folded_class (ctx : Provider_context.t) (name : type_key) :
    Decl_defs.decl_class_type * Decl_store.class_members option =
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis -> failwith "invalid"
  | _ ->
    (match
       Errors.run_in_decl_mode (fun () ->
           Decl_folded_class.class_decl_if_missing ~sh:SharedMem.Uses ctx name)
     with
    | None -> err_not_found None name
    | Some decl_and_members -> decl_and_members)

let lookup_or_populate_class_cache class_name populate =
  match Cache.get class_name with
  | Some _ as result -> result
  | None -> begin
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
  (* There are several possibilities:
     LOCAL BACKEND - the class_t is cached in the local backend.
     SHAREDMEM BACKEND - the class_t is cached in the worker-local 'Cache' heap.
       Note that in the case of eager, the class_t is really just a fairly simple
       derivation of the decl_class_type that lives in shmem.
     DECL BACKEND - the class_t is cached in the worker-local 'Cache' heap *)
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis -> begin
    match
      lookup_or_populate_class_cache class_name (fun class_name ->
          Decl_store.((get ()).get_class class_name)
          |> Option.map ~f:Typing_classes_heap.make_eager_class_decl)
    with
    | None -> None
    | Some v -> Some (counter, v, Some ctx)
  end
  | Provider_backend.Pessimised_shared_memory _ -> begin
    (* No pessimisation needs to be done here directly. All pessimisation is
     * done on the shallow classes within [Shallow_classes_provider] that the
     * [Typing_classes_heap.Api.t] returned here is constructed from
     * Crucially, we do not use the [Cache] here, which would contain
     * outdated member types once we update its members during
     * pessimisation. *)
    match Typing_classes_heap.get ctx class_name declare_folded_class with
    | None -> None
    | Some v -> Some (counter, v, Some ctx)
  end
  | Provider_backend.Shared_memory
  | Provider_backend.Decl_service _ -> begin
    match
      lookup_or_populate_class_cache class_name (fun class_name ->
          Typing_classes_heap.get ctx class_name declare_folded_class)
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
      declare_folded_class
    >>| fun cls -> (counter, cls, Some ctx)
  | Provider_backend.Rust_provider_backend backend -> begin
    match
      lookup_or_populate_class_cache class_name (fun class_name ->
          Rust_provider_backend.Decl.get_folded_class
            backend
            (Naming_provider.rust_backend_ctx_proxy ctx)
            class_name
          |> Option.map ~f:Typing_classes_heap.make_eager_class_decl)
    with
    | None -> None
    | Some v -> Some (counter, v, Some ctx)
  end

let maybe_pessimise_fun_decl ctx fun_decl =
  if Provider_context.implicit_sdt_for_fun ctx fun_decl then
    Typing_defs.
      {
        fun_decl with
        fe_type =
          Decl_enforceability.(
            pessimise_fun_type
              ~fun_kind:Function
              ~this_class:None
              ctx
              fun_decl.fe_pos
              fun_decl.fe_type);
      }
  else
    fun_decl

let get_fun
    ?(tracing_info : Decl_counters.tracing_info option)
    (ctx : Provider_context.t)
    (fun_name : fun_key) : fun_decl option =
  Option.map ~f:(maybe_pessimise_fun_decl ctx)
  @@ Decl_counters.count_decl Decl_counters.Fun ?tracing_info fun_name
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
        find_in_direct_decl_parse
          ~cache_results:true
          ctx
          filename
          fun_name
          Shallow_decl_defs.to_fun_decl_opt
      | None -> None))
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } ->
    Provider_backend.Decl_cache.find_or_add
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
  | Provider_backend.Decl_service { decl; _ } ->
    Decl_service_client.rpc_get_fun decl fun_name
  | Provider_backend.Rust_provider_backend backend ->
    Rust_provider_backend.Decl.get_fun
      backend
      (Naming_provider.rust_backend_ctx_proxy ctx)
      fun_name

let maybe_pessimise_typedef_decl ctx typedef_decl =
  if
    TypecheckerOptions.everything_sdt (Provider_context.get_tcopt ctx)
    && not
         (Typing_defs.Attributes.mem
            Naming_special_names.UserAttributes.uaNoAutoDynamic
            typedef_decl.Typing_defs.td_attributes)
  then
    (* TODO: deal with super constraint *)
    match typedef_decl.Typing_defs.td_as_constraint with
    | Some _ -> typedef_decl
    | None ->
      let open Typing_defs in
      let pos = typedef_decl.td_pos in
      {
        typedef_decl with
        td_as_constraint =
          Some
            (Decl_enforceability.supportdyn_mixed
               pos
               (Reason.Rwitness_from_decl pos));
      }
  else
    typedef_decl

let get_typedef
    ?(tracing_info : Decl_counters.tracing_info option)
    (ctx : Provider_context.t)
    (typedef_name : type_key) : typedef_decl option =
  Option.map ~f:(maybe_pessimise_typedef_decl ctx)
  @@ Decl_counters.count_decl Decl_counters.Typedef ?tracing_info typedef_name
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
          find_in_direct_decl_parse
            ~cache_results:true
            ctx
            filename
            typedef_name
            Shallow_decl_defs.to_typedef_decl_opt
        | None -> None)
  | Provider_backend.Decl_service { decl; _ } ->
    Decl_service_client.rpc_get_typedef decl typedef_name
  | Provider_backend.Rust_provider_backend backend ->
    Rust_provider_backend.Decl.get_typedef
      backend
      (Naming_provider.rust_backend_ctx_proxy ctx)
      typedef_name

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
  | Provider_backend.Decl_service { decl; _ } ->
    Decl_service_client.rpc_get_gconst decl gconst_name
  | Provider_backend.Rust_provider_backend backend ->
    Rust_provider_backend.Decl.get_gconst
      backend
      (Naming_provider.rust_backend_ctx_proxy ctx)
      gconst_name

let get_module
    ?(tracing_info : Decl_counters.tracing_info option)
    (ctx : Provider_context.t)
    (module_name : module_key) : module_decl option =
  Decl_counters.count_decl Decl_counters.Module_decl ?tracing_info module_name
  @@ fun _counter ->
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
  | Provider_backend.Decl_service { decl; _ } ->
    Decl_service_client.rpc_get_module decl module_name
  | Provider_backend.Rust_provider_backend backend ->
    Rust_provider_backend.Decl.get_module
      backend
      (Naming_provider.rust_backend_ctx_proxy ctx)
      module_name

let get_overridden_method ctx ~class_name ~method_name ~is_static :
    Typing_defs.class_elt option =
  let open Option.Monad_infix in
  get_class ctx class_name >>= fun cls ->
  Class.overridden_method cls ~method_name ~is_static ~get_class

let local_changes_push_sharedmem_stack () =
  Decl_store.((get ()).push_local_changes ())

let local_changes_pop_sharedmem_stack () =
  Decl_store.((get ()).pop_local_changes ())

let declare_folded_class_in_file_FOR_TESTS_ONLY ctx cid =
  fst (declare_folded_class ctx cid)
