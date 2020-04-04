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
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory ->
    Typing_lazy_heap.get_fun ~sh:SharedMem.Uses ctx fun_name
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

let get_class (ctx : Provider_context.t) (class_name : class_key) :
    class_decl option =
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory -> Typing_lazy_heap.get_class ctx class_name
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } ->
    let result : Obj.t option =
      Provider_backend.Decl_cache.find_or_add
        decl_cache
        ~key:(Provider_backend.Decl_cache_entry.Class_decl class_name)
        ~default:(fun () ->
          let result : class_decl option =
            Typing_classes_heap.compute_class_decl_no_cache ctx class_name
          in
          Option.map result ~f:Obj.repr)
    in
    let result : class_decl option = Option.map result ~f:Obj.obj in
    result
  | Provider_backend.Decl_service _ ->
    (* The decl service caches shallow decls, so we communicate with it in
       Shallow_classes_provider. Typing_lazy_heap lazily folds shallow decls to
       provide a folded-decl API.  *)
    Typing_lazy_heap.get_class ctx class_name

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

let get_type_id_filename ctx x expected_kind =
  match Naming_provider.get_type_path_and_kind ctx x with
  | Some (pos, kind) when kind = expected_kind -> Some pos
  | _ -> None

let get_typedef (ctx : Provider_context.t) (typedef_name : string) :
    typedef_decl option =
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory ->
    Typing_lazy_heap.get_typedef ~sh:SharedMem.Uses ctx typedef_name
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } ->
    Provider_backend.Decl_cache.find_or_add
      decl_cache
      ~key:(Provider_backend.Decl_cache_entry.Typedef_decl typedef_name)
      ~default:(fun () ->
        match get_type_id_filename ctx typedef_name Naming_types.TTypedef with
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

let get_record_def (ctx : Provider_context.t) (record_name : string) :
    record_def_decl option =
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory ->
    Typing_lazy_heap.get_record_def ~sh:SharedMem.Uses ctx record_name
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
  | Provider_backend.Decl_service _ ->
    failwith
      "Decl_provider.get_record_def not yet impl. for decl memory provider"

let get_gconst (ctx : Provider_context.t) (gconst_name : string) :
    gconst_decl option =
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory ->
    Typing_lazy_heap.get_gconst ~sh:SharedMem.Uses ctx gconst_name
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
    |> Option.map ~f:(fun decl -> (decl, Errors.empty))

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
