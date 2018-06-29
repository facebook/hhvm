(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open OUnit
open SignatureSearchParser

let assert_parse ~query ~exp () =
  let test_case = parse_query query in
  assert_equal exp test_case

let parse_test_suite =
  "search_parser" >:::
  [ "string_to_void" >::
    assert_parse
      ~query:"function(string) : void"
      ~exp:(Some {function_params = ["string"]; function_output = "void"})

  ; "empty_to_int" >::
    assert_parse
      ~query:"function(): int"
      ~exp:(Some {function_params = []; function_output = "int"})

  ; "missing_colon" >::
    assert_parse
      ~query:"function(int) Container"
      ~exp:None

  ; "num_and_vec_with_space_to_dict" >::
    assert_parse
      ~query:"function (num    , vec   ): dict       "
      ~exp:(Some {function_params = ["num"; "vec"]; function_output = "dict"})

  ; "vec_to_dict_and_Foo" >::
    assert_parse
      ~query:"function(vec) : dict, Foo"
      ~exp:(Some {function_params = ["vec"]; function_output = "dict, Foo"})

  ; "incorrect_input_to_int" >::
    assert_parse
      ~query:"function(,,): int"
      ~exp:(Some {function_params = []; function_output = "int"})

  ; "function_int_to_void" >::
    assert_parse
      ~query:"function(int): void"
      ~exp:(Some {function_params = ["int"]; function_output = "void"})

  ; "missing_function_identifier" >::
    assert_parse
      ~query:"(vec): Traversable"
      ~exp:None
  ]

let _ : OUnit.test_result list =
  run_test_tt_main parse_test_suite
