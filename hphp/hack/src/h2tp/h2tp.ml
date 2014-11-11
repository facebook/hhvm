(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module A = Ast
module CE = Common_exns
module Sys = Sys_ext
open Utils

let parse_options () =
  let src = ref None
  and dest = ref None
  and usage = Printf.sprintf
    "Usage: %s source destination\n"
    Sys.argv.(0) in

  let fail_with_usage error_str = begin
    prerr_endline error_str;
    prerr_endline usage;
    exit 1
  end in

  let parse_arg s = begin
    match !Arg.current with
      | 1 -> src := Some s;
      | 2 -> dest := Some s;
      | _ -> fail_with_usage "This program only accepts two arguments";
  end in

  Arg.parse [] parse_arg usage;

  if !Arg.current < 3 then fail_with_usage
    "This program requires a source file and a destination file";
  (unsafe_opt !src, unsafe_opt !dest)

let _ =
  let (src, dest) = parse_options () in
  try
    SharedMem.init ();
    Engine.go (Sys.chop_dirsymbol src) (Sys.chop_dirsymbol dest);
    print_string "The Conversion was successful\n"
  with
  | e -> Sys.die (CE.flatten_error e)
