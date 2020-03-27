(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let test_no_float_part () =
  Asserter.String_asserter.assert_equals
    "1"
    (Hhbc_string_utils.Float.to_string 1.0)
    "Integral floats are printed without decimal parts";
  true

let test_precision () =
  Asserter.String_asserter.assert_equals
    "1.1000000000000001"
    (Hhbc_string_utils.Float.to_string 1.1)
    "16 decimal places are expected";
  true

let test_no_trailing_zeroes () =
  Asserter.String_asserter.assert_equals
    "1.2"
    (Hhbc_string_utils.Float.to_string 1.2)
    "Trailing zeroes should be omitted";
  true

let test_scientific () =
  Asserter.String_asserter.assert_equals
    "1e+100"
    (Hhbc_string_utils.Float.to_string 1e+100)
    "Big numbers are printed in scientific notation";
  true

let test_scientific_precision () =
  Asserter.String_asserter.assert_equals
    "-2147483648.0001001"
    (Hhbc_string_utils.Float.to_string (-2.1474836480001e9))
    "";
  true

let test_negative_nan () =
  Asserter.String_asserter.assert_equals
    "NAN"
    (Hhbc_string_utils.Float.to_string (-.nan))
    "";
  true

let tests =
  [
    ("test_no_float_part", test_no_float_part);
    ("test_precision", test_precision);
    ("test_no_trailing_zeroes", test_no_trailing_zeroes);
    ("test_scientific", test_scientific);
    ("test_scientific_precision", test_scientific_precision);
    ("test_negative_nan", test_negative_nan);
  ]

let () = Unit_test.run_all tests
