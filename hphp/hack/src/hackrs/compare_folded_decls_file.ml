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
    ~keep_user_attributes
    ~interpret_soft_types_as_like_types
    ~everything_sdt
    ~enable_strict_const_semantics =
  let po = ParserOptions.default in
  let po =
    ParserOptions.with_disable_xhp_element_mangling
      po
      disable_xhp_element_mangling
  in
  let po = ParserOptions.with_keep_user_attributes po keep_user_attributes in
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
  let po =
    {
      po with
      GlobalOptions.tco_enable_strict_const_semantics =
        enable_strict_const_semantics;
    }
  in
  po

let init ~enable_strict_const_semantics popt : Provider_context.t =
  let (_handle : SharedMem.handle) =
    SharedMem.init ~num_workers:0 SharedMem.default_config
  in
  let tcopt =
    {
      popt with
      GlobalOptions.tco_higher_kinded_types = true;
      GlobalOptions.tco_enable_strict_const_semantics =
        enable_strict_const_semantics;
    }
  in
  let ctx =
    Provider_context.empty_for_tool
      ~popt
      ~tcopt
      ~backend:Provider_backend.Shared_memory
      ~deps_mode:(Typing_deps_mode.InMemoryMode None)
  in

  (* Push local stacks here so we don't include shared memory in our timing. *)
  File_provider.local_changes_push_sharedmem_stack ();
  Decl_provider.local_changes_push_sharedmem_stack ();

  ctx

let direct_decl_parse ctx fn text =
  let popt = Provider_context.get_popt ctx in
  let opts = DeclParserOptions.from_parser_options popt in
  let parsed_file = Direct_decl_parser.parse_decls opts fn text in
  parsed_file.pf_decls

let print_diff ~expected_name ~actual_name ~expected_contents ~actual_contents =
  Tempfile.with_real_tempdir @@ fun dir ->
  let temp_dir = Path.to_string dir in
  let expected = Stdlib.Filename.temp_file ~temp_dir "expected" ".txt" in
  let actual = Stdlib.Filename.temp_file ~temp_dir "actual" ".txt" in
  Disk.write_file ~file:expected ~contents:(expected_contents ^ "\n");
  Disk.write_file ~file:actual ~contents:(actual_contents ^ "\n");
  Ppxlib_print_diff.print
    ~diff_command:
      (Printf.sprintf
         "diff -U9999 --label '%s' --label '%s'"
         expected_name
         actual_name)
    ~file1:expected
    ~file2:actual
    ()

let compare_folded
    ctx
    rupro_decls
    ~print_ocaml
    ~print_rupro
    ~test_ocamlrep_marshal
    multifile
    filename
    text =
  let class_names =
    direct_decl_parse ctx filename text
    |> List.rev_filter_map ~f:(function
           | (name, Shallow_decl_defs.Class _) -> Some name
           | _ -> None)
  in
  let ocaml_folded_classes =
    List.map class_names ~f:(fun cid ->
        Decl_provider.declare_folded_class_in_file_FOR_TESTS_ONLY ctx cid)
  in
  let rupro_folded_classes = Relative_path.Map.find rupro_decls filename in

  (if test_ocamlrep_marshal then
    let folded_class_decls = List.zip_exn class_names ocaml_folded_classes in
    let test_marshaling (c, o) =
      let ocaml_marshaled = Marshal.to_string o [] in
      let rust_marshaled = Ocamlrep_marshal_ffi.to_string o [] in
      let _ =
        if not (String.equal rust_marshaled ocaml_marshaled) then
          failwith
            (Printf.sprintf
               "Marshaling of '%s' differs between Rust and OCaml. This indicates 'ocamlrep_marshal_output_value_to_string' is broken.\nocaml:\n%S\nrust:\n%S\n"
               c
               ocaml_marshaled
               rust_marshaled)
        else
          ()
      in
      let rust_read_back = Ocamlrep_marshal_ffi.from_string rust_marshaled 0 in
      let _ =
        if
          not
            (String.equal
               (Decl_defs.show_decl_class_type o)
               (Decl_defs.show_decl_class_type rust_read_back))
        then
          failwith
            (Printf.sprintf
               "Rust unmarshaling of '%s' is wrong. This indicates 'ocamlrep_marshal_input_value' is broken."
               c)
      in
      ()
    in
    List.iter folded_class_decls ~f:test_marshaling);

  let show_folded_decls decls =
    decls
    |> List.map ~f:Decl_folded_class_rupro.show_decl_class_type
    |> String.concat ~sep:"\n"
  in
  let folded = show_folded_decls ocaml_folded_classes in
  if print_ocaml then Format.eprintf "OCaml folded decls:\n%s\n\n" folded;
  let rupro_folded = show_folded_decls rupro_folded_classes in
  if print_rupro then Format.eprintf "rupro folded decls:\n%s\n\n" rupro_folded;
  let matched = String.equal folded rupro_folded in
  if not matched then (
    (* All output is printed to stderr because that's the output
       channel Ppxlib_print_diff prints to. *)
    if multifile then
      Printf.eprintf "File %s\n%!" (Relative_path.storage_to_string filename);
    print_diff
      ~expected_name:"ocaml"
      ~actual_name:"rupro"
      ~expected_contents:folded
      ~actual_contents:rupro_folded
  );
  matched

let () =
  let usage = Printf.sprintf "Usage: %s [OPTIONS] filename\n" Sys.argv.(0) in
  let usage_and_exit () =
    prerr_endline usage;
    exit 1
  in
  let file = ref None in
  let set_file f =
    match !file with
    | None -> file := Some f
    | Some _ -> usage_and_exit ()
  in
  let auto_namespace_map = ref [] in
  let enable_xhp_class_modifier = ref false in
  let disable_xhp_element_mangling = ref false in
  let keep_user_attributes = ref false in
  let disallow_static_memoized = ref false in
  let interpret_soft_types_as_like_types = ref false in
  let everything_sdt = ref false in
  let enable_strict_const_semantics = ref 0 in
  let print_ocaml = ref false in
  let print_rupro = ref false in
  let test_ocamlrep_marshal = ref false in
  let ignored_flag flag = (flag, Arg.Unit (fun _ -> ()), "(ignored)") in
  let ignored_arg flag = (flag, Arg.String (fun _ -> ()), "(ignored)") in
  Arg.parse
    [
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
      ("--keep-user-attributes", Arg.Set keep_user_attributes, ".");
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
      ( "--enable-strict-const-semantics",
        Arg.Int (fun x -> enable_strict_const_semantics := x),
        " Raise an error when a concrete constants is overridden or multiply defined"
      );
      ( "--print-ocaml",
        Arg.Set print_ocaml,
        " Print OCaml folded decls to stdout" );
      ( "--print-rupro",
        Arg.Set print_rupro,
        " Print rupro folded decls to stdout" );
      ( "--test-marshaling",
        Arg.Set test_ocamlrep_marshal,
        " Test ocamlrep_marshal (rust) marshaling/unmarshaling" );
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
      ignored_flag "--disallow-trait-reuse";
      ignored_flag "--enable-class-level-where-clauses";
      ignored_flag "--enable-higher-kinded-types";
      ignored_flag "--forbid_nullable_cast";
      ( "--hh-log-level",
        Arg.Tuple [Arg.String (fun _ -> ()); Arg.String (fun _ -> ())],
        "(ignored)" );
      ignored_flag "--is-systemlib";
      ignored_flag "--method-call-inference";
      ignored_flag "--no-builtins";
      ignored_flag "--no-strict-contexts";
      ignored_flag "--report-pos-from-reason";
      ignored_arg "--timeout";
      ignored_flag "--union-intersection-type-hints";
      ignored_flag "--enable-strict-string-concat-interp";
      ignored_arg "--extra-builtin";
      ignored_flag "--disallow-inst-meth";
      ignored_flag "--ignore-unsafe-cast";
      ignored_flag "--inc-dec-new-code";
      ignored_flag "--disallow-partially-abstract-typeconst-definitions";
      ignored_flag "--typeconst-concrete-concrete-error";
      ignored_arg "--strict-wellformedness";
      ignored_arg "--meth-caller-only-public-visibility";
      ignored_flag "--require-extends-implements-ancestors";
      ignored_flag "--strict-value-equality";
      ignored_flag "--enable-sealed-subclasses";
      ignored_flag "--enable-sound-dynamic-type";
      ignored_arg "--explicit-consistent-constructors";
      ignored_arg "--require-types-class-consts";
      ignored_flag "--skip-tast-checks";
      ignored_flag "--expression-tree-virtualize-functions";
      ignored_arg "--config";
    ]
    set_file
    usage;
  match !file with
  | None -> usage_and_exit ()
  | Some file ->
    let file = Path.make file in
    let auto_namespace_map = !auto_namespace_map in
    let enable_xhp_class_modifier = !enable_xhp_class_modifier in
    let disable_xhp_element_mangling = !disable_xhp_element_mangling in
    let keep_user_attributes = !keep_user_attributes in
    let interpret_soft_types_as_like_types =
      !interpret_soft_types_as_like_types
    in
    let enable_strict_const_semantics = !enable_strict_const_semantics in
    let everything_sdt = !everything_sdt in
    let print_ocaml = !print_ocaml in
    let print_rupro = !print_rupro in
    let test_ocamlrep_marshal = !test_ocamlrep_marshal in
    let popt =
      popt
        ~auto_namespace_map
        ~enable_xhp_class_modifier
        ~disable_xhp_element_mangling
        ~keep_user_attributes
        ~interpret_soft_types_as_like_types
        ~everything_sdt
        ~enable_strict_const_semantics
    in
    let tco_experimental_features =
      TypecheckerOptions.experimental_from_flags
        ~disallow_static_memoized:!disallow_static_memoized
    in
    let popt = { popt with GlobalOptions.tco_experimental_features } in
    (* Temporarily set the root to the location of the test file so that
       Multifile will strip the dirname prefix. *)
    Relative_path.(set_path_prefix Root (Path.dirname file));
    let file = Relative_path.(create Root (Path.to_string file)) in
    let files =
      Multifile.file_to_file_list file
      |> List.map ~f:(fun (file, contents) ->
             (Relative_path.to_absolute file, contents))
    in
    Tempfile.with_real_tempdir @@ fun tmpdir ->
    (* Set the Relative_path root to the tmpdir. *)
    Relative_path.(set_path_prefix Root tmpdir);
    (* Write the files to a tempdir so that rupro can read them, then map
       them to relative paths. *)
    let files =
      List.map files ~f:(fun (filename, contents) ->
          let tmpdir = Path.to_string tmpdir in
          let basename = Filename.basename filename in
          let filename = Filename.concat tmpdir basename in
          Disk.write_file ~file:filename ~contents;
          (Relative_path.(create Root filename), contents))
    in
    let ctx = init ~enable_strict_const_semantics popt in
    let iter_files f =
      let multifile = List.length files > 1 in
      List.fold files ~init:true ~f:(fun matched (filename, contents) ->
          Provider_utils.respect_but_quarantine_unsaved_changes
            ~ctx
            ~f:(fun () -> f multifile filename contents)
          && matched)
    in
    let all_matched =
      let files = List.map files ~f:fst in
      let () =
        List.iter files ~f:(fun filename ->
            let _ =
              Direct_decl_utils.direct_decl_parse_and_cache ctx filename
            in
            ())
      in
      (* Compute OCaml folded decls *)
      List.iter files ~f:(fun filename ->
          Decl.make_env ~sh:SharedMem.Uses ctx filename);
      (* Compute rupro folded decls *)
      let rupro_decls =
        match
          Decl_folded_class_rupro.fold_classes_in_files
            ~root:(Path.to_string tmpdir)
            popt
            files
        with
        | Ok rupro_decls -> rupro_decls
        | Error e ->
          Printf.eprintf "%s\n%!" e;
          exit 1
      in
      iter_files
        (compare_folded
           ctx
           rupro_decls
           ~print_ocaml
           ~print_rupro
           ~test_ocamlrep_marshal)
    in
    if not all_matched then exit 1
