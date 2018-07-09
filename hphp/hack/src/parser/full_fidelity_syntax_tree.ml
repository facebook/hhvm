(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
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
module SyntaxError = Full_fidelity_syntax_error
module TK = Full_fidelity_token_kind

module WithSyntax(Syntax : Syntax_sig.Syntax_S) = struct
module WithSmartConstructors(SCI : SmartConstructors.SmartConstructors_S
  with type r = Syntax.t
  with module Token = Syntax.Token
) = struct

module Parser_ = Full_fidelity_parser.WithSyntax(Syntax)
module Parser = Parser_.WithSmartConstructors(SCI)

open Syntax

type t = {
  text : SourceText.t;
  root : Syntax.t;
  errors : SyntaxError.t list;
  language : string;
  mode : string;
  state : SCI.t;
} [@@deriving show]

let parse_mode_comment s =
  let s = String.trim s in
  let len = String.length s in
  if len >= 2 && (String.get s 0) = '/' && (String.get s 1) = '/' then
    String.trim @@ String.sub s 2 (len - 2)
  else
    ""

let first_section script_declarations =
  match syntax script_declarations with
  | SyntaxList (h :: _) ->
    begin match syntax h with
    | MarkupSection ms -> (ms.markup_prefix, ms.markup_text, ms.markup_suffix)
    | _ -> failwith "unexpected: first element in a script should be markup"
    end
  | _ -> failwith "unexpected: script content should be list"

let analyze_header text script =
  let (markup_prefix, markup_text, markup_suffix) = first_section script in
  match syntax markup_suffix with
  | MarkupSuffix {
    markup_suffix_less_than_question;
    markup_suffix_name; _ } ->
    begin match syntax markup_suffix_name with
    | Missing -> "php", ""
    | Token t when Token.kind t = TK.Equal -> "php", ""
    | _ ->
      let prefix_width = full_width markup_prefix in
      let text_width = full_width markup_text in
      let ltq_width = full_width markup_suffix_less_than_question in
      let name_leading = leading_width markup_suffix_name in
      let name_width = Syntax.width markup_suffix_name in
      let name_trailing = trailing_width markup_suffix_name in
      let language = SourceText.sub text (prefix_width + text_width +
        ltq_width + name_leading) name_width
      in
      let language = String.lowercase_ascii language in
      let mode = SourceText.sub text (prefix_width + text_width +
        ltq_width + name_leading + name_width) name_trailing
      in
      let mode = parse_mode_comment mode in
      language, mode
    end
  | _ -> "php", ""
  (* The parser never produces a leading markup section; it fills one in with zero
     width tokens if it needs to. *)

let get_language_and_mode text root =
  match syntax root with
  | Script s -> analyze_header text s.script_declarations
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

let build
    (text: SourceText.t)
    (root: Syntax.t)
    (errors: SyntaxError.t list)
    (language: string)
    (mode: string)
    (state: SCI.t): t =
  { text; root; errors; language; mode; state }

let process_errors errors =
  (* We've got the lexical errors and the parser errors together, both
  with lexically later errors earlier in the list. We want to reverse the
  list so that later errors come later, and then do a stable sort to put the
  lexical and parser errors together. *)
  let errors = List.rev errors in
  let errors = List.stable_sort SyntaxError.compare errors in
  remove_duplicates errors SyntaxError.exactly_equal

let from_root text root errors state =
  let errors = process_errors errors in
  let (language, mode) = get_language_and_mode text root in
  build text root errors language mode state

let create text root errors lang mode state =
  let errors = process_errors errors in
  let language = FileInfo.string_of_file_type lang in
  let mode = FileInfo.string_of_mode @@ Option.value ~default:FileInfo.Mphp mode in
  { text; root; errors; language; mode; state }

let make_impl ?(env = Env.default) text =
  let parser = Parser.make env text in
  let (parser, root) = Parser.parse_script parser in
  let errors = Parser.errors parser in
  let state = Parser.sc_state parser in
  from_root text root errors state

let make ?(env = Env.default) text =
  Stats_container.wrap_nullary_fn_timing
    ?stats:(Env.stats env)
    ~key:"Syntax_tree.make"
    ~f:(fun () -> make_impl ~env text)

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

let sc_state tree =
  tree.state

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

let to_json ?with_value tree =
  let version = Full_fidelity_schema.full_fidelity_schema_version_number in
  let root = Syntax.to_json ?with_value tree.root in
  let text = Hh_json.JSON_String (SourceText.text tree.text) in
  Hh_json.JSON_Object [
    "parse_tree", root;
    "program_text", text;
    "version", Hh_json.JSON_String version
  ]
end (* WithSmartConstructors *)

include WithSmartConstructors(SyntaxSmartConstructors.WithSyntax(Syntax))

let from_root text root errors =
  from_root text root errors ()

let create text root errors lang mode =
  create text root errors lang mode ()

let build text root errors language mode =
  build text root errors language mode ()

end (* WithSyntax *)
