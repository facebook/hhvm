(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
open Core_kernel

exception FunctionNotFound

let get_filename func =
  let fname_pos = match Decl_provider.get_fun func with
    | Some f -> f.ft_pos
    | None -> raise FunctionNotFound in
  Pos.filename fname_pos

let extract_function_body func =
  let filename = get_filename func in
  let abs_filename = Relative_path.to_absolute filename in
  let file_content = In_channel.read_all abs_filename in
  let open Ast in
  match Ast_provider.find_fun_in_file filename func with
    | Some ast_function ->
      let pos = ast_function.f_span in
      let include_first_whsp = Pos.merge (Pos.first_char_of_line pos) pos in
      Pos.get_text_from_pos file_content include_first_whsp
    | None -> raise FunctionNotFound

(* TODO: extract declaration *)
let extract_object_declaration obj =
  Typing_deps.Dep.to_string obj

let collect_dependencies tcopt func =
  let dependencies = HashSet.create 0 in
  let add_dependency root obj =
    let name_from_variant v =
      match String.split_on_chars ~on:[' '] (Typing_deps.Dep.to_string v) with
      | _::object_name::_ -> object_name
      | _ -> "" in
    let root_name = name_from_variant root in
    if root_name = (Utils.strip_ns func) then
    HashSet.add dependencies obj in
  Typing_deps.add_dependency_callback "add_dependency" add_dependency;
  let filename = get_filename func in
  let _ : Tast.def option = Typing_check_service.type_fun tcopt filename func in
  HashSet.fold (fun el l -> (extract_object_declaration el) :: l) dependencies []


let go tcopt function_name =
  try
    let function_text = extract_function_body function_name in
    let dependencies = collect_dependencies tcopt function_name in
    dependencies @ [function_text]
  with FunctionNotFound -> ["Function not found!"]
