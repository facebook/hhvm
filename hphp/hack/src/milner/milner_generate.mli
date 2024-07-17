(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type ty

type def

val show_def : def -> string

val expr_for : ty -> string * def list

val show_ty : ty -> string

val ty : unit -> ty * def list
