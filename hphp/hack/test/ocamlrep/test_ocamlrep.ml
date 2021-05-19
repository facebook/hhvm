open Hh_prelude

(* primitive tests *)
external get_a : unit -> char = "get_a"

external get_five : unit -> int = "get_five"

external get_true : unit -> bool = "get_true"

external get_false : unit -> bool = "get_false"

(* option tests *)
external get_none : unit -> int option = "get_none"

external get_some_five : unit -> int option = "get_some_five"

external get_some_none : unit -> int option option = "get_some_none"

external get_some_some_five : unit -> int option option = "get_some_some_five"

(* ref tests *)
external get_int_ref : unit -> int ref = "get_int_ref"

external get_int_option_ref : unit -> int option ref = "get_int_option_ref"

(* unsized type tests *)
external get_str : unit -> string = "get_str"

external get_byte_slice : unit -> Caml.Bytes.t = "get_byte_slice"

external get_int_opt_slice : unit -> int option list = "get_int_opt_slice"

(* list tests *)
external get_empty_list : unit -> int list = "get_empty_list"

external get_five_list : unit -> int list = "get_five_list"

external get_one_two_three_list : unit -> int list = "get_one_two_three_list"

external get_float_list : unit -> float list = "get_float_list"

(* struct tests *)
type foo = {
  a: int;
  b: bool;
}

type bar = {
  c: foo;
  d: int option list option;
}

external get_foo : unit -> foo = "get_foo"

external get_bar : unit -> bar = "get_bar"

(* string tests *)
external get_empty_string : unit -> string = "get_empty_string"

external get_a_string : unit -> string = "get_a_string"

external get_ab_string : unit -> string = "get_ab_string"

external get_abcde_string : unit -> string = "get_abcde_string"

external get_abcdefg_string : unit -> string = "get_abcdefg_string"

external get_abcdefgh_string : unit -> string = "get_abcdefgh_string"

(* float tests *)
external get_zero_float : unit -> float = "get_zero_float"

external get_one_two_float : unit -> float = "get_one_two_float"

(* variant tests *)
type fruit =
  | Apple
  | Orange of int
  | Pear of { num: int }
  | Kiwi

external get_apple : unit -> fruit = "get_apple"

external get_orange : unit -> fruit = "get_orange"

external get_pear : unit -> fruit = "get_pear"

external get_kiwi : unit -> fruit = "get_kiwi"

(* map tests *)

module SMap = Caml.Map.Make (struct
  type t = string

  let compare = String.compare
end)

external get_empty_smap : unit -> 'a SMap.t = "get_empty_smap"

external get_int_smap_singleton : unit -> int SMap.t = "get_int_smap_singleton"

external get_int_smap : unit -> int SMap.t = "get_int_smap"

(* set tests *)

module SSet = Caml.Set.Make (struct
  type t = string

  let compare = String.compare
end)

external get_empty_sset : unit -> SSet.t = "get_empty_sset"

external get_sset_singleton : unit -> SSet.t = "get_sset_singleton"

external get_sset : unit -> SSet.t = "get_sset"

external convert_to_ocamlrep : 'a -> 'a = "convert_to_ocamlrep"

external realloc_in_ocaml_heap : 'a -> 'a = "realloc_in_ocaml_heap"

(* int64 tests *)
external roundtrip_int64 : Int64.t -> Int64.t = "roundtrip_int64"

let test_int64 () =
  let cases = [Int64.max_value; Int64.min_value; 2L; 0xfaceb00cL] in
  List.iter ~f:(fun x -> assert (Int64.equal (roundtrip_int64 x) x)) cases

let test_char () =
  let x = get_a () in
  assert (Char.equal x 'a')

let test_int () =
  let x = get_five () in
  assert (x = 5)

let test_true () =
  let x = get_true () in
  assert x

let test_false () =
  let x = get_false () in
  assert (not x)

let test_none () =
  let opt = get_none () in
  assert (Option.is_none opt)

