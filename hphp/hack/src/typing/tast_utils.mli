(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val type_non_nullable : Tast_env.env -> Tast.ty -> bool

val valid_newable_class : Decl_provider.Class.t -> bool

type truthiness =
  | Unknown
  | Always_truthy
  | Always_falsy
  | Possibly_falsy

val truthiness : Tast_env.env -> Tast.ty -> truthiness

type sketchy_type_kind =
  | String
  | Arraykey
  | Stringish
  | XHPChild
  | Traversable_interface of string

val find_sketchy_types : Tast_env.env -> Tast.ty -> sketchy_type_kind list
