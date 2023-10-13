(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(*****************************************************************************)
(* Types, constants *)
(*****************************************************************************)

type options = {
  files: string list;
  extra_builtins: string list;
  ai_options: Ai_options.t;
  error_format: Errors.format;
  no_builtins: bool;
  tcopt: GlobalOptions.t;
}

(** If the user passed --root, then all pathnames have to be canonicalized.
The fact of whether they passed --root is kind of stored inside Relative_path
global variables: the Relative_path.(path_of_prefix Root) is either "/"
if they failed to pass something, or the thing that they passed. *)
let use_canonical_filenames () =
  not (String.equal "/" (Relative_path.path_of_prefix Relative_path.Root))

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
      ^ "function hh_expect(<<__AcceptDisposable>> $val) {}\n"
      ^ "function hh_expect_equivalent(<<__AcceptDisposable>> $val) {}\n"
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

let print_error format ?(oc = stderr) l =
  let formatter =
    match format with
    | Errors.Context -> (fun e -> Contextual_error_formatter.to_string e)
    | Errors.Raw -> (fun e -> Raw_error_formatter.to_string e)
    | Errors.Plain -> (fun e -> Errors.to_string e)
    | Errors.Highlighted -> Highlighted_error_formatter.to_string
  in
  let absolute_errors =
    if use_canonical_filenames () then
      User_error.to_absolute l
    else
      User_error.to_absolute_for_test l
  in
  Out_channel.output_string oc (formatter absolute_errors)

let comma_string_to_iset (s : string) : ISet.t =
  Str.split (Str.regexp ", *") s |> List.map ~f:int_of_string |> ISet.of_list

let parse_options () =
  let fn_ref = ref [] in
  let extra_builtins = ref [] in
  let usage = Printf.sprintf "Usage: %s filename\n" Sys.argv.(0) in
  let no_builtins = ref false in
  let ai_options = ref None in
  let set_ai_options x =
    let options = Ai_options.prepare ~server:false x in
    match !ai_options with
    | None -> ai_options := Some options
    | Some existing ->
      ai_options := Some (Ai_options.merge_for_unit_tests existing options)
  in
  let error_format = ref Errors.Highlighted in
  let check_xhp_attribute = ref false in
  let disable_xhp_element_mangling = ref false in
  let disable_xhp_children_declarations = ref false in
  let enable_xhp_class_modifier = ref false in
  let allowed_fixme_codes_strict = ref None in
  let disable_hh_ignore_error = ref 0 in
  let allowed_decl_fixme_codes = ref None in
  let options =
    [
      ( "--extra-builtin",
        Arg.String (fun f -> extra_builtins := f :: !extra_builtins),
        " HHI file to parse and declare" );
      ( "--ai",
        Arg.String set_ai_options,
        " Run the abstract interpreter (Zoncolan)" );
      ( "--error-format",
        Arg.String
          (fun s ->
            match s with
            | "raw" -> error_format := Errors.Raw
            | "plain" -> error_format := Errors.Plain
            | "context" -> error_format := Errors.Context
            | "highlighted" -> error_format := Errors.Highlighted
            | _ -> print_string "Warning: unrecognized error format.\n"),
        "<raw|context|highlighted|plain> Error formatting style; (default: highlighted)"
      );
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
      ( "--allowed-fixme-codes-strict",
        Arg.String
          (fun s -> allowed_fixme_codes_strict := Some (comma_string_to_iset s)),
        "List of fixmes that are allowed in strict mode." );
      ( "--disable-hh-ignore-error",
        Arg.Int (( := ) disable_hh_ignore_error),
        " Forbid HH_IGNORE_ERROR comments as an alternative to HH_FIXME, or treat them as normal comments."
      );
      ( "--allowed-decl-fixme-codes",
        Arg.String
          (fun s -> allowed_decl_fixme_codes := Some (comma_string_to_iset s)),
        "List of fixmes that are allowed in declarations." );
    ]
  in
  let options = Arg.align ~limit:25 options in
  Arg.parse options (fun fn -> fn_ref := fn :: !fn_ref) usage;
  let (fns, ai_options) =
    match (!fn_ref, !ai_options) with
    | ([], _)
    | (_, None) ->
      die usage
    | (x, Some ai_options) -> (x, ai_options)
  in

  let root = Path.make "/" (* if none specified, we use this dummy *) in
  let tcopt =
    GlobalOptions.set
      ~tco_saved_state:GlobalOptions.default_saved_state
      ~allowed_fixme_codes_strict:
        (Option.value !allowed_fixme_codes_strict ~default:ISet.empty)
      ~po_disable_hh_ignore_error:!disable_hh_ignore_error
      ~po_keep_user_attributes:true
      ~tco_check_xhp_attribute:!check_xhp_attribute
      ~po_disable_xhp_element_mangling:!disable_xhp_element_mangling
      ~po_disable_xhp_children_declarations:!disable_xhp_children_declarations
      ~po_enable_xhp_class_modifier:!enable_xhp_class_modifier
      ~po_allowed_decl_fixme_codes:
        (Option.value !allowed_decl_fixme_codes ~default:ISet.empty)
      ~tco_enable_sound_dynamic:true
      ~tco_everything_sdt:true
      GlobalOptions.default
  in
  Errors.allowed_fixme_codes_strict :=
    GlobalOptions.allowed_fixme_codes_strict tcopt;
  Errors.report_pos_from_reason :=
    TypecheckerOptions.report_pos_from_reason tcopt;
  ( {
      files = fns;
      extra_builtins = !extra_builtins;
      ai_options;
      error_format = !error_format;
      no_builtins = !no_builtins;
      tcopt;
    },
    root,
    None,
    Ai_options.modify_shared_mem ai_options SharedMem.default_config )

let parse_and_name ctx files_contents =
  Relative_path.Map.mapi files_contents ~f:(fun fn contents ->
      (* Get parse errors *)
      let _ =
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
      match Direct_decl_utils.direct_decl_parse_and_cache ctx fn with
      | None -> failwith "no file contents"
      | Some decls -> Direct_decl_utils.decls_to_fileinfo fn decls)

let parse_name_and_skip_decl ctx files_contents =
  Errors.do_ (fun () ->
      let files_info = parse_and_name ctx files_contents in
      Relative_path.Map.iter files_info ~f:(fun fn fileinfo ->
          let _failed_naming_fns =
            Naming_global.ndecl_file_and_get_conflict_files ctx fn fileinfo
          in
          ());
      files_info)

let handle_mode ai_options ctx files_info parse_errors error_format =
  if not (List.is_empty parse_errors) then
    List.iter ~f:(print_error error_format) parse_errors
  else
    (* No type check *)
    Ai.do_ files_info ai_options ctx

(*****************************************************************************)
(* Main entry point *)
(*****************************************************************************)

let decl_and_run_mode
    { files; extra_builtins; ai_options; error_format; no_builtins; tcopt }
    (popt : TypecheckerOptions.t)
    (hhi_root : Path.t)
    (naming_table_path : string option) : unit =
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
  let files =
    if use_canonical_filenames () then
      files
      |> List.map ~f:Sys_utils.realpath
      |> List.map ~f:(fun s -> Option.value_exn s)
      |> List.map ~f:Relative_path.create_detect_prefix
    else
      files |> List.map ~f:(Relative_path.create Relative_path.Dummy)
  in
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
      File_provider.(provide_file_for_tests filename contents));
  (* Don't declare all the filenames in batch_errors mode *)
  let to_decl = files_contents_with_builtins in
  let ctx =
    Provider_context.empty_for_test
      ~popt
      ~tcopt
      ~deps_mode:(Typing_deps_mode.InMemoryMode None)
  in
  (* We make the following call for the side-effect of updating ctx's "naming-table fallback"
     so it will look in the sqlite database for names it doesn't know.
     This function returns the forward naming table, but we don't care about that;
     it's only needed for tools that process file changes, to know in the event
     of a file-change which old symbols used to be defined in the file. *)
  let _naming_table_for_root : Naming_table.t option =
    Option.map naming_table_path ~f:(fun path ->
        Naming_table.load_from_sqlite ctx path)
  in
  let (errors, files_info) = parse_name_and_skip_decl ctx to_decl in
  let ctx = Provider_context.set_backend ctx Provider_backend.Analysis in
  handle_mode
    ai_options
    ctx
    files_info
    (Errors.get_sorted_error_list errors)
    error_format

let main_hack
    ({ tcopt; _ } as opts)
    (root : Path.t)
    (naming_table : string option)
    (sharedmem_config : SharedMem.config) : unit =
  Folly.ensure_folly_init ();
  Sys_utils.signal Sys.sigusr1 (Sys.Signal_handle Typing.debug_print_last_pos);
  EventLogger.init_fake ();

  let () = FlashSharedMem.pre_init None in
  let (_handle : SharedMem.handle) =
    SharedMem.init ~num_workers:0 sharedmem_config
  in
  Decl_store.set Ai_decl_heap.decl_store;
  Tempfile.with_tempdir (fun hhi_root ->
      Hhi.set_hhi_root_for_unit_test hhi_root;
      Relative_path.set_path_prefix Relative_path.Root root;
      Relative_path.set_path_prefix Relative_path.Hhi hhi_root;
      Relative_path.set_path_prefix Relative_path.Tmp (Path.make "tmp");
      decl_and_run_mode opts tcopt hhi_root naming_table;
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
  let (options, root, naming_table, sharedmem_config) = parse_options () in
  Unix.handle_unix_error main_hack options root naming_table sharedmem_config
