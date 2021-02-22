(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(***********************************************)
(* Which dependency graph format are we using? *)
(***********************************************)
type t =
  | SQLiteMode  (** Legacy mode, with SQLite saved-state dependency graph *)
  | CustomMode of string option
      (** Custom mode, with the new custom dependency graph format.
        * The parameter is the path to the database. *)
  | SaveCustomMode of {
      graph: string option;
      new_edges_dir: string;
    }
      (** Mode to produce both the legacy SQLite saved-state dependency graph,
        * and, along side it, the new custom 64-bit dependency graph.
        *
        * The first parameter is (optionally) a path to an existing custom 64-bit
        * dependency graph. If it is present, only new edges will be written,
        * of not, all edges will be written. *)
[@@deriving show]

type hash_mode =
  | Hash32Bit
  | Hash64Bit
[@@deriving show, eq]

let hash_mode : t -> hash_mode = function
  | SQLiteMode -> Hash32Bit
  | CustomMode _ -> Hash64Bit
  | SaveCustomMode _ -> Hash64Bit

let is_64bit : t -> bool =
 fun mode ->
  match hash_mode mode with
  | Hash32Bit -> false
  | Hash64Bit -> true
