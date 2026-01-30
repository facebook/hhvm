(*
 * Copyright (c) 2025, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Packages locally asserted by RequirePackage and if package *)
type local_package_requirement =
  | Exists_in_deployment
  | Not_exists_in_deployment
  | Unsatisfiable_package_constraints

(** Information about a loaded package in the current environment *)
type loaded_package_info = {
  pos: Pos.t;
  status: local_package_requirement;
  from_includes: bool;
}

(** Each package maps to its loaded package info *)
type t = loaded_package_info SMap.t

val show : t -> string

val as_log_value : t -> Typing_log_value.value

val empty : t

val join : t -> t -> t

val add :
  package_info:PackageInfo.t ->
  Pos.t ->
  SMap.key ->
  local_package_requirement ->
  loaded_package_info SMap.t ->
  loaded_package_info SMap.t

val sub : t -> t -> bool
