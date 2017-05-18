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
  nesting: Nesting.t;
  start_char: int;
  end_char: int;
  indentable: bool;
}

let default_chunk = {
  text = "";
  spans = [];
  is_appendable = true;
  space_if_not_split = false;
  comma_rule = None;
  rule = Rule.null_rule_id;
  nesting = Nesting.dummy;
  start_char = -1;
  end_char = -1;
  indentable = true;
}

let make text rule nesting start_char =
  let c = match rule with
    | None -> {default_chunk with start_char}
    | Some rule -> {default_chunk with rule; start_char}
  in
  {c with text; nesting;}

let finalize chunk rule ra space comma end_char =
  let end_char = max chunk.start_char end_char in
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
    end_char;
  }

let get_nesting_id chunk =
  chunk.nesting.Nesting.id
