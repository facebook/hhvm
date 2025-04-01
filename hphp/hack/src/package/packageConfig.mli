(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Load and parse [pkgs_config_abs_path] if it exists.
  * If "strict", then errors are raised if a file or directory specified in an
  * include_path does not exist in the filesystem.  `hh` runs with `strict=true`,
  * while `hh_single_type_check` with `strict=false` to avoid false positves in
  * the test suite.
  *)
val load_and_parse :
  package_v2:bool -> strict:bool -> pkgs_config_abs_path:string -> PackageInfo.t

val repo_config_path : Relative_path.t
