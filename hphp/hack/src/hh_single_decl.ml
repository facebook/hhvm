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
    ~enable_class_pointer_hint =
  ParserOptions.
    {
      default with
      auto_namespace_map;
      disable_xhp_element_mangling;
      keep_user_attributes;
      enable_xhp_class_modifier;
      interpret_soft_types_as_like_types;
      everything_sdt;
      enable_class_pointer_hint;
    }

let init root tcopt ~rust_provider_backend : Provider_context.t =
  Relative_path.(set_path_prefix Root root);
  Relative_path.(set_path_prefix Tmp (Path.make "/tmp"));
  Relative_path.(set_path_prefix Hhi (Path.make "/tmp/non_existent"));

  let sharedmem_config =
    if rust_provider_backend then
      SharedMem.
        {
          default_config with
          shm_use_sharded_hashtbl = true;
          shm_cache_size =
            max SharedMem.default_config.shm_cache_size (2 * 1024 * 1024 * 1024);
        }
    else
      SharedMem.default_config
  in
  let (_handle : SharedMem.handle) =
    SharedMem.init ~num_workers:0 sharedmem_config
  in
  let popt = tcopt.GlobalOptions.po in
  if rust_provider_backend then
    let backend =
      Hh_server_provider_backend.make
        (DeclFoldOptions.from_global_options tcopt)
        (DeclParserOptions.from_parser_options popt)
    in
    Provider_backend.set_rust_backend backend
  else
    Provider_backend.set_shared_memory_backend ();
  let ctx =
    Provider_context.empty_for_tool
      ~popt
      ~tcopt
      ~backend:(Provider_backend.get ())
      ~deps_mode:(Typing_deps_mode.InMemoryMode None)
  in

  (* Push local stacks here so we don't include shared memory in our timing. *)
  File_provider.local_changes_push_sharedmem_stack ();
  Decl_provider.local_changes_push_sharedmem_stack ();

  ctx

let direct_decl_parse ~by_ref ctx fn text =
  let popt = Provider_context.get_popt ctx in
  let opts = DeclParserOptions.from_parser_options popt in
  let parsed_file =
    if by_ref then
      parse_decls_obr opts fn text
    else
      parse_decls opts fn text
  in
  parsed_file.pf_decls

let parse_and_print_decls ~by_ref ctx fn text =
  let (ctx, _entry) =
    Provider_context.(
      add_or_overwrite_entry_contents ~ctx ~path:fn ~contents:text)
  in
  let decls = direct_decl_parse ~by_ref ctx fn text in
  let decls_str = show_decls (List.rev decls) ^ "\n" in
  Printf.eprintf "%s%!" decls_str;
  (* This mode doesn't compare anything. Return false so that we don't print "They matched!". *)
  let matched = false in
  matched

let compare_oxidized ctx fn text =
  let (ctx, _entry) =
    Provider_context.(
      add_or_overwrite_entry_contents ~ctx ~path:fn ~contents:text)
  in
  let oxidized_decls = direct_decl_parse ~by_ref:false ctx fn text in
  let oxidized_by_ref_decls = direct_decl_parse ~by_ref:true ctx fn text in
  let oxidized = show_decls (List.rev oxidized_decls) in
  let oxidized_by_ref = show_decls (List.rev oxidized_by_ref_decls) in
  let decls_matched = String.equal oxidized oxidized_by_ref in
  let () =
    if not decls_matched then begin
      Printf.printf
        "oxidized direct decl parser does not match oxidized_by_ref DDP:\n%!";
      Tempfile.with_real_tempdir @@ fun dir ->
      let temp_dir = Path.to_string dir in
      let oxidized_name = "oxidized" in
      let oxidized_by_ref_name = "oxidized_by_ref" in
      let oxidized_file =
        Stdlib.Filename.temp_file ~temp_dir oxidized_name ".txt"
      in
      let oxidized_by_ref_file =
        Stdlib.Filename.temp_file ~temp_dir oxidized_by_ref_name ".txt"
      in
      Disk.write_file ~file:oxidized_file ~contents:(oxidized ^ "\n");
      Disk.write_file
        ~file:oxidized_by_ref_file
        ~contents:(oxidized_by_ref ^ "\n");
      Ppxlib_print_diff.print
        ~diff_command:
          (Printf.sprintf
             "diff -U9999 --label '%s' --label '%s'"
             oxidized_name
             oxidized_by_ref_name)
        ~file1:oxidized_file
        ~file2:oxidized_by_ref_file
        ()
    end
  in
  decls_matched

let compare_marshal ctx fn text =
  let (ctx, _entry) =
    Provider_context.(
      add_or_overwrite_entry_contents ~ctx ~path:fn ~contents:text)
  in
  let decls = direct_decl_parse ~by_ref:true ctx fn text in
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

let show_name_results
    ~ctx
    ~ctx_with_entry
    (pos, name)
    ~name_type
    ~f_name_exists
    ~f_name_pos
    ~f_name_canon
    ~f_decl_exists =
  let show_pos (pos : FileInfo.pos) : string =
    match pos with
    | FileInfo.Full pos ->
      Printf.sprintf
        "(FileInfo.Full: %s)"
        (Pos.to_relative_string pos |> Pos.string)
    | FileInfo.File (name_type, fn) ->
      Printf.sprintf
        "(FileInfo.File: %s %s)"
        (FileInfo.show_name_type name_type)
        (Relative_path.show fn)
  in
  let name_type_lower = FileInfo.show_name_type name_type |> String.lowercase in
  let show_winner (winner : Decl_provider.winner) : string =
    match winner with
    | Decl_provider.Winner -> "winner"
    | Decl_provider.Loser_to pos ->
      Printf.sprintf "Loser_to %s" (Pos.to_relative_string pos |> Pos.string)
    | Decl_provider.Not_found -> "not_found"
  in
  let print_item fmt arg ctx_arg =
    let (value, value_with_entry) = (ctx_arg ctx, ctx_arg ctx_with_entry) in
    Printf.eprintf fmt arg value;
    if not (String.equal value value_with_entry) then begin
      Printf.eprintf "  [hh_server / typing_check_service]\n%!";
      Printf.eprintf fmt arg value_with_entry;
      Printf.eprintf "  [Provider_context.entry]"
    end;
    Printf.eprintf "\n%!"
  in
  Printf.eprintf
    "name=%s, name_type=%s, pos=%s\n%!"
    name
    (FileInfo.show_name_type name_type)
    (Pos.to_relative_string pos |> Pos.string);
  print_item
    "   Naming_provider.%s_exists(name): %s"
    name_type_lower
    (fun ctx -> f_name_exists ctx name |> string_of_bool);
  print_item
    "   Naming_provider.get_%s_pos(name): %s"
    name_type_lower
    (fun ctx ->
      f_name_pos ctx name |> Option.value_map ~default:"[none]" ~f:show_pos);
  print_item
    "   Naming_provider.get_%s_canon_name(name): %s"
    name_type_lower
    (fun ctx -> f_name_canon ctx name |> Option.value ~default:"[none]");
  print_item
    "   Decl_provider.get_%s(name) |> Option.is_some: %s"
    name_type_lower
    (fun ctx -> f_decl_exists ctx name |> Option.is_some |> string_of_bool);
  print_item
    "   Decl_provider.get_pos_from_decl_of_winner%s(name_type,name): %s"
    ""
    (fun ctx ->
      Decl_provider.get_pos_from_decl_of_winner_FOR_TESTS_ONLY
        ctx
        name_type
        name
      |> Option.value_map ~default:"[none]" ~f:(fun pos ->
             Pos.to_relative_string pos |> Pos.string));
  print_item
    "   Decl_provider.is_this_def_the_winner%s(name_type,name,pos): %s"
    ""
    (fun ctx ->
      Decl_provider.is_this_def_the_winner ctx name_type (pos, name)
      |> show_winner);
  Printf.eprintf "\n%!";
  ()

(** Constructs a list of [name_type * id] pairs for each top-level declaration
we find in the AST. *)
let ast_to_toplevels (ast : Nast.program) :
    (FileInfo.name_type * Ast_defs.id) list =
  List.filter_map ast ~f:(fun def ->
      match def with
      | Aast.Fun { Aast_defs.fd_name; _ } -> Some (FileInfo.Fun, fd_name)
      | Aast.Constant { Aast_defs.cst_name; _ } ->
        Some (FileInfo.Const, cst_name)
      | Aast.Typedef { Aast_defs.t_name; _ } -> Some (FileInfo.Typedef, t_name)
      | Aast.Class { Aast_defs.c_name; _ } -> Some (FileInfo.Class, c_name)
      | Aast.Module { Aast_defs.md_name; _ } -> Some (FileInfo.Module, md_name)
      | Aast.(
          ( Stmt _ | SetModule _ | Namespace _ | NamespaceUse _
          | SetNamespaceEnv _ | FileAttributes _ )) ->
        None)

(** Constructs a list of [name_type * id] pairs for each top-level declaration
we find from the direct-decl-parser. *)
let decls_to_toplevels (decls : Direct_decl_parser.parsed_file_with_hashes) :
    (FileInfo.name_type * Ast_defs.id) list =
  List.map decls.pfh_decls ~f:(fun (name, decl, _hash, _sort_text) ->
      let (name_type, pos) =
        match decl with
        | Shallow_decl_defs.Class { Shallow_decl_defs.sc_name = (pos, _id); _ }
          ->
          (FileInfo.Class, pos)
        | Shallow_decl_defs.Fun { Typing_defs.fe_pos; _ } ->
          (FileInfo.Fun, fe_pos)
        | Shallow_decl_defs.Typedef { Typing_defs.td_pos; _ } ->
          (FileInfo.Typedef, td_pos)
        | Shallow_decl_defs.Const { Typing_defs.cd_pos; _ } ->
          (FileInfo.Const, cd_pos)
        | Shallow_decl_defs.Module { Typing_defs.mdt_pos; _ } ->
          (FileInfo.Module, mdt_pos)
      in
      let pos = Pos_or_decl.unsafe_to_raw_pos pos in
      (name_type, (pos, name)))

let compare_toplevels
    (a_name_type, (a_pos, a_name)) (b_name_type, (b_pos, b_name)) =
  let c = String.compare (String.lowercase a_name) (String.lowercase b_name) in
  if c <> 0 then
    c
  else
    let c = String.compare a_name b_name in
    if c <> 0 then
      c
    else
      let c = FileInfo.compare_name_type a_name_type b_name_type in
      if c <> 0 then
        c
      else
        Pos.compare a_pos b_pos

(** This does "naming_global" on all the files, i.e. parses them,
figures out winners, updates the reverse naming table. Then for
every toplevel symbol name it encountered, it prints out results from
a few Naming_provider and Decl_provider APIs on that symbol name.

This function does NOT use Provider_context "entries". To explain:
you can set up a Provider_context.t with zero or more entries.
Each one provides file content, caches for AST and TAST, and it
acts as an "override" reverse-naming-table for all names
defined in those entries. If a name isn't found in any entry,
then we fall back to the global reverse naming table
which was set up by Naming_global.ml. Entries are not used
in hh_server's Typing_check_service, and they're not used in
this function either.

[decl_make_env] causes us to call [Decl.make_env]. Its job is to populate
the decl-heap with the content of a file. It's called in some codepaths
e.g. hh_single_type_check and when we make edits to a file. It's a terrible
function which doesn't respect winners/losers. We call it here to make our
behavior similar to that of hh_single_type_check. *)
let name_and_then_print_name_results ctx files ~decl_make_env =
  let popt = Provider_context.get_popt ctx in
  let names =
    List.map files ~f:(fun (fn, contents) ->
        File_provider.provide_file_for_tests fn contents;
        let (_parse_errors, parsed_file) =
          Errors.do_with_context fn (fun () ->
              Full_fidelity_ast.defensive_program popt fn contents)
        in
        let ast =
          let { Parser_return.ast; _ } = parsed_file in
          if popt.ParserOptions.deregister_php_stdlib then
            Nast.deregister_ignored_attributes ast
          else
            ast
        in
        Ast_provider.provide_ast_hint fn ast Ast_provider.Full;
        let decls =
          Direct_decl_utils.direct_decl_parse ctx fn |> Option.value_exn
        in
        let fi = Direct_decl_utils.decls_to_fileinfo fn decls in
        let _conflict_filenames =
          Naming_global.ndecl_file_and_get_conflict_files ctx fn fi.FileInfo.ids
        in
        if decl_make_env then Decl.make_env ~sh:SharedMem.Uses ctx fn;
        (* Here we assemble top-level definitions as discovered by
           AST, and also as discovered by direct-decl-parser. We include
           them both so as to exercise any differences between the two! *)
        ast_to_toplevels ast @ decls_to_toplevels decls)
    |> List.concat
    |> Stdlib.List.sort_uniq compare_toplevels
  in
  List.iter names ~f:(fun (name_type, id) ->
      let path = Pos.filename (fst id) in
      let contents =
        List.Assoc.find_exn files path ~equal:Relative_path.equal
      in
      let (ctx_with_entry, _entry) =
        Provider_context.add_or_overwrite_entry_contents ~ctx ~path ~contents
      in
      match name_type with
      | FileInfo.Fun ->
        show_name_results
          ~ctx
          ~ctx_with_entry
          id
          ~name_type
          ~f_name_exists:Naming_provider.fun_exists
          ~f_name_pos:Naming_provider.get_fun_pos
          ~f_name_canon:Naming_provider.get_fun_canon_name
          ~f_decl_exists:(fun ctx x ->
            Decl_provider.get_fun ctx x |> Decl_entry.to_option)
      | FileInfo.Class ->
        show_name_results
          ~ctx
          ~ctx_with_entry
          id
          ~name_type
          ~f_name_exists:(fun ctx name ->
            Naming_provider.get_class_path ctx name |> Option.is_some)
          ~f_name_pos:Naming_provider.get_type_pos
          ~f_name_canon:Naming_provider.get_type_canon_name
          ~f_decl_exists:(fun ctx x ->
            Decl_provider.get_class ctx x |> Decl_entry.to_option)
      | FileInfo.Typedef ->
        show_name_results
          ~ctx
          ~ctx_with_entry
          id
          ~name_type
          ~f_name_exists:(fun ctx name ->
            Naming_provider.get_typedef_path ctx name |> Option.is_some)
          ~f_name_pos:Naming_provider.get_type_pos
          ~f_name_canon:Naming_provider.get_type_canon_name
          ~f_decl_exists:(fun ctx x ->
            Decl_provider.get_typedef ctx x |> Decl_entry.to_option)
      | FileInfo.Const ->
        show_name_results
          ~ctx
          ~ctx_with_entry
          id
          ~name_type
          ~f_name_exists:Naming_provider.const_exists
          ~f_name_pos:Naming_provider.get_const_pos
          ~f_name_canon:(fun _ _ -> Some "[undefined]")
          ~f_decl_exists:Decl_provider.get_gconst
      | FileInfo.Module ->
        show_name_results
          ~ctx
          ~ctx_with_entry
          id
          ~name_type
          ~f_name_exists:Naming_provider.module_exists
          ~f_name_pos:Naming_provider.get_module_pos
          ~f_name_canon:(fun _ _ -> Some "[undefined]")
          ~f_decl_exists:Decl_provider.get_module);
  ()

type modes =
  | DirectDeclParse of bool
      (** Runs the direct decl parser on the given file. If true, run the arena-based DDP. *)
  | VerifyOxidized
      (** Runs the oxidized and oxidized-by-ref DDPs on a file and compares their output *)
  | VerifyOcamlrepMarshal
      (** Marshals the output of the direct decl parser using Marshal and ocamlrep_marshal and compares their output *)
  | Winners
      (** reports what Naming_provider and Decl_provider return for each name *)

let iterate_files files ~f : bool option =
  let num_files = List.length files in
  let (all_matched, _) =
    List.fold
      files
      ~init:(true, true)
      ~f:(fun (matched, is_first) (filename, contents) ->
        (* All output is printed to stderr because that's the output
           channel Ppxlib_print_diff prints to. *)
        if not is_first then Printf.eprintf "\n%!";
        if num_files > 1 then
          Printf.eprintf
            "File %s\n%!"
            (Relative_path.storage_to_string filename);
        let matched = f filename contents && matched in
        (matched, false))
  in
  Some all_matched

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
  let keep_user_attributes = ref false in
  let disallow_static_memoized = ref false in
  let interpret_soft_types_as_like_types = ref false in
  let everything_sdt = ref false in
  let rust_provider_backend = ref false in
  let enable_class_pointer_hint = ref true in
  let ignored_flag flag = (flag, Arg.Unit (fun _ -> ()), "(ignored)") in
  let ignored_arg flag = (flag, Arg.String (fun _ -> ()), "(ignored)") in
  Arg.parse
    [
      ( "--decl-parse",
        Arg.Unit (set_mode (DirectDeclParse false)),
        "(mode) Runs the direct decl parser on the given file" );
      ( "--decl-parse-obr",
        Arg.Unit (set_mode (DirectDeclParse true)),
        "(mode) Runs the arena-based direct decl parser on the given file" );
      ( "--verify-oxidized",
        Arg.Unit (set_mode VerifyOxidized),
        "(mode) Runs the oxidized and oxidized-by-ref DDPs on a file and compares their output"
      );
      ( "--verify-ocamlrep-marshal",
        Arg.Unit (set_mode VerifyOcamlrepMarshal),
        "(mode) Marshals the output of the direct decl parser using Marshal and ocamlrep_marshal and compares their output"
      );
      ( "--winners",
        Arg.Unit (set_mode Winners),
        "(mode) reports what Decl_provider returns for each name" );
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
      ("--keep-user-attributes", Arg.Set keep_user_attributes, ".");
      ( "--disallow-static-memoized",
        Arg.Set disallow_static_memoized,
        " Disallow static memoized methods on non-final methods" );
      ( "--interpret-soft-types-as-like-types",
        Arg.Set interpret_soft_types_as_like_types,
        "Interpret <<__Soft>> type hints as like types" );
      ( "--rust-provider-backend",
        Arg.Set rust_provider_backend,
        " Use the Rust implementation of Provider_backend (including decl-folding)"
      );
      ( "--enable-class-pointer-hint",
        Arg.Bool (fun x -> enable_class_pointer_hint := x),
        " Killswitch to interpret class<T> hint as class<T> type when true, classname<T> when false"
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
      ignored_flag "--forbid_nullable_cast";
      ( "--hh-log-level",
        Arg.Tuple [Arg.String (fun _ -> ()); Arg.String (fun _ -> ())],
        "(ignored)" );
      ignored_flag "--is-systemlib";
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
      ignored_arg "--meth-caller-only-public-visibility";
      ignored_flag "--require-extends-implements-ancestors";
      ignored_flag "--strict-value-equality";
      ignored_flag "--enable-sealed-subclasses";
      ignored_arg "--explicit-consistent-constructors";
      ignored_arg "--require-types-class-consts";
      ignored_flag "--skip-tast-checks";
      ignored_flag "--expression-tree-virtualize-functions";
      ignored_arg "--config";
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
  EventLogger.init_fake ();
  let file = Path.make file in
  let auto_namespace_map = !auto_namespace_map in
  let enable_xhp_class_modifier = !enable_xhp_class_modifier in
  let disable_xhp_element_mangling = !disable_xhp_element_mangling in
  let keep_user_attributes = !keep_user_attributes in
  let interpret_soft_types_as_like_types =
    !interpret_soft_types_as_like_types
  in
  let everything_sdt = !everything_sdt in
  let enable_class_pointer_hint = !enable_class_pointer_hint in
  let popt =
    popt
      ~auto_namespace_map
      ~enable_xhp_class_modifier
      ~disable_xhp_element_mangling
      ~keep_user_attributes
      ~interpret_soft_types_as_like_types
      ~everything_sdt
      ~enable_class_pointer_hint
  in
  let tco_experimental_features =
    TypecheckerOptions.experimental_from_flags
      ~disallow_static_memoized:!disallow_static_memoized
  in
  let tcopt =
    GlobalOptions.{ default with po = popt; tco_experimental_features }
  in
  let ctx =
    init (Path.dirname file) tcopt ~rust_provider_backend:!rust_provider_backend
  in
  let file = Relative_path.(create Root (Path.to_string file)) in
  let files = Multifile.file_to_file_list file in
  (* Multifile produces Dummy paths, but we want root-relative paths *)
  let files =
    List.map files ~f:(fun (fn, content) ->
        (Relative_path.(create Root (Relative_path.to_absolute fn)), content))
  in
  let all_matched =
    match mode with
    | DirectDeclParse by_ref ->
      iterate_files files ~f:(fun filename contents ->
          Provider_utils.respect_but_quarantine_unsaved_changes
            ~ctx
            ~f:(fun () -> parse_and_print_decls ~by_ref ctx filename contents))
    | VerifyOxidized ->
      iterate_files files ~f:(fun filename contents ->
          Provider_utils.respect_but_quarantine_unsaved_changes
            ~ctx
            ~f:(fun () -> compare_oxidized ctx filename contents))
    | VerifyOcamlrepMarshal ->
      iterate_files files ~f:(fun filename contents ->
          Provider_utils.respect_but_quarantine_unsaved_changes
            ~ctx
            ~f:(fun () -> compare_marshal ctx filename contents))
    | Winners ->
      name_and_then_print_name_results ctx files ~decl_make_env:true;
      None
  in
  match all_matched with
  | Some true -> Printf.eprintf "\nThey matched!\n%!"
  | Some false -> exit 1
  | None -> ()
