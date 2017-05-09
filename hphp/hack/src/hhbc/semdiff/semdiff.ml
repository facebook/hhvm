(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)
open Hhas_parser
open Diff

let program_parser = program Hhas_lexer.read
let _ = print_endline "Hhas differencing tool"
let _ = if Array.length Sys.argv != 3 then
        (print_endline "Usage: semdiff file1 file2"; exit 1)
        else ()

let channel1 = open_in Sys.argv.(1) (* TODO: error handling *)
let channel2 = open_in Sys.argv.(2)

let lexer1 = Lexing.from_channel channel1
let prog1 = try program_parser lexer1
            with Parsing.Parse_error -> (
              print_endline "Parsing of file1 failed";
              raise Parsing.Parse_error
              )
let _ = close_in channel1

(* copy paste... *)
let lexer2 = Lexing.from_channel channel2
let prog2 = try program_parser lexer2
            with Parsing.Parse_error -> (
              print_endline "Parsing of file2 failed";
              raise Parsing.Parse_error
              )
let _ = close_in channel2

let foo () =
let (d,(s,e)) = program_comparer.comparer prog1 prog2 in
Printf.printf "distance = %d\nsize = %d\nsimilarity =%.2f%%\nedits=\n%s"
          d s (100.0 *. (1.0 -. float_of_int d /. float_of_int (s+1))) e;
print_endline defaultstring (* make sure the colours are back to normal *)

let _ = foo ()
