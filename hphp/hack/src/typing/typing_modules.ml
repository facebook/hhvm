(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type module_ = string [@@deriving eq, show]

let of_string s = s

let of_maybe_string = Option.map of_string

type t = module_ option [@@deriving eq, show]

let can_access ~current ~target =
  match (current, target) with
  | (Some m, Some n) when String.equal m n -> `Yes
  | (Some _, None)
  | (None, None) ->
    `Yes
  | (Some m, Some n) -> `Disjoint (m, n)
  | (None, Some n) -> `Outside n
