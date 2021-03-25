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

type fun_decl = Typing_defs.fun_elt

type class_decl = Typing_classes_heap.Api.t

type record_def_decl = Typing_defs.record_def_type

type typedef_decl = Typing_defs.typedef_type

type gconst_decl = Typing_defs.const_decl

let err_not_found (file : Relative_path.t) (name : string) : 'a =
  let err_str =
    Printf.sprintf "%s not found in %s" name (Relative_path.to_absolute file)
  in
  raise (Decl_defs.Decl_not_found err_str)

let direct_decl_parse_and_cache ctx filename name =
  match Direct_decl_utils.direct_decl_parse_and_cache ctx filename with
  | None -> err_not_found filename name
  | Some (decls, _mode, _hash) -> decls

let use_direct_decl_parser ctx =
  TypecheckerOptions.use_direct_decl_parser (Provider_context.get_tcopt ctx)

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
    | Some (_name, decl_and_members) -> decl_and_members)

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
      match Cache.get class_name with
      | Some t -> Some (counter, t)
      | None ->
        begin
          match
            Decl_store.((get ()).get_class class_name)
            |> Option.map ~f:Typing_classes_heap.make_eager_class_decl
          with
          | None -> None
          | Some v ->
            Cache.add class_name v;
            Some (counter, v)
        end
    end
  | Provider_backend.Shared_memory
  | Provider_backend.Decl_service _ ->
    begin
      match Cache.get class_name with
      | Some t -> Some (counter, t)
      | None ->
        begin
          match
            Typing_classes_heap.get ctx class_name declare_folded_class_in_file
          with
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
            Typing_classes_heap.get ctx class_name declare_folded_class_in_file
          in
          Option.map v ~f:Obj.repr)
    in
    (match result with
    | None -> None
    | Some obj ->
      let v : Typing_classes_heap.class_t = Obj.obj obj in
      Some (counter, v))

let declare_fun_in_file
    (ctx : Provider_context.t) (file : Relative_path.t) (name : fun_key) :
    Typing_defs.fun_elt =
  match Ast_provider.find_fun_in_file ctx file name with
  | Some f ->
    let (_name, decl) = Decl_nast.fun_naming_and_decl ctx f in
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
  | Provider_backend.Shared_memory ->
    (match Decl_store.((get ()).get_fun fun_name) with
    | Some c -> Some c
    | None ->
      (match Naming_provider.get_fun_path ctx fun_name with
      | Some filename ->
        if use_direct_decl_parser ctx then
          direct_decl_parse_and_cache ctx filename fun_name
          |> List.find_map ~f:(function
                 | (name, Shallow_decl_defs.Fun decl)
                   when String.equal fun_name name ->
                   Some decl
                 | _ -> None)
        else
          let ft =
            Errors.run_in_decl_mode filename (fun () ->
                declare_fun_in_file ctx filename fun_name)
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
            direct_decl_parse_and_cache ctx filename fun_name
            |> List.find_map ~f:(function
                   | (name, Shallow_decl_defs.Fun decl)
                     when String.equal fun_name name ->
                     Some decl
                   | _ -> None)
          else
            let ft =
              Errors.run_in_decl_mode filename (fun () ->
                  declare_fun_in_file ctx filename fun_name)
            in
            Some ft
        | None -> None)
  | Provider_backend.Decl_service { decl; _ } ->
    Decl_service_client.rpc_get_fun decl fun_name

let declare_typedef_in_file
    (ctx : Provider_context.t) (file : Relative_path.t) (name : type_key) :
    Typing_defs.typedef_type =
  match Ast_provider.find_typedef_in_file ctx file name with
  | Some t ->
    let (_name, decl) = Decl_nast.typedef_naming_and_decl ctx t in
    decl
  | None -> err_not_found file name

let get_typedef
    ?(tracing_info : Decl_counters.tracing_info option)
    (ctx : Provider_context.t)
    (typedef_name : type_key) : typedef_decl option =
  Decl_counters.count_decl Decl_counters.Typedef ?tracing_info typedef_name
  @@ fun _counter ->
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis -> Decl_store.((get ()).get_typedef typedef_name)
  | Provider_backend.Shared_memory ->
    (match Decl_store.((get ()).get_typedef typedef_name) with
    | Some c -> Some c
    | None ->
      (match Naming_provider.get_typedef_path ctx typedef_name with
      | Some filename ->
        if use_direct_decl_parser ctx then
          direct_decl_parse_and_cache ctx filename typedef_name
          |> List.find_map ~f:(function
                 | (name, Shallow_decl_defs.Typedef decl)
                   when String.equal typedef_name name ->
                   Some decl
                 | _ -> None)
        else
          let tdecl =
            Errors.run_in_decl_mode filename (fun () ->
                declare_typedef_in_file ctx filename typedef_name)
          in
          Decl_store.((get ()).add_typedef typedef_name tdecl);
          Some tdecl
      | None -> None))
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } ->
    Provider_backend.Decl_cache.find_or_add
      decl_cache
      ~key:(Provider_backend.Decl_cache_entry.Typedef_decl typedef_name)
      ~default:(fun () ->
        match Naming_provider.get_typedef_path ctx typedef_name with
        | Some filename ->
          if use_direct_decl_parser ctx then
            direct_decl_parse_and_cache ctx filename typedef_name
            |> List.find_map ~f:(function
                   | (name, Shallow_decl_defs.Typedef decl)
                     when String.equal typedef_name name ->
                     Some decl
                   | _ -> None)
          else
            let tdecl =
              Errors.run_in_decl_mode filename (fun () ->
                  declare_typedef_in_file ctx filename typedef_name)
            in
            Some tdecl
        | None -> None)
  | Provider_backend.Decl_service { decl; _ } ->
    Decl_service_client.rpc_get_typedef decl typedef_name

