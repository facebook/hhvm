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
  bool -> bool -> string -> string -> string -> (Package.t list, errors) result
  = "extract_packages_from_text_ffi"

let repo_config_path =
  Relative_path.from_root ~suffix:(Config_file_common.get_pkgconfig_path ())

let log_debug (msg : ('a, unit, string, string, string, unit) format6) : 'a =
  Hh_logger.debug ~category:"packages" ?exn:None msg

let parse (package_v2 : bool) (strict : bool) (path : string) =
  let contents = Sys_utils.cat path in
  let root = Relative_path.(path_of_prefix Root) in
  match extract_packages_from_text package_v2 strict root path contents with
  | Error [] -> failwith "Bad package specifiction"
  | Error errors ->
    let strings =
      List.map
        ~f:(fun (pos, msg, reasons) ->
          let reasons =
            List.map ~f:(fun (p, s) -> (Pos_or_decl.of_raw_pos p, s)) reasons
          in
          User_error.(
            to_string
              (to_absolute
                 Parsing_error.(
                   to_user_error @@ Package_config_error { pos; msg; reasons }))))
        errors
    in
    failwith (String.concat strings)
  | Ok packages -> PackageInfo.from_packages packages

let load_and_parse
    ~(strict : bool) ~(package_v2 : bool) ~(pkgs_config_abs_path : string) :
    PackageInfo.t =
  if not @@ Sys.file_exists pkgs_config_abs_path then (
    log_debug "Package config at %s does not exist" pkgs_config_abs_path;
    PackageInfo.empty
  ) else
    let result = parse package_v2 strict pkgs_config_abs_path in
    log_debug "Parsed %s" pkgs_config_abs_path;
    result