let test_some () =
  let opt = get_some_five () in
  match opt with
  | None -> assert false
  | Some x -> assert (x = 5)

let test_some_none () =
  let opt = get_some_none () in
  match opt with
  | None -> assert false
  | Some x -> assert (Option.is_none x)

let test_some_some_five () =
  let opt = get_some_some_five () in
  match opt with
  | None -> assert false
  | Some x ->
    (match x with
    | None -> assert false
    | Some y -> assert (y = 5))

let test_int_ref () =
  let int_ref = get_int_ref () in
  assert (!int_ref = 5)

let test_int_option_ref () =
  let int_opt_ref = get_int_option_ref () in
  match !int_opt_ref with
  | Some 5 -> ()
  | _ -> assert false

let test_str () =
  let s = get_str () in
  assert (String.equal s "static str")

let test_byte_slice () =
  let b = get_byte_slice () in
  assert (String.equal (Caml.Bytes.sub_string b 0 4) "byte");
  assert (Char.equal (Caml.Bytes.get b 4) '\x00');
  assert (Char.equal (Caml.Bytes.get b 5) '\xFF');
  assert (String.equal (Caml.Bytes.sub_string b 6 5) "slice")

let test_int_opt_slice () =
  match get_int_opt_slice () with
  | [None; Some 2; Some 3] -> ()
  | _ -> assert false

let test_empty_list () =
  let lst = get_empty_list () in
  assert (List.length lst = 0);
  match lst with
  | [] -> ()
  | _ -> assert false

let test_five_list () =
  let lst = get_five_list () in
  assert (List.length lst = 1);
  match lst with
  | [5] -> ()
  | _ -> assert false

let test_one_two_three_list () =
  match get_one_two_three_list () with
  | [1; 2; 3] -> ()
  | _ -> assert false

let test_float_list () =
  match get_float_list () with
  | [1.0; 2.0; 3.0] -> ()
  | _ -> assert false

let test_foo () =
  match get_foo () with
  | { a = 25; b = true } -> ()
  | _ -> assert false

let test_bar () =
  match get_bar () with
  | { c = { a = 42; b = false }; d = Some [Some 88; None; Some 66] } -> ()
  | _ -> assert false

let test_empty_string () =
  let s = get_empty_string () in
  assert (String.length s = 0);
  assert (String.equal s "")

let test_a_string () =
  let s = get_a_string () in
  assert (String.length s = 1);
  assert (String.equal s "a")

let test_ab_string () =
  let s = get_ab_string () in
  assert (String.length s = 2);
  assert (String.equal s "ab")

let test_abcde_string () =
  let s = get_abcde_string () in
  assert (String.length s = 5);
  assert (String.equal s "abcde")

let test_abcdefg_string () =
  let s = get_abcdefg_string () in
  assert (String.length s = 7);
  assert (String.equal s "abcdefg")

let test_abcdefgh_string () =
  let s = get_abcdefgh_string () in
  assert (String.length s = 8);
  assert (String.equal s "abcdefgh")

let float_compare f1 f2 =
  let abs_diff = Float.abs (f1 -. f2) in
  Float.(abs_diff < 0.0001)

let test_zero_float () =
  let f = get_zero_float () in
  assert (float_compare f 0.)

let test_one_two_float () =
  let f = get_one_two_float () in
  assert (float_compare f 1.2)

let test_apple () =
  assert (
    match get_apple () with
    | Apple -> true
    | _ -> false )

let test_kiwi () =
  assert (
    match get_kiwi () with
    | Kiwi -> true
    | _ -> false )

let test_orange () =
  match get_orange () with
  | Orange 39 -> ()
  | _ -> assert false

let test_pear () =
  match get_pear () with
  | Pear { num = 76 } -> ()
  | _ -> assert false

let test_empty_smap () =
  match SMap.bindings (get_empty_smap ()) with
  | [] -> ()
  | _ -> assert false

