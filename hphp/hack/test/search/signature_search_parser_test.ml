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
  assert_equal (Some exp) test_case

let assert_parse_error query () =
  let test_case = parse_query query in
  assert_equal None test_case

let parse_test_suite =
  "search_parser" >:::
  [ "string_to_void" >::
    assert_parse
      ~query:"function(string) : void"
      ~exp:{ function_params = [QTtype (TSsimple "string")]
          ; function_output = QTtype (TSsimple "void")
          }

  ; "empty_to_int" >::
    assert_parse
      ~query:"function(): int"
      ~exp:{function_params = []; function_output = QTtype (TSsimple "int")}

  ; "missing_colon" >::
    assert_parse_error "function(int) Container"

  ; "num_and_vec_with_space_to_dict" >::
    assert_parse
      ~query:"function (num    , vec   ): dict       "
      ~exp:{ function_params = [QTtype (TSsimple "num"); QTtype (TSsimple "vec")]
          ; function_output = QTtype (TSsimple "dict")
          }

  ; "vec_to_dict_and_Foo" >::
    assert_parse
      ~query:"function(vec) : dict, Foo"
      ~exp:{ function_params = [QTtype (TSsimple "vec")]
          ; function_output = QTtype (TSsimple "dict, Foo")
          }

  ; "incorrect_input_to_int" >::
    assert_parse
      ~query:"function(,,): int"
      ~exp:{function_params = []; function_output = QTtype (TSsimple "int")}

  ; "function_int_to_void" >::
    assert_parse
      ~query:"function(int): void"
      ~exp:{ function_params = [QTtype (TSsimple "int")]
          ; function_output = QTtype (TSsimple "void")
          }

  ; "missing_function_identifier" >::
    assert_parse_error "(vec): Traversable"

  ; "wildcards" >::
    assert_parse
      ~query:"function(_,_): _"
      ~exp:{function_params = [QTwildcard; QTwildcard]; function_output = QTwildcard}

  ; "option_parameter" >::
    assert_parse
      ~query:"function(?int,?string): _"
      ~exp:{ function_params =
            [ QTtype (TSoption (TSsimple "int"))
            ; QTtype (TSoption (TSsimple "string"))
            ]
          ; function_output = QTwildcard
          }
  ; "too_many_options" >::
    assert_parse_error "function(??int): _"

  ; "option_empty_parameter" >::
    assert_parse_error "function(?): _"
  ; "option_parameter_with_space" >::
    assert_parse
      ~query:"function(? int): _"
      ~exp:{ function_params = [QTtype (TSoption (TSsimple "int"))]
          ; function_output = QTwildcard
          }
  ]

let _ : OUnit.test_result list =
  run_test_tt_main parse_test_suite
