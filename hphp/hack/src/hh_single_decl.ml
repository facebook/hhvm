(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Direct_decl_parser

type verbosity =
  | Standard
  | Verbose
  | Silent

let init root : Provider_context.t =
  Relative_path.(set_path_prefix Root root);
  let (_handle : SharedMem.handle) =
    SharedMem.init ~num_workers:0 GlobalConfig.default_sharedmem_config
  in
  let tcopt =
    {
      TypecheckerOptions.default with
      GlobalOptions.tco_shallow_class_decl = true;
    }
  in
  let ctx =
    Provider_context.empty_for_tool
      ~popt:ParserOptions.default
      ~tcopt
      ~backend:Provider_backend.Shared_memory
  in

  (* Push local stacks here so we don't include shared memory in our timing. *)
  File_provider.local_changes_push_sharedmem_stack ();
  Decl_provider.local_changes_push_sharedmem_stack ();
  Shallow_classes_provider.local_changes_push_sharedmem_stack ();
  Linearization_provider.local_changes_push_sharedmem_stack ();

  ctx

let time verbosity msg f =
  let before = Unix.gettimeofday () in
  let ret = f () in
  let after = Unix.gettimeofday () in
  if verbosity = Verbose then
    Printf.printf "%s: %f ms\n" msg ((after -. before) *. 1000.);
  ret

let compare_decl ctx verbosity fn =
  let fn = Path.to_string fn in
  let text = RealDisk.cat fn in
  let fn = Relative_path.(create Root fn) in
  let decls =
    time verbosity "Parsed decls" (fun () ->
        Result.ok_or_failwith (parse_decls ~contents:text fn))
  in
  let facts =
    Option.value_exn
      ~message:"Could not parse facts from file"
      (Facts_parser.from_text
         ~php5_compat_mode:false
         ~hhvm_compat_mode:false
         ~disable_nontoplevel_declarations:false
         ~disable_legacy_soft_typehints:false
         ~allow_new_attribute_syntax:true
         ~disable_legacy_attribute_syntax:false
         ~enable_xhp_class_modifier:false
         ~disable_xhp_element_mangling:false
         ~filename:fn
         ~text)
  in
  let passes_symbol_check () =
    let compare name facts_symbols decl_symbols =
      let facts_symbols = SSet.of_list facts_symbols in
      let decl_symbols = SSet.of_list decl_symbols in
      let facts_only = SSet.diff facts_symbols decl_symbols in
      let decl_only = SSet.diff decl_symbols facts_symbols in
      if (not @@ SSet.is_empty facts_only) || (not @@ SSet.is_empty decl_only)
      then (
        if not @@ SSet.is_empty facts_only then
          Printf.eprintf
            "The following %s were found in the facts parse but not the decl parse: %s\n"
            name
            (SSet.show facts_only);
        if not @@ SSet.is_empty decl_only then
          Printf.eprintf
            "The following %s were found in the decl parse but not the facts parse: %s\n"
            name
            (SSet.show decl_only);
        prerr_endline "";
        false
      ) else
        true
    in
    [
      compare "typedef(s)" facts.Facts.type_aliases (SMap.keys decls.typedefs);
      compare "constant(s)" facts.Facts.constants (SMap.keys decls.consts);
      compare "function(s)" facts.Facts.functions (SMap.keys decls.funs);
      compare
        "class(es)"
        (Facts.InvSMap.keys facts.Facts.types)
        (SMap.keys decls.classes @ SMap.keys decls.typedefs);
    ]
    |> List.reduce_exn ~f:( && )
  in
  let passes_decl_check () =
    let () =
      time verbosity "Calculated legacy decls" (fun () ->
          (* Put the file contents in the disk heap so both the decl parsing and
           * legacy decl branches can avoid having to wait for file I/O. *)
          File_provider.provide_file fn (File_provider.Disk text);
          Decl.make_env ~sh:SharedMem.Uses ctx fn)
    in
    let compare name get_decl eq_decl show_decl parsed_decls =
      let different_decls =
        SMap.fold
          (fun key parsed_decl acc ->
            let legacy_decl = get_decl ctx fn ("\\" ^ key) in
            let legacy_decl_str = show_decl legacy_decl in
            let parsed_decl_str = show_decl parsed_decl in
            if not @@ eq_decl legacy_decl parsed_decl then
              (key, legacy_decl_str, parsed_decl_str) :: acc
            else
              acc)
          parsed_decls
          []
      in
      match different_decls with
      | [] -> true
      | different_decls ->
        Printf.eprintf
          "The following %s differed between the legacy and parsed versions:\n"
          name;
        List.iter different_decls ~f:(fun (key, legacy_decl, parsed_decl) ->
            Tempfile.with_real_tempdir (fun dir ->
                let temp_dir = Path.to_string dir in
                let temp_file () =
                  Filename.temp_file
                    ~temp_dir
                    (Printf.sprintf "%s_%s" name key)
                    ".txt"
                in
                let expected = temp_file () in
                let actual = temp_file () in
                Disk.write_file ~file:expected ~contents:legacy_decl;
                Disk.write_file ~file:actual ~contents:parsed_decl;
                Printf.eprintf "\n\n[%s]\n" key;
                Out_channel.flush stderr;
                Ppxlib_print_diff.print
                  ~diff_command:"diff -U9999 --label legacy --label parsed"
                  ~file1:expected
                  ~file2:actual
                  ()));
        false
    in
    [
      compare
        "typedef(s)"
        (Decl.declare_typedef_in_file ~write_shmem:true)
        Typing_defs.equal_typedef_type
        Pp_type.show_typedef_type
        decls.typedefs;
      compare
        "constant(s)"
        (fun ctx a b ->
          Decl.declare_const_in_file ~write_shmem:true ctx a b |> fst)
        Typing_defs.equal_decl_ty
        Pp_type.show_decl_ty
        decls.consts;
      compare
        "function(s)"
        (Decl.declare_fun_in_file ~write_shmem:true)
        Typing_defs.equal_fun_elt
        Pp_type.show_fun_elt
        decls.funs;
      compare
        "class(es)"
        (fun ctx fn name ->
          let class_ = Ast_provider.find_class_in_file ctx fn name in
          let class_ = Option.value_exn class_ in
          let class_ =
            Shallow_classes_provider.decl ctx ~use_cache:true class_
          in
          { class_ with Shallow_decl_defs.sc_decl_errors = Errors.empty })
        Shallow_decl_defs.equal_shallow_class
        Shallow_decl_defs.show_shallow_class
        decls.classes;
    ]
    |> List.reduce_exn ~f:( && )
  in
  let matched = passes_symbol_check () && passes_decl_check () in
  if matched then (
    print_endline "Parsed decls:\n";
    print_endline (show_decls decls);
    print_endline "\nThey matched!"
  );
  matched

type modes = CompareDirectDeclParser

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
  let verbosity = ref Standard in
  Arg.parse
    [
      ( "--compare-direct-decl-parser",
        Arg.Unit (set_mode CompareDirectDeclParser),
        "(mode) Runs the direct decl parser against the FFP -> naming -> decl pipeline and compares their output"
      );
      ( "--verbosity",
        Arg.Symbol
          ( ["silent"; "standard"; "verbose"],
            fun v ->
              verbosity :=
                match v with
                | "silent" -> Silent
                | "standard" -> Standard
                | "verbose" -> Verbose
                | _ ->
                  failwith
                  @@ Printf.sprintf "Did not understand verbosity level %s" v ),
        " Set the verbosity level. Silent will hide the \"no differences\" message on a successful "
        ^ "run, and verbose will print debugging information to the console" );
    ]
    set_file
    usage;
  match !mode with
  | None -> usage_and_exit ()
  | Some CompareDirectDeclParser ->
    begin
      match !file with
      | None -> usage_and_exit ()
      | Some file ->
        let file = Path.make file in
        let ctx = init (Path.dirname file) in
        Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
            if not @@ compare_decl ctx !verbosity file then exit 1)
    end
