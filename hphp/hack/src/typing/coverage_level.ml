(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Utils

module TUtils = Typing_utils

type level =
  | Unchecked (* Completely unchecked code, i.e. Tanys *)
  | Partial   (* Partially checked code, e.g. array, Awaitable<_> with no
                 concrete type parameters *)
  | Checked   (* Completely checked code *)

let string_of_level = function
  | Checked   -> "checked"
  | Partial   -> "partial"
  | Unchecked -> "unchecked"

module CLMap = MyMap(struct
  type t = level
  let compare x y = Pervasives.compare x y
end)

let empty_counter =
  let m = CLMap.empty in
  let m = CLMap.add Checked 0 m in
  let m = CLMap.add Partial 0 m in
  CLMap.add Unchecked 0 m

let incr_counter k c =
  CLMap.add k (1 + CLMap.find_unsafe k c) c

(* An assoc list that counts the number of expressions at each coverage level *)
type level_counts = int CLMap.t

(* There is a trie in utils/, but it is not quite what we need ... *)

type 'a trie =
  | Leaf of 'a
  | Node of 'a * 'a trie SMap.t

let mk_level_map fn_opt pos_ty_m =
  let pos_lvl_m = Pos.Map.map (function
    | _, Typing_defs.Tany -> Unchecked
    | ty when TUtils.HasTany.check ty -> Partial
    | _ -> Checked) pos_ty_m
  in
  (* If the line has a HH_FIXME, then mark it as (at most) partially checked *)
  (* NOTE(jez): can we monadize this? *)
  match fn_opt with
  | None -> pos_lvl_m
  | Some fn ->
    match Parser_heap.HH_FIXMES.get fn with
    | None -> pos_lvl_m
    | Some fixme_map ->
        Pos.Map.mapi (fun p lvl ->
          let line = p.Pos.pos_start.Lexing.pos_lnum in
          match lvl with
          | Checked when IMap.mem line fixme_map ->
              Partial
          | Unchecked | Partial | Checked -> lvl
        ) pos_lvl_m
