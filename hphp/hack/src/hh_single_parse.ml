(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Hh_debug = Debug
open Core_kernel
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
  | FFP
  | ValidatedFFP
  | Benchmark_batch of int

let exit_with : result -> 'a = fun r -> exit (exit_code r)

let handle_errors : Errors.t -> unit = fun errorl ->
  let open Errors in
  let print_err err = Out_channel.output_string stderr (to_string (to_absolute err)) in
  if is_empty errorl
  then ()
  else begin
    iter_error_list print_err errorl;
    exit_with ParseError
  end


let run_ffp
  ?(iters = 0)
  ~codegen
  ~allow_malformed
  ~pocket_universes
  (file : Relative_path.t)
: Lowerer.result =
  let env =
    let popt = ParserOptions.setup_pocket_universes ParserOptions.default
        pocket_universes in
    Lowerer.make_env
      ~codegen
      ~include_line_comments:true
      ~fail_open:allow_malformed
      ~keep_errors:(not allow_malformed)
      ~parser_options:popt
      file
  in
  if iters < 1 then () else
  for i = 1 to iters do
    ignore(Lowerer.from_file env : Lowerer.result);
  done;
  Lowerer.from_file env
let run_validated_ffp : bool -> Relative_path.t -> Lowerer.result =
  fun pocket_universes file ->
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
  let popt = ParserOptions.setup_pocket_universes ParserOptions.default
      pocket_universes in
  let env = Lowerer.make_env ~is_hh_file ~parser_options:popt file in
  let comments = Lowerer.scour_comments_and_add_fixmes env source_text script in
  let module Lowerer = Lowerer.WithPositionedSyntax(Full_fidelity_editable_positioned_syntax) in
  Lowerer.lower env ~source_text ~script:invalidated comments

let measure : (unit -> 'a) -> 'a * float = fun f ->
  let start = Unix.gettimeofday () in
  let res = f () in
  let stop = Unix.gettimeofday () in
  res, stop -. start


let run_parsers
  dumper
  (file : Relative_path.t)
  (conf : parser_config)
  ~hash
  ~codegen
  ~allow_malformed
  ~dump_nast
  ~pocket_universes
=
  match conf with
  | FFP ->
    let res = run_ffp ~codegen ~allow_malformed ~pocket_universes file in
    let ast = res.Lowerer.ast in
    let output =
      if dump_nast then (
        let nast = Ast_to_nast.convert ast in
        Nast.show_program nast
      ) else
      if not hash then dumper ast
      else
        let decl_hash = Ast_utils.generate_ast_decl_hash ast in
        OpaqueDigest.to_hex decl_hash
    in
    Printf.printf "%s" output
  | ValidatedFFP ->
    let res = run_validated_ffp pocket_universes file in
    Printf.printf "%s" (dumper res.Lowerer.ast)
  | Benchmark_batch iters ->
    let filename = Relative_path.S.to_string file in
    let _, duration =
      try (measure (fun () ->
        run_ffp ~codegen ~iters ~allow_malformed:false ~pocket_universes file))
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
  let use_parser = ref "ffp"  in
  let hash       = ref false in
  let dumper     = ref Hh_debug.dump_ast in
  let filename   = ref ""     in
  let num_runs   = ref 100 in
  let benchmark_files      = ref [] in
  let no_codegen    = ref false in
  let allow_malformed = ref false in
  let dump_nast = ref false in
  let pocket_universes = ref false in
  Arg.(parse
    [ ("--hash", Set hash,
        "Get the decl level parsing hash of a given file "
      )
    ; ("--sorted", Unit (fun () -> dumper := Hh_debug.dump_sorted_ast),
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
    ; ("--nast", Set dump_nast,
        "Convert to NAST and print [default: false]"
    )
    ; ("--allow-malformed", Set allow_malformed,
        "Allow malformed files (such as for testing IDE services) [default: false]"
    )
    ; ("--pocket-universes", Set pocket_universes,
        "Enables support for Pocket Universes [default: false]"
    );
    ]) (fun fn -> filename := fn) usage;
  let parse_function = match !use_parser with
    | _ when !benchmark_files <> [] -> Benchmark_batch !num_runs
    | "ffp"       -> FFP
    | "validated" -> ValidatedFFP
    | s -> raise (Failure (Printf.sprintf "Unknown parser '%s'\n" s))
  in
  if String.length !filename = 0 && !benchmark_files = [] then failwith "No filename given";
  EventLogger.init EventLogger.Event_logger_fake 0.0;
  let handle = SharedMem.init ~num_workers:0 GlobalConfig.default_sharedmem_config in
  ignore (handle: SharedMem.handle);
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
      ~dump_nast:!dump_nast
      ~pocket_universes:!pocket_universes
  in
  if !benchmark_files <> [] then
    List.iter ~f:parse_file !benchmark_files
  else
    Unix.handle_unix_error (parse_file) !filename
