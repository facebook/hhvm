(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Hhas_parser

let program_parser = program Hhas_lexer.read

let optchannel =
  if Array.length Sys.argv > 1
  then Some (open_in Sys.argv.(1))
  else None

let sample_hhas =
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
  CGetL $t\n\
  ClsRefGetC 0\n\
  QueryM 1 CGet EC:0\n\
  RetC\n
  }\n\n\
  .alias Point = <\"array\"  > \"\"\"a:2:{s:4:\"kind\";i:10;s:10:\"elem_types\";a:2:{i:0;a:1:{s:4:\"kind\";i:1;}i:1;a:1:{s:4:\"kind\";i:4;}}}\"\"\";\n\n\
  .function_refs {\nf1\n}\n\n\
  .includes {\n\n/home/akr/fbcode/hphp/BUILD_MODE    \n      parsetest.ml\n\n}\n\n\
  .class_refs {   \n  \n\n     }\n\n\
  .constant_refs {k1}\n\n"

let testlexer = match optchannel with
 | Some c -> Lexing.from_channel c
 | None -> Lexing.from_string sample_hhas

let parsed =
  try
    program_parser testlexer
  with exc ->
    Printf.eprintf "%s\n" (Printexc.to_string exc);
    raise exc

let pp =
  (match optchannel with
    | Some c -> close_in c
    | None -> ());
  Hhbc_hhas.to_string parsed

let _ = print_string pp
