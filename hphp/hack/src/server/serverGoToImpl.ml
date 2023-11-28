(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerEnv
open Reordered_argument_collections
open ServerCommandTypes.Find_refs
open ServerCommandTypes.Done_or_retry
open Typing_defs

let parallel_limit = 10

let find_positions_of_classes
    (ctx : Provider_context.t)
    (acc : (string * Pos.t) list)
    (child_classes : string list) : (string * Pos.t) list =
  acc
  @ List.map child_classes ~f:(fun child_class ->
        match Naming_provider.get_type_pos ctx child_class with
        | None ->
          failwith ("Could not find definition of child class: " ^ child_class)
        | Some (FileInfo.Full pos) -> (child_class, pos)
        | Some (FileInfo.File (FileInfo.Class, path)) ->
          (match
             Ast_provider.find_class_in_file ctx path child_class ~full:false
           with
          | None ->
            failwith
              (Printf.sprintf
                 "Could not find class %s in %s"
                 child_class
                 (Relative_path.to_absolute path))
          | Some { Aast.c_name = (name_pos, _); _ } -> (child_class, name_pos))
        | Some FileInfo.(File ((Fun | Typedef | Const | Module), _path)) ->
          failwith
            (Printf.sprintf
               "Information for class %s was returned as not a class"
               child_class))

let parallel_find_positions_of_classes
    (ctx : Provider_context.t)
    (child_classes : string list)
    (workers : MultiWorker.worker list option) : (string * Pos.t) list =
  MultiWorker.call
    workers
    ~job:(find_positions_of_classes ctx)
    ~neutral:[]
    ~merge:List.append
    ~next:(MultiWorker.next workers child_classes)

let add_if_valid_origin ctx class_elt child_class method_name result =
  if String.equal class_elt.ce_origin child_class then
    ( method_name,
      Lazy.force class_elt.ce_pos |> Naming_provider.resolve_position ctx )
    :: result
  else
    let origin_decl = Decl_provider.get_class ctx class_elt.ce_origin in
    match origin_decl with
    | Decl_entry.Found origin_decl ->
      let origin_kind = Decl_provider.Class.kind origin_decl in
      if Ast_defs.is_c_trait origin_kind then
        ( method_name,
          Lazy.force class_elt.ce_pos |> Naming_provider.resolve_position ctx )
        :: result
      else
        result
    | Decl_entry.DoesNotExist
    | Decl_entry.NotYetAvailable ->
      failwith "TODO"

let find_positions_of_methods
    (ctx : Provider_context.t)
    (method_name : string)
    (acc : (string * Pos.t) list)
    (child_classes : string list) : (string * Pos.t) list =
  List.fold child_classes ~init:acc ~f:(fun result child_class ->
      let class_decl = Decl_provider.get_class ctx child_class in
      match class_decl with
      | Decl_entry.Found decl ->
        let method_info = Decl_provider.Class.get_method decl method_name in
        (match method_info with
        | Some class_elt ->
          add_if_valid_origin ctx class_elt child_class method_name result
        | None ->
          let smethod_info = Decl_provider.Class.get_smethod decl method_name in
          (match smethod_info with
          | Some class_elt ->
            add_if_valid_origin ctx class_elt child_class method_name result
          | None -> result))
      | Decl_entry.DoesNotExist
      | Decl_entry.NotYetAvailable ->
        failwith ("Could not find definition of child class: " ^ child_class))

let parallel_find_positions_of_methods
    (ctx : Provider_context.t)
    (child_classes : string list)
    (method_name : string)
    (workers : MultiWorker.worker list option) : (string * Pos.t) list =
  MultiWorker.call
    workers
    ~job:(find_positions_of_methods ctx method_name)
    ~neutral:[]
    ~merge:List.append
    ~next:(MultiWorker.next workers child_classes)

let find_child_classes
    (ctx : Provider_context.t)
    (class_name : string)
    (genv : ServerEnv.genv)
    (env : ServerEnv.env) : string list =
  let files =
    FindRefsService.get_dependent_files
      ctx
      genv.ServerEnv.workers
      (SSet.singleton class_name)
  in
  let ctx = Provider_utils.ctx_from_server_env env in
  FindRefsService.find_child_classes ctx class_name env.naming_table files
  |> SSet.elements

let find_child_classes_in_file
    (ctx : Provider_context.t)
    (class_name : string)
    (naming_table : Naming_table.t)
    (filename : Relative_path.t) : string list =
  let fileset = Relative_path.Set.(add empty filename) in
  FindRefsService.find_child_classes ctx class_name naming_table fileset
  |> SSet.elements

let search_class
    (ctx : Provider_context.t)
    (class_name : string)
    (genv : ServerEnv.genv)
    (env : ServerEnv.env) : ServerEnv.env * server_result_or_retry =
  let class_name = ServerFindRefs.add_ns class_name in
  ServerFindRefs.handle_prechecked_files
    genv
    env
    Typing_deps.(Dep.(make (Type class_name)))
  @@ fun () ->
  let child_classes = find_child_classes ctx class_name genv env in
  if List.length child_classes < parallel_limit then
    find_positions_of_classes ctx [] child_classes
  else
    parallel_find_positions_of_classes ctx child_classes genv.workers

let search_single_file_for_class
    (ctx : Provider_context.t)
    (class_name : string)
    (naming_table : Naming_table.t)
    (filename : Relative_path.t) : server_result =
  let class_name = ServerFindRefs.add_ns class_name in
  let child_classes =
    find_child_classes_in_file ctx class_name naming_table filename
  in
  find_positions_of_classes ctx [] child_classes

let search_member
    (ctx : Provider_context.t)
    (class_name : string)
    (member : member)
    (genv : ServerEnv.genv)
    (env : ServerEnv.env) : ServerEnv.env * server_result_or_retry =
  match member with
  | Method method_name ->
    let class_name = ServerFindRefs.add_ns class_name in
    let class_name =
      FindRefsService.get_origin_class_name ctx class_name member
    in
    ServerFindRefs.handle_prechecked_files
      genv
      env
      Typing_deps.(Dep.(make (Type class_name)))
    @@ fun () ->
    (* Find all the classes that extend this one *)
    let child_classes = find_child_classes ctx class_name genv env in
    let results =
      if List.length child_classes < parallel_limit then
        find_positions_of_methods ctx method_name [] child_classes
      else
        parallel_find_positions_of_methods
          ctx
          child_classes
          method_name
          genv.workers
    in
    List.dedup_and_sort results ~compare:(fun (_, pos1) (_, pos2) ->
        Pos.compare pos1 pos2)
  | Property _
  | Class_const _
  | Typeconst _ ->
    (env, Done [])

let search_single_file_for_member
    (ctx : Provider_context.t)
    (class_name : string)
    (member : member)
    (naming_table : Naming_table.t)
    (filename : Relative_path.t) : server_result =
  match member with
  | Method method_name ->
    let class_name = ServerFindRefs.add_ns class_name in
    let class_name =
      FindRefsService.get_origin_class_name ctx class_name member
    in
    (* Find all the classes that extend this one *)
    let child_classes =
      find_child_classes_in_file ctx class_name naming_table filename
    in
    let results = find_positions_of_methods ctx method_name [] child_classes in
    List.dedup_and_sort results ~compare:(fun (_, pos1) (_, pos2) ->
        Pos.compare pos1 pos2)
  | Property _
  | Class_const _
  | Typeconst _ ->
    []

let is_searchable ~action =
  match action with
  | Class _
  | ExplicitClass _
  | Member (_, _) ->
    true
  | Function _
  | GConst _
  | LocalVar _ ->
    false

let go_for_single_file
    ~(ctx : Provider_context.t)
    ~(action : action)
    ~(naming_table : Naming_table.t)
    ~(filename : Relative_path.t) : server_result =
  match action with
  | Class class_name
  | ExplicitClass class_name ->
    search_single_file_for_class ctx class_name naming_table filename
  | Member (class_name, member) ->
    search_single_file_for_member ctx class_name member naming_table filename
  | Function _
  | GConst _
  | LocalVar _ ->
    []

let go ~(action : action) ~(genv : ServerEnv.genv) ~(env : ServerEnv.env) :
    ServerEnv.env * server_result_or_retry =
  let ctx = Provider_utils.ctx_from_server_env env in
  match action with
  | Class class_name
  | ExplicitClass class_name ->
    search_class ctx class_name genv env
  | Member (class_name, member) -> search_member ctx class_name member genv env
  | Function _
  | GConst _
  | LocalVar _ ->
    (env, Done [])
