(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core

type t =
  | Exactly of string
  | Starts_with of string
  | Ends_with of string
  | Contains of string
  | Or of t list
  | And of t list
  | Not of t
[@@deriving compare, eq, sexp, show]
