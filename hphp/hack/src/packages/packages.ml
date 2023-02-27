(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type package_info = {
  pkg_name: Aast_defs.sid;
  pkg_includes: Aast_defs.sid list option;
  pkg_uses: Aast_defs.md_name_kind list;
}
[@@deriving show]

(* TODO(milliechen): Consider switching to Hashtbl.t if we decide to continue
   with this data structure as opposed to, say, a trie. *)
let glob_to_package_ref : package_info SMap.t ref = ref SMap.empty

let initialize glob_to_package = glob_to_package_ref := glob_to_package

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
  | (_, pkg) :: _ -> Some pkg

let get_package_name (pkg : package_info) : string =
  Ast_defs.get_id pkg.pkg_name

let includes (pkg1 : package_info) (pkg2 : package_info) : bool =
  match pkg1.pkg_includes with
  | Some pkg1_includes ->
    List.mem pkg1_includes pkg2.pkg_name ~equal:(fun name1 name2 ->
        String.equal (Ast_defs.get_id name1) (Ast_defs.get_id name2))
  | None -> false
