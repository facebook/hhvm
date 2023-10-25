(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = private int

(* return consecutive fact ids, starting from 1 *)
val next : unit -> t

val compare : t -> t -> int

val to_json_number : t -> Hh_json.json

module Map : Map.S with type key = t
