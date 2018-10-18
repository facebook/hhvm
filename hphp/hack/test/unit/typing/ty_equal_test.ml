open Ast
open Core_kernel
open Nast
open Typing_defs

let k1 = SFlit_int (Pos.none, "123")
let k2 = SFlit_str (Pos.none, "foo")
let k3 = SFlit_str (Pos.none, "bar")

let tk1 = Reason.none, Tprim Tint
let tk2 = Reason.none, Tprim Tstring
let tk3 = Reason.none, Tprim Tstring

let tv1 = Reason.none, Tprim Tint
let tv2 = Reason.none, Tprim Tfloat
let tv3 = Reason.none, Tprim Tstring

let shape fields =
  Reason.none, Tarraykind (AKshape (ShapeMap.of_list fields))
let tuple values =
  let imap_of_list values =
    List.foldi values
      ~init:IMap.empty
      ~f:(fun index acc value -> IMap.add index value acc) in
  Reason.none, Tarraykind (AKtuple (imap_of_list values))

let test_shape_like_arrays_same_fields_same_order () =
  ty_equal
    (shape [k1, (tk1, tv1); k2, (tk2, tv2); k3, (tk3, tv3)])
    (shape [k1, (tk1, tv1); k2, (tk2, tv2); k3, (tk3, tv3)])

let test_shape_like_arrays_same_fields_different_order () =
  ty_equal
    (shape [k1, (tk1, tv1); k2, (tk2, tv2); k3, (tk3, tv3)])
    (shape [k3, (tk3, tv3); k2, (tk2, tv2); k1, (tk1, tv1)])

let test_shape_like_arrays_same_keys_different_values () =
  not @@ ty_equal
    (shape [k1, (tk1, tv1); k2, (tk2, tv2); k3, (tk3, tv3)])
    (shape [k1, (tk1, tv1); k2, (tk2, tv3); k3, (tk3, tv2)])

let test_shape_like_arrays_different_keys () =
  not @@ ty_equal
    (shape [k1, (tk1, tv1); k3, (tk3, tv3)])
    (shape [k1, (tk1, tv1); k2, (tk2, tv2)])

let test_tuple_like_arrays_same_length_same_values () =
  ty_equal
    (tuple [tv1; tv2; tv3])
    (tuple [tv1; tv2; tv3])

let test_tuple_like_arrays_same_length_different_values () =
  not @@ ty_equal
    (tuple [tv1; tv2; tv3])
    (tuple [tv2; tv3; tv1])

let test_tuple_like_arrays_different_length () =
  not @@ ty_equal
    (tuple [tv1; tv2; tv3])
    (tuple [tv1; tv2])

let tests =
  [
    "compare shape-like arrays: same fields, same order",
    test_shape_like_arrays_same_fields_same_order;
    "compare shape-like arrays: same fields, different order",
    test_shape_like_arrays_same_fields_different_order;
    "compare shape-like arrays: same keys, different values",
    test_shape_like_arrays_same_keys_different_values;
    "compare shape-like arrays: different keys",
    test_shape_like_arrays_different_keys;
    "compare tuple-like arrays: same length, same values",
    test_tuple_like_arrays_same_length_same_values;
    "compare tuple-like arrays: same length, different values",
    test_tuple_like_arrays_same_length_different_values;
    "compare tuple-like arrays: different length",
    test_tuple_like_arrays_different_length;
  ]

let () = Unit_test.run_all tests
