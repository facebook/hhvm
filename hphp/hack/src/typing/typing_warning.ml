(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type warn = private Warn_tag

type migrated = private Migrated_tag

type quickfix = {
  can_be_captured: bool;
  original_pos: Pos.t;
  replacement_pos: Pos.t;
}

module IsAsAlways = struct
  type kind =
    | Is_is_always_true
    | Is_is_always_false
    | As_always_succeeds of quickfix
    | As_always_fails

  type t = {
    kind: kind;
    lhs_ty: string;
    rhs_ty: string;
  }
end

type _ t_ =
  | Sketchy_equality : {
      result: bool;
      left: Pos_or_decl.t Message.t list Lazy.t;
      right: Pos_or_decl.t Message.t list Lazy.t;
      left_trail: Pos_or_decl.t list;
      right_trail: Pos_or_decl.t list;
    }
      -> warn t_
  | Is_as_always : IsAsAlways.t -> migrated t_

type 'a t = Pos.t * 'a t_
