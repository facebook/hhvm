(*
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Sys_utils

(*****************************************************************************)
(* Types, constants *)
(*****************************************************************************)

type mode =
  | NoMode
  | Autocomplete
  | Autocomplete_manually_invoked

type options = {
  files: string list;
  extra_builtins: string list;
  mode: mode;
  no_builtins: bool;
  tcopt: GlobalOptions.t;
}

(* Canonical builtins from our hhi library *)
let hhi_builtins = Hhi.get_raw_hhi_contents ()

(* All of the stuff that hh_single_type_check relies on is sadly not contained
 * in the hhi library, so we include a very small number of magic builtins *)
let magic_builtins =
  [|
    ( "hh_single_type_check_magic.hhi",
      "<?hh\n"
      ^ "namespace {\n"
      ^ "async function gena<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
): Awaitable<darray<Tk, Tv>>;\n"
      ^ "function hh_show(<<__AcceptDisposable>> $val) {}\n"
      ^ "function hh_expect<T>(<<__AcceptDisposable>> $val) {}\n"
      ^ "function hh_expect_equivalent<T>(<<__AcceptDisposable>> $val) {}\n"
      ^ "function hh_show_env() {}\n"
      ^ "function hh_log_level($key, $level) {}\n"
      ^ "function hh_force_solve () {}"
      ^ "}\n"
      ^ "namespace HH\\Lib\\Tuple{\n"
      ^ "function gen();\n"
      ^ "function from_async();\n"
      ^ "}\n" );
  |]

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let die str =
  let oc = stderr in
  Out_channel.output_string oc str;
  Out_channel.close oc;
  exit 2

let parse_options () =
  let fn_ref = ref [] in
  let extra_builtins = ref [] in
  let usage = Printf.sprintf "Usage: %s filename\n" Sys.argv.(0) in
  let mode = ref NoMode in
  let no_builtins = ref false in
  let set_mode x () =
    match !mode with
    | NoMode -> mode := x
    | _ -> raise (Arg.Bad "only a single mode should be specified")
  in
  let set name reference value =
    match !reference with
    | None -> reference := Some value
    | Some _ -> failwith (Printf.sprintf "Attempted to set %s twice" name)
  in
  let check_xhp_attribute = ref false in
  let disable_xhp_element_mangling = ref false in
  let disable_xhp_children_declarations = ref false in
  let enable_xhp_class_modifier = ref false in
  let saved_state_manifold_api_key = ref None in
  let options =
    [
      ( "--extra-builtin",
        Arg.String (fun f -> extra_builtins := f :: !extra_builtins),
        " HHI file to parse and declare" );
      ( "--no-builtins",
        Arg.Set no_builtins,
        " Don't use builtins (e.g. ConstSet); implied by --root" );
      ( "--check-xhp-attribute",
        Arg.Set check_xhp_attribute,
        " Typechecks xhp required attributes" );
      ( "--disable-xhp-element-mangling",
        Arg.Set disable_xhp_element_mangling,
        "Disable mangling of XHP elements :foo. That is, :foo:bar is now \\foo\\bar, not xhp_foo__bar"
      );
      ( "--disable-xhp-children-declarations",
        Arg.Set disable_xhp_children_declarations,
        "Disable XHP children declarations, e.g. children (foo, bar+)" );
      ( "--enable-xhp-class-modifier",
        Arg.Set enable_xhp_class_modifier,
        "Enable the XHP class modifier, xhp class name {} will define an xhp class."
      );
      ( "--auto-complete",
        Arg.Unit (set_mode Autocomplete),
        " Produce autocomplete suggestions as if triggered by trigger character"
      );
      ( "--auto-complete-manually-invoked",
        Arg.Unit (set_mode Autocomplete_manually_invoked),
        " Produce autocomplete suggestions as if manually triggered by user" );
      ( "--manifold-api-key",
        Arg.String (set "manifold api key" saved_state_manifold_api_key),
        " API key used to download a saved state from Manifold (optional)" );
    ]
  in
  let options = Arg.align ~limit:25 options in
  Arg.parse options (fun fn -> fn_ref := fn :: !fn_ref) usage;
  let fns =
    match (!fn_ref, !mode) with
    | ([], _) -> die usage
    | (x, _) -> x
  in

  let root = Path.make "/" (* we use this dummy *) in

  let tcopt =
    GlobalOptions.set
      ~tco_saved_state:
        (GlobalOptions.default_saved_state
        |> GlobalOptions.with_saved_state_manifold_api_key
             !saved_state_manifold_api_key)
      ~tco_check_xhp_attribute:!check_xhp_attribute
      ~po_disable_xhp_element_mangling:!disable_xhp_element_mangling
      ~po_disable_xhp_children_declarations:!disable_xhp_children_declarations
      ~po_enable_xhp_class_modifier:!enable_xhp_class_modifier
      ~tco_everything_sdt:true
      ~tco_enable_sound_dynamic:true
      GlobalOptions.default
  in
  (* Configure symbol index settings *)
  let namespace_map = ParserOptions.auto_namespace_map tcopt in
  let sienv =
    SymbolIndex.initialize
      ~globalrev:None
      ~gleanopt:tcopt
      ~namespace_map
      ~provider_name:"LocalIndex"
      ~quiet:true
      ~savedstate_file_opt:None
      ~workers:None
  in
  let sienv =
    {
      sienv with
      SearchUtils.sie_resolve_signatures = true;
      SearchUtils.sie_resolve_positions = true;
      SearchUtils.sie_resolve_local_decl = true;
    }
  in
  ( {
      files = fns;
      extra_builtins = !extra_builtins;
      mode = !mode;
      no_builtins = !no_builtins;
      tcopt;
    },
    sienv,
    root,
    SharedMem.default_config )

