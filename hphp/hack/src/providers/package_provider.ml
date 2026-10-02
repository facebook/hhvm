(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

(* Reduces a multifile-test path [<container>--<simulated/path.php>] to the path
   it simulates. *)
let normalize_path (ctx : Provider_context.t) (path : string) : string =
  let popt = Provider_context.get_popt ctx in
  if popt.Parser_options.package_support_multifile_tests then
    Multifile.strip_multifile_prefix path
  else
    path

let get_package_for_file (ctx : Provider_context.t) ~(path : string) :
    Package.t option =
  Package_info.get_package_for_file
    (Provider_context.get_package_info ctx)
    ~path:(normalize_path ctx path)

(* Every override spells out the attribute, so a file without it needs no
   parse. The decl parser stamps the override on each definition. *)
let get_package_override
    (ctx : Provider_context.t) ~(path : string) ~(content : string) :
    string option =
  if
    not
      (String.is_substring
         content
         ~substring:Naming_special_names.UserAttributes.uaPackageOverride)
  then
    None
  else
    (* Callers index arbitrary content, so a parse failure resolves by path. *)
    try
      let { Direct_decl_parser.pf_decls; _ } =
        Direct_decl_parser.parse_decls
          (Decl_parser_options.from_parser_options
             (Provider_context.get_popt ctx))
          (Relative_path.from_root ~suffix:path)
          content
      in
      List.find_map pf_decls ~f:(fun (_, decl) ->
          let package =
            match decl with
            | Shallow_decl_defs.Class
                { Shallow_decl_defs.sc_package = package; _ }
            | Shallow_decl_defs.Fun { Typing_defs.fe_package = package; _ }
            | Shallow_decl_defs.Typedef { Typing_defs.td_package = package; _ }
            | Shallow_decl_defs.Const { Typing_defs.cd_package = package; _ } ->
              package
            | Shallow_decl_defs.Module _ -> None
          in
          match package with
          | Some (Aast_defs.PackageOverride (_, package)) -> Some package
          | Some (Aast_defs.PackageConfigAssignment _)
          | None ->
            None)
    with
    | _ -> None

let get_package_with_override_for_file_no_env
    (ctx : Provider_context.t) ~(path : string) ~(content : string) :
    Package.t option * bool =
  let path = normalize_path ctx path in
  let info = Provider_context.get_package_info ctx in
  match get_package_override ctx ~path ~content with
  | Some package -> (Package_info.get_package info package, true)
  | None -> (Package_info.get_package_for_file info ~path, false)
