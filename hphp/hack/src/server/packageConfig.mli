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

val load_and_parse : ServerEnv.env -> unit

val get_package_for_module : string -> package_info option
