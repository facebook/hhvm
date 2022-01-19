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
[@@deriving eq, show { with_path = false }]

type t = {
  xa_tag: tag option;
  xa_has_default: bool;
}
[@@deriving eq, show { with_path = false }]

let init : t = { xa_tag = None; xa_has_default = false }

let map_tag attr ~f = { attr with xa_tag = f attr.xa_tag }
