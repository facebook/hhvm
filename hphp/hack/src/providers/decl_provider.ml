(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Class = Typing_classes_heap.Api

type fun_key = string
type class_key = string
type typedef_key = string
type gconst_key = string

type fun_decl = Typing_defs.decl Typing_defs.fun_type
type class_decl = Typing_classes_heap.Api.t
type typedef_decl = Typing_defs.typedef_type
type gconst_decl = Typing_defs.decl Typing_defs.ty * Errors.t

let get_fun (fun_name: fun_key): fun_decl option =
  match Provider_config.get_backend () with
  | Provider_config.Lru_shared_memory ->
    Decl_lru_cache.get_fun fun_name
  | Provider_config.Shared_memory ->
    Typing_lazy_heap.get_fun fun_name
  | Provider_config.Local_memory { decl_cache } ->
    let result: Obj.t =
      Memory_bounded_lru_cache.find_or_add
        decl_cache
        ~key:(Provider_config.Fun_decl fun_name)
        ~default:(fun () ->
          let result: fun_decl option =
            match Naming_table.Funs.get_pos fun_name with
            | Some pos ->
              let filename = FileInfo.get_pos_filename pos in
              let ft = Errors.run_in_decl_mode filename
                (fun () -> Decl.declare_fun_in_file filename fun_name) in
              Some ft
            | None -> None
          in
          Obj.repr result
        )
    in
    let result: fun_decl option = Obj.obj result in
    result

let get_class (class_name: class_key): class_decl option =
  match Provider_config.get_backend () with
  | Provider_config.Lru_shared_memory
  | Provider_config.Shared_memory ->
    Typing_lazy_heap.get_class class_name
  | Provider_config.Local_memory { decl_cache } ->
    let result: Obj.t =
      Memory_bounded_lru_cache.find_or_add
        decl_cache
        ~key:(Provider_config.Class_decl class_name)
        ~default:(fun () ->
          let result: class_decl option =
            Typing_classes_heap.compute_class_decl_no_cache class_name in
          Obj.repr result
        )
    in
    let result: class_decl option = Obj.obj result in
    result

let get_type_id_filename x expected_kind =
  match Naming_table.Types.get_pos x with
  | Some (pos, kind) when kind = expected_kind ->
    let res = FileInfo.get_pos_filename pos in
    Some res
  | _ -> None

let get_typedef (typedef_name: string): typedef_decl option =
  match Provider_config.get_backend () with
  | Provider_config.Lru_shared_memory ->
    Decl_lru_cache.get_typedef typedef_name
  | Provider_config.Shared_memory ->
    Typing_lazy_heap.get_typedef typedef_name
  | Provider_config.Local_memory { decl_cache } ->
    let result: Obj.t =
      Memory_bounded_lru_cache.find_or_add
        decl_cache
        ~key:(Provider_config.Typedef_decl typedef_name)
        ~default:(fun () ->
          let result: typedef_decl option =
            match get_type_id_filename typedef_name Naming_table.TTypedef with
            | Some filename ->
              let tdecl = Errors.run_in_decl_mode filename
                (fun () ->
                  Decl.declare_typedef_in_file filename typedef_name) in
              Some tdecl
            | None -> None
          in
          Obj.repr result
        )
    in
    let result: typedef_decl option = Obj.obj result in
    result

let get_gconst (gconst_name: string): gconst_decl option =
  match Provider_config.get_backend () with
  | Provider_config.Lru_shared_memory ->
    Decl_lru_cache.get_gconst gconst_name
  | Provider_config.Shared_memory ->
    Typing_lazy_heap.get_gconst gconst_name
  | Provider_config.Local_memory { decl_cache } ->
    let result: Obj.t =
      Memory_bounded_lru_cache.find_or_add
        decl_cache
        ~key:(Provider_config.Gconst_decl gconst_name)
        ~default:(fun () ->
          let result: gconst_decl option =
            match Naming_table.Consts.get_pos gconst_name with
            | Some pos ->
                let filename = FileInfo.get_pos_filename pos in
                let gconst = Errors.run_in_decl_mode filename
                  (fun () -> Decl.declare_const_in_file filename gconst_name) in
                Some gconst
            | None -> None
          in
          Obj.repr result
        )
    in
    let result: gconst_decl option = Obj.obj result in
    result


let local_changes_push_stack () =
  (* For now, decl production still writes into shared memory, even when we're
  not using shared memory as the principal decl store. Until we change decl
  production to write into the `Decl_provider` interface, we'll enable the local
  changes heap. After it uses the `Decl_provider` interface, we'll move the
  below into the following `Shared_memory` match case. *)
  Decl_heap.Funs.LocalChanges.push_stack();
  Decl_heap.Constructors.LocalChanges.push_stack();
  Decl_heap.Props.LocalChanges.push_stack();
  Decl_heap.StaticProps.LocalChanges.push_stack();
  Decl_heap.Methods.LocalChanges.push_stack();
  Decl_heap.StaticMethods.LocalChanges.push_stack();
  Decl_heap.Classes.LocalChanges.push_stack();
  Decl_heap.Typedefs.LocalChanges.push_stack();
  Decl_heap.GConsts.LocalChanges.push_stack();

  Shallow_classes_heap.push_local_changes ();
  Decl_linearize.push_local_changes ();

  match Provider_config.get_backend () with
  | Provider_config.Lru_shared_memory
  | Provider_config.Shared_memory ->
    ()
  | Provider_config.Local_memory { decl_cache } ->
    Memory_bounded_lru_cache.clear decl_cache

let local_changes_pop_stack () =
  (* See comment in [local_changes_push_stack] above. *)
  Decl_heap.Funs.LocalChanges.pop_stack();
  Decl_heap.Constructors.LocalChanges.pop_stack();
  Decl_heap.Props.LocalChanges.pop_stack();
  Decl_heap.StaticProps.LocalChanges.pop_stack();
  Decl_heap.Methods.LocalChanges.pop_stack();
  Decl_heap.StaticMethods.LocalChanges.pop_stack();
  Decl_heap.Classes.LocalChanges.pop_stack();
  Decl_heap.Typedefs.LocalChanges.pop_stack();
  Decl_heap.GConsts.LocalChanges.pop_stack();

  Shallow_classes_heap.pop_local_changes ();
  Decl_linearize.pop_local_changes ();

  match Provider_config.get_backend () with
  | Provider_config.Lru_shared_memory
  | Provider_config.Shared_memory ->
    ()
  | Provider_config.Local_memory { decl_cache } ->
    Memory_bounded_lru_cache.clear decl_cache
