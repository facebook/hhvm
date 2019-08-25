(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type foo1 =
  | AA
  | BB of bool * string
  | CC
  | DD of int

and foo2 = bool

and foo3 = {
  aa: int;
  bb: bool;
}

and foo4 = bool * string

external getString : unit -> string list = "getString"

external getIsize : unit -> int list = "getIsize"

external getOption : unit -> int option list = "getOption"

external getVec : unit -> int list list = "getVec"

external getBool : unit -> bool list = "getBool"

external getBox : unit -> string list = "getBox"

external getFoo1 : unit -> foo1 list = "getFoo1"

external getFoo2 : unit -> foo2 list = "getFoo2"

external getFoo3 : unit -> foo3 list = "getFoo3"

external getFoo4 : unit -> foo4 list = "getFoo4"

external getStrRef : unit -> string list = "getStrRef"

external getRc : unit -> foo2 list = "getRc"

external getPathBuf : unit -> string list = "getPathBuf"

let run f expected () =
  let rust_values = f () in
  if List.length expected <> List.length rust_values then
    Some "Length not equal"
  else if expected <> rust_values then
    Some "not equal"
  else
    None

let tests =
  [ ("getString", run getString [""; "TEST"]);
    ("getIsize", run getIsize [-1; 0; 1; max_int]);
    ("getOption", run getOption [None; Some 1]);
    ("getVec", run getVec [[]; [0]]);
    ("getBool", run getBool [true; false]);
    ("getBox", run getBox [""; "A"]);
    ("getFoo1", run getFoo1 [AA; BB (true, "A"); CC; DD 2]);
    ("getFoo2", run getFoo2 [true; false]);
    ("getFoo3", run getFoo3 [{ aa = 2; bb = true }]);
    ("getFoo4", run getFoo4 [(true, "C")]);
    ("getStrRef", run getStrRef [""; "TEST"]);
    ("getRc", run getRc [true]);
    ("getPathBuf", run getPathBuf ["foo.txt"; ""]) ]

let () =
  let results =
    List.map (fun (name, f) -> (name, f ())) tests
    |> List.filter (fun (_, result) -> Option.is_some result)
  in
  if List.length results <> 0 then
    List.map
      (fun (name, result) -> name ^ Option.value ~default:"" result)
      results
    |> String.concat "\n"
    |> failwith
