(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Hh_prelude

exception Expected_throw_missing

exception Thrown_exception_mismatched of (exn * exn)

let expect_throws e f x =
  try
    let _ = f x in
    Printf.eprintf "Error. Did not throw expected: %s\n" (Exn.to_string e);
    false
  with err ->
    if Poly.(e <> err) then
      let () =
        Printf.eprintf
          "Error. Expected exn: %s. But got : %s\n"
          (Exn.to_string e)
          (Exn.to_string err)
      in
      false
    else
      true

let run (name, f) =
  Printf.printf "Running %s ... %!" name;
  let result =
    try f ()
    with e ->
      let e = Exception.wrap e in
      let () = Printf.printf "Exception %s\n" (Exception.to_string e) in
      let () =
        Printf.printf "Backtrace %s\n" (Exception.get_backtrace_string e)
      in
      false
  in
  if result then
    Printf.printf "ok\n%!"
  else
    Printf.printf "fail\n%!";
  result

(** List.for_all but without shortcircuiting "&&", so runs all failures too. *)
let for_all_non_shortcircuit tests f =
  List.fold_left tests ~init:true ~f:(fun acc test -> f test && acc)

let run_all (tests : (string * (unit -> bool)) list) =
  Printexc.record_backtrace true;
  exit
    ( if for_all_non_shortcircuit tests run then
      0
    else
      1 )

let run_only tests names =
  let f =
    match names with
    | [] -> const true
    | _ -> (fun (n, _) -> List.mem names n ~equal:String.( = ))
  in
  let tests = List.filter tests ~f in
  run_all tests

let main tests =
  let names = Option.value (List.tl @@ Array.to_list Sys.argv) ~default:[] in
  run_only tests names
