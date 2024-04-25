(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t =
  | Sketchy_equality of {
      pos: Pos.t;
      result: bool;
      left: Pos_or_decl.t Message.t list Lazy.t;
      right: Pos_or_decl.t Message.t list Lazy.t;
      left_trail: Pos_or_decl.t list;
      right_trail: Pos_or_decl.t list;
    }
