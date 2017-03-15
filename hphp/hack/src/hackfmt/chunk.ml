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
  text: string;
  spans: Span.t list;
  is_appendable: bool;
  space_if_not_split: bool;
  comma_rule: int option;
  rule: int;
  nesting: Nesting.t
}

let default_chunk = {
  text = "";
  spans = [];
  is_appendable = true;
  space_if_not_split = false;
  comma_rule = None;
  rule = Rule.null_rule_id;
  nesting = Nesting.dummy;
}

let make text rule nesting =
  let c = match rule with
    | None -> default_chunk
    | Some r -> {default_chunk with rule = r}
  in
  {c with text; nesting;}

let finalize chunk rule ra space comma =
  let rule = if Rule_allocator.get_rule_kind ra rule = Rule.Always
    || chunk.rule = Rule.null_rule_id
    then rule
    else chunk.rule
  in
  {chunk with
    is_appendable = false;
    rule;
    space_if_not_split = space;
    comma_rule = comma;
  }

let get_nesting_id chunk =
  chunk.nesting.Nesting.id

let to_string chunk =
  Printf.sprintf "rule_id:%d\t text:%s" chunk.rule chunk.text
