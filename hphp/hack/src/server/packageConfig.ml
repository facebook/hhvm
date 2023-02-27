(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let pkgs_config_path_relative_to_repo_root = "__PACKAGES__.php"

type package_info = {
  pkg_name: Aast_defs.sid;
  pkg_includes: Aast_defs.sid list option;
  pkg_uses: Aast_defs.md_name_kind list;
}
[@@deriving show]

(* TODO(milliechen): Consider switching to Hashtbl.t if we decide to continue
   with this data structure as opposed to, say, a trie. *)
let glob_to_package_ref : package_info SMap.t ref = ref SMap.empty

let log_debug (msg : ('a, unit, string, string, string, unit) format6) : 'a =
  Hh_logger.debug ~category:"packages" ?exn:None msg

let get_package_for_module md_name =
  let matching_pkgs =
    SMap.filter
      (fun md_prefix _ -> Str.string_match (Str.regexp md_prefix) md_name 0)
      !glob_to_package_ref
  in
  let sorted_pkgs =
    List.sort (SMap.elements matching_pkgs) ~compare:(fun (md1, _) (md2, _) ->
        String.compare md1 md2)
    |> List.rev
  in
  match sorted_pkgs with
  | [] -> None
  | (_, pkg) :: _ -> Some (Ast_defs.get_id pkg.pkg_name)

let load_and_parse (env : ServerEnv.env) : string -> string option =
  let ctx = Provider_utils.ctx_from_server_env env in
  let repo_pkgs_config_path =
    Relative_path.from_root ~suffix:pkgs_config_path_relative_to_repo_root
  in
  let pkgs_config_abs_path = Relative_path.to_absolute repo_pkgs_config_path in
  if Sys.file_exists pkgs_config_abs_path then (
    let (errors, ast) =
      Ast_provider.get_ast_with_error ctx repo_pkgs_config_path
    in
    (* TODO(milliechen): make hh report errors *)
    if not (Errors.is_empty errors) then
      log_debug
        "Encountered errors while parsing %s:\n%s"
        pkgs_config_abs_path
        (Errors.show errors);

    log_debug "Parsed %s" pkgs_config_abs_path;
    let glob_to_package =
      List.fold ast ~init:SMap.empty ~f:(fun acc def ->
          let open Aast in
          match def with
          | Package { pkg_uses = Some mds; pkg_name; pkg_includes; _ } ->
            let pkg = { pkg_name; pkg_includes; pkg_uses = mds } in
            List.fold mds ~init:acc ~f:(fun acc md ->
                SMap.add (module_name_kind_to_string md) pkg acc)
          | _ -> acc)
    in
    glob_to_package_ref := glob_to_package;
    SMap.iter
      (fun k v -> log_debug "Module %s belongs to %s" k (show_package_info v))
      !glob_to_package_ref
  );
  get_package_for_module
