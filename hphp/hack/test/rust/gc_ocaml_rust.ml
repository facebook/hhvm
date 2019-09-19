type rust_struct

external create_the_struct : unit -> rust_struct = "create_the_struct_ocaml"

external use_the_struct : rust_struct -> unit = "use_the_struct_ocaml"

external drop_the_struct : rust_struct -> unit = "drop_the_struct_ocaml"

let run_test x f =
  Gc.full_major ();
  Printf.printf "\n\nRunning test %d\n" x;
  flush stdout;
  f ();
  Gc.full_major ();
  Printf.printf "Completed test %d\n" x;
  flush stdout

(* Simplest usage *)
let test_case_1 () = use_the_struct (create_the_struct ())

(* Make sure GC doesn't collect prematurely *)
let test_case_2 () =
  let x = create_the_struct () in
  Gc.full_major ();
  use_the_struct x

(* No double-free on the Rust side if the struct gets Drop'ed early *)
let test_case_3 () =
  let x = create_the_struct () in
  Gc.full_major ();
  use_the_struct x;
  Gc.full_major ();
  drop_the_struct x

(* x doesn't get free'd by OCaml's GC while y is still in use --> double-free *)
let test_case_4 () =
  let x = create_the_struct () in
  use_the_struct x;
  let y = x in
  Gc.full_major ();
  drop_the_struct y

(* x, a ref, doesn't get free'd by OCaml's GC while y is still in use --> double-free *)
let test_case_5 () =
  let x = ref (create_the_struct ()) in
  use_the_struct !x;
  let y = !x in
  Gc.full_major ();
  drop_the_struct y

(* Make sure we don't crash if we try to use-after-free*)
let test_case_6 () =
  let x = create_the_struct () in
  use_the_struct x;
  drop_the_struct x;
  use_the_struct x

(* Make sure we don't crash if we try to double-free *)
let test_case_7 () =
  let x = create_the_struct () in
  use_the_struct x;
  drop_the_struct x;
  drop_the_struct x

let () =
  run_test 1 test_case_1;
  run_test 2 test_case_2;
  run_test 3 test_case_3;
  run_test 4 test_case_4;
  run_test 5 test_case_5;
  run_test 6 test_case_6;
  run_test 7 test_case_7
