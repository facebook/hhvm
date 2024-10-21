(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let line_gap = 4

let pad_before = 1

let pad_after = 2

let cursor_l = "»"

let cursor_r = "«"

type merge_result =
  | Only of Pos.t
  | Both of Pos.t * Pos.t

(* Invariant: pos0.start <= pos1.start *)
let merge pos0 pos1 =
  if Pos.contains pos0 pos1 then
    Only pos0
  else
    let end_0 = Pos.end_line pos0 and beg_1 = Pos.line pos1 in
    let gap = abs (beg_1 - end_0) in
    if gap >= line_gap then
      Both (pos0, pos1)
    else
      Only (Pos.merge pos0 pos1)

let rec merge_all ps ~cur ~acc =
  match ps with
  | p :: ps ->
    (match merge cur p with
    | Only cur -> merge_all ps ~cur ~acc
    | Both (p, q) -> merge_all ps ~cur:q ~acc:(p :: acc))
  | [] -> List.rev (cur :: acc)

let merge ps =
  match ps with
  | p :: ps -> merge_all ps ~cur:p ~acc:[]
  | [] -> []

(* Given a position and the list of all spans for each file:
   1) Find the relevant file and find the first span that contains the position
   2) Select the lines from that file corresponding to the containing span
   3) For each line prepend the line number and a gutter marker
   4) For the line(s) containing the position we want to mark, insert a cursor
      before and after the start column on the start line and after the end
      column on the end line
