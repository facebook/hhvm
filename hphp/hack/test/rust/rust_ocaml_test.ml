(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module OcamlPervasives = Pervasives
module OcamlPrintf = Printf
open Core_kernel
module Pervasives = OcamlPervasives
module Printf = OcamlPrintf

[@@@warning "-3"]

module SourceText = Full_fidelity_source_text
module SyntaxError = Full_fidelity_syntax_error
module MinimalSyntax = Full_fidelity_minimal_syntax
module PositionedSyntax = Full_fidelity_positioned_syntax
module Env = Full_fidelity_parser_env

let user = match Sys_utils.getenv_user () with
| Some x -> x
| None -> failwith "..."

let fbcode = Printf.sprintf "/data/users/%s/fbsource/fbcode/" user

type parser =
  | MINIMAL
  | POSITIONED
  | COROUTINE
  | DECL_MODE

type mode =
  | RUST
  | OCAML
  | COMPARE

type args = {
   mode : mode;
   parser : parser;
   is_experimental : bool;
   enable_stronger_await_binding : bool;
   disable_unsafe_expr : bool;
   disable_unsafe_block  : bool;
   force_hh : bool;
   enable_xhp : bool;
   hhvm_compat_mode : bool;
   php5_compat_mode : bool;
   codegen : bool;
   check_sizes : bool;
   check_json_equal_only : bool;
   keep_going : bool;
   filter : string;
   dir : string option;
}

module WithSyntax(Syntax : Syntax_sig.Syntax_S) = struct

module WithSmartConstructors(SC : SmartConstructors.SmartConstructors_S
  with type r = Syntax.t
  with module Token = Syntax.Token
) = struct

module SyntaxTree_ = Full_fidelity_syntax_tree.WithSyntax(Syntax)
module SyntaxTree = SyntaxTree_.WithSmartConstructors(SC)

let syntax_tree_into_parts tree =
  let mode, root, errors, state = SyntaxTree.(mode tree, root tree, errors tree, sc_state tree) in
  mode, root, errors, state

let to_json x=
  Syntax.to_json ~with_value:true x |>
  Hh_json.json_to_string ~pretty:true

let print_full_fidelity_error source_text error =
  let text = SyntaxError.to_positioned_string
    error (SourceText.offset_to_position source_text) in
  Printf.printf "%s\n" text

let reachable x = Obj.(x |> repr |> reachable_words)

let mode_to_string = function
  | None -> "None"
  | Some mode ->FileInfo.string_of_mode mode

let total = ref 0
let correct = ref 0
let crashed = ref 0 (* not all parse modes are supposed to work with all test files *)

let test args ~ocaml_env ~rust_env path =
  let file = Relative_path.(create Dummy (path)) in
  let source_text = SourceText.from_file file in

  let ok_ocaml, from_ocaml = match args.mode with
    | OCAML | COMPARE ->
      Printf.printf "CAML: %s\n" path;
      (try true, Some (SyntaxTree.make ~env:ocaml_env source_text) with
      | _ -> (false, None)
      )
    | RUST -> true, None
  in
  let ok_rust, from_rust = match args.mode with
    | RUST | COMPARE ->
      Printf.printf "RUST: %s\n" path;
      flush(stdout); (* make sure OCaml output is shown before Rust output *)
      (try true, Some (SyntaxTree.make ~env:rust_env source_text) with
      | _ -> (false, None)
      )
    | OCAML -> true, None
  in
  flush(stdout); (* ensure that Rust output precedes the rest of OCaml output *)

  let failed = ref false in
  begin match from_rust, from_ocaml with
  | Some from_rust, Some from_ocaml ->
    begin
      let (mode_from_rust, syntax_from_rust, errors_from_rust, state_from_rust)
        = syntax_tree_into_parts from_rust in
      let (mode_from_ocaml, syntax_from_ocaml, errors_from_ocaml, state_from_ocaml)
        = syntax_tree_into_parts from_ocaml in
      let rust_reachable_words = reachable syntax_from_rust  in
      let ocaml_reachable_words = reachable syntax_from_ocaml in
      if syntax_from_rust <> syntax_from_ocaml then begin
        let syntax_from_rust_as_json = to_json syntax_from_rust in
        let syntax_from_ocaml_as_json = to_json syntax_from_ocaml in

        let oc = Pervasives.open_out "/tmp/rust.json" in
        Printf.fprintf oc "%s\n" syntax_from_rust_as_json;
        close_out oc;
        let oc = Pervasives.open_out "/tmp/ocaml.json" in
        Printf.fprintf oc "%s\n" syntax_from_ocaml_as_json;
        close_out oc;

        if syntax_from_rust_as_json <> syntax_from_ocaml_as_json then begin
          Printf.printf "JSONs not equal: %s\n" path;
          failed := true
        end else
          Printf.printf "Structurally not equal: %s\n" path;
          failed := not @@ args.check_json_equal_only;
      end;
      if state_from_rust <> state_from_ocaml then begin
        failed := true;
        Printf.printf "States not equal: %s\n" path;
      end;
      if args.check_sizes && rust_reachable_words <> ocaml_reachable_words then begin
        failed := true;
        Printf.printf "Sizes not equal: %s (%d vs %d)\n"
          path rust_reachable_words ocaml_reachable_words;
      end;
      if mode_from_rust <> mode_from_ocaml then begin
        failed := true;
        Printf.printf "Modes not equal: %s (%s vs %s)\n" path
          (mode_to_string mode_from_ocaml) (mode_to_string mode_from_rust);
      end;
      (* Unlike other cases, errors make little sense when parse trees don't match *)
      if not !failed && errors_from_rust <> errors_from_ocaml then begin
        failed := true;
        Printf.printf "Errors not equal: %s (counts: %d vs %d)\n"
          path (List.length errors_from_rust) (List.length errors_from_ocaml);
        Printf.printf "---OCaml errors---\n";
        List.iter ~f:(print_full_fidelity_error source_text) errors_from_ocaml;
        Printf.printf "---Rust erors---\n";
        List.iter ~f:(print_full_fidelity_error source_text) errors_from_rust;
      end
    end
  | _ when ok_rust <> ok_ocaml ->
      (* some parsers other than positioned fail on some inputs; report failure if comparing *)
      failed := (args.parser = POSITIONED || args.mode = COMPARE);
      Printf.printf "Either crashed: %s (%b vs %b)\n" path (not ok_ocaml) (not ok_rust);
  | _ -> ()
  end;
  flush(stdout);

  incr total;
  if not ok_ocaml || not ok_rust then incr crashed
  else if not !failed then incr correct
  else Printf.printf "FAILED %s\n" path;

  let is_compare = args.mode = COMPARE in
  if is_compare || !crashed <> 0 then
    Printf.printf "%s/%d (crashed=%d)\n"
      (if is_compare then string_of_int !correct else "?")
      !total
      !crashed;
  if !failed && not args.keep_going then exit 1

let test_batch args ~ocaml_env ~rust_env files =
  List.iter files ~f:(test args ~ocaml_env ~rust_env)

end (* WithSmartConstructors *)

include WithSmartConstructors(SyntaxSmartConstructors.WithSyntax(Syntax))

end (* WithSyntax *)

let get_files_in_path ~args path =
  let files = Find.find [Path.make path] in
  let filter_re = Str.regexp args.filter in
  let matches_filter f =
    args.filter = "" || (
      try Str.search_forward filter_re f 0 >= 0
      with Not_found -> false
    )
  in
  List.filter ~f:begin fun f ->
    (
      String_utils.string_ends_with f ".php" ||
      String_utils.string_ends_with f ".hhi" ||
      String_utils.string_ends_with f ".hack"
    ) &&
    matches_filter f &&
    (not @@ String_utils.string_ends_with f "memory_exhaust.php") &&
    (not @@ String_utils.string_ends_with f "parser_massive_concat_exp.php") &&
    (not @@ String_utils.string_ends_with f "parser_massive_add_exp.php") &&
    (not @@ String_utils.string_ends_with f "byref-assignment-lvarvar.php") &&
    (not @@ String_utils.string_ends_with f "giant-arrays.php") &&
    (not @@ String_utils.string_ends_with f "byref-assignment4.php") &&
    (not @@ String_utils.string_ends_with f "byref-assignment3.php") &&
    (not @@ String_utils.string_ends_with f "byref-assignment2.php") &&
    (not @@ String_utils.string_ends_with f "byref-assignment1.php") &&
    (not @@ String_utils.string_ends_with f "phpvar1.php") &&
    (not @@ String_utils.string_ends_with f "bug64660.php") &&
    match args.parser with
    | COROUTINE ->
        true
    | DECL_MODE ->
      (* Note: these crash in both OCaml and Rust version of positioned DeclMode parser *)
      (not @@ String_utils.string_ends_with f "ffp/yield_bad1.php") &&
      (not @@ String_utils.string_ends_with f "ffp/yield_from_bad1.php") &&
      (not @@ String_utils.string_ends_with f "let/let_closure.php") &&
      (not @@ String_utils.string_ends_with f "let/let_lambda.php") &&
      (not @@ String_utils.string_ends_with f "test_variadic_type_hint.php") &&
      (not @@ String_utils.string_ends_with f "namespace_group_use_decl.php") &&
      (* FIXME: These ones crashes during Rust parse but in OCaml code with: *)
      (not @@ String_utils.string_ends_with f "nullsafe_call_on_expr_dep.php") &&
      (not @@ String_utils.string_ends_with f "await_as_an_expression_simple.php") &&
      (not @@ String_utils.string_ends_with f "compile_test_yield_erling.php") &&
      (not @@ String_utils.string_ends_with f "compile_test_yield.php") &&
      (not @@ String_utils.string_ends_with f "functional_generator.php") &&
      (not @@ String_utils.string_ends_with f "typecheck/yield_from3.php") &&
      (not @@ String_utils.string_ends_with f "yield_wait_for_result_bad2.php") &&
      (not @@ String_utils.string_ends_with f "await_as_an_expression_simple.php") &&
      (not @@ String_utils.string_ends_with f "/ai/backwards_analysis/yield.php") &&
      (not @@ String_utils.string_ends_with f "/ai/forward_analysis/yield.php") &&
      (not @@ String_utils.string_ends_with f "/unreachable/yield.php") &&
      (not @@ String_utils.string_ends_with f "/lint/dead_statement.php") &&
      (not @@ String_utils.string_ends_with f "/emitter/generator.php") &&
      (*
Uncaught exception:

  (Invalid_argument "index out of bounds")

Raised by primitive operation at file "$HOME/fbsource/fbcode/hphp/hack/src/utils/line_break_map.ml", line 60, characters 19-41
Called from file "$HOME/fbsource/fbcode/hphp/hack/src/utils/line_break_map.ml", line 67, characters 4-43
Called from file "$HOME/fbsource/fbcode/hphp/hack/src/parser/full_fidelity_source_text.ml" (inlined), line 83, characters 2-60
Called from file "$HOME/fbsource/fbcode/hphp/hack/src/parser/full_fidelity_positioned_token.ml" (inlined), line 317, characters 2-72
      *)
      true
    | _ -> true
  end files

let get_files args =
  match args.dir with
  | None -> (get_files_in_path (fbcode ^ "hphp/hack/test/") ~args)
  | Some dir -> get_files_in_path dir ~args

let parse_args () =
  let mode = ref COMPARE in
  let parser = ref MINIMAL in
  let is_experimental = ref false in
  let enable_stronger_await_binding = ref false in
  let disable_unsafe_expr = ref false in
  let disable_unsafe_block = ref false in
  let codegen = ref false in
  let force_hh = ref false in
  let enable_xhp = ref false in
  let hhvm_compat_mode = ref false in
  let php5_compat_mode = ref false in
  let check_sizes = ref false in
  let check_json_equal_only = ref false in
  let keep_going = ref false in
  let filter = ref "" in
  let dir = ref None in

  let options =  [
    "--rust", Arg.Unit (fun () -> mode := RUST), "";
    "--ocaml", Arg.Unit (fun () -> mode := OCAML), "";
    "--positioned", Arg.Unit (fun () -> parser := POSITIONED), "";
    "--coroutine", Arg.Unit (fun () -> parser := COROUTINE), "";
    "--decl-mode", Arg.Unit (fun () -> (parser := DECL_MODE; check_json_equal_only := true)), "";
    "--experimental", Arg.Set is_experimental, "";
    "--enable-stronger-await-binding", Arg.Set enable_stronger_await_binding, "";
    "--disable-unsafe-expr", Arg.Set disable_unsafe_expr, "";
    "--disable-unsafe-block", Arg.Set disable_unsafe_block, "";
    "--codegen", Arg.Set codegen, "";
    "--force-hh", Arg.Set force_hh, "";
    "--enable-xhp", Arg.Set enable_xhp, "";
    "--hhvm-compat-mode", Arg.Set hhvm_compat_mode, "";
    "--php5-compat-mode", Arg.Set php5_compat_mode, "";
    "--check-sizes", Arg.Set check_sizes, "";
    "--check-json-equal-only", Arg.Set check_json_equal_only, "";
    "--keep-going", Arg.Set keep_going, "";
    "--filter", Arg.String (fun s -> filter := s), "";
    "--dir", Arg.String (fun s -> dir := Some (s)), "";
    "--hhvm-tests", Arg.Unit (fun () -> dir := Some (fbcode ^ "hphp/test/")), "";
  ] in
  Arg.parse options (fun _ -> ()) "";
  {
    mode = !mode;
    parser = !parser;
    is_experimental = !is_experimental;
    enable_stronger_await_binding = !enable_stronger_await_binding;
    disable_unsafe_expr = !disable_unsafe_expr;
    disable_unsafe_block = !disable_unsafe_block;
    codegen = !codegen;
    force_hh = !force_hh;
    enable_xhp = !enable_xhp;
    hhvm_compat_mode = !hhvm_compat_mode;
    php5_compat_mode = !php5_compat_mode;
    check_sizes = !check_sizes;
    check_json_equal_only = !check_json_equal_only;
    keep_going = !keep_going;
    filter = !filter;
    dir = !dir;
  }

module MinimalTest = WithSyntax(MinimalSyntax)
module PositionedTest = WithSyntax(PositionedSyntax)

module CoroutineTest_ = WithSyntax(PositionedSyntax)
module CoroutineSC = Coroutine_smart_constructor.WithSyntax(PositionedSyntax)
module CoroutineTest = CoroutineTest_.WithSmartConstructors(CoroutineSC)

module DeclModeTest_ = WithSyntax(PositionedSyntax)
module DeclModeSC = DeclModeSmartConstructors.WithSyntax(PositionedSyntax)
module DeclModeTest = DeclModeTest_.WithSmartConstructors(DeclModeSC)

(*
Tool comparing outputs of Rust and OCaml parsers. Example usage:

  buck run @mode/dbg hphp/hack/test/rust:rust_ocaml

See parse_args for all the options
*)
let () =
  let args = parse_args () in
  let files = get_files args in
  Hh_logger.log "Starting...";
  let t = Unix.gettimeofday() in
  let mode = if args.is_experimental then Some (FileInfo.Mexperimental) else None in
  let make_env = Full_fidelity_parser_env.make
    ~enable_stronger_await_binding:args.enable_stronger_await_binding
    ~disable_unsafe_expr:args.disable_unsafe_expr
    ~disable_unsafe_block:args.disable_unsafe_block
    ~force_hh:args.force_hh
    ~enable_xhp:args.enable_xhp
    ~hhvm_compat_mode:args.hhvm_compat_mode
    ~php5_compat_mode:args.php5_compat_mode
    ?mode
  in
  let ocaml_env = make_env () in
  let rust_env = make_env ~rust:true () in
  begin match args.parser with
    | MINIMAL -> MinimalTest.test_batch args ~ocaml_env ~rust_env  files
    | POSITIONED -> PositionedTest.test_batch args ~ocaml_env ~rust_env  files
    | COROUTINE -> CoroutineTest.test_batch args ~ocaml_env ~rust_env files
    | DECL_MODE -> DeclModeTest.test_batch args ~ocaml_env ~rust_env files
  end;
  let _ = Hh_logger.log_duration "Done:" t  in
  ()
