(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type package_info = {
  pkg_name: Aast_defs.sid;
  pkg_includes: Aast_defs.sid list option;
  pkg_uses: Aast_defs.md_name_kind list;
}
[@@deriving show]

val get_package_name : package_info -> string

val get_package_for_module : string -> package_info option

val includes : package_info -> package_info -> bool

val initialize : package_info SMap.t -> unit
