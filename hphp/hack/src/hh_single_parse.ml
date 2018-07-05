(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Lowerer = Full_fidelity_ast
module Syntax = Full_fidelity_positioned_syntax
module SyntaxKind = Full_fidelity_syntax_kind
module SourceText = Full_fidelity_source_text
module SyntaxTree = Full_fidelity_syntax_tree
  .WithSyntax(Syntax)

let purpose = "Read a single Hack file and produce the resulting S-Expression."
let extra   = "(Options for development / parser selection and comparisson.)"
let usage   = Printf.sprintf
  "Usage: %s <options> filename\n%s\n%s"
  Sys.argv.(0)
  purpose
  extra

type parser_return = Parser_return.t * float
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
  | Benchmark_batch of int
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

let run_ast ?(quick=false) ~allow_malformed file =
  let parse_call () = Parser_hack.from_file ParserOptions.default ~quick file in
  let errorl, result = Errors.do_ parse_call in
  if not allow_malformed then handle_errors errorl;
  result

let run_ffp
  ?(iters = 0)
  ~codegen
  ~allow_malformed
  (file : Relative_path.t)
: Lowerer.result =
  let env =
    Lowerer.make_env
      ~codegen
      ~include_line_comments:true
      ~fail_open:allow_malformed
      ~keep_errors:(not allow_malformed)
      file
  in
  if iters < 1 then () else
  for i = 1 to iters do
    ignore(Lowerer.from_file env : Lowerer.result);
  done;
  Lowerer.from_file env
let run_validated_ffp : Relative_path.t -> Lowerer.result = fun file ->
  let open SyntaxTree in
  let source_text = SourceText.from_file file in
  let tree        = make source_text in
  let script      = root tree in
  let validated   =
    try
      Syntax.Validated.validate_script script
    with
    | Syntax.Validated.Validation_failure (k,s) as e -> begin
      Printf.eprintf "FAILURE: expected: %s  actual: %s\n"
        (Option.value_map ~f:SyntaxKind.to_string ~default:"Some token" k)
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
  let is_hh_file = is_hack tree in
  let env = Lowerer.make_env ~is_hh_file file in
  let comments = Lowerer.scour_comments_and_add_fixmes env source_text script in
  let module Lowerer = Lowerer.WithPositionedSyntax(Full_fidelity_editable_positioned_syntax) in
  Lowerer.lower env ~source_text ~script:invalidated comments

let measure : (unit -> 'a) -> 'a * float = fun f ->
  let start = Unix.gettimeofday () in
  let res = f () in
  let stop = Unix.gettimeofday () in
  res, stop -. start

let compare_asts dumper filename diff_cmd ast_result ffp_result =
  let open Unix in
  let open Printf in
  let ast_sexpr = dumper ast_result.Parser_return.ast in
  let ffp_sexpr =
    match ffp_result.Lowerer.ast with
    | Ast.Stmt (_, Ast.Markup ((_, ""), _)) :: defs
    | defs -> dumper defs
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
  ast_sexpr

let compare_fixmes ast_fixmes ffp_fixmes =
  let open Printf in
  let print_fixmes map_map_pos =
    let bindings_bindings_pos =
      IMap.bindings @@ IMap.map IMap.bindings map_map_pos
    in
    let inner (k,p) = eprintf "   - %d: %s" k (Pos.print_verbose_relative p) in
    let outer (k,vs) = eprintf " - %d:\n" k; List.iter inner vs in
    List.iter outer bindings_bindings_pos;
    ()
  in
  match ast_fixmes, ffp_fixmes with
  | None, None -> ()
  | None, Some ffp ->
    eprintf "Legacy parser did not find FIXMEs, but FFP gave:\n";
    print_fixmes ffp
  | Some ast, None ->
    eprintf "FFP didn't find FIMXEs, but legacy gave:\n";
    print_fixmes ast
  | Some ast, Some ffp when IMap.equal ast ffp ->
    ()
  | Some ast, Some ffp ->
    eprintf "FIXMES ARE DIFFERENT:\n\nLegacy\n------\n";
    print_fixmes ast;
    eprintf "\n\nFFP\n---\n";
    print_fixmes ffp;
    flush Pervasives.stderr;
    exit_with CmpDifferent

let compare_pos ast_result ffp_result =
  let fetch_posses ast =
    let pos_fetcher = object
      inherit [_] Ast.iter
      val mutable result = []
      method! on_pos () p = result <- p :: result
      method get = result
    end in
    pos_fetcher#on_program () ast;
    pos_fetcher#get
  in
  let ast = List.rev @@ fetch_posses ast_result in
  let ffp =
    match List.rev @@ fetch_posses ffp_result with
    | (p::ps) when Pos.line p = 1 -> ps (* header is dropped by Parser_return *)
    | ps -> ps
  in
  let ast_count = List.length ast in
  let ffp_count = List.length ffp in
  if ast_count = ffp_count then
    let diff_pos x y = if Pos.compare x y <> 0 then begin
      Printf.eprintf " ): legacy: %s\n    ffp:    %s\n"
        (Pos.print_verbose_relative x)
        (Pos.print_verbose_relative y)
    end in
    List.iter2 diff_pos ast ffp
  else begin
    let show_pos p =
      Printf.eprintf "     - %s\n" @@ Pos.string_no_file @@ Pos.to_absolute p
    in
    Printf.eprintf " ): legacy has %d positions and ffp has %d:\n"
      ast_count ffp_count;
    Printf.eprintf "    Legacy\n    ------\n";
    List.iter show_pos ast;
    Printf.eprintf "    FFP\n    ---\n";
    List.iter show_pos ffp
  end

let compare_comments filename ast_result ffp_result =
  let open Printf in
  let is_pragma_free (_, s) =
    let s = Prim_defs.string_of_comment s in
    let regexp = Str.regexp "HH_\\(IGNORE_ERROR\\|FIXME\\)\\[[0-9]*\\]" in
    try ignore (Str.search_forward regexp s 0); false with
    | Not_found -> true
  in
  let by_pos (p, _) (p', _) = Pos.compare p p' in
  let ast_comments = List.sort by_pos ast_result.Parser_return.comments in
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
  end

let run_parsers
  dumper
  (file : Relative_path.t)
  (conf : parser_config)
  ~hash
  ~codegen
  ~allow_malformed
=
  match conf with
  | AST ->
    let ast = (run_ast ~allow_malformed file).Parser_return.ast in
    let output =
      if not hash then dumper (run_ast ~allow_malformed file).Parser_return.ast else
        let decl_hash = Ast_utils.generate_ast_decl_hash ast in
        OpaqueDigest.to_hex decl_hash
    in
    Printf.printf "%s" output
  | FFP -> Printf.printf "%s" (dumper (run_ffp ~codegen ~allow_malformed file).Lowerer.ast)
  | ValidatedFFP ->
    Printf.printf "%s" (dumper (run_validated_ffp file).Lowerer.ast)
  | Compare diff_cmd ->
    let open Printf in
    let filename = Relative_path.S.to_string file in
    let ast_result = run_ast ~allow_malformed file in
    let ast_fixmes = Fixmes.get_fixmes_from_heap file in
    Fixmes.HH_FIXMES.remove_batch (Relative_path.Set.singleton file);
    Fixmes.DECL_HH_FIXMES.remove_batch (Relative_path.Set.singleton file);

    let ffp_result = run_ffp ~codegen ~allow_malformed file in
    let ffp_fixmes = Fixmes.get_fixmes_from_heap file in
    let sexpr = compare_asts dumper filename diff_cmd ast_result ffp_result in
    let () = compare_pos ast_result.Parser_return.ast ffp_result.Lowerer.ast in
    let () = compare_fixmes ast_fixmes ffp_fixmes in
    let () = compare_comments filename ast_result ffp_result in
    printf "%s\n" sexpr
  | Benchmark ->
    let filename = Relative_path.S.to_string file in
    let (ast_result, ast_duration), (ffp_result, ffp_duration) =
      try (measure (fun () -> run_ast ~allow_malformed file),
           measure (fun () -> run_ffp ~codegen ~allow_malformed file))
      with _ -> begin
        Printf.printf "FAIL, %s\n" filename;
        exit_with ParseError
      end
    in
    let ast_sexpr = dumper ast_result.Parser_return.ast in
    let ffp_sexpr = dumper ffp_result.Lowerer.ast in
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
  | Benchmark_batch iters ->
    let filename = Relative_path.S.to_string file in
    let _, duration =
      try (measure (fun () -> run_ffp ~codegen ~iters file))
      with _ -> begin
        Printf.printf "FAIL, %s\n" filename;
        exit_with ParseError
      end
    in
    let res = Printf.sprintf
      "PASS, %s, %12.10f\n"
      filename duration in
    print_endline res

let () =
  Printexc.record_backtrace true;
  let use_parser = ref "ast"  in
  let use_diff   = ref "diff" in
  let hash       = ref false in
  let dumper     = ref Debug.dump_ast in
  let filename   = ref ""     in
  let num_runs   = ref 100 in
  let benchmark_files      = ref [] in
  let no_codegen    = ref false in
  let allow_malformed = ref false in
  Arg.(parse
    [ ("--parser", Set_string use_parser,
        "Which parser to use (ast, ffp, compare) [def: ast]"
      )
    ; ("--diff", Set_string use_diff,
        "Which diff tool to compare different S-Expressions with [default: vimdiff]"
      )
    ; ("--hash", Set hash,
        "Get the decl level parsing hash of a given file "
      )
    ; ("--sorted", Unit (fun () -> dumper := Debug.dump_sorted_ast),
        "When using the `compare` parser, the (lexicographically) sort the " ^
        "S-Expressions before diffing"
      )
    ; ("--show-pos", Unit (fun () -> Sof.show_pos := true),
        "Show positional information on the AST"
      )
    ; ("--num-runs", Int (fun x -> num_runs := x),
        "How many times to benchmark if in benchmark mode [default: 100]"
      )
    ; ("--benchmark_batch", Rest (fun fn -> benchmark_files := fn::!benchmark_files),
        "Run benchmarking on a list of files"
    )
    ; ("--no-codegen", Set no_codegen,
        "Turn off codegen mode when parsing with FFP [default: false]"
    )
    ; ("--allow-malformed", Set allow_malformed,
        "Allow malformed files (such as for testing IDE services) [default: false]"
    ) ;
    ]) (fun fn -> filename := fn) usage;
  let parse_function = match !use_parser with
    | _ when !benchmark_files <> [] -> Benchmark_batch !num_runs
    | "ast"       -> AST
    | "ffp"       -> FFP
    | "validated" -> ValidatedFFP
    | "benchmark" -> Benchmark
    | "compare"   -> Compare !use_diff
    | s -> raise (Failure (Printf.sprintf "Unknown parser '%s'\n" s))
  in
  if String.length !filename = 0 && !benchmark_files = [] then failwith "No filename given";
  EventLogger.init EventLogger.Event_logger_fake 0.0;
  let _handle = SharedMem.init GlobalConfig.default_sharedmem_config in
  let dumper ast = !dumper (Ast.AProgram ast) in
  let parse_file fn =
    let file = Relative_path.create Relative_path.Dummy fn in
    run_parsers
      dumper
      file
      ~hash:!hash
      parse_function
      ~codegen:(not !no_codegen)
      ~allow_malformed:!allow_malformed
  in
  if !benchmark_files <> [] then
    List.iter parse_file !benchmark_files
  else
    Unix.handle_unix_error (parse_file) !filename
