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

type t = int [@@deriving eq, hash, ord, show]

let track_names = ref false

let trace = ref IMap.empty

let set_name id name = trace := IMap.add id name !trace

let tmp () =
  let res = hh_counter_next () in
  if !track_names then set_name res ("__tmp" ^ string_of_int res);
  res

let to_string x =
  match IMap.find_opt x !trace with
  | Some res -> res
  | None -> "v" ^ string_of_int x

let make name =
  let res = hh_counter_next () in
  if !track_names then set_name res name;
  res
