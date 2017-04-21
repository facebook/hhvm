(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Typing_heap

let get_class tcopt x =
  match Classes.get x with
  | Some c ->
    Some c
  | None ->
    match Naming_heap.TypeIdHeap.get x with
    | Some (pos, `Class) ->
      let filename = FileInfo.get_pos_filename pos in
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
    match Naming_heap.TypeIdHeap.get x with
    | Some (pos, `Typedef) ->
        let filename = FileInfo.get_pos_filename pos in
        Errors.run_in_decl_mode filename
        (fun () -> Decl.declare_typedef_in_file tcopt filename x);
      Typedefs.get x
    | _ -> None
