(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
open ClientCommand
open ClientEnv
open Utils

let parse_command () =
  if Array.length Sys.argv < 2
  then CKNone
  else match String.lowercase_ascii Sys.argv.(1) with
  | "check" -> CKCheck
  | "start" -> CKStart
  | "stop" -> CKStop
  | "restart" -> CKRestart
  | "build" -> CKBuild
  | "ide" -> CKIde
  | "debug" -> CKDebug
  | _ -> CKNone

let parse_without_command options usage command =
  let args = ref [] in
  Arg.parse (Arg.align options) (fun x -> args := x::!args) usage;
  match List.rev !args with
  | x::rest when (String.lowercase_ascii x) = (String.lowercase_ascii command)
    -> rest
  | args -> args

(* *** *** NB *** *** ***
 * Commonly-used options are documented in hphp/hack/man/hh_client.1 --
 * if you are making significant changes you need to update the manpage as
 * well. Experimental or otherwise volatile options need not be documented
 * there, but keep what's there up to date please. *)
let parse_check_args cmd =
  (* arg parse output refs *)
  let mode = ref None in
  let retries = ref 800 in
  let output_json = ref false in
  let retry_if_init = ref true in
  let no_load = ref false in
  let timeout = ref None in
  let autostart = ref true in
  let from = ref "" in
  let version = ref false in
  let monitor_logname = ref false in
  let logname = ref false in
  let refactor_mode = ref "" in
  let refactor_before = ref "" in
  let format_from = ref 0 in
  let ai_mode = ref None in

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
    (* modes *)
    "--status",
      Arg.Unit (set_mode MODE_STATUS),
      " (mode) show a human readable list of errors (default)";
    "--type-at-pos",
      Arg.String (fun x -> set_mode (MODE_TYPE_AT_POS x) ()),
      " (mode) show type at a given position in file [line:character]";
    "--list-files",
      Arg.Unit (set_mode MODE_LIST_FILES),
      " (mode) list files with errors";
    "--list-modes",
      Arg.Unit (set_mode MODE_LIST_MODES),
      " (mode) list all files with their associated hack modes";
    "--auto-complete",
      Arg.Unit (set_mode MODE_AUTO_COMPLETE),
      " (mode) auto-completes the text on stdin";
    "--colour",
      Arg.String (fun x -> set_mode (MODE_COLORING x) ()), " ";
    "--color",
      Arg.String (fun x -> set_mode (MODE_COLORING x) ()),
      " (mode) pretty prints the file content \
       showing what is checked (give '-' for stdin)";
    "--coverage",
      Arg.String (fun x -> set_mode (MODE_COVERAGE x) ()),
      " (mode) calculates the extent of typing of a given file or directory";
    "--find-dependent-files",
      Arg.String (fun x -> set_mode (MODE_FIND_DEPENDENT_FILES x) ()),
      " (mode) list all files that make any use of the provided list of files";
    "--find-refs",
      Arg.String (fun x -> set_mode (MODE_FIND_REFS x) ()),
      " (mode) finds references of the provided method name";
    "--trace_ai",
      Arg.String (fun x -> set_mode (MODE_TRACE_AI x) ()),
       "";
    "--find-class-refs",
      Arg.String (fun x -> set_mode (MODE_FIND_CLASS_REFS x) ()),
      " (mode) finds references of the provided class name";
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
    "--dump-ai-info",
      Arg.String (fun files -> set_mode (MODE_DUMP_AI_INFO files) ()),
        (* Just like --dump-symbol-info, but uses the AI to obtain info *)
        "";
    "--identify-function",
      Arg.String (fun x -> set_mode (MODE_IDENTIFY_SYMBOL1 x) ()),
      " (mode) print the full function name at the position " ^
      "[line:character] of the text on stdin";
    "--ide-get-definition",
      Arg.String (fun x -> set_mode (MODE_IDENTIFY_SYMBOL2 x) ()),
      (* alias for --identify-function *) "";
    "--get-method-name",
      Arg.String (fun x -> set_mode (MODE_IDENTIFY_SYMBOL3 x) ()),
      (* alias for --identify-function *) "";
    "--get-definition-by-id",
      Arg.String (fun x -> set_mode (MODE_GET_DEFINITION_BY_ID x) ()),
      "";
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
    "--search",
      Arg.String (fun x -> set_mode (MODE_SEARCH (x, "")) ()),
      " (mode) fuzzy search symbol definitions";
    "--search-class",
      Arg.String (fun x -> set_mode (MODE_SEARCH (x, "class")) ()),
      " (mode) fuzzy search class definitions";
    "--search-function",
      Arg.String (fun x -> set_mode (MODE_SEARCH (x, "function")) ()),
      " (mode) fuzzy search function definitions";
    "--search-typedef",
      Arg.String (fun x -> set_mode (MODE_SEARCH (x, "typedef")) ()),
      " (mode) fuzzy search typedef definitions";
    "--search-constant",
      Arg.String (fun x -> set_mode (MODE_SEARCH (x, "constant")) ()),
      " (mode) fuzzy search constant definitions";
    "--outline",
      Arg.Unit (set_mode MODE_OUTLINE),
      " (mode) prints an outline of the text on stdin";
    "--ide-outline",
      Arg.Unit (set_mode (MODE_OUTLINE2)), "";
    "--inheritance-children",
      Arg.String (fun x -> set_mode (MODE_METHOD_JUMP_CHILDREN x) ()),
      " (mode) prints a list of all related classes or methods \
       to the given class";
    "--inheritance-ancestors",
      Arg.String (fun x -> set_mode (MODE_METHOD_JUMP_ANCESTORS x) ()),
      " (mode) prints a list of all related classes or methods \
       to the given class";
    "--show",
      Arg.String (fun x -> set_mode (MODE_SHOW x) ()),
      " (mode) show human-readable type info for the given name; \
       output is not meant for machine parsing";
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
    "--ignore-fixme",
        Arg.Rest begin fun x ->
          mode := match !mode with
            | None -> Some (MODE_IGNORE_FIXMES [x])
            | Some (MODE_IGNORE_FIXMES xs) -> Some (MODE_IGNORE_FIXMES (x::xs))
            | _ -> raise (Arg.Bad "only a single mode should be specified")
          end,
        " (mode) ignores fixmes in the given space separated files " ^
        "(provide relative path inside code's root directory)";
    "--lint", Arg.Rest begin fun fn ->
        mode := match !mode with
          | None -> Some (MODE_LINT [fn])
          | Some (MODE_LINT fnl) -> Some (MODE_LINT (fn :: fnl))
          | _ -> raise (Arg.Bad "only a single mode should be specified")
      end,
      " (mode) lint the given list of files";
    "--lint-all",
      Arg.Int (fun x -> set_mode (MODE_LINT_ALL x) ()),
      " (mode) find all occurrences of lint with the given error code";
    "--version",
      Arg.Set version,
      " (mode) show version and exit\n";
    "--monitor-logname",
      Arg.Set monitor_logname,
      " (mode) show monitor log filename and exit\n";
    "--logname",
      Arg.Set logname,
      " (mode) show log filename and exit\n";
    (* Create a checkpoint which can be used to retrieve changed files later *)
    "--create-checkpoint",
      Arg.String (fun x -> set_mode (MODE_CREATE_CHECKPOINT x) ()),
      "";
    (* Retrieve changed files since input checkpoint.
     * Output is separated by newline.
     * Exit code will be non-zero if no checkpoint is found *)
    "--retrieve-checkpoint",
      Arg.String (fun x -> set_mode (MODE_RETRIEVE_CHECKPOINT x) ()),
      "";
    (* Delete an existing checkpoint.
     * Exitcode will be non-zero if no checkpoint is found *)
    "--delete-checkpoint",
      Arg.String (fun x -> set_mode (MODE_DELETE_CHECKPOINT x) ()),
      "";
    "--stats",
      Arg.Unit (set_mode MODE_STATS),
      " display some server statistics";
    "--format",
      Arg.Tuple ([
        Arg.Int (fun x -> format_from := x);
        Arg.Int (fun x -> set_mode (MODE_FORMAT (!format_from, x)) ())
      ]), "";
    "--ide-find-refs",
      Arg.String (fun x -> set_mode (MODE_IDE_FIND_REFS x) ()), "";
    "--ide-highlight-refs",
      Arg.String (fun x -> set_mode (MODE_IDE_HIGHLIGHT_REFS x) ()),
    (* Similar to --ide-find-refs, but returns references in current file only,
     * and is optimized to be faster in that case *) "";
    "--ai-query", Arg.String (fun x -> set_mode (MODE_AI_QUERY x) ()),
    (* Send an AI query *) "";
    "--dump-full-fidelity-parse",
        Arg.String (fun x -> set_mode (MODE_FULL_FIDELITY_PARSE x) ()),
        "";
    "--full-fidelity-schema",
      Arg.Unit (set_mode MODE_FULL_FIDELITY_SCHEMA), "";
    (* flags *)
    "--json",
      Arg.Set output_json,
      " output json for machine consumption. (default: false)";
    "--retries",
      Arg.Set_int retries,
      spf " set the number of retries. (default: %d)" !retries;
    "--retry-if-init",
      Arg.Bool (fun x -> retry_if_init := x),
      " retry if the server is initializing (default: true)";
    "--no-load",
      Arg.Set no_load,
      " start from a fresh state";
    "--from",
      Arg.Set_string from,
      " set this so we know who is calling hh_client";
    "--timeout",
      Arg.Float (fun x -> timeout := Some (Unix.time() +. x)),
      " set the timeout in seconds (default: no timeout)";
    "--autostart-server",
      Arg.Bool (fun x -> autostart := x),
      " automatically start hh_server if it's not running (default: true)";
    "--ai",
      Arg.String (fun s -> ai_mode :=
         Some (ignore (Ai_options.prepare ~server:true s); s)),
      " run AI module with provided options\n";

    (* deprecated *)
    "--from-vim",
      Arg.Unit (fun () -> from := "vim"; retries := 0; retry_if_init := false),
      " (deprecated) equivalent to \
       --from vim --retries 0 --retry-if-init false";
    "--from-emacs", Arg.Unit (set_from "emacs"),
      " (deprecated) equivalent to --from emacs";
    "--from-arc-diff", Arg.Unit (set_from "arc_diff"),
      " (deprecated) equivalent to --from arc_diff";
    "--from-arc-land", Arg.Unit (set_from "arc_land"),
      " (deprecated) equivalent to --from arc_land";
    "--from-check-trunk", Arg.Unit (set_from "check_trunk"),
      " (deprecated) equivalent to --from check_trunk";
  ] in
  let args = parse_without_command options usage "check" in

  if !version then begin
    if !output_json then ServerArgs.print_json_version ()
    else print_endline Build_id.build_id_ohai;
    exit 0;
  end;

  (* fixups *)
  let root =
    match args with
    | [] -> ClientArgsUtils.get_root None
    | [x] -> ClientArgsUtils.get_root (Some x)
    | _ ->
        Printf.fprintf stderr
          "Error: please provide at most one www directory\n%!";
        exit 1;
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
    mode = Option.value !mode ~default:MODE_STATUS;
    root = root;
    from = !from;
    output_json = !output_json;
    retry_if_init = !retry_if_init;
    retries = !retries;
    timeout = !timeout;
    autostart = !autostart;
    no_load = !no_load;
    ai_mode = !ai_mode;
  }

