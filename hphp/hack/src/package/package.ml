(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type pos_id = Pos.t * string [@@deriving eq, show]

type package = {
  name: pos_id;
  uses: pos_id list;
  includes: pos_id list;
}
[@@deriving eq, show]

external extract_packages_from_text : string -> string -> package list
  = "extract_packages_from_text_ffi"

let glob_to_package : (string, package) Hashtbl.t = Hashtbl.create 0

let initialize_packages_info (path : string) =
  let contents = Sys_utils.cat path in
  let packages = extract_packages_from_text path contents in
  List.iter
    (fun pkg ->
      List.iter (fun (_, glob) -> Hashtbl.add glob_to_package glob pkg) pkg.uses)
    packages

let get_package_for_module md =
  let matching_pkgs =
    Hashtbl.fold
      (fun glob pkg acc ->
        if Str.string_match (Str.regexp glob) md 0 then
          (glob, pkg) :: acc
        else
          acc)
      glob_to_package
      []
  in
  let sorted_pkgs =
    List.sort (fun (md1, _) (md2, _) -> String.compare md1 md2) matching_pkgs
    |> List.rev
  in
  match sorted_pkgs with
  | [] -> None
  | (_, pkg) :: _ -> Some pkg

let get_package_pos pkg = fst pkg.name

let get_package_name pkg = snd pkg.name

let includes pkg1 pkg2 =
  List.exists
    (fun (_, name) -> String.equal name @@ get_package_name pkg2)
    pkg1.includes