*)
let mark_with_context (pos : Pos.absolute) ~buf ~spans =
  (* Find the containing span for our position *)
  let path = Pos.filename pos in
  let ctxt_pos_opt =
    Option.bind
      (SMap.find_opt path spans)
      ~f:
        (List.find ~f:(fun spos ->
             try Pos.contains spos @@ Pos.to_relative pos with
             | _ -> false))
  in
  match ctxt_pos_opt with
  | Some ctxt_pos ->
    (* Determine start and end lines and columns and reindex to 0 *)
    let (ctxt_start_ln, ctxt_end_ln, start_ln, end_ln, start_col, end_col) =
      let ctxt_start_ln = Pos.line ctxt_pos
      and ctxt_end_ln = Pos.end_line ctxt_pos in
      let (start_ln, end_ln, start_col, end_col) = Pos.info_pos_extended pos in
      ( ctxt_start_ln - pad_before - 1,
        ctxt_end_ln + pad_after - 1,
        start_ln - 1,
        end_ln - 1,
        start_col - 1,
        end_col - 1 )
    in
    (* We'll use the same number of characters for each line number so find how
       many chars the longest line number needs *)
    let line_num_width = 1 + (String.length @@ string_of_int end_ln) in
    (* Write to buffer *)
    List.iteri (Errors.read_lines path) ~f:(fun ln str ->
        if ln < ctxt_start_ln || ln > ctxt_end_ln then
          (* This line isn't in our containing span so skip it *)
          ()
        else if
          (ln = ctxt_start_ln || ln = ctxt_end_ln)
          && String.(is_empty @@ lstrip str)
        then
          (* If the line is blank and is either the first of last line, skip it *)
          ()
        else begin
          (* First write the line number and gutter marker  *)
          let ln_num =
            String.pad_left
              (Int.to_string (ln + 1))
              ~char:' '
              ~len:line_num_width
          in
          let (_ : unit) =
            Buffer.add_string buf ln_num;
            Buffer.add_string buf " | "
          in
          let is_start = ln = start_ln and is_end = ln = end_ln in
          let (_ : unit) =
            if is_start && is_end then (
              (* If our target pos starts and ends on the line..
                 - write the prefix before the start of the marked pos
                 - write the lhs cursor marker
                 - write the marked string
                 - write the rhs cursor marker
                 - write the suffix after the end of the marked pos
              *)
              Buffer.add_string buf (String.prefix str start_col);
              Buffer.add_string buf cursor_l;
              Buffer.add_string buf (String.slice str start_col (end_col + 1));
              Buffer.add_string buf cursor_r;
              Buffer.add_string buf (String.drop_prefix str (end_col + 1))
            ) else if is_start then (
              (* If this is the start line, write the prefix, the lhs cursor and
                 substring of the marked pos that is on this line *)
              Buffer.add_string buf (String.prefix str start_col);
              Buffer.add_string buf cursor_l;
              Buffer.add_string buf (String.drop_prefix str start_col)
            ) else if is_end then (
              (* If this is the end line, write the substring of the marked pos
                 that is on the line, the rhs cursor and the suffix *)
              Buffer.add_string buf (String.prefix str end_col);
              Buffer.add_string buf cursor_r;
              Buffer.add_string buf (String.drop_prefix str end_col)
            ) else
              (* Either the whole line is part of the target pos or the target
                 is not on the line at all; either way, write the entire line *)
              Buffer.add_string buf str
          in
          Buffer.add_string buf "\n"
        end)
  | _ -> Buffer.add_string buf "No source found.\n\n"

let render_reason buf spans (pos, msg) =
  let pos_string =
    let fn = Pos.filename pos in
    if String.is_suffix fn ~suffix:".hhi" then
      let pos_str = Pos.multiline_string_no_file pos in
      match String.split fn ~on:'/' with
      | _ :: "tmp" :: _ :: rest ->
        let nm = String.concat ~sep:"/" rest in
        Format.sprintf {|File "%s", %s:|} nm pos_str
      | _ -> Format.sprintf "%s:" pos_str
    else
      Pos.multiline_string pos
  in
  Buffer.add_string buf msg;
  Buffer.add_string buf "\n\n";
  Buffer.add_string buf pos_string;
  Buffer.add_string buf "\n\n";
  mark_with_context pos ~buf ~spans;
  Buffer.add_string buf "\n"

let render_elem buf spans elem =
  match elem with
  | Explanation.Witness (pos, msg) -> render_reason buf spans (pos, msg)
  | Explanation.Witness_no_pos msg ->
    Buffer.add_string buf msg;
    Buffer.add_string buf "\n"
  | Explanation.Rule msg ->
    Buffer.add_string buf "\n";
    Buffer.add_string buf msg;
    Buffer.add_string buf "\n\n"
  | Explanation.Path (msg, false) ->
    Buffer.add_string buf "\n";
    Buffer.add_string buf msg;
    Buffer.add_string buf "\n"
  | Explanation.Path (msg, true) ->
    Buffer.add_string buf "\n";
    Buffer.add_string buf msg;
    Buffer.add_string buf " (here is where the error occurred)";
    Buffer.add_string buf "\n"
  | Explanation.Trans msg ->
    Buffer.add_string buf msg;
    Buffer.add_string buf "\n"
  | Explanation.Prefix { prefix; sep } ->
    Buffer.add_string buf prefix;
    Buffer.add_string buf sep
  | Explanation.Suffix { suffix; sep = _ } ->
    Buffer.add_string buf suffix;
    Buffer.add_string buf "\n\n"

let render_derivation buf spans elems =
  Buffer.add_string buf "Here's why:\n\n";
  List.iter elems ~f:(render_elem buf spans)

let render_claim buf spans code (pos, msg) severity =
  Buffer.add_string buf
  @@ Format.sprintf
       "%s: %s %s\n\n"
       (User_error.Severity.to_string severity)
       (User_error.error_code_to_string code)
       msg;
  Buffer.add_string buf (Pos.multiline_string pos);
  Buffer.add_string buf "\n\n";
  mark_with_context pos ~buf ~spans;
  Buffer.add_string buf "\n"

let add_span acc p =
  let file_checked = Pos.filename p in
  try
    let p = Pos.to_relative p in
    SMap.update
      file_checked
      (function
        | Some ps -> Some (p :: ps)
        | None -> Some [p])
      acc
  with
  | _ -> acc

let to_string
    User_error.{ code; claim = (pos, msg); reasons; explanation; severity; _ } =
  match explanation with
  | Explanation.Debug json ->
    (* Just render the json, no other info *)
    json
  | Explanation.Empty ->
    (* Just render the claim and reasons - only subtyping errors will have
       explanations at the moment *)
    let file_checked = Pos.filename pos in
    let init = SMap.singleton file_checked [Pos.to_relative pos] in
    let spans =
      SMap.map (fun ps ->
          merge @@ List.sort ps ~compare:(fun x y -> Pos.compare y x))
      @@ List.fold_left reasons ~init ~f:(fun acc (p, _) -> add_span acc p)
    in
    let buf = Buffer.create 500 in
    let (_ : unit) = render_claim buf spans code (pos, msg) severity in
    let (_ : unit) = List.iter reasons ~f:(render_reason buf spans) in
    Buffer.contents buf
  | Explanation.Derivation elems ->
    let file_checked = Pos.filename pos in
    let init =
      List.fold_left
        elems
        ~f:(fun acc elem ->
          match elem with
          | Explanation.Witness (p, _) -> add_span acc p
          | _ -> acc)
        ~init:(SMap.singleton file_checked [Pos.to_relative pos])
    in
    let spans =
      SMap.map (fun ps ->
          merge @@ List.sort ps ~compare:(fun x y -> Pos.compare y x))
      @@ List.fold_left reasons ~init ~f:(fun acc (p, _) -> add_span acc p)
    in
    let buf = Buffer.create 1000 in
    let (_ : unit) = render_claim buf spans code (pos, msg) severity in
    let (_ : unit) = List.iter reasons ~f:(render_reason buf spans) in
    let (_ : unit) = render_derivation buf spans elems in
    Buffer.contents buf
