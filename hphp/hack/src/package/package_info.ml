(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type t = {
  existing_packages: Package.t SMap.t;
  include_path_to_package_map: (string * Package.t) list;
}
[@@deriving eq, show]

let empty = { existing_packages = SMap.empty; include_path_to_package_map = [] }

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
  let existing_packages =
    List.fold packages ~init:SMap.empty ~f:(fun acc pkg ->
        let pkg_name = Package.get_package_name pkg in
        SMap.add pkg_name pkg acc)
  in

  let include_path_to_package_map : (string * Package.t) list =
    List.sort
      (List.fold
         ~init:[]
         ~f:(fun acc (p : Package.t) ->
           List.fold
             ~f:(fun (acc : (string * Package.t) list) ip -> (snd ip, p) :: acc)
             p.Package.include_paths
             ~init:acc)
         packages)
      ~compare:(fun (p1, _) (p2, _) -> String.compare p2 p1)
  in
  { existing_packages; include_path_to_package_map }

let get_package_for_file (info : t) (path : string) : Package.t option =
  Option.map
    ~f:snd
    (List.find
       ~f:(fun (ip, _) -> String.is_prefix path ~prefix:ip)
       info.include_path_to_package_map)
