(*
 * Copyright (c) Meta, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let regex_package_override =
  Str.regexp "__PackageOverride([\"']\\([^\"']+\\)[\"'])"

let extract_package_override text =
  try
    let _ = Str.search_forward regex_package_override text 0 in
    Some (Str.matched_group 1 text)
  with
  | _ -> None

let get_package ctx path text =
  match extract_package_override text with
  | Some package_override -> (package_override, true)
  | None ->
    let package_name =
      match
        PackageInfo.get_package_for_file
          (Provider_context.get_package_info ctx)
          path
      with
      | Some package -> snd package.Package.name
      | None ->
        (* If Hack enforces the definition of a default package
         * with include_path=["//"], then this case should never happen *)
        ""
    in
    (package_name, false)
