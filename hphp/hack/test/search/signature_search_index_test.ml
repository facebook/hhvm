(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open OUnit
open SignatureSearchIndex

let default_terms =
  [ "\\int_to_string",
    [ Arity 1
    ; Parameter {position = 1; type_ = ITprim Nast.Tint}
    ; Return_type (ITprim Nast.Tstring)
    ]

  ; "\\sum",
    [ Arity 2
    ; Parameter {position = 1; type_ = ITprim Nast.Tint}
    ; Parameter {position = 2; type_ = ITprim Nast.Tint}
    ; Return_type (ITprim Nast.Tint)
    ]

  ; "\\C\\count",
    [ Arity 1
    ; Parameter {position = 1; type_ = ITapply "Container"}
    ; Return_type (ITprim Nast.Tint)
    ]

  ; "\\get_string",
    [ Arity 0
    ; Return_type (ITprim Nast.Tstring)
    ]

  ; "\\set_string",
    [ Arity 1
    ; Parameter {position = 1; type_ = ITprim Nast.Tstring}
    ; Return_type (ITprim Nast.Tvoid)
    ]
  ]

let verify_query index query exp =
  let results = get index query in
  assert_equal exp (SSet.elements results)

let assert_results
    ~search_term
    ~exp
    () =
  let index = make () in
  List.iter default_terms ~f:(fun (name, terms) ->
    update index name terms
  );
  verify_query index search_term exp

let index_test_suite =
  "update_index" >:::
  [ "no_results" >::
    assert_results
      ~search_term:(Arity 3)
      ~exp:[]

  ; "valid_return_type" >::
    assert_results
      ~search_term:(Return_type (ITprim Nast.Tvoid))
      ~exp:["\\set_string"]

  ; "identical_search_terms" >::
    begin fun () ->
      let index = make () in
      let identical_funs =
        [ "\\identical_fun",
          [ Arity 1
          ; Parameter {position = 1; type_ = ITapply "vec"}
          ; Return_type (ITprim Nast.Tstring)
          ]

        ; "\\identical_fun",
          [ Arity 1
          ; Parameter {position = 1; type_ = ITapply "vec"}
          ; Return_type (ITprim Nast.Tstring)
          ]

        ; "\\identical_fun",
          [ Arity 1
          ; Parameter {position = 1; type_ = ITapply "vec"}
          ; Return_type (ITprim Nast.Tstring)
          ]
        ]
      in
      List.iter identical_funs ~f:(fun (name, terms) ->
        update index name terms
      );
      let search_term = Return_type (ITprim Nast.Tstring) in
      verify_query index search_term ["\\identical_fun"]
    end

  ; "verify_update" >::
    assert_results
      ~search_term:(Parameter {position = 1; type_ = ITprim Nast.Tint})
      ~exp:["\\int_to_string"; "\\sum"]

  ; "invalid_query" >::
    assert_results
      ~search_term:(Parameter {position = 0; type_ = ITapply ""})
      ~exp:[]
  ]

let _ : OUnit.test_result list =
  run_test_tt_main index_test_suite
