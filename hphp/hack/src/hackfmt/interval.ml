(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Helpers for half-open intervals *)

open Core_kernel

type t = int * int

let pp fmt (st, ed) = Format.fprintf fmt "[%d, %d)" st ed

let show = Format.asprintf "%a" pp

let contains ((st, ed) : t) (point : int) : bool = st <= point && point < ed

let contains_interval (a : t) (b : t) : bool =
  let (a_start, a_end) = a in
  let (b_start, b_end) = b in
  a_start <= b_start && a_end >= b_end

let intervals_overlap (a : t) (b : t) : bool =
  let (a_start, a_end) = a in
  let (b_start, b_end) = b in
  a_start = b_start
  || (a_start < b_start && b_start < a_end)
  || (b_start < a_start && a_start < b_end)

(* Does not union adjacent intervals *)
let union (a : t) (b : t) : t option =
  if intervals_overlap a b then
    Some (min (fst a) (fst b), max (snd a) (snd b))
  else
    None

(* Does not union adjacent intervals *)
let union_consecutive_overlapping (intervals : t list) : t list =
  match intervals with
  | [] -> []
  | hd :: tl ->
    let (last, rev_unioned_except_last) =
      List.fold tl ~init:(hd, []) ~f:(fun (unioned_ival, rest) next_ival ->
          match union unioned_ival next_ival with
          | None -> (next_ival, unioned_ival :: rest)
          | Some unioned_ival -> (unioned_ival, rest))
    in
    List.rev (last :: rev_unioned_except_last)

let compare (a : t) (b : t) : int =
  let (a_start, a_end) = a in
  let (b_start, b_end) = b in
  if a_start = b_start then
    a_end - b_end
  else
    a_start - b_start

let difference (a : t) (b : t) : t list =
  let (a_start, a_end) = a in
  let (b_start, b_end) = b in
  if not (intervals_overlap a b) then
    [a]
  else if contains_interval b a then
    []
  else if contains b a_start then
    [(b_end, a_end)]
  else if contains b a_end then
    [(a_start, b_start)]
  else if contains_interval a b then
    [(a_start, b_start); (b_end, a_end)]
  else
    failwith
      (Format.asprintf
         "Interval.difference: invalid intervals: %a %a"
         pp
         a
         pp
         b)

(** Subtract [subtrahends] from [minuends].

    Assumes both lists are sorted and non-overlapping (i.e., [List.sort
    ~compare:Interval.compare] and [union_consecutive_overlapping] would be
    no-ops for both) *)
let diff_sorted_lists (minuends : t list) (subtrahends : t list) : t list =
  let rec aux acc minuends subtrahends =
    match (minuends, subtrahends) with
    (* Base case: nothing to diminish *)
    | ([], _) -> acc
    (* Base case: nothing to subtract *)
    | (minuends, []) -> List.rev_append minuends acc
    (* The next subtrahend overlaps with the next minuend. Take the difference
       and put any results back into minuends. Continue to the next minuend with
       the same subtrahend. *)
    | (minuend :: minuends_tl, subtrahend :: _)
      when intervals_overlap minuend subtrahend ->
      let diff = difference minuend subtrahend in
      aux acc (diff @ minuends_tl) subtrahends
    (* The next minuend ends before the remaining subtrahends start. Append it
       to the result. Continue to the next minuend with the same subtrahend. *)
    | (minuend :: minuends_tl, subtrahend :: _)
      when snd minuend <= fst subtrahend ->
      aux (minuend :: acc) minuends_tl subtrahends
    (* The next subtrahend ends before the remaining minuends start.
       Continue to the next subtrahend with the same minuend. *)
    | (minuend :: _, subtrahend :: subtrahends_tl)
      when snd subtrahend <= fst minuend ->
      aux acc minuends subtrahends_tl
    | _ -> failwith "Invalid interval lists"
  in
  (* Avoid double-reverse of minuends when given nothing to subtract. *)
  if List.is_empty subtrahends then
    minuends
  else
    List.rev (aux [] minuends subtrahends)
