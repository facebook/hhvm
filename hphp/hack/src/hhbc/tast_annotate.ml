(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module T = Tast

(* In order to create Tast nodes, sometimes it is necessary to annotate the
 * artificially created nodes with a tast annotation of pos * typing reason
 * Provides some helpers for doing so
 *)

(* Used to create artificial Tast.expr_annotation *)
let p = Pos.none

let witness = Typing_reason.Rwitness p

let null_annotation pos =
  T.make_expr_annotation pos (Typing_make_type.null witness)

(* TODO: (thomasjiang) T42271206 Annotate with the correct type when we start using types *)
let with_pos pos item = (null_annotation pos, item)

let make item = with_pos p item

let get_pos expr =
  let ((pos, _), _) = expr in
  pos
