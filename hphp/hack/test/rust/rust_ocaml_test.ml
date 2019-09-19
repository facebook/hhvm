(*
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
module EditablePositionedSyntax = Full_fidelity_editable_positioned_syntax
module Env = Full_fidelity_parser_env

let user =
  match Sys_utils.getenv_user () with
  | Some x -> x
  | None -> failwith "..."

let fbcode = Printf.sprintf "/data/users/%s/fbsource/fbcode/" user

type parser =
  | MINIMAL
  | POSITIONED
  | COROUTINE
  | DECL_MODE
  | PPL_REWRITER
  | LOWERER
  | COROUTINE_ERRORS

type mode =
  | RUST
  | OCAML
  | COMPARE

type args = {
  mode: mode;
  parser: parser;
  is_experimental: bool;
  hhvm_compat_mode: bool;
  php5_compat_mode: bool;
  codegen: bool;
  check_sizes: bool;
  check_json_equal_only: bool;
  check_printed_tree: bool;
  keep_going: bool;
  filter: string;
  dir: string option;
}

module type TreeBuilder_S = sig
  type t

  val make : env:Env.t -> SourceText.t -> t
end

module WithSyntax (Syntax : Syntax_sig.Syntax_S) = struct
  module WithSmartConstructors
      (SC : SmartConstructors.SmartConstructors_S
              with module Token = Syntax.Token
              with type r = Syntax.t) =
  struct
    module SyntaxTree_ = Full_fidelity_syntax_tree.WithSyntax (Syntax)
    module SyntaxTree = SyntaxTree_.WithSmartConstructors (SC)

    module WithTreeBuilder
        (TreeBuilder : TreeBuilder_S with type t = SyntaxTree.t) =
    struct
      let syntax_tree_into_parts tree =
        let (mode, root, errors, state) =
          SyntaxTree.(mode tree, root tree, errors tree, sc_state tree)
        in
        (mode, root, errors, state)

      let to_json x =
        Syntax.to_json ~with_value:true x
        |> Hh_json.json_to_string ~pretty:true

      let print_full_fidelity_error source_text error =
        let text =
          SyntaxError.to_positioned_string
            error
            (SourceText.offset_to_position source_text)
        in
        Printf.printf "%s\n" text

      let reachable x = Obj.(x |> repr |> reachable_words)

      let mode_to_string = function
        | None -> "None"
        | Some mode -> FileInfo.string_of_mode mode

      let total = ref 0

      let correct = ref 0

      let crashed = ref 0

      (* not all parse modes are supposed to work with all test files *)

      let test args ~ocaml_env ~rust_env file contents =
        let source_text = SourceText.make file contents in
        let path = Relative_path.to_absolute file in
        let (ok_ocaml, from_ocaml) =
          match args.mode with
          | OCAML
          | COMPARE ->
            Printf.printf "CAML: %s\n" path;
            (try (true, Some (TreeBuilder.make ~env:ocaml_env source_text))
             with _ -> (false, None))
          | RUST -> (true, None)
        in
        let (ok_rust, from_rust) =
          match args.mode with
          | RUST
          | COMPARE ->
            Printf.printf "RUST: %s\n" path;
            flush stdout;

            (* make sure OCaml output is shown before Rust output *)
            (try (true, Some (TreeBuilder.make ~env:rust_env source_text))
             with _ -> (false, None))
          | OCAML -> (true, None)
        in
        flush stdout;

        (* ensure that Rust output precedes the rest of OCaml output *)
        let failed = ref false in
        (match (from_rust, from_ocaml) with
        | (Some from_rust, Some from_ocaml) ->
          let ( mode_from_rust,
                syntax_from_rust,
                errors_from_rust,
                state_from_rust ) =
            syntax_tree_into_parts from_rust
          in
          let ( mode_from_ocaml,
                syntax_from_ocaml,
                errors_from_ocaml,
                state_from_ocaml ) =
            syntax_tree_into_parts from_ocaml
          in
          let rust_reachable_words = reachable syntax_from_rust in
          let ocaml_reachable_words = reachable syntax_from_ocaml in
          ( if args.check_printed_tree then
            match
              Syntax.
                (extract_text syntax_from_ocaml, extract_text syntax_from_rust)
            with
            | (Some text_from_ocaml, Some text_from_rust)
              when text_from_ocaml <> text_from_rust ->
              let oc = Pervasives.open_out "/tmp/rust.php" in
              Printf.fprintf oc "%s\n" text_from_rust;
              close_out oc;
              let oc = Pervasives.open_out "/tmp/ocaml.php" in
              Printf.fprintf oc "%s\n" text_from_ocaml;
              close_out oc;
              Printf.printf "Printed tree not equal: %s\n" path;
              failed := true
            | (Some _, Some _) -> () (* equal *)
            | _ ->
              Printf.printf
                "Tree to source transformation is not supported for this syntax type\n";
              failed := true );
          if syntax_from_rust <> syntax_from_ocaml then (
            let syntax_from_rust_as_json = to_json syntax_from_rust in
            let syntax_from_ocaml_as_json = to_json syntax_from_ocaml in
            let oc = Pervasives.open_out "/tmp/rust.json" in
            Printf.fprintf oc "%s\n" syntax_from_rust_as_json;
            close_out oc;
            let oc = Pervasives.open_out "/tmp/ocaml.json" in
            Printf.fprintf oc "%s\n" syntax_from_ocaml_as_json;
            close_out oc;

            if syntax_from_rust_as_json <> syntax_from_ocaml_as_json then (
              Printf.printf "JSONs not equal: %s\n" path;
              failed := true
            ) else
              Printf.printf "Structurally not equal: %s\n" path;
            failed := not @@ args.check_json_equal_only
          );
          if state_from_rust <> state_from_ocaml then (
            failed := true;
            Printf.printf "States not equal: %s\n" path
          );
          if args.check_sizes && rust_reachable_words <> ocaml_reachable_words
          then (
            failed := true;
            Printf.printf
              "Sizes not equal: %s (%d vs %d)\n"
              path
              rust_reachable_words
              ocaml_reachable_words
          );
          if mode_from_rust <> mode_from_ocaml then (
            failed := true;
            Printf.printf
              "Modes not equal: %s (%s vs %s)\n"
              path
              (mode_to_string mode_from_ocaml)
              (mode_to_string mode_from_rust)
          );

          (* Unlike other cases, errors make little sense when parse trees don't match *)
          if (not !failed) && errors_from_rust <> errors_from_ocaml then (
            failed := true;
            Printf.printf
              "Errors not equal: %s (counts: %d vs %d)\n"
              path
              (List.length errors_from_rust)
              (List.length errors_from_ocaml);
            Printf.printf "---OCaml errors---\n";
            List.iter
              ~f:(print_full_fidelity_error source_text)
              errors_from_ocaml;
            Printf.printf "---Rust erors---\n";
            List.iter
              ~f:(print_full_fidelity_error source_text)
              errors_from_rust
          )
        | _ when ok_rust <> ok_ocaml ->
          (* some parsers other than positioned fail on some inputs; report failure if comparing *)
          failed := args.parser = POSITIONED || args.mode = COMPARE;
          Printf.printf
            "Either crashed: %s (%b vs %b)\n"
            path
            (not ok_ocaml)
            (not ok_rust)
        | _ -> ());
        flush stdout;

        incr total;
        if (not ok_ocaml) || not ok_rust then
          incr crashed
        else if not !failed then
          incr correct
        else
          Printf.printf "FAILED %s\n" path;

        let is_compare = args.mode = COMPARE in
        if is_compare || !crashed <> 0 then
          Printf.printf
            "%s/%d (crashed=%d)\n"
            ( if is_compare then
              string_of_int !correct
            else
              "?" )
            !total
            !crashed;
        if !failed && not args.keep_going then exit 1

      (* WithTreeBuilder *)
    end

    include WithTreeBuilder (struct
      type t = SyntaxTree.t

      let make ~env source_text = SyntaxTree.make ~env source_text
    end)

    (* WithSmartConstructors *)
  end

  include WithSmartConstructors (SyntaxSmartConstructors.WithSyntax (Syntax))
end

(* WithSyntax *)

module type SingleRunner_S = sig
  val test :
    args ->
    ocaml_env:Env.t ->
    rust_env:Env.t ->
    Relative_path.t ->
    string ->
    unit
end

module Runner (SingleRunner : SingleRunner_S) = struct
  let test_multi args ~ocaml_env ~rust_env path =
    (* Some typechecked files embed multiple files; they're invalid without a split *)
    Relative_path.(create Dummy path)
    |> Multifile.file_to_files
    |> Relative_path.Map.iter ~f:(SingleRunner.test args ~ocaml_env ~rust_env)

  let test_batch args ~ocaml_env ~rust_env paths =
    List.iter paths ~f:(test_multi args ~ocaml_env ~rust_env)
end

let get_files_in_path ~args path =
  let files = Find.find [Path.make path] in
  let filter_re = Str.regexp args.filter in
  let matches_filter f =
    args.filter = ""
    || (try Str.search_forward filter_re f 0 >= 0 with Not_found -> false)
  in
  List.filter
    ~f:(fun f ->
      ( String_utils.string_ends_with f ".php"
      || String_utils.string_ends_with f ".hhi"
      || String_utils.string_ends_with f ".hack" )
      && matches_filter f
      &&
      match args.parser with
      | COROUTINE -> true
      | DECL_MODE ->
        (* Note: these crash in both OCaml and Rust version of positioned DeclMode parser *)
        (not @@ String_utils.string_ends_with f "ffp/yield_bad1.php")
        && (not @@ String_utils.string_ends_with f "ffp/yield_from_bad1.php")
        && (not @@ String_utils.string_ends_with f "let/let_closure.php")
        && (not @@ String_utils.string_ends_with f "let/let_lambda.php")
        && not
           @@ String_utils.string_ends_with f "test_variadic_type_hint.php"
        && not
           @@ String_utils.string_ends_with f "namespace_group_use_decl.php"
        && (not @@ String_utils.string_ends_with f "parser_massive_add_exp.php")
        && not
           @@ String_utils.string_ends_with f "parser_massive_concat_exp.php"
        && true
      | LOWERER ->
        (not @@ String_utils.string_ends_with f "parser_massive_add_exp.php")
        && not
           @@ String_utils.string_ends_with f "parser_massive_concat_exp.php"
        && not
           @@ String_utils.string_ends_with
                f
                "parser_reasonable_nested_array.php"
        && (not @@ String_utils.string_ends_with f "bug64555.php")
        && (not @@ String_utils.string_ends_with f "bug64660.php")
      | _ -> true)
    files

let get_files args =
  match args.dir with
  | None -> get_files_in_path (fbcode ^ "hphp/hack/test/") ~args
  | Some dir -> get_files_in_path dir ~args

let parse_args () =
  let mode = ref COMPARE in
  let parser = ref MINIMAL in
  let is_experimental = ref false in
  let codegen = ref false in
  let hhvm_compat_mode = ref false in
  let php5_compat_mode = ref false in
  let check_sizes = ref false in
  let check_json_equal_only = ref false in
  let check_printed_tree = ref false in
  let keep_going = ref false in
  let filter = ref "" in
  let dir = ref None in
  let options =
    [
      ("--rust", Arg.Unit (fun () -> mode := RUST), "");
      ("--ocaml", Arg.Unit (fun () -> mode := OCAML), "");
      ("--positioned", Arg.Unit (fun () -> parser := POSITIONED), "");
      ("--coroutine", Arg.Unit (fun () -> parser := COROUTINE), "");
      ( "--coroutine-errors",
        Arg.Unit (fun () -> parser := COROUTINE_ERRORS),
        "" );
      ( "--decl-mode",
        Arg.Unit
          (fun () ->
            parser := DECL_MODE;
            check_json_equal_only := true),
        "" );
      ("--ppl-rewriter", Arg.Unit (fun () -> parser := PPL_REWRITER), "");
      ("--experimental", Arg.Set is_experimental, "");
      ("--lower", Arg.Unit (fun () -> parser := LOWERER), "");
      ("--codegen", Arg.Set codegen, "");
      ("--hhvm-compat-mode", Arg.Set hhvm_compat_mode, "");
      ("--php5-compat-mode", Arg.Set php5_compat_mode, "");
      ("--check-sizes", Arg.Set check_sizes, "");
      ("--check-json-equal-only", Arg.Set check_json_equal_only, "");
      ("--check-printed-tree", Arg.Set check_printed_tree, "");
      ("--keep-going", Arg.Set keep_going, "");
      ("--filter", Arg.String (fun s -> filter := s), "");
      ("--dir", Arg.String (fun s -> dir := Some s), "");
      ( "--hhvm-tests",
        Arg.Unit (fun () -> dir := Some (fbcode ^ "hphp/test/")),
        "" );
    ]
  in
  Arg.parse options (fun _ -> ()) "";
  {
    mode = !mode;
    parser = !parser;
    is_experimental = !is_experimental;
    codegen = !codegen;
    hhvm_compat_mode = !hhvm_compat_mode;
    php5_compat_mode = !php5_compat_mode;
    check_sizes = !check_sizes;
    check_json_equal_only = !check_json_equal_only;
    check_printed_tree = !check_printed_tree;
    keep_going = !keep_going;
    filter = !filter;
    dir = !dir;
  }

module MinimalTest = Runner (WithSyntax (MinimalSyntax))
module PositionedTest_ = WithSyntax (PositionedSyntax)
module PositionedTest = Runner (PositionedTest_)
module CoroutineTest__ = WithSyntax (PositionedSyntax)
module CoroutineSC = Coroutine_smart_constructor.WithSyntax (PositionedSyntax)
module CoroutineTest_ = CoroutineTest__.WithSmartConstructors (CoroutineSC)
module CoroutineTest = Runner (CoroutineTest_)

module CoroutineErrorsTest_ = CoroutineTest_.WithTreeBuilder (struct
  module ParserErrors_ =
    Full_fidelity_parser_errors.WithSyntax (PositionedSyntax)
  module ParserErrors = ParserErrors_.WithSmartConstructors (CoroutineSC)

  type t = CoroutineTest_.SyntaxTree.t

  let make ~env source_text =
    (* We only care about errors here *)
    let fake_root = PositionedSyntax.make_missing source_text 0 in
    (* TODO:
      - make parser_options configurable and use them
      - make the arguments to ParserErrors.make_env configurable and use them
    *)
    let parser_options = ParserOptions.default in
    let tree = CoroutineTest_.SyntaxTree.make ~env source_text in
    let errors =
      ParserErrors.(
        make_env ~parser_options ~codegen:false tree |> parse_errors)
    in
    CoroutineTest_.SyntaxTree.build
      source_text
      fake_root
      None
      errors
      None
      false
end)

module CoroutineErrorsTest = Runner (CoroutineErrorsTest_)
module DeclModeTest_ = WithSyntax (PositionedSyntax)
module DeclModeSC = DeclModeSmartConstructors.WithSyntax (PositionedSyntax)
module DeclModeTest = Runner (DeclModeTest_.WithSmartConstructors (DeclModeSC))
module EditablePositionedSyntaxSC =
  SyntaxSmartConstructors.WithSyntax (EditablePositionedSyntax)
module PPLRewriterTest___ = WithSyntax (EditablePositionedSyntax)
module PPLRewriterTest__ =
  PPLRewriterTest___.WithSmartConstructors (EditablePositionedSyntaxSC)

module PPLRewriterTest_ = PPLRewriterTest__.WithTreeBuilder (struct
  module EditableSyntaxTree = PPLRewriterTest__.SyntaxTree

  type t = EditableSyntaxTree.t

  let make ~env source_text =
    let root =
      if Env.rust env then
        Ppl_class_rewriter_ffi.parse_and_rewrite_ppl_classes source_text
      else
        PositionedTest_.SyntaxTree.(make source_text |> root)
        |> EditablePositionedSyntax.from_positioned_syntax
        |> Ppl_class_rewriter.rewrite_ppl_classes
    in
    (* We don't care about errors / mode / state here *)
    EditableSyntaxTree.build source_text root None [] None ()
end)

module PPLRewriterTest = Runner (PPLRewriterTest_)

module LowererTest_ = struct
  module OcamlLowerer = Full_fidelity_ast
  module RustLowerer = Lowerer_ffi

  type r =
    | Tree of (Ast_defs.pos, unit, unit, unit) Aast.program
    | Crash of string

  let print_err path s =
    let oc = Pervasives.open_out path in
    Printf.fprintf oc "%s\n" s

  let print_aast path aast =
    let print_pos pos = Format.asprintf "(%a)" Pos.pp pos in
    let pp_pos fmt pos = Format.pp_print_string fmt (print_pos pos) in
    let pp_unit fmt _ = Format.pp_print_string fmt "" in
    let oc = Pervasives.open_out path in
    Printf.fprintf
      oc
      "%s\n"
      (Aast.show_program pp_pos pp_unit pp_unit pp_unit aast)

  let build_ocaml_tree _env file source_text =
    let i x = x in
    let env =
      OcamlLowerer.make_env file ~keep_errors:false ~elaborate_namespaces:false
    in
    try
      (Errors.is_hh_fixme := (fun _ _ -> false));
      (Errors.get_hh_fixme_pos := (fun _ _ -> None));
      (Errors.is_hh_fixme_disallowed := (fun _ _ -> false));
      let ast = OcamlLowerer.from_text env source_text in
      let aast = Ast_to_aast.convert_program i () () () ast.OcamlLowerer.ast in
      Tree aast
    with e -> Crash (Caml.Printexc.to_string e)

  let build_rust_tree _env file source_text =
    let env = OcamlLowerer.make_env file in
    try
      let result = RustLowerer.from_text_rust env source_text in
      match result with
      | RustLowerer.Ok aast -> Tree aast.RustLowerer.ast
      | RustLowerer.Err s -> Crash s
    with e -> Crash (Caml.Printexc.to_string e)

  let test args ~ocaml_env ~rust_env file contents =
    let source_text = SourceText.make file contents in
    let path = Relative_path.to_absolute file in
    let ocaml_tree = build_ocaml_tree ocaml_env file source_text in
    let rust_tree = build_rust_tree rust_env file source_text in
    (match (ocaml_tree, rust_tree) with
    | (Tree ot, Tree rt) when ot = rt -> Printf.printf ":EQUAL: "
    | (Tree _, Tree _) -> Printf.printf ":NOT_EQUAL: "
    | (Tree _, Crash _) -> Printf.printf ":OCAML_PASS: :RUST_CRASH: "
    | (Crash _, Tree _) -> Printf.printf ":OCAML_CRASH: :RUST_PASS: "
    | (Crash _, Crash _) -> Printf.printf ":OCAML_CRASH: :RUST_CRASH: ");
    Printf.printf "%s\n" path;
    flush stdout;
    if args.check_printed_tree then (
      (match ocaml_tree with
      | Tree t -> print_aast "/tmp/ocaml.aast" t
      | Crash e -> print_err "/tmp/ocaml.aast" e);
      match rust_tree with
      | Tree t -> print_aast "/tmp/rust.aast" t
      | Crash e -> print_err "/tmp/rust.aast" e
    )
end

module LowererTest = Runner (LowererTest_)

(*
Tool comparing outputs of Rust and OCaml parsers. Example usage:

  buck run @mode/opt-clang hphp/hack/test/rust:rust_ocaml

See parse_args for all the options
*)
let () =
  let args = parse_args () in
  let files = get_files args in
  Hh_logger.log "Starting...";
  let t = Unix.gettimeofday () in
  let mode =
    if args.is_experimental then
      Some FileInfo.Mexperimental
    else
      None
  in
  let make_env =
    Full_fidelity_parser_env.make
      ~hhvm_compat_mode:args.hhvm_compat_mode
      ~php5_compat_mode:args.php5_compat_mode
      ~codegen:args.codegen
      ~leak_rust_tree:(args.parser = COROUTINE_ERRORS)
      ?mode
  in
  let ocaml_env = make_env ~rust:false () in
  let rust_env = make_env ~rust:true () in
  let f =
    match args.parser with
    | MINIMAL -> MinimalTest.test_batch args ~ocaml_env ~rust_env
    | POSITIONED -> PositionedTest.test_batch args ~ocaml_env ~rust_env
    | COROUTINE -> CoroutineTest.test_batch args ~ocaml_env ~rust_env
    | COROUTINE_ERRORS ->
      CoroutineErrorsTest.test_batch args ~ocaml_env ~rust_env
    | DECL_MODE -> DeclModeTest.test_batch args ~ocaml_env ~rust_env
    | PPL_REWRITER -> PPLRewriterTest.test_batch args ~ocaml_env ~rust_env
    | LOWERER -> LowererTest.test_batch args ~ocaml_env ~rust_env
  in
  let (user, runs, _mem) =
    Profile.profile_longer_than (fun () -> f files) ~retry:false 0.
  in
  ignore (Hh_logger.log_duration "Done:" t);
  ignore (Hh_logger.log "User:: %f" user);
  ignore (Hh_logger.log "Runs:: %d" runs);
  ()