(** This is an almost-pure function which returns what we get out of parsing.
The only side-effect it has is on the global errors list. *)
let parse_and_name ctx files_contents =
  Relative_path.Map.mapi files_contents ~f:(fun fn contents ->
      (* Get parse errors. *)
      let () =
        Errors.run_in_context fn (fun () ->
            let popt = Provider_context.get_tcopt ctx in
            let parsed_file =
              Full_fidelity_ast.defensive_program popt fn contents
            in
            let ast =
              let { Parser_return.ast; _ } = parsed_file in
              if ParserOptions.deregister_php_stdlib popt then
                Nast.deregister_ignored_attributes ast
              else
                ast
            in
            Ast_provider.provide_ast_hint fn ast Ast_provider.Full;
            ())
      in
      match Direct_decl_utils.direct_decl_parse ctx fn with
      | None -> failwith "no file contents"
      | Some decls -> Direct_decl_utils.decls_to_fileinfo fn decls)

(** This function is used for gathering naming and parsing errors,
and the side-effect of updating the global reverse naming table (and
picking up duplicate-name errors along the way), and for the side effect
of updating the decl heap (and picking up decling errors along the way). *)
let parse_name_and_decl ctx files_contents =
  Errors.do_ (fun () ->
      (* parse_and_name has side effect of reporting errors *)
      let files_info = parse_and_name ctx files_contents in
      (* ndecl_file has side effect of updating the global reverse naming-table,
         and reporting errors. *)
      Relative_path.Map.iter files_info ~f:(fun fn fileinfo ->
          let _failed_naming_fns =
            Naming_global.ndecl_file_and_get_conflict_files ctx fn fileinfo
          in
          ());
      (* Decl.make_env has the side effect of updating the decl heap, and
         reporting errors. *)
      Relative_path.Map.iter files_info ~f:(fun fn _ ->
          Decl.make_env ~sh:SharedMem.Uses ctx fn);
      files_info)

let scan_files_for_symbol_index
    (filename : Relative_path.t)
    (sienv : SearchUtils.si_env)
    (ctx : Provider_context.t) : SearchUtils.si_env =
  let files_contents = Multifile.file_to_files filename in
  let individual_file_info =
    Errors.ignore_ (fun () -> parse_and_name ctx files_contents)
  in
  let fileinfo_list = Relative_path.Map.values individual_file_info in
  let transformed_list =
    List.map fileinfo_list ~f:(fun fileinfo ->
        (filename, fileinfo, SearchUtils.TypeChecker))
  in
  SymbolIndexCore.update_files ~ctx ~sienv ~paths:transformed_list

