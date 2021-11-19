(*
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module I = Phparser.MenhirInterpreter

let last_pos = ref 0

let get_line pos ic =
  let max_sz = in_channel_length ic in
  seek_in ic (max 0 (pos - 80));
  let text = really_input_string ic (min (pos + 160) max_sz - pos + 80) in
  let min_bound = ref 0 in
  let rec find_newline i =
    match String.index_from text i '\n' with
    | exception Not_found -> String.length text
    | i when i < 80 ->
      min_bound := (i + 1); find_newline (i + 1)
    | i -> i
  in
  let max_bound = find_newline 0 in
  let ofs = pos - max 0 (pos - 80) in
  String.sub text !min_bound (max_bound - !min_bound) ^ "\n" ^
  String.make (max 0 (ofs - !min_bound)) ' ' ^ "^"

let () =
  let new_state () = Phlexer.new_state
      ~strict_lexer:false
      ~verbose_lexer:false
      ~case_sensitive:false
      ~xhp_builtin:true
      ~facebook_lang_extensions:true
      ()
  in
  let trace state lexbuf =
    let token = Phlexer.token state lexbuf in
    let offset = lexbuf.Lexing.lex_abs_pos in
    let startp = offset + lexbuf.Lexing.lex_start_pos in
    let endp = offset + lexbuf.Lexing.lex_curr_pos in
    Printf.printf "%s %d-%d @ %s\n"
      (Phparser_driver.token_to_string token) startp endp
      (Phlexer.dump_modes state);
    token
  in
  let dummy f lexbuf =
    let rec loop () =
      match f lexbuf with
      | Phparser.EOF -> ()
      | _ -> loop ()
    in
    loop ()
  in
  let action =
    match if Array.length Sys.argv >= 2 then Sys.argv.(1) else "" with
    | "debug-lex" -> (fun name -> dummy (trace (new_state ())))
    | "debug-parse" ->
      (fun name ->
         let state = new_state () in
         let rec filter lexbuf =
           let offset = lexbuf.Lexing.lex_abs_pos in
           let pos = offset + lexbuf.Lexing.lex_curr_pos in
           last_pos := pos;
           match trace state lexbuf with
           | Phparser.OPEN_TAG
           | Phparser.COMMENT
           | Phparser.DOC_COMMENT
           | Phparser.SPACES
           | Phparser.NEWLINE ->
             filter lexbuf
           | result -> result
         in
         fun lexbuf ->
         try Phparser_driver.parse ~verbose:true filter lexbuf
         with exn ->
           raise Exit
      )
    | "lex" -> (fun name -> dummy (Phlexer.token (new_state ())))
    | "parse" ->
      (fun name ->
         let state = new_state () in
         let rec filter lexbuf =
           let offset = lexbuf.Lexing.lex_abs_pos in
           let pos = offset + lexbuf.Lexing.lex_curr_pos in
           last_pos := pos;
           match Phlexer.token state lexbuf with
           | Phparser.OPEN_TAG
           | Phparser.COMMENT
           | Phparser.DOC_COMMENT
           | Phparser.SPACES
           | Phparser.NEWLINE ->
             filter lexbuf
           | result -> result
         in
         fun lexbuf ->
           Phparser_driver.parse ~verbose:true filter lexbuf
      )
    | "silent-parse" ->
      (fun name ->
         let state = new_state () in
         let rec filter lexbuf =
           let offset = lexbuf.Lexing.lex_abs_pos in
           let pos = offset + lexbuf.Lexing.lex_curr_pos in
           last_pos := pos;
           match Phlexer.token state lexbuf with
           | Phparser.OPEN_TAG
           | Phparser.COMMENT
           | Phparser.DOC_COMMENT
           | Phparser.SPACES
           | Phparser.NEWLINE ->
             filter lexbuf
           | result -> result
         in
         fun lexbuf ->
           Phparser_driver.parse ~verbose:false filter lexbuf
      )
    | _ ->
      prerr_endline "driver [lex | parse | debug-lex | debug-parse]";
      exit 1
  in
  if Array.length Sys.argv = 2 then
    let lexbuf = Lexing.from_channel stdin in
    lexbuf.Lexing.lex_start_p <- Lexing.dummy_pos;
    action "-" lexbuf
  else
    for i = 2 to Array.length Sys.argv - 1 do
      let ic = open_in_bin Sys.argv.(i) in
      begin try
          action Sys.argv.(i) (Lexing.from_channel ic);
          close_in_noerr ic;
          print_endline ("successful " ^ Sys.argv.(i))
        with
        | Exit ->
          let text = get_line !last_pos ic in
          close_in_noerr ic;
          print_endline ("failed " ^ Sys.argv.(i) ^ ":" ^ string_of_int !last_pos);
          print_endline text;
          exit 1
        | exn ->
          let text = get_line !last_pos ic in
          close_in_noerr ic;
          print_endline ("failed " ^ Sys.argv.(i) ^ ":" ^ string_of_int !last_pos);
          print_endline text;
          (*reraise exn*)
      end;
    done
