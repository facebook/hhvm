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

let make (pos, ty) =
  pos, match ty with
  | _, Typing_defs.Tany -> Unchecked
  | _ when TUtils.HasTany.check ty -> Partial
  | _ -> Checked

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
