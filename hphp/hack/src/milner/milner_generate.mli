(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type ty

val defs : string list ref

val expr_for : ty -> string

val show_ty : ty -> string

val ty : unit -> ty
