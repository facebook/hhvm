(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type package = {
  name: string;
  uses: string list;
  includes: string list;
}
[@@deriving eq, show]

val get_package_name : package -> string

val get_package_pos : package -> Pos.t

val get_package_for_module : string -> package option

val includes : package -> package -> bool

val initialize_packages_info : string -> unit
