(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open ClientCommand
open ClientEnv
open Utils

(** Arg specs shared across more than 1 arg parser. *)
module Common_argspecs = struct
  let config value_ref =
    "--config",
      Arg.String (fun s -> value_ref := (String_utils.split2_exn '=' s) :: !value_ref),
      " override arbitrary value from hh.conf (format: <key>=<value>)"

  let force_dormant_start value_ref =
    ("--force-dormant-start",
      Arg.Bool (fun x -> value_ref := x),
      " If server is dormant, force start a new one instead of waiting for"^
      " the next one to start up automatically (default: false)")

  let from value_ref =
    ("--from",
      Arg.Set_string value_ref,
      " so we know who's calling hh_client - e.g. nuclide, vim, emacs, vscode")

  let no_prechecked value_ref =
    "--no-prechecked", Arg.Unit (fun () -> value_ref := Some false),
    " override value of \"prechecked_files\" flag from hh.conf"

  let prechecked value_ref =
    "--prechecked", Arg.Unit (fun () -> value_ref := Some true),
    " override value of \"prechecked_files\" flag from hh.conf"

  let retries value_ref =
    ("--retries",
      Arg.Set_int value_ref,
      spf (" set the number of retries for connecting to server. " ^^
        "Roughly 1 retry per second (default: %d)") !value_ref;)

  let watchman_debug_logging value_ref =
    ("--watchman-debug-logging",
      Arg.Set value_ref,
      " Enable debug logging on Watchman client. This is very noisy")
end


let parse_command () =
  if Array.length Sys.argv < 2
  then CKNone
  else match String.lowercase Sys.argv.(1) with
  | "check" -> CKCheck
  | "start" -> CKStart
  | "stop" -> CKStop
  | "restart" -> CKRestart
  | "lsp" -> CKLsp
  | "debug" -> CKDebug
  | _ -> CKNone

let parse_without_command options usage command =
  let args = ref [] in
  Arg.parse (Arg.align options) (fun x -> args := x::!args) usage;
  match List.rev !args with
  | x::rest when (String.lowercase x) = (String.lowercase command)
    -> rest
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
  let dynamic_view = ref false in
  let error_format = ref Errors.Context in
  let file_info_on_disk = ref false in
  let force_dormant_start = ref false in
  let format_from = ref 0 in
  let from = ref "" in
  let hot_classes_threshold = ref 0 in
  let gen_saved_ignore_type_errors = ref false in
  let ignore_hh_version = ref false in
  let saved_state_ignore_hhconfig = ref false in
  let logname = ref false in
  let log_inference_constraints = ref false in
  let mode = ref None in
  let monitor_logname = ref false in
  let no_load = ref false in
  let output_json = ref false in
  let prechecked = ref None in
  let profile_log = ref false in
  let refactor_before = ref "" in
  let refactor_mode = ref "" in
  let replace_state_after_saving = ref false in
  let retries = ref 1300 in
  let sort_results = ref false in
  let timeout = ref None in
  let version = ref false in
  let watchman_debug_logging = ref false in

  (* custom behaviors *)
  let set_from x () = from := x in
  let set_mode x () =
    if !mode <> None
    then raise (Arg.Bad "only a single mode should be specified")
    else mode := Some x
  in

  (* parse args *)
  let usage =
    match cmd with
    | CKCheck -> Printf.sprintf
      "Usage: %s check [OPTION]... [WWW-ROOT]\n\n\
      WWW-ROOT is assumed to be current directory if unspecified\n"
      Sys.argv.(0)
    | CKNone -> Printf.sprintf
      "Usage: %s [COMMAND] [OPTION]... [WWW-ROOT]\n\n\
      Valid values for COMMAND:\n\
        \tcheck\
          \t\tShows current Hack errors\n\
        \tstart\
          \t\tStarts a Hack server\n\
        \tstop\
          \t\tStops a Hack server\n\
        \trestart\
          \t\tRestarts a Hack server\n\
        \tlsp\
          \t\t[experimental] runs a persistent language service\n\
        \tdebug\
          \t\tDebug mode\n\
      \n\
      Default values if unspecified:\n\
        \tCOMMAND\
          \t\tcheck\n\
        \tWWW-ROOT\
          \tCurrent directory\n\
      \n\
      Check command options:\n"
      Sys.argv.(0)
    | _ -> failwith "No other keywords should make it here"
  in
  let options = [
    (* modes - please keep sorted in the alphabetical order *)
    "--ai",
      Arg.String (fun s -> ai_mode :=
         Some (ignore (Ai_options.prepare ~server:true s); s)),
      " run AI module with provided options\n";
    "--ai-query", Arg.String (fun x -> set_mode (MODE_AI_QUERY x) ()),
      (* Send an AI query *) "";
    "--auto-complete",
      Arg.Unit (set_mode MODE_AUTO_COMPLETE),
      " (mode) auto-completes the text on stdin";
    "--autostart-server",
      Arg.Bool (fun x -> autostart := x),
      " automatically start hh_server if it's not running (default: true)";
    "--color",
      Arg.String (fun x -> set_mode (MODE_COLORING x) ()),
      " (mode) pretty prints the file content \
       showing what is checked (give '-' for stdin)";
    "--colour",
      Arg.String (fun x -> set_mode (MODE_COLORING x) ()), " ";
    Common_argspecs.config config;
    "--coverage",
      Arg.String (fun x -> set_mode (MODE_COVERAGE x) ()),
      " (mode) calculates the extent of typing of a given file or directory";
    "--create-checkpoint",
      Arg.String (fun x -> set_mode (MODE_CREATE_CHECKPOINT x) ()),
      (* Create a checkpoint which can be used to retrieve changed files later *)"";
    "--cst-search",
      Arg.Unit (set_mode (MODE_CST_SEARCH None)),
      " (mode) Search the concrete syntax trees of files in the codebase" ^
      " for a given pattern";
    "--cst-search-files",
      Arg.Rest begin fun fn ->
        mode := match !mode with
          | None
          | Some (MODE_CST_SEARCH (None)) ->
            Some (MODE_CST_SEARCH (Some [fn]))
          | Some (MODE_CST_SEARCH (Some fnl)) ->
            Some (MODE_CST_SEARCH (Some (fn :: fnl)))
          | _ -> raise (Arg.Bad "only a single mode should be specified")
      end,
      " Run CST search on this set of files," ^
      " rather than all the files in the codebase.";
      (* Delete an existing checkpoint.
       * Exitcode will be non-zero if no checkpoint is found *)
    "--delete-checkpoint",
      Arg.String (fun x -> set_mode (MODE_DELETE_CHECKPOINT x) ()),
      "";
    "--dump-full-fidelity-parse",
        Arg.String (fun x -> set_mode (MODE_FULL_FIDELITY_PARSE x) ()),
        "";
    "--dump-symbol-info",
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
      "";
    "--dynamic-view",
      Arg.Set dynamic_view,
      " Replace occurrences of untyped code with dynamic";
    "--error-format",
      Arg.String (fun s ->
          match s with
          | "raw" -> error_format := Errors.Raw
          | "context" -> error_format := Errors.Context
          | _ -> print_string "Warning: unrecognized error format.\n"),
      "<raw|context> Error formatting style";
    "--file-info-on-disk",
      Arg.Set file_info_on_disk,
      " [experimental] a saved state option to store file info" ^
      " (the naming table) in SQLite. Only has meaning in --saved-state mode.";
    "--find-class-refs",
      Arg.String (fun x -> set_mode (MODE_FIND_CLASS_REFS x) ()),
      " (mode) finds references of the provided class name";
    "--find-refs",
      Arg.String (fun x -> set_mode (MODE_FIND_REFS x) ()),
      " (mode) finds references of the provided method name";
    Common_argspecs.force_dormant_start force_dormant_start;
    "--format",
      Arg.Tuple ([
        Arg.Int (fun x -> format_from := x);
        Arg.Int (fun x -> set_mode (MODE_FORMAT (!format_from, x)) ())
      ]), "";
    Common_argspecs.from from;
    "--from-arc-diff", Arg.Unit (set_from "arc_diff"),
      " (deprecated) equivalent to --from arc_diff";
    "--from-arc-land", Arg.Unit (set_from "arc_land"),
      " (deprecated) equivalent to --from arc_land";
    "--from-check-trunk", Arg.Unit (set_from "check_trunk"),
      " (deprecated) equivalent to --from check_trunk";
    "--from-emacs", Arg.Unit (set_from "emacs"),
      " (deprecated) equivalent to --from emacs";
    "--from-vim",
      Arg.Unit (fun () -> from := "vim"; retries := 0),
      " (deprecated) equivalent to \
       --from vim --retries 0";
    "--full-fidelity-schema",
      Arg.Unit (set_mode MODE_FULL_FIDELITY_SCHEMA), "";
    "--gen-hot-classes-file",
      Arg.Tuple ([
        Arg.Int (fun x -> hot_classes_threshold := x);
        Arg.String (fun x -> set_mode (MODE_GEN_HOT_CLASSES (!hot_classes_threshold, x)) ());
      ]),
      " generate a JSON file listing all classes with more dependents than the" ^
      " given threshold. Usage: --gen-hot-classes-file 500 ~/hh_hot_classes.json";
    "--gen-saved-ignore-type-errors",
      Arg.Set gen_saved_ignore_type_errors,
      " generate a saved state even if there are type errors (default: false).";
    "--get-method-name",
      Arg.String (fun x -> set_mode (MODE_IDENTIFY_SYMBOL3 x) ()),
      (* alias for --identify-function *) "";
    "--ide-find-refs",
      Arg.String (fun x -> set_mode (MODE_IDE_FIND_REFS x) ()), "";
    "--ide-get-definition",
      Arg.String (fun x -> set_mode (MODE_IDENTIFY_SYMBOL2 x) ()),
      (* alias for --identify-function *) "";
    "--ide-highlight-refs",
      Arg.String (fun x -> set_mode (MODE_IDE_HIGHLIGHT_REFS x) ()),
      (* Similar to --ide-find-refs, but returns references in current file only,
       * and is optimized to be faster in that case *) "";
    "--ide-outline",
      Arg.Unit (set_mode (MODE_OUTLINE2)), "";
    "--ide-refactor", Arg.String (fun x -> set_mode (MODE_IDE_REFACTOR x) ()),
      " (mode) rename a symbol, Usage: --ide-refactor " ^
      " <filename>:<line number>:<col number>:<new name>";
    "--identify-function",
      Arg.String (fun x -> set_mode (MODE_IDENTIFY_SYMBOL1 x) ()),
      " (mode) print the full function name at the position " ^
      "[line:character] of the text on stdin";
    "--ignore-hh-version",
      Arg.Set ignore_hh_version,
      " ignore hh_version check when loading saved states (default: false)";
    "--saved-state-ignore-hhconfig",
      Arg.Set saved_state_ignore_hhconfig,
      " ignore hhconfig hash when loading saved states (default: false)";
    "--in-memory-dep-table-size",
      Arg.Unit (set_mode MODE_IN_MEMORY_DEP_TABLE_SIZE),
      " number of entries in the in-memory dependency table";
    "--infer-return-type",
      Arg.String (fun s -> set_mode (MODE_INFER_RETURN_TYPE s) ()),
       " (mode) infers return type of given function or method\n";
    "--inheritance-ancestor-classes",
      Arg.String
      (fun x -> set_mode (MODE_METHOD_JUMP_ANCESTORS (x, "Class")) ()),
      " (mode) prints a list of classes that this class extends";
    "--inheritance-ancestor-classes-batch",
      Arg.Rest begin fun class_ ->
        mode := match !mode with
          | None -> Some (MODE_METHOD_JUMP_ANCESTORS_BATCH ([class_], "Class"))
          | Some (MODE_METHOD_JUMP_ANCESTORS_BATCH (classes, "Class")) ->
            Some (MODE_METHOD_JUMP_ANCESTORS_BATCH ((class_::classes, "Class")))
          | _ -> raise (Arg.Bad "only a single mode should be specified")
        end,
      " (mode) prints a list of classes that these classes extend";
    "--inheritance-ancestor-interfaces",
      Arg.String
      (fun x -> set_mode (MODE_METHOD_JUMP_ANCESTORS (x, "Interface")) ()),
      " (mode) prints a list of interfaces that this class implements";
    "--inheritance-ancestor-interfaces-batch",
      Arg.Rest begin fun class_ ->
        mode := match !mode with
          | None -> Some (MODE_METHOD_JUMP_ANCESTORS_BATCH ([class_], "Interface"))
          | Some (MODE_METHOD_JUMP_ANCESTORS_BATCH (classes, "Interface")) ->
            Some (MODE_METHOD_JUMP_ANCESTORS_BATCH ((class_::classes, "Interface")))
          | _ -> raise (Arg.Bad "only a single mode should be specified")
        end,
      " (mode) prints a list of interfaces that these classes implement";
    "--inheritance-ancestor-traits",
      Arg.String
      (fun x -> set_mode (MODE_METHOD_JUMP_ANCESTORS (x, "Trait")) ()),
      " (mode) prints a list of traits that this class uses";
    "--inheritance-ancestor-traits-batch",
      Arg.Rest begin fun class_ ->
        mode := match !mode with
          | None -> Some (MODE_METHOD_JUMP_ANCESTORS_BATCH ([class_], "Trait"))
          | Some (MODE_METHOD_JUMP_ANCESTORS_BATCH (classes, "Trait")) ->
            Some (MODE_METHOD_JUMP_ANCESTORS_BATCH ((class_::classes, "Trait")))
          | _ -> raise (Arg.Bad "only a single mode should be specified")
        end,
      " (mode) prints a list of traits that these classes use";
    "--inheritance-ancestors",
      Arg.String
      (fun x -> set_mode (MODE_METHOD_JUMP_ANCESTORS (x, "No_filter")) ()),
      " (mode) prints a list of all related classes or methods" ^
      " to the given class";
    "--inheritance-children",
      Arg.String (fun x -> set_mode (MODE_METHOD_JUMP_CHILDREN x) ()),
      " (mode) prints a list of all related classes or methods" ^
      " to the given class";
    "--json",
      Arg.Set output_json,
      " output json for machine consumption. (default: false)";
    "--lint",
    Arg.Unit (set_mode MODE_LINT),
      " (mode) lint the given list of files";
    "--lint-all",
      Arg.Int (fun x -> set_mode (MODE_LINT_ALL x) ()),
      " (mode) find all occurrences of lint with the given error code";
    "--lint-stdin",
      Arg.String (fun filename -> set_mode (MODE_LINT_STDIN filename) ()),
      " (mode) lint a file given on stdin; the filename should be the" ^
      " argument to this option";
    "--lint-xcontroller",
      Arg.String (fun filename -> set_mode (MODE_LINT_XCONTROLLER filename) ()), ""
      (* (mode) lint all xcontrollers in files listed in given file (i.e. the argument is
       * a path to a file that contains a list of files) *);
    "--list-files",
      Arg.Unit (set_mode MODE_LIST_FILES),
      " (mode) list files with errors";
    "--log-inference-constraints",
      Arg.Set log_inference_constraints,
      "  (for hh debugging purpose only) log type" ^
      " inference constraints into external logger (e.g. Scuba)";
    "--logname",
      Arg.Set logname,
      " (mode) show log filename and exit\n";
    "--monitor-logname",
      Arg.Set monitor_logname,
      " (mode) show monitor log filename and exit\n";
    "--no-load",
      Arg.Set no_load,
      " start from a fresh state";
    "--outline",
      Arg.Unit (set_mode MODE_OUTLINE),
      " (mode) prints an outline of the text on stdin";
    Common_argspecs.prechecked prechecked;
    Common_argspecs.no_prechecked prechecked;
    "--profile-log",
      Arg.Set profile_log,
      " enable profile logging";
    "--refactor", Arg.Tuple ([
        Arg.Symbol (
          ["Class"; "Function"; "Method"],
          (fun x -> refactor_mode := x));
        Arg.String (fun x -> refactor_before := x);
        Arg.String (fun x ->
          set_mode (MODE_REFACTOR (!refactor_mode, !refactor_before, x)) ())
      ]),
      " (mode) rename a symbol, Usage: --refactor " ^
      "[\"Class\", \"Function\", \"Method\"] <Current Name> <New Name>";
    "--remove-dead-fixme",
        Arg.Int begin fun code ->
        mode := match !mode with
          | None -> Some (MODE_REMOVE_DEAD_FIXMES [code])
          | Some (MODE_REMOVE_DEAD_FIXMES codel) ->
            Some (MODE_REMOVE_DEAD_FIXMES (code :: codel))
          | _ -> raise (Arg.Bad "only a single mode should be specified")
        end,
      " (mode) remove dead HH_FIXME for specified error code " ^
      "(first do hh_client restart --no-load)";
    "--remove-dead-fixmes",
        Arg.Unit (set_mode (MODE_REMOVE_DEAD_FIXMES [])),
      " (mode) remove dead HH_FIXME for any error code < 5000 " ^
      "(first do hh_client restart --no-load)";
    "--rewrite-lambda-parameters",
       Arg.Rest begin fun fn ->
         mode := match !mode with
          | None ->
            Some (MODE_REWRITE_LAMBDA_PARAMETERS [fn])
          | Some (MODE_REWRITE_LAMBDA_PARAMETERS fnl) ->
            Some (MODE_REWRITE_LAMBDA_PARAMETERS (fn :: fnl))
          | _ -> raise (Arg.Bad "only a single mode should be specified")
         end,
      " (mode) rewrite lambdas in the files from the given list" ^
      " with suggested parameter types";
    "--replace-state-after-saving",
      Arg.Set replace_state_after_saving,
      " if combined with --save-mini, causes the saved state" ^
      " to replace the program state; otherwise, the state files are not" ^
      " used after being written to disk (default: false)";
    (* Retrieve changed files since input checkpoint.
     * Output is separated by newline.
     * Exit code will be non-zero if no checkpoint is found *)
    "--retrieve-checkpoint",
      Arg.String (fun x -> set_mode (MODE_RETRIEVE_CHECKPOINT x) ()),
      "";
    "--retry-if-init",
      Arg.Bool (fun _ -> ()),
      " (deprecated and ignored)";
    Common_argspecs.retries retries;
    "--save-state",
      Arg.String (fun x -> set_mode (MODE_SAVE_STATE x) ()),
      (" (mode) Save a saved state to the given file." ^
      " Returns number of edges dumped from memory to the database.");
    "--search",
      Arg.String (fun x -> set_mode (MODE_SEARCH (x, "")) ()),
      " (mode) fuzzy search symbol definitions";
    "--search-class",
      Arg.String (fun x -> set_mode (MODE_SEARCH (x, "class")) ()),
      " (mode) fuzzy search class definitions";
    "--search-constant",
      Arg.String (fun x -> set_mode (MODE_SEARCH (x, "constant")) ()),
      " (mode) fuzzy search constant definitions";
    "--search-function",
      Arg.String (fun x -> set_mode (MODE_SEARCH (x, "function")) ()),
      " (mode) fuzzy search function definitions";
    "--search-typedef",
      Arg.String (fun x -> set_mode (MODE_SEARCH (x, "typedef")) ()),
      " (mode) fuzzy search typedef definitions";
    "--single",
      Arg.String (fun x -> set_mode (MODE_STATUS_SINGLE x) ()),
      "Return errors in file with provided name (give '-' for stdin)";
    "--sort-results",
      Arg.Set sort_results,
      " sort output for CST search.";
    "--stats",
      Arg.Unit (set_mode MODE_STATS),
      " display some server statistics";
    "--status",
      Arg.Unit (set_mode MODE_STATUS),
      " (mode) show a human readable list of errors (default)";
    "--timeout",
      Arg.Float (fun x -> timeout := Some (Unix.time() +. x)),
      " set the timeout in seconds (default: no timeout)";
    "--type-at-pos",
      Arg.String (fun x -> set_mode (MODE_TYPE_AT_POS x) ()),
      " (mode) show type at a given position in file [line:character]";
    "--type-at-pos-batch",
      Arg.Rest begin fun position ->
        mode := match !mode with
          | None -> Some (MODE_TYPE_AT_POS_BATCH [position])
          | Some (MODE_TYPE_AT_POS_BATCH positions) ->
            Some (MODE_TYPE_AT_POS_BATCH (position::positions))
          | _ -> raise (Arg.Bad "only a single mode should be specified")
        end,
      " (mode) show types at multiple positions [file:line:character list]";
    "--fun-deps-at-pos-batch",
      Arg.Rest begin fun position ->
        mode := match !mode with
          | None -> Some (MODE_FUN_DEPS_AT_POS_BATCH [position])
          | Some (MODE_FUN_DEPS_AT_POS_BATCH positions) ->
            Some (MODE_FUN_DEPS_AT_POS_BATCH (position::positions))
          | _ -> raise (Arg.Bad "only a single mode should be specified")
        end,
      " (mode) for each entry in input list get list of function dependencies \
        [file:line:character list]";
    "--fun-is-locallable-at-pos-batch",
      Arg.Rest begin fun position ->
        mode := match !mode with
          | None -> Some (MODE_FUN_IS_LOCALLABLE_AT_POS_BATCH [position])
          | Some (MODE_FUN_IS_LOCALLABLE_AT_POS_BATCH positions) ->
            Some (MODE_FUN_IS_LOCALLABLE_AT_POS_BATCH (position::positions))
          | _ -> raise (Arg.Bad "only a single mode should be specified")
        end,
      " (mode) for each entry in input list checks if function at position can be \
        made RxLocal [file:line:character list]";
    "--typed-full-fidelity-json",
      Arg.String (fun filename -> set_mode (MODE_TYPED_FULL_FIDELITY_PARSE filename) ()),
      " (mode) show full fidelity parse tree with types. Implies --json.";
    "--version",
      Arg.Set version,
      " (mode) show version and exit\n";
    Common_argspecs.watchman_debug_logging watchman_debug_logging;
  ] in
  let args = parse_without_command options usage "check" in

  if !version then begin
    if !output_json then ServerArgs.print_json_version ()
    else print_endline Build_id.build_id_ohai;
    exit 0;
  end;

  let mode = Option.value !mode ~default:MODE_STATUS in

  (* fixups *)
  let root, lint_paths =
    match mode, args with
    | MODE_LINT, _ -> ClientArgsUtils.get_root None, args
    | _, [] -> ClientArgsUtils.get_root None, []
    | _, [x] -> ClientArgsUtils.get_root (Some x), []
    | _, _ ->
           Printf.fprintf stderr
             "Error: please provide at most one www directory\n%!";
           exit 1
  in

  if !monitor_logname then begin
    let monitor_log_link = ServerFiles.monitor_log_link root in
    Printf.printf "%s\n%!" monitor_log_link;
    exit 0;
  end;

  if !logname then begin
    let log_link = ServerFiles.log_link root in
    Printf.printf "%s\n%!" log_link;
    exit 0;
  end;

  let () = if (!from) = "emacs" then
      Printf.fprintf stdout "-*- mode: compilation -*-\n%!"
  in
  CCheck {
    ai_mode = !ai_mode;
    autostart = !autostart;
    config = !config;
    dynamic_view = !dynamic_view;
    error_format = !error_format;
    file_info_on_disk = !file_info_on_disk;
    force_dormant_start = !force_dormant_start;
    from = !from;
    gen_saved_ignore_type_errors = !gen_saved_ignore_type_errors;
    ignore_hh_version = !ignore_hh_version;
    saved_state_ignore_hhconfig = !saved_state_ignore_hhconfig;
    lint_paths = lint_paths;
    log_inference_constraints = !log_inference_constraints;
    mode = mode;
    no_load = !no_load || (
      match mode with
      | MODE_REMOVE_DEAD_FIXMES _ -> true
      | _ -> false);
    output_json = !output_json;
    prechecked = !prechecked;
    profile_log = !profile_log;
    replace_state_after_saving = !replace_state_after_saving;
    retries = !retries;
    root = root;
    sort_results = !sort_results;
    timeout = !timeout;
    watchman_debug_logging = !watchman_debug_logging;
  }

let parse_start_env command =
  let usage =
    Printf.sprintf
      "Usage: %s %s [OPTION]... [WWW-ROOT]\n\
      %s a Hack server\n\n\
      WWW-ROOT is assumed to be current directory if unspecified\n"
      Sys.argv.(0) command (String.capitalize command) in
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
  let wait_deprecation_msg () = Printf.eprintf
    "WARNING: --wait is deprecated, does nothing, and will be going away \
     soon!\n%!" in
  let options = [
    "--wait", Arg.Unit wait_deprecation_msg,
    " this flag is deprecated and does nothing!";
    "--no-load", Arg.Set no_load,
    " start from a fresh state";
    Common_argspecs.watchman_debug_logging watchman_debug_logging;
    Common_argspecs.from from;
    "--profile-log", Arg.Set profile_log,
    " enable profile logging";
    "--log-inference-constraints", Arg.Set log_inference_constraints,
      "  (for hh debugging purpose only) log type" ^
      " inference constraints into external logger (e.g. Scuba)";
    "--ai", Arg.String (fun x -> ai_mode := Some x),
    "  run ai with options ";
    "--ignore-hh-version", Arg.Set ignore_hh_version,
      " ignore hh_version check when loading saved states (default: false)";
    "--saved-state-ignore-hhconfig", Arg.Set saved_state_ignore_hhconfig,
      " ignore hhconfig hash when loading saved states (default: false)";
    Common_argspecs.prechecked prechecked;
    Common_argspecs.no_prechecked prechecked;
    Common_argspecs.config config;
  ] in
  let args = parse_without_command options usage command in
  let root =
    match args with
    | [] -> ClientArgsUtils.get_root None
    | [x] -> ClientArgsUtils.get_root (Some x)
    | _ ->
        Printf.fprintf stderr
          "Error: please provide at most one www directory\n%!";
        exit 1 in
  { ClientStart.
    ai_mode = !ai_mode;
    config = !config;
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
    root = root;
    silent = false;
    watchman_debug_logging = !watchman_debug_logging;
  }

let parse_start_args () =
  CStart (parse_start_env "start")

let parse_restart_args () =
  CRestart (parse_start_env "restart")

let parse_stop_args () =
  let usage =
    Printf.sprintf
      "Usage: %s stop [OPTION]... [WWW-ROOT]\n\
      Stop a hack server\n\n\
      WWW-ROOT is assumed to be current directory if unspecified\n"
      Sys.argv.(0) in
  let from = ref "" in
  let options = [
    Common_argspecs.from from;
  ] in
  let args = parse_without_command options usage "stop" in
  let root =
    match args with
    | [] -> ClientArgsUtils.get_root None
    | [x] -> ClientArgsUtils.get_root (Some x)
    | _ ->
        Printf.fprintf stderr
          "Error: please provide at most one www directory\n%!";
        exit 1
  in CStop {ClientStop.root = root; from = !from;}

let parse_lsp_args () =
  let usage = Printf.sprintf
    "Usage: %s lsp [OPTION]...\n\
    [experimental] runs a persistent language service\n"
    Sys.argv.(0) in
  let from = ref "" in
  let use_ffp_autocomplete = ref false in
  let noop_enhanced_hover  = ref false in
  let options = [
    Common_argspecs.from from;
    "--ffp-autocomplete",
    Arg.Set use_ffp_autocomplete,
    " [experimental] (mode) use the full-fidelity parser based autocomplete ";
    "--enhanced-hover",
    Arg.Set noop_enhanced_hover,
    " [legacy] no-op";
  ] in
  let args = parse_without_command options usage "lsp" in
  match args with
  | [] -> CLsp {
      ClientLsp.from = !from;
      ClientLsp.use_ffp_autocomplete = !use_ffp_autocomplete;
    }
  | _ -> Printf.printf "%s\n" usage; exit 2

let parse_debug_args () =
  let usage =
    Printf.sprintf "Usage: %s debug [OPTION]... [WWW-ROOT]\n" Sys.argv.(0) in
  let from = ref "" in
  let options = [
    Common_argspecs.from from;
  ] in
  let args = parse_without_command options usage "debug" in
  let root =
    match args with
    | [] -> ClientArgsUtils.get_root None
    | [x] -> ClientArgsUtils.get_root (Some x)
    | _ -> Printf.printf "%s\n" usage; exit 2 in
  CDebug { ClientDebug.
    root;
    from = !from;
  }

let parse_args () =
  match parse_command () with
    | CKNone
    | CKCheck as cmd -> parse_check_args cmd
    | CKStart -> parse_start_args ()
    | CKStop -> parse_stop_args ()
    | CKRestart -> parse_restart_args ()
    | CKDebug -> parse_debug_args ()
    | CKLsp -> parse_lsp_args ()

let root = function
  | CCheck { ClientEnv.root; _ }
  | CStart { ClientStart.root; _ }
  | CRestart { ClientStart.root; _ }
  | CStop { ClientStop.root; _ }
  | CDebug { ClientDebug.root; _ } -> root
  | CLsp _ -> Path.dummy_path
