(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type tag =
  | Required
  | LateInit
[@@deriving eq, show]

type t = {
  xa_tag: tag option;
  xa_has_default: bool;
}
[@@deriving eq, show]

val init : t

val map_tag : t -> f:(tag option -> tag option) -> t
