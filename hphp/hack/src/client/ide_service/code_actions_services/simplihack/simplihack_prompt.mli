(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  attribute_pos: Pos.t;
  derive_prompt: unit -> string option;
  edit_span: Pos.t;
}

val find : Provider_context.t -> Tast.def list -> t list
