(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

external get_build_revision : unit -> string = "hh_get_build_revision"

external get_build_commit_time : unit -> int = "hh_get_build_commit_time"

external get_build_commit_time_string : unit -> string
  = "hh_get_build_commit_time_string"

external get_build_mode : unit -> string = "hh_get_build_mode"

external get_build_package_name : unit -> string = "hh_get_build_package_name"

let build_revision = get_build_revision ()

let build_commit_time = get_build_commit_time ()

let build_commit_time_string = get_build_commit_time_string ()

let build_mode = get_build_mode ()

let build_package_name = get_build_package_name ()

let is_build_optimized =
  String.is_prefix build_mode ~prefix:"dbgo"
  || String.is_prefix build_mode ~prefix:"opt"
  || String.equal build_mode ""

let is_dev_build =
  (* The package name is empty when not built by fbpkg *)
  String.equal build_package_name ""
