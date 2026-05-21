(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type pos_id = Pos.t * string [@@deriving eq, show]

type t = {
  name: pos_id;
  includes: pos_id list;
  soft_includes: pos_id list;
  include_paths: pos_id list;
}
[@@deriving eq, show]

(* Represents how two packages are related *)
type package_relationship =
  | Unrelated
  | Includes
  | Soft_includes
  | Equal

val show_package : t -> string

val get_package_name : t -> string

val get_package_pos : t -> Pos.t

val relationship : t -> t -> package_relationship
