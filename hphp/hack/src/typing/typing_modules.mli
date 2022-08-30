(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** [can_access env ~current ~target] returns whether a symbol defined in
  * module [current] is allowed to access an internal symbol defined in
  * [target] under [env].
  *)
val can_access :
  env:Typing_env_types.env ->
  current:string option ->
  target:string option ->
  [ `Yes
  | `Disjoint of string * string
  | `Outside of string
  | `OutsideViaTrait of Pos_or_decl.t
  ]

val is_class_visible : Typing_env_types.env -> Decl_provider.class_decl -> bool