let handle_mode mode filenames ctx (sienv : SearchUtils.si_env) naming_table =
  let expect_single_file () : Relative_path.t =
    match filenames with
    | [x] -> x
    | _ -> die "Only single file expected"
  in
  match mode with
  | NoMode -> die "Exactly one mode must be setup"
  | Autocomplete
  | Autocomplete_manually_invoked ->
    let path = expect_single_file () in
    let contents = cat (Relative_path.to_absolute path) in
    (* Search backwards: there should only be one /real/ case. If there's multiple, *)
    (* guess that the others are preceding explanation comments *)
    let offset =
      Str.search_backward
        (Str.regexp AutocompleteTypes.autocomplete_token)
        contents
        (String.length contents)
    in
    let pos = File_content.offset_to_position contents offset in
    let is_manually_invoked =
      match mode with
      | Autocomplete_manually_invoked -> true
      | _ -> false
    in
    let (ctx, entry) =
      Provider_context.add_or_overwrite_entry_contents ~ctx ~path ~contents
    in
    let autocomplete_context =
      ServerAutoComplete.get_autocomplete_context
        ~file_content:contents
        ~pos
        ~is_manually_invoked
    in
    let sienv = scan_files_for_symbol_index path sienv ctx in
    let result =
      ServerAutoComplete.go_at_auto332_ctx
        ~ctx
        ~entry
        ~sienv
        ~autocomplete_context
        ~naming_table
    in
    List.iter
      ~f:
        begin
          fun r ->
            begin
              let open AutocompleteTypes in
              Printf.printf "%s\n" r.res_label;
              List.iter r.res_additional_edits ~f:(fun (s, _) ->
                  Printf.printf "  INSERT %s\n" s);
              Printf.printf
                "  INSERT %s\n"
                (match r.res_insert_text with
                | InsertLiterally s -> s
                | InsertAsSnippet { snippet; _ } -> snippet);
              Printf.printf "  %s\n" r.res_detail;
              match r.res_documentation with
              | Some doc ->
                List.iter (String.split_lines doc) ~f:(fun line ->
                    Printf.printf "  %s\n" line)
              | None -> ()
            end
        end
      result.Utils.With_complete_flag.value

(*****************************************************************************)
(* Main entry point *)
(*****************************************************************************)

let decl_and_run_mode
    { files; extra_builtins; mode; no_builtins; tcopt }
    (popt : TypecheckerOptions.t)
    (hhi_root : Path.t)
    (sienv : SearchUtils.si_env) : unit =
  Ident.track_names := true;
  let builtins =
    if no_builtins then
      Relative_path.Map.empty
    else
      let extra_builtins =
        let add_file_content map filename =
          Relative_path.create Relative_path.Dummy filename
          |> Multifile.file_to_file_list
          |> List.map ~f:(fun (path, contents) ->
                 (Filename.basename (Relative_path.suffix path), contents))
          |> List.unordered_append map
        in
        extra_builtins
        |> List.fold ~f:add_file_content ~init:[]
        |> Array.of_list
      in
      let magic_builtins = Array.append magic_builtins extra_builtins in
      (* Check that magic_builtin filenames are unique *)
      let () =
        let n_of_builtins = Array.length magic_builtins in
        let n_of_unique_builtins =
          Array.to_list magic_builtins
          |> List.map ~f:fst
          |> SSet.of_list
          |> SSet.cardinal
        in
        if n_of_builtins <> n_of_unique_builtins then
          die "Multiple magic builtins share the same base name.\n"
      in
      Array.iter magic_builtins ~f:(fun (file_name, file_contents) ->
          let file_path = Path.concat hhi_root file_name in
          let file = Path.to_string file_path in
          Sys_utils.try_touch
            (Sys_utils.Touch_existing { follow_symlinks = true })
            file;
          Sys_utils.write_file ~file file_contents);

      (* Take the builtins (file, contents) array and create relative paths *)
      Array.fold
        (Array.append magic_builtins hhi_builtins)
        ~init:Relative_path.Map.empty
        ~f:(fun acc (f, src) ->
          let f = Path.concat hhi_root f |> Path.to_string in
          Relative_path.Map.add
            acc
            ~key:(Relative_path.create Relative_path.Hhi f)
            ~data:src)
  in
  let files = files |> List.map ~f:(Relative_path.create Relative_path.Dummy) in
  let files_contents =
    List.fold
      files
      ~f:(fun acc filename ->
        let files_contents = Multifile.file_to_files filename in
        Relative_path.Map.union acc files_contents)
      ~init:Relative_path.Map.empty
  in
  (* Merge in builtins *)
  let files_contents_with_builtins =
    Relative_path.Map.fold
      builtins
      ~f:
        begin
          (fun k src acc -> Relative_path.Map.add acc ~key:k ~data:src)
        end
      ~init:files_contents
  in
  Relative_path.Map.iter files_contents ~f:(fun filename contents ->
      File_provider.provide_file_for_tests filename contents);
  let to_decl = files_contents_with_builtins in
  let ctx =
    Provider_context.empty_for_test
      ~popt
      ~tcopt
      ~deps_mode:(Typing_deps_mode.InMemoryMode None)
  in
  let (_errors, files_info) = parse_name_and_decl ctx to_decl in
  let naming_table = Naming_table.create files_info in
  handle_mode mode files ctx sienv naming_table

let main_hack
    ({ tcopt; _ } as opts)
    (sienv : SearchUtils.si_env)
    (root : Path.t)
    (sharedmem_config : SharedMem.config) : unit =
  (* TODO: We should have a per file config *)
  Sys_utils.signal Sys.sigusr1 (Sys.Signal_handle Typing.debug_print_last_pos);
  EventLogger.init_fake ();

  let (_handle : SharedMem.handle) =
    SharedMem.init ~num_workers:0 sharedmem_config
  in
  Tempfile.with_tempdir (fun hhi_root ->
      Hhi.set_hhi_root_for_unit_test hhi_root;
      Relative_path.set_path_prefix Relative_path.Root root;
      Relative_path.set_path_prefix Relative_path.Hhi hhi_root;
      Relative_path.set_path_prefix Relative_path.Tmp (Path.make "tmp");
      decl_and_run_mode opts tcopt hhi_root sienv;
      TypingLogger.flush_buffers ())

(* command line driver *)
let () =
  if !Sys.interactive then
    ()
  else
    (* On windows, setting 'binary mode' avoids to output CRLF on
       stdout.  The 'text mode' would not hurt the user in general, but
       it breaks the testsuite where the output is compared to the
       expected one (i.e. in given file without CRLF). *)
    Out_channel.set_binary_mode stdout true;
  let (options, sienv, root, sharedmem_config) = parse_options () in
  Unix.handle_unix_error main_hack options sienv root sharedmem_config
