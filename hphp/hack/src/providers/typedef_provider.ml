(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type type_key = string

type typedef_decl = Typing_defs.typedef_type

let err_not_found (file : Relative_path.t option) (name : string) : 'a =
  let err_str =
    match file with
    | Some file ->
      Printf.sprintf "%s not found in %s" name (Relative_path.to_absolute file)
    | None -> Printf.sprintf "%s not found" name
  in
  raise (Decl_defs.Decl_not_found err_str)

let find_in_direct_decl_parse ~cache_results ctx filename name extract_decl_opt
    =
  let parse_result =
    if cache_results then
      Direct_decl_utils.direct_decl_parse_and_cache ctx filename
    else
      Direct_decl_utils.direct_decl_parse ctx filename
  in
  match parse_result with
  | None -> err_not_found (Some filename) name
  | Some parsed_file ->
    let decls = parsed_file.Direct_decl_utils.pfh_decls in
    List.find_map decls ~f:(function
        | (decl_name, decl, _) when String.equal decl_name name ->
          extract_decl_opt decl
        | _ -> None)

let get_typedef (ctx : Provider_context.t) (typedef_name : type_key) :
    typedef_decl option =
  match Decl_store.((get ()).get_typedef typedef_name) with
  | Some c -> Some c
  | None ->
    (match Naming_provider.get_typedef_path ctx typedef_name with
    | Some filename ->
      find_in_direct_decl_parse
        ~cache_results:true
        ctx
        filename
        typedef_name
        Shallow_decl_defs.to_typedef_decl_opt
    | None -> None)
