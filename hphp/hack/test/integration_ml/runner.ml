(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let run (f:unit -> unit): (unit -> bool) =
  (fun () -> f (); true)

let tests = [
  "added_parent" , run Test_added_parent.test;
  "capitalization" , run Test_capitalization.test;
  "coverage_counts" , run Test_coverage_counts.test;
  "coverage_levels_checked" , run Test_coverage_levels_checked.test;
  "coverage_levels_multi" , run Test_coverage_levels_multi.test;
  "coverage_levels_partial" , run Test_coverage_levels_partial.test;
  "coverage_levels_unchecked" , run Test_coverage_levels_unchecked.test;
  "decl_decl" , run Test_decl_decl.test;
  "delete_file" , run Test_delete_file.test;
  "duplicated_file" , run Test_duplicated_file.test;
  "duplicate_parent" , run Test_duplicate_parent.test;
  "failed_decl" , run Test_failed_decl.test;
  "failed_naming" , run Test_failed_naming.test;
  "function_arg_rx_if_implements1" , run Test_function_arg_rx_if_implements1.test;
  "function_arg_rx_if_implements2" , run Test_function_arg_rx_if_implements2.test;
  "gconst_file" , run Test_gconst_file.test;
  "get_dependent_classes" , run Test_get_dependent_classes.test;
  "getfundeps" , run Test_getfundeps.test;
  "ide_utils" , run Test_ide_utils.test;
  "infer_type" , run Test_infer_type.test;
  "interrupt2" , run Test_interrupt2.test;
  "interrupt" , run Test_interrupt.test;
  "isfunlocallable" , run Test_isfunlocallable.test;
  "lazy_decl_idempotence" , run Test_lazy_decl_idempotence.test;
  "modify_file" , run Test_modify_file.test;
  "new_file" , run Test_new_file.test;
  "rx_if_implements" , run Test_rx_if_implements.test;
  "server_hover" , run Test_server_hover.test;
  "unbound_name" , run Test_unbound_name.test;
]

let () =
  if Array.length Sys.argv != 2 then
    failwith "Expecting exactly one test name"
  else
  let test_name = Sys.argv.(1) in
  let tests = List.filter (fun (name, _test) -> test_name = name) tests in
  Unit_test.run_all tests
