(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type fixmes = Pos.t IMap.t IMap.t [@@deriving show, eq]

type t = {
  sc_comments: (Pos.t * Prim_defs.comment) list;
  sc_fixmes: fixmes;
  sc_misuses: fixmes;
  sc_error_pos: Pos.t list;
}
[@@deriving show, eq]

let get_fixme_pos (fixmes : fixmes) (pos : Pos.t) (code : int) : Pos.t option =
  let (line, _, _) = Pos.info_pos pos in
  Option.bind (IMap.find_opt line fixmes) ~f:(IMap.find_opt code)
