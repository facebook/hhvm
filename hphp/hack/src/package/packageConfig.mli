(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Load and parse PACKAGES.toml if it exists at the root.
  * If `strict` is true, then an error is raised if an include_path
  * does not exist in the filesystem.
  *)
val load_and_parse :
  package_v2:bool ->
  ?strict:bool ->
  ?pkgs_config_abs_path:string option ->
  unit ->
  Errors.t * PackageInfo.t

val repo_config_path : Relative_path.t
