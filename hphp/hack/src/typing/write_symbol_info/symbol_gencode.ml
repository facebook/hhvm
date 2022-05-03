(*
 * Copyright (c) Meta, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type t = {
  mutable is_generated: bool;
  mutable fully_generated: bool;
  mutable source: string option;
  mutable command: string option;
  mutable class_: string option;
  mutable signature: string option;
}

let regex_sig =
  {|\(^.*@\(partially-\)?generated\( SignedSource<<\([0-9a-f]+\)>>\)?.*$\)|}

let regex_codegen = {|\(^.*@codegen-\(command\|class\|source\).*: *\(.*\)$\)|}

let regex = Str.regexp (regex_sig ^ {|\||} ^ regex_codegen)

let has_group text i =
  Option.(try_with (fun () -> Str.matched_group i text) |> is_some)

let extract_group text i = Option.try_with (fun () -> Str.matched_group i text)

let update_fields t text =
  t.is_generated <- true;
  if has_group text 1 then (
    t.fully_generated <- not (has_group text 2);
    t.signature <- extract_group text 4
  ) else
    match extract_group text 6 with
    | Some "command" -> t.command <- extract_group text 7
    | Some "source" -> t.source <- extract_group text 7
    | Some "class" -> t.class_ <- extract_group text 7
    | _ -> Hh_logger.log "WARNING: this shouldn't happen."

let search_and_update t text pos =
  try
    let found_pos = Str.search_forward regex text pos in
    update_fields t text;
    (true, found_pos)
  with
  | Caml.Not_found -> (false, 0)

let get_gencode_status text =
  let t =
    {
      is_generated = false;
      fully_generated = true;
      source = None;
      command = None;
      class_ = None;
      signature = None;
    }
  in
  let cur_pos = ref 0 in
  let matched = ref true in
  while !matched do
    let (m, cp) = search_and_update t text !cur_pos in
    cur_pos := cp + 1;
    matched := m
  done;
  t
