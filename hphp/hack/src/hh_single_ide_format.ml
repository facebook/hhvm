open Hh_prelude

let usage =
  {|
For lightweight testing of the formatting logic in **the language server**.

Usage: hh_single_ide_format $FILENAME

Used for testing:
- Logic in the language server: this is where we convert hackfmt results
to minimal edits to send to the language client (such as VSCode)

Tests for any other hackfmt logic should be in:
- hackfmt tests for hackfmt

Tests for handling LSP request/response and converting between position representations:
- [lsp integration tests](https://fburl.com/code/bfxd9ohr)
|}

let str_of_range
    Ide_api_types.
      {
        st = { line = start_line; column = start_column };
        ed = { line = end_line; column = end_column };
      } =
  (Printf.sprintf "%d:%d-%d:%d (1-indexed)")
    start_line
    start_column
    end_line
    end_column

let with_line_numbers (code : string) : string =
  String.split_lines code
  |> List.mapi ~f:(fun i line -> Printf.sprintf "%d %s" (i + 1) line)
  |> String.concat ~sep:"\n"

let apply_edit
    (orig_code : string)
    ServerFormatTypes.
      {
        new_text;
        range =
          Ide_api_types.
            (* 1-indexed *)
          {
            st = { line = start_line; column = start_column };
            ed = { line = end_line; column = end_column };
          };
      } : string =
  (* We expect only changed lines, not column-level information. Update the test if this changes *)
  assert (start_column = 1);
  assert (end_column = 1);
  let orig_lines = String.split_lines orig_code in
  let before_lines =
    List.filteri orig_lines ~f:(fun i (*0-indexed*) _orig_line ->
        let line_number = i + 1 in
        line_number < start_line)
  in
  let lines_to_insert = String.split_lines new_text in
  let after_lines =
    List.filteri orig_lines ~f:(fun i (*0-indexed*) _orig_line ->
        let line_number = i + 1 in
        line_number >= end_line)
  in
  let updated_code_lines = before_lines @ lines_to_insert @ after_lines in
  String.concat ~sep:"\n" updated_code_lines

let run_format_test (code : string) : unit =
  match
    Ide_format.go_ide
      ~filename_for_logging:"foo.php"
      ~content:code
      ~action:ServerFormatTypes.Document
      ~options:Lsp.DocumentFormatting.{ tabSize = 2; insertSpaces = true }
  with
  | Ok (ServerFormatTypes.{ new_text; range } as edit) -> begin
    Printf.printf "> lines and columns in ranges are 1-indexed\n";
    Printf.printf "received code:\n%s\n\n" (with_line_numbers code);
    Printf.printf
      "New text at range %s is\n'%s'\n\n"
      (str_of_range range)
      new_text;
    Printf.printf
      "Updated code:\n%s\n"
      (with_line_numbers (apply_edit code edit))
  end
  | Error s -> failwith (Printf.sprintf "something went wrong! %s" s)

let parse_args () : string =
  if Array.length Sys.argv < 2 then
    failwith usage
  else
    Sys.argv.(1)

let () =
  let filename = parse_args () in
  run_format_test (Disk.cat filename)