let parse_start_env command =
  let usage =
    Printf.sprintf
      "Usage: %s %s [OPTION]... [WWW-ROOT]\n\
      %s a Hack server\n\n\
      WWW-ROOT is assumed to be current directory if unspecified\n"
      Sys.argv.(0) command (String.capitalize_ascii command) in
  let no_load = ref false in
  let ai_mode = ref None in
  let wait_deprecation_msg () = Printf.eprintf
    "WARNING: --wait is deprecated, does nothing, and will be going away \
     soon!\n%!" in
  let options = [
    "--wait", Arg.Unit wait_deprecation_msg,
    " this flag is deprecated and does nothing!";
    "--no-load", Arg.Set no_load,
    " start from a fresh state";
    "--ai", Arg.String (fun x -> ai_mode := Some x),
    "  run ai with options ";
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
    root = root;
    no_load = !no_load;
    ai_mode = !ai_mode;
    silent = false;
    debug_port = None;
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
  let options = [] in
  let args = parse_without_command options usage "stop" in
  let root =
    match args with
    | [] -> ClientArgsUtils.get_root None
    | [x] -> ClientArgsUtils.get_root (Some x)
    | _ ->
        Printf.fprintf stderr
          "Error: please provide at most one www directory\n%!";
        exit 1
  in CStop {ClientStop.root = root}

let parse_build_args () =
  let usage =
    Printf.sprintf
      "Usage: %s build [WWW-ROOT]\n\
      Generates build files\n"
      Sys.argv.(0) in
  let steps = ref None in
  let ignore_killswitch = ref false in
  let no_steps = ref None in
  let use_factsdb_static = ref false in
  let verbose = ref false in
  let serial = ref false in
  let test_dir = ref None in
  let grade = ref true in
  let check = ref false in
  let is_push = ref false in
  let clean = ref false in
  (* todo: for now better to default to true here, but this is temporary! *)
  let clean_before_build = ref true in
  let run_scripts = ref true in
  let wait = ref false in
  let options = [
    "--steps", Arg.String (fun x ->
      steps := Some (Str.split (Str.regexp ",") x)),
    " comma-separated list of build steps to run";
    "--ignore-killswitch", Arg.Set ignore_killswitch,
    " run all steps (including kill-switched ones) except steps in --no-steps";
    "--no-steps", Arg.String (fun x ->
      no_steps := Some (Str.split (Str.regexp ",") x)),
    " comma-separated list of build steps not to run";
    "--use-factsdb-static", Arg.Set use_factsdb_static,
    " build autoload-map and arc-facts using FactsDB";
    "--no-run-scripts", Arg.Clear run_scripts,
    " don't run unported arc build scripts";
    "--serial", Arg.Set serial,
    " run without parallel worker processes";
    "--test-dir", Arg.String (fun x -> test_dir := Some x),
    " <dir> generates into <dir> and compares with root";
    "--no-grade", Arg.Clear grade,
    " skip full comparison with root";
    "--check", Arg.Set check,
    " run some sanity checks on the server state";
    "--push", Arg.Set is_push,
    " run steps appropriate for push build";
    "--clean", Arg.Set clean,
    " erase all previously generated files";
    "--clean-before-build", Arg.Set clean_before_build,
    " erase previously generated files before building (default)";
    "--no-clean-before-build", Arg.Clear clean_before_build,
    " do not erase previously generated files before building";
    "--wait", Arg.Set wait,
    " wait forever for hh_server intialization (default: false)";
    "--verbose", Arg.Set verbose,
    " guess what";
  ] in
  let args = parse_without_command options usage "build" in
  let root =
    match args with
    | [x] -> ClientArgsUtils.get_root (Some x)
    | _ -> Printf.printf "%s\n" usage; exit 2
  in
  CBuild { ClientBuild.
    root = root;
    wait = !wait;
    build_opts = { ServerBuild.
      steps = !steps;
      ignore_killswitch = !ignore_killswitch;
      no_steps = !no_steps;
      use_factsdb_static = !use_factsdb_static;
      run_scripts = !run_scripts;
      serial = !serial;
      test_dir = !test_dir;
      grade = !grade;
      is_push = !is_push;
      clean = !clean;
      clean_before_build = !clean_before_build;
      check = !check;
      user = Sys_utils.logname ();
      verbose = !verbose;
      id = Random_id.short_string ();
    }
  }

let parse_ide_args () =
  let usage =
    Printf.sprintf
      "Usage: %s ide [WWW-ROOT]\n"
      Sys.argv.(0) in

  let options = [] in
  let args = parse_without_command options usage "ide" in
  let root =
    match args with
    | [] -> ClientArgsUtils.get_root None
    | [x] -> ClientArgsUtils.get_root (Some x)
    | _ -> Printf.printf "%s\n" usage; exit 2 in
  CIde { ClientIde.
    root = root
  }

let parse_debug_args () =
  let usage =
    Printf.sprintf "Usage: %s debug [WWW-ROOT]\n" Sys.argv.(0) in
  let options = [] in
  let args = parse_without_command options usage "debug" in
  let root =
    match args with
    | [] -> ClientArgsUtils.get_root None
    | [x] -> ClientArgsUtils.get_root (Some x)
    | _ -> Printf.printf "%s\n" usage; exit 2 in
  CDebug { ClientDebug.
    root
  }

let parse_args () =
  match parse_command () with
    | CKNone
    | CKCheck as cmd -> parse_check_args cmd
    | CKStart -> parse_start_args ()
    | CKStop -> parse_stop_args ()
    | CKRestart -> parse_restart_args ()
    | CKBuild -> parse_build_args ()
    | CKDebug -> parse_debug_args ()
    | CKIde -> parse_ide_args ()

let root = function
  | CBuild { ClientBuild.root; _ }
  | CCheck { ClientEnv.root; _ }
  | CStart { ClientStart.root; _ }
  | CRestart { ClientStart.root; _ }
  | CStop { ClientStop.root; _ }
  | CIde { ClientIde.root; _}
  | CDebug { ClientDebug.root } -> root
