(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shallow_decl_defs

module Classes =
  SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey)
    (struct
      type t = shallow_class

      let prefix = Prefix.make ()

      let description = "Decl_ShallowClass"
    end)
    (struct
      let capacity = 1000
    end)

let push_local_changes = Classes.LocalChanges.push_stack

let pop_local_changes = Classes.LocalChanges.pop_stack

let class_naming_and_decl ctx c =
  let c = Errors.ignore_ (fun () -> Naming.class_ ctx c) in
  Shallow_decl.class_ ctx c

let shallow_decl_enabled (ctx : Provider_context.t) : bool =
  TypecheckerOptions.shallow_class_decl (Provider_context.get_tcopt ctx)

let get_from_store ctx cid =
  if shallow_decl_enabled ctx then
    Classes.get cid
  else
    failwith "shallow_class_decl not enabled"

let add_to_store ctx cid c =
  if shallow_decl_enabled ctx then
    Classes.add cid c
  else
    failwith "shallow_class_decl not enabled"

let class_decl_if_missing (ctx : Provider_context.t) (c : Nast.class_) =
  let (_, cid) = c.Aast.c_name in
  match get_from_store ctx cid with
  | Some c -> c
  | None ->
    let c = class_naming_and_decl ctx c in
    add_to_store ctx cid c;
    c

let err_not_found file name =
  let err_str =
    Printf.sprintf "%s not found in %s" name (Relative_path.to_absolute file)
  in
  raise (Decl_defs.Decl_not_found err_str)

let declare_class_in_file ctx file name =
  match Ast_provider.find_class_in_file ctx file name with
  | Some cls -> class_decl_if_missing ctx cls
  | None -> err_not_found file name

let get ctx cid =
  match get_from_store ctx cid with
  | Some _ as c -> c
  | None ->
    (match Naming_provider.get_class_path ctx cid with
    | None -> None
    | Some filename -> Some (declare_class_in_file ctx filename cid))

let get_batch = Classes.get_batch

let get_old_batch = Classes.get_old_batch

let oldify_batch = Classes.oldify_batch

let remove_old_batch = Classes.remove_old_batch

let remove_batch = Classes.remove_batch
