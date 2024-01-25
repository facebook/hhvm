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
  uses: pos_id list;
  includes: pos_id list;
  soft_includes: pos_id list;
  allow_directories: pos_id list;
}
[@@deriving eq, show]

type package_relationship =
  | Unrelated
  | Includes
  | Soft_includes
  | Equal

let get_package_pos pkg = fst pkg.name

let get_package_name pkg = snd pkg.name

let get_allow_directories_span pkg =
  match pkg.allow_directories with
  | [] -> Pos.none
  | hd :: tl ->
    let last =
      match List.rev tl with
      | [] -> hd
      | l :: _ -> l
    in
    Pos.btw (fst hd) (fst last)

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

let module_in_allowed_dirs (pkg : t) (file : Path.t) =
  let dir = Path.(to_string @@ dirname file) in
  List.is_empty pkg.allow_directories
  || List.exists pkg.allow_directories ~f:(fun (_, allowed) ->
         match Sys_utils.realpath allowed with
         | None ->
           String.is_prefix dir ~prefix:(String_utils.rstrip allowed "/*")
         | Some allowed -> String.equal dir allowed)
