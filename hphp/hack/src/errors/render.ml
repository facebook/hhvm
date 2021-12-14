(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let strip_ns id = id |> Utils.strip_ns |> Hh_autoimport.reverse_type

let vis_to_string = function
  | `public -> "public"
  | `private_ -> "private"
  | `internal -> "internal"
  | `protected -> "protected"

(* Given two equal-length strings, highlights the characters in
   the second that differ from the first *)
let highlight_differences base to_highlight =
  match List.zip (String.to_list base) (String.to_list to_highlight) with
  | List.Or_unequal_lengths.Ok l ->
    List.group l ~break:(fun (o1, s1) (o2, s2) ->
        not (Bool.equal (Char.equal o1 s1) (Char.equal o2 s2)))
    |> List.map ~f:(fun cs ->
           let s = List.map cs ~f:snd |> String.of_char_list in
           let (c1, c2) = List.hd_exn cs in
           if Char.equal c1 c2 then
             s
           else
             Markdown_lite.md_highlight s)
    |> String.concat
  | List.Or_unequal_lengths.Unequal_lengths -> to_highlight

let suggestion_message ?(modifier = "") orig hint hint_pos =
  let s =
    if
      (not (String.equal orig hint))
      && String.equal (String.lowercase orig) (String.lowercase hint)
    then
      Printf.sprintf
        "Did you mean %s%s instead (which only differs by case)?"
        modifier
        (highlight_differences orig hint |> Markdown_lite.md_codify)
    else
      Printf.sprintf
        "Did you mean %s%s instead?" (* here? *)
        modifier
        (Markdown_lite.md_codify hint)
  in
  (hint_pos, s)
