(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module OcamlPrintf = Printf
open Hh_prelude
module Printf = OcamlPrintf

[@@@warning "-3"]

module SourceText = Full_fidelity_source_text
module SyntaxError = Full_fidelity_syntax_error
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
  | LOWERER

type mode =
  | RUST
  | OCAML
  | COMPARE

type args = {
  mode: mode;
  parser: parser;
  hhvm_compat_mode: bool;
  php5_compat_mode: bool;
  codegen: bool;
  check_sizes: bool;
  check_json_equal_only: bool;
  check_printed_tree: bool;
  keep_going: bool;
  filter: string;
  dir: string option;
  no_tree_compare: bool;
  ignore_lid: bool;
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
        Syntax.to_json ~with_value:true x |> Hh_json.json_to_string ~pretty:true

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
              let oc = Stdlib.open_out "/tmp/rust.php" in
              Printf.fprintf oc "%s\n" text_from_rust;
              close_out oc;
              let oc = Stdlib.open_out "/tmp/ocaml.php" in
              Printf.fprintf oc "%s\n" text_from_ocaml;
              close_out oc;
              Printf.printf "Printed tree not equal: %s\n" path;
              failed := true
            | (Some _, Some _) -> () (* equal *)
            | _ ->
              Printf.printf
                "Tree to source transformation is not supported for this syntax type\n";
              failed := true );
          if not @@ args.no_tree_compare then (
            if syntax_from_rust <> syntax_from_ocaml then (
              let syntax_from_rust_as_json = to_json syntax_from_rust in
              let syntax_from_ocaml_as_json = to_json syntax_from_ocaml in
              let oc = Stdlib.open_out "/tmp/rust.json" in
              Printf.fprintf oc "%s\n" syntax_from_rust_as_json;
              close_out oc;
              let oc = Stdlib.open_out "/tmp/ocaml.json" in
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
            )
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
      (not (Sys.is_directory f))
      && ( String_utils.string_ends_with f ".php"
         || String_utils.string_ends_with f ".hhi"
         || String_utils.string_ends_with f ".hack" )
      && matches_filter f
      &&
      match args.parser with
      | LOWERER ->
        (* TODO(shiqicao): parser_massive_add_exp.php and parser_massive_concat_exp.php crashs
          Ocaml with SYNTAX ERROR: Expression recursion limit reached. Rust doesn't crash,
          but we still need to set a limit for Rust lowerer.
         *)
        (not @@ String_utils.string_ends_with f "parser_massive_add_exp.php")
        && not
           @@ String_utils.string_ends_with f "parser_massive_concat_exp.php"
      | CLOSURE_CONVERT ->
        (* Needs elastic stack support to pass *)
        (not @@ String_utils.string_ends_with f "parser_massive_add_exp.php")
        && (not @@ String_utils.string_ends_with f "massive_concat_exp.php")
        && (not @@ String_utils.string_ends_with f "reasonable_nested_array.php")
        && (not @@ String_utils.string_ends_with f "bug64660.php")
        (* Bug in lowerer *)
        && not
           @@ String_utils.string_ends_with
                f
                "cases/await_as_an_expression/await_as_an_expression_simple.php"
        && true
      | _ -> true)
    files

let get_files args =
  match args.dir with
  | None -> get_files_in_path (fbcode ^ "hphp/hack/test/") ~args
  | Some dir -> get_files_in_path dir ~args

let parse_args () =
  let mode = ref COMPARE in
  let parser = ref MINIMAL in
  let codegen = ref false in
  let hhvm_compat_mode = ref false in
  let php5_compat_mode = ref false in
  let check_sizes = ref false in
  let check_json_equal_only = ref false in
  let check_printed_tree = ref false in
  let keep_going = ref false in
  let no_tree_compare = ref false in
  let filter = ref "" in
  let dir = ref None in
  let ignore_lid = ref false in
  let options =
    [
      ("--rust", Arg.Unit (fun () -> mode := RUST), "");
      ("--ocaml", Arg.Unit (fun () -> mode := OCAML), "");
      ("--positioned", Arg.Unit (fun () -> parser := POSITIONED), "");
      ( "--decl-mode",
        Arg.Unit
          (fun () ->
            parser := DECL_MODE;
            check_json_equal_only := true),
        "" );
      ("--lower", Arg.Unit (fun () -> parser := LOWERER), "");
      ("--closure-convert", Arg.Unit (fun () -> parser := CLOSURE_CONVERT), "");
      ("--codegen", Arg.Set codegen, "");
      ("--hhvm-compat-mode", Arg.Set hhvm_compat_mode, "");
      ("--php5-compat-mode", Arg.Set php5_compat_mode, "");
      ("--check-sizes", Arg.Set check_sizes, "");
      ("--check-json-equal-only", Arg.Set check_json_equal_only, "");
      ("--check-printed-tree", Arg.Set check_printed_tree, "");
      ("--no-tree-compare", Arg.Set no_tree_compare, "");
      ("--keep-going", Arg.Set keep_going, "");
      ("--filter", Arg.String (fun s -> filter := s), "");
      ("--dir", Arg.String (fun s -> dir := Some s), "");
      ( "--hhvm-tests",
        Arg.Unit (fun () -> dir := Some (fbcode ^ "hphp/test/")),
        "" );
      ("--ignore-lid", Arg.Set ignore_lid, "");
    ]
  in
  Arg.parse options (fun _ -> ()) "";
  {
    mode = !mode;
    parser = !parser;
    codegen = !codegen;
    hhvm_compat_mode = !hhvm_compat_mode;
    php5_compat_mode = !php5_compat_mode;
    check_sizes = !check_sizes;
    check_json_equal_only = !check_json_equal_only;
    check_printed_tree = !check_printed_tree;
    keep_going = !keep_going;
    filter = !filter;
    dir = !dir;
    no_tree_compare = !no_tree_compare;
    ignore_lid = !ignore_lid;
  }

module PositionedTest_ = WithSyntax (PositionedSyntax)
module PositionedTest = Runner (PositionedTest_)
module EditablePositionedSyntaxSC =
  SyntaxSmartConstructors.WithSyntax (EditablePositionedSyntax)

module LowererTest_ = struct
  module Lowerer = Full_fidelity_ast

  type r =
    | Tree of Rust_aast_parser_types.result
    | Crash of string
    | Skip

  let print_err path s =
    let oc = Stdlib.open_out path in
    Printf.fprintf oc "%s\n" s

  let print_lid ~skip_lid fmt lid =
    Format.pp_print_string
      fmt
      ( if skip_lid then
        let name = Local_id.get_name lid in
        let name =
          if Naming_special_names.SpecialIdents.is_tmp_var name then
            "[tmp_var]"
          else
            name
        in
        Format.asprintf "([id], %s)" name
      else
        Format.asprintf "(%d, %s)" (Local_id.to_int lid) (Local_id.get_name lid)
      )

  let print_pos pos = Format.asprintf "(%a)" Pos.pp pos

  let print_aast_result ?(skip_lid = false) r =
    Local_id.pp_ref := print_lid ~skip_lid;
    let pp_pos fmt pos = Format.pp_print_string fmt (print_pos pos) in
    let pp_unit fmt _ = Format.pp_print_string fmt "" in
    match r with
    | Ok aast -> Aast.show_program pp_pos pp_unit pp_unit pp_unit aast
    | Error msg -> Printf.sprintf "SYNTAX ERROR: %s" msg

  let print_result args path result =
    let print_lowpri_err (p, e) = Printf.sprintf "[%s %s]" (print_pos p) e in
    let print_lowpri_errs errs =
      String.concat ~sep:"\n" @@ List.map ~f:print_lowpri_err errs
    in
    let print_errs errs =
      String.concat ~sep:"\n"
      @@ List.map ~f:Full_fidelity_syntax_error.show errs
    in
    let oc = Stdlib.open_out path in
    Rust_aast_parser_types.(
      Printf.fprintf
        oc
        "%s\n%s\n%s\n%s"
        (print_aast_result ~skip_lid:args.ignore_lid result.aast)
        (Scoured_comments.show result.scoured_comments)
        (print_lowpri_errs result.lowpri_errors)
        (print_errs result.syntax_errors))

  let lower lower_env source_text =
    (Errors.is_hh_fixme := (fun _ _ -> false));
    (Errors.get_hh_fixme_pos := (fun _ _ -> None));
    (Errors.is_hh_fixme_disallowed := (fun _ _ -> false));
    let (_err, r) =
      Errors.do_ (fun () -> Lowerer.from_text_rust lower_env source_text)
    in
    r

  let build_tree args _env file source_text =
    let popt =
      {
        ParserOptions.default with
        GlobalOptions.po_disable_xhp_children_declarations = true;
        GlobalOptions.po_disable_xhp_element_mangling = true;
        GlobalOptions.po_allow_unstable_features = true;
      }
    in
    let lower_env =
      Lowerer.make_env
        file
        ~codegen:args.codegen
        ~disable_global_state_mutation:true
        ~show_all_errors:true
        ~keep_errors:true
        ~elaborate_namespaces:true
        ~parser_options:popt
    in
    try Tree (lower lower_env source_text)
    with e -> Crash (Caml.Printexc.to_string e)

  let test args ~ocaml_env ~rust_env file contents =
    let source_text = SourceText.make file contents in
    let path = Relative_path.to_absolute file in
    Printf.printf "Start %s\n" path;
    flush stdout;
    let ocaml_tree =
      match args.mode with
      | RUST -> Skip
      | _ -> build_tree args ocaml_env file source_text
    in
    let rust_tree =
      match args.mode with
      | OCAML -> Skip
      | _ -> build_tree args rust_env file source_text
    in
    let aast_equal =
      (if args.ignore_lid then Local_id.equal_ref := (fun _ _ -> true));
      Aast.equal_program ( = ) ( = ) ( = ) ( = )
    in
    let compare_result r1 r2 =
      Rust_aast_parser_types.(
        let compare_aast_result a1 a2 =
          match (a1, a2) with
          | (Ok aast1, Ok aast2) -> aast_equal aast1 aast2
          | (Error msg1, Error msg2) -> msg1 = msg2
          | _ -> false
        in
        let tree = compare_aast_result r1.aast r2.aast in
        let file_mode = r1.file_mode = r2.file_mode in
        let lowpri_err = r1.lowpri_errors = r2.lowpri_errors in
        let err = r1.syntax_errors = r2.syntax_errors in
        let comments =
          Scoured_comments.equal r1.scoured_comments r2.scoured_comments
        in
        if tree && file_mode && lowpri_err && err && comments then
          Printf.printf ":EQUAL: "
        else
          let print s r =
            Printf.printf
              ":%s_%sEQUAL: "
              s
              ( if r then
                ""
              else
                "NOT_" )
          in
          Printf.printf ":NOT_EQUAL: ";
          print "Tree" tree;
          print "Comments" comments;
          print "Mode" file_mode;
          print "Lerr" lowpri_err;
          print "Err" err)
    in
    let print_lowpri_errs o_le r_le =
      let print_pos pos = Format.asprintf "(%a)" Pos.pp pos in
      let print_err pre (p, e) =
        Printf.printf "%s: %s, %s\n%s_MSG:%s\n" pre (print_pos p) e pre e
      in
      let print_errs pre es = List.iter es ~f:(print_err pre) in
      Printf.printf "OCAML_LP_ERR: %s\n" path;
      print_errs "OCAML_LP_ERR" o_le;
      Printf.printf "RUST_LP_ERR: %s\n" path;
      print_errs "RUST_LP_ERR" r_le
    in
    (match (ocaml_tree, rust_tree) with
    | (Tree ot, Tree rt) -> compare_result ot rt
    | (_, _) -> ());
    (match ocaml_tree with
    | Tree _ -> Printf.printf ":OCAML_PASS: "
    | Crash e -> Printf.printf ":OCAML_CRASH(%s): " e
    | Skip -> Printf.printf ":OCAML_SKIP: ");
    (match rust_tree with
    | Tree _ -> Printf.printf ":RUST_PASS: "
    | Crash e -> Printf.printf ":RUST_CRASH(%s): " e
    | Skip -> Printf.printf ":RUST_SKIP: ");
    Printf.printf "%s\n" path;
    Rust_aast_parser_types.(
      match (ocaml_tree, rust_tree) with
      | (Tree ot, Tree rt) when ot.lowpri_errors <> rt.lowpri_errors ->
        print_lowpri_errs ot.lowpri_errors rt.lowpri_errors
      | (_, _) -> ());
    flush stdout;
    if args.check_printed_tree then (
      (match ocaml_tree with
      | Tree r -> print_result args "/tmp/ocaml.aast" r
      | Crash e -> print_err "/tmp/ocaml.aast" e
      | _ -> ());
      match rust_tree with
      | Tree r -> print_result args "/tmp/rust.aast" r
      | Crash e -> print_err "/tmp/rust.aast" e
      | _ -> ()
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
  let make_env =
    Full_fidelity_parser_env.make
      ~hhvm_compat_mode:args.hhvm_compat_mode
      ~php5_compat_mode:args.php5_compat_mode
      ~codegen:args.codegen
      ~leak_rust_tree:false
  in
  let ocaml_env = make_env ~rust:false () in
  let rust_env = make_env ~rust:true () in
  let f =
    match args.parser with
    | MINIMAL -> MinimalTest.test_batch args ~ocaml_env ~rust_env
    | POSITIONED -> PositionedTest.test_batch args ~ocaml_env ~rust_env
    | LOWERER -> LowererTest.test_batch args ~ocaml_env ~rust_env
  in
  let (user, runs, _mem) =
    Profile.profile_longer_than (fun () -> f files) ~retry:false 0.
  in
  ignore (Hh_logger.log_duration "Done:" t);
  ignore (Hh_logger.log "User:: %f" user);
  ignore (Hh_logger.log "Runs:: %d" runs);
  ()
