(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t =
  | GlobalConstant of string
  | Function of string
  | Type of string
[@@deriving eq, show, ord]
