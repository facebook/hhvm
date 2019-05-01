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
   check_sizes : bool;
   keep_going : bool;
   dir : string option;
}

module type Parser_S = sig
  type r
  val parse : SourceText.t -> Env.t -> (FileInfo.mode option * r * SyntaxError.t list)
end

module WithSyntax(Syntax : Syntax_sig.Syntax_S) = struct
module WithParser(Parser : Parser_S with type r = Syntax.t) = struct

module SyntaxTree = Full_fidelity_syntax_tree.WithSyntax(Syntax)

let rust_parse parse source_text env =
  let mode, root, errors = parse source_text env in
  SyntaxTree.create source_text root errors mode

let ocaml_parse env source_text =
  SyntaxTree.make ~env source_text

let syntax_tree_into_parts tree =
  let mode, root, errors = SyntaxTree.(mode tree, root tree, errors tree) in
  mode, root, errors

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

let test args env path =
  if args.mode == COMPARE then Printf.printf "%d/%d\n" !correct !total;
  Printf.printf "%s\n" path;
  flush(stdout);
  incr total;

  let file = Relative_path.(create Dummy (path)) in
  let source_text = SourceText.from_file file in

  let from_rust = match args.mode with
    | RUST | COMPARE -> Some (rust_parse Parser.parse source_text env)
    | OCAML -> None
  in
  let from_ocaml = match args.mode with
    | OCAML | COMPARE -> Some (ocaml_parse env source_text)
    | RUST -> None
  in

  match from_rust, from_ocaml with
  | Some from_rust, Some from_ocaml -> begin
      let (mode_from_rust, syntax_from_rust, errors_from_rust)
        = syntax_tree_into_parts from_rust in
      let (mode_from_ocaml, syntax_from_ocaml, errors_from_ocaml)
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

        Printf.printf "Not equal: %s\n" path;
        flush(stdout);
        if not args.keep_going then exit 1
      end else if args.check_sizes && rust_reachable_words <> ocaml_reachable_words  then begin
        Printf.printf "Sizes not equal: %s (%d vs %d)\n"
          path rust_reachable_words ocaml_reachable_words;
        if not args.keep_going then exit 1
      end else if mode_from_rust <> mode_from_ocaml then begin
        Printf.printf "Modes not equal: %s (%s vs %s)\n" path
          (mode_to_string mode_from_ocaml) (mode_to_string mode_from_rust);
        if not args.keep_going then exit 1
      end else if errors_from_rust <> errors_from_ocaml then begin
        Printf.printf "Errors not equal: %s\n" path;
        Printf.printf "---OCaml errors---\n";

        List.iter ~f:(print_full_fidelity_error source_text) errors_from_ocaml;
        Printf.printf "---Rust erors---\n";
        List.iter ~f:(print_full_fidelity_error source_text) errors_from_rust;

        if not args.keep_going then exit 1
      end else begin
        incr correct
      end
    end
  | _ -> ()

let test_batch args env files =
  List.iter files ~f:(test args env)
end end

let get_files_in_path path =
  let files = Find.find [Path.make path] in
  List.filter ~f:begin fun f ->
    (
      String_utils.string_ends_with f ".php" ||
      String_utils.string_ends_with f ".hhi" ||
      String_utils.string_ends_with f ".hack"
    ) &&
    (not @@ String_utils.string_ends_with f "memory_exhaust.php") &&
    (not @@ String_utils.string_ends_with f "parser_massive_concat_exp.php") &&
    (not @@ String_utils.string_ends_with f "parser_massive_add_exp.php") &&
    (not @@ String_utils.string_ends_with f "byref-assignment-lvarvar.php") &&
    (not @@ String_utils.string_ends_with f "giant-arrays.php") &&
    (not @@ String_utils.string_ends_with f "byref-assignment4.php") &&
    (not @@ String_utils.string_ends_with f "byref-assignment3.php") &&
    (not @@ String_utils.string_ends_with f "byref-assignment2.php") &&
    (not @@ String_utils.string_ends_with f "byref-assignment1.php") &&
    (not @@ String_utils.string_ends_with f "full_analysis/www_repro_2.php") &&
    (not @@ String_utils.string_ends_with f "byref1.php") &&
    (not @@ String_utils.string_ends_with f "byref2.php") &&
    (not @@ String_utils.string_ends_with f "phpvar1.php") &&
    (not @@ String_utils.string_ends_with f "bug64660.php") &&
    true
  end files

let get_files args =
  match args.dir with
  | None -> (get_files_in_path (fbcode ^ "hphp/hack/test/"))
  | Some dir -> get_files_in_path dir

let parse_args () =
  let mode = ref COMPARE in
  let parser = ref MINIMAL in
  let is_experimental = ref false in
  let enable_stronger_await_binding = ref false in
  let disable_unsafe_expr = ref false in
  let disable_unsafe_block = ref false in
  let force_hh = ref false in
  let enable_xhp = ref false in
  let hhvm_compat_mode = ref false in
  let php5_compat_mode = ref false in
  let check_sizes = ref false in
  let keep_going = ref false in
  let dir = ref None in

  let options =  [
    "--rust", Arg.Unit (fun () -> mode := RUST), "";
    "--ocaml", Arg.Unit (fun () -> mode := OCAML), "";
    "--positioned", Arg.Unit (fun () -> parser := POSITIONED), "";
    "--experimental", Arg.Set is_experimental, "";
    "--enable-stronger-await-binding", Arg.Set enable_stronger_await_binding, "";
    "--disable-unsafe-expr", Arg.Set disable_unsafe_expr, "";
    "--disable-unsafe-block", Arg.Set disable_unsafe_block, "";
    "--force-hh", Arg.Set force_hh, "";
    "--enable-xhp", Arg.Set enable_xhp, "";
    "--hhvm-compat-mode", Arg.Set hhvm_compat_mode, "";
    "--php5-compat-mode", Arg.Set php5_compat_mode, "";
    "--check-sizes", Arg.Set check_sizes, "";
    "--keep-going", Arg.Set keep_going, "";
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
    force_hh = !force_hh;
    enable_xhp = !enable_xhp;
    hhvm_compat_mode = !hhvm_compat_mode;
    php5_compat_mode = !php5_compat_mode;
    check_sizes = !check_sizes;
    keep_going = !keep_going;
    dir = !dir;
  }

module MinimalParser = struct
  type r = MinimalSyntax.t
  let parse = Rust_parser_ffi.parse_minimal
end

module PositionedParser = struct
  type r = PositionedSyntax.t
  let parse = Rust_parser_ffi.parse_positioned
end

module MinimalTest_ = WithSyntax(MinimalSyntax)
module MinimalTest = MinimalTest_.WithParser(MinimalParser)

module PositionedTest_ = WithSyntax(PositionedSyntax)
module PositionedTest = PositionedTest_.WithParser(PositionedParser)
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
  let env = Full_fidelity_parser_env.make
    ~enable_stronger_await_binding:args.enable_stronger_await_binding
    ~disable_unsafe_expr:args.disable_unsafe_expr
    ~disable_unsafe_block:args.disable_unsafe_block
    ~force_hh:args.force_hh
    ~enable_xhp:args.enable_xhp
    ~hhvm_compat_mode:args.hhvm_compat_mode
    ~php5_compat_mode:args.php5_compat_mode
    ?mode
    ()
  in
  begin match args.parser with
    | MINIMAL -> MinimalTest.test_batch args env files
    | POSITIONED -> PositionedTest.test_batch args env files
  end;
  let _ = Hh_logger.log_duration "Done:" t  in
  ()
