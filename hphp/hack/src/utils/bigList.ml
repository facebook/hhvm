(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type 'a t = int * 'a list

let empty = (0, [])

let cons hd (length, tl) = (length + 1, hd :: tl)

let create list = (List.length list, list)

let as_list (_length, list) = list

let is_empty (length, _list) = length = 0

let length (length, _list) = length

let decons (length, list) =
  match list with
  | [] -> None
  | hd :: tl -> Some (hd, (length - 1, tl))

let map (length, list) ~f = (length, List.map list ~f)

let append x (length, list) =
  (* List.append has several constant-factor performance tricks up its sleeve,
  which I don't want to replicate here. That's why I'm calling both List.length
  and List.append here, rather than doing only a single traversal of x.
  In any case, by assumption, x isn't a big list so it doesn't matter. *)
  (length + List.length x, List.append x list)

let rev_append x (length, list) =
  (* See implementtion notes in [append] about efficiency. *)
  (length + List.length x, List.rev_append x list)

let rev (length, list) = (length, List.rev list)

let rev_filter (_length, list) ~f =
  let rec find ~f (length, acc) list =
    match list with
    | [] -> (length, acc)
    | hd :: tl ->
      if f hd then
        find ~f (length + 1, hd :: acc) tl
      else
        find ~f (length, acc) tl
  in
  find ~f empty list

let filter t ~f = rev (rev_filter t ~f)

let split_n (length, list) n =
  (* Could be more efficient if we copy the implementation of List.split_n, to avoid
  iterating over [split] twice, but by assumption n is small so it doesn't matter. *)
  let (split, rest) = List.split_n list n in
  (split, (length - List.length split, rest))

let pp _elem_pp fmt t = Format.pp_print_int fmt (length t)
