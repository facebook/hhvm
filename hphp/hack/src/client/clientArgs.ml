(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open ClientCommand
open ClientEnv

let rec guess_root start recursion_limit : Path.path option =
  let fs_root = Path.mk_path "/" in
  if Path.equal start fs_root then None
  else if Wwwroot.is_www_directory start then Some start
  else if recursion_limit <= 0 then None
  else guess_root (Path.parent start) (recursion_limit - 1)

let parse_command () =
  if Array.length Sys.argv < 2
  then CKNone
  else match String.lowercase Sys.argv.(1) with
  | "check" -> CKCheck
  | "start" -> CKStart
  | "stop" -> CKStop
  | "restart" -> CKRestart
  | "status" -> CKStatus
  | "build" -> CKBuild
  | "prolog" -> CKProlog
  | _ -> CKNone

let parse_without_command options usage command =
  let args = ref [] in
  Arg.parse (Arg.align options) (fun x -> args := x::!args) usage;
  match List.rev !args with
  | x::rest when x = command -> rest
  | args -> args

let get_root path_opt =
  let root =
    match path_opt with
    | None ->
      (match guess_root (Path.mk_path ".") 50 with
      | Some path -> path
      | None ->
        Printf.fprintf stderr
        "Error: not a www tree (or any of the parent directories): %s\n"
        (Path.string_of_path (Path.mk_path "."));
        exit 1;)
    | Some p -> Path.mk_path p
  in Wwwroot.assert_www_directory root;
  root

let parse_check_args cmd =
  (* arg parse output refs *)
  let mode = ref MODE_UNSPECIFIED in
  let retries = ref 3 in
  let output_json = ref false in
  let retry_if_init = ref true in
  let timeout = ref None in
  let autostart = ref true in
  let from = ref "" in

  (* custom behaviors *)
  let set_from x () = from := x in
  let set_mode x () =
    if !mode <> MODE_UNSPECIFIED
    then raise (Arg.Bad "only a single mode should be specified")
    else mode := x
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
        \tstatus\
          \t\tLists running Hack servers\n\
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
    "--status", Arg.Unit (set_mode MODE_STATUS),
      " (mode) show a human readable list of errors (default)";
    "--types", Arg.String (fun x -> set_mode (MODE_SHOW_TYPES x) ()),
      " (mode) show the types for file specified";
    "--type-at-pos", Arg.String (fun x -> set_mode (MODE_TYPE_AT_POS x) ()),
      " (mode) show type at a given position in file [filename:line:character]";
    "--skip", Arg.Unit (set_mode MODE_SKIP),
      " (mode) unsafe! ignore any existing errors";
    "--list-files", Arg.Unit (set_mode MODE_LIST_FILES),
      " (mode) list files with errors";
    "--auto-complete", Arg.Unit (set_mode MODE_AUTO_COMPLETE),
      " (mode) auto-completes the text on stdin";
    "--color", Arg.String (fun x -> set_mode (MODE_COLORING x) ()),
      " (mode) pretty prints the file content showing what is checked";
    "--find-refs", Arg.String (fun x -> set_mode (MODE_FIND_REFS x) ()),
      " (mode) finds references of the provided method name";
    "--find-class-refs", Arg.String (fun x -> set_mode (MODE_FIND_CLASS_REFS x) ()),
      " (mode) finds references of the provided class name";
    "--identify-function", Arg.String (fun x -> set_mode (MODE_IDENTIFY_FUNCTION x) ()),
      " (mode) print the full function name at the position [line:character] of the text on stdin";
    "--outline", Arg.Unit (set_mode MODE_OUTLINE),
      " (mode) prints an outline of the text on stdin";
    "--version", Arg.Unit (set_mode MODE_VERSION),
      " (mode) show version and exit\n";

    (* flags *)
    "--json", Arg.Set output_json,
      " output json for machine consumption. (default: false)";
    "--retries", Arg.Set_int retries,
      " set the number of retries. (default: 3)";
    "--retry-if-init", Arg.Bool (fun x -> retry_if_init := x),
      " retry if the server is initializing (default: true)";
    "--from", Arg.Set_string from,
      " set this so we know who is calling hh_client";
    "--timeout",  Arg.Float (fun x -> timeout := Some (Unix.time() +. x)),
      " set the timeout in seconds (default: no timeout)";
    "--autostart-server", Arg.Bool (fun x -> autostart := x),
      " automatically start hh_server if it's not running (default: true)\n";

    (* deprecated *)
    "--from-vim", Arg.Unit (fun () -> from := "vim"; retries := 0; retry_if_init := false),
      " (deprecated) equivalent to --from vim --retries 0 --retry-if-init false";
    "--from-emacs", Arg.Unit (set_from "emacs"),
      " (deprecated) equivalent to --from emacs";
    "--from-arc-diff", Arg.Unit (set_from "arc_diff"),
      " (deprecated) equivalent to --from arc_diff";
    "--from-arc-land", Arg.Unit (set_from "arc_land"),
      " (deprecated) equivalent to --from arc_land";
    "--from-check-trunk", Arg.Unit (set_from "check_trunk"),
      " (deprecated) equivalent to --from check_trunk";
    "--save-state", Arg.String (fun x -> set_mode (MODE_SAVE_STATE x) ()),
      " <file> debug mode (do not use)";
    "--show", Arg.String (fun x -> set_mode (MODE_SHOW x) ()),
      " debug mode (do not use)";
  ] in
  let args = parse_without_command options usage "check" in

  (* fixups *)
  if !mode == MODE_UNSPECIFIED then mode := MODE_STATUS;
  let root =
    match args with
    | [] -> get_root None
    | [x] -> get_root (Some x)
    | _ ->
        Printf.fprintf stderr "Error: please provide at most one www directory\n%!";
        exit 1;
  in
  let () = if (!from) = "emacs" then
      Printf.fprintf stdout "-*- mode: compilation -*-\n%!"
  in
  CCheck {
    mode = !mode;
    root = root;
    from = !from;
    output_json = !output_json;
    retry_if_init = !retry_if_init;
    retries = !retries;
    timeout = !timeout;
    autostart = !autostart;
  }

