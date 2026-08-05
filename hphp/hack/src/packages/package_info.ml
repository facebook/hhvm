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

(* Synthesize the member package [F.D] of an implicit family. [family] is the
 * flagged family entry (its single include_path is the family path); [member_dir]
 * is the first path segment [D] below that path. Pure function of the family
 * declaration and [D] -- it touches no filesystem. *)
let synthesize_member (family : Package.t) (member_dir : string) : Package.t =
  let (fpos, fname) = family.Package.name in
  let member_name = fname ^ "." ^ member_dir in
  let member_path =
    match family.Package.include_paths with
    | (pos, path) :: _ -> (pos, path ^ member_dir ^ "/")
    | [] -> (fpos, member_dir ^ "/")
  in
  {
    Package.name = (fpos, member_name);
    Package.includes = family.Package.includes;
    Package.soft_includes = family.Package.soft_includes;
    Package.include_paths = [member_path];
    (* Members inherit the family's strict-isolation setting. *)
    Package.enable_strict_isolation = family.Package.enable_strict_isolation;
    Package.is_implicit = true;
  }

(* Splits a (possibly synthesized) name [F.D] into family [F] and member [D],
 * splitting on the *first* [.]. The member [D] is a single directory name that
 * may itself contain [.] (e.g. [proto.v1]), so we keep the remainder after the
 * first [.] as the member. Family names are forbidden from containing [.] (see
 * the family-name validation in config.rs), so this split is unambiguous.
 * Returns None unless both the family and the member are non-empty. *)
let split_member_name (pkg : string) : (string * string) option =
  match String.lsplit2 pkg ~on:'.' with
  | Some (family, member)
    when (not (String.is_empty family)) && not (String.is_empty member) ->
    Some (family, member)
  | _ -> None

let get_package (info : t) (pkg : string) : Package.t option =
  match SMap.find_opt pkg info.existing_packages with
  | Some p -> Some p
  | None ->
    (* Not declared directly -- it may be a member [F.D] of a declared family. *)
    (match split_member_name pkg with
    | Some (family, member) ->
      (match SMap.find_opt family info.existing_packages with
      | Some f when f.Package.is_implicit -> Some (synthesize_member f member)
      | _ -> None)
    | None -> None)

let package_exists (info : t) (pkg : string) : bool =
  Option.is_some (get_package info pkg)

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

(** The get_package_for_file returns the package a file path belongs to;
  * it ignores PackageOverride annotations. 
  *)
let get_package_for_file ~support_multifile_tests (info : t) ~(path : string) :
    Package.t option =
  let path =
    if support_multifile_tests then
      let re = Str.regexp "[^/]*--" in
      Str.replace_first re "" path
    else
      path
  in
  match
    List.find
      ~f:(fun (ip, _) -> String.is_prefix path ~prefix:ip)
      info.include_path_to_package_map
  with
  | None -> None
  | Some (_, p) when not p.Package.is_implicit -> Some p
  | Some (ip, p) ->
    (* Implicit family match: the member directory [D] is the first path segment
     * after the family [path]. Only direct child *directories* denote members,
     * so a file lying directly in the family path (no [/] after the prefix)
     * belongs to no package. *)
    let remainder = String.drop_prefix path (String.length ip) in
    (match String.lsplit2 remainder ~on:'/' with
    | Some (dir, _) when not (String.is_empty dir) ->
      Some (synthesize_member p dir)
    | _ -> None)

(** The get_package_with_override function returns the package a file belongs
  * taking into account __PackageOverride annotations.  This function scans the
  * content of the file and is __very inefficient__.  It should be used ONLY
  * from services that cannot access decls, notably the Glean indexer and the
  * redundant PackageOverride linter.
  *)
let regex_package_override =
  Str.regexp "__PackageOverride([\"']\\([^\"']+\\)[\"'])"

let extract_package_override text =
  try
    let _ = Str.search_forward regex_package_override text 0 in
    Some (Str.matched_group 1 text)
  with
  | _ -> None

let get_package_with_override_for_file_no_env
    ~support_multifile_tests (info : t) ~(path : string) ~(content : string) :
    Package.t option * bool =
  match extract_package_override content with
  | Some package_override -> (get_package info package_override, true)
  | None -> (get_package_for_file ~support_multifile_tests info ~path, false)
