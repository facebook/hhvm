(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type event =
  | Errors of Errors.t
  | TypingStart of int
  | TypingProgress of int
[@@deriving show]