let parse_start_args () =
  let wait = ref false in
  let usage =
    Printf.sprintf
      "Usage: %s start [OPTION]... [WWW-ROOT]\n\
      Start a Hack server\n\n\
      WWW-ROOT is assumed to be current directory if unspecified\n"
      Sys.argv.(0) in
  let options = [
    "--wait", Arg.Unit (fun () -> wait := true ),
    " wait for the server to finish initializing"
  ] in
  let args = parse_without_command options usage "start" in
  let root =
    match args with
    | [] -> get_root None
    | [x] -> get_root (Some x)
    | _ ->
        Printf.fprintf stderr "Error: please provide at most one www directory\n%!";
        exit 1
  in CStart {ClientStart.root = root; ClientStart.wait = !wait}

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
    | [] -> get_root None
    | [x] -> get_root (Some x)
    | _ ->
        Printf.fprintf stderr "Error: please provide at most one www directory\n%!";
        exit 1
  in CStop {ClientStop.root = root}

let parse_restart_args () =
  let usage =
    Printf.sprintf
      "Usage: %s restart [OPTION]... [WWW-ROOT]\n\
      Restart a hack server\n\n\
      WWW-ROOT is assumed to be current directory if unspecified\n"
      Sys.argv.(0) in
  let wait = ref false in
  let options = [
    "--wait", Arg.Unit (fun () -> wait := true ),
    " wait for the new server to finish initializing"
  ] in
  let args = parse_without_command options usage "restart" in
  let root =
    match args with
    | [] -> get_root None
    | [x] -> get_root (Some x)
    | _ ->
        Printf.fprintf stderr "Error: please provide at most one www directory\n%!";
        exit 1
  in CRestart {ClientRestart.root = root; ClientRestart.wait = !wait;}

let parse_status_args () =
  let usage =
    Printf.sprintf
      "Usage: %s status [OPTION]...\n\
      List running servers\n"
      Sys.argv.(0) in
  let root = ref None in
  let user = ref None in
  let options = [
    "--root", Arg.String (fun x -> root := Some (Path.mk_path x)),
    " --root /some/path/www only shows servers for /some/path/www";
    "--user", Arg.String (fun x -> user := Some x),
    " --user billy only shows servers for the user \"billy\"";
  ] in
  let _ = parse_without_command options usage "status" in
  CStatus {ClientStatus.root = !root; ClientStatus.user = !user }

let parse_build_args () =
  let usage =
    Printf.sprintf
      "Usage: %s build [WWW-ROOT]\n\
      Generates build files\n"
      Sys.argv.(0) in
  let steps = ref None in
  let verbose = ref false in
  let serial = ref false in
  let test_dir = ref None in
  let grade = ref true in
  let list_classes = ref false in
  (* todo: for now better to default to true here, but this is temporary! *)
  let clean = ref true in
  let run_scripts = ref true in
  let options = [
    "--steps", Arg.String (fun x ->
      steps := Some (Str.split (Str.regexp ",") x)),
    " comma-separated list of build steps to run";
    "--serial", Arg.Set serial,
    " run without parallel worker processes";
    "--test-dir", Arg.String (fun x -> test_dir := Some x),
    " <dir> generates into <dir> and compares with root";
    "--no-grade", Arg.Clear grade,
    " skip full comparison with root";
    "--list-classes", Arg.Set list_classes,
    " generate files listing subclasses used in analysis";
    "--clean", Arg.Set clean,
    " erase all previously generated files before building";
    "--no-clean", Arg.Clear clean,
    " guess what";
    "--no-run-scripts", Arg.Clear run_scripts,
    " don't run unported arc build scripts";
    "--verbose", Arg.Set verbose,
    " guess what";
  ] in
  let args = parse_without_command options usage "build" in
  let root =
    match args with
    | [x] -> get_root (Some x)
    | _ -> Printf.printf "%s\n" usage; exit 2
  in
  CBuild { ServerMsg.
           root = root;
           steps = !steps;
           serial = !serial;
           test_dir = !test_dir;
           grade = !grade;
           list_classes = !list_classes;
           clean = !clean;
           run_scripts = !run_scripts;
           verbose = !verbose;
         }

let parse_prolog_args () =
  let usage =
    Printf.sprintf
      "Usage: %s prolog [WWW-ROOT]\n\
      run prolog interpreter on code database\n"
      Sys.argv.(0) in
  let options = [
  ] in
  let args = parse_without_command options usage "prolog" in
  let root =
    match args with
    | [x] -> get_root (Some x)
    | _ -> Printf.printf "%s\n" usage; exit 2
  in
  CProlog { ClientProlog. 
           root;
         }


let parse_args () =
  match parse_command () with
    | CKNone
    | CKCheck as cmd -> parse_check_args cmd
    | CKStart -> parse_start_args ()
    | CKStop -> parse_stop_args ()
    | CKRestart -> parse_restart_args ()
    | CKStatus -> parse_status_args ()
    | CKBuild -> parse_build_args ()
    | CKProlog -> parse_prolog_args ()
