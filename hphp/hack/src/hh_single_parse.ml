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

type parser_return = Parser_hack.parser_return * float
type result =
  | CmpDifferent
  | Unsupported
  | ParseError

let exit_code : result -> int = function
  | ParseError   -> 1
  | Unsupported  -> 3
  | CmpDifferent -> 42

type parser_config =
  | AST
  | FFP
  | Benchmark
  | Compare of string

let exit_with : result -> 'a = fun r -> exit (exit_code r)

let handle_errors : Errors.t -> unit = fun errorl ->
  let open Errors in
  let print_err err = output_string stderr (to_string (to_absolute err)) in
  if is_empty errorl
  then ()
  else begin
    iter_error_list print_err errorl;
    exit_with ParseError
  end

let run_ast : Relative_path.t -> Parser_hack.parser_return = fun file ->
  let parse_call () = Parser_hack.from_file ParserOptions.default file in
  let errorl, result, _ = Errors.do_ parse_call in
  handle_errors errorl;
  result

let run_ffp : Relative_path.t -> Parser_hack.parser_return =
  Full_fidelity_ast.from_file_with_legacy

let dump_sexpr ast = Debug.dump_ast (Ast.AProgram ast.Parser_hack.ast)


let measure : ('a -> 'b) -> 'a -> 'b * float = fun f x ->
  let start = Unix.gettimeofday () in
  let res = f x in
  let stop = Unix.gettimeofday () in
  res, stop -. start

let run_parsers (file : Relative_path.t) (conf : parser_config)
  = match conf with
  | AST -> Printf.printf "%s" (dump_sexpr @@ run_ast file)
  | FFP -> Printf.printf "%s" (dump_sexpr @@ run_ffp file)
  | Compare diff_cmd ->
    let open Unix in
    let open Printf in
    let ast_result = run_ast file in
    let ast_sexpr = dump_sexpr ast_result in
    let unsupported = Str.regexp "Fallthrough\\|Unsafe" in
    (try
        ignore (Str.search_forward unsupported ast_sexpr 0);
        eprintf "Warning: Unsupported features found: %s\n"
          (Str.matched_group 0 ast_sexpr);
        exit_with Unsupported
    with Not_found -> ());
    let ffp_result = run_ffp file in
    let ffp_sexpr = dump_sexpr ffp_result in
    if ast_sexpr = ffp_sexpr
    then printf "%s\n" ast_sexpr
    else begin
      let filename = Relative_path.S.to_string file in
      let mkTemp (name : string) (content : string) = begin
        let ic = open_process_in (sprintf "mktemp tmp.%s.XXXXXXXX" name) in
        let path = input_line ic in
        ignore (close_process_in ic);
        let oc = open_out path in
        fprintf oc "%s\n\n%s\n\n%s\n" filename content filename;
        close_out oc;
        path
      end in

      let pathOld = mkTemp "OLD" ast_sexpr in
      let pathNew = mkTemp "NEW" ffp_sexpr in
      eprintf "\n\n****** Different\n";
      eprintf "  Filename:     %s\n" filename;
      eprintf "  AST output:   %s\n" pathOld;
      eprintf "  FFP output:   %s\n" pathNew;
      eprintf "  Diff command: %s\n" diff_cmd;
      flush Pervasives.stderr;
      let command = sprintf "%s %s %s" diff_cmd pathOld pathNew in
      ignore (system command);
      ignore (unlink pathOld);
      ignore (unlink pathNew);
      exit_with CmpDifferent
    end
  | Benchmark ->
    let filename = Relative_path.S.to_string file in
    let (ast_result, ast_duration), (ffp_result, ffp_duration) =
      try (measure run_ast file, measure run_ffp file)
      with _ -> begin
        Printf.printf "FAIL, %s\n" filename;
        exit_with ParseError
      end
    in
    let ast_sexpr = Debug.dump_ast (Ast.AProgram ast_result.Parser_hack.ast) in
    let ffp_sexpr = Debug.dump_ast (Ast.AProgram ffp_result.Parser_hack.ast) in
    if ast_sexpr = ffp_sexpr
    then
      Printf.printf
        "PASS, %s, %12.10f, %12.10f\n"
        filename
        ast_duration
        ffp_duration
    else begin
      Printf.printf "FAIL, %s\n" filename;
      exit_with CmpDifferent
    end




let () =
  Printexc.record_backtrace true;
  let use_parser = ref "ast"  in
  let use_diff   = ref "diff" in
  let filename   = ref ""     in
  Arg.(parse
    [ ("--parser", Set_string use_parser,
        "Which parser to use (ast, ffp, compare) [def: ast]"
      )
    ; ("--diff", Set_string use_diff,
        "Which diff tool to compare different S-Expressions with [def: vimdiff]"
      )
    ]) (fun fn -> filename := fn) usage;
  let parse_function = match !use_parser with
    | "ast"       -> AST
    | "ffp"       -> FFP
    | "benchmark" -> Benchmark
    | "compare"   -> Compare !use_diff
    | s -> raise (Failure (Printf.sprintf "Unknown parser '%s'\n" s))
  in
  if String.length !filename = 0 then raise (Failure "No filename given");
  EventLogger.init EventLogger.Event_logger_fake 0.0;
  let _handle = SharedMem.init GlobalConfig.default_sharedmem_config in
  Unix.handle_unix_error (fun fn ->
    let file = Relative_path.create Relative_path.Dummy fn in
    run_parsers file parse_function
  ) !filename
