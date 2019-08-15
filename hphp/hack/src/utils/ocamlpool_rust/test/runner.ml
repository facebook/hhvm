(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

external getString : unit -> string list = "getString"

external getIsize : unit -> int list = "getIsize"

external getOption : unit -> int option list = "getOption"

external getVec : unit -> int list list = "getVec"

external getBool : unit -> bool list = "getBool"

external getBox : unit -> string list = "getBox"

let run f expected () =
  let rust_values = f () in
  if List.length expected <> List.length rust_values then
    Some "Length not equal"
  else if expected <> rust_values then
    Some "not equal"
  else
    None


let tests = [
  ("getString", run getString [""; "TEST"]);
  ("getIsize", run getIsize [-1; 0; 1; max_int]);
  ("getOption", run getOption [None; Some 1]);
  ("getVec", run getVec [ []; [0] ]);
  ("getBool", run getBool [true; false]);
  ("getBox", run getBox [""; "A"]);
]

let () =
  let results =
    List.map (fun (name, f) -> (name, f ())) tests |>
    List.filter (fun (_, result) -> Option.is_some result)
  in
  if List.length results <> 0 then
    List.map (fun (name, result) -> name ^ (Option.value ~default:"" result)) results |>
    String.concat "\n" |>
    failwith
