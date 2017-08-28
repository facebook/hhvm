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
  indent: bool;
  parent: t option;
  skip_parent_if_nested: bool;
}

let dummy =
  {id = -1; indent = false; parent = None; skip_parent_if_nested = false;}

let make ~id ~indent parent skip_parent = {
  id;
  indent;
  parent;
  skip_parent_if_nested = skip_parent;
}

let get_indent_level nesting nesting_set =
  let rec aux acc n =
    match n with
      | None -> acc
      | Some n ->
        let in_set = ISet.mem n.id nesting_set in
        let acc = if n.indent && in_set then acc + 1 else acc in
        aux acc @@
          if in_set && n.skip_parent_if_nested
          then Option.(>>=) n.parent (fun p -> p.parent)
          else n.parent
  in
  aux 0 (Some nesting)

let get_self_and_parent_list nesting =
  let rec aux acc n =
    Option.value_map n ~default:acc ~f:(fun n -> aux (n.id :: acc) n.parent)
  in
  aux [] nesting
