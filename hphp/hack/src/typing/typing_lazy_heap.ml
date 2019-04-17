(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_heap

let check_cache_consistency x expected_kind expected_result =
  if
    expected_result = Relative_path.default &&
    (not @@ Naming_table.has_local_changes ())
  then begin
    Hh_logger.log "WARNING: found dummy path in shared heap for %s" x;
    match Naming_table.Types.get_pos ~bypass_cache:true x with
    | Some (pos, kind) when kind = expected_kind &&
      FileInfo.get_pos_filename pos = expected_result -> ()
    | _ ->
      Hh_logger.log "WARNING: get and get_no_cache returned different results";
  end

let get_type_id_filename x expected_kind =
  match Naming_table.Types.get_pos x with
  | Some (pos, kind) when kind = expected_kind ->
    let res = FileInfo.get_pos_filename pos in
    check_cache_consistency x expected_kind res;
    Some res
  | _ -> None

let get_class = Typing_classes_heap.Classes.get

let get_fun x =
  match Funs.get x with
  | Some c -> Some c
  | None ->
    match Naming_table.Funs.get_pos x with
    | Some pos ->
      let filename = FileInfo.get_pos_filename pos in
      Errors.run_in_decl_mode filename
        (fun () -> Decl.declare_fun_in_file filename x);
      Funs.get x
    | None -> None

let get_gconst cst_name =
  match GConsts.get cst_name with
  | Some c -> Some c
  | None ->
    match Naming_table.Consts.get_pos cst_name with
    | Some pos ->
        let filename = FileInfo.get_pos_filename pos in
        Errors.run_in_decl_mode filename
          (fun () -> Decl.declare_const_in_file filename cst_name);
      GConsts.get cst_name
    | None -> None

let get_typedef x =
  match Typedefs.get x with
  | Some c -> Some c
  | None ->
    match get_type_id_filename x Naming_table.TTypedef with
    | Some filename ->
        Errors.run_in_decl_mode filename
        (fun () -> Decl.declare_typedef_in_file filename x);
      Typedefs.get x
    | _ -> None
