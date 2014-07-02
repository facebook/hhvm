(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(*****************************************************************************)
(* Debugging sections.
 *
 * This is useful when we modify the formatter and we want to make sure
 * we didn't introduce errors.
 * You can run 'hh_format --debug directory_name' and it will output all
 * the errors.
 *
 * It takes all the files in a directory and verifies that:
 * -) The result parses
 * -) formatting is idempotent (for each file)
 *
 *)
(*****************************************************************************)
exception Format_error

let debug () fnl =
  List.fold_left begin fun () filename ->
    try
      Pos.file := filename;
      let content = Utils.cat filename in

      (* Checking that we can parse the output *)
      let parsing_errors1, parser_output1 = Errors.do_ begin fun () ->
        Parser_hack.program content
      end in
      if not parser_output1.Parser_hack.is_hh_file || parsing_errors1 <> []
      then raise Exit;

      if parsing_errors1 <> []
      then begin
        Printf.printf
          "The file had a syntax error before we even started: %s\n"
          filename;
        flush stdout
      end;

      let content = Format_hack.program content in
      let content =
        match content with
        | Format_hack.Success content -> content
        | Format_hack.Php_or_decl ->
            raise Exit
        | Format_hack.Parsing_error _ ->
            Printf.printf "Parsing: %s\n" filename; flush stdout;
            ""
        | Format_hack.Internal_error ->
            Printf.printf "Internal: %s\n" filename; flush stdout;
            ""
      in

      (* Checking for idempotence *)
      let content2 = Format_hack.program content in
      let content2 =
        match content2 with
        | Format_hack.Success content2 -> content2
        | _ -> raise Format_error
      in
      if content <> content2
      then begin
        Printf.printf
          "Applying the formatter twice lead to different results: %s\n"
          filename; flush stdout;
        let () = Random.self_init() in
        let nbr = string_of_int (Random.int 100000) in
        let tmp = "/tmp/xx_"^nbr in
        let file1 = tmp^"_1.php" in
        let file2 = tmp^"_2.php" in
        let oc = open_out file1 in
        output_string oc content;
        close_out oc;
        let oc = open_out file2 in
        output_string oc content2;
        close_out oc;
        let _ = Sys.command ("diff "^file1^" "^file2) in
        let _ = Sys.command ("rm "^file1^" "^file2) in
        flush stdout
      end;

      (* Checking that we can parse the output *)
      let parsing_errors2, _parser_output2 = Errors.do_ begin fun () ->
        Parser_hack.program content
      end in
      if parsing_errors2 <> []
      then begin
        Printf.printf
          "The output of the formatter could not be parsed: %s\n"
          filename;
        flush stdout
      end;

      ()
    with
    | Format_error ->
        Printf.printf "Format error: %s\n" filename;
        flush stdout
    | Exit ->
        ()
  end () fnl

let debug_directory dir =
  let path = Path.mk_path dir in
  let next = Find.make_next_files_php path in
  let workers = Worker.make ServerConfig.nbr_procs in
  MultiWorker.call
    (Some workers)
    ~job:debug
    ~neutral:()
    ~merge:(fun () () -> ())
    ~next

(*****************************************************************************)
(* Parsing the command line *)
(*****************************************************************************)

let parse_args() =
  let from = ref 0 in
  let to_ = ref max_int in
  let files = ref [] in
  let in_place = ref false in
  let debug = ref false in
  Arg.parse
    [
     "--from", Arg.Int (fun x -> from := x),
     "[int] start after character position";

     "--to", Arg.Int (fun x -> to_ := x),
     "[int] stop after character position";

     "-i", Arg.Unit (fun () -> in_place := true),
     "modify the files in place";

     "--in-place", Arg.Unit (fun () -> in_place := true),
     "modify the files in place";

     "--debug", Arg.Unit (fun () -> debug := true), "";
   ]
    (fun file -> files := file :: !files)
    (Printf.sprintf "Usage: %s (filename|directory)" Sys.argv.(0));
  !files, !from, !to_, !in_place, !debug

(*****************************************************************************)
(* Formats a file in place *)
(*****************************************************************************)

let format_in_place filename =
  Pos.file := filename;
  match Format_hack.program (Utils.cat filename) with
  | Format_hack.Success result ->
      let oc = open_out filename in
      output_string oc result;
      close_out oc;
      None
  | Format_hack.Internal_error ->
      Some "Internal error\n"
  | Format_hack.Parsing_error errorl ->
      Some (Errors.to_string (List.hd errorl))
  | Format_hack.Php_or_decl ->
      None

(*****************************************************************************)
(* Formats all the hack files in a directory (in place) *)
(*****************************************************************************)

let job_in_place acc fnl =
  List.fold_left begin fun acc filename ->
    match format_in_place filename with
    | None -> acc
    | Some err -> err :: acc
  end acc fnl

let directory dir =
  let path = Path.mk_path dir in
  let next = Find.make_next_files_php path in
  let workers = Worker.make ServerConfig.nbr_procs in
  let messages =
    MultiWorker.call
      (Some workers)
      ~job:job_in_place
      ~neutral:[]
      ~merge:List.rev_append
      ~next
  in
  List.iter (Printf.fprintf stderr "%s\n") messages

(*****************************************************************************)
(* Applies the formatter directly to a string. *)
(*****************************************************************************)

let format_string from to_ content =
  match Format_hack.region from to_ content with
  | Format_hack.Success content ->
      output_string stdout content
  | Format_hack.Internal_error ->
      Printf.fprintf stderr "Internal error\n";
      exit 2
  | Format_hack.Parsing_error error ->
      Printf.fprintf stderr "Parsing error\n%s\n"
        (Errors.to_string (List.hd error));
      exit 2
  | Format_hack.Php_or_decl ->
      exit 0

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let format_stdin from to_ =
  let buf = Buffer.create 256 in
  (try
    while true do
      Buffer.add_string buf (read_line());
      Buffer.add_char buf '\n';
    done;
    assert false
  with End_of_file ->
    let content = Buffer.contents buf in
    format_string from to_ content
  )

(*****************************************************************************)
(* The main entry point. *)
(*****************************************************************************)

let () =
  PidLog.log_oc := Some (open_out "/dev/null");
  let files, from, to_, in_place, debug = parse_args() in
  match files with
  | [] when in_place ->
      Printf.fprintf stderr "Cannot modify stdin in-place\n";
      exit 2
  | [] -> format_stdin from to_
  | [dir] when Sys.is_directory dir ->
      if debug
      then debug_directory dir
      else directory dir
  | [filename] ->
      Pos.file := filename;
      if in_place
      then
        match format_in_place filename with
        | None -> ()
        | Some error ->
            Printf.fprintf stderr "Error: %s\n" error;
            exit 2
      else format_string from to_ (Utils.cat filename)
  | _ ->
      Printf.fprintf stderr "More than one file given\n";
      exit 2
