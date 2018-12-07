(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Shallow_decl_defs

module Classes = SharedMem.LocalCache (StringKey) (struct
  type t = shallow_class
  let prefix = Prefix.make ()
  let description = "ShallowClass"
  let use_sqlite_fallback () = false
end)

let class_naming_and_decl tcopt c =
  let c = Errors.ignore_ (fun () -> Naming.class_ tcopt c) in
  Shallow_decl.class_ tcopt c

let class_decl_if_missing tcopt c =
  let _, cid = c.Ast.c_name in
  match Classes.get cid with
  | Some c -> c
  | None ->
    let c = class_naming_and_decl tcopt c in
    Classes.add cid c;
    c

let err_not_found file name =
  let err_str =
    Printf.sprintf "%s not found in %s" name (Relative_path.to_absolute file) in
  raise (Decl_defs.Decl_not_found err_str)

let declare_class_in_file tcopt file name =
  match Parser_heap.find_class_in_file tcopt file name with
  | Some cls -> class_decl_if_missing tcopt cls
  | None -> err_not_found file name

let get_class_filename x =
  match Naming_heap.TypeIdHeap.get x with
  | Some (pos, `Class) -> Some (FileInfo.get_pos_filename pos)
  | _ -> None

let get tcopt cid =
  match Classes.get cid with
  | Some _ as c -> c
  | None ->
    match get_class_filename cid with
    | None -> None
    | Some filename -> Some (declare_class_in_file tcopt filename cid)
