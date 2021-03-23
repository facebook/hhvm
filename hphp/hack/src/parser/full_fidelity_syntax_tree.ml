(*
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

open Hh_prelude
module SourceText = Full_fidelity_source_text
module Env = Full_fidelity_parser_env
module SyntaxError = Full_fidelity_syntax_error

module WithSyntax (Syntax : Syntax_sig.Syntax_S) = struct
  module WithSmartConstructors
      (SCI : SmartConstructors.SmartConstructors_S
               with type r = Syntax.t
               with module Token = Syntax.Token) =
  struct
    module Parser_ = Full_fidelity_parser.WithSyntax (Syntax)
    module Parser = Parser_.WithSmartConstructors (SCI)
    open Syntax

    type t = {
      text: SourceText.t;
      root: Syntax.t;
      rust_tree: Rust_pointer.t option;
      errors: SyntaxError.t list;
      mode: FileInfo.mode option;
      state: SCI.t;
    }
    [@@deriving show]

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
          end
      in
      let result = aux errors [] in
      List.rev result

    let build
        (text : SourceText.t)
        (root : Syntax.t)
        (rust_tree : Rust_pointer.t option)
        (errors : SyntaxError.t list)
        (mode : FileInfo.mode option)
        (state : SCI.t) : t =
      { text; root; rust_tree; errors; mode; state }

    let process_errors errors =
      (* We've got the lexical errors and the parser errors together, both
  with lexically later errors earlier in the list. We want to reverse the
  list so that later errors come later, and then do a stable sort to put the
  lexical and parser errors together. *)
      let errors = List.rev errors in
      let errors = List.stable_sort ~compare:SyntaxError.compare errors in
      remove_duplicates errors SyntaxError.exactly_equal

    let create text root rust_tree errors mode state =
      let errors = process_errors errors in
      build text root rust_tree errors mode state

    let make ?(env = Env.default) text =
      let mode = Full_fidelity_parser.parse_mode text in
      let parser = Parser.make env text in
      let (parser, root, rust_tree) = Parser.parse_script parser in
      let errors = Parser.errors parser in
      let state = Parser.sc_state parser in
      create text root rust_tree errors mode state

    let root tree = tree.root

    let rust_tree tree = tree.rust_tree

    let text tree = tree.text

    let all_errors tree = tree.errors

    let remove_cascading errors =
      let equals e1 e2 = SyntaxError.compare e1 e2 = 0 in
      remove_duplicates errors equals

    let mode tree = tree.mode

    let sc_state tree = tree.state

    let is_strict tree =
      match tree.mode with
      | Some mode -> FileInfo.is_strict mode
      | None -> false

    let is_hhi tree =
      match tree.mode with
      | Some FileInfo.Mhhi -> true
      | _ -> false

    let errors_no_bodies tree =
      let not_in_body error =
        not (is_in_body tree.root error.SyntaxError.start_offset)
      in
      List.filter ~f:not_in_body tree.errors

    (* By default we strip out (1) all cascading errors, and (2) in decl mode,
all errors that happen in a body. *)

    let errors tree =
      let e =
        if is_hhi tree then
          errors_no_bodies tree
        else
          all_errors tree
      in
      remove_cascading e

    let parse_tree_to_json ?with_value ?ignore_missing tree =
      Syntax.to_json ?with_value ?ignore_missing tree.root

    let to_json ?with_value ?ignore_missing tree =
      let version = Full_fidelity_schema.full_fidelity_schema_version_number in
      let root = parse_tree_to_json ?with_value ?ignore_missing tree in
      let text = Hh_json.JSON_String (SourceText.text tree.text) in
      Hh_json.JSON_Object
        [
          ("parse_tree", root);
          ("program_text", text);
          ("version", Hh_json.JSON_String version);
        ]
  end

  include WithSmartConstructors (SyntaxSmartConstructors.WithSyntax (Syntax))

  let create text root errors mode = create text root None errors mode ()

  let build text root errors mode = build text root None errors mode ()
end
