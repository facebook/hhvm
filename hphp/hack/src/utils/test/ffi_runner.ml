(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

external offset_to_file_pos_triple : string -> int -> int * int * int
  = "offset_to_file_pos_triple"

external offset_to_file_pos_triple_with_cursor :
  string -> int -> int -> int * int * int
  = "offset_to_file_pos_triple_with_cursor"

external offset_to_line_start_offset : string -> int -> int
  = "offset_to_line_start_offset"

external offset_to_position : string -> int -> int * int = "offset_to_position"

external position_to_offset : string -> bool -> int -> int -> int * bool
  = "position_to_offset"

let all = List.for_all (fun x -> x)

let rec range a b =
  if a > b then
    []
  else
    a :: range (a + 1) b

let rec prod l1 l2 =
  match (l1, l2) with
  | ([], _)
  | (_, []) ->
    []
  | (h1 :: t1, h2 :: t2) -> ((h1, h2) :: prod [h1] t2) @ prod t1 l2

let run_1 data =
  let len = String.length data in
  let lbm = Line_break_map.make data in
  let inputs = range (-1) (len + 1) in
  let cmp i =
    Line_break_map.reset_global_state ();
    let ((e1, e2, e3) as e) = Line_break_map.offset_to_file_pos_triple lbm i in
    let ((a1, a2, a3) as a) = offset_to_file_pos_triple data i in
    if a <> e then (
      Printf.printf
        "Array: %s : len: %d \n"
        (Line_break_map.show lbm)
        (String.length data);
      Printf.printf "String: %s\nInput: %d\n" data i;
      Printf.printf "Expected %d %d %d\nActual %d %d %d\n\n" e1 e2 e3 a1 a2 a3;
      false
    ) else
      true
  in
  List.map cmp inputs |> all

let run_2 data =
  let len = String.length data in
  let lbm = Line_break_map.make data in
  let inputs = range (-1) (len + 1) in
  let inputs = prod inputs inputs in
  let cmp (i, j) =
    Line_break_map.reset_global_state ();
    ignore (Line_break_map.offset_to_file_pos_triple lbm j);
    let ((e1, e2, e3) as e) = Line_break_map.offset_to_file_pos_triple lbm i in
    let ((a1, a2, a3) as a) = offset_to_file_pos_triple_with_cursor data j i in
    if a <> e then (
      Printf.printf
        "Array: %s : len: %d \n"
        (Line_break_map.show lbm)
        (String.length data);
      Printf.printf "String: %s\nInput: %d\n" data i;
      Printf.printf "Expected %d %d %d\nActual %d %d %d\n\n" e1 e2 e3 a1 a2 a3;
      false
    ) else
      true
  in
  List.map cmp inputs |> all

let run_3 data =
  let len = String.length data in
  let lbm = Line_break_map.make data in
  let inputs = range (-1) (len + 1) in
  let cmp i =
    Line_break_map.reset_global_state ();
    let e = Line_break_map.offset_to_line_start_offset lbm i in
    let a = offset_to_line_start_offset data i in
    if a <> e then (
      Printf.printf
        "Array: %s : len: %d \n"
        (Line_break_map.show lbm)
        (String.length data);
      Printf.printf "String: %s\nInput: %d\n" data i;
      Printf.printf "Expected %d \nActual %d \n\n" e a;
      false
    ) else
      true
  in
  List.map cmp inputs |> all

let run_4 data =
  let len = String.length data in
  let lbm = Line_break_map.make data in
  let inputs = range (-1) (len + 1) in
  let cmp i =
    Line_break_map.reset_global_state ();
    let ((e1, e2) as e) = Line_break_map.offset_to_position lbm i in
    let ((a1, a2) as a) = offset_to_position data i in
    if a <> e then (
      Printf.printf
        "Array: %s : len: %d \n"
        (Line_break_map.show lbm)
        (String.length data);
      Printf.printf "String: %s\nInput: %d\n" data i;
      Printf.printf "Expected %d %d\nActual %d %d\n\n" e1 e2 a1 a2;
      false
    ) else
      true
  in
  List.map cmp inputs |> all

let exception_to_bool f =
  try
    let x = f () in
    (x, true)
  with _ -> (0, false)

let run_5 data =
  let len = String.length data in
  let lbm = Line_break_map.make data in
  let inputs = range (-1) (len + 1) in
  let inputs = prod inputs inputs in
  let inputs = prod inputs [true; false] in
  let cmp ((i, j), existing) =
    Line_break_map.reset_global_state ();
    let ((e1, e2) as e) =
      exception_to_bool (fun () ->
          Line_break_map.position_to_offset ~existing lbm (i, j))
    in
    let ((a1, a2) as a) = position_to_offset data existing i j in
    if a <> e then (
      Printf.printf
        "Array: %s : len: %d \n"
        (Line_break_map.show lbm)
        (String.length data);
      Printf.printf "String: %s\nInput: %d, %d, %b\n" data i j existing;
      Printf.printf "Expected %d %b\nActual %d %b\n\n" e1 e2 a1 a2;
      false
    ) else
      true
  in
  List.map cmp inputs |> all

let data_ =
  [
    "";
    "\n";
    "\n\n";
    "12345";
    "12345\n";
    "123\n123";
    "\n123\n123\n";
    "";
    "\r\n";
    "\r\n\r\n";
    "12345";
    "12345\r\n";
    "123\r\n123";
    "\r\n123\r\n123\r\n";
    "\r\r\n";
    "\r\n\r\n";
    "\n\r\n\r\n";
  ]

let data = String.concat "" data_ :: data_

let tests = [run_1; run_2; run_3; run_4; run_5]

let () =
  let result = all (List.map (fun t -> all (List.map t data)) tests) in
  if not result then failwith "Fail"
