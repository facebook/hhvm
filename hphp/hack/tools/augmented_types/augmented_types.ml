(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

let parse_options () =
  let usage = Printf.sprintf "Usage: %s filename\n" Sys.argv.(0) in
  let fn = ref "" in
  Arg.parse [] (fun s -> fn := s) usage;
  if !fn = "" then begin
    prerr_endline "Must specify a file to operate on";
    prerr_endline usage;
    exit 1
  end;
  !fn

let main () =
  let fn = parse_options () in
  let sopt, errl = Convert.convert fn in
  let s = match sopt with
    | None -> "[No change]"
    | Some s -> s in
  List.iter prerr_endline errl;
  print_endline s

let () = main ()
