(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module SyntaxError = Full_fidelity_syntax_error
module SyntaxTree = Full_fidelity_syntax_tree
module SourceText = Full_fidelity_source_text

let usage = Printf.sprintf "Usage: %s filename\n" Sys.argv.(0)

let print_error error = error
  |> Errors.to_absolute
  |> Errors.to_string
  |> output_string stderr

let parse_and_print filename =
  let file = Relative_path.create Relative_path.Dummy filename in
  let source_file = SourceText.from_file file in
  let syntax_tree = SyntaxTree.make source_file in

  let errors = SyntaxTree.errors syntax_tree in
  (* TODO: Errors do not know positions, just offsets. *)
  let printer err = Printf.printf "%s\n" (SyntaxError.to_string err) in
  let str = Debug.dump_full_fidelity syntax_tree in
  let module AstGenGrammar = Hack_grammar_descriptor.HackGrammar in
  let module AstGen = Random_ast_generator.Make(AstGenGrammar) in
  let gen_str = AstGen.generate 30 in
  Printf.printf "gen: \n %s \n %!"  gen_str;
  Printf.printf "----\n";
  let gen_source = SourceText.make gen_str in
  let gen_syntax_tree = SyntaxTree.make gen_source in
  let gen_errors = SyntaxTree.errors gen_syntax_tree in
  let gen_ast_str = Debug.dump_full_fidelity gen_syntax_tree in
  Printf.printf "generated parse: \n";
  List.iter printer gen_errors;
  Printf.printf "%s" gen_ast_str;
  Printf.printf "\n----\n";
  Printf.printf "parse:\n";
  List.iter printer errors;
  Printf.printf "%s" str;
  Printf.printf "\n----\n";
  let errorl, result =
    Errors.do_ begin fun () ->
      Parser_hack.from_file file
    end
  in
  if not (Errors.is_empty errorl) then begin
    Errors.iter_error_list print_error errorl;
    exit 1
  end;
  let str = Debug.dump_ast (Ast.AProgram result.Parser_hack.ast) in
  Printf.printf "%s" str

let main filename =
  EventLogger.init (Daemon.devnull ()) 0.0;
  let _ = SharedMem.(init default_config) in
  if (String.length filename) = 0 then begin
    Printf.eprintf "%s" usage;
    exit 1
  end;
  Unix.handle_unix_error parse_and_print filename

let () =
  Arg.parse [] main usage;
