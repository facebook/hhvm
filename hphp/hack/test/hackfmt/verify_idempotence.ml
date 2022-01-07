(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** hackfmt promises that on a file with no syntax errors, it will not add,
    remove, or modify tokens except for trailing commas. This binary verifies
    that this is the case after formatting once, and also ensures that the
    formatting does not change after formatting a second time. *)

module EditableSyntax = Full_fidelity_editable_syntax
module ParserErrors =
  Full_fidelity_parser_errors.WithSyntax (Full_fidelity_positioned_syntax)
module PositionedSyntaxTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)
module SourceText = Full_fidelity_source_text
module SyntaxError = Full_fidelity_syntax_error
module Token = Full_fidelity_editable_token
module TokenKind = Full_fidelity_token_kind

let parse args filename text =
  let popt = FullFidelityParseArgs.to_parser_options args in
  (* Parse with the full fidelity parser *)
  let file = Relative_path.create Relative_path.Dummy filename in
  let source_text = SourceText.make file text in
  let mode = Full_fidelity_parser.parse_mode source_text in
  let env =
    FullFidelityParseArgs.to_parser_env ~leak_rust_tree:true ~mode args
  in
  let syntax_tree = PositionedSyntaxTree.make ~env source_text in
  let error_env =
    ParserErrors.make_env
      syntax_tree
      ~level:ParserErrors.Maximum
      ~hhvm_compat_mode:ParserErrors.NoCompat
      ~codegen:false
      ~parser_options:popt
  in
  let errors = ParserErrors.parse_errors error_env in
  (syntax_tree, errors)

(* Prints a single FFP error. *)
let print_full_fidelity_error source_text error =
  let text =
    SyntaxError.to_positioned_string
      error
      (SourceText.offset_to_position source_text)
  in
  Printf.printf "%s\n" text

let filename_of_tree syntax_tree =
  PositionedSyntaxTree.text syntax_tree
  |> SourceText.file_path
  |> Relative_path.to_absolute

let text_of_tree syntax_tree =
  PositionedSyntaxTree.text syntax_tree |> SourceText.text

let get_non_comma_tokens tree =
  SyntaxTransforms.editable_from_positioned tree
  |> EditableSyntax.all_tokens
  |> List.filter (fun t ->
         match Token.kind t with
         | TokenKind.Comma -> false
         | _ -> true)

let dump_texts msg syntax_tree formatted_tree =
  let filename = filename_of_tree syntax_tree in
  let text = text_of_tree syntax_tree in
  let formatted_text = text_of_tree formatted_tree in
  Printf.printf "\n%s AFTER FORMATTING: %s\n\n" msg filename;
  Printf.printf
    "ORIGINAL FILE **************************************************\n";
  Printf.printf "%s\n" text;
  Printf.printf
    "FORMATTED ******************************************************\n";
  Printf.printf "%s\n" formatted_text

(** Return true if the tokens are equal for the purpose of this test
    (i.e., ignoring trivia). *)
let tokens_equal token1 token2 =
  let kind1 = Token.kind token1 in
  let kind2 = Token.kind token2 in
  let text1 = Token.text token1 in
  let text2 = Token.text token2 in
  (* Whitespace at the start of the file is treated as part of the Hashbang
     token instead of trivia, so do not consider the text of Hashbang tokens. *)
  TokenKind.equal kind1 kind2
  && (String.equal text1 text2 || TokenKind.(equal kind1 Hashbang))

let assert_tokens_equal syntax_tree formatted_tree =
  let tokens = get_non_comma_tokens syntax_tree in
  let formatted_tokens = get_non_comma_tokens formatted_tree in
  let lengths_differ = List.compare_lengths tokens formatted_tokens <> 0 in
  let print_error_and_die () =
    dump_texts "TOKEN STREAM CHANGED" syntax_tree formatted_tree;
    exit 1
  in
  if lengths_differ then print_error_and_die ();
  List.iter2
    (fun a b -> if not (tokens_equal a b) then print_error_and_die ())
    tokens
    formatted_tokens

let assert_no_errors_after_formatting syntax_tree formatted_tree errors =
  if List.length errors != 0 then (
    dump_texts "SYNTAX ERRORS ADDED" syntax_tree formatted_tree;
    Printf.printf
      "ERRORS *********************************************************\n";
    let source_text = PositionedSyntaxTree.text formatted_tree in
    List.iter (print_full_fidelity_error source_text) errors;
    exit 1
  )

let assert_reformatting_idempotent filename formatted_text reformatted_text =
  if not (String.equal formatted_text reformatted_text) then (
    Printf.printf "\nFORMATTING NOT IDEMPOTENT ON: %s\n\n" filename;
    Printf.printf
      "Formatting this file twice produced different results the first and second time.\n";
    Printf.printf
      "hackfmt is expected to be idempotent, and produce no changes on formatted files.\n\n";
    Printf.printf
      "FORMATTED ONCE *************************************************\n";
    Printf.printf "%s\n" formatted_text;
    Printf.printf
      "FORMATTED TWICE ************************************************\n";
    Printf.printf "%s\n" reformatted_text;
    exit 1
  )

(** Parse the given file. If it has no syntax errors, format it using hackfmt.
    Assert that the formatted file still contains no syntax errors, its token
    stream is the same as the input file (except for commas and trivia), and
    that formatting a second time produces the same output as the first time. *)
let verify_hackfmt_idempotence args filename =
  let text = Sys_utils.cat filename in
  let (syntax_tree, errors) = parse args filename text in
  if List.length errors = 0 then (
    try
      let formatted_text = Libhackfmt.format_tree syntax_tree in
      let (formatted_tree, errors) = parse args filename formatted_text in
      assert_no_errors_after_formatting syntax_tree formatted_tree errors;
      assert_tokens_equal syntax_tree formatted_tree;
      let reformatted_text = Libhackfmt.format_tree formatted_tree in
      assert_reformatting_idempotent filename formatted_text reformatted_text
    with
    | exn ->
      let backtrace = Printexc.get_backtrace () in
      let msg = Printexc.to_string exn in
      Printf.printf
        "\nEXCEPTION WHILE FORMATTING: %s\n\n%s\n\n%s\n"
        filename
        msg
        backtrace;
      exit 1
  )

let handle_file args filename =
  if Path.file_exists (Path.make filename) then
    verify_hackfmt_idempotence args filename
  else
    Printf.printf "File %s does not exist.\n" filename

let main args files =
  List.iter (Unix.handle_unix_error (handle_file args)) files

let () =
  let args = FullFidelityParseArgs.parse_args () in
  EventLogger.init_fake ();
  main args args.FullFidelityParseArgs.files
