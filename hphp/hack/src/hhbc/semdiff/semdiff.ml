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
let (data_decls1, prog1) = try program_parser lexer1
            with Parsing.Parse_error -> (
              print_endline "Parsing of file1 failed";
              raise Parsing.Parse_error
              )
let _ = close_in channel1

(* copy paste... *)
let lexer2 = Lexing.from_channel channel2
let (data_decls2, prog2) = try program_parser lexer2
            with Parsing.Parse_error -> (
              print_endline "Parsing of file2 failed";
              raise Parsing.Parse_error
              )
let _ = close_in channel2

(* This is an absolutely foul, and hopefully temporary, hack to get around
the fact that hhbc_hhas now uses a global ref to generate adata indices on the
fly, by looking up the data. That means you can't have two programs around
at once and print their instructions correctly *)
let _ = Diff.data_decls_ref1 := data_decls1

let _ = Diff.data_decls_ref2 := data_decls2

let foo () =
 let (d,(s,e)) = program_comparer.comparer prog1 prog2 in
  (Printf.printf "distance = %d\nsize = %d\nsimilarity =%.2f%%\nedits=\n%s"
          d s (100.0 *. (1.0 -. float_of_int d /. float_of_int (s+1))) e;
   print_endline defaultstring) (* make sure the colours are back to normal *)

let _ = foo ()
