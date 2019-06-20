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
  "saved_state/deps_all_members" , run Test_deps_all_members.test;
  "saved_state/disable_conservative_redecl_class" , run Test_disable_conservative_redecl_class.test;
  "saved_state/disk_race_conditions" , run Test_disk_race_conditions.test;
  "saved_state/fun_deps_load_from_state" , run Test_fun_deps_load_from_state.test;
  "saved_state/ide_cache" , run Test_ide_cache.test;
  "saved_state/ide_tast_cache" , run Test_ide_tast_cache.test;
  "saved_state/load_decls_cold_synthesized_ancestors" , run Test_load_decls_cold_synthesized_ancestors.test;
  "saved_state/load_decls_enum_add_member" , run Test_load_decls_enum_add_member.test;
  "saved_state/load_decls_fixme_in_hot_changed_class" , run Test_load_decls_fixme_in_hot_changed_class.test;
  "saved_state/load_decls_fixme_in_hot_similar_class" , run Test_load_decls_fixme_in_hot_similar_class.test;
  "saved_state/load_decls_fixme_in_hot_unchanged_interface" , run Test_load_decls_fixme_in_hot_unchanged_interface.test;
  "saved_state/load_decls_stale_derived_class" , run Test_load_decls_stale_derived_class.test;
  "saved_state/naming_table_sqlite_fallback" , run Test_naming_table_sqlite_fallback.test;
  "saved_state/no_op_close" , run Test_no_op_close.test;
  "saved_state/no_op_edit" , run Test_no_op_edit.test;
  "saved_state/no_op_open" , run Test_no_op_open.test;
  "saved_state/prechecked_advanced" , run Test_prechecked_advanced.test;
  "saved_state/prechecked_basic" , run Test_prechecked_basic.test;
  "saved_state/prechecked_find_refs" , run Test_prechecked_find_refs.test;
  "saved_state/prechecked_incremental_after_init" , run Test_prechecked_incremental_after_init.test;
  "saved_state/prechecked_incremental" , run Test_prechecked_incremental.test;
  "saved_state/predeclare_ide_deps" , run Test_predeclare_ide_deps.test;
  "saved_state/recheck_stats" , run Test_recheck_stats.test;
  "saved_state/saved_state" , run Test_saved_state.test;
  "saved_state/saved_state_with_decl_error" , run Test_saved_state_with_decl_error.test;
  "saved_state/saved_state_with_mode_change" , run Test_saved_state_with_mode_change.test;
  "saved_state/saved_state_with_naming_error" , run Test_saved_state_with_naming_error.test;
  "saved_state/saved_state_with_parse_error" , run Test_saved_state_with_parse_error.test;
  "saved_state/similar_files" , run Test_similar_files.test;
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
