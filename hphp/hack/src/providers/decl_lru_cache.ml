(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type fun_key = string

type typedef_key = string

type gconst_key = string

type fun_decl = Typing_defs.decl_fun_type

type typedef_decl = Typing_defs.typedef_type

type gconst_decl = Typing_defs.decl_ty * Errors.t

module Funs = SharedMem.LocalCache (StringKey) (Decl_heap.Fun)
module GConsts = SharedMem.LocalCache (StringKey) (Decl_heap.GConst)
module Typedefs = SharedMem.LocalCache (StringKey) (Decl_heap.Typedef)

let get_type_id_filename x expected_kind =
  match Naming_table.Types.get_pos x with
  | Some (pos, kind) when kind = expected_kind ->
    let res = FileInfo.get_pos_filename pos in
    Some res
  | _ -> None

let get_fun (fun_name : fun_key) : fun_decl option =
  match Funs.get fun_name with
  | Some s -> Some s
  | None ->
    let fun_name_key = Provider_config.Fun_decl fun_name in
    (match Lru_worker.get_with_offset fun_name_key with
    | Some s -> Some s
    | None ->
      (match Naming_table.Funs.get_pos fun_name with
      | Some pos ->
        let filename = FileInfo.get_pos_filename pos in
        let ft =
          Errors.run_in_decl_mode filename (fun () ->
              Decl.declare_fun_in_file filename fun_name)
        in
        (* Add to shared LRU cache *)
        let _success = Lru_worker.set fun_name_key ft in
        (* Add to worker local cache *)
        Funs.add fun_name ft;
        Some ft
      | None -> None))

let get_gconst gconst_name =
  let gconst_name_key = Provider_config.Gconst_decl gconst_name in
  match GConsts.get gconst_name with
  | Some s -> Some s
  | None ->
    begin
      match Lru_worker.get_with_offset gconst_name_key with
      | Some s -> Some s
      | None ->
        (match Naming_table.Consts.get_pos gconst_name with
        | Some pos ->
          let filename = FileInfo.get_pos_filename pos in
          let gconst =
            Errors.run_in_decl_mode filename (fun () ->
                Decl.declare_const_in_file filename gconst_name)
          in
          (* Add to shared LRU cache *)
          let _success = Lru_worker.set gconst_name_key gconst in
          (* Add to worker local cache *)
          GConsts.add gconst_name gconst;
          Some gconst
        | None -> None)
    end

let get_typedef (typedef_name : string) : typedef_decl option =
  match Typedefs.get typedef_name with
  | Some s -> Some s
  | None ->
    let typedef_name_key = Provider_config.Typedef_decl typedef_name in
    (match Lru_worker.get_with_offset typedef_name_key with
    | Some s -> Some s
    | None ->
      (match get_type_id_filename typedef_name Naming_table.TTypedef with
      | Some filename ->
        let tdecl =
          Errors.run_in_decl_mode filename (fun () ->
              Decl.declare_typedef_in_file filename typedef_name)
        in
        (* Add to shared LRU cache *)
        let _success = Lru_worker.set typedef_name_key tdecl in
        (* Add to worker local cache *)
        Typedefs.add typedef_name tdecl;
        Some tdecl
      | None -> None))
