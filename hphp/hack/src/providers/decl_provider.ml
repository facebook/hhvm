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

type class_key = string

type record_def_key = string

type typedef_key = string

type gconst_key = string

type fun_decl = Typing_defs.fun_elt

type class_decl = Typing_classes_heap.Api.t

type record_def_decl = Typing_defs.record_def_type

type typedef_decl = Typing_defs.typedef_type

type gconst_decl = Typing_defs.decl_ty

(** This cache caches the result of full class computations
      (the class merged with all its inherited members.)  *)
module Cache =
  SharedMem.LocalCache
    (StringKey)
    (struct
      type t = Typing_classes_heap.class_t

      let prefix = Prefix.make ()

      let description = "Decl_Typing_ClassType"
    end)
    (struct
      let capacity = 1000
    end)

let get_class
    ?(tracing_info : Decl_counters.tracing_info option)
    (ctx : Provider_context.t)
    (class_name : class_key) : class_decl option =
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
  | Provider_backend.Shared_memory
  | Provider_backend.Decl_service _ ->
    begin
      match Cache.get class_name with
      | Some t -> Some (counter, t)
      | None ->
        begin
          match Typing_classes_heap.get ctx class_name with
          | None -> None
          | Some v ->
            Cache.add class_name v;
            Some (counter, v)
        end
    end
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } ->
    let result : Obj.t option =
      Provider_backend.Decl_cache.find_or_add
        decl_cache
        ~key:(Provider_backend.Decl_cache_entry.Class_decl class_name)
        ~default:(fun () ->
          let v : Typing_classes_heap.class_t option =
            Typing_classes_heap.get ctx class_name
          in
          Option.map v ~f:Obj.repr)
    in
    (match result with
    | None -> None
    | Some obj ->
      let v : Typing_classes_heap.class_t = Obj.obj obj in
      Some (counter, v))

let get_fun
    ?(tracing_info : Decl_counters.tracing_info option)
    (ctx : Provider_context.t)
    (fun_name : fun_key) : fun_decl option =
  Decl_counters.count_decl Decl_counters.Fun ?tracing_info fun_name
  @@ fun _counter ->
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory ->
    (match Decl_heap.Funs.get fun_name with
    | Some c -> Some c
    | None ->
      (match Naming_provider.get_fun_path ctx fun_name with
      | Some filename ->
        let ft =
          Errors.run_in_decl_mode filename (fun () ->
              Decl.declare_fun_in_file ~write_shmem:true ctx filename fun_name)
        in
        Some ft
      | None -> None))
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } ->
    Provider_backend.Decl_cache.find_or_add
      decl_cache
      ~key:(Provider_backend.Decl_cache_entry.Fun_decl fun_name)
      ~default:(fun () ->
        match Naming_provider.get_fun_path ctx fun_name with
        | Some filename ->
          let ft =
            Errors.run_in_decl_mode filename (fun () ->
                Decl.declare_fun_in_file
                  ~write_shmem:false
                  ctx
                  filename
                  fun_name)
          in
          Some ft
        | None -> None)
  | Provider_backend.Decl_service { decl; _ } ->
    Decl_service_client.rpc_get_fun decl fun_name

let get_typedef
    ?(tracing_info : Decl_counters.tracing_info option)
    (ctx : Provider_context.t)
    (typedef_name : string) : typedef_decl option =
  Decl_counters.count_decl Decl_counters.Typedef ?tracing_info typedef_name
  @@ fun _counter ->
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory ->
    (match Decl_heap.Typedefs.get typedef_name with
    | Some c -> Some c
    | None ->
      (match Naming_provider.get_typedef_path ctx typedef_name with
      | Some filename ->
        let tdecl =
          Errors.run_in_decl_mode filename (fun () ->
              Decl.declare_typedef_in_file
                ~write_shmem:true
                ctx
                filename
                typedef_name)
        in
        Some tdecl
      | None -> None))
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } ->
    Provider_backend.Decl_cache.find_or_add
      decl_cache
      ~key:(Provider_backend.Decl_cache_entry.Typedef_decl typedef_name)
      ~default:(fun () ->
        match Naming_provider.get_typedef_path ctx typedef_name with
        | Some filename ->
          let tdecl =
            Errors.run_in_decl_mode filename (fun () ->
                Decl.declare_typedef_in_file
                  ~write_shmem:false
                  ctx
                  filename
                  typedef_name)
          in
          Some tdecl
        | None -> None)
  | Provider_backend.Decl_service { decl; _ } ->
    Decl_service_client.rpc_get_typedef decl typedef_name

