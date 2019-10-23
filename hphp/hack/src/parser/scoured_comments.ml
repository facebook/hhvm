(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type fixmes = Pos.t IMap.t IMap.t [@@deriving show]

let equal_fixmes = IMap.equal (IMap.equal ( = ))

type t = {
  sc_comments: (Pos.t * Prim_defs.comment) list;
  sc_fixmes: fixmes;
  sc_misuses: fixmes;
  sc_error_pos: Pos.t list;
}
[@@deriving show, eq]
