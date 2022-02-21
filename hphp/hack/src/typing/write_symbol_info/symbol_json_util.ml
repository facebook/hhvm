(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Fact_id = Symbol_fact_id

let get_type_from_hint ctx h =
  let mode = FileInfo.Mhhi in
  let decl_env = Decl_env.{ mode; droot = None; ctx } in
  let tcopt = Provider_context.get_tcopt ctx in
  Typing_print.full_decl tcopt (Decl_hint.hint decl_env h)

(* Replace any codepoints that are not valid UTF-8 with
the unrepresentable character. *)
let check_utf8 str =
  let b = Buffer.create (String.length str) in
  let replace_malformed () _index = function
    | `Uchar u -> Uutf.Buffer.add_utf_8 b u
    | `Malformed _ -> Uutf.Buffer.add_utf_8 b Uutf.u_rep
  in
  Uutf.String.fold_utf_8 replace_malformed () str;
  Buffer.contents b

let source_at_span source_text pos =
  let st = Pos.start_offset pos in
  let fi = Pos.end_offset pos in
  let source_text = Full_fidelity_source_text.sub source_text st (fi - st) in
  check_utf8 source_text

let strip_nested_quotes str =
  let len = String.length str in
  let firstc = str.[0] in
  let lastc = str.[len - 1] in
  if
    len >= 2
    && ((Char.equal '"' firstc && Char.equal '"' lastc)
       || (Char.equal '\'' firstc && Char.equal '\'' lastc))
  then
    String.sub str ~pos:1 ~len:(len - 2)
  else
    str

let strip_tparams name =
  match String.index name '<' with
  | None -> name
  | Some i -> String.sub name ~pos:0 ~len:i

let ends_in_newline source_text =
  let last_char =
    Full_fidelity_source_text.(get source_text (source_text.length - 1))
  in
  Char.equal '\n' last_char || Char.equal '\r' last_char

let has_tabs_or_multibyte_codepoints source_text =
  let open Full_fidelity_source_text in
  let check_codepoint (num, found) _index = function
    | `Uchar u -> (num + 1, found || Uchar.equal u (Uchar.of_char '\t'))
    | `Malformed _ -> (num + 1, true)
  in
  let (num_chars, found_tab_or_malformed) =
    Uutf.String.fold_utf_8 check_codepoint (0, false) source_text.text
  in
  found_tab_or_malformed || num_chars < source_text.length

let split_name (s : string) : (string * string) option =
  match String.rindex s '\\' with
  | None -> None
  | Some pos ->
    let name_start = pos + 1 in
    let name =
      String.sub s ~pos:name_start ~len:(String.length s - name_start)
    in
    let parent_namespace = String.sub s ~pos:0 ~len:(name_start - 1) in
    if String.is_empty parent_namespace || String.is_empty name then
      None
    else
      Some (parent_namespace, name)

let add_xref target_json target_id ref_pos xrefs =
  let filepath = Relative_path.to_absolute (Pos.filename ref_pos) in
  SMap.update
    filepath
    (fun file_map ->
      let new_ref = (target_json, [ref_pos]) in
      match file_map with
      | None -> Some (Fact_id.Map.singleton target_id new_ref)
      | Some map ->
        let updated_xref_map =
          Fact_id.Map.update
            target_id
            (fun target_tuple ->
              match target_tuple with
              | None -> Some new_ref
              | Some (json, refs) -> Some (json, ref_pos :: refs))
            map
        in
        Some updated_xref_map)
    xrefs
