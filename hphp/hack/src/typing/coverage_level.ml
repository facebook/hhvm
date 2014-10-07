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

type t =
  | Unchecked (* Completely unchecked code, i.e. Tanys *)
  | Checked   (* Completely checked code *)
  | Partial   (* Partially checked code, e.g. array, Awaitable<_> with no
                 concrete type parameters *)

let string = function
  | Checked   -> "checked"
  | Partial   -> "partial"
  | Unchecked -> "unchecked"

let empty_counter = [
  Unchecked, 0;
  Checked, 0;
  Partial, 0;
]

type result = {
  (* An assoc list that counts the number of expressions at each coverage level *)
  counts     : (t * int) list;
  (* A number between 0 to 1 that summarizes the extent of coverage *)
  percentage : float;
}

(* There is a trie in utils/, but it is not quite what we need ... *)

type 'a trie =
  | Leaf of 'a
  | Node of 'a * 'a trie SMap.t

let mk_level_list fn_opt pos_ty_l =
  let pos_lvl_l = rev_rev_map (fun (pos, ty) ->
    pos, match ty with
    | _, Typing_defs.Tany -> Unchecked
    | _ when TUtils.HasTany.check ty -> Partial
    | _ -> Checked) pos_ty_l
  in
  (* If the line has a HH_FIXME, then mark it as (at most) partially checked *)
  (* NOTE(jez): can we monadize this? *)
  match fn_opt with
  | None -> pos_lvl_l
  | Some fn ->
    match Parser_heap.HH_FIXMES.get fn with
    | None -> pos_lvl_l
    | Some fixme_map ->
        rev_rev_map (fun (p, lvl as pos_lvl) ->
          let line = p.Pos.pos_start.Lexing.pos_lnum in
          match lvl with
          | Checked when IMap.mem line fixme_map ->
              (p, Partial)
          | Unchecked | Partial | Checked ->
              pos_lvl
        ) pos_lvl_l
