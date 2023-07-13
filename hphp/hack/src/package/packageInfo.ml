(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type t = {
  glob_to_package: Package.t SMap.t;
  existing_packages: Package.t SMap.t;
}
[@@deriving eq, show]

let empty = { glob_to_package = SMap.empty; existing_packages = SMap.empty }

let get_package_for_module (info : t) (md : string) : Package.t option =
  let candidates =
    SMap.filter
      (fun glob _ -> Str.string_match (Str.regexp glob) md 0)
      info.glob_to_package
  in
  let (_strictest_matching_glob, package_with_strictest_matching_glob) =
    SMap.fold
      (fun glob pkg ((glob', _) as acc) ->
        if String.compare glob glob' > 0 then
          (glob, Some pkg)
        else
          acc)
      candidates
      ("", None)
  in
  package_with_strictest_matching_glob

let package_exists (info : t) (pkg : string) : bool =
  SMap.mem pkg info.existing_packages

let get_package (info : t) (pkg : string) : Package.t option =
  SMap.find_opt pkg info.existing_packages

let from_packages (packages : Package.t list) : t =
  List.fold packages ~init:empty ~f:(fun acc pkg ->
      let pkg_name = Package.get_package_name pkg in
      let existing_packages = SMap.add pkg_name pkg acc.existing_packages in
      let acc = { acc with existing_packages } in
      List.fold pkg.Package.uses ~init:acc ~f:(fun acc (_, glob) ->
          let glob_to_package = SMap.add glob pkg acc.glob_to_package in
          let acc = { acc with glob_to_package } in
          acc))
