(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(**
 * A syntax tree is just a thin wrapper around all the output of the parser:
 * the source text that was parsed, the root of the parse tree, and a
 * collection of parser and lexer errors.
 *
 * "Making" a syntax tree from text parses the text.
 *
 *)

module SourceText = Full_fidelity_source_text
module Env = Full_fidelity_parser_env
module Parser = Minimal_parser
module SyntaxError = Full_fidelity_syntax_error
module TK = Full_fidelity_token_kind
open Full_fidelity_minimal_syntax

type t = {
  text : SourceText.t;
  root : Full_fidelity_minimal_syntax.t;
  errors : SyntaxError.t list;
  language : string;
  mode : string
}

let strip_comment_start s =
  let len = String.length s in
  if len >= 2 && (String.get s 0) = '/' && (String.get s 1) = '/' then
    String.sub s 2 (len - 2)
  else
    s

let analyze_header text script =
  match syntax script.script_declarations with
  | SyntaxList ({
      syntax = MarkupSection { markup_prefix; markup_text; markup_suffix; _ }
    ; _}::_) ->
    begin match syntax markup_suffix with
    | MarkupSuffix {
        markup_suffix_name = {
          syntax = Missing | Token { Token.kind = TK.Equal; _ }
        ; _ }
      ; _ } -> "php", ""
    | MarkupSuffix {
        markup_suffix_less_than_question = ltq;
        markup_suffix_name = name;
        _
      } ->
      let prefix_width = full_width markup_prefix in
      let text_width = full_width markup_text in
      let ltq_width = full_width ltq in
      let name_leading = leading_width name in
      let name_width = width name in
      let name_trailing = trailing_width name in
      let language = SourceText.sub text (prefix_width + text_width +
        ltq_width + name_leading) name_width
      in
      let language = String.lowercase_ascii language in
      let mode = SourceText.sub text (prefix_width + text_width +
        ltq_width + name_leading + name_width) name_trailing
      in
      let mode = String.trim mode in
      let mode = strip_comment_start mode in
      let mode = String.trim mode in
      language, mode
    | _ -> "php", ""
    end
  | _ -> failwith "unexpected: script content should be list"
  (* The parser never produces a leading markup section; it fills one in with zero
     width tokens if it needs to. *)

let get_language_and_mode text root =
  match syntax root with
  | Script s -> analyze_header text s
  | _ -> failwith "unexpected missing script node"
    (* The parser never produces a missing script, even if the file is empty *)

let remove_duplicates errors equals =
  (* Assumes the list is sorted so that equal items are together. *)
  let rec aux errors acc =
    match errors with
    | [] -> acc
    | h1 :: t1 ->
      begin
        match t1 with
        | [] -> h1 :: acc
        | h2 :: t2 ->
        if equals h1 h2 then
          aux (h1 :: t2) acc
        else
          aux t1 (h1 :: acc)
      end in
  let result = aux errors [] in
  List.rev result

let make ?(env = Env.default) text =
  let parser = Parser.make env text in
  let (parser, root) = Parser.parse_script parser in
  (* We've got the lexical errors and the parser errors together, both
  with lexically later errors earlier in the list. We want to reverse the
  list so that later errors come later, and then do a stable sort to put the
  lexical and parser errors together. *)
  let errors = Parser.errors parser in
  let errors = List.rev errors in
  let errors = List.stable_sort SyntaxError.compare errors in
  let errors = remove_duplicates errors SyntaxError.exactly_equal in
  let (language, mode) = get_language_and_mode text root in
  { text; root; errors; language; mode }

let root tree =
  tree.root

let text tree =
  tree.text

let all_errors tree =
  tree.errors

let remove_cascading errors =
  let equals e1 e2 = (SyntaxError.compare e1 e2) = 0 in
  remove_duplicates errors equals

let language tree =
  tree.language

let mode tree =
  tree.mode

let is_hack tree =
  tree.language = "hh"

let is_php tree =
  tree.language = "php"

let is_strict tree =
  (is_hack tree) && tree.mode = "strict"

let is_decl tree =
  (is_hack tree) && tree.mode = "decl"

let errors_no_bodies tree =
  let not_in_body error =
    not (is_in_body tree.root error.SyntaxError.start_offset) in
  List.filter not_in_body tree.errors

(* By default we strip out (1) all cascading errors, and (2) in decl mode,
all errors that happen in a body. *)

let errors tree =
  let e =
    if is_decl tree then begin
      errors_no_bodies tree end
    else
      all_errors tree in
  remove_cascading e

let to_json tree =
  let version = Full_fidelity_schema.full_fidelity_schema_version_number in
  let root = to_json tree.root in
  let text = Hh_json.JSON_String (SourceText.text tree.text) in
  Hh_json.JSON_Object [
    "parse_tree", root;
    "program_text", text;
    "version", Hh_json.JSON_String version
  ]
