(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Load and parse [pkgs_config_abs_path] if it exists.
  * If `strict` is true, then an error is raised if an include_path
  * does not exist in the filesystem.
  *)
val load_and_parse :
  strict:bool -> package_v2:bool -> pkgs_config_abs_path:string -> PackageInfo.t

val repo_config_path : Relative_path.t
