(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type package_error_info = {
  current_package_pos: Pos.t;
  current_package_name: string option;
  current_package_assignment_kind: string;
  target_package_name: string option;
  target_package_pos: Pos.t;
  target_package_assignment_kind: string;
  target_id: string;
}

type check_reason =
  [ `Yes of string
  | `No
  ]

(** Calculate the packages of the current and target symbols using their filenames.
  * The target symbol can be accessed if:
  *    - current and target are in the same file
  *    - current and target are in the same package
  *    - target is a builtin symbol from hhi
  *    - current symbol is in a test file whose path contains __tests__
  *    - current symbol's package includes target symbol's package
  *)
val can_access_by_package_rules :
  env:Typing_env_types.env ->
  target_package_membership:Aast_defs.package_membership option ->
  target_pos:Pos_or_decl.t ->
  target_id:string ->
  [ `Yes
  | `PackageNotSatisfied of package_error_info
  | `PackageSoftIncludes of package_error_info
  ]

val get_package_violation :
  Typing_env_types.env ->
  Package.t option ->
  Package.t option ->
  Package.package_relationship option
