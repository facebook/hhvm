(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module CT = Convert_ty

let parse_options () =
  let usage = Printf.sprintf
    "Usage: %s [--loose|--strict] filename\n"
    Sys.argv.(0) in

  let loose = ref false in
  let strict = ref false in
  let fn = ref "" in
  let options = [
    "--loose", Arg.Set loose,
    " Use approximate, loose conversions.";
    "--strict", Arg.Set strict,
    " Convert only types that exactly match.";
  ] in

  Arg.parse options (fun s -> fn := s) usage;

  let mode = match (!loose, !strict) with
    | (true, false) -> CT.Loose
    | (false, true) -> CT.Strict
    | _ -> begin
      prerr_endline "Must specify exactly one of --loose and --strict";
      prerr_endline usage;
      exit 1
    end in

  if !fn = "" then begin
    prerr_endline "Must specify a file to operate on";
    prerr_endline usage;
    exit 1
  end;

  mode, !fn

let main () =
  let mode, fn = parse_options () in
  let sopt, errl = Convert.convert mode fn in
  let s = match sopt with
    | None -> "[No change]"
    | Some s -> s in
  List.iter prerr_endline errl;
  print_endline s

let () = main ()
