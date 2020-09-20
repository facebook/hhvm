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
    )

  let no_prechecked value_ref =
    ( "--no-prechecked",
      Arg.Unit (fun () -> value_ref := Some false),
      " override value of \"prechecked_files\" flag from hh.conf" )

  let prechecked value_ref =
    ( "--prechecked",
      Arg.Unit (fun () -> value_ref := Some true),
      " override value of \"prechecked_files\" flag from hh.conf" )

  let watchman_debug_logging value_ref =
    ( "--watchman-debug-logging",
      Arg.Set value_ref,
      " Enable debug logging on Watchman client. This is very noisy" )

  let allow_non_opt_build value_ref =
    ( "--allow-non-opt-build",
      Arg.Set value_ref,
      " Override build mode check triggered by warn_on_non_opt_build .hhconfig option"
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
    | "debug" -> CKDebug
    | "download-saved-state" -> CKDownloadSavedState
    | "rage" -> CKRage
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
let parse_check_args cmd =
  (* arg parse output refs *)
  let ai_mode = ref None in
  let autostart = ref true in
  let config = ref [] in
  let custom_telemetry_data = ref [] in
  let dynamic_view = ref false in
  let error_format = ref Errors.Highlighted in
  let force_dormant_start = ref false in
  let format_from = ref 0 in
  let from = ref "" in
  let show_spinner = ref None in
  let hot_classes_threshold = ref 0 in
  let gen_saved_ignore_type_errors = ref false in
  let ignore_hh_version = ref false in
  let saved_state_ignore_hhconfig = ref false in
  let log_inference_constraints = ref false in
  let max_errors = ref None in
  let mode = ref None in
  let logname = ref false in
  let monitor_logname = ref false in
  let client_logname = ref false in
  let ide_logname = ref false in
  let lsp_logname = ref false in
  let no_load = ref false in
  let output_json = ref false in
  let prechecked = ref None in
  let profile_log = ref false in
  let refactor_before = ref "" in
  let refactor_mode = ref "" in
  let remote = ref false in
  let replace_state_after_saving = ref false in
  let sort_results = ref false in
  let stdin_name = ref None in
  let timeout = ref None in
  let version = ref false in
  let watchman_debug_logging = ref false in
  let allow_non_opt_build = ref false in
  (* custom behaviors *)
  let set_from x () = from := x in
  let set_mode x () =
    if Option.is_some !mode then
      raise (Arg.Bad "only a single mode should be specified")
    else
      mode := Some x
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
        ( "Usage: %s [COMMAND] [OPTION]... [WWW-ROOT]\n\nValid values for COMMAND:\n"
        ^^ "\tcheck\t\tShows current Hack errors\n"
        ^^ "\tstart\t\tStarts a Hack server\n"
        ^^ "\tstop\t\tStops a Hack server\n"
        ^^ "\trestart\t\tRestarts a Hack server\n"
        ^^ "\tlsp\t\tRuns a persistent language service\n"
        ^^ "\tdebug\t\tDebug mode\n"
        ^^ "\trage\t\tReport a bug\n"
        ^^ "\nDefault values if unspecified:\n"
        ^^ "\tCOMMAND\t\tcheck\n"
        ^^ "\tWWW-ROOT\tCurrent directory\n\nCheck command options:\n" )
        Sys.argv.(0)
    | _ -> failwith "No other keywords should make it here"
  in
  let options =
    [
      (* Please keep these sorted in the alphabetical order *)
      ( "--ai",
        Arg.String
          (fun s ->
            ai_mode :=
              Some
                ( ignore (Ai_options.prepare ~server:true s);
                  s )),
        " run AI module with provided options" );
      ( "--ai-query",
        Arg.String (fun x -> set_mode (MODE_AI_QUERY x) ()),
        (* Send an AI query *) "" );
      Common_argspecs.allow_non_opt_build allow_non_opt_build;
      ( "--auto-complete",
        Arg.Unit (set_mode MODE_AUTO_COMPLETE),
        " (mode) auto-completes the text on stdin" );
      ( "--autostart-server",
        Arg.Bool (fun x -> autostart := x),
        " automatically start hh_server if it's not running (default: true)" );
      ( "--bigcode",
        Arg.String (fun filename -> set_mode (MODE_BIGCODE filename) ()),
        " (mode) source code indexing functionalities for Big Code analysis" );
      ( "--color",
        Arg.String (fun x -> set_mode (MODE_COLORING x) ()),
        " (mode) pretty prints the file content showing what is checked (give '-' for stdin)"
      );
      ("--colour", Arg.String (fun x -> set_mode (MODE_COLORING x) ()), " ");
      Common_argspecs.config config;
      ( "--coverage",
        Arg.String (fun x -> set_mode (MODE_COVERAGE x) ()),
        " (mode) calculates the extent of typing of a given file or directory"
      );
      ( "--create-checkpoint",
        Arg.String (fun x -> set_mode (MODE_CREATE_CHECKPOINT x) ()),
        (* Create a checkpoint which can be used to retrieve changed files later *)
        "" );
      ( "--cst-search",
        Arg.Unit (set_mode (MODE_CST_SEARCH None)),
        " (mode) Search the concrete syntax trees of files in the codebase"
        ^ " for a given pattern" );
      ( "--cst-search-files",
        Arg.Rest
          begin
            fun fn ->
            mode :=
              match !mode with
              | None
              | Some (MODE_CST_SEARCH None) ->
                Some (MODE_CST_SEARCH (Some [fn]))
              | Some (MODE_CST_SEARCH (Some fnl)) ->
                Some (MODE_CST_SEARCH (Some (fn :: fnl)))
              | _ -> raise (Arg.Bad "only a single mode should be specified")
          end,
        " Run CST search on this set of files,"
        ^ " rather than all the files in the codebase." );
      Common_argspecs.custom_telemetry_data custom_telemetry_data;
      (* Delete an existing checkpoint.
       * Exitcode will be non-zero if no checkpoint is found *)
      ( "--delete-checkpoint",
        Arg.String (fun x -> set_mode (MODE_DELETE_CHECKPOINT x) ()),
        "" );
      ( "--dump-full-fidelity-parse",
        Arg.String (fun x -> set_mode (MODE_FULL_FIDELITY_PARSE x) ()),
        "" );
      ( "--dump-symbol-info",
        Arg.String (fun files -> set_mode (MODE_DUMP_SYMBOL_INFO files) ()),
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
      ( "--dynamic-view",
        Arg.Set dynamic_view,
        " Replace occurrences of untyped code with dynamic" );
      ( "--error-format",
        Arg.String
          (fun s ->
            match s with
            | "raw" -> error_format := Errors.Raw
            | "context" -> error_format := Errors.Context
            | "highlighted" -> error_format := Errors.Highlighted
            | _ -> print_string "Warning: unrecognized error format.\n"),
        "<raw|context|highlighted> Error formatting style" );
      ( "--extract-standalone",
        Arg.String (fun name -> set_mode (MODE_EXTRACT_STANDALONE name) ()),
        " extract a given function / method together with its dependencies as a standalone file"
      );
      ( "--concatenate-all",
        Arg.Unit (fun () -> set_mode MODE_CONCATENATE_ALL ()),
        "(mode) create a single file containing all Hack code in the specified prefix"
      );
      ( "--file-dependents",
        Arg.Unit
          (fun () ->
            let () = prechecked := Some false in
            set_mode MODE_FILE_DEPENDENTS ()),
        " (mode) Given a list of filepaths, shows list of (possibly) dependent files"
      );
      ( "--find-class-refs",
        Arg.String (fun x -> set_mode (MODE_FIND_CLASS_REFS x) ()),
        " (mode) finds references of the provided class name" );
      ( "--find-refs",
        Arg.String (fun x -> set_mode (MODE_FIND_REFS x) ()),
        " (mode) finds references of the provided method name" );
      Common_argspecs.force_dormant_start force_dormant_start;
      ( "--format",
        Arg.Tuple
          [
            Arg.Int (fun x -> format_from := x);
            Arg.Int (fun x -> set_mode (MODE_FORMAT (!format_from, x)) ());
          ],
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
        Arg.Unit (set_mode MODE_FULL_FIDELITY_SCHEMA),
        "" );
      ( "--fun-deps-at-pos-batch",
        Arg.Rest
          begin
            fun position ->
            mode :=
              match !mode with
              | None -> Some (MODE_FUN_DEPS_AT_POS_BATCH [position])
              | Some (MODE_FUN_DEPS_AT_POS_BATCH positions) ->
                Some (MODE_FUN_DEPS_AT_POS_BATCH (position :: positions))
              | _ -> raise (Arg.Bad "only a single mode should be specified")
          end,
        " (mode) for each entry in input list get list of function dependencies [file:line:character list]"
      );
      ( "--fun-is-locallable-at-pos-batch",
        Arg.Rest
          begin
            fun position ->
            mode :=
              match !mode with
              | None -> Some (MODE_FUN_IS_LOCALLABLE_AT_POS_BATCH [position])
              | Some (MODE_FUN_IS_LOCALLABLE_AT_POS_BATCH positions) ->
                Some
                  (MODE_FUN_IS_LOCALLABLE_AT_POS_BATCH (position :: positions))
              | _ -> raise (Arg.Bad "only a single mode should be specified")
          end,
        " (mode) for each entry in input list checks if function at position can be made RxLocal [file:line:character list]"
      );
      ( "--gen-hot-classes-file",
        Arg.Tuple
          [
            Arg.Int (fun x -> hot_classes_threshold := x);
            Arg.String
              (fun x ->
                set_mode (MODE_GEN_HOT_CLASSES (!hot_classes_threshold, x)) ());
          ],
        " generate a JSON file listing all classes with more dependents than the"
        ^ " given threshold. Usage: --gen-hot-classes-file 500 ~/hh_hot_classes.json"
      );
      ( "--gen-saved-ignore-type-errors",
        Arg.Set gen_saved_ignore_type_errors,
        " generate a saved state even if there are type errors (default: false)."
      );
      ( "--get-method-name",
        Arg.String (fun x -> set_mode (MODE_IDENTIFY_SYMBOL3 x) ()),
        (* alias for --identify-function *) "" );
      ( "--go-to-impl-class",
        Arg.String (fun x -> set_mode (MODE_GO_TO_IMPL_CLASS x) ()),
        " (mode) goes to implementation of the provided class/trait/interface/etc. with the given name"
      );
      ( "--go-to-impl-class-remote",
        Arg.String (fun x -> set_mode (MODE_GO_TO_IMPL_CLASS_REMOTE x) ()),
        " (mode) similar to go-to-class-impl, but uses a glean database for faster but potentially out-of-date results"
      );
      ( "--go-to-impl-method",
        Arg.String (fun x -> set_mode (MODE_GO_TO_IMPL_METHOD x) ()),
        " (mode) goes to implementation of the provided method name" );
      ( "--ide-find-refs",
        Arg.String (fun x -> set_mode (MODE_IDE_FIND_REFS x) ()),
        "" );
      ( "--ide-get-definition",
        Arg.String (fun x -> set_mode (MODE_IDENTIFY_SYMBOL2 x) ()),
        (* alias for --identify-function *) "" );
      ( "--ide-highlight-refs",
        Arg.String (fun x -> set_mode (MODE_IDE_HIGHLIGHT_REFS x) ()),
        (* Similar to --ide-find-refs, but returns references in current file only,
         * and is optimized to be faster in that case *)
        "" );
      ( "--global-inference",
        Arg.Rest
          begin
            fun fn ->
            mode :=
              match !mode with
              | None ->
                let submode =
                  (*
                - "merge" will gather all artifacts generated by typechecking with
                  global inference on and generate the global constraint graph
                - "solve" will solve the global constraint graph and bind all
                  global type variable to a concrete type
                - "export-json" will export the global constraint graph in a
                  json file *)
                  match fn with
                  | "merge" -> ServerGlobalInferenceTypes.MMerge
                  | "solve" -> ServerGlobalInferenceTypes.MSolve
                  | "export-json" -> ServerGlobalInferenceTypes.MExport
                  | "rewrite" -> ServerGlobalInferenceTypes.MRewrite
                  | _ ->
                    raise
                      (Arg.Bad
                         ("No " ^ fn ^ " submode supported for global inference"))
                in
                Some (MODE_GLOBAL_INFERENCE (submode, []))
              | Some (MODE_GLOBAL_INFERENCE (submode, fnl)) ->
                Some (MODE_GLOBAL_INFERENCE (submode, fn :: fnl))
              | _ -> raise (Arg.Bad "only a single mode should be specified")
          end,
        " (mode) global inference operations, Usage: --global-inference "
        ^ "[\"merge\", \"solve\", \"export-json\", \"rewrite\"] files..." );
      ("--ide-outline", Arg.Unit (set_mode MODE_OUTLINE2), "");
      ( "--ide-refactor",
        Arg.String (fun x -> set_mode (MODE_IDE_REFACTOR x) ()),
        " (mode) rename a symbol, Usage: --ide-refactor "
        ^ " <filename>:<line number>:<col number>:<new name>" );
      ( "--identify-function",
        Arg.String (fun x -> set_mode (MODE_IDENTIFY_SYMBOL1 x) ()),
        " (mode) print the full function name at the position "
        ^ "[line:character] of the text on stdin" );
      ( "--identify",
        Arg.String (fun x -> set_mode (MODE_IDENTIFY_SYMBOL x) ()),
        " (mode) identify the named symbol" );
      ( "--ignore-hh-version",
        Arg.Set ignore_hh_version,
        " ignore hh_version check when loading saved states (default: false)" );
      ( "--in-memory-dep-table-size",
        Arg.Unit (set_mode MODE_IN_MEMORY_DEP_TABLE_SIZE),
        " number of entries in the in-memory dependency table" );
      ( "--inheritance-ancestor-classes",
        Arg.String
          (fun x -> set_mode (MODE_METHOD_JUMP_ANCESTORS (x, "Class")) ()),
        " (mode) prints a list of classes that this class extends" );
      ( "--inheritance-ancestor-classes-batch",
        Arg.Rest
          begin
            fun class_ ->
            mode :=
              match !mode with
              | None ->
                Some (MODE_METHOD_JUMP_ANCESTORS_BATCH ([class_], "Class"))
              | Some (MODE_METHOD_JUMP_ANCESTORS_BATCH (classes, "Class")) ->
                Some
                  (MODE_METHOD_JUMP_ANCESTORS_BATCH (class_ :: classes, "Class"))
              | _ -> raise (Arg.Bad "only a single mode should be specified")
          end,
        " (mode) prints a list of classes that these classes extend" );
      ( "--inheritance-ancestor-interfaces",
        Arg.String
          (fun x -> set_mode (MODE_METHOD_JUMP_ANCESTORS (x, "Interface")) ()),
        " (mode) prints a list of interfaces that this class implements" );
      ( "--inheritance-ancestor-interfaces-batch",
        Arg.Rest
          begin
            fun class_ ->
            mode :=
              match !mode with
              | None ->
                Some (MODE_METHOD_JUMP_ANCESTORS_BATCH ([class_], "Interface"))
              | Some (MODE_METHOD_JUMP_ANCESTORS_BATCH (classes, "Interface"))
                ->
                Some
                  (MODE_METHOD_JUMP_ANCESTORS_BATCH
                     (class_ :: classes, "Interface"))
              | _ -> raise (Arg.Bad "only a single mode should be specified")
          end,
        " (mode) prints a list of interfaces that these classes implement" );
      ( "--inheritance-ancestor-traits",
        Arg.String
          (fun x -> set_mode (MODE_METHOD_JUMP_ANCESTORS (x, "Trait")) ()),
        " (mode) prints a list of traits that this class uses" );
      ( "--inheritance-ancestor-traits-batch",
        Arg.Rest
          begin
            fun class_ ->
            mode :=
              match !mode with
              | None ->
                Some (MODE_METHOD_JUMP_ANCESTORS_BATCH ([class_], "Trait"))
              | Some (MODE_METHOD_JUMP_ANCESTORS_BATCH (classes, "Trait")) ->
                Some
                  (MODE_METHOD_JUMP_ANCESTORS_BATCH (class_ :: classes, "Trait"))
              | _ -> raise (Arg.Bad "only a single mode should be specified")
          end,
        " (mode) prints a list of traits that these classes use" );
      ( "--inheritance-ancestors",
        Arg.String
          (fun x -> set_mode (MODE_METHOD_JUMP_ANCESTORS (x, "No_filter")) ()),
        " (mode) prints a list of all related classes or methods"
        ^ " to the given class" );
      ( "--inheritance-children",
        Arg.String (fun x -> set_mode (MODE_METHOD_JUMP_CHILDREN x) ()),
        " (mode) prints a list of all related classes or methods"
        ^ " to the given class" );
      ( "--json",
        Arg.Set output_json,
        " output json for machine consumption. (default: false)" );
      ( "--lint",
        Arg.Unit (set_mode MODE_LINT),
        " (mode) lint the given list of files" );
      ( "--lint-all",
        Arg.Int (fun x -> set_mode (MODE_LINT_ALL x) ()),
        " (mode) find all occurrences of lint with the given error code" );
      ( "--lint-stdin",
        Arg.String (fun filename -> set_mode (MODE_LINT_STDIN filename) ()),
        " (mode) lint a file given on stdin; the filename should be the"
        ^ " argument to this option" );
      ( "--lint-xcontroller",
        Arg.String
          (fun filename -> set_mode (MODE_LINT_XCONTROLLER filename) ()),
        "" )
      (* (mode) lint all xcontrollers in files listed in given file (i.e. the argument is
       * a path to a file that contains a list of files) *);
      ( "--list-files",
        Arg.Unit (set_mode MODE_LIST_FILES),
        " (mode) list files with errors" );
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
        Arg.Unit (set_mode MODE_OUTLINE),
        " (mode) prints an outline of the text on stdin" );
      Common_argspecs.prechecked prechecked;
      Common_argspecs.no_prechecked prechecked;
      ( "--pause",
        Arg.Unit (set_mode (MODE_PAUSE true)),
        " (mode) pause recheck-on-file-change [EXPERIMENTAL]" );
      ("--profile-log", Arg.Set profile_log, " enable profile logging");
      ( "--refactor",
        Arg.Tuple
          [
            Arg.Symbol
              (["Class"; "Function"; "Method"], (fun x -> refactor_mode := x));
            Arg.String (fun x -> refactor_before := x);
            Arg.String
              (fun x ->
                set_mode
                  (MODE_REFACTOR (!refactor_mode, !refactor_before, x))
                  ());
          ],
        " (mode) rename a symbol, Usage: --refactor "
        ^ "[\"Class\", \"Function\", \"Method\"] <Current Name> <New Name>" );
      ("--remote", Arg.Set remote, " force remote type checking");
      ( "--remove-dead-fixme",
        Arg.Int
          begin
            fun code ->
            mode :=
              match !mode with
              | None -> Some (MODE_REMOVE_DEAD_FIXMES [code])
              | Some (MODE_REMOVE_DEAD_FIXMES codel) ->
                Some (MODE_REMOVE_DEAD_FIXMES (code :: codel))
              | _ -> raise (Arg.Bad "only a single mode should be specified")
          end,
        " (mode) remove dead HH_FIXME for specified error code "
        ^ "(first do hh_client restart --no-load)" );
      ( "--remove-dead-fixmes",
        Arg.Unit (set_mode (MODE_REMOVE_DEAD_FIXMES [])),
        " (mode) remove dead HH_FIXME for any error code < 5000 "
        ^ "(first do hh_client restart --no-load)" );
      ( "--replace-state-after-saving",
        Arg.Set replace_state_after_saving,
        " if combined with --save-mini, causes the saved state"
        ^ " to replace the program state; otherwise, the state files are not"
        ^ " used after being written to disk (default: false)" );
      ( "--resume",
        Arg.Unit (set_mode (MODE_PAUSE false)),
        " (mode) resume recheck-on-file-change [EXPERIMENTAL]" );
      ( "--retries",
        Arg.Int (fun n -> timeout := Some (float_of_int (max 5 n))),
        " (deprecated) same as --timeout" );
      (* Retrieve changed files since input checkpoint.
       * Output is separated by newline.
       * Exit code will be non-zero if no checkpoint is found *)
      ( "--retrieve-checkpoint",
        Arg.String (fun x -> set_mode (MODE_RETRIEVE_CHECKPOINT x) ()),
        "" );
      ("--retry-if-init", Arg.Bool (fun _ -> ()), " (deprecated and ignored)");
      ( "--rewrite-lambda-parameters",
        Arg.Rest
          begin
            fun fn ->
            mode :=
              match !mode with
              | None -> Some (MODE_REWRITE_LAMBDA_PARAMETERS [fn])
              | Some (MODE_REWRITE_LAMBDA_PARAMETERS fnl) ->
                Some (MODE_REWRITE_LAMBDA_PARAMETERS (fn :: fnl))
              | _ -> raise (Arg.Bad "only a single mode should be specified")
          end,
        " (mode) rewrite lambdas in the files from the given list"
        ^ " with suggested parameter types" );
      ( "--rewrite-partial-parameters-type-hints",
        Arg.Rest
          begin
            fun fn ->
            mode :=
              match !mode with
              | None -> Some (MODE_REWRITE_TYPE_PARAMS_TYPE [fn])
              | Some (MODE_REWRITE_TYPE_PARAMS_TYPE fnl) ->
                Some (MODE_REWRITE_TYPE_PARAMS_TYPE (fn :: fnl))
              | _ -> raise (Arg.Bad "only a single mode should be specified")
          end,
        " (mode) add missing type parameters in the type hints for function"
        ^ " parameters (e.g.: C $x -> C<int> $x) in the files from the given list"
      );
      ( "--save-naming",
        Arg.String (fun x -> set_mode (MODE_SAVE_NAMING x) ()),
        " (mode) Save the naming table to the given file."
        ^ " Returns the number of files and symbols written to disk." );
      ( "--save-state",
        Arg.String (fun x -> set_mode (MODE_SAVE_STATE x) ()),
        " (mode) Save a saved state to the given file."
        ^ " Returns number of edges dumped from memory to the database." );
      ( "--saved-state-ignore-hhconfig",
        Arg.Set saved_state_ignore_hhconfig,
        " ignore hhconfig hash when loading saved states (default: false)" );
      ( "--search",
        Arg.String (fun x -> set_mode (MODE_SEARCH (x, "")) ()),
        " (mode) fuzzy search symbol definitions" );
      ( "--search-class",
        Arg.String (fun x -> set_mode (MODE_SEARCH (x, "class")) ()),
        " (mode) fuzzy search class definitions" );
      ( "--search-constant",
        Arg.String (fun x -> set_mode (MODE_SEARCH (x, "constant")) ()),
        " (mode) fuzzy search constant definitions" );
      ( "--search-function",
        Arg.String (fun x -> set_mode (MODE_SEARCH (x, "function")) ()),
        " (mode) fuzzy search function definitions" );
      ( "--search-typedef",
        Arg.String (fun x -> set_mode (MODE_SEARCH (x, "typedef")) ()),
        " (mode) fuzzy search typedef definitions" );
      ( "--server-rage",
        Arg.Unit (set_mode MODE_SERVER_RAGE),
        " (mode) dumps internal state of hh_server" );
      ( "--show-spinner",
        Arg.Bool (fun x -> show_spinner := Some x),
        " shows a spinner while awaiting the typechecker" );
      ( "--single",
        Arg.String (fun x -> set_mode (MODE_STATUS_SINGLE x) ()),
        "<path> Return errors in file with provided name (give '-' for stdin)"
      );
      ("--sort-results", Arg.Set sort_results, " sort output for CST search.");
      ( "--stats",
        Arg.Unit (set_mode MODE_STATS),
        " display some server statistics" );
      ( "--stdin-name",
        Arg.String (fun x -> stdin_name := Some x),
        " substitute stdin for contents of file with specified name" );
      ( "--status",
        Arg.Unit (set_mode MODE_STATUS),
        " (mode) show a human readable list of errors (default)" );
      ( "--timeout",
        Arg.Float (fun x -> timeout := Some (Float.max 5. x)),
        " set the timeout in seconds (default: no timeout)" );
      ( "--type-at-pos",
        Arg.String (fun x -> set_mode (MODE_TYPE_AT_POS x) ()),
        " (mode) show type at a given position in file [line:character]" );
      ( "--type-at-pos-batch",
        Arg.Rest
          begin
            fun position ->
            mode :=
              match !mode with
              | None -> Some (MODE_TYPE_AT_POS_BATCH [position])
              | Some (MODE_TYPE_AT_POS_BATCH positions) ->
                Some (MODE_TYPE_AT_POS_BATCH (position :: positions))
              | _ -> raise (Arg.Bad "only a single mode should be specified")
          end,
        " (mode) show types at multiple positions [file:line:character list]" );
      ( "--verbose-on",
        Arg.Unit (fun () -> set_mode (MODE_VERBOSE true) ()),
        " (mode) turn on verbose server log" );
      ( "--verbose-off",
        Arg.Unit (fun () -> set_mode (MODE_VERBOSE false) ()),
        " (mode) turn off verbose server log" );
      ("--version", Arg.Set version, " (mode) show version and exit");
      Common_argspecs.watchman_debug_logging watchman_debug_logging;
      (* Please keep these sorted in the alphabetical order *)
    ]
  in
  let args = parse_without_command options usage "check" in
  if !version then (
    if !output_json then
      ServerArgs.print_json_version ()
    else
      print_endline Hh_version.version;
    exit 0
  );

  let mode = Option.value !mode ~default:MODE_STATUS in
  (* fixups *)
  let (root, paths) =
    match (mode, args) with
    | (MODE_LINT, _)
    | (MODE_CONCATENATE_ALL, _)
    | (MODE_FILE_DEPENDENTS, _) ->
      (Wwwroot.get None, args)
    | (_, []) -> (Wwwroot.get None, [])
    | (_, [x]) -> (Wwwroot.get (Some x), [])
    | (_, _) ->
      Printf.fprintf
        stderr
        "Error: please provide at most one www directory\n%!";
      exit 1
  in
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

  let () =
    if String.equal !from "emacs" then
      Printf.fprintf stdout "-*- mode: compilation -*-\n%!"
  in
  CCheck
    {
      ai_mode = !ai_mode;
      autostart = !autostart;
      config = !config;
      custom_telemetry_data = !custom_telemetry_data;
      dynamic_view = !dynamic_view;
      error_format = !error_format;
      force_dormant_start = !force_dormant_start;
      from = !from;
      show_spinner = Option.value ~default:(String.equal !from "") !show_spinner;
      gen_saved_ignore_type_errors = !gen_saved_ignore_type_errors;
      ignore_hh_version = !ignore_hh_version;
      saved_state_ignore_hhconfig = !saved_state_ignore_hhconfig;
      paths;
      log_inference_constraints = !log_inference_constraints;
      max_errors = !max_errors;
      mode;
      no_load =
        ( !no_load
        ||
        match mode with
        | MODE_REMOVE_DEAD_FIXMES _ -> true
        | _ -> false );
      output_json = !output_json;
      prechecked = !prechecked;
      profile_log = !profile_log;
      remote = !remote;
      replace_state_after_saving = !replace_state_after_saving;
      root;
      sort_results = !sort_results;
      stdin_name = !stdin_name;
      deadline = Option.map ~f:(fun t -> Unix.time () +. t) !timeout;
      watchman_debug_logging = !watchman_debug_logging;
      allow_non_opt_build = !allow_non_opt_build;
    }

let parse_start_env command =
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
  let profile_log = ref false in
  let ai_mode = ref None in
  let ignore_hh_version = ref false in
  let saved_state_ignore_hhconfig = ref false in
  let prechecked = ref None in
  let from = ref "" in
  let config = ref [] in
  let custom_telemetry_data = ref [] in
  let allow_non_opt_build = ref false in
  let wait_deprecation_msg () =
    Printf.eprintf
      "WARNING: --wait is deprecated, does nothing, and will be going away soon!\n%!"
  in
  let options =
    [
      (* Please keep these sorted in the alphabetical order *)
      ("--ai", Arg.String (fun x -> ai_mode := Some x), " run ai with options ");
      Common_argspecs.allow_non_opt_build allow_non_opt_build;
      Common_argspecs.config config;
      Common_argspecs.custom_telemetry_data custom_telemetry_data;
      Common_argspecs.from from;
      ( "--ignore-hh-version",
        Arg.Set ignore_hh_version,
        " ignore hh_version check when loading saved states (default: false)" );
      ( "--log-inference-constraints",
        Arg.Set log_inference_constraints,
        " (for hh debugging purpose only) log type"
        ^ " inference constraints into external logger (e.g. Scuba)" );
      ("--no-load", Arg.Set no_load, " start from a fresh state");
      Common_argspecs.no_prechecked prechecked;
      Common_argspecs.prechecked prechecked;
      ("--profile-log", Arg.Set profile_log, " enable profile logging");
      ( "--saved-state-ignore-hhconfig",
        Arg.Set saved_state_ignore_hhconfig,
        " ignore hhconfig hash when loading saved states (default: false)" );
      ( "--wait",
        Arg.Unit wait_deprecation_msg,
        " this flag is deprecated and does nothing!" );
      Common_argspecs.watchman_debug_logging watchman_debug_logging;
      (* Please keep these sorted in the alphabetical order *)
    ]
  in
  let args = parse_without_command options usage command in
  let root =
    match args with
    | [] -> Wwwroot.get None
    | [x] -> Wwwroot.get (Some x)
    | _ ->
      Printf.fprintf
        stderr
        "Error: please provide at most one www directory\n%!";
      exit 1
  in
  {
    ClientStart.ai_mode = !ai_mode;
    config = !config;
    custom_telemetry_data = !custom_telemetry_data;
    debug_port = None;
    dynamic_view = false;
    exit_on_failure = true;
    from = !from;
    ignore_hh_version = !ignore_hh_version;
    saved_state_ignore_hhconfig = !saved_state_ignore_hhconfig;
    log_inference_constraints = !log_inference_constraints;
    no_load = !no_load;
    prechecked = !prechecked;
    profile_log = !profile_log;
    root;
    silent = false;
    watchman_debug_logging = !watchman_debug_logging;
    allow_non_opt_build = !allow_non_opt_build;
  }

let parse_start_args () = CStart (parse_start_env "start")

let parse_restart_args () = CRestart (parse_start_env "restart")

let parse_stop_args () =
  let usage =
    Printf.sprintf
      "Usage: %s stop [OPTION]... [WWW-ROOT]\nStop a hack server\n\nWWW-ROOT is assumed to be current directory if unspecified\n"
      Sys.argv.(0)
  in
  let from = ref "" in
  let options = [Common_argspecs.from from] in
  let args = parse_without_command options usage "stop" in
  let root =
    match args with
    | [] -> Wwwroot.get None
    | [x] -> Wwwroot.get (Some x)
    | _ ->
      Printf.fprintf
        stderr
        "Error: please provide at most one www directory\n%!";
      exit 1
  in
  CStop { ClientStop.root; from = !from }

let parse_lsp_args ~(init_id : string) =
  let usage =
    Printf.sprintf
      "Usage: %s lsp [OPTION]...\nRuns a persistent language service\n"
      Sys.argv.(0)
  in
  let from = ref "" in
  let config = ref [] in
  let use_ffp_autocomplete = ref false in
  let use_ranked_autocomplete = ref false in
  let use_serverless_ide = ref false in
  let verbose = ref false in
  let options =
    [
      (* Please keep these sorted in the alphabetical order *)
      ("--enhanced-hover", Arg.Unit (fun () -> ()), " [legacy] no-op");
      ( "--ffp-autocomplete",
        Arg.Set use_ffp_autocomplete,
        " [experimental] use the full-fidelity parser based autocomplete " );
      Common_argspecs.from from;
      Common_argspecs.config config;
      ( "--ranked-autocomplete",
        Arg.Set use_ranked_autocomplete,
        " [experimental] display ranked autocompletion results" );
      ( "--serverless-ide",
        Arg.Set use_serverless_ide,
        " [experimental] provide IDE services from hh_client instead of hh_server"
      );
      ( "--verbose",
        Arg.Set verbose,
        " verbose logs to stderr and `hh --ide-logname` and `--lsp-logname`" );
      (* Please keep these sorted in the alphabetical order *)
    ]
  in
  let args = parse_without_command options usage "lsp" in
  match args with
  | [] ->
    CLsp
      {
        ClientLsp.from = !from;
        config = !config;
        use_ffp_autocomplete = !use_ffp_autocomplete;
        use_ranked_autocomplete = !use_ranked_autocomplete;
        use_serverless_ide = !use_serverless_ide;
        verbose = !verbose;
        init_id;
      }
  | _ ->
    Printf.printf "%s\n" usage;
    exit 2

let parse_debug_args () =
  let usage =
    Printf.sprintf "Usage: %s debug [OPTION]... [WWW-ROOT]\n" Sys.argv.(0)
  in
  let from = ref "" in
  let options = [Common_argspecs.from from] in
  let args = parse_without_command options usage "debug" in
  let root =
    match args with
    | [] -> Wwwroot.get None
    | [x] -> Wwwroot.get (Some x)
    | _ ->
      Printf.printf "%s\n" usage;
      exit 2
  in
  CDebug { ClientDebug.root; from = !from }

let parse_rage_args () =
  let usage =
    Printf.sprintf "Usage: %s rage [OPTION]... [WWW-ROOT]\n" Sys.argv.(0)
  in
  let from = ref "" in
  let desc = ref None in
  let rageid = ref None in
  let options =
    [
      Common_argspecs.from from;
      ("--desc", Arg.String (fun s -> desc := Some s), " description of problem");
      ( "--rageid",
        Arg.String (fun s -> rageid := Some s),
        " (optional) use this id, and finish even if parent process dies" );
    ]
  in
  let args = parse_without_command options usage "rage" in
  let root =
    match args with
    | [] -> Wwwroot.get None
    | [x] -> Wwwroot.get (Some x)
    | _ ->
      Printf.printf "%s\n" usage;
      exit 2
  in
  (* hh_client normally handles Ctrl+C by printing an exception-stack.
  But for us, in an interactive prompt, Ctrl+C is an unexceptional way to quit. *)
  Sys_utils.set_signal Sys.sigint Sys.Signal_default;

  let desc =
    match !desc with
    | Some desc -> desc
    | None ->
      Printf.printf
        ( "Sorry that hh isn't working. What's wrong?\n"
        ^^ "1. hh_server takes ages to initialize\n"
        ^^ "2. hh is stuck in an infinite loop\n"
        ^^ "3. hh gives some error message about the monitor\n"
        ^^ "4. hack says it has an internal typecheck bug and asked me to report it\n"
        ^^ "5. hack is reporting errors that are clearly incorrect [please elaborate]\n"
        ^^ "6. I'm not sure how to write my code to avoid these hack errors\n"
        ^^ "7. hh says something about unsaved changes from an editor even after I've quit my editor\n"
        ^^ "8. something's wrong with hack VS Code or other editor\n"
        ^^ "[other] Please type either one of the above numbers, or a freeform description\n"
        ^^ "\nrage> %!" );
      let response = In_channel.input_line_exn In_channel.stdin in
      let (response, info) =
        if String.equal response "1" then
          ("hh_server slow initialize", `Verbose_hh_start)
        else if String.equal response "2" then
          ("hh stuck in infinite loop", `Verbose_hh_start)
        else if String.equal response "3" then
          ("hh monitor problem", `Verbose_hh_start)
        else if String.equal response "4" then
          ("internal typecheck bug", `No_info)
        else if String.equal response "5" then
          let () =
            Printf.printf
              "Please elaborate on which errors are incorrect...\nrage> %!"
          in
          (In_channel.input_line_exn In_channel.stdin, `No_info)
        else if String.equal response "6" then
          let () =
            Printf.printf
              ( "Please ask in the appropriate support groups for advice on coding in Hack; "
              ^^ "`hh rage` is solely for reporting bugs in the tooling, not for reporting typechecker or "
              ^^ "language issues." )
          in
          exit 0
        else if String.equal response "7" then
          ("unsaved editor changes", `Unsaved)
        else if String.equal response "8" then
          let () =
            Printf.printf
              ( "Please file the bug from within your editor to capture the right logs. "
              ^^ "Note: you can do Preferences > Settings > Hack > Verbose, then `pkill hh_client`, "
              ^^ "then reproduce the error, then file the bug. This way we'll get even richer logs.\n%!"
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
            ( "\nPOWER USERS ONLY: Sometimes the normal logging from hh_server isn't "
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
  CRage { ClientRage.root; from = !from; desc; rageid = !rageid }

let parse_download_saved_state_args () =
  let usage =
    Printf.sprintf
      {|Usage: %s download-saved-state [OPTION]... [WWW-ROOT]

Download a saved-state to disk for the given repository, to make future
invocations of `hh` faster.|}
      Sys.argv.(0)
  in
  let valid_types_message =
    "Valid values are: naming-and-dep-table, naming-table"
  in

  let from = ref "" in
  let saved_state_type = ref None in
  let should_save_replay = ref false in
  let replay_token = ref None in
  let options =
    Arg.align
      [
        Common_argspecs.from from;
        ( "--type",
          Arg.String (fun arg -> saved_state_type := Some arg),
          Printf.sprintf
            " The type of saved-state to download. %s"
            valid_types_message );
        ( "--save-replay",
          Arg.Set should_save_replay,
          " Produce a token that can be later consumed by --replay-token to replay the same saved-state download."
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
    | [x] -> Wwwroot.get (Some x)
    | _ ->
      print_endline usage;
      exit 2
  in
  let from =
    match !from with
    | "" ->
      print_endline "The '--from' option is required.";
      exit 2
    | from -> from
  in
  let saved_state_type =
    match !saved_state_type with
    | None ->
      Printf.printf "The '--type' option is required. %s\n" valid_types_message;
      exit 2
    | Some "naming-and-dep-table" ->
      ClientDownloadSavedState.Naming_and_dep_table
    | Some "naming-table" -> ClientDownloadSavedState.Naming_table
    | Some saved_state_type ->
      Printf.printf
        "Unrecognized value '%s' for '--type'. %s\n"
        saved_state_type
        valid_types_message;
      exit 2
  in
  CDownloadSavedState
    {
      ClientDownloadSavedState.root;
      from;
      saved_state_type;
      should_save_replay = !should_save_replay;
      replay_token = !replay_token;
    }

let parse_args ~(init_id : string) : command =
  match parse_command () with
  | (CKNone | CKCheck) as cmd -> parse_check_args cmd
  | CKStart -> parse_start_args ()
  | CKStop -> parse_stop_args ()
  | CKRestart -> parse_restart_args ()
  | CKDebug -> parse_debug_args ()
  | CKLsp -> parse_lsp_args ~init_id
  | CKRage -> parse_rage_args ()
  | CKDownloadSavedState -> parse_download_saved_state_args ()

let root = function
  | CCheck { ClientEnv.root; _ }
  | CStart { ClientStart.root; _ }
  | CRestart { ClientStart.root; _ }
  | CStop { ClientStop.root; _ }
  | CDebug { ClientDebug.root; _ }
  | CRage { ClientRage.root; _ }
  | CDownloadSavedState { ClientDownloadSavedState.root; _ } ->
    Some root
  | CLsp _ -> None
