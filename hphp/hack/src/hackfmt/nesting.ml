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
  id: int;
  amount: int;
  parent: t option;
  skip_parent_if_nested: bool;
}

let dummy =
  {id = -1; amount = 0; parent = None; skip_parent_if_nested = false;}

let make ~id amount parent skip_parent = {
  id;
  amount;
  parent;
  skip_parent_if_nested = skip_parent;
}

let get_indent nesting nesting_set =
  let rec aux n =
    match n with
      | None -> 0
      | Some n ->
        if ISet.mem n.id nesting_set then
          if n.skip_parent_if_nested
          then n.amount + (aux @@ Option.(>>=) n.parent (fun p -> p.parent))
          else n.amount + aux n.parent
        else aux n.parent
  in
  aux (Some nesting)

let get_self_and_parent_list nesting =
  let rec aux acc n =
    Option.value_map n ~default:acc ~f:(fun n -> aux (n.id :: acc) n.parent)
  in
  aux [] nesting
