(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t =
  | NoCost
  | Base
  | Moderate
  | High

let get_cost t =
  match t with
  | NoCost -> 0
  | Base -> 1
  | Moderate -> 2
  | High -> 3
