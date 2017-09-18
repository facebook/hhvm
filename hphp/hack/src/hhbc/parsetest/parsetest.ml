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

let program_parser = program Hhas_lexer.read

let optchannel = if Array.length Sys.argv > 1
                 then Some (open_in Sys.argv.(1))
                 else None

let testlexer =
match optchannel with
 | Some c ->
  Lexing.from_channel c
 | None ->
 Lexing.from_string
"# starts here\n\
.adata A_3 = \"\"\"a:1:{i:0;i:2;}\"\"\";\n\n\
.main {\n\
  .declvars $x $y;\n\
  DefTypeAlias 0\n\
  Int 1\n\
  RetC\n\
  }\n\
.class [\"C\"(\"\"\"a:1:{i:0;b:0;}\"\"\") \"B\"(\"\"\"a:1:{i:0;b:1;}\"\"\") \"A\"(\"\"\"a:1:{i:0;N;}\"\"\") interface final] C1 extends foo implements (bar baz){\
  .uses fred jim;\n\
  .enum_ty <\"HH\\\\string\" hh_type extended_hint >;\n\
  }\n\
.function [\"Name\"(\"\"\"a:1:{N;N;}\"\"\")] \
<\"HH\\\\Awaitable<HH\\\\void>\" N hh_type extended_hint> \
f(<\"HH\\\\int\" \"HH\\\\int\" hh_type > $x, <\"HH\\\\int\" \"HH\\\\int\" hh_type > $y, \
<\"HH\\\\int\" \"HH\\\\int\" hh_type > $z) isAsync isGenerator{\n\
  .declvars $i1 $i2;\n\
  L0: \n Nop \n \
  DV1: \nPopA \n False \n\
  Double 1.23\n\
  Array @A_3\n\
  .try_fault L1 {\n\
   PopC\n\
  }\n\
  NewArray 10\n\
  CGetL $a\n\
  Jmp L0\n\
  Int 42\n String \"foo\"\n\
  PushL _0\n\
  ClsRefGetL $t 0\n\
  SetOpN PlusEqual\n\
  IncDecN PreInc\n\
  QueryM 1 CGet EC:0\n\
  RetC\n
  }\n\n\
  .alias Point = <\"array\"  > \"\"\"a:2:{s:4:\"kind\";i:10;s:10:\"elem_types\";a:2:{i:0;a:1:{s:4:\"kind\";i:1;}i:1;a:1:{s:4:\"kind\";i:4;}}}\"\"\";"

let parsed =  try program_parser testlexer
              with Parsing.Parse_error -> (
                print_string "oops!\n"; raise Parsing.Parse_error
                )
                (* TODO: put proper error messages and location tracking in *)

let pp = (match optchannel with
          | Some c -> close_in c
          | None -> ())
          ; Hhbc_hhas.to_string parsed

let _ = print_string pp
