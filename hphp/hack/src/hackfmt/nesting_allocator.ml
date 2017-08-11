(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t = {
  next_id: int;
  current_nesting: Nesting.t;
}

let make () = {
  next_id = 1;
  current_nesting = Nesting.make ~id:0 ~indent:false None false;
}

let nest t conditional =
  let current_nesting =
    Nesting.make ~id:t.next_id ~indent:true (Some t.current_nesting) conditional
  in
  { next_id = t.next_id + 1; current_nesting }

let unnest t =
  let current_nesting = match t.current_nesting.Nesting.parent with
    | Some p -> p
    | None -> raise (Failure "unnested too far")
  in
  {t with current_nesting}

let is_nested t = match t.current_nesting.Nesting.parent with
  | Some _ -> true
  | None -> false
