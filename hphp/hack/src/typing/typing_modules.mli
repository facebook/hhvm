(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** [can_access_internal ~env ~current ~target] returns whether a symbol defined in
  * module [current] is allowed to access an internal symbol defined in
  * [target] under [env].
  *)
val can_access_internal :
  env:Typing_env_types.env ->
  current:string option ->
  target:string option ->
  [ `Yes
  | `Disjoint of string * string
  | `Outside of string
  | `OutsideViaTrait of Pos_or_decl.t
  ]

type package_error_info = {
  current_module_pos: Pos_or_decl.t;
  current_package_pos: Pos.t;
  current_package_name: string option;
  target_package_name: string option;
}

(** [can_access_public ~env ~current ~target] returns whether a symbol defined in
  * module [current] is allowed to access an public symbol defined in
  * [target] under [env].
  *)
val can_access_public :
  env:Typing_env_types.env ->
  current:string option ->
  target:string option ->
  [ `Yes
  | `PackageNotSatisfied of package_error_info
  | `PackageSoftIncludes of package_error_info
  ]

val is_class_visible : Typing_env_types.env -> Decl_provider.class_decl -> bool

val get_package_violation :
  Typing_env_types.env ->
  Package.t option ->
  Package.t option ->
  Package.package_relationship option
