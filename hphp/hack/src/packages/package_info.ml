(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type t = {
  existing_packages: Package.t S_map.t;
  include_path_to_package_map: (string * Package.t) list;
}
[@@deriving eq, show]

let empty =
  { existing_packages = S_map.empty; include_path_to_package_map = [] }

let log_package_info (info : t) : unit =
  let package_info =
    S_map.fold
      (fun _ p acc -> Package.show_package p ^ acc)
      info.existing_packages
      ""
  in
  Hh_logger.log
    "*** Package info: %s\n%s"
    (if S_map.is_empty info.existing_packages then
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
    (* Without the family path the member would get the root-relative
     * include_path [D/], matching unrelated files anywhere in the repo. *)
    | [] ->
      failwith
        (Printf.sprintf
           "implicit package family %s carries no include_path"
           fname)
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

(* Splits a (possibly synthesized) name [F.D] into family [F] and member [D].
 * In a well-formed repo a member directory is a valid Hack identifier and so
 * contains no [.], and package_info.rs rejects family names containing [.], so
 * [F.D] contains exactly one [.].
 * Resolution does not re-verify that: a directory that is not a valid identifier
 * is reported by the lowerer's placement check, and this code is only meaningful
 * for a repo that type-checks clean. Returns None unless both sides are
 * non-empty. *)
let split_member_name (pkg : string) : (string * string) option =
  match String.lsplit2 pkg ~on:'.' with
  | Some (family, member)
    when (not (String.is_empty family)) && not (String.is_empty member) ->
    Some (family, member)
  | _ -> None

let get_package (info : t) (pkg : string) : Package.t option =
  match S_map.find_opt pkg info.existing_packages with
  | Some p -> Some p
  | None ->
    (* Not declared directly -- it may be a member [F.D] of a declared family. *)
    (match split_member_name pkg with
    | Some (family, member) ->
      (match S_map.find_opt family info.existing_packages with
      | Some f when f.Package.is_implicit -> Some (synthesize_member f member)
      | _ -> None)
    | None -> None)

let package_exists (info : t) (pkg : string) : bool =
  Option.is_some (get_package info pkg)

(** The get_package_for_file returns the package a file path belongs to;
  * it ignores PackageOverride annotations. 
  *)
let get_package_for_file (info : t) ~(path : string) : Package.t option =
  match
    List.find
      ~f:(fun (ip, _) -> String.is_prefix path ~prefix:ip)
      info.include_path_to_package_map
  with
  | None -> None
  | Some (_, p) when not p.Package.is_implicit -> Some p
  | Some (ip, p) ->
    (* The typechecker rejects files directly under a family path. Return None
     * defensively if one reaches package lookup without a member directory. *)
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
    (info : t) ~(path : string) ~(content : string) : Package.t option * bool =
  match extract_package_override content with
  | Some package_override -> (get_package info package_override, true)
  | None -> (get_package_for_file info ~path, false)
