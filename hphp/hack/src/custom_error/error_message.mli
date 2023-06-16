(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type elem =
  | Lit of string
  | Ty_var of Patt_var.t
  | Name_var of Patt_var.t
[@@deriving eq, show]

type t = { message: elem list } [@@deriving eq, show]
