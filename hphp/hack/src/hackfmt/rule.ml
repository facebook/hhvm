(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type kind =
  | Simple of Cost.t
  | Always
  | Parental

type t = {
  id: int;
  kind: kind;
}

let null_rule_id = -1

let get_cost kind =
  Cost.get_cost
  @@
  match kind with
  | Simple cost -> cost
  | Always -> Cost.NoCost
  | Parental -> Cost.Base

let cares_about_children kind =
  match kind with
  | Simple _ -> false
  | Always -> false
  | Parental -> true

let compare r1 r2 = Core_kernel.Int.compare r1.id r2.id

let to_string rule =
  let kind =
    match rule.kind with
    | Simple cost -> Printf.sprintf "Simple %d" @@ Cost.get_cost cost
    | Always -> "Always"
    | Parental -> "Parental"
  in
  string_of_int rule.id ^ " - " ^ kind
