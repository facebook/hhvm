(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Nast
open Result
open Typing_infer_return

let add_ns name =
  if String_utils.string_starts_with name "\\" then name else "\\" ^ name

let infer_return_type tcopt def name =
  let types = get_matching_types tcopt def name in
  let tyopt = Typing_ops.LeastUpperBound.compute types in
  Option.map tyopt print_type

let handle_return tcopt file def name ret =
  match ret with
  | None ->
    let tyopt = infer_return_type tcopt def name in
    of_option tyopt ~error:"Could not infer type"
  | Some h ->
    let env, ty = type_from_hint tcopt file h in
    Ok (print_type_locl env ty)

let to_tuple r1 r2 =
  combine ~ok:(fun r1 r2 -> (r1, r2)) ~err:(fun _ e2 -> e2) r1 r2

let get_fun_return_ty tcopt fun_name =
  let fun_name = add_ns fun_name in
  let pos =
    of_option (Naming_table.Funs.get_pos fun_name) "Could not find function"
  in
  let file = map pos FileInfo.get_pos_filename in
  let funopt =
    map file (fun file -> Parser_heap.find_fun_in_file file fun_name)
  in
  let f = join @@ map funopt (of_option ~error:"Could not find function") in
  let f = map f Naming.fun_ in
  let args = to_tuple file f in
  bind args
    (fun (file, f) -> handle_return tcopt file (Fun f) f.f_name f.f_ret)

let get_meth_return_ty tcopt class_name meth_name =
  let class_name = add_ns class_name in
  let pos =
    match Naming_table.Types.get_pos class_name with
    | Some (pos, Naming_table.TClass) -> Ok pos
    | _ -> Error "Could not find class"
  in
  let file = map pos FileInfo.get_pos_filename in
  let classopt =
     map file (fun file -> Parser_heap.find_class_in_file file class_name)
  in
  let c = join @@ map classopt (of_option ~error:"Could not find class") in
  let c = map c Naming.class_ in
  let meth_list = map c (fun c -> c.c_static_methods @ c.c_methods) in
  let mopt = map meth_list (List.find ~f:(fun m -> snd m.m_name = meth_name)) in
  let m = join @@ map mopt (of_option ~error:"Could not find method") in
  let args = to_tuple file (to_tuple c m) in
  bind args
    (fun (file, (c, m)) ->
      handle_return tcopt file (Class c) m.m_name m.m_ret)