let test_int_smap_singleton () =
  match SMap.bindings (get_int_smap_singleton ()) with
  | [("a", 1)] -> ()
  | _ -> assert false

let test_int_smap () =
  match SMap.bindings (get_int_smap ()) with
  | [("a", 1); ("b", 2); ("c", 3)] -> ()
  | _ -> assert false

let test_empty_sset () =
  match SSet.elements (get_empty_sset ()) with
  | [] -> ()
  | _ -> assert false

let test_sset_singleton () =
  match SSet.elements (get_sset_singleton ()) with
  | ["a"] -> ()
  | _ -> assert false

let test_sset () =
  match SSet.elements (get_sset ()) with
  | ["a"; "b"; "c"] -> ()
  | _ -> assert false

(* Conversion tests *)

let test_convert_char () =
  let x = convert_to_ocamlrep 'a' in
  assert (Char.equal x 'a')

let test_convert_int () =
  let x = convert_to_ocamlrep 5 in
  assert (x = 5)

let test_convert_true () =
  let x = convert_to_ocamlrep true in
  assert x

let test_convert_false () =
  let x = convert_to_ocamlrep false in
  assert (not x)

let test_convert_none () =
  let opt = convert_to_ocamlrep None in
  assert (Option.is_none opt)

let test_convert_some () =
  let opt = convert_to_ocamlrep (Some 5) in
  match opt with
  | None -> assert false
  | Some x -> assert (x = 5)

let test_convert_some_none () =
  let opt = convert_to_ocamlrep (Some None) in
  match opt with
  | None -> assert false
  | Some x -> assert (Option.is_none x)

let test_convert_some_some_five () =
  let opt = convert_to_ocamlrep (Some (Some 5)) in
  match opt with
  | None -> assert false
  | Some x ->
    (match x with
    | None -> assert false
    | Some y -> assert (y = 5))

let test_convert_int_ref () =
  let int_ref = convert_to_ocamlrep (ref 5) in
  assert (!int_ref = 5)

let test_convert_int_option_ref () =
  let int_opt_ref = convert_to_ocamlrep (ref (Some 5)) in
  match !int_opt_ref with
  | Some 5 -> ()
  | _ -> assert false

let test_convert_empty_list () =
  let lst = convert_to_ocamlrep [] in
  assert (List.length lst = 0);
  match lst with
  | [] -> ()
  | _ -> assert false

let test_convert_five_list () =
  let lst = convert_to_ocamlrep [5] in
  assert (List.length lst = 1);
  match lst with
  | [5] -> ()
  | _ -> assert false

let test_convert_one_two_three_list () =
  match convert_to_ocamlrep [1; 2; 3] with
  | [1; 2; 3] -> ()
  | _ -> assert false

let test_convert_float_list () =
  match convert_to_ocamlrep [1.0; 2.0; 3.0] with
  | [1.0; 2.0; 3.0] -> ()
  | _ -> assert false

let test_convert_foo () =
  match convert_to_ocamlrep { a = 25; b = true } with
  | { a = 25; b = true } -> ()
  | _ -> assert false

let test_convert_bar () =
  match
    convert_to_ocamlrep
      { c = { a = 42; b = false }; d = Some [Some 88; None; Some 66] }
  with
  | { c = { a = 42; b = false }; d = Some [Some 88; None; Some 66] } -> ()
  | _ -> assert false

let test_convert_empty_string () =
  let s = convert_to_ocamlrep "" in
  assert (String.length s = 0);
  assert (String.equal s "")

let test_convert_a_string () =
  let s = convert_to_ocamlrep "a" in
  assert (String.length s = 1);
  assert (String.equal s "a")

let test_convert_ab_string () =
  let s = convert_to_ocamlrep "ab" in
  assert (String.length s = 2);
  assert (String.equal s "ab")

let test_convert_abcde_string () =
  let s = convert_to_ocamlrep "abcde" in
  assert (String.length s = 5);
  assert (String.equal s "abcde")