let declare_record_def_in_file
    (ctx : Provider_context.t) (file : Relative_path.t) (name : type_key) :
    Typing_defs.record_def_type =
  match Ast_provider.find_record_def_in_file ctx file name with
  | Some rd ->
    let (_name, decl) = Decl_nast.record_def_naming_and_decl ctx rd in
    decl
  | None -> err_not_found file name

let get_record_def
    ?(tracing_info : Decl_counters.tracing_info option)
    (ctx : Provider_context.t)
    (record_name : type_key) : record_def_decl option =
  Decl_counters.count_decl Decl_counters.Record_def ?tracing_info record_name
  @@ fun _counter ->
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis -> Decl_store.((get ()).get_recorddef record_name)
  | Provider_backend.Shared_memory ->
    (match Decl_store.((get ()).get_recorddef record_name) with
    | Some c -> Some c
    | None ->
      (match Naming_provider.get_record_def_path ctx record_name with
      | Some filename ->
        if use_direct_decl_parser ctx then
          direct_decl_parse_and_cache ctx filename record_name
          |> List.find_map ~f:(function
                 | (name, Shallow_decl_defs.Record decl)
                   when String.equal record_name name ->
                   Some decl
                 | _ -> None)
        else
          let record_decl =
            Errors.run_in_decl_mode filename (fun () ->
                declare_record_def_in_file ctx filename record_name)
          in
          Decl_store.((get ()).add_recorddef record_name record_decl);
          Some record_decl
      | None -> None))
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } ->
    Provider_backend.Decl_cache.find_or_add
      decl_cache
      ~key:(Provider_backend.Decl_cache_entry.Record_decl record_name)
      ~default:(fun () ->
        match Naming_provider.get_record_def_path ctx record_name with
        | Some filename ->
          if use_direct_decl_parser ctx then
            direct_decl_parse_and_cache ctx filename record_name
            |> List.find_map ~f:(function
                   | (name, Shallow_decl_defs.Record decl)
                     when String.equal record_name name ->
                     Some decl
                   | _ -> None)
          else
            let rdecl =
              Errors.run_in_decl_mode filename (fun () ->
                  declare_record_def_in_file ctx filename record_name)
            in
            Some rdecl
        | None -> None)
  | Provider_backend.Decl_service { decl; _ } ->
    Decl_service_client.rpc_get_record_def decl record_name

let declare_const_in_file
    (ctx : Provider_context.t) (file : Relative_path.t) (name : gconst_key) :
    gconst_decl =
  match Ast_provider.find_gconst_in_file ctx file name with
  | Some cst ->
    let (_name, decl) = Decl_nast.const_naming_and_decl ctx cst in
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
  | Provider_backend.Shared_memory ->
    (match Decl_store.((get ()).get_gconst gconst_name) with
    | Some c -> Some c
    | None ->
      (match Naming_provider.get_const_path ctx gconst_name with
      | Some filename ->
        if use_direct_decl_parser ctx then
          direct_decl_parse_and_cache ctx filename gconst_name
          |> List.find_map ~f:(function
                 | (name, Shallow_decl_defs.Const decl)
                   when String.equal gconst_name name ->
                   Some decl
                 | _ -> None)
        else
          let gconst =
            Errors.run_in_decl_mode filename (fun () ->
                declare_const_in_file ctx filename gconst_name)
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
            direct_decl_parse_and_cache ctx filename gconst_name
            |> List.find_map ~f:(function
                   | (name, Shallow_decl_defs.Const decl)
                     when String.equal gconst_name name ->
                     Some decl
                   | _ -> None)
          else
            let gconst =
              Errors.run_in_decl_mode filename (fun () ->
                  declare_const_in_file ctx filename gconst_name)
            in
            Some gconst
        | None -> None)
  | Provider_backend.Decl_service { decl; _ } ->
    Decl_service_client.rpc_get_gconst decl gconst_name

let prepare_for_typecheck
    (ctx : Provider_context.t) (path : Relative_path.t) (content : string) :
    unit =
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis
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

let local_changes_push_sharedmem_stack () =
  Decl_store.((get ()).push_local_changes ())

let local_changes_pop_sharedmem_stack () =
  Decl_store.((get ()).pop_local_changes ())