let get_record_def
    ?(tracing_info : Decl_counters.tracing_info option)
    (ctx : Provider_context.t)
    (record_name : string) : record_def_decl option =
  Decl_counters.count_decl Decl_counters.Record_def ?tracing_info record_name
  @@ fun _counter ->
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory ->
    (match Decl_heap.RecordDefs.get record_name with
    | Some c -> Some c
    | None ->
      (match Naming_provider.get_record_def_path ctx record_name with
      | Some filename ->
        let tdecl =
          Errors.run_in_decl_mode filename (fun () ->
              Decl.declare_record_def_in_file
                ~write_shmem:true
                ctx
                filename
                record_name)
        in
        Some tdecl
      | None -> None))
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } ->
    Provider_backend.Decl_cache.find_or_add
      decl_cache
      ~key:(Provider_backend.Decl_cache_entry.Record_decl record_name)
      ~default:(fun () ->
        match Naming_provider.get_record_def_path ctx record_name with
        | Some filename ->
          let rdecl =
            Errors.run_in_decl_mode filename (fun () ->
                Decl.declare_record_def_in_file
                  ~write_shmem:false
                  ctx
                  filename
                  record_name)
          in
          Some rdecl
        | None -> None)
  | Provider_backend.Decl_service { decl; _ } ->
    Decl_service_client.rpc_get_record_def decl record_name

let get_gconst
    ?(tracing_info : Decl_counters.tracing_info option)
    (ctx : Provider_context.t)
    (gconst_name : string) : gconst_decl option =
  Decl_counters.count_decl Decl_counters.GConst ?tracing_info gconst_name
  @@ fun _counter ->
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory ->
    (match Decl_heap.GConsts.get gconst_name with
    | Some c -> Some c
    | None ->
      (match Naming_provider.get_const_path ctx gconst_name with
      | Some filename ->
        let gconst =
          Errors.run_in_decl_mode filename (fun () ->
              Decl.declare_const_in_file
                ~write_shmem:true
                ctx
                filename
                gconst_name)
        in
        Some gconst
      | None -> None))
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } ->
    Provider_backend.Decl_cache.find_or_add
      decl_cache
      ~key:(Provider_backend.Decl_cache_entry.Gconst_decl gconst_name)
      ~default:(fun () ->
        match Naming_provider.get_const_path ctx gconst_name with
        | Some filename ->
          let gconst =
            Errors.run_in_decl_mode filename (fun () ->
                Decl.declare_const_in_file
                  ~write_shmem:false
                  ctx
                  filename
                  gconst_name)
          in
          Some gconst
        | None -> None)
  | Provider_backend.Decl_service { decl; _ } ->
    Decl_service_client.rpc_get_gconst decl gconst_name

let prepare_for_typecheck
    (ctx : Provider_context.t) (path : Relative_path.t) (content : string) :
    unit =
  match Provider_context.get_backend ctx with
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

let local_changes_push_sharedmem_stack () : unit =
  Decl_heap.Funs.LocalChanges.push_stack ();
  Decl_heap.RecordDefs.LocalChanges.push_stack ();
  Decl_heap.Constructors.LocalChanges.push_stack ();
  Decl_heap.Props.LocalChanges.push_stack ();
  Decl_heap.StaticProps.LocalChanges.push_stack ();
  Decl_heap.Methods.LocalChanges.push_stack ();
  Decl_heap.StaticMethods.LocalChanges.push_stack ();
  Decl_heap.Classes.LocalChanges.push_stack ();
  Decl_heap.Typedefs.LocalChanges.push_stack ();
  Decl_heap.GConsts.LocalChanges.push_stack ();
  ()

let local_changes_pop_sharedmem_stack () : unit =
  Decl_heap.Funs.LocalChanges.pop_stack ();
  Decl_heap.RecordDefs.LocalChanges.pop_stack ();
  Decl_heap.Constructors.LocalChanges.pop_stack ();
  Decl_heap.Props.LocalChanges.pop_stack ();
  Decl_heap.StaticProps.LocalChanges.pop_stack ();
  Decl_heap.Methods.LocalChanges.pop_stack ();
  Decl_heap.StaticMethods.LocalChanges.pop_stack ();
  Decl_heap.Classes.LocalChanges.pop_stack ();
  Decl_heap.Typedefs.LocalChanges.pop_stack ();
  Decl_heap.GConsts.LocalChanges.pop_stack ();
  ()
