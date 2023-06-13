(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type errors = (Pos.t * string * (Pos.t * string) list) list

external extract_packages_from_text :
  string -> string -> (Package.package list, errors) result
  = "extract_packages_from_text_ffi"

let pkgs_config_path_relative_to_repo_root = "PACKAGES.toml"

let repo_config_path =
  Relative_path.from_root ~suffix:pkgs_config_path_relative_to_repo_root

let log_debug (msg : ('a, unit, string, string, string, unit) format6) : 'a =
  Hh_logger.debug ~category:"packages" ?exn:None msg

let parse (path : string) =
  let contents = Sys_utils.cat path in
  match extract_packages_from_text path contents with
  | Error errors ->
    let error_list =
      List.map errors ~f:(fun (pos, msg, reasons) ->
          let reasons =
            List.map ~f:(fun (p, s) -> (Pos_or_decl.of_raw_pos p, s)) reasons
          in
          Parsing_error.(
            to_user_error @@ Package_config_error { pos; msg; reasons }))
    in
    (Errors.from_error_list error_list, Package.Info.empty)
  | Ok packages -> (Errors.empty, Package.Info.from_packages packages)

let load_and_parse ?(pkgs_config_abs_path = None) () : Errors.t * Package.Info.t
    =
  let pkgs_config_abs_path =
    match pkgs_config_abs_path with
    | None -> Relative_path.to_absolute repo_config_path
    | Some path -> path
  in
  if not @@ Sys.file_exists pkgs_config_abs_path then
    (Errors.empty, Package.Info.empty)
  else
    let result = parse pkgs_config_abs_path in
    log_debug "Parsed %s" pkgs_config_abs_path;
    result
