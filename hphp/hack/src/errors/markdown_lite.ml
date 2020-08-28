(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type parse_state = string * string

type parser = parse_state -> parse_state option

type delimiter =
  | Backtick
  | Asterisk
  | DoubleAsterisk
  | DoubleTilde

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
    ~~foo bar~~ for highlighting the text in red
*)
let format_markdown_section
    delimiter ?(add_bold = false) ?(color = Tty.Red) section =
  let style =
    match delimiter with
    | Backtick ->
      if add_bold then
        Tty.BoldUnderline Tty.Default
      else
        Tty.Underline Tty.Default
    | Asterisk ->
      if add_bold then
        Tty.BoldItalics Tty.Default
      else
        Tty.Italics Tty.Default
    | DoubleAsterisk ->
      (* If we're in the main message which will be entirely bolded,
        then bold italicize it instead *)
      if add_bold then
        Tty.BoldItalics Tty.Default
      else
        Tty.Bold Tty.Default
    | DoubleTilde ->
      if add_bold then
        Tty.Bold color
      else
        Tty.Normal color
  in
  Tty.apply_color style section

let eat_prefix (prefix : string) (state : parse_state) : parse_state option =
  let (parsed, rest) = state in
  Option.Monad_infix.(
    String.chop_prefix rest prefix >>= fun s -> Some (parsed, s))

let rec parse_and_format_until
    (delimiter : delimiter)
    ?(add_bold = false)
    ?(color = Tty.Red)
    (state : parse_state) : parse_state option =
  let is_valid_section section =
    (* Example: "*foo bar*" is a valid section, while
       "**", "* foo bar*", "*foo bar *", and "* foo bar *" are not.
       The absence of a leading space is checked before entering this function. *)
    String.length section > 0 && not (String.is_suffix ~suffix:" " section)
  in
  match eat_prefix (delimiter_to_string delimiter) state with
  | Some (parsed, rest) when is_valid_section parsed ->
    (* Matched a delimiter over a non-empty section *)
    Some (format_markdown_section delimiter ~add_bold ~color parsed, rest)
  | _ ->
    let (parsed, rest) = state in
    (match rest with
    | "" ->
      (* Nothing left to try *)
      None
    | _ ->
      let (first, tail) = first_and_tail_exn rest in
      (* Haven't matched a delimiter yet (or did, but the section
         wasn't valid), and we still have text we can try *)
      parse_and_format_until
        delimiter
        ~add_bold
        ~color
        (parsed ^ String.of_char first, tail))

(* Success if p s fails, returning original s *)
let fail (p : parser) (s : parse_state) =
  match p s with
  | None -> Some s
  | Some _ -> None

let parse_and_format_section
    ~(delimiter : delimiter)
    ~(add_bold : bool)
    ~(color : Tty.raw_color)
    (s : parse_state) =
  Option.Monad_infix.(
    eat_prefix (delimiter_to_string delimiter) s
    >>= (eat_prefix " " |> fail)
    >>= parse_and_format_until delimiter ~add_bold ~color)

(* Try first parser; if it fails, try second with original state *)
let ( |? ) (p1 : parser) (p2 : parser) (s : parse_state) =
  match p1 s with
  | None -> p2 s
  | Some s -> Some s

let parse_single_char ~add_bold (parsed, rest) =
  let formatter c =
    if add_bold then
      Tty.apply_color (Tty.Bold Tty.Default) c
    else
      c
  in
  let (first, tail) = first_and_tail_exn rest in
  let c = String.of_char first |> formatter in
  Some (parsed ^ c, tail)

let rec render ?(add_bold = false) ?(color = Tty.Red) (msg : string) =
  if String.is_empty msg then
    ""
  else
    let parse =
      parse_and_format_section ~delimiter:DoubleAsterisk ~add_bold ~color
      |? parse_and_format_section ~delimiter:DoubleTilde ~add_bold ~color
      |? parse_and_format_section ~delimiter:Asterisk ~add_bold ~color
      |? parse_and_format_section ~delimiter:Backtick ~add_bold ~color
      |? parse_single_char ~add_bold
    in
    match parse ("", msg) with
    | None ->
      failwith "Impossible: unable to parse a single character in error message"
    | Some (parsed, rest) -> parsed ^ render ~add_bold ~color rest

let md_codify s = "`" ^ s ^ "`"

let md_highlight s = "~~" ^ s ^ "~~"

let md_bold s = "**" ^ s ^ "**"

let md_italicize s = "*" ^ s ^ "*"