let test_convert_abcdefg_string () =
  let s = convert_to_ocamlrep "abcdefg" in
  assert (String.length s = 7);
  assert (String.equal s "abcdefg")

let test_convert_abcdefgh_string () =
  let s = convert_to_ocamlrep "abcdefgh" in
  assert (String.length s = 8);
  assert (String.equal s "abcdefgh")

let float_compare f1 f2 =
  let abs_diff = Float.abs (f1 -. f2) in
  Float.(abs_diff < 0.0001)

let test_convert_zero_float () =
  let f = convert_to_ocamlrep 0. in
  assert (float_compare f 0.)

let test_convert_one_two_float () =
  let f = convert_to_ocamlrep 1.2 in
  assert (float_compare f 1.2)

let test_convert_apple () =
  assert (
    match convert_to_ocamlrep Apple with
    | Apple -> true
    | _ -> false )

let test_convert_kiwi () =
  assert (
    match convert_to_ocamlrep Kiwi with
    | Kiwi -> true
    | _ -> false )

let test_convert_orange () =
  match convert_to_ocamlrep (Orange 39) with
  | Orange 39 -> ()
  | _ -> assert false

let test_convert_pear () =
  match convert_to_ocamlrep (Pear { num = 76 }) with
  | Pear { num = 76 } -> ()
  | _ -> assert false

let test_convert_empty_smap () =
  match SMap.bindings (convert_to_ocamlrep SMap.empty) with
  | [] -> ()
  | _ -> assert false

let test_convert_int_smap_singleton () =
  match SMap.bindings (convert_to_ocamlrep (SMap.singleton "a" 1)) with
  | [("a", 1)] -> ()
  | _ -> assert false

let test_convert_int_smap () =
  let map = SMap.empty in
  let map = SMap.add "a" 1 map in
  let map = SMap.add "b" 2 map in
  let map = SMap.add "c" 3 map in
  match SMap.bindings (convert_to_ocamlrep map) with
  | [("a", 1); ("b", 2); ("c", 3)] -> ()
  | _ -> assert false

let test_convert_empty_sset () =
  match SSet.elements (convert_to_ocamlrep SSet.empty) with
  | [] -> ()
  | _ -> assert false

let test_convert_sset_singleton () =
  match SSet.elements (convert_to_ocamlrep (SSet.singleton "a")) with
  | ["a"] -> ()
  | _ -> assert false

let test_convert_sset () =
  let set = SSet.empty in
  let set = SSet.add "a" set in
  let set = SSet.add "b" set in
  let set = SSet.add "c" set in
  match SSet.elements (convert_to_ocamlrep set) with
  | ["a"; "b"; "c"] -> ()
  | _ -> assert false

let test_convert_shared_value () =
  let str = "foo" in
  let tup = (str, str) in
  let (str1, str2) = convert_to_ocamlrep tup in
  assert (phys_equal str1 str2)

let test_realloc_char () =
  let x = realloc_in_ocaml_heap 'a' in
  assert (Char.equal x 'a')

let test_realloc_int () =
  let x = realloc_in_ocaml_heap 5 in
  assert (x = 5)

let test_realloc_true () =
  let x = realloc_in_ocaml_heap true in
  assert x

let test_realloc_false () =
  let x = realloc_in_ocaml_heap false in
  assert (not x)

let test_realloc_none () =
  let opt = realloc_in_ocaml_heap None in
  assert (Option.is_none opt)

let test_realloc_some () =
  let opt = realloc_in_ocaml_heap (Some 5) in
  match opt with
  | None -> assert false
  | Some x -> assert (x = 5)

let test_realloc_some_none () =
  let opt = realloc_in_ocaml_heap (Some None) in
  match opt with
  | None -> assert false
  | Some x -> assert (Option.is_none x)

let test_realloc_some_some_five () =
  let opt = realloc_in_ocaml_heap (Some (Some 5)) in
  match opt with
  | None -> assert false
  | Some x ->
    (match x with
    | None -> assert false
    | Some y -> assert (y = 5))

