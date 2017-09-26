(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(** Helpers for half-open intervals *)

open Core

type t = int * int

let contains ((st, ed): t) (point: int) : bool =
  st <= point && point < ed

let intervals_overlap (a: t) (b: t) : bool =
  let a_start, a_end = a in
  let b_start, b_end = b in
  a_start = b_start ||
  a_start < b_start && b_start < a_end ||
  b_start < a_start && a_start < b_end

(* Does not union adjacent intervals *)
let union (a: t) (b: t) : t option =
  if intervals_overlap a b
    then Some (min (fst a) (fst b), max (snd a) (snd b))
    else None

(* Does not union adjacent intervals *)
let union_list (intervals: t list) : t list =
  List.fold intervals ~init:[] ~f:(fun unioned interval ->
    let unioned_ival, others = List.fold unioned ~init:(interval, [])
      ~f:(fun (unioned_ival, others) interval ->
        match union unioned_ival interval with
        | None -> unioned_ival, interval :: others
        | Some unioned_ival -> unioned_ival, others
      )
    in
    unioned_ival :: others
  )

let comparator (a: t) (b: t) : int =
  let a_start, a_end = a in
  let b_start, b_end = b in
  if a_start = b_start then a_end - b_end else a_start - b_start
