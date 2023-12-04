(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
  *
  * Usage: hh_parse [OPTIONS] [FILES]
  *
  * --full-fidelity-json
  * --full-fidelity-json-parse-tree
  * --full-fidelity-errors
  * --full-fidelity-errors-all
  * --full-fidelity-ast-s-expression
  * --program-text
  * --pretty-print
  * --pretty-print-json
  * --show-file-name
  *
  * TODO: Parser for things other than scripts:
  *       types, expressions, statements, declarations, etc.
  *)

module Schema = Full_fidelity_schema
module SyntaxError = Full_fidelity_syntax_error
module SyntaxTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)
module SourceText = Full_fidelity_source_text
module ParserErrors =
  Full_fidelity_parser_errors.WithSyntax (Full_fidelity_positioned_syntax)
open FullFidelityParseArgs

(* Prints a single FFP error. *)
let print_full_fidelity_error source_text error =
  let text =
    SyntaxError.to_positioned_string
      error
      (SourceText.offset_to_position source_text)
  in
  Printf.printf "%s\n" text

let print_ast_check_errors errors =
  let error_list = Errors.get_error_list errors in
  List.iter
    (fun e ->
      let text = Errors.to_string (User_error.to_absolute e) in
      if Core.String.is_substring text ~substring:SyntaxError.this_in_static
      then
        Printf.eprintf "%s\n%!" text)
    error_list

let handle_existing_file args filename =
  let popt = FullFidelityParseArgs.to_parser_options args in
  (* Parse with the full fidelity parser *)
  let file = Relative_path.create Relative_path.Dummy filename in
  let source_text = SourceText.from_file file in
  let mode = Full_fidelity_parser.parse_mode source_text in
  let print_errors =
    args.codegen || args.full_fidelity_errors || args.full_fidelity_errors_all
  in
  let env =
    FullFidelityParseArgs.to_parser_env
    (* When print_errors is true, the leaked tree will be passed to ParserErrors,
     * which will consume it. *)
      ~leak_rust_tree:print_errors
      ~mode
      args
  in
  let syntax_tree = SyntaxTree.make ~env source_text in
  let editable = SyntaxTransforms.editable_from_positioned syntax_tree in
  if args.show_file_name then Printf.printf "%s\n" filename;
  (if args.program_text then
    let text = Full_fidelity_editable_syntax.text editable in
    Printf.printf "%s\n" text);
  (if args.pretty_print then
    let pretty = Libhackfmt.format_tree syntax_tree in
    Printf.printf "%s\n" pretty);
  (if args.generate_hhi then
    let hhi = Generate_hhi.go editable in
    Printf.printf "%s\n" hhi);

  (if print_errors then
    let level =
      if args.full_fidelity_errors_all then
        ParserErrors.Maximum
      else
        ParserErrors.Typical
    in
    let hhvm_compat_mode =
      if args.codegen then
        ParserErrors.HHVMCompat
      else
        ParserErrors.NoCompat
    in
    let error_env =
      ParserErrors.make_env
        syntax_tree
        ~level
        ~hhvm_compat_mode
        ~codegen:args.codegen
        ~parser_options:popt
    in
    let errors = ParserErrors.parse_errors error_env in
    List.iter (print_full_fidelity_error source_text) errors);
  begin
    let dump_needed = args.full_fidelity_ast_s_expr in
    let lowered =
      if dump_needed || print_errors then (
        let env =
          Full_fidelity_ast.make_env
            ~codegen:args.codegen
            ~php5_compat_mode:args.php5_compat_mode
            ~elaborate_namespaces:args.elaborate_namespaces
            ~include_line_comments:args.include_line_comments
            ~quick_mode:args.quick_mode
            ~parser_options:popt
            file
        in
        try
          let (errors, res) =
            Errors.do_ (fun () -> Full_fidelity_ast.from_file_with_legacy env)
          in
          if print_errors then print_ast_check_errors errors;
          Some res
        with
        | e when print_errors ->
          let err = Base.Exn.to_string e in
          let fn = Relative_path.suffix file in
          (* If we've already found a parsing error, it's okay for lowering to fail *)
          if not (Errors.currently_has_errors ()) then
            Printf.eprintf
              "Warning, lowering failed for %s\n  - error: %s\n"
              fn
              err;

          None
      ) else
        None
    in
    match lowered with
    | Some res ->
      if dump_needed then
        let ast = res.Parser_return.ast in
        let str = Nast.show_program ast in
        Printf.printf "%s\n" str
    | None -> ()
  end;
  (if args.full_fidelity_json_parse_tree then
    let json =
      SyntaxTree.parse_tree_to_json
        ~ignore_missing:args.ignore_missing_json
        syntax_tree
    in
    let str = Hh_json.json_to_string json ~pretty:args.pretty_print_json in
    Printf.printf "%s\n" str);
  (if args.full_fidelity_json then
    let json =
      SyntaxTree.to_json ~ignore_missing:args.ignore_missing_json syntax_tree
    in
    let str = Hh_json.json_to_string json ~pretty:args.pretty_print_json in
    Printf.printf "%s\n" str);
  (if args.full_fidelity_text_json then
    let json = Full_fidelity_editable_syntax.to_json editable in
    let str = Hh_json.json_to_string json in
    Printf.printf "%s\n" str);
  (if args.full_fidelity_dot then
    let dot = Full_fidelity_editable_syntax.to_dot editable false in
    Printf.printf "%s\n" dot);
  if args.full_fidelity_dot_edges then
    let dot = Full_fidelity_editable_syntax.to_dot editable true in
    Printf.printf "%s\n" dot

let handle_file args filename =
  if Path.file_exists (Path.make filename) then
    handle_existing_file args filename
  else
    Printf.printf "File %s does not exist.\n" filename

let rec main args files =
  (if args.schema then
    let schema = Schema.schema_as_json () in
    Printf.printf "%s\n" schema);
  match files with
  | [] -> ()
  | file :: tail ->
    Unix.handle_unix_error (handle_file args) file;
    main args tail

let () =
  let args = parse_args () in
  EventLogger.init_fake ();
  let handle = SharedMem.init ~num_workers:0 SharedMem.default_config in
  ignore (handle : SharedMem.handle);
  main args args.files