let test_realloc_empty_list () =
  let lst = realloc_in_ocaml_heap [] in
  assert (List.length lst = 0);
  match lst with
  | [] -> ()
  | _ -> assert false

let test_realloc_five_list () =
  let lst = realloc_in_ocaml_heap [5] in
  assert (List.length lst = 1);
  match lst with
  | [5] -> ()
  | _ -> assert false

let test_realloc_one_two_three_list () =
  match realloc_in_ocaml_heap [1; 2; 3] with
  | [1; 2; 3] -> ()
  | _ -> assert false

let test_realloc_float_list () =
  match realloc_in_ocaml_heap [1.0; 2.0; 3.0] with
  | [1.0; 2.0; 3.0] -> ()
  | _ -> assert false

let test_realloc_foo () =
  match realloc_in_ocaml_heap { a = 25; b = true } with
  | { a = 25; b = true } -> ()
  | _ -> assert false

let test_realloc_bar () =
  match
    realloc_in_ocaml_heap
      { c = { a = 42; b = false }; d = Some [Some 88; None; Some 66] }
  with
  | { c = { a = 42; b = false }; d = Some [Some 88; None; Some 66] } -> ()
  | _ -> assert false

let test_realloc_empty_string () =
  let s = realloc_in_ocaml_heap "" in
  assert (String.length s = 0);
  assert (String.equal s "")

let test_realloc_a_string () =
  let s = realloc_in_ocaml_heap "a" in
  assert (String.length s = 1);
  assert (String.equal s "a")

let test_realloc_ab_string () =
  let s = realloc_in_ocaml_heap "ab" in
  assert (String.length s = 2);
  assert (String.equal s "ab")

let test_realloc_abcde_string () =
  let s = realloc_in_ocaml_heap "abcde" in
  assert (String.length s = 5);
  assert (String.equal s "abcde")

let test_realloc_abcdefg_string () =
  let s = realloc_in_ocaml_heap "abcdefg" in
  assert (String.length s = 7);
  assert (String.equal s "abcdefg")

let test_realloc_abcdefgh_string () =
  let s = realloc_in_ocaml_heap "abcdefgh" in
  assert (String.length s = 8);
  assert (String.equal s "abcdefgh")

let float_compare f1 f2 =
  let abs_diff = Float.abs (f1 -. f2) in
  Float.(abs_diff < 0.0001)

let test_realloc_zero_float () =
  let f = realloc_in_ocaml_heap 0. in
  assert (float_compare f 0.)

let test_realloc_one_two_float () =
  let f = realloc_in_ocaml_heap 1.2 in
  assert (float_compare f 1.2)

let test_realloc_apple () =
  assert (
    match realloc_in_ocaml_heap Apple with
    | Apple -> true
    | _ -> false )

let test_realloc_kiwi () =
  assert (
    match realloc_in_ocaml_heap Kiwi with
    | Kiwi -> true
    | _ -> false )

let test_realloc_orange () =
  match realloc_in_ocaml_heap (Orange 39) with
  | Orange 39 -> ()
  | _ -> assert false

let test_realloc_pear () =
  match realloc_in_ocaml_heap (Pear { num = 76 }) with
  | Pear { num = 76 } -> ()
  | _ -> assert false

let test_realloc_empty_smap () =
  match SMap.bindings (realloc_in_ocaml_heap SMap.empty) with
  | [] -> ()
  | _ -> assert false

let test_realloc_int_smap_singleton () =
  match SMap.bindings (realloc_in_ocaml_heap (SMap.singleton "a" 1)) with
  | [("a", 1)] -> ()
  | _ -> assert false

let test_realloc_int_smap () =
  let map = SMap.empty in
  let map = SMap.add "a" 1 map in
  let map = SMap.add "b" 2 map in
  let map = SMap.add "c" 3 map in
  match SMap.bindings (realloc_in_ocaml_heap map) with
  | [("a", 1); ("b", 2); ("c", 3)] -> ()
  | _ -> assert false

