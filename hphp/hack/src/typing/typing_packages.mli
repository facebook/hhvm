(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type packageV1_error_info = {
  current_module_pos: Pos_or_decl.t;
  current_package_pos: Pos.t;
  current_package_name: string option;
  target_package_name: string option;
}

type packageV2_error_info = {
  current_package_pos: Pos.t;
  current_package_name: string option;
  target_package_name: string option;
}

(** Calculate the packages of the current and target symbols using their filenames.
  * The target symbol can be accessed if:
  *    - current and target are in the same file
  *    - current and target are in the same package
  *    - target is a builtin symbol from hhi
  *    - current symbol is in a test file whose path contains __tests__
  *    - current symbol's package includes target symbol's package
  *)
val can_access_by_package_v2_rules :
  env:Typing_env_types.env ->
  target_package:string option ->
  target_pos:Pos_or_decl.t ->
  [ `Yes
  | `PackageNotSatisfied of packageV2_error_info
  | `PackageSoftIncludes of packageV2_error_info
  ]

(** Calculate the packages of the current and target symbols using their modules.
  * The target symbol can be accessed if:
  *    - current and target are in the same module
  *    - current and target are in the same package
  *    - current symbol's package includes target symbol's package
  *)
val can_access_by_package_v1_rules :
  env:Typing_env_types.env ->
  current_module:string option ->
  target_module:string option ->
  [ `Yes
  | `PackageNotSatisfied of packageV1_error_info
  | `PackageSoftIncludes of packageV1_error_info
  ]

val get_package_violation :
  Typing_env_types.env ->
  Package.t option ->
  Package.t option ->
  Package.package_relationship option
