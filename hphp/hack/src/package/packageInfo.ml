(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type t = { existing_packages: Package.t SMap.t } [@@deriving eq, show]

let empty = { existing_packages = SMap.empty }

let log_package_info (info : t) : unit =
  let package_info =
    SMap.fold
      (fun _ p acc -> Package.show_package p ^ acc)
      info.existing_packages
      ""
  in
  Hh_logger.log
    "*** Package info: %s\n%s"
    (if SMap.is_empty info.existing_packages then
      "empty"
    else
      "")
    package_info

let package_exists (info : t) (pkg : string) : bool =
  SMap.mem pkg info.existing_packages

let get_package (info : t) (pkg : string) : Package.t option =
  SMap.find_opt pkg info.existing_packages

let from_packages (packages : Package.t list) : t =
  List.fold packages ~init:empty ~f:(fun acc pkg ->
      let pkg_name = Package.get_package_name pkg in
      let existing_packages = SMap.add pkg_name pkg acc.existing_packages in
      { existing_packages })
