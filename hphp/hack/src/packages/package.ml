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

open Hh_prelude

type pos_id = Pos.t * string [@@deriving eq, show]

type t = {
  name: pos_id;
  includes: pos_id list;
  soft_includes: pos_id list;
  include_paths: pos_id list;
  enable_strict_isolation: bool;
      (** When true, this package opts into strict isolation: its presence
          cannot be dynamically observed (the [package] expression and the
          [__RequirePackage] / [__SoftRequirePackage] attributes are rejected
          for it), and [package_exclude_patterns] (e.g. [__tests__]) do not
          grant a typecheck exemption for references into it. *)
  allow_deployed_packages_checking: bool;
      (** Opt-in: when true, code may check at runtime whether this package is
          deployed (the [package] expression and the [__RequirePackage] /
          [__SoftRequirePackage] attributes). A package that does not declare it
          may not be checked for, so a new package is closed by default. It does
          not restrict [__PackageOverride], which is the only way into a package
          declaring no [include_paths]. Independent of [enable_strict_isolation],
          which forbids the same checks: declaring both is contradictory and is
          rejected when the config is parsed. *)
  is_implicit: bool;
      (** True for an implicit-package family entry (from an
          [implicit_packages] stanza) and for the members synthesized from it.
          A family entry's [include_paths] holds the family [path]; member
          packages are synthesized lazily during lookup. *)
}
[@@deriving eq, show]

type package_relationship =
  | Unrelated
  | Includes
  | Soft_includes
  | Equal

let show_package p =
  Printf.sprintf
    "Package: %s\n  | includes: %s\n  | soft_includes: %s\n  | include_paths: %s\n"
    (snd p.name)
    (String.concat ~sep:", " (List.map ~f:snd p.includes))
    (String.concat ~sep:", " (List.map ~f:snd p.soft_includes))
    (String.concat
       ~sep:", "
       (List.map ~f:(fun (_, p) -> "//" ^ p) p.include_paths))

let get_package_pos pkg = fst pkg.name

let get_package_name pkg = snd pkg.name

let matches_package_reference (_, referenced_name) target =
  let target_name = get_package_name target in
  String.equal referenced_name target_name
  || target.is_implicit
     &&
     match String.lsplit2 target_name ~on:'.' with
     | Some (family_name, member_name) ->
       (not (String.is_empty family_name))
       && (not (String.is_empty member_name))
       && String.equal referenced_name family_name
     | None -> false

let includes pkg1 pkg2 =
  List.exists
    ~f:(fun package -> matches_package_reference package pkg2)
    pkg1.includes

let soft_includes pkg1 pkg2 =
  List.exists
    ~f:(fun package -> matches_package_reference package pkg2)
    pkg1.soft_includes

let relationship pkg1 pkg2 =
  if equal pkg1 pkg2 then
    Equal
  else if includes pkg1 pkg2 then
    Includes
  else if soft_includes pkg1 pkg2 then
    Soft_includes
  else
    Unrelated
