(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open ServerEnv
open Reordered_argument_collections
open ServerCommandTypes.Find_refs
open ServerCommandTypes.Done_or_retry
open Typing_defs

let find_positions_of_classes
    (acc : (string * Pos.t) list) (child_classes : string list) :
    (string * Pos.t) list =
  acc
  @ List.map child_classes ~f:(fun child_class ->
        match Naming_table.Types.get_pos child_class with
        | None ->
          failwith ("Could not find definition of child class: " ^ child_class)
        | Some (FileInfo.Full pos, _type) -> (child_class, pos)
        | Some (FileInfo.File (FileInfo.Class, path), _type) ->
          (match Ast_provider.find_class_in_file path child_class with
          | None ->
            failwith
              (Printf.sprintf
                 "Could not find class %s in %s"
                 child_class
                 (Relative_path.to_absolute path))
          | Some { Aast.c_span; _ } -> (child_class, c_span))
        | Some
            FileInfo.(File ((Fun | RecordDef | Typedef | Const), _path), _type)
          ->
          failwith
            (Printf.sprintf
               "Information for class %s was returned as not a class"
               child_class))

let parallel_find_positions_of_classes
    (child_classes : string list) (workers : MultiWorker.worker list option) :
    (string * Pos.t) list =
  MultiWorker.call
    workers
    ~job:find_positions_of_classes
    ~neutral:[]
    ~merge:List.append
    ~next:(MultiWorker.next workers child_classes)

let add_if_valid_origin class_elt child_class method_name result =
  if class_elt.ce_origin = child_class then
    (method_name, Lazy.force class_elt.ce_pos) :: result
  else
    let origin_decl = Decl_provider.get_class class_elt.ce_origin in
    match origin_decl with
    | Some origin_decl ->
      let origin_kind = Decl_provider.Class.kind origin_decl in
      (match origin_kind with
      | Ast_defs.Ctrait -> (method_name, Lazy.force class_elt.ce_pos) :: result
      | Ast_defs.Cabstract
      | Ast_defs.Cnormal
      | Ast_defs.Cinterface
      | Ast_defs.Cenum ->
        result)
    | None -> failwith "TODO"

let find_positions_of_methods
    (acc : (string * Pos.t) list)
    (child_classes : string list)
    (method_name : string) : (string * Pos.t) list =
  List.fold child_classes ~init:acc ~f:(fun result child_class ->
      let class_decl = Decl_provider.get_class child_class in
      match class_decl with
      | Some decl ->
        let method_info = Decl_provider.Class.get_method decl method_name in
        (match method_info with
        | Some class_elt ->
          add_if_valid_origin class_elt child_class method_name result
        | None ->
          let smethod_info =
            Decl_provider.Class.get_smethod decl method_name
          in
          (match smethod_info with
          | Some class_elt ->
            add_if_valid_origin class_elt child_class method_name result
          | None -> result))
      | None ->
        failwith ("Could not find definition of child class: " ^ child_class))

let find_child_classes
    (class_name : string) (genv : ServerEnv.genv) (env : ServerEnv.env) :
    string list =
  let files =
    FindRefsService.get_dependent_files
      genv.ServerEnv.workers
      (SSet.singleton class_name)
  in
  FindRefsService.find_child_classes class_name env.naming_table files
  |> SSet.elements

let search_class
    (class_name : string) (genv : ServerEnv.genv) (env : ServerEnv.env) :
    ServerEnv.env * server_result_or_retry =
  let class_name = ServerFindRefs.add_ns class_name in
  ServerFindRefs.handle_prechecked_files
    genv
    env
    Typing_deps.Dep.(make (Class class_name))
  @@ fun () ->
  let child_classes = find_child_classes class_name genv env in
  if List.length child_classes < 10 then
    find_positions_of_classes [] child_classes
  else
    parallel_find_positions_of_classes child_classes genv.workers

let search_member
    (class_name : string)
    (member : member)
    (genv : ServerEnv.genv)
    (env : ServerEnv.env) : ServerEnv.env * server_result_or_retry =
  match member with
  | Method method_name ->
    let class_name = ServerFindRefs.add_ns class_name in
    let class_name = FindRefsService.get_origin_class_name class_name member in
    ServerFindRefs.handle_prechecked_files
      genv
      env
      Typing_deps.Dep.(make (Class class_name))
    @@ fun () ->
    (* Find all the classes that extend this one *)
    let child_classes = find_child_classes class_name genv env in
    find_positions_of_methods [] child_classes method_name
  | Property _
  | Class_const _
  | Typeconst _ ->
    (env, Done [])

let go action genv env =
  match action with
  | Class class_name -> search_class class_name genv env
  | Member (class_name, member) -> search_member class_name member genv env
  | Function _
  | Record _
  | GConst _
  | LocalVar _ ->
    (env, Done [])
