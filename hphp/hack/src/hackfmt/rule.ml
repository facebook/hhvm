(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type kind =
  | Simple of Cost.t
  | Always
  | Argument
  | XHPExpression

type t = {
  id: int;
  kind: kind;
}

let null_rule_id = -1

let is_split _rule v =
  match v with
    | None
    | Some 0 -> false
    | _ -> true

let get_cost kind =
  Cost.get_cost @@ match kind with
    | Simple cost -> cost
    | Always -> Cost.NoCost
    | Argument
    | XHPExpression -> Cost.Base

let get_possible_values _id =
  [1]

let cares_about_children kind =
  match kind with
    | Simple _ -> false
    | Always -> false
    | Argument -> true
    | XHPExpression -> true

let compare r1 r2 = Pervasives.compare r1.id r2.id

let to_string rule =
  let kind = match rule.kind with
    | Simple cost -> Printf.sprintf "Simple %d" @@ Cost.get_cost cost
    | Always -> "Always"
    | Argument -> "Argument"
    | XHPExpression -> "XHPExpression"
  in
  (string_of_int rule.id) ^ " - " ^ kind
