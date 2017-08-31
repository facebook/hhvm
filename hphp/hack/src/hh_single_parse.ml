(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module Lowerer = Full_fidelity_ast
module Syntax = Full_fidelity_positioned_syntax
module SyntaxKind = Full_fidelity_syntax_kind

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
  | ValidatedFFP
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

let run_ffp : Relative_path.t -> Lowerer.result =
  Lowerer.from_file ~include_line_comments:true

let run_validated_ffp : Relative_path.t -> Lowerer.result = fun file ->
  let open Full_fidelity_syntax_tree in
  let content =
    try Sys_utils.cat (Relative_path.to_absolute file) with _ -> "" in
  let source_text = Full_fidelity_source_text.make content in
  let tree        = make source_text in
  let language    = language tree in
  let script      = Syntax.from_tree tree in
  let validated   =
    try
      Syntax.Validated.validate_script script
    with
    | Syntax.Validated.Validation_failure (k,s) as e -> begin
      Printf.eprintf "FAILURE: expected: %s  actual: %s\n"
        (SyntaxKind.to_string k)
        (SyntaxKind.to_string (Syntax.kind s));
      raise e
    end
  in
  let invalidated = Syntax.Validated.invalidate_script validated in
  let revalidated = Syntax.Validated.validate_script invalidated in
  assert (validated = revalidated); (* Idempotence *after* validation *)
  assert (script = invalidated); (* Idempotence *of* validation *)
  let invalidated =
    Full_fidelity_editable_positioned_syntax.from_positioned_syntax
      invalidated in
  Lowerer.lower
    ~elaborate_namespaces:true
    ~include_line_comments:false
    ~keep_errors:true
    ~ignore_pos:false
    ~quick:false
    ~suppress_output:false
    ~parser_options:ParserOptions.default
    ~content
    ~language
    ~file
    ~fi_mode:(if is_php tree then FileInfo.Mphp else
        let mode_string = String.trim (mode tree) in
        let mode_word =
          try Some (List.hd (Str.split (Str.regexp " +") mode_string)) with
          | _ -> None
        in
        Option.value_map mode_word ~default:FileInfo.Mpartial ~f:(function
          | "decl"           -> FileInfo.Mdecl
          | "strict"         -> FileInfo.Mstrict
          | ("partial" | "") -> FileInfo.Mpartial
          (* TODO: Come up with better mode detection *)
          | _                -> FileInfo.Mpartial
        )
      )
    ~source_text
    ~script:invalidated

let dump_sexpr ast = Debug.dump_ast (Ast.AProgram ast)


let measure : ('a -> 'b) -> 'a -> 'b * float = fun f x ->
  let start = Unix.gettimeofday () in
  let res = f x in
  let stop = Unix.gettimeofday () in
  res, stop -. start

let diff_comments : (Pos.t * string) list -> (Pos.t * string) list -> string
  = fun exp act ->
    let open Printf in
    let f (p, x) =
      sprintf "'%s' (%s)" (String.escaped x) (Pos.string_no_file p)
    in
    let exp_comments = String.concat "\n    -> " @@ List.map f exp in
    let act_comments = String.concat "\n    -> " @@ List.map f act in
    sprintf "
>> %d elements expected, found %d elements.
  Expected:
    -> %s
  Actual:
    -> %s
" (List.length exp) (List.length act) exp_comments act_comments

let run_parsers (file : Relative_path.t) (conf : parser_config)
  = match conf with
  | AST -> Printf.printf "%s" (dump_sexpr (run_ast file).Parser_hack.ast)
  | FFP -> Printf.printf "%s" (dump_sexpr (run_ffp file).Lowerer.ast)
  | ValidatedFFP ->
    Printf.printf "%s" (dump_sexpr (run_validated_ffp file).Lowerer.ast)
  | Compare diff_cmd ->
    let open Unix in
    let open Printf in
    let filename = Relative_path.S.to_string file in
    let ast_result = run_ast file in
    let ast_sexpr = dump_sexpr ast_result.Parser_hack.ast in
    let unsupported = Str.regexp "Fallthrough\\|Unsafe" in
    (try
        ignore (Str.search_forward unsupported ast_sexpr 0);
        eprintf "Warning: Unsupported features found: %s\n"
          (Str.matched_group 0 ast_sexpr);
        exit_with Unsupported
    with Not_found -> ());
    let ffp_result = run_ffp file in
    let ffp_sexpr =
      match ffp_result.Lowerer.ast with
      | Ast.Stmt (Ast.Markup ((_, ""), _)) :: defs
      | defs
        -> dump_sexpr defs
    in
    if ast_sexpr <> ffp_sexpr then begin
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
      eprintf "

****** Different
  Filename:     %s
  AST output:   %s
  FFP output:   %s
  Diff command: %s
" filename pathOld pathNew diff_cmd;
      flush Pervasives.stderr;
      let command = sprintf "%s %s %s" diff_cmd pathOld pathNew in
      ignore (system command);
      ignore (unlink pathOld);
      ignore (unlink pathNew);
      exit_with CmpDifferent
    end;

    let is_pragma_free (_, s) =
      let s = Prim_defs.string_of_comment s in
      let regexp = Str.regexp "HH_\\(IGNORE_ERROR\\|FIXME\\)\\[[0-9]*\\]" in
      try ignore (Str.search_forward regexp s 0); false with
      | Not_found -> true
    in
    let by_pos (p, _) (p', _) = Pos.compare p p' in
    let ast_comments = List.sort by_pos ast_result.Parser_hack.comments in
    let ffp_comments = List.filter is_pragma_free ffp_result.Lowerer.comments in
    let ffp_comments = List.sort by_pos ffp_comments in
    let only_exp (p,s) =
      let s = Prim_defs.string_of_comment s in
      sprintf "  - Only in expected: '%s' (%s)\n" s (Pos.string_no_file p)
    in
    let only_act (p,s) =
      let s = Prim_defs.string_of_comment s in
      sprintf "  - Only in actual: '%s' (%s)\n" s (Pos.string_no_file p)
    in
    let rec comments_diff
      (xs : (Pos.t * Prim_defs.comment) list)
      (ys : (Pos.t * Prim_defs.comment) list)
    : string list
    = match xs, ys with
    | [], [] -> []
    | xs, [] -> List.map only_exp xs
    | [], ys -> List.map only_act ys
    | (px,sx)::xs, (py,sy)::ys
      when sx = sy
        && Pos.line px = Pos.line py
        && Pos.end_line px = Pos.end_line py
        && Pos.start_cnum px = Pos.start_cnum py
      -> comments_diff xs ys
    | (px,sx)::xs, (py,sy)::ys when sx = sy ->
      sprintf "  - Pos diff for comment: %s expected, but found %s\n"
        (Pos.string_no_file px) (Pos.string_no_file py) :: comments_diff xs ys
    | x::xs, ys ->
      let diff, ys = skip x ys in
      diff @ comments_diff xs ys
    and skip x ys = match ys with
    | [] -> [only_exp x], []
    | y::ys when x = y -> [], ys
    | y::ys ->
      let diff, ys' = skip x ys in
      diff, y :: ys'
    in
    let diff = comments_diff ast_comments ffp_comments in
    if diff <> [] then begin
      eprintf "
****** Different (comments)
  Filename:     %s
  Difference:
%s
" filename (String.concat "" diff);
    end;
    printf "%s\n" ast_sexpr
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
    let ffp_sexpr = Debug.dump_ast (Ast.AProgram ffp_result.Lowerer.ast) in
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
    | "validated" -> ValidatedFFP
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
