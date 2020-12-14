(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type 'a t = 'a list

let empty = []

let cons x xs = x :: xs

let create t = t

let as_list t = t

let is_empty t = List.is_empty t

let length t = List.length t

let decons t =
  match t with
  | [] -> None
  | hd :: tl -> Some (hd, tl)

let filter t ~f = List.filter t f

let map t ~f = List.map t f

let append (xs : 'a list) (ys : 'a t) : 'a t = List.append xs ys

let rev_append (xs : 'a list) (ys : 'a t) : 'a t = List.rev_append xs ys

let rev t = List.rev t

let split_n t_orig n = List.split_n t_orig n

let pp _elem_pp fmt t = Format.pp_print_int fmt (List.length t)
