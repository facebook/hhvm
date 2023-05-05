(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

external hh_counter_next : unit -> int = "hh_counter_next"

type t = int [@@deriving eq, hash]

let compare = Int.compare

let track_names = ref false

let trace = ref IMap.empty

let tmp () =
  let res = hh_counter_next () in
  if !track_names then
    trace := IMap.add res ("__tmp" ^ string_of_int res) !trace;
  res

let to_string x =
  match IMap.find_opt x !trace with
  | Some res -> res
  | None -> "v" ^ string_of_int x

let debug ?normalize:(f = (fun x -> x)) x =
  let normalized_x = string_of_int (f x) in
  match IMap.find_opt x !trace with
  | Some result -> result ^ "[" ^ normalized_x ^ "]"
  | None -> "tvar_" ^ normalized_x

[@@@warning "+3"]

let get_name x =
  assert !track_names;
  IMap.find x !trace

let set_name x y = trace := IMap.add x y !trace

let make x =
  let res = hh_counter_next () in
  if !track_names then set_name res x;
  res

let pp = Format.pp_print_int

let not_equal x y = not @@ equal x y

let hash_range_min = 100_000_000_000_000

let hash_range_max = 1_000_000_000_000_000

(* Probability of collision: if N = hash_range_max - hash_range_min and k is the
 * number of values to hash, the probability of collision is 1 - e^((-k*(k-1)/(2*N))).
 * So if k = 1'000'000, N = 10^14 gives a collision probability of 0.005 *)
let from_string_hash s =
  let hash = Base.String.hash s in
  (* make this an int between hash_range_min and hash_range_max - 1 *)
  let hash = (hash % (hash_range_max - hash_range_min)) + hash_range_min in
  hash

let immutable_mask = 1 lsl 62

let is_immutable i = i land immutable_mask <> 0

let make_immutable i = i lor immutable_mask
