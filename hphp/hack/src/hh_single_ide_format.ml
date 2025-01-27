open Hh_prelude

let usage =
  {|
For lightweight testing of the formatting logic in **the language server**
and verifying that we get the same results for formatting a file:
- with the language server
- with hackfmt directly

Usage: hh_single_ide_format $FILENAME

Used for testing:
- Logic in the language server: this is where we convert hackfmt results
to minimal edits to send to the language client (such as VSCode)

Tests for any other hackfmt logic should be in:
- hackfmt tests for hackfmt

Tests for handling LSP request/response and converting between position representations:
- [lsp integration tests](https://fburl.com/code/bfxd9ohr)
|}

let run_hackfmt filename : string =
  let cmd = Exec_command.Hackfmt BuildOptions.default_hackfmt_path in
  let lwt_result = Lwt_utils.exec_checked cmd [| filename |] in
  match Lwt_main.run lwt_result with
  | Ok Lwt_utils.Process_success.{ stdout; _ } -> stdout
  | Error failure -> failwith (Lwt_utils.Process_failure.to_string failure)

let str_of_range Ide_api_types.{ st; ed } =
  (Printf.sprintf "%s-%s (1-indexed)")
    (File_content.Position.to_string_one_based st)
    (File_content.Position.to_string_one_based ed)

let with_line_numbers (code : string) : string =
  String.split_lines code
  |> List.mapi ~f:(fun i line -> Printf.sprintf "%d %s" (i + 1) line)
  |> String.concat ~sep:"\n"

let apply_edit
    (orig_code : string)
    ServerFormatTypes.{ new_text; range = Ide_api_types.{ st; ed } } : string =
  (* We expect only changed lines, not column-level information. Update the test if this changes *)
  assert (File_content.Position.is_beginning_of_line st);
  assert (File_content.Position.is_beginning_of_line ed);
  let orig_lines = String.split_lines orig_code in
  let (start_line, _) = File_content.Position.line_column_zero_based st in
  let (end_line, _) = File_content.Position.line_column_zero_based ed in
  let before_lines =
    List.filteri orig_lines ~f:(fun line_number (*0-indexed*) _orig_line ->
        line_number < start_line)
  in
  let lines_to_insert = String.split_lines new_text in
  let after_lines =
    List.filteri orig_lines ~f:(fun line_number (*0-indexed*) _orig_line ->
        line_number >= end_line)
  in
  let updated_code_lines = before_lines @ lines_to_insert @ after_lines in
  let suffix =
    match (List.length updated_code_lines, List.last updated_code_lines) with
    | (0, _)
    | (_, Some "") ->
      ""
    | _ -> "\n"
  in
  String.concat ~sep:"\n" updated_code_lines ^ suffix

let run_format_test (filename : string) : unit =
  let code = Disk.cat filename in
  match
    Ide_format.go_ide
      ~filename_for_logging:"foo.php"
      ~content:code
      ~action:ServerFormatTypes.Document
      ~options:Lsp.DocumentFormatting.{ tabSize = 2; insertSpaces = true }
  with
  | Ok (ServerFormatTypes.{ new_text; range } as edit) -> begin
    let new_code = apply_edit code edit in
    Printf.printf "> lines and columns in ranges are 1-indexed\n";
    Printf.printf "received code:\n%s\n\n" (with_line_numbers code);
    Printf.printf
      "New text at range %s is\n'%s'\n\n"
      (str_of_range range)
      new_text;
    Printf.printf
      "Code after applying language server edits:\n'%s'\n\n%!"
      new_code;
    let hackfmt_stdout = run_hackfmt filename in
    if String.equal new_code hackfmt_stdout then
      Printf.printf "Matched Hackfmt result\n"
    else
      failwith
        (Printf.sprintf "Did not match hackfmt result!'%s'\n" hackfmt_stdout)
  end
  | Error s -> failwith (Printf.sprintf "something went wrong! %s" s)

let parse_args () : string =
  if Array.length Sys.argv < 2 then
    failwith usage
  else
    Sys.argv.(1)

let () =
  let filename = parse_args () in
  run_format_test filename
