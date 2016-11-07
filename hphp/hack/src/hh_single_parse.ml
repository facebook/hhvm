(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

let purpose = "Read a single Hack file and produce the resulting S-Expression."
let extra   = "(Options for development / parser selection and comparisson.)"
let usage   = Printf.sprintf
  "Usage: %s <options> filename\n%s\n%s"
  Sys.argv.(0)
  purpose
  extra

type parser_return = Parser_hack.parser_return
type result =
  | ParseError   of Errors.t
  | CmpDifferent of Relative_path.t * string * string
  | Result       of parser_return
  | Unsupported  of string * parser_return

let process_result : string -> result -> parser_return = fun use_diff ->
  let open Printf in
  function
  | Result r -> r
  | ParseError errorl ->
      let print_error error = error
        |> Errors.to_absolute
        |> Errors.to_string
        |> output_string stderr
      in
      Errors.iter_error_list print_error errorl;
      exit 1
  | Unsupported (s, r) ->
      eprintf "Warning: Unsupported features found: %s" s;
      exit 2
  | CmpDifferent (path, oldSExpr, newSExpr) ->
      let open Unix in
      let filename = Relative_path.S.to_string path in
      let mkTemp (name : string) (content : string) = begin
        let ic = open_process_in (sprintf "mktemp tmp.%s.XXXXXXXX" name) in
        let path = input_line ic in
        ignore (close_process_in ic);
        let oc = open_out path in
        fprintf oc "%s\n\n%s\n\n%s\n" filename content filename;
        close_out oc;
        path
      end in

      let pathOld = mkTemp "OLD" oldSExpr in
      let pathNew = mkTemp "NEW" newSExpr in
      eprintf "\n\n****** Different\n";
      eprintf "  Filename:     %s\n" filename;
      eprintf "  AST output:   %s\n" pathOld;
      eprintf "  FFP output:   %s\n" pathNew;
      eprintf "  Diff command: %s\n" use_diff;
      flush Pervasives.stderr;
      let command = sprintf "%s %s %s" use_diff pathOld pathNew in
      ignore (system command);
      ignore (unlink pathOld);
      ignore (unlink pathNew);
      exit 42

let ast_parser : Relative_path.t -> result = fun file ->
  let parse_call () = Parser_hack.from_file ParserOptions.default file in
  let errorl, result, _ = Errors.do_ parse_call in
  if Errors.is_empty errorl
  then Result result
  else ParseError errorl

let full_fidelity_parser : Relative_path.t -> result = fun file ->
  let module Tree = Full_fidelity_syntax_tree in
  let module Text = Full_fidelity_source_text in
  let open Parser_hack in
  let source_text = Text.from_file file in
  let syntax_tree = Tree.make source_text in
  let mode = match Tree.mode syntax_tree with
    | "strict" -> FileInfo.Mstrict
    | "decl"   -> FileInfo.Mdecl
    | _        -> FileInfo.Mpartial
  in
  Result
  { file_mode = Option.some_if (Tree.language syntax_tree = "hh") mode
  ; comments  = []
  ; ast       = Namespaces.elaborate_defs ParserOptions.default @@
                  Classic_ast_mapper.from_tree file syntax_tree
  ; content   = Text.text source_text
  }

let compare_parsers file =
  let open Parser_hack in
  let ast = process_result "echo" @@ ast_parser file in
  let ffp = process_result "echo" @@ full_fidelity_parser file in
  let ast_sexpr = Debug.dump_ast (Ast.AProgram ast.ast) in
  let ffp_sexpr = Debug.dump_ast (Ast.AProgram ffp.ast) in
  if ast_sexpr = ffp_sexpr
  then Result ast
  else
    let unsafe = Str.regexp_string "Unsafe" in
    if try Str.search_forward unsafe ast_sexpr 0 >= 0 with | _ -> false
    then Unsupported ("Unsafe", ast)
    else CmpDifferent (file, ast_sexpr, ffp_sexpr)

let parse_and_print parser run_diff filename =
  let file = Relative_path.create Relative_path.Dummy filename in
  let result = process_result run_diff @@ parser file in
  Printf.printf "%s" @@ Debug.dump_ast @@ Ast.AProgram result.Parser_hack.ast

let () =
  let use_parser = ref "ast"     in
  let use_diff   = ref "vimdiff" in
  let filename   = ref ""        in
  Arg.(parse
    [ ("--parser", Set_string use_parser,
        "Which parser to use (ast, ffp, compare) [def: ast]"
      )
    ; ("--diff", Set_string use_diff,
        "Which diff tool to compare different S-Expressions with [def: vimdiff]"
      )
    ]) (fun fn -> filename := fn) usage;
  let parse_function = match !use_parser with
    | "ast"      -> ast_parser
    | "ffp"      -> full_fidelity_parser
    | "compare"  -> compare_parsers
    | s -> raise (Failure (Printf.sprintf "Unknown parser '%s'\n" s))
  in
  if String.length !filename = 0 then raise (Failure "No filename given");
  EventLogger.init EventLogger.Event_logger_fake 0.0;
  let _handle = SharedMem.init GlobalConfig.default_sharedmem_config in
  Unix.handle_unix_error (parse_and_print parse_function !use_diff) !filename

