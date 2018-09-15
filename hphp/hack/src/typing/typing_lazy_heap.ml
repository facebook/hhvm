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
    (not @@ Naming_heap.TypeIdHeap.LocalChanges.has_local_changes ())
  then begin
    Hh_logger.log "WARNING: found dummy path in shared heap for %s" x;
    match Naming_heap.TypeIdHeap.get_no_cache x with
    | Some (pos, kind) when kind = expected_kind &&
      FileInfo.get_pos_filename pos = expected_result -> ()
    | _ ->
      Hh_logger.log "WARNING: get and get_no_cache returned different results";
  end

let get_type_id_filename x expected_kind =
  match Naming_heap.TypeIdHeap.get x with
  | Some (pos, kind) when kind = expected_kind ->
    let res = FileInfo.get_pos_filename pos in
    check_cache_consistency x expected_kind res;
    Some res
  | _ -> None

let get_class tcopt x =
  match Classes.get x with
  | Some c ->
    Some c
  | None ->
    match get_type_id_filename x `Class with
    | Some filename ->
      Errors.run_in_decl_mode filename
        (fun () -> Decl.declare_class_in_file tcopt filename x);
      Classes.get x
    | _ -> None

let get_fun tcopt x =
  match Funs.get x with
  | Some c -> Some c
  | None ->
    match Naming_heap.FunPosHeap.get x with
    | Some pos ->
      let filename = FileInfo.get_pos_filename pos in
      Errors.run_in_decl_mode filename
        (fun () -> Decl.declare_fun_in_file tcopt filename x);
      Funs.get x
    | None -> None

let get_gconst tcopt cst_name =
  match GConsts.get cst_name with
  | Some c -> Some c
  | None ->
    match Naming_heap.ConstPosHeap.get cst_name with
    | Some pos ->
        let filename = FileInfo.get_pos_filename pos in
        Errors.run_in_decl_mode filename
          (fun () -> Decl.declare_const_in_file tcopt filename cst_name);
      GConsts.get cst_name
    | None -> None

let get_typedef tcopt x =
  match Typedefs.get x with
  | Some c -> Some c
  | None ->
    match get_type_id_filename x `Typedef with
    | Some filename ->
        Errors.run_in_decl_mode filename
        (fun () -> Decl.declare_typedef_in_file tcopt filename x);
      Typedefs.get x
    | _ -> None
