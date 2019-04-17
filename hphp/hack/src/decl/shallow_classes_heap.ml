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

module Classes = SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey) (struct
  type t = shallow_class
  let prefix = Prefix.make ()
  let description = "ShallowClass"
end)

let push_local_changes = Classes.LocalChanges.push_stack
let pop_local_changes = Classes.LocalChanges.pop_stack

let class_naming_and_decl c =
  let c = Errors.ignore_ (fun () -> Naming.class_ c) in
  Shallow_decl.class_ c

let shallow_decl_enabled () =
  TypecheckerOptions.shallow_class_decl (GlobalNamingOptions.get ())

let get_from_store cid =
  if shallow_decl_enabled ()
  then Classes.get cid
  else failwith "shallow_class_decl not enabled"

let add_to_store cid c =
  if shallow_decl_enabled ()
  then Classes.add cid c
  else failwith "shallow_class_decl not enabled"

let class_decl_if_missing c =
  let _, cid = c.Ast.c_name in
  match get_from_store cid with
  | Some c -> c
  | None ->
    let c = class_naming_and_decl c in
    add_to_store cid c;
    c

let err_not_found file name =
  let err_str =
    Printf.sprintf "%s not found in %s" name (Relative_path.to_absolute file) in
  raise (Decl_defs.Decl_not_found err_str)

let declare_class_in_file file name =
  match Parser_heap.find_class_in_file file name with
  | Some cls -> class_decl_if_missing cls
  | None -> err_not_found file name

let get_class_filename x =
  match Naming_table.Types.get_pos x with
  | Some (pos, Naming_table.TClass) -> Some (FileInfo.get_pos_filename pos)
  | _ -> None

let get cid =
  match get_from_store cid with
  | Some _ as c -> c
  | None ->
    match get_class_filename cid with
    | None -> None
    | Some filename -> Some (declare_class_in_file filename cid)

let oldify_batch = Classes.oldify_batch
let remove_old_batch = Classes.remove_old_batch
