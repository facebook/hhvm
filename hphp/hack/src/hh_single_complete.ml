(*
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let comma_string_to_iset (s : string) : ISet.t =
  Str.split (Str.regexp ", *") s |> List.map ~f:int_of_string |> ISet.of_list

(*****************************************************************************)
(* Types, constants *)
(*****************************************************************************)

type mode =
  | NoMode
  | Autocomplete of { is_manually_invoked: bool }
  | Autocomplete_glean of { dry_run: bool }
  | Search of {
      glean_only: bool;  (** if true, uses glean; if false, uses ServerSearch *)
      dry_run: bool;
          (** only applies to [glean_only]; skips actually running the glean query *)
    }
  | Findrefs_glean of { dry_run: bool }
  | Resolve of { is_manually_invoked: bool }
  | XHP_close

type options = {
  files: string list;
  extra_builtins: string list;
  mode: mode;
  no_builtins: bool;
  naming_table_path: string option;
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
  let root = ref None in
  let naming_table = ref None in
  let saved_state_manifold_api_key = ref None in
  let glean_reponame = ref "" in
  let auto_namespace_map = ref None in
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
      ( "--auto-namespace-map",
        Arg.String
          (fun m ->
            auto_namespace_map :=
              Some (ServerConfig.convert_auto_namespace_to_map m)),
        " Alias namespaces" );
      ( "--auto-complete",
        Arg.Unit (set_mode (Autocomplete { is_manually_invoked = false })),
        " Produce autocomplete suggestions as if triggered by trigger character"
      );
      ( "--auto-complete-manually-invoked",
        Arg.Unit (set_mode (Autocomplete { is_manually_invoked = true })),
        " Produce autocomplete suggestions as if manually triggered by user" );
      ( "--auto-complete-show-glean",
        Arg.Unit (set_mode (Autocomplete_glean { dry_run = true })),
        " Show the glean query for the prefix contained in the file" );
      ( "--auto-complete-glean",
        Arg.Unit (set_mode (Autocomplete_glean { dry_run = false })),
        " Show the glean query for the prefix contained in the file, and run that query"
      );
      ( "--search-show-glean",
        Arg.Unit (set_mode (Search { glean_only = true; dry_run = true })),
        " Show the glean query for newline-separated queries in file" );
      ( "--search-glean",
        Arg.Unit (set_mode (Search { glean_only = true; dry_run = false })),
        " Show+run the glean query for newline-separated queries in file" );
      ( "--search",
        Arg.Unit (set_mode (Search { glean_only = false; dry_run = false })),
        " Run the search (including new definitions) for newline-separated queries in file"
      );
      ( "--find-refs-show-glean",
        Arg.Unit (set_mode (Findrefs_glean { dry_run = true })),
        " Show the glean query for newline-separated queries in file" );
      ( "--find-refs-glean",
        Arg.Unit (set_mode (Findrefs_glean { dry_run = false })),
        " Show+run the glean query for newline-separated queries in file" );
      ( "--manifold-api-key",
        Arg.String (set "manifold api key" saved_state_manifold_api_key),
        " API key used to download a saved state from Manifold (optional)" );
      ( "--naming-table",
        Arg.String (fun s -> naming_table := Some s),
        " Naming table, to typecheck undefined symbols; needs --root."
        ^ " (Hint: buck2 run //hphp/hack/src/hh_naming_table_builder)" );
      ( "--resolve",
        Arg.Unit (set_mode (Resolve { is_manually_invoked = false })),
        " Produce what is returned from Completion_resolve requests - including signature and docblock"
      );
      ( "--xhp-close",
        Arg.Unit (set_mode XHP_close),
        " Produce what is returned from AutoClose requests, aka the closing xhp tag"
      );
      ( "--root",
        Arg.String (fun s -> root := Some s),
        " Root for where to typecheck undefined symbols; needs --naming-table"
      );
      ( "--glean-reponame",
        Arg.String (fun str -> glean_reponame := str),
        " Glean repo for autocompleting undefined symbols" );
    ]
  in
  let options = Arg.align ~limit:25 options in
  Arg.parse options (fun fn -> fn_ref := fn :: !fn_ref) usage;
  let fns =
    match (!fn_ref, !mode) with
    | ([], _) -> die usage
    | (x, _) -> x
  in

  if Option.is_some !naming_table && Option.is_none !root then
    failwith "--naming-table needs --root";

  begin
    match !mode with
    | Search { glean_only = false; _ }
      when (not (String.is_empty !glean_reponame))
           && (Option.is_none !root || Option.is_none !naming_table) ->
      (* --search --glean-reponame looks up glean to find symbol names, but then
         uses reverse-naming-table and root to fetch the AST and find its position. *)
      failwith "--search --glean-reponame needs --naming-table and --root"
    | _ -> ()
  end;

  (* --root implies certain things... *)
  let (allowed_fixme_codes_strict, sharedmem_config, root) =
    match !root with
    | None ->
      let allowed_fixme_codes_strict = None in
      let sharedmem_config = SharedMem.default_config in
      let root = Path.make "/" (* if none specified, we use this dummy *) in
      (allowed_fixme_codes_strict, sharedmem_config, root)
    | Some root ->
      if Option.is_none !naming_table then
        failwith "--root needs --naming-table";

      (* builtins are already provided by project at --root, so we shouldn't provide our own *)
      no_builtins := true;
      (* Following will throw an exception if .hhconfig not found *)
      let (_config_hash, config) =
        Config_file.parse_hhconfig
          (Filename.concat root Config_file.file_path_relative_to_repo_root)
      in
      (* We will pick up values from .hhconfig, unless they've been overridden at the command-line. *)
      if Option.is_none !auto_namespace_map then
        auto_namespace_map :=
          config
          |> Config_file.Getters.string_opt "auto_namespace_map"
          |> Option.map ~f:ServerConfig.convert_auto_namespace_to_map;
      let allowed_fixme_codes_strict =
        config
        |> Config_file.Getters.string_opt "allowed_fixme_codes_strict"
        |> Option.map ~f:comma_string_to_iset
      in
      let sharedmem_config =
        ServerConfig.make_sharedmem_config
          config
          (ServerArgs.default_options ~root)
          ServerLocalConfig.default
      in
      (* Path.make canonicalizes it, i.e. resolves symlinks *)
      let root = Path.make root in
      (allowed_fixme_codes_strict, sharedmem_config, root)
  in

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
      ?po_auto_namespace_map:!auto_namespace_map
      ~allowed_fixme_codes_strict:
        (Option.value allowed_fixme_codes_strict ~default:ISet.empty)
      ~glean_reponame:!glean_reponame
      GlobalOptions.default
  in
  ( {
      files = fns;
      extra_builtins = !extra_builtins;
      mode = !mode;
      no_builtins = !no_builtins;
      naming_table_path = !naming_table;
      tcopt;
    },
    root,
    sharedmem_config )

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
      | Some decls ->
        ( Direct_decl_parser.decls_to_fileinfo fn decls,
          Direct_decl_parser.decls_to_addenda decls ))

(** This function is used for gathering naming and parsing errors,
and the side-effect of updating the global reverse naming table (and
picking up duplicate-name errors along the way), and for the side effect
of updating the decl heap (and picking up decling errors along the way). *)
let parse_name_and_decl ctx files_contents =
  Errors.do_ (fun () ->
      (* parse_and_name has side effect of reporting errors *)
      let files_info_and_addenda = parse_and_name ctx files_contents in
      (* ndecl_file has side effect of updating the global reverse naming-table,
         and reporting errors. *)
      Relative_path.Map.iter
        files_info_and_addenda
        ~f:(fun fn (fileinfo, _addenda) ->
          let _failed_naming_fns =
            Naming_global.ndecl_file_and_get_conflict_files ctx fn fileinfo
          in
          ());
      (* Decl.make_env has the side effect of updating the decl heap, and
         reporting errors. *)
      Relative_path.Map.iter files_info_and_addenda ~f:(fun fn _ ->
          Decl.make_env ~sh:SharedMem.Uses ctx fn);
      files_info_and_addenda)

(** Our tests expect files that contain newline-separated queries.
This function takes a file and splits it on newlines into queries,
trimming and filtering out blank lines and //comment lines.
To help avoid foot-guns (in the form of the tester accidentally forgetting <?hh
so that hack source code would get interpreted as queries),
we fail if any line contains a space in it... all the queries
we care about don't have spaces in it.
*)
let extract_nonblank_lines contents =
  String_utils.split_on_newlines contents
  |> List.map ~f:String.strip
  |> List.filter ~f:(fun s ->
         if String.is_empty s then
           false
         else if String.is_prefix s ~prefix:"//" then
           false
         else begin
           if String.is_substring s ~substring:" " then
             failwith
               ("Files must be either hack with <?hh, or lists of query_text; not "
               ^ s);
           true
         end)

(** This helper invokes ServerAutoComplete, with the additional
arguments it needs to describe the "context" of the AUTO332
i.e. surrounding characters. *)
let do_auto332
    ~(ctx : Provider_context.t)
    ~(is_manually_invoked : bool)
    ~(sienv_ref : SearchUtils.si_env ref)
    ~(naming_table : Naming_table.t)
    (path : Relative_path.t)
    (contents : string) :
    AutocompleteTypes.autocomplete_item list Utils.With_complete_flag.t =
  (* Search backwards: there should only be one /real/ case. If there's multiple, *)
  (* guess that the others are preceding explanation comments *)
  let offset =
    Str.search_backward
      (Str.regexp AutocompleteTypes.autocomplete_token)
      contents
      (String.length contents)
  in
  let pos = File_content.offset_to_position contents offset in
  let (ctx, entry) =
    Provider_context.add_or_overwrite_entry_contents ~ctx ~path ~contents
  in
  let autocomplete_context =
    ServerAutoComplete.get_autocomplete_context
      ~file_content:contents
      ~pos
      ~is_manually_invoked
  in
  ServerAutoComplete.go_at_auto332_ctx
    ~ctx
    ~entry
    ~sienv_ref
    ~autocomplete_context
    ~naming_table

(** This function takes a list of searches of the form (title, query_text, context, kind_filter).
If [show_query_text] then it prints each search.
It obtains and prints the Glean angle query for it, using [Glean_autocomplete_query.make_symbols_query].
If not [dry_run] then it runs the query against [reponame] and displays the results, plus timing information. *)
let do_glean_symbol_searches
    ~reponame
    ~dry_run
    ~show_query_text
    (searches :
      (string
      * string
      * SearchTypes.autocomplete_type
      * SearchTypes.si_kind option)
      list) : unit =
  let handle =
    if dry_run then
      None
    else
      let () =
        if String.is_empty reponame then failwith "--glean-reponame required"
      in
      let () = Folly.ensure_folly_init () in
      Some (Glean.initialize ~reponame ~prev_init_time:None |> Option.value_exn)
  in
  List.iter searches ~f:(fun (title, query_text, context, kind_filter) ->
      if List.length searches > 1 then Printf.printf "//// %s\n" title;
      let angle =
        Glean_autocomplete_query.make_symbols_query
          ~prefix:query_text
          ~context
          ~kind_filter
      in
      let query_text_for_show =
        Printf.sprintf
          "%s [%s,%s]"
          query_text
          (SearchTypes.show_autocomplete_type context)
          (Option.value_map
             kind_filter
             ~default:"*"
             ~f:SearchTypes.show_si_kind)
      in
      if show_query_text then
        Printf.printf "query_text:\n%s\n\n" query_text_for_show;
      Printf.printf "query:\n%s\n\n%!" angle;
      if not dry_run then begin
        let start_time = Unix.gettimeofday () in
        let (results, _is_complete) =
          Glean.query_autocomplete
            (Option.value_exn handle)
            ~query_text
            ~max_results:100
            ~context
            ~kind_filter
        in
        List.iter
          results
          ~f:(fun { SearchTypes.si_name; si_kind; si_file; si_fullname = _ } ->
            let file =
              match si_file with
              | SearchTypes.SI_Filehash hash -> Printf.sprintf "#%s" hash
              | SearchTypes.SI_Path path -> Relative_path.show path
            in
            Printf.printf
              "[%s] %s - %s\n%!"
              (SearchTypes.show_si_kind si_kind)
              si_name
              file);
        Printf.printf
          "\n--> %s - %d results, %0.3fs\n\n%!"
          query_text_for_show
          (List.length results)
          (Unix.gettimeofday () -. start_time)
      end)

(** This handles "--auto-complete-show-glean" and "--auto-complete-glean".
The input [filename] is either a single file or a multifile.
It can contain <?hh files, or newline-separated query_text files.
In the case of <?hh files, it figures out query_text+context+filter based on the AUTO332 in that file (if any).
Once it has the search (comprising query_text, context, filter)
it either simply shows the glean angle query,
or also runs the query against glean and shows the results. *)
let handle_autocomplete_glean ctx sienv naming_table ~dry_run filename =
  let files_contents = Multifile.file_to_file_list filename in
  let any_hack_files =
    List.exists files_contents ~f:(fun (_path, content) ->
        String.is_prefix content ~prefix:"<?hh")
  in
  let searches =
    if any_hack_files then
      files_contents
      |> List.filter_map ~f:(fun (path, contents) ->
             (* We're going to run the file through autocomplete, but with a mock sienv, one which tells us
                what [SymbolIndex.find] was performed -- i.e. what query_text, context, filter. *)
             let search = ref None in
             let mock_sienv =
               SymbolIndex.mock
                 ~on_find:(fun ~query_text ~context ~kind_filter ->
                   search :=
                     Some
                       ( Multifile.short_suffix path,
                         query_text,
                         context,
                         kind_filter );
                   [])
             in
             let sienv_ref = ref mock_sienv in
             let _results =
               do_auto332
                 ~ctx
                 ~is_manually_invoked:true
                 ~naming_table
                 ~sienv_ref
                 path
                 contents
             in
             !search)
    else
      files_contents
      |> List.concat_map ~f:(fun (_path, contents) ->
             extract_nonblank_lines contents
             |> List.map ~f:(fun s -> (s, s, SearchTypes.Acid, None)))
  in
  do_glean_symbol_searches
    searches
    ~reponame:sienv.SearchUtils.glean_reponame
    ~dry_run
    ~show_query_text:any_hack_files;
  ()

(** This handles "--auto-complete" and "--auto-complete-manually-invoked".
It parses the input file/multifiles for AUTO332, and runs them through
ServerAutoComplete, and shows the results. These results will include
both locally defined symbols in input files if present, and glean
results if --glean-reponame has been provided. *)
let handle_autocomplete ctx sienv naming_table ~is_manually_invoked filename =
  let files_contents = Multifile.file_to_file_list filename in
  let files_with_token =
    files_contents
    |> List.filter ~f:(fun (_path, contents) ->
           String.is_substring
             contents
             ~substring:AutocompleteTypes.autocomplete_token)
  in
  let show_file_titles = List.length files_with_token > 1 in
  List.iter files_with_token ~f:(fun (path, contents) ->
      let sienv_ref = ref sienv in
      let result =
        do_auto332
          ~ctx
          ~is_manually_invoked
          ~sienv_ref
          ~naming_table
          path
          contents
      in
      if show_file_titles then
        Printf.printf "//// %s\n" (Multifile.short_suffix path);
      List.iter result.Utils.With_complete_flag.value ~f:(fun r ->
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
          | None -> ()))

(** This handles "--xhp-close"  *)
let handle_xhp_close ctx _ _ filename =
  let files_contents = Multifile.file_to_file_list filename in
  let files_with_token =
    files_contents
    |> List.filter ~f:(fun (_path, contents) ->
           String.is_substring contents ~substring:"AUTOCLOSE332")
  in
  List.iter files_with_token ~f:(fun (path, contents) ->
      let (ctx, entry) =
        Provider_context.add_or_overwrite_entry_contents ~ctx ~path ~contents
      in
      let offset = String_utils.substring_index "AUTOCLOSE332" contents in
      let position = File_content.offset_to_position contents offset in
      let line = position.File_content.line in
      let column = position.File_content.column in

      let close_tag =
        AutocloseTags.go_xhp_close_tag ~ctx ~entry ~line ~column
      in
      match close_tag with
      | Some close_tag -> Printf.printf "Close Tag: %s\n" close_tag
      | None -> Printf.printf "Close Tag: None")

(** This handles "--resolve" which for now, just autocomplete and shows the results
and the signature (next up - docblock!), AKA whatever is already done
in ClientIdeDaemon in response to Completion_resolve requests. *)
let handle_resolve ctx sienv naming_table ~is_manually_invoked filename =
  let files_contents = Multifile.file_to_file_list filename in
  let files_with_token =
    files_contents
    |> List.filter ~f:(fun (_path, contents) ->
           String.is_substring
             contents
             ~substring:AutocompleteTypes.autocomplete_token)
  in
  let show_file_titles =
    match files_with_token with
    | []
    | [_] ->
      false
    | _ -> true
  in
  List.iter files_with_token ~f:(fun (path, contents) ->
      let sienv_ref = ref sienv in
      let result =
        do_auto332
          ~ctx
          ~is_manually_invoked
          ~sienv_ref
          ~naming_table
          path
          contents
      in
      if show_file_titles then
        Printf.printf "//// %s\n" (Multifile.short_suffix path);
      List.iter result.Utils.With_complete_flag.value ~f:(fun r ->
          let open AutocompleteTypes in
          Printf.printf
            "%s\n"
            (Option.value
               ~default:"none"
               (ServerAutoComplete.get_signature ctx r.res_fullname));
          Printf.printf "%s\n" r.res_fullname;
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
          | None -> ()))

(** This handles --search, --search-glean, --search-show-glean.
The filename be a single file or a multifile,
and accepts both <?hh files that define symbols, and newline-separated
lists of search queries.
For each query, it either shows the angle query (--search-show-glean)
or shows the query and runs it and shows the results (--search-glean)
or runs the query through ServerSearch (--search) which will also pick up
any symbols defined in <?hh files. Note that in the final case, ServerSearch
uses the naming-table and root in [ctx] to validate the results it got from
glean -- to find their filename and load the file and get its AST -- and
if glean gives results without [ctx] being set up right then the glean
results will be filtered out. *)
let handle_search ctx sienv ~glean_only ~dry_run filename =
  let queries =
    Multifile.file_to_file_list filename
    |> List.filter ~f:(fun (_path, contents) ->
           not (String.is_prefix contents ~prefix:"<?hh"))
    |> List.concat_map ~f:(fun (_path, contents) ->
           extract_nonblank_lines contents)
  in
  if glean_only then
    queries
    |> List.map ~f:(fun query ->
           (query, query, SearchTypes.Ac_workspace_symbol, None))
    |> do_glean_symbol_searches
         ~reponame:sienv.SearchUtils.glean_reponame
         ~dry_run
         ~show_query_text:true
  else
    List.iter queries ~f:(fun query ->
        Printf.printf "query: %s\n%!" query;
        let sienv_ref = ref sienv in
        let results = ServerSearch.go ctx query ~kind_filter:"" sienv_ref in
        List.iter results ~f:(fun { SearchUtils.name; pos; result_type } ->
            let filename = Pos.filename pos |> Filename.basename in
            let (line, start_, end_) = Pos.info_pos pos in
            Printf.printf
              "  %s:%s - %s:%d:%d-%d\n%!"
              name
              (SearchTypes.show_si_kind result_type)
              filename
              line
              start_
              end_));
  ()

let handle_findrefs_glean sienv ~dry_run filename =
  (* We expect a newline-separated query file. See
     ServerCommandTypes.Find_refs.symbol_and_action_to_string_exn
     for documentation and examples of that format.

     Another way to obtain examples is to open a hack file in the IDE, right-click > FindRefs,
     and then examine [hh --client-logname] to find the invocation of [hh --ide-find-refs-by-symbol-name <EXAMPLE>].
  *)
  let queries =
    Sys_utils.cat (Relative_path.to_absolute filename) |> extract_nonblank_lines
  in
  let reponame = sienv.SearchUtils.glean_reponame in
  let handle =
    if dry_run then
      None
    else
      let () =
        if String.is_empty reponame then failwith "--glean-reponame required"
      in
      let () = Folly.ensure_folly_init () in
      Some (Glean.initialize ~reponame ~prev_init_time:None |> Option.value_exn)
  in
  List.iter queries ~f:(fun query ->
      let { FindRefsWireFormat.CliArgs.action; _ } =
        FindRefsWireFormat.CliArgs.from_string_exn query
      in
      Printf.printf "//// %s\n" query;
      let angle = Glean_autocomplete_query.make_refs_query ~action in
      Printf.printf "query:\n%s\n\n%!" angle;
      if not dry_run then begin
        let start_time = Unix.gettimeofday () in
        let results =
          Glean.query_refs (Option.value_exn handle) ~action ~max_results:10
        in
        List.iter results ~f:(fun path ->
            Printf.printf "%s\n" (Relative_path.show path));
        Printf.printf
          "\n--> %s - %d results, %0.3fs\n\n%!"
          query
          (List.length results)
          (Unix.gettimeofday () -. start_time)
      end);
  ()

let handle_mode mode filenames ctx (sienv : SearchUtils.si_env) naming_table =
  let filename =
    match filenames with
    | [x] -> x
    | _ -> die "Only single file expected"
  in
  match mode with
  | NoMode -> die "Exactly one mode must be set up"
  | Search { glean_only; dry_run } ->
    handle_search ctx sienv ~glean_only ~dry_run filename
  | Autocomplete_glean { dry_run } ->
    handle_autocomplete_glean ctx sienv naming_table ~dry_run filename
  | Autocomplete { is_manually_invoked } ->
    handle_autocomplete ctx sienv naming_table ~is_manually_invoked filename
  | Findrefs_glean { dry_run } -> handle_findrefs_glean sienv ~dry_run filename
  | Resolve { is_manually_invoked } ->
    handle_resolve ctx sienv naming_table ~is_manually_invoked filename
  | XHP_close -> handle_xhp_close ctx sienv naming_table filename

(*****************************************************************************)
(* Main entry point *)
(*****************************************************************************)

let decl_and_run_mode
    { files; extra_builtins; mode; no_builtins; tcopt; naming_table_path }
    (popt : TypecheckerOptions.t)
    (hhi_root : Path.t) : unit =
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
  let hh_files_contents =
    List.fold
      files
      ~f:(fun acc filename ->
        let hh_files_contents =
          Multifile.file_to_files filename
          |> Relative_path.Map.filter ~f:(fun _path contents ->
                 String.is_prefix contents ~prefix:"<?hh")
        in
        Relative_path.Map.union acc hh_files_contents)
      ~init:Relative_path.Map.empty
  in
  (* Merge in builtins *)
  let hh_files_contents_with_builtins =
    Relative_path.Map.fold
      builtins
      ~f:
        begin
          (fun k src acc -> Relative_path.Map.add acc ~key:k ~data:src)
        end
      ~init:hh_files_contents
  in
  Relative_path.Map.iter hh_files_contents ~f:(fun filename contents ->
      File_provider.provide_file_for_tests filename contents);
  let to_decl = hh_files_contents_with_builtins in
  let ctx =
    Provider_context.empty_for_test
      ~popt
      ~tcopt
      ~deps_mode:(Typing_deps_mode.InMemoryMode None)
  in

  (* The reverse naming table (name->filename, used for Naming_provider and Decl_provider)
     is stored (1) through ctx pointing to the backing sqlite file if desired, (2) plus
     a delta stored in a shmem heap, as per Provider_backend.

     The forward naming table (filename->FileInfo.t, used for incremental updates and also for
     fake-arrow autocomplete) is stored (1) through our [Naming_table.t] having a pointer
     to the sqlitefile if desired, (2) plus a delta stored in [Naming_table.t] ocaml data structures.

     This hh_single_complete tool is run in two modes: either with a sqlite file in which case
     sqlite should contain builtins since [to_decl] doesn't, or without a sqlite file in which
     case the delta ends up containing all provided files and all builtins. *)

  (* NAMING PHASE 1: point to the sqlite backing if desired, for both reverse and forward,
     but leave both reverse and forward deltas empty for now. *)
  let naming_table =
    match naming_table_path with
    | Some path -> Naming_table.load_from_sqlite ctx path
    | None -> Naming_table.create Relative_path.Map.empty
  in

  (* NAMING PHASE 2: for the reverse naming table delta, remove any old names from the files
     we're about to redeclare -- otherwise when we declare them it'd count as a duplicate definition! *)
  if Option.is_some naming_table_path then begin
    Relative_path.Map.iter hh_files_contents ~f:(fun file _content ->
        let file_info = Naming_table.get_file_info naming_table file in
        Option.iter file_info ~f:(fun file_info ->
            let ids_to_strings ids =
              List.map ids ~f:(fun (_, name, _) -> name)
            in
            Naming_global.remove_decls
              ~backend:(Provider_context.get_backend ctx)
              ~funs:(ids_to_strings file_info.FileInfo.funs)
              ~classes:(ids_to_strings file_info.FileInfo.classes)
              ~typedefs:(ids_to_strings file_info.FileInfo.typedefs)
              ~consts:(ids_to_strings file_info.FileInfo.consts)
              ~modules:(ids_to_strings file_info.FileInfo.modules)))
  end;

  (* NAMING PHASE 3: for the reverse naming table delta, add all new items from files we're declaring.
     Note that [to_decl] either omits or includes builtins, according to whether we're
     working from a sqlite naming table or from nothing. *)
  let (_errors, files_info_and_addenda) = parse_name_and_decl ctx to_decl in

  (* NAMING PHASE 4: for the forward naming table delta, add all new items *)
  let files_info = Relative_path.Map.map files_info_and_addenda ~f:fst in
  let naming_table =
    Naming_table.combine naming_table (Naming_table.create files_info)
  in

  (* SYMBOL INDEX PHASE 1: initialize *)
  let glean_reponame = GleanOptions.reponame tcopt in
  let namespace_map = ParserOptions.auto_namespace_map tcopt in
  let sienv =
    SymbolIndex.initialize
      ~gleanopt:tcopt
      ~namespace_map
      ~provider_name:
        (if String.is_empty glean_reponame then
          "LocalIndex"
        else
          "CustomIndex")
      ~quiet:true
  in
  let sienv =
    {
      sienv with
      SearchUtils.sie_quiet_mode = false;
      SearchUtils.sie_resolve_signatures = true;
      SearchUtils.sie_resolve_positions = true;
      SearchUtils.sie_resolve_local_decl = true;
    }
  in

  (* SYMBOL INDEX PHASE 2: update *)
  let paths_with_addenda =
    files_info_and_addenda
    |> Relative_path.Map.elements
    |> List.map ~f:(fun (path, (_fi, addenda)) ->
           (path, addenda, SearchUtils.TypeChecker))
  in
  let sienv = SymbolIndexCore.update_from_addenda ~sienv ~paths_with_addenda in

  handle_mode mode files ctx sienv naming_table

let main_hack
    ({ tcopt; _ } as opts) (root : Path.t) (sharedmem_config : SharedMem.config)
    : unit =
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
      decl_and_run_mode opts tcopt hhi_root;
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
  let (options, root, sharedmem_config) = parse_options () in
  Unix.handle_unix_error main_hack options root sharedmem_config
