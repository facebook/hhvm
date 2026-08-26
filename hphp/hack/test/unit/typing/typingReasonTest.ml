(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open OUnit2
module Reason = Typing_reason

let nested_lower_bounds depth =
  let leaf = Reason.witness Pos.none in
  let rec loop depth reason =
    if depth = 0 then
      reason
    else
      loop (depth - 1) (Reason.trans_lower_bound ~bound:reason ~of_:leaf)
  in
  loop depth leaf

let test_explanation_of_deep_lower_bounds_is_bounded _ =
  let reason = nested_lower_bounds 48 in
  match
    Reason.explain ~sub:reason ~super:(Reason.witness Pos.none) ~complexity:1
  with
  | Explanation.Derivation elements ->
    assert_bool
      "The rendered explanation should respect its configured depth limit"
      (List.length elements < 100_000)
  | Explanation.Debug _
  | Explanation.Empty ->
    assert_failure "Expected a derivation"

let () =
  "typingReasonTest"
  >::: [
         "test_explanation_of_deep_lower_bounds_is_bounded"
         >:: test_explanation_of_deep_lower_bounds_is_bounded;
       ]
  |> run_test_tt_main
