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

let module_name_matches_pattern module_name pattern =
  let pattern_len = String.length pattern in
  if pattern_len = 0 then
    false
  else if String.equal pattern "*" then
    true
  else if
    (* If `pattern` is a prefix glob, check that its prefix
       matches `module_name` or `module_name`'s prefix. *)
    String.is_suffix ~suffix:".*" pattern
  then
    let prefix = String.sub pattern ~pos:0 ~len:(pattern_len - 2) in
    if
      String.equal prefix module_name
      || String.is_prefix ~prefix:(prefix ^ ".") module_name
    then
      true
    else
      false
  else
    (* If `pattern` is a direct module name, check for an exact match. *)
    String.equal module_name pattern

let get_package_for_module (info : t) (md : string) : Package.t option =
  let candidates =
    SMap.filter
      (fun glob _ -> module_name_matches_pattern md glob)
      info.glob_to_package
  in
  let (_strictest_matching_glob, package_with_strictest_matching_glob) =
    SMap.fold
      (fun glob pkg ((glob', _) as acc) ->
        (* By construction all candidate globs are at most as long as `md`.
           If there is a candidate that's an exact match as `md`, it'll win
           as the strictest matching glob; otherwise, we want to find the
           longest glob that's a prefix glob of `md`. *)
        if (not @@ String.equal glob' md) && String.compare glob glob' > 0 then
          (glob, Some pkg)
        else
          acc)
      candidates
      ("", None)
  in
  package_with_strictest_matching_glob

let get_package_for_file
    ?(support_multifile_tests = false) (info : t) (file : Relative_path.t) :
    Package.t option =
  let filepath =
    if support_multifile_tests then
      (* When checking unit tests, attempt to strip out the `filename.php--/`
       * file name prefix added by the multifile support
       *)
      Multifile.short_suffix file
    else
      Relative_path.suffix file
  in
  let candidates : string SMap.t =
    SMap.filter_map
      (fun _ pkg ->
        List.filter_map pkg.Package.include_paths ~f:(fun (_, path) ->
            if String.is_prefix ~prefix:path filepath then
              Some path
            else
              None)
        |> List.sort ~compare:(fun s1 s2 -> ~-(String.compare s1 s2))
        |> List.hd)
      info.existing_packages
  in
  let (_strictest_matching_path, package_with_strictest_matching_path) =
    SMap.fold
      (fun pkg path ((path', _) as acc) ->
        (* By construction all candidate paths are at most as long as `filepath`.
           If there is a candidate that's an exact match as `filepath`, it'll win
           as the strictest matching path; otherwise, we want to find the longest path
           that's a prefix glob of `filepath`. *)
        if
          (not @@ String.equal path' filepath) && String.compare path path' >= 0
        then
          (path, Some pkg)
        else
          acc)
      candidates
      ("", None)
  in
  Option.bind package_with_strictest_matching_path ~f:(fun pkg_name ->
      SMap.find_opt pkg_name info.existing_packages)

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
