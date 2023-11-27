(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ClientCommand
open ClientEnv

(** Arg specs shared across more than 1 arg parser. *)
module Common_argspecs = struct
  let config value_ref =
    ( "--config",
      Arg.String
        (fun s -> value_ref := String_utils.split2_exn '=' s :: !value_ref),
      " override arbitrary value from hh.conf and .hhconfig (format: <key>=<value>)"
    )

  let custom_hhi_path value_ref =
    ( "--custom-hhi-path",
      Arg.String (fun s -> value_ref := Some s),
      " use custom hhi files" )

  let custom_telemetry_data value_ref =
    ( "--custom-telemetry-data",
      Arg.String
        (fun s -> value_ref := String_utils.split2_exn '=' s :: !value_ref),
      "Add a custom column to all logged telemetry samples (format: <column>=<value>)"
    )

  let force_dormant_start value_ref =
    ( "--force-dormant-start",
      Arg.Bool (fun x -> value_ref := x),
      " If server is dormant, force start a new one instead of waiting for"
      ^ " the next one to start up automatically (default: false)" )

  let from value_ref =
    ( "--from",
      Arg.Set_string value_ref,
      " so we know who's calling hh_client - e.g. nuclide, vim, emacs, vscode"
      (* This setting also controls whether spinner is displayed, and whether
         clientCheckStatus.ml uses error-formatting. *) )

  let no_prechecked value_ref =
    ( "--no-prechecked",
      Arg.Unit (fun () -> value_ref := Some false),
      " override value of \"prechecked_files\" flag from hh.conf" )

  let prechecked value_ref =
    ( "--prechecked",
      Arg.Unit (fun () -> value_ref := Some true),
      " override value of \"prechecked_files\" flag from hh.conf" )

  let with_mini_state (value_ref : string option ref) =
    ( "--with-mini-state",
      Arg.String (fun s -> value_ref := Some s),
      " Init with the given saved state instead of the one based on current repo version."
    )

  let watchman_debug_logging value_ref =
    ( "--watchman-debug-logging",
      Arg.Set value_ref,
      " Logs full Watchman requests and responses. This is very noisy" )

  let allow_non_opt_build value_ref =
    ( "--allow-non-opt-build",
      Arg.Set value_ref,
      " Override build mode check triggered by warn_on_non_opt_build .hhconfig option"
    )

  let ignore_hh_version value_ref =
    ( "--ignore-hh-version",
      Arg.Set value_ref,
      " ignore hh_version check when loading saved states (default: false)" )

  let saved_state_ignore_hhconfig value_ref =
    ( "--saved-state-ignore-hhconfig",
      Arg.Set value_ref,
      " ignore hhconfig hash when loading saved states (default: false)" )

  let naming_table value_ref =
    ( "--naming-table",
      Arg.String (fun s -> value_ref := Some s),
      " use the provided naming table instead of fetching it from a saved state"
    )
end

let parse_command () =
  if Array.length Sys.argv < 2 then
    CKNone
  else
    match String.lowercase Sys.argv.(1) with
    | "check" -> CKCheck
    | "start" -> CKStart
    | "stop" -> CKStop
    | "restart" -> CKRestart
    | "lsp" -> CKLsp
    | "saved-state-project-metadata" -> CKSavedStateProjectMetadata
    | "download-saved-state" -> CKDownloadSavedState
    | "rage" -> CKRage
    | "decompress-zhhdg" -> CKDecompressZhhdg
    | _ -> CKNone

let parse_without_command options usage command =
  let args = ref [] in
  Arg.parse (Arg.align options) (fun x -> args := x :: !args) usage;
  match List.rev !args with
  | x :: rest when String.(lowercase x = lowercase command) -> rest
  | args -> args

(* *** *** NB *** *** ***
 * Commonly-used options are documented in hphp/hack/man/hh_client.1 --
 * if you are making significant changes you need to update the manpage as
 * well. Experimental or otherwise volatile options need not be documented
 * there, but keep what's there up to date please. *)
let parse_check_args cmd ~from_default =
  (* arg parse output refs *)
  let autostart = ref true in
  let config = ref [] in
  let custom_telemetry_data = ref [] in
  let custom_hhi_path = ref None in
  let error_format = ref Errors.Highlighted in
  let force_dormant_start = ref false in
  let from = ref from_default in
  let show_spinner = ref None in
  let gen_saved_ignore_type_errors = ref false in
  let ignore_hh_version = ref false in
  let save_64bit = ref None in
  let save_human_readable_64bit_dep_map = ref None in
  let saved_state_ignore_hhconfig = ref false in
  let log_inference_constraints = ref false in
  let max_errors = ref None in
  let mode = ref None in
  let logname = ref false in
  let monitor_logname = ref false in
  let client_logname = ref false in
  let ide_logname = ref false in
  let lsp_logname = ref false in
  let lock_file = ref false in
  let no_load = ref false in
  let output_json = ref false in
  let prechecked = ref None in
  let mini_state : string option ref = ref None in
  let rename_before = ref "" in
  let sort_results = ref false in
  let stdin_name = ref None in
  let timeout = ref None in
  let version = ref false in
  let watchman_debug_logging = ref false in
  let allow_non_opt_build = ref false in
  let desc = ref (ClientCommand.command_name cmd) in
  (* custom behaviors *)
  let current_option = ref None in
  let set_from x () = from := x in
  let single_files = ref [] in
  let set_mode ?(validate = true) x =
    if validate && Option.is_some !mode then
      raise (Arg.Bad "only a single mode should be specified")
    else begin
      mode := Some x;
      Option.iter !current_option ~f:(fun option -> desc := option);
      ()
    end
  in
  let add_single x = single_files := x :: !single_files in
  let set_mode_from_single_files () =
    match !single_files with
    | [] -> ()
    | single_files ->
      (match !mode with
      | Some (MODE_POPULATE_REMOTE_DECLS None) ->
        mode := Some (MODE_POPULATE_REMOTE_DECLS (Some single_files))
      | _ -> set_mode (MODE_STATUS_SINGLE single_files))
  in
  (* parse args *)
  let usage =
    match cmd with
    | CKCheck ->
      Printf.sprintf
        "Usage: %s check [OPTION]... [WWW-ROOT]\n\nWWW-ROOT is assumed to be current directory if unspecified\n"
        Sys.argv.(0)
    | CKNone ->
      Printf.sprintf
        ("Usage: %s [COMMAND] [OPTION]... [WWW-ROOT]\n\nValid values for COMMAND:\n"
        ^^ "\tcheck\t\tShows current Hack errors\n"
        ^^ "\tstart\t\tStarts a Hack server\n"
        ^^ "\tstop\t\tStops a Hack server\n"
        ^^ "\trestart\t\tRestarts a Hack server\n"
        ^^ "\tlsp\t\tRuns a persistent language service\n"
        ^^ "\trage\t\tReport a bug\n"
        ^^ "\nDefault values if unspecified:\n"
        ^^ "\tCOMMAND\t\tcheck\n"
        ^^ "\tWWW-ROOT\tCurrent directory\n\nCheck command options:\n")
        Sys.argv.(0)
    | CKSavedStateProjectMetadata ->
      Printf.sprintf
        "Usage: %s saved-state-project-metadata [OPTION]... [WWW-ROOT]\nOutput the project metadata for the current saved state\n\nWWW-ROOT is assumed to be current directory if unspecified\n"
        Sys.argv.(0)
    | CKDownloadSavedState
    | CKLsp
    | CKRage
    | CKRestart
    | CKStart
    | CKDecompressZhhdg
    | CKStop ->
      failwith "No other keywords should make it here"
  in
  let options =
    [
      (* Please keep these sorted in the alphabetical order *)
      Common_argspecs.allow_non_opt_build allow_non_opt_build;
      ( "--autostart-server",
        Arg.Bool (( := ) autostart),
        " automatically start hh_server if it's not running (default: true)" );
      Common_argspecs.config config;
      ( "--cst-search",
        Arg.Unit (fun () -> set_mode (MODE_CST_SEARCH None)),
        " (mode) Search the concrete syntax trees of files in the codebase"
        ^ " for a given pattern" );
      ( "--cst-search-files",
        Arg.Rest
          begin
            fun fn ->
              set_mode
                ~validate:false
                (match !mode with
                | None
                | Some (MODE_CST_SEARCH None) ->
                  MODE_CST_SEARCH (Some [fn])
                | Some (MODE_CST_SEARCH (Some fnl)) ->
                  MODE_CST_SEARCH (Some (fn :: fnl))
                | _ -> raise (Arg.Bad "only a single mode should be specified"))
          end,
        " Run CST search on this set of files,"
        ^ " rather than all the files in the codebase." );
      Common_argspecs.custom_hhi_path custom_hhi_path;
      Common_argspecs.custom_telemetry_data custom_telemetry_data;
      ( "--deps-out-at-pos-batch",
        Arg.Rest
          begin
            fun position ->
              set_mode
                ~validate:false
                (match !mode with
                | None -> MODE_DEPS_OUT_AT_POS_BATCH [position]
                | Some (MODE_DEPS_OUT_AT_POS_BATCH positions) ->
                  MODE_FUN_DEPS_AT_POS_BATCH (position :: positions)
                | _ -> raise (Arg.Bad "only a single mode should be specified"))
          end,
        " (mode) for each entry in input list get list of what it depends on [file:line:character list]"
      );
      ( "--deps-in-at-pos-batch",
        Arg.Rest
          begin
            fun position ->
              set_mode
                ~validate:false
                (match !mode with
                | None -> MODE_DEPS_IN_AT_POS_BATCH [position]
                | Some (MODE_DEPS_IN_AT_POS_BATCH positions) ->
                  MODE_DEPS_IN_AT_POS_BATCH (position :: positions)
                | _ -> raise (Arg.Bad "only a single mode should be specified"))
          end,
        " (mode) for each entry in input list get list of what depends on it [file:line:character list]"
      );
      ( "--dump-full-fidelity-parse",
        Arg.String (fun x -> set_mode (MODE_FULL_FIDELITY_PARSE x)),
        "" );
      ( "--dump-symbol-info",
        Arg.String (fun files -> set_mode (MODE_DUMP_SYMBOL_INFO files)),
        (*  Input format:
         *  The file list can either be "-" which accepts the input from stdin
         *  separated by newline(for long list) or directly from command line
         *  separated by semicolon.
         *  Output format:
         *    [
         *      "function_calls": list of fun_calls;
         *    ]
         *  Note: results list can be in any order *)
        "" );
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
      ( "--file-dependents",
        Arg.Unit
          (fun () ->
            let () = prechecked := Some false in
            set_mode MODE_FILE_LEVEL_DEPENDENCIES),
        " (mode) Given a list of filepaths, shows list of (possibly) dependent files"
      );
      ( "--find-class-refs",
        Arg.String (fun x -> set_mode (MODE_FIND_CLASS_REFS x)),
        " (mode) finds references of the provided class name" );
      ( "--find-refs",
        Arg.String (fun x -> set_mode (MODE_FIND_REFS x)),
        " (mode) finds references of the provided symbol; optionally specify the symbol kind like \"Kind|Symbol\" (looks for functions or methods if unspecified)"
        ^ "; valid kinds are Method, Property, Class_const, Typeconst, Function, Class, ExplicitClass, and GConst"
        ^ "; use ExplicitClass instead of Class to exclude references via self/static/parent"
      );
      Common_argspecs.force_dormant_start force_dormant_start;
      ( "--format",
        (let format_from = ref 0 in
         Arg.Tuple
           [
             Arg.Int (( := ) format_from);
             Arg.Int (fun x -> set_mode (MODE_FORMAT (!format_from, x)));
           ]),
        "" );
      Common_argspecs.from from;
      ( "--from-arc-diff",
        Arg.Unit (set_from "arc_diff"),
        " (deprecated) equivalent to --from arc_diff" );
      ( "--from-arc-land",
        Arg.Unit (set_from "arc_land"),
        " (deprecated) equivalent to --from arc_land" );
      ( "--from-check-trunk",
        Arg.Unit (set_from "check_trunk"),
        " (deprecated) equivalent to --from check_trunk" );
      ( "--from-emacs",
        Arg.Unit (set_from "emacs"),
        " (deprecated) equivalent to --from emacs" );
      ( "--from-vim",
        Arg.Unit (fun () -> from := "vim"),
        " (deprecated) equivalent to --from vim" );
      ( "--full-fidelity-schema",
        Arg.Unit (fun () -> set_mode MODE_FULL_FIDELITY_SCHEMA),
        "" );
      ( "--fun-deps-at-pos-batch",
        Arg.Rest
          begin
            fun position ->
              set_mode
                ~validate:false
                (match !mode with
                | None -> MODE_FUN_DEPS_AT_POS_BATCH [position]
                | Some (MODE_FUN_DEPS_AT_POS_BATCH positions) ->
                  MODE_FUN_DEPS_AT_POS_BATCH (position :: positions)
                | _ -> raise (Arg.Bad "only a single mode should be specified"))
          end,
        " (mode) for each entry in input list get list of function dependencies [file:line:character list]"
      );
      ( "--gen-prefetch-dir",
        Arg.String (fun _x -> set_mode (MODE_POPULATE_REMOTE_DECLS None)),
        " Compute all decls for the repo and upload them to the remote decl service."
        ^ " Usage: --gen-prefetch-dir unused" );
      ( "--populate-remote-decls",
        Arg.Unit (fun () -> set_mode (MODE_POPULATE_REMOTE_DECLS None)),
        " Compute all decls for the repo and upload them to the remote decl service."
      );
      ( "--gen-saved-ignore-type-errors",
        Arg.Set gen_saved_ignore_type_errors,
        " generate a saved state even if there are type errors (default: false)."
      );
      ( "--get-method-name",
        Arg.String (fun x -> set_mode (MODE_IDENTIFY_SYMBOL3 x)),
        (* alias for --identify-function *) "" );
      ( "--go-to-impl-class",
        Arg.String (fun x -> set_mode (MODE_GO_TO_IMPL_CLASS x)),
        " (mode) goes to implementation of the provided class/trait/interface/etc. with the given name"
      );
      ( "--go-to-impl-method",
        Arg.String (fun x -> set_mode (MODE_GO_TO_IMPL_METHOD x)),
        " (mode) goes to implementation of the provided method name" );
      ( "--ide-find-refs-by-symbol",
        Arg.String
          (fun x ->
            set_mode
              (MODE_IDE_FIND_REFS_BY_SYMBOL
                 (FindRefsWireFormat.CliArgs.from_string_exn x))),
        "(mode) similar to IDE_FIND_REFS, but takes a symbol name rather than position"
      );
      ( "--ide-find-refs-by-symbol3",
        (let action = ref "" in
         let stream_file = ref "-" in
         Arg.Tuple
           [
             Arg.String (fun s -> action := s);
             Arg.String (fun s -> stream_file := s);
             Arg.String
               (fun hints ->
                 set_mode
                   (MODE_IDE_FIND_REFS_BY_SYMBOL
                      (FindRefsWireFormat.CliArgs.from_string_triple_exn
                         (!action, !stream_file, hints))));
           ]),
        "(mode) similar to FIND_REFS, but takes [action stream_file hints]" );
      ( "--ide-go-to-impl-by-symbol",
        Arg.String
          (fun x ->
            set_mode
              (MODE_IDE_GO_TO_IMPL_BY_SYMBOL
                 (FindRefsWireFormat.CliArgs.from_string_exn x))),
        "(mode) similar to IDE_GO_TO_IMPL, but takes a symbol name rather than position"
      );
      ( "--ide-get-definition",
        Arg.String (fun x -> set_mode (MODE_IDENTIFY_SYMBOL2 x)),
        (* alias for --identify-function *) "" );
      ("--ide-outline", Arg.Unit (fun () -> set_mode MODE_OUTLINE2), "");
      ( "--ide-rename-by-symbol",
        Arg.String (fun x -> set_mode (MODE_IDE_RENAME_BY_SYMBOL x)),
        " (mode) renames, but takes a Find_refs.action. Usage: "
        ^ " <new name>|<comma_separated_action>" );
      ( "--identify-function",
        Arg.String (fun x -> set_mode (MODE_IDENTIFY_SYMBOL1 x)),
        " (mode) print the full function name at the position "
        ^ "[line:character] of the text on stdin" );
      ( "--identify",
        Arg.String (fun x -> set_mode (MODE_IDENTIFY_SYMBOL x)),
        " (mode) identify the named symbol" );
      Common_argspecs.ignore_hh_version ignore_hh_version;
      ( "--in-memory-dep-table-size",
        Arg.Unit (fun () -> set_mode MODE_IN_MEMORY_DEP_TABLE_SIZE),
        " number of entries in the in-memory dependency table" );
      ( "--inheritance-ancestor-classes",
        Arg.String (fun x -> set_mode (MODE_METHOD_JUMP_ANCESTORS (x, "Class"))),
        " (mode) prints a list of classes that this class extends" );
      ( "--inheritance-ancestor-classes-batch",
        Arg.Rest
          begin
            fun class_ ->
              set_mode
                ~validate:false
                (match !mode with
                | None -> MODE_METHOD_JUMP_ANCESTORS_BATCH ([class_], "Class")
                | Some (MODE_METHOD_JUMP_ANCESTORS_BATCH (classes, "Class")) ->
                  MODE_METHOD_JUMP_ANCESTORS_BATCH (class_ :: classes, "Class")
                | _ -> raise (Arg.Bad "only a single mode should be specified"))
          end,
        " (mode) prints a list of classes that these classes extend" );
      ( "--inheritance-ancestor-interfaces",
        Arg.String
          (fun x -> set_mode (MODE_METHOD_JUMP_ANCESTORS (x, "Interface"))),
        " (mode) prints a list of interfaces that this class implements" );
      ( "--inheritance-ancestor-interfaces-batch",
        Arg.Rest
          begin
            fun class_ ->
              set_mode
                ~validate:false
                (match !mode with
                | None ->
                  MODE_METHOD_JUMP_ANCESTORS_BATCH ([class_], "Interface")
                | Some (MODE_METHOD_JUMP_ANCESTORS_BATCH (classes, "Interface"))
                  ->
                  MODE_METHOD_JUMP_ANCESTORS_BATCH
                    (class_ :: classes, "Interface")
                | _ -> raise (Arg.Bad "only a single mode should be specified"))
          end,
        " (mode) prints a list of interfaces that these classes implement" );
      ( "--inheritance-ancestor-traits",
        Arg.String (fun x -> set_mode (MODE_METHOD_JUMP_ANCESTORS (x, "Trait"))),
        " (mode) prints a list of traits that this class uses" );
      ( "--inheritance-ancestor-traits-batch",
        Arg.Rest
          begin
            fun class_ ->
              set_mode
                ~validate:false
                (match !mode with
                | None -> MODE_METHOD_JUMP_ANCESTORS_BATCH ([class_], "Trait")
                | Some (MODE_METHOD_JUMP_ANCESTORS_BATCH (classes, "Trait")) ->
                  MODE_METHOD_JUMP_ANCESTORS_BATCH (class_ :: classes, "Trait")
                | _ -> raise (Arg.Bad "only a single mode should be specified"))
          end,
        " (mode) prints a list of traits that these classes use" );
      ( "--inheritance-ancestors",
        Arg.String
          (fun x -> set_mode (MODE_METHOD_JUMP_ANCESTORS (x, "No_filter"))),
        " (mode) prints a list of all related classes or methods"
        ^ " to the given class" );
      ( "--inheritance-children",
        Arg.String (fun x -> set_mode (MODE_METHOD_JUMP_CHILDREN x)),
        " (mode) prints a list of all related classes or methods"
        ^ " to the given class" );
      ( "--json",
        Arg.Set output_json,
        " output json for machine consumption. (default: false)" );
      ( "--lint",
        Arg.Unit (fun () -> set_mode MODE_LINT),
        " (mode) lint the given list of files" );
      ( "--lint-all",
        Arg.Int (fun x -> set_mode (MODE_LINT_ALL x)),
        " (mode) find all occurrences of lint with the given error code" );
      ( "--lint-stdin",
        Arg.String (fun filename -> set_mode (MODE_LINT_STDIN filename)),
        " (mode) lint a file given on stdin; the filename should be the"
        ^ " argument to this option" );
      ( "--list-files",
        Arg.Unit (fun () -> set_mode MODE_LIST_FILES),
        " (mode) list files with errors" );
      ("--lock-file", Arg.Set lock_file, " (mode) show lock file name and exit");
      ( "--log-inference-constraints",
        Arg.Set log_inference_constraints,
        "  (for hh debugging purpose only) log type"
        ^ " inference constraints into external logger (e.g. Scuba)" );
      ( "--max-errors",
        Arg.Int (fun num_errors -> max_errors := Some num_errors),
        " Maximum number of errors to display" );
      ("--logname", Arg.Set logname, " (mode) show log filename and exit");
      ( "--monitor-logname",
        Arg.Set monitor_logname,
        " (mode) show monitor log filename and exit" );
      ( "--client-logname",
        Arg.Set client_logname,
        " (mode) show client log filename and exit" );
      ( "--ide-logname",
        Arg.Set ide_logname,
        " (mode) show client ide log filename and exit" );
      ( "--lsp-logname",
        Arg.Set lsp_logname,
        " (mode) show client lsp log filename and exit" );
      ("--no-load", Arg.Set no_load, " start from a fresh state");
      ( "--outline",
        Arg.Unit (fun () -> set_mode MODE_OUTLINE),
        " (mode) prints an outline of the text on stdin" );
      Common_argspecs.prechecked prechecked;
      Common_argspecs.no_prechecked prechecked;
      Common_argspecs.with_mini_state mini_state;
      ( "--profile-log",
        Arg.Unit (fun () -> config := ("profile_log", "true") :: !config),
        " enable profile logging" );
      ( "--refactor",
        (let rename_mode = ref Unspecified in
         Arg.Tuple
           [
             Arg.Symbol
               ( ["Class"; "Function"; "Method"],
                 (fun x -> rename_mode := string_to_rename_mode x) );
             Arg.String (fun x -> rename_before := x);
             Arg.String
               (fun x ->
                 set_mode (MODE_RENAME (!rename_mode, !rename_before, x)));
           ]),
        " (mode) rename a symbol, Usage: --refactor "
        ^ "[\"Class\", \"Function\", \"Method\"] <Current Name> <New Name>" );
      ( "--remove-dead-fixme",
        Arg.Int
          begin
            fun code ->
              set_mode
                ~validate:false
                (match !mode with
                | None -> MODE_REMOVE_DEAD_FIXMES [code]
                | Some (MODE_REMOVE_DEAD_FIXMES codel) ->
                  MODE_REMOVE_DEAD_FIXMES (code :: codel)
                | _ -> raise (Arg.Bad "only a single mode should be specified"))
          end,
        " (mode) remove dead HH_FIXME for specified error code "
        ^ "(first do hh_client restart --no-load)" );
      ( "--remove-dead-fixmes",
        Arg.Unit (fun () -> set_mode (MODE_REMOVE_DEAD_FIXMES [])),
        " (mode) remove dead HH_FIXME for any error code < 5000 "
        ^ "(first do hh_client restart --no-load)" );
      ( "--remove-dead-unsafe-casts",
        Arg.Unit (fun () -> set_mode MODE_REMOVE_DEAD_UNSAFE_CASTS),
        " (mode) remove dead UNSAFE_CASTS (first do hh_client restart --no-load)"
      );
      ( "--retries",
        Arg.Int (fun n -> timeout := Some (float_of_int (max 5 n))),
        " (deprecated) same as --timeout" );
      ("--retry-if-init", Arg.Bool (fun _ -> ()), " (deprecated and ignored)");
      ( "--rewrite-declarations",
        Arg.Unit (fun () -> set_mode MODE_REWRITE_DECLARATIONS),
        {|Rewrite Hack source code allowed in notebooks to valid Hack.
For example in `function foo(): void {} function foo(): void{}` we
rewrite to the function names to something like `foo_1` and `foo_2`.
|}
      );
      ( "--rewrite-lambda-parameters",
        Arg.Rest
          begin
            fun fn ->
              set_mode
                ~validate:false
                (match !mode with
                | None -> MODE_REWRITE_LAMBDA_PARAMETERS [fn]
                | Some (MODE_REWRITE_LAMBDA_PARAMETERS fnl) ->
                  MODE_REWRITE_LAMBDA_PARAMETERS (fn :: fnl)
                | _ -> raise (Arg.Bad "only a single mode should be specified"))
          end,
        " (mode) rewrite lambdas in the files from the given list"
        ^ " with suggested parameter types" );
      ( "--save-naming",
        Arg.String (fun x -> set_mode (MODE_SAVE_NAMING x)),
        " (mode) Save the naming table to the given file."
        ^ " Returns the number of files and symbols written to disk." );
      ( "--save-state",
        Arg.String (fun x -> set_mode (MODE_SAVE_STATE x)),
        " (mode) Save a saved state to the given file."
        ^ " Returns number of edges dumped from memory to the database." );
      ( "--save-64bit",
        Arg.String (fun x -> save_64bit := Some x),
        " save discovered 64-bit to the given directory" );
      ( "--save-human-readable-64bit-dep-map",
        Arg.String (fun x -> save_human_readable_64bit_dep_map := Some x),
        " save map of 64bit hashes to names to files in the given directory" );
      Common_argspecs.saved_state_ignore_hhconfig saved_state_ignore_hhconfig;
      ( "--search",
        Arg.String (fun x -> set_mode (MODE_SEARCH x)),
        " (mode) --search this_is_just_to_check_liveness_of_hh_server" );
      ( "--server-rage",
        Arg.Unit (fun () -> set_mode MODE_SERVER_RAGE),
        " (mode) dumps internal state of hh_server" );
      ( "--show-spinner",
        Arg.Bool (fun x -> show_spinner := Some x),
        " shows a spinner while awaiting the typechecker" );
      ( "--single",
        Arg.String add_single,
        "<path> Return errors in file with provided name (give '-' for stdin)"
      );
      ("--sort-results", Arg.Set sort_results, " sort output for CST search.");
      ( "--stats",
        Arg.Unit (fun () -> set_mode MODE_STATS),
        " display some server statistics" );
      ( "--stdin-name",
        Arg.String (fun x -> stdin_name := Some x),
        " substitute stdin for contents of file with specified name" );
      ( "--status",
        Arg.Unit (fun () -> set_mode MODE_STATUS),
        " (mode) show a human readable list of errors (default)" );
      ( "--timeout",
        Arg.Float (fun x -> timeout := Some (Float.max 5. x)),
        " set the timeout in seconds (default: no timeout)" );
      ( "--type-at-pos",
        Arg.String (fun x -> set_mode (MODE_TYPE_AT_POS x)),
        " (mode) show type at a given position in file [file:line:character]" );
      ( "--type-at-pos-batch",
        Arg.Rest
          begin
            fun position ->
              set_mode
                ~validate:false
                (match !mode with
                | None -> MODE_TYPE_AT_POS_BATCH [position]
                | Some (MODE_TYPE_AT_POS_BATCH positions) ->
                  MODE_TYPE_AT_POS_BATCH (position :: positions)
                | _ -> raise (Arg.Bad "only a single mode should be specified"))
          end,
        " (mode) show types at multiple positions [file:line:character list]" );
      ( "--type-error-at-pos",
        Arg.String (fun x -> set_mode (MODE_TYPE_ERROR_AT_POS x)),
        " (mode) show type error at a given position in file [line:character]"
      );
      ( "--is-subtype",
        Arg.Unit (fun () -> set_mode MODE_IS_SUBTYPE),
        " (mode) take a JSON list of subtype queries via stdin" );
      ( "--tast-holes",
        Arg.String (fun x -> set_mode (MODE_TAST_HOLES x)),
        " (mode) return all TAST Holes in a given file" );
      ( "--tast-holes-batch",
        Arg.String (fun x -> set_mode (MODE_TAST_HOLES_BATCH x)),
        " (mode) return all TAST Holes for a set of files. Argument is a file containing a newline-separated list of files"
      );
      ( "--verbose-on",
        Arg.Unit (fun () -> set_mode (MODE_VERBOSE true)),
        " (mode) turn on verbose server log" );
      ( "--verbose-off",
        Arg.Unit (fun () -> set_mode (MODE_VERBOSE false)),
        " (mode) turn off verbose server log" );
      ("--version", Arg.Set version, " (mode) show version and exit");
      Common_argspecs.watchman_debug_logging watchman_debug_logging;
      ( "--xhp-autocomplete-snippet",
        Arg.String (fun x -> set_mode (MODE_XHP_AUTOCOMPLETE_SNIPPET x)),
        "(mode) Look up for XHP component and return its snippet" );
      (* Please keep these sorted in the alphabetical order *)
    ]
  in
  (* If the user typed an option like "--type-at-pos" which set a mode which triggered
     a command, we want to be able to show "hh_server is busy [--type-at-pos]" to show that
     hh_server is currently busy with something that the user typed. The string
     description that appears inside the square brackets must go into args.desc.
     Unfortunately this isn't well supported by the Arg library, so we have to hack it up
     ourselves: (1) For any option that takes say Arg.Unit(callback), we'll change it into
     Arg.Unit(modified_callback) where modified_callback sets 'current_option' to the
     option string being handled and then calls the original callback. (2) If the original
     callback calls set_mode, then set_mode will take the opportunity to do desc := current_option.
  *)
  let modify_callback : type a. string -> (a -> unit) -> a -> unit =
   fun option callback value ->
    current_option := Some option;
    callback value;
    current_option := None
  in
  let rec modify_spec ~option spec =
    match spec with
    | Arg.Unit callback -> Arg.Unit (modify_callback option callback)
    | Arg.Bool callback -> Arg.Bool (modify_callback option callback)
    | Arg.String callback -> Arg.String (modify_callback option callback)
    | Arg.Int callback -> Arg.Int (modify_callback option callback)
    | Arg.Float callback -> Arg.Float (modify_callback option callback)
    | Arg.Rest callback -> Arg.Rest (modify_callback option callback)
    | Arg.Tuple specs -> Arg.Tuple (List.map specs ~f:(modify_spec ~option))
    | spec -> spec
  in
  let options =
    List.map options ~f:(fun (option, spec, text) ->
        (option, modify_spec ~option spec, text))
  in
  let args =
    parse_without_command options usage (ClientCommand.command_name cmd)
  in
  if !version then (
    if !output_json then
      ServerArgs.print_json_version ()
    else
      print_endline Hh_version.version;
    exit 0
  );

  set_mode_from_single_files ();
  let mode = Option.value !mode ~default:MODE_STATUS in
  (* fixups *)
  let (root, paths) =
    match (mode, args) with
    | (MODE_LINT, _)
    | (MODE_FILE_LEVEL_DEPENDENCIES, _) ->
      (Wwwroot.interpret_command_line_root_parameter [], args)
    | (_, _) -> (Wwwroot.interpret_command_line_root_parameter args, [])
  in

  if !lock_file then (
    let lock_file_link = ServerFiles.lock_file root in
    Printf.printf "%s\n%!" lock_file_link;
    exit 0
  );

  if !ide_logname then (
    let ide_log_link = ServerFiles.client_ide_log root in
    Printf.printf "%s\n%!" ide_log_link;
    exit 0
  );

  if !lsp_logname then (
    let lsp_log_link = ServerFiles.client_lsp_log root in
    Printf.printf "%s\n%!" lsp_log_link;
    exit 0
  );

  if !monitor_logname then (
    let monitor_log_link = ServerFiles.monitor_log_link root in
    Printf.printf "%s\n%!" monitor_log_link;
    exit 0
  );

  if !client_logname then (
    let client_log_link = ServerFiles.client_log root in
    Printf.printf "%s\n%!" client_log_link;
    exit 0
  );

  if !logname then (
    let log_link = ServerFiles.log_link root in
    Printf.printf "%s\n%!" log_link;
    exit 0
  );

  if String.equal !from "emacs" then
    Printf.fprintf stdout "-*- mode: compilation -*-\n%!";
  {
    autostart = !autostart;
    config = !config;
    custom_hhi_path = !custom_hhi_path;
    custom_telemetry_data = !custom_telemetry_data;
    error_format = !error_format;
    force_dormant_start = !force_dormant_start;
    from = !from;
    show_spinner =
      Option.value
        ~default:(String.is_empty !from || String.equal !from "[sh]")
        !show_spinner;
    gen_saved_ignore_type_errors = !gen_saved_ignore_type_errors;
    ignore_hh_version = !ignore_hh_version;
    saved_state_ignore_hhconfig = !saved_state_ignore_hhconfig;
    paths;
    log_inference_constraints = !log_inference_constraints;
    max_errors = !max_errors;
    mode;
    no_load =
      (!no_load
      ||
      match mode with
      | MODE_REMOVE_DEAD_FIXMES _ -> true
      | _ -> false);
    save_64bit = !save_64bit;
    save_human_readable_64bit_dep_map = !save_human_readable_64bit_dep_map;
    output_json = !output_json;
    prechecked = !prechecked;
    mini_state = !mini_state;
    root;
    sort_results = !sort_results;
    stdin_name = !stdin_name;
    deadline = Option.map ~f:(fun t -> Unix.time () +. t) !timeout;
    watchman_debug_logging = !watchman_debug_logging;
    allow_non_opt_build = !allow_non_opt_build;
    desc = !desc;
  }

let parse_start_env command ~from_default =
  let usage =
    Printf.sprintf
      "Usage: %s %s [OPTION]... [WWW-ROOT]\n%s a Hack server\n\nWWW-ROOT is assumed to be current directory if unspecified\n"
      Sys.argv.(0)
      command
      (String.capitalize command)
  in
  let log_inference_constraints = ref false in
  let no_load = ref false in
  let watchman_debug_logging = ref false in
  let ignore_hh_version = ref false in
  let saved_state_ignore_hhconfig = ref false in
  let prechecked = ref None in
  let mini_state = ref None in
  let from = ref from_default in
  let config = ref [] in
  let custom_hhi_path = ref None in
  let custom_telemetry_data = ref [] in
  let allow_non_opt_build = ref false in
  let wait_deprecation_msg () =
    Printf.eprintf
      "WARNING: --wait is deprecated, does nothing, and will be going away soon!\n%!"
  in
  let options =
    [
      (* Please keep these sorted in the alphabetical order *)
      Common_argspecs.allow_non_opt_build allow_non_opt_build;
      Common_argspecs.config config;
      Common_argspecs.custom_hhi_path custom_hhi_path;
      Common_argspecs.custom_telemetry_data custom_telemetry_data;
      Common_argspecs.from from;
      Common_argspecs.ignore_hh_version ignore_hh_version;
      ( "--log-inference-constraints",
        Arg.Set log_inference_constraints,
        " (for hh debugging purpose only) log type"
        ^ " inference constraints into external logger (e.g. Scuba)" );
      ("--no-load", Arg.Set no_load, " start from a fresh state");
      Common_argspecs.no_prechecked prechecked;
      Common_argspecs.prechecked prechecked;
      Common_argspecs.with_mini_state mini_state;
      ( "--profile-log",
        Arg.Unit (fun () -> config := ("profile_log", "true") :: !config),
        " enable profile logging" );
      Common_argspecs.saved_state_ignore_hhconfig saved_state_ignore_hhconfig;
      ( "--wait",
        Arg.Unit wait_deprecation_msg,
        " this flag is deprecated and does nothing!" );
      Common_argspecs.watchman_debug_logging watchman_debug_logging;
      (* Please keep these sorted in the alphabetical order *)
    ]
  in
  let args = parse_without_command options usage command in
  let root = Wwwroot.interpret_command_line_root_parameter args in
  {
    ClientStart.config = !config;
    custom_hhi_path = !custom_hhi_path;
    custom_telemetry_data = !custom_telemetry_data;
    exit_on_failure = true;
    from = !from;
    ignore_hh_version = !ignore_hh_version;
    saved_state_ignore_hhconfig = !saved_state_ignore_hhconfig;
    save_64bit = None;
    save_human_readable_64bit_dep_map = None;
    log_inference_constraints = !log_inference_constraints;
    no_load = !no_load;
    prechecked = !prechecked;
    mini_state = !mini_state;
    root;
    silent = false;
    watchman_debug_logging = !watchman_debug_logging;
    allow_non_opt_build = !allow_non_opt_build;
  }

let parse_saved_state_project_metadata_args ~from_default : command =
  CSavedStateProjectMetadata
    (parse_check_args CKSavedStateProjectMetadata ~from_default)

let parse_start_args ~from_default =
  CStart (parse_start_env "start" ~from_default)

let parse_restart_args ~from_default =
  CRestart (parse_start_env "restart" ~from_default)

let parse_stop_args ~from_default =
  let usage =
    Printf.sprintf
      "Usage: %s stop [OPTION]... [WWW-ROOT]\nStop a hack server\n\nWWW-ROOT is assumed to be current directory if unspecified\n"
      Sys.argv.(0)
  in
  let from = ref from_default in
  let options = [Common_argspecs.from from] in
  let args = parse_without_command options usage "stop" in
  let root = Wwwroot.interpret_command_line_root_parameter args in
  CStop { ClientStop.root; from = !from }

let parse_lsp_args () =
  let usage =
    Printf.sprintf
      "Usage: %s lsp [OPTION]...\nRuns a persistent language service\n"
      Sys.argv.(0)
  in
  let from = ref "" in
  let config = ref [] in
  let verbose = ref false in
  let ignore_hh_version = ref false in
  let naming_table = ref None in
  let options =
    [
      Common_argspecs.from from;
      Common_argspecs.config config;
      (* Please keep these sorted in the alphabetical order *)
      ("--enhanced-hover", Arg.Unit (fun () -> ()), " [legacy] no-op");
      ("--ffp-autocomplete", Arg.Unit (fun () -> ()), " [legacy] no-op");
      Common_argspecs.ignore_hh_version ignore_hh_version;
      Common_argspecs.naming_table naming_table;
      ("--ranked-autocomplete", Arg.Unit (fun () -> ()), " [legacy] no-op");
      ("--serverless-ide", Arg.Unit (fun () -> ()), " [legacy] no-op");
      ( "--verbose",
        Arg.Set verbose,
        " verbose logs to stderr and `hh --ide-logname` and `--lsp-logname`" );
      (* Please keep these sorted in the alphabetical order *)
    ]
  in
  let args = parse_without_command options usage "lsp" in
  let root = Wwwroot.interpret_command_line_root_parameter args in
  CLsp
    {
      ClientLsp.from = !from;
      config = !config;
      ignore_hh_version = !ignore_hh_version;
      naming_table = !naming_table;
      verbose = !verbose;
      root_from_cli = root;
    }

let parse_rage_args () =
  let usage =
    Printf.sprintf "Usage: %s rage [OPTION]... [WWW-ROOT]\n" Sys.argv.(0)
  in
  let from = ref "" in
  let desc = ref None in
  let rageid = ref None in
  let lsp_log = ref None in
  let options =
    [
      Common_argspecs.from from;
      ("--desc", Arg.String (fun s -> desc := Some s), " description of problem");
      ( "--rageid",
        Arg.String (fun s -> rageid := Some s),
        " (optional) use this id, and finish even if parent process dies" );
      ( "--lsp-log",
        Arg.String (fun s -> lsp_log := Some s),
        " (optional) gather lsp logs from this filename" );
    ]
  in
  let args = parse_without_command options usage "rage" in
  let root = Wwwroot.interpret_command_line_root_parameter args in
  (* hh_client normally handles Ctrl+C by printing an exception-stack.
     But for us, in an interactive prompt, Ctrl+C is an unexceptional way to quit. *)
  Sys_utils.set_signal Sys.sigint Sys.Signal_default;

  let desc =
    match !desc with
    | Some desc -> desc
    | None ->
      Printf.printf
        ("Sorry that hh isn't working. What's wrong?\n"
        ^^ "0. There's something wrong relating to VSCode or IDE\n"
        ^^ "1. hh_server takes ages to initialize\n"
        ^^ "2. hh is stuck in an infinite loop\n"
        ^^ "3. hh gives some error message about the monitor\n"
        ^^ "4. hack says it has an internal typecheck bug and asked me to report it\n"
        ^^ "5. hack is reporting errors that are clearly incorrect [please elaborate]\n"
        ^^ "6. I'm not sure how to write my code to avoid these hack errors\n"
        ^^ "7. hh says something about unsaved changes from an editor even after I've quit my editor\n"
        ^^ "8. something's wrong with hack VS Code or other editor\n"
        ^^ "[other] Please type either one of the above numbers, or a freeform description\n"
        ^^ "\nrage> %!");
      let response = In_channel.input_line_exn In_channel.stdin in
      let (response, info) =
        if String.equal response "0" then
          let () =
            Printf.printf
              "Please use the VSCode bug nub (at the right of the status bar) instead of hh rage.\n"
          in
          exit 0
        else if String.equal response "1" then
          ("hh_server slow initialize", `Verbose_hh_start)
        else if String.equal response "2" then
          ("hh stuck in infinite loop", `Verbose_hh_start)
        else if String.equal response "3" then
          ("hh monitor problem", `Verbose_hh_start)
        else if String.equal response "4" then begin
          ClientRage.verify_typechecker_err_src ();
          ("internal typecheck bug", `No_info)
        end else if String.equal response "5" then
          let () =
            Printf.printf
              "Please elaborate on which errors are incorrect...\nrage> %!"
          in
          (In_channel.input_line_exn In_channel.stdin, `No_info)
        else if String.equal response "6" then
          let () =
            Printf.printf
              ("Please ask in the appropriate support groups for advice on coding in Hack; "
              ^^ "`hh rage` is solely for reporting bugs in the tooling, not for reporting typechecker or "
              ^^ "language issues.\n")
          in
          exit 0
        else if String.equal response "7" then
          ("unsaved editor changes", `Unsaved)
        else if String.equal response "8" then
          let () =
            Printf.printf
              ("Please file the bug from within your editor to capture the right logs. "
              ^^ "Note: you can do Preferences > Settings > Hack > Verbose, then `pkill hh_client`, "
              ^^ "then reproduce the error, then file the bug. This way we'll get even richer logs.\n"
              )
          in
          exit 0
        else
          (response, `Verbose_hh_start)
      in
      begin
        match info with
        | `No_info -> ()
        | `Verbose_hh_start ->
          Printf.printf
            ("\nPOWER USERS ONLY: Sometimes the normal logging from hh_server isn't "
            ^^ "enough to diagnose an issue, and we'll ask you to switch hh_server to "
            ^^ "write verbose logs, then have you repro the issue, then use `hh rage` to "
            ^^ "gather up those now-verbose logs. To restart hh_server with verbose logs, "
            ^^ "do `hh stop && hh start --config min_log_level=debug`. "
            ^^ "Once done, then you can repro the issue, and then do rage again.\n\n%!"
            )
        | `Unsaved ->
          Printf.printf
            "\nNote: you can often work around this issue yourself by quitting your editor, then `pkill hh_client`.\n%!"
      end;
      response
  in
  CRage
    {
      ClientRage.root;
      from = !from;
      desc;
      rageid = !rageid;
      lsp_log = !lsp_log;
    }

let parse_decompress_zhhdg_args () =
  let usage =
    Printf.sprintf
      {|Usage: %s decompress-zhhdg --path [PATH]

Decompress a .zhhdg file by running the depgraph decompressor, and write a .hhdg file in the same directory.
|}
      Sys.argv.(0)
  in
  let from = ref "" in
  let path = ref "" in
  let options =
    Arg.align
      [
        Common_argspecs.from from;
        ( "--path",
          Arg.String (fun arg -> path := arg),
          " The path on disk to the .zhhdg file" );
      ]
  in
  let _args = parse_without_command options usage "decompress-zhhdg" in
  let path =
    match !path with
    | "" ->
      print_endline "The '--path' option is required.";
      exit 2
    | p -> p
  in
  let from = !from in
  CDecompressZhhdg { ClientDecompressZhhdg.path; from }

let parse_download_saved_state_args () =
  let usage =
    Printf.sprintf
      {|Usage: %s download-saved-state [OPTION]... [WWW-ROOT]

Download a saved-state to disk for the given repository, to make future
invocations of `hh` faster.|}
      Sys.argv.(0)
  in

  let from = ref "" in
  let should_save_replay = ref false in
  let replay_token = ref None in
  let saved_state_manifold_api_key = ref None in
  let options =
    Arg.align
      [
        Common_argspecs.from from;
        ("--type", Arg.String (fun _ -> ()), " (unused)");
        ( "--save-replay",
          Arg.Set should_save_replay,
          " Produce a token that can be later consumed by --replay-token to replay the same saved-state download."
        );
        ( "--saved-state-manifold-api-key",
          Arg.String (fun arg -> saved_state_manifold_api_key := Some arg),
          " An API key for Manifold to use when downloading the specified saved state."
        );
        ( "--replay-token",
          Arg.String (fun arg -> replay_token := Some arg),
          " A token produced from a previous invocation of this command with --save-replay."
        );
      ]
  in
  let args = parse_without_command options usage "download-saved-state" in
  let root =
    match args with
    | [] ->
      print_endline usage;
      exit 2
    | _ -> Wwwroot.interpret_command_line_root_parameter args
  in
  let from =
    match !from with
    | "" ->
      print_endline "The '--from' option is required.";
      exit 2
    | from -> from
  in
  CDownloadSavedState
    {
      ClientDownloadSavedState.root;
      from;
      saved_state_manifold_api_key = !saved_state_manifold_api_key;
      should_save_replay = !should_save_replay;
      replay_token = !replay_token;
    }

let parse_args ~(from_default : string) : command =
  match parse_command () with
  | CKNone
  | CKCheck ->
    CCheck (parse_check_args CKCheck ~from_default)
  | CKStart -> parse_start_args ~from_default
  | CKStop -> parse_stop_args ~from_default
  | CKRestart -> parse_restart_args ~from_default
  | CKLsp -> parse_lsp_args ()
  | CKRage -> parse_rage_args ()
  | CKSavedStateProjectMetadata ->
    parse_saved_state_project_metadata_args ~from_default
  | CKDownloadSavedState -> parse_download_saved_state_args ()
  | CKDecompressZhhdg -> parse_decompress_zhhdg_args ()

let root = function
  | CCheck { ClientEnv.root; _ }
  | CStart { ClientStart.root; _ }
  | CRestart { ClientStart.root; _ }
  | CStop { ClientStop.root; _ }
  | CRage { ClientRage.root; _ }
  | CSavedStateProjectMetadata { ClientEnv.root; _ }
  | CDownloadSavedState { ClientDownloadSavedState.root; _ } ->
    Some root
  | CLsp { ClientLsp.root_from_cli; _ } -> Some root_from_cli
  | CDecompressZhhdg _ -> None

let config = function
  | CCheck { ClientEnv.config; _ }
  | CStart { ClientStart.config; _ }
  | CRestart { ClientStart.config; _ }
  | CLsp { ClientLsp.config; _ }
  | CSavedStateProjectMetadata { ClientEnv.config; _ } ->
    Some config
  | CStop _
  | CDownloadSavedState _
  | CRage _
  | CDecompressZhhdg _ ->
    None

let from = function
  | CCheck { ClientEnv.from; _ }
  | CStart { ClientStart.from; _ }
  | CRestart { ClientStart.from; _ }
  | CLsp { ClientLsp.from; _ }
  | CSavedStateProjectMetadata { ClientEnv.from; _ }
  | CStop { ClientStop.from; _ }
  | CDownloadSavedState { ClientDownloadSavedState.from; _ }
  | CRage { ClientRage.from; _ }
  | CDecompressZhhdg { ClientDecompressZhhdg.from; _ } ->
    from
