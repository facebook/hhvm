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

let documents =
  [ "\\int_to_string", ["arity=1"; "arg1<:int"; "ret<:string"]
  ; "\\sum"          , ["arity=2"; "arg1<:int"; "arg2<:int"; "ret<:int"]
  ; "\\C\\count"     , ["arity=1"; "arg1<:Container"; "ret<:int"]
  ; "\\get_string"   , ["arity=0"; "ret<:string"]
  ; "\\set_string"   , ["arity=1"; "arg1<:string"; "ret<:void"]
  ]

let verify_query index query exp =
  assert_equal exp (get index query)

let assert_results
    ~search_term
    ~exp
    () =
  let index = make () in
  List.iter documents ~f:(fun (name, terms) ->
    update index name terms
  );
  verify_query index search_term exp

let index_test_suite =
  "update_index" >:::
  [ "no_results" >::
    assert_results
      ~search_term:(Term "arity=3")
      ~exp:[]

  ; "valid_return_type" >::
    assert_results
      ~search_term:(Term "ret<:void")
      ~exp:["\\set_string"]

  ; "identical_search_terms" >::
    begin fun () ->
      let index = make () in
      let identical_funs =
        [ "\\identical_fun", ["ret<:string"]
        ; "\\identical_fun", ["ret<:string"]
        ; "\\identical_fun", ["ret<:string"]
        ]
      in
      List.iter identical_funs ~f:(fun (name, terms) ->
        update index name terms
      );
      verify_query index (Term "ret<:string") ["\\identical_fun"]
    end

  ; "verify_update" >::
    assert_results
      ~search_term:(Term "arg1<:int")
      ~exp:["\\int_to_string"; "\\sum"]

  ; "invalid_query" >::
    assert_results
      ~search_term:(Term "arg0<:")
      ~exp:[]
  ]

let _ : OUnit.test_result list =
  run_test_tt_main index_test_suite
