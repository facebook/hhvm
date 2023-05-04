(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Direct_decl_parser

let popt
    ~auto_namespace_map
    ~enable_xhp_class_modifier
    ~disable_xhp_element_mangling
    ~interpret_soft_types_as_like_types
    ~everything_sdt =
  let po = ParserOptions.default in
  let po =
    ParserOptions.with_disable_xhp_element_mangling
      po
      disable_xhp_element_mangling
  in
  let po = ParserOptions.with_auto_namespace_map po auto_namespace_map in
  let po =
    ParserOptions.with_enable_xhp_class_modifier po enable_xhp_class_modifier
  in
  let po =
    ParserOptions.with_interpret_soft_types_as_like_types
      po
      interpret_soft_types_as_like_types
  in
  let po = ParserOptions.with_everything_sdt po everything_sdt in
  po

let init root popt : Provider_context.t =
  Relative_path.(set_path_prefix Root root);
  let (_handle : SharedMem.handle) =
    SharedMem.init ~num_workers:0 SharedMem.default_config
  in
  let tcopt = { popt with GlobalOptions.tco_higher_kinded_types = true } in
  let ctx =
    Provider_context.empty_for_tool
      ~popt
      ~tcopt
      ~backend:Provider_backend.Shared_memory
      ~deps_mode:(Typing_deps_mode.InMemoryMode None)
      ~package_info:Package.Info.empty
  in

  (* Push local stacks here so we don't include shared memory in our timing. *)
  File_provider.local_changes_push_sharedmem_stack ();
  Decl_provider.local_changes_push_sharedmem_stack ();
  Shallow_classes_provider.local_changes_push_sharedmem_stack ();

  ctx

let direct_decl_parse ctx fn text =
  let popt = Provider_context.get_popt ctx in
  let opts = DeclParserOptions.from_parser_options popt in
  let parsed_file = parse_decls opts fn text in
  parsed_file.pf_decls

let parse_and_print_decls ctx fn text =
  let (ctx, _entry) =
    Provider_context.(
      add_or_overwrite_entry_contents ~ctx ~path:fn ~contents:text)
  in
  let decls = direct_decl_parse ctx fn text in
  let decls_str = show_decls (List.rev decls) ^ "\n" in
  Printf.eprintf "%s%!" decls_str;
  (* This mode doesn't compare anything. Return false so that we don't print "They matched!". *)
  let matched = false in
  matched

let compare_marshal ctx fn text =
  let (ctx, _entry) =
    Provider_context.(
      add_or_overwrite_entry_contents ~ctx ~path:fn ~contents:text)
  in
  let decls = direct_decl_parse ctx fn text in
  (* Test that Rust produces the same marshaled bytes as OCaml. *)
  let ocaml_marshaled = Marshal.to_string decls [] in
  let rust_marshaled = Ocamlrep_marshal_ffi.to_string decls [] in
  let marshaled_bytes_matched = String.equal ocaml_marshaled rust_marshaled in
  let () =
    if not marshaled_bytes_matched then begin
      Printf.printf
        "OCaml Marshal output does not match Rust ocamlrep_marshal output:\n%!";
      Printf.printf "ocaml:\t%S\n%!" ocaml_marshaled;
      Printf.printf "rust:\t%S\n%!" rust_marshaled
    end
  in
  (* Test that Rust unmarshaling works as expected. *)
  let rust_read_back_decls =
    Ocamlrep_marshal_ffi.from_string rust_marshaled 0
  in
  let rust_read_back_matched =
    String.equal (show_decls decls) (show_decls rust_read_back_decls)
  in
  let () =
    if not rust_read_back_matched then begin
      Printf.printf
        "Rust ocamlrep_marshal from_string decl read-back failed:\n%!";
      Printf.printf "ocaml:\n%s%!" (show_decls decls);
      Printf.printf "rust:\n%s%!" (show_decls rust_read_back_decls)
    end
  in
  marshaled_bytes_matched && rust_read_back_matched

type modes =
  | DirectDeclParse
  | VerifyOcamlrepMarshal

let () =
  let usage =
    Printf.sprintf "Usage: %s [OPTIONS] mode filename\n" Sys.argv.(0)
  in
  let usage_and_exit () =
    prerr_endline usage;
    exit 1
  in
  let mode = ref None in
  let set_mode m () =
    match !mode with
    | None -> mode := Some m
    | Some _ -> usage_and_exit ()
  in
  let file = ref None in
  let set_file f =
    match !file with
    | None -> file := Some f
    | Some _ -> usage_and_exit ()
  in
  let skip_if_errors = ref false in
  let expect_extension = ref ".exp" in
  let set_expect_extension s = expect_extension := s in
  let auto_namespace_map = ref [] in
  let enable_xhp_class_modifier = ref false in
  let disable_xhp_element_mangling = ref false in
  let disallow_static_memoized = ref false in
  let interpret_soft_types_as_like_types = ref false in
  let everything_sdt = ref false in
  let ignored_flag flag = (flag, Arg.Unit (fun _ -> ()), "(ignored)") in
  let ignored_arg flag = (flag, Arg.String (fun _ -> ()), "(ignored)") in
  Arg.parse
    [
      ( "--decl-parse",
        Arg.Unit (set_mode DirectDeclParse),
        "(mode) Runs the direct decl parser on the given file" );
      ( "--verify-ocamlrep-marshal",
        Arg.Unit (set_mode VerifyOcamlrepMarshal),
        "(mode) Marshals the output of the direct decl parser using Marshal and ocamlrep_marshal and compares their output"
      );
      ( "--skip-if-errors",
        Arg.Set skip_if_errors,
        "Skip comparison if the corresponding .exp file has errors" );
      ( "--expect-extension",
        Arg.String set_expect_extension,
        "The extension with which the output of the legacy pipeline should be written"
      );
      ( "--auto-namespace-map",
        Arg.String
          (fun m ->
            auto_namespace_map := ServerConfig.convert_auto_namespace_to_map m),
        "Namespace aliases" );
      ( "--enable-xhp-class-modifier",
        Arg.Set enable_xhp_class_modifier,
        "Enable the XHP class modifier, xhp class name {} will define an xhp class."
      );
      ( "--disable-xhp-element-mangling",
        Arg.Set disable_xhp_element_mangling,
        "." );
      ( "--disallow-static-memoized",
        Arg.Set disallow_static_memoized,
        " Disallow static memoized methods on non-final methods" );
      ( "--interpret-soft-types-as-like-types",
        Arg.Set interpret_soft_types_as_like_types,
        "Interpret <<__Soft>> type hints as like types" );
      ( "--everything-sdt",
        Arg.Set everything_sdt,
        " Treat all classes, functions, and traits as though they are annotated with <<__SupportDynamicType>>, unless they are annotated with <<__NoAutoDynamic>>"
      );
      (* The following options do not affect the direct decl parser and can be ignored
         (they are used by hh_single_type_check, and we run hh_single_decl over all of
         the typecheck test cases). *)
      ignored_flag "--enable-global-access-check";
      ignored_flag "--abstract-static-props";
      ignored_arg "--allowed-decl-fixme-codes";
      ignored_arg "--allowed-fixme-codes-strict";
      ignored_flag "--allow-toplevel-requires";
      ignored_flag "--check-xhp-attribute";
      ignored_flag "--complex-coercion";
      ignored_flag "--const-attribute";
      ignored_flag "--const-static-props";
      ignored_arg "--disable-hh-ignore-error";
      ignored_flag "--disable-modes";
      ignored_flag "--disable-partially-abstract-typeconsts";
      ignored_flag "--disable-unset-class-const";
      ignored_flag "--disable-xhp-children-declarations";
      ignored_flag "--disallow-discarded-nullable-awaitables";
      ignored_flag "--disallow-fun-and-cls-meth-pseudo-funcs";
      ignored_flag "--disallow-func-ptrs-in-constants";
      ignored_flag "--disallow-invalid-arraykey-constraint";
      ignored_flag "--disallow-php-lambdas";
      ignored_flag "--disallow-silence";
      ignored_flag "--enable-class-level-where-clauses";
      ignored_flag "--enable-higher-kinded-types";
      ignored_flag "--forbid_nullable_cast";
      ( "--hh-log-level",
        Arg.Tuple [Arg.String (fun _ -> ()); Arg.String (fun _ -> ())],
        "(ignored)" );
      ignored_flag "--is-systemlib";
      ignored_flag "--like-casts";
      ignored_flag "--like-type-hints";
      ignored_flag "--like-types-all";
      ignored_flag "--method-call-inference";
      ignored_flag "--no-builtins";
      ignored_flag "--no-strict-contexts";
      ignored_flag "--report-pos-from-reason";
      ignored_arg "--simple-pessimize";
      ignored_arg "--timeout";
      ignored_flag "--union-intersection-type-hints";
      ignored_flag "--enable-strict-string-concat-interp";
      ignored_arg "--extra-builtin";
      ignored_flag "--disallow-inst-meth";
      ignored_flag "--ignore-unsafe-cast";
      ignored_flag "--inc-dec-new-code";
      ignored_flag "--math-new-code";
      ignored_flag "--disallow-partially-abstract-typeconst-definitions";
      ignored_flag "--typeconst-concrete-concrete-error";
      ignored_arg "--enable-strict-const-semantics";
      ignored_arg "--strict-wellformedness";
      ignored_arg "--meth-caller-only-public-visibility";
      ignored_flag "--require-extends-implements-ancestors";
      ignored_flag "--strict-value-equality";
      ignored_flag "--enable-sealed-subclasses";
      ignored_flag "--enable-sound-dynamic-type";
      ignored_flag "--pessimise-builtins";
      ignored_arg "--explicit-consistent-constructors";
      ignored_arg "--require-types-class-consts";
      ignored_flag "--skip-tast-checks";
      ignored_flag "--expression-tree-virtualize-functions";
    ]
    set_file
    usage;
  let mode =
    match !mode with
    | None -> usage_and_exit ()
    | Some mode -> mode
  in
  let file =
    match !file with
    | None -> usage_and_exit ()
    | Some file -> file
  in
  let () =
    if
      !skip_if_errors
      && not
         @@ String.is_substring
              ~substring:"No errors"
              (RealDisk.cat (file ^ ".exp"))
    then begin
      print_endline "Skipping because input file has errors";
      exit 0
    end
  in
  let file = Path.make file in
  let auto_namespace_map = !auto_namespace_map in
  let enable_xhp_class_modifier = !enable_xhp_class_modifier in
  let disable_xhp_element_mangling = !disable_xhp_element_mangling in
  let interpret_soft_types_as_like_types =
    !interpret_soft_types_as_like_types
  in
  let everything_sdt = !everything_sdt in
  let popt =
    popt
      ~auto_namespace_map
      ~enable_xhp_class_modifier
      ~disable_xhp_element_mangling
      ~interpret_soft_types_as_like_types
      ~everything_sdt
  in
  let tco_experimental_features =
    TypecheckerOptions.experimental_from_flags
      ~disallow_static_memoized:!disallow_static_memoized
  in
  let popt = { popt with GlobalOptions.tco_experimental_features } in
  let ctx = init (Path.dirname file) popt in
  let file = Relative_path.(create Root (Path.to_string file)) in
  let files = Multifile.file_to_file_list file in
  let num_files = List.length files in
  let (all_matched, _) =
    List.fold
      files
      ~init:(true, true)
      ~f:(fun (matched, is_first) (filename, contents) ->
        (* All output is printed to stderr because that's the output
           channel Ppxlib_print_diff prints to. *)
        if not is_first then Printf.eprintf "\n%!";
        (* Multifile turns the path into an absolute path instead of a
           relative one. Turn it back into a relative path. *)
        let filename =
          Relative_path.(create Root (Relative_path.to_absolute filename))
        in
        if num_files > 1 then
          Printf.eprintf
            "File %s\n%!"
            (Relative_path.storage_to_string filename);
        let matched =
          Provider_utils.respect_but_quarantine_unsaved_changes
            ~ctx
            ~f:(fun () ->
              match mode with
              | DirectDeclParse -> parse_and_print_decls ctx filename contents
              | VerifyOcamlrepMarshal -> compare_marshal ctx filename contents)
          && matched
        in
        (matched, false))
  in
  if all_matched then
    Printf.eprintf "\nThey matched!\n%!"
  else
    exit 1
