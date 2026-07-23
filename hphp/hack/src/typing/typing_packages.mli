(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type package_warning_info = {
  current_package: Package.pos_id option;
  target_package: Package.pos_id option;
  target_package_before_override: string option;
  classptr_reference_warning: bool;
  caller_has_package_override: bool;
}

type package_error_info = {
  current_package: Package.pos_id option;
  current_package_assignment_kind: string;
  target_package: Package.pos_id option;
  target_package_assignment_kind: string;
  target_id: string;
}

type check_reason =
  [ `Yes of Typing_error.Primary.Package.target_symbol_spec
  | `LintOnly
  | `ClassPtrLinterOnly
  | `No
  ]

val get_package_profile :
  Typing_env_types.env ->
  Aast_defs.package_membership option ->
  Package.t option * Package.pos_id option * string

val can_access_ignoring_package_override :
  env:Typing_env_types.env ->
  current_package:Package.pos_id option ->
  target_package:Package.pos_id option ->
  target_file:Relative_path.t ->
  classptr_reference_warning:bool ->
  [ `Yes | `YesWarning of package_warning_info ]

(** Calculate the packages of the current and target symbols using their filenames.
  * The target symbol can be accessed if:
  *    - current and target are in the same file
  *    - current and target are in the same package
  *    - target is a builtin symbol from hhi
  *    - current symbol's package includes target symbol's package
  *    - for a target package without strict isolation, either symbol is in a
  *      file matching package_exclude_patterns (e.g. __tests__)
  *
  * For a strict-isolation target package, package_exclude_patterns does NOT grant
  * an exemption (it only affects deployment): [`ExcludedPathAccess] is returned
  * when non-excluded code references excluded-path code inside the strict-isolation
  * package.
  *)
val can_access_by_package_rules :
  env:Typing_env_types.env ->
  target_package_membership:Aast_defs.package_membership option ->
  target_pos:Pos_or_decl.t ->
  target_id:string ->
  [ `Yes
  | `YesWarning of package_warning_info
  | `PackageNotSatisfied of package_error_info
  | `PackageSoftIncludes of package_error_info
  | `ExcludedPathAccess of package_error_info
  ]

val get_package_violation :
  Typing_env_types.env ->
  Package.t option ->
  Package.t option ->
  Package.package_relationship option

val is_excluded : Typing_env_types.env -> Relative_path.t -> bool
