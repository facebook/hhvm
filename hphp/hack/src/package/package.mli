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
  soft_includes: pos_id list;
}
[@@deriving eq, show]

(* Represents how two packages are related *)
type package_relationship =
  | Unrelated
  | Includes
  | Soft_includes
  | Equal

val get_package_name : package -> string

val get_package_pos : package -> Pos.t

val relationship : package -> package -> package_relationship

module Info : sig
  type t

  val empty : t

  val initialize : string -> Errors.t * t

  val get_package_for_module : t -> string -> package option

  val get_package : t -> string -> package option

  val package_exists : t -> string -> bool
end
