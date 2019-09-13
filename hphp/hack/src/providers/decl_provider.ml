(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module Class = Typing_classes_heap.Api

type fun_key = string

type class_key = string

type typedef_key = string

type gconst_key = string

type fun_decl = Typing_defs.decl_fun_type

type class_decl = Typing_classes_heap.Api.t

type typedef_decl = Typing_defs.typedef_type

type gconst_decl = Typing_defs.decl_ty * Errors.t

let get_fun (fun_name : fun_key) : fun_decl option =
  match Provider_config.get_backend () with
  | Provider_config.Lru_shared_memory -> Decl_lru_cache.get_fun fun_name
  | Provider_config.Shared_memory -> Typing_lazy_heap.get_fun fun_name
  | Provider_config.Local_memory { decl_cache } ->
    let result : Obj.t =
      Memory_bounded_lru_cache.find_or_add
        decl_cache
        ~key:(Provider_config.Fun_decl fun_name)
        ~default:(fun () ->
          let result : fun_decl option =
            match Naming_table.Funs.get_pos fun_name with
            | Some pos ->
              let filename = FileInfo.get_pos_filename pos in
              let ft =
                Errors.run_in_decl_mode filename (fun () ->
                    Decl.declare_fun_in_file filename fun_name)
              in
              Some ft
            | None -> None
          in
          Obj.repr result)
    in
    let result : fun_decl option = Obj.obj result in
    result

let get_class (class_name : class_key) : class_decl option =
  match Provider_config.get_backend () with
  | Provider_config.Lru_shared_memory
  | Provider_config.Shared_memory ->
    Typing_lazy_heap.get_class class_name
  | Provider_config.Local_memory { decl_cache } ->
    let result : Obj.t =
      Memory_bounded_lru_cache.find_or_add
        decl_cache
        ~key:(Provider_config.Class_decl class_name)
        ~default:(fun () ->
          let result : class_decl option =
            Typing_classes_heap.compute_class_decl_no_cache class_name
          in
          Obj.repr result)
    in
    let result : class_decl option = Obj.obj result in
    result

let convert_class_elt_to_fun_decl class_elt_opt : fun_decl option =
  match class_elt_opt with
  | Some { Typing_defs.ce_type = (lazy (_, Typing_defs.Tfun ft)); _ } ->
    Some ft
  | _ -> None

let get_class_constructor (class_name : class_key) : fun_decl option =
  match get_class class_name with
  | None -> None
  | Some cls ->
    let (class_elt_option, _) = Typing_classes_heap.Api.construct cls in
    convert_class_elt_to_fun_decl class_elt_option

let get_class_method (class_name : class_key) (method_name : fun_key) :
    fun_decl option =
  match get_class class_name with
  | None -> None
  | Some cls ->
    let meth = Class.get_method cls method_name in
    convert_class_elt_to_fun_decl meth

let get_static_method (class_name : class_key) (method_name : fun_key) :
    fun_decl option =
  match get_class class_name with
  | None -> None
  | Some cls ->
    let smeth = Class.get_smethod cls method_name in
    convert_class_elt_to_fun_decl smeth

let get_type_id_filename x expected_kind =
  match Naming_table.Types.get_pos x with
  | Some (pos, kind) when kind = expected_kind ->
    let res = FileInfo.get_pos_filename pos in
    Some res
  | _ -> None

let get_typedef (typedef_name : string) : typedef_decl option =
  match Provider_config.get_backend () with
  | Provider_config.Lru_shared_memory ->
    Decl_lru_cache.get_typedef typedef_name
  | Provider_config.Shared_memory -> Typing_lazy_heap.get_typedef typedef_name
  | Provider_config.Local_memory { decl_cache } ->
    let result : Obj.t =
      Memory_bounded_lru_cache.find_or_add
        decl_cache
        ~key:(Provider_config.Typedef_decl typedef_name)
        ~default:(fun () ->
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
          Obj.repr result)
    in
    let result : typedef_decl option = Obj.obj result in
    result

let get_gconst (gconst_name : string) : gconst_decl option =
  match Provider_config.get_backend () with
  | Provider_config.Lru_shared_memory -> Decl_lru_cache.get_gconst gconst_name
  | Provider_config.Shared_memory -> Typing_lazy_heap.get_gconst gconst_name
  | Provider_config.Local_memory { decl_cache } ->
    let result : Obj.t =
      Memory_bounded_lru_cache.find_or_add
        decl_cache
        ~key:(Provider_config.Gconst_decl gconst_name)
        ~default:(fun () ->
          let result : gconst_decl option =
            match Naming_table.Consts.get_pos gconst_name with
            | Some pos ->
              let filename = FileInfo.get_pos_filename pos in
              let gconst =
                Errors.run_in_decl_mode filename (fun () ->
                    Decl.declare_const_in_file filename gconst_name)
              in
              Some gconst
            | None -> None
          in
          Obj.repr result)
    in
    let result : gconst_decl option = Obj.obj result in
    result

let invalidate_fun (fun_name : fun_key) : unit =
  match Provider_config.get_backend () with
  | Provider_config.Lru_shared_memory ->
    failwith
      "Function decl invalidation not yet supported with LRU shared memory"
  | Provider_config.Shared_memory ->
    Decl_heap.Funs.remove_batch (SSet.singleton fun_name)
  | Provider_config.Local_memory { decl_cache } ->
    Memory_bounded_lru_cache.remove
      decl_cache
      (Provider_config.Fun_decl fun_name)

let invalidate_class (class_name : class_key) : unit =
  match Provider_config.get_backend () with
  | Provider_config.Lru_shared_memory ->
    failwith "Class decl invalidation not yet supported with LRU shared memory"
  | Provider_config.Shared_memory ->
    Decl_heap.Classes.remove_batch (SSet.singleton class_name)
  | Provider_config.Local_memory { decl_cache } ->
    Memory_bounded_lru_cache.remove
      decl_cache
      (Provider_config.Class_decl class_name)

let invalidate_typedef (typedef_name : typedef_key) : unit =
  match Provider_config.get_backend () with
  | Provider_config.Lru_shared_memory ->
    failwith
      "Typedef decl invalidation not yet supported with LRU shared memory"
  | Provider_config.Shared_memory ->
    Decl_heap.Typedefs.remove_batch (SSet.singleton typedef_name)
  | Provider_config.Local_memory { decl_cache } ->
    Memory_bounded_lru_cache.remove
      decl_cache
      (Provider_config.Typedef_decl typedef_name)

let invalidate_gconst (gconst_name : gconst_key) : unit =
  match Provider_config.get_backend () with
  | Provider_config.Lru_shared_memory ->
    failwith
      "Constant decl invalidation not yet supported with LRU shared memory"
  | Provider_config.Shared_memory ->
    Decl_heap.GConsts.remove_batch (SSet.singleton gconst_name)
  | Provider_config.Local_memory { decl_cache } ->
    Memory_bounded_lru_cache.remove
      decl_cache
      (Provider_config.Gconst_decl gconst_name)

let invalidate_context_decls ~(ctx : Provider_context.t) =
  match Provider_config.get_backend () with
  | Provider_config.Local_memory _ ->
    Relative_path.Map.iter ctx.Provider_context.entries ~f:(fun _ entry ->
        let (funs, classes, typedefs, gconsts) =
          Nast.get_defs entry.Provider_context.ast
        in
        List.iter funs ~f:(fun (_, fun_name) -> invalidate_fun fun_name);
        List.iter classes ~f:(fun (_, class_name) ->
            invalidate_class class_name);
        List.iter typedefs ~f:(fun (_, typedef_name) ->
            invalidate_typedef typedef_name);
        List.iter gconsts ~f:(fun (_, gconst_name) ->
            invalidate_gconst gconst_name))
  | Provider_config.Shared_memory
  | Provider_config.Lru_shared_memory ->
    (* Don't attempt to invalidate decls with shared memory, as we may not be
    running in the master process where that's allowed. *)
    ()

let local_changes_push_stack () =
  (* For now, decl production still writes into shared memory, even when we're
  not using shared memory as the principal decl store. Until we change decl
  production to write into the `Decl_provider` interface, we'll enable the local
  changes heap. After it uses the `Decl_provider` interface, we'll move the
  below into the following `Shared_memory` match case. *)
  Decl_heap.Funs.LocalChanges.push_stack ();
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

  match Provider_context.get_global_context () with
  | Some ctx -> invalidate_context_decls ~ctx
  | None -> ()

let local_changes_pop_stack () =
  (* See comment in [local_changes_push_stack] above. *)
  Decl_heap.Funs.LocalChanges.pop_stack ();
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

  match Provider_context.get_global_context () with
  | Some ctx -> invalidate_context_decls ~ctx
  | None -> ()
