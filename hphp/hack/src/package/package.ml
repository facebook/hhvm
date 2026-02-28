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

let includes pkg1 pkg2 =
  List.exists
    ~f:(fun (_, name) -> String.equal name @@ get_package_name pkg2)
    pkg1.includes

let soft_includes pkg1 pkg2 =
  List.exists
    ~f:(fun (_, name) -> String.equal name @@ get_package_name pkg2)
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
