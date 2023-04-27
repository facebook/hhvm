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

type errors = (Pos.t * string * (Pos.t * string) list) list

type package = {
  name: pos_id;
  uses: pos_id list;
  includes: pos_id list;
  soft_includes: pos_id list;
}
[@@deriving eq, show]

type package_relationship =
  | Unrelated
  | Includes
  | Soft_includes
  | Equal

external extract_packages_from_text :
  string -> string -> (package list, errors) result
  = "extract_packages_from_text_ffi"

module Info = struct
  type t = {
    glob_to_package: (string, package) Hashtbl.t;
    existing_packages: (string, package) Hashtbl.t;
  }

  let empty =
    { glob_to_package = Hashtbl.create 0; existing_packages = Hashtbl.create 0 }

  let get_package_for_module (info : t) (md : string) : package option =
    let matching_pkgs =
      Hashtbl.fold
        (fun glob pkg acc ->
          if Str.string_match (Str.regexp glob) md 0 then
            (glob, pkg) :: acc
          else
            acc)
        info.glob_to_package
        []
    in
    let sorted_pkgs =
      List.sort (fun (md1, _) (md2, _) -> String.compare md1 md2) matching_pkgs
      |> List.rev
    in
    match sorted_pkgs with
    | [] -> None
    | (_, pkg) :: _ -> Some pkg

  let package_exists (info : t) (pkg : string) : bool =
    Hashtbl.mem info.existing_packages pkg

  let get_package (info : t) (pkg : string) : package option =
    Hashtbl.find_opt info.existing_packages pkg

  let initialize (path : string) =
    let contents = Sys_utils.cat path in
    let info = empty in
    let errors =
      match extract_packages_from_text path contents with
      | Error errors ->
        List.map
          (fun (pos, msg, reasons) ->
            let reasons =
              List.map (fun (p, s) -> (Pos_or_decl.of_raw_pos p, s)) reasons
            in
            Parsing_error.(
              to_user_error @@ Package_config_error { pos; msg; reasons }))
          errors
        |> Errors.from_error_list
      | Ok packages ->
        List.iter
          (fun pkg ->
            List.iter
              (fun (_, glob) ->
                Hashtbl.add info.glob_to_package glob pkg;
                Hashtbl.add info.existing_packages (snd pkg.name) pkg)
              pkg.uses)
          packages;
        Errors.empty
    in
    (errors, info)
end

let get_package_pos pkg = fst pkg.name

let get_package_name pkg = snd pkg.name

let includes pkg1 pkg2 =
  List.exists
    (fun (_, name) -> String.equal name @@ get_package_name pkg2)
    pkg1.includes

let soft_includes pkg1 pkg2 =
  List.exists
    (fun (_, name) -> String.equal name @@ get_package_name pkg2)
    pkg1.soft_includes

let relationship pkg1 pkg2 =
  if equal_package pkg1 pkg2 then
    Equal
  else if includes pkg1 pkg2 then
    Includes
  else if soft_includes pkg1 pkg2 then
    Soft_includes
  else
    Unrelated
