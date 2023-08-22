(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type delimiter =
  | Backtick
  | Asterisk
  | DoubleAsterisk
  | DoubleTilde
[@@deriving ord]

module DelimiterKind = struct
  type t = delimiter

  let compare = compare_delimiter
end

module DelimiterSet = Caml.Set.Make (DelimiterKind)

type tagged_char = DelimiterSet.t * char

type parse_state = {
  parsed: tagged_char list;
  remaining: string;
}

type parser = parse_state -> parse_state option

let delimiter_to_string = function
  | Backtick -> "`"
  | Asterisk -> "*"
  | DoubleAsterisk -> "**"
  | DoubleTilde -> "~~"

let first_and_tail_exn s =
  let first = s.[0] in
  let tail = String.suffix s (String.length s - 1) in
  (first, tail)

(* Supports a small subset of Markdown, with some caveats
   in order to handle bolding the main message:

    *foo bar* for italics
    **foo bar** for bold
    `foo bar` for code (underlines text)
    ~~foo bar~~ for highlighting the text (in red by default)

   The `color` parameter is only used on characters
   tagged with DoubleTilde.
*)
let format_markdown
    ?(add_bold = false) ?(color = Tty.Red) (parsed : tagged_char list) =
  let format delimiters s : string =
    let styles =
      if DelimiterSet.mem Backtick delimiters then
        [`Underline]
      else
        []
    in
    let styles =
      (* Second condition: if we're supposed to bold the message anyway
         (e.g. in the main message), then add italics to it (and add bold later below) *)
      if
        DelimiterSet.mem Asterisk delimiters
        || (DelimiterSet.mem DoubleAsterisk delimiters && add_bold)
      then
        `Italics :: styles
      else
        styles
    in
    let styles =
      if DelimiterSet.mem DoubleAsterisk delimiters || add_bold then
        `Bold :: styles
      else
        styles
    in
    let color =
      if DelimiterSet.mem DoubleTilde delimiters then
        color
      else
        Tty.Default
    in
    if List.is_empty styles then
      s
    else
      Tty.apply_color_from_style (Tty.style_num_from_list color styles) s
  in
  List.group parsed ~break:(fun (set1, _) (set2, _) ->
      not (DelimiterSet.equal set1 set2))
  |> List.map ~f:(fun tagged_chars ->
         let (delimiters, _) = List.hd_exn tagged_chars in
         let s = List.map tagged_chars ~f:snd |> String.of_char_list in
         format delimiters s)
  |> String.concat

let eat_prefix (prefix : string) (state : parse_state) : parse_state option =
  let { parsed; remaining } = state in
  Option.Monad_infix.(
    String.chop_prefix remaining ~prefix >>= fun remaining ->
    Some { parsed; remaining })

(* Success if p s fails, returning original s *)
let fail (p : parser) (s : parse_state) =
  match p s with
  | None -> Some s
  | Some _ -> None

(* Try first parser; if it fails, try second with original state *)
let ( |? ) (p1 : parser) (p2 : parser) (s : parse_state) =
  match p1 s with
  | None -> p2 s
  | Some s -> Some s

let parse_single_char { parsed; remaining } =
  let (first, remaining) = first_and_tail_exn remaining in
  Some { parsed = parsed @ [(DelimiterSet.empty, first)]; remaining }

let rec parse_and_format_until (delimiter : delimiter) (state : parse_state) :
    parse_state option =
  let is_valid_section section =
    (* Example: "*foo bar*" is a valid section, while
       "**", "* foo bar*", "*foo bar *", and "* foo bar *" are not.
       The absence of a leading space is checked before entering this function. *)
    (not (List.is_empty section))
    && not (Char.equal ' ' (List.hd_exn section |> snd))
  in
  match eat_prefix (delimiter_to_string delimiter) state with
  | Some { parsed; remaining } when is_valid_section parsed ->
    (* Matched a delimiter over a non-empty section;
       now try and see if there are nested delimited sections
       in that larger section. *)
    let parsed =
      parse_markdown (List.map ~f:snd parsed |> String.of_char_list) []
      |> List.map
           ~f:
             (List.map ~f:(fun (delims, c) ->
                  (DelimiterSet.add delimiter delims, c)))
      |> List.concat
    in
    Some { parsed; remaining }
  | _ ->
    let { parsed; remaining } = state in
    (match remaining with
    | "" ->
      (* Nothing left to try *)
      None
    | _ ->
      let (first, remaining) = first_and_tail_exn remaining in
      (* Haven't matched a delimiter yet (or did, but the section
         wasn't valid), and we still have text we can try *)
      parse_and_format_until
        delimiter
        { parsed = (DelimiterSet.empty, first) :: parsed; remaining })

and parse_and_format_section ~(delimiter : delimiter) (s : parse_state) =
  Option.Monad_infix.(
    eat_prefix (delimiter_to_string delimiter) s
    >>= (eat_prefix " " |> fail)
    >>= parse_and_format_until delimiter)

and parse_markdown (msg : string) (acc : tagged_char list list) :
    tagged_char list list =
  if String.is_empty msg then
    List.rev acc
  else
    let parse =
      parse_and_format_section ~delimiter:DoubleAsterisk
      |? parse_and_format_section ~delimiter:DoubleTilde
      |? parse_and_format_section ~delimiter:Asterisk
      |? parse_and_format_section ~delimiter:Backtick
      |? parse_single_char
    in
    match parse { parsed = []; remaining = msg } with
    | None ->
      failwith "Impossible: unable to parse a single character in error message"
    | Some { parsed; remaining } ->
      parse_markdown remaining (List.rev parsed :: acc)

let render ?(add_bold = false) ?(color = Tty.Red) (msg : string) =
  parse_markdown msg [] |> List.concat |> format_markdown ~add_bold ~color

let md_codify s = "`" ^ s ^ "`"

let md_highlight s = "~~" ^ s ^ "~~"

let md_bold s = "**" ^ s ^ "**"

let md_italicize s = "*" ^ s ^ "*"
