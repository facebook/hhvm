(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
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

type gconst_decl = Typing_defs.decl_ty * Errors.t

let get_fun (ctx : Provider_context.t) (fun_name : fun_key) : fun_decl option =
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory -> Typing_lazy_heap.get_fun fun_name
  | Provider_backend.Local_memory { decl_cache } ->
    let result : Obj.t =
      Provider_backend.Decl_cache.find_or_add
        decl_cache
        ~key:(Provider_backend.Decl_cache_entry.Fun_decl fun_name)
        ~default:(fun () ->
          let start_time = Unix.gettimeofday () in
          let result : fun_decl option =
            match Naming_table.Funs.get_filename fun_name with
            | Some filename ->
              let ft =
                Errors.run_in_decl_mode filename (fun () ->
                    Decl.declare_fun_in_file filename fun_name)
              in
              Some ft
            | None -> None
          in
          Deferred_decl.count_decl_cache_miss fun_name ~start_time;
          Obj.repr result)
    in
    let result : fun_decl option = Obj.obj result in
    result
  | Provider_backend.Decl_service _ ->
    failwith "Decl_provider.get_fun not yet impl. for decl memory provider"

let get_class (ctx : Provider_context.t) (class_name : class_key) :
    class_decl option =
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory -> Typing_lazy_heap.get_class class_name
  | Provider_backend.Local_memory { decl_cache } ->
    let result : Obj.t =
      Provider_backend.Decl_cache.find_or_add
        decl_cache
        ~key:(Provider_backend.Decl_cache_entry.Class_decl class_name)
        ~default:(fun () ->
          let start_time = Unix.gettimeofday () in
          let result : class_decl option =
            Typing_classes_heap.compute_class_decl_no_cache class_name
          in
          Deferred_decl.count_decl_cache_miss class_name ~start_time;
          Obj.repr result)
    in
    let result : class_decl option = Obj.obj result in
    result
  | Provider_backend.Decl_service _ ->
    failwith "Decl_provider.get_class not yet impl. for decl memory provider"

let convert_class_elt_to_fun_decl class_elt_opt : fun_decl option =
  Typing_defs.(
    match class_elt_opt with
    | Some { ce_type = (lazy ty); ce_deprecated; ce_pos = (lazy pos); _ } ->
      Some
        {
          fe_pos = pos;
          fe_type = ty;
          fe_deprecated = ce_deprecated;
          fe_decl_errors = None;
        }
    | _ -> None)

let get_class_constructor (ctx : Provider_context.t) (class_name : class_key) :
    fun_decl option =
  match get_class ctx class_name with
  | None -> None
  | Some cls ->
    let (class_elt_option, _) = Typing_classes_heap.Api.construct cls in
    convert_class_elt_to_fun_decl class_elt_option

let get_class_method
    (ctx : Provider_context.t) (class_name : class_key) (method_name : fun_key)
    : fun_decl option =
  match get_class ctx class_name with
  | None -> None
  | Some cls ->
    let meth = Class.get_method cls method_name in
    convert_class_elt_to_fun_decl meth

let get_static_method
    (ctx : Provider_context.t) (class_name : class_key) (method_name : fun_key)
    : fun_decl option =
  match get_class ctx class_name with
  | None -> None
  | Some cls ->
    let smeth = Class.get_smethod cls method_name in
    convert_class_elt_to_fun_decl smeth

let get_type_id_filename x expected_kind =
  match Naming_table.Types.get_filename_and_kind x with
  | Some (fn, kind) when kind = expected_kind -> Some fn
  | _ -> None

let get_typedef (ctx : Provider_context.t) (typedef_name : string) :
    typedef_decl option =
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory -> Typing_lazy_heap.get_typedef typedef_name
  | Provider_backend.Local_memory { decl_cache } ->
    let result : Obj.t =
      Provider_backend.Decl_cache.find_or_add
        decl_cache
        ~key:(Provider_backend.Decl_cache_entry.Typedef_decl typedef_name)
        ~default:(fun () ->
          let start_time = Unix.gettimeofday () in
          let result : typedef_decl option =
            match get_type_id_filename typedef_name Naming_table.TTypedef with
            | Some filename ->
              let tdecl =
                Errors.run_in_decl_mode filename (fun () ->
                    Decl.declare_typedef_in_file filename typedef_name)
              in
              Some tdecl
            | None -> None
          in
          Deferred_decl.count_decl_cache_miss typedef_name ~start_time;
          Obj.repr result)
    in
    let result : typedef_decl option = Obj.obj result in
    result
  | Provider_backend.Decl_service _ ->
    failwith "Decl_provider.get_typedef not yet impl. for decl memory provider"

let get_record_def (ctx : Provider_context.t) (record_name : string) :
    record_def_decl option =
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory ->
    Typing_lazy_heap.get_record_def record_name
  | Provider_backend.Local_memory { decl_cache } ->
    let result : Obj.t =
      Provider_backend.Decl_cache.find_or_add
        decl_cache
        ~key:(Provider_backend.Decl_cache_entry.Record_decl record_name)
        ~default:(fun () ->
          let start_time = Unix.gettimeofday () in
          let result : record_def_decl option =
            match Naming_table.Consts.get_filename record_name with
            | Some filename ->
              let rdecl =
                Errors.run_in_decl_mode filename (fun () ->
                    Decl.declare_record_def_in_file filename record_name)
              in
              Some rdecl
            | None -> None
          in
          Deferred_decl.count_decl_cache_miss record_name ~start_time;
          Obj.repr result)
    in
    let result : record_def_decl option = Obj.obj result in
    result
  | Provider_backend.Decl_service _ ->
    failwith
      "Decl_provider.get_record_def not yet impl. for decl memory provider"

let get_gconst (ctx : Provider_context.t) (gconst_name : string) :
    gconst_decl option =
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory -> Typing_lazy_heap.get_gconst gconst_name
  | Provider_backend.Local_memory { decl_cache } ->
    let result : Obj.t =
      Provider_backend.Decl_cache.find_or_add
        decl_cache
        ~key:(Provider_backend.Decl_cache_entry.Gconst_decl gconst_name)
        ~default:(fun () ->
          let start_time = Unix.gettimeofday () in
          let result : gconst_decl option =
            match Naming_table.Consts.get_filename gconst_name with
            | Some filename ->
              let gconst =
                Errors.run_in_decl_mode filename (fun () ->
                    Decl.declare_const_in_file filename gconst_name)
              in
              Some gconst
            | None -> None
          in
          Deferred_decl.count_decl_cache_miss gconst_name ~start_time;
          Obj.repr result)
    in
    let result : gconst_decl option = Obj.obj result in
    result
  | Provider_backend.Decl_service decl ->
    decl.Decl_service_client.rpc_get_gconst gconst_name
    |> Option.map ~f:(fun decl -> (decl, Errors.empty))

let invalidate_fun (ctx : Provider_context.t) (fun_name : fun_key) : unit =
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory ->
    Decl_heap.Funs.remove_batch (SSet.singleton fun_name)
  | Provider_backend.Local_memory { decl_cache } ->
    Provider_backend.Decl_cache.remove
      decl_cache
      (Provider_backend.Decl_cache_entry.Fun_decl fun_name)
  | Provider_backend.Decl_service _ ->
    failwith
      "Decl_provider.invalidate_fun not yet impl. for decl memory provider"

let invalidate_class (ctx : Provider_context.t) (class_name : class_key) : unit
    =
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory ->
    Decl_heap.Classes.remove_batch (SSet.singleton class_name)
  | Provider_backend.Local_memory { decl_cache } ->
    Provider_backend.Decl_cache.remove
      decl_cache
      (Provider_backend.Decl_cache_entry.Class_decl class_name)
  | Provider_backend.Decl_service _ ->
    failwith
      "Decl_provider.invalidate_class not yet impl. for decl memory provider"

let invalidate_record_def
    (ctx : Provider_context.t) (record_name : record_def_key) : unit =
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory ->
    Decl_heap.RecordDefs.remove_batch (SSet.singleton record_name)
  | Provider_backend.Local_memory { decl_cache } ->
    Provider_backend.Decl_cache.remove
      decl_cache
      (Provider_backend.Decl_cache_entry.Record_decl record_name)
  | Provider_backend.Decl_service _ ->
    failwith
      "Decl_provider.invalidate_record_def not yet impl. for decl memory provider"

let invalidate_typedef (ctx : Provider_context.t) (typedef_name : typedef_key) :
    unit =
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory ->
    Decl_heap.Typedefs.remove_batch (SSet.singleton typedef_name)
  | Provider_backend.Local_memory { decl_cache } ->
    Provider_backend.Decl_cache.remove
      decl_cache
      (Provider_backend.Decl_cache_entry.Typedef_decl typedef_name)
  | Provider_backend.Decl_service _ ->
    failwith
      "Decl_provider.invalidate_typedef not yet impl. for decl memory provider"

let invalidate_gconst (ctx : Provider_context.t) (gconst_name : gconst_key) :
    unit =
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory ->
    Decl_heap.GConsts.remove_batch (SSet.singleton gconst_name)
  | Provider_backend.Local_memory { decl_cache } ->
    Provider_backend.Decl_cache.remove
      decl_cache
      (Provider_backend.Decl_cache_entry.Gconst_decl gconst_name)
  | Provider_backend.Decl_service _ ->
    failwith
      "Decl_provider.invalidate_gconst not yet impl. for decl memory provider"

let invalidate_context_decls ~(ctx : Provider_context.t) =
  match ctx.Provider_context.backend with
  | Provider_backend.Local_memory _ ->
    Relative_path.Map.iter ctx.Provider_context.entries ~f:(fun _ entry ->
        let (funs, classes, record_defs, typedefs, gconsts) =
          Nast.get_defs entry.Provider_context.ast
        in
        List.iter funs ~f:(fun (_, fun_name) -> invalidate_fun ctx fun_name);
        List.iter classes ~f:(fun (_, class_name) ->
            invalidate_class ctx class_name);
        List.iter record_defs ~f:(fun (_, record_name) ->
            invalidate_record_def ctx record_name);
        List.iter typedefs ~f:(fun (_, typedef_name) ->
            invalidate_typedef ctx typedef_name);
        List.iter gconsts ~f:(fun (_, gconst_name) ->
            invalidate_gconst ctx gconst_name))
  | Provider_backend.Shared_memory ->
    (* Don't attempt to invalidate decls with shared memory, as we may not be
    running in the master process where that's allowed. *)
    ()
  | Provider_backend.Decl_service _ ->
    failwith
      "Decl_provider.invalidate_context_decls not yet impl. for decl memory provider"

let local_changes_push_stack (ctx : Provider_context.t) =
  (* For now, decl production still writes into shared memory, even when we're
  not using shared memory as the principal decl store. Until we change decl
  production to write into the `Decl_provider` interface, we'll enable the local
  changes heap. After it uses the `Decl_provider` interface, we'll move the
  below into the following `Shared_memory` match case. *)
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

  Shallow_classes_heap.push_local_changes ();
  Decl_linearize.push_local_changes ();

  invalidate_context_decls ~ctx

let local_changes_pop_stack (ctx : Provider_context.t) =
  (* See comment in [local_changes_push_stack] above. *)
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

  Shallow_classes_heap.pop_local_changes ();
  Decl_linearize.pop_local_changes ();

  invalidate_context_decls ~ctx