let test_realloc_empty_sset () =
  match SSet.elements (realloc_in_ocaml_heap SSet.empty) with
  | [] -> ()
  | _ -> assert false

let test_realloc_sset_singleton () =
  match SSet.elements (realloc_in_ocaml_heap (SSet.singleton "a")) with
  | ["a"] -> ()
  | _ -> assert false

let test_realloc_sset () =
  let set = SSet.empty in
  let set = SSet.add "a" set in
  let set = SSet.add "b" set in
  let set = SSet.add "c" set in
  match SSet.elements (realloc_in_ocaml_heap set) with
  | ["a"; "b"; "c"] -> ()
  | _ -> assert false

let test_realloc_shared_value () =
  let str = "foo" in
  let tup = (str, str) in
  let (str1, str2) = realloc_in_ocaml_heap tup in
  assert (phys_equal str1 str2)

let test_cases =
  [
    test_int64;
    test_char;
    test_int;
    test_true;
    test_false;
    test_none;
    test_some;
    test_some_none;
    test_some_some_five;
    test_int_ref;
    test_int_option_ref;
    test_str;
    test_byte_slice;
    test_int_opt_slice;
    test_empty_list;
    test_five_list;
    test_one_two_three_list;
    test_float_list;
    test_foo;
    test_bar;
    test_empty_string;
    test_a_string;
    test_ab_string;
    test_abcde_string;
    test_abcdefg_string;
    test_abcdefgh_string;
    test_zero_float;
    test_one_two_float;
    test_apple;
    test_kiwi;
    test_orange;
    test_pear;
    test_empty_smap;
    test_int_smap_singleton;
    test_int_smap;
    test_empty_sset;
    test_sset_singleton;
    test_sset;
    test_convert_char;
    test_convert_int;
    test_convert_true;
    test_convert_false;
    test_convert_none;
    test_convert_some;
    test_convert_some_none;
    test_convert_some_some_five;
    test_convert_int_ref;
    test_convert_int_option_ref;
    test_convert_empty_list;
    test_convert_five_list;
    test_convert_one_two_three_list;
    test_convert_float_list;
    test_convert_foo;
    test_convert_bar;
    test_convert_empty_string;
    test_convert_a_string;
    test_convert_ab_string;
    test_convert_abcde_string;
    test_convert_abcdefg_string;
    test_convert_abcdefgh_string;
    test_convert_zero_float;
    test_convert_one_two_float;
    test_convert_apple;
    test_convert_kiwi;
    test_convert_orange;
    test_convert_pear;
    test_convert_empty_smap;
    test_convert_int_smap_singleton;
    test_convert_int_smap;
    test_convert_empty_sset;
    test_convert_sset_singleton;
    test_convert_sset;
    test_convert_shared_value;
    test_realloc_char;
    test_realloc_int;
    test_realloc_true;
    test_realloc_false;
    test_realloc_none;
    test_realloc_some;
    test_realloc_some_none;
    test_realloc_some_some_five;
    test_realloc_empty_list;
    test_realloc_five_list;
    test_realloc_one_two_three_list;
    test_realloc_float_list;
    test_realloc_foo;
    test_realloc_bar;
    test_realloc_empty_string;
    test_realloc_a_string;
    test_realloc_ab_string;
    test_realloc_abcde_string;
    test_realloc_abcdefg_string;
    test_realloc_abcdefgh_string;
    test_realloc_zero_float;
    test_realloc_one_two_float;
    test_realloc_apple;
    test_realloc_kiwi;
    test_realloc_orange;
    test_realloc_pear;
    test_realloc_empty_smap;
    test_realloc_int_smap_singleton;
    test_realloc_int_smap;
    test_realloc_empty_sset;
    test_realloc_sset_singleton;
    test_realloc_sset;
    test_realloc_shared_value;
  ]

let main () = List.iter test_cases (fun test -> test ())

let () = main ()
