(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Subtype_op : sig
  type t = {
    ty_sub: Typing_defs_core.locl_ty;
    ty_super: Typing_defs_core.locl_ty;
  }
  [@@deriving eq]
end

include module type of Recursion_tracker.Make (Subtype_op)
