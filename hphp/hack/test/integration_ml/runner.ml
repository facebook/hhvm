(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let run = Runner_base.run

let tests =
  [
    ("ide/added_parent", run Test_added_parent.test);
    ("ide/auto_ns_aliasing", run Test_auto_ns_aliasing.test);
    ("ide/diagnostics_in_closed_file", run Test_diagnostics_in_closed_file.test);
    ("ide/error_pos", run Test_error_pos.test);
    ("ide/error_throttling", run Test_error_throttling.test);
    ("ide/error_throttling_open_file", run Test_error_throttling_open_file.test);
    ("ide/exception_handling", run Test_exception_handling.test);
    ("ide/failed_naming", run Test_failed_naming.test);
    ("ide/failed_parsing", run Test_failed_parsing.test);
    ("ide/hhi_phpstdlib", run Test_hhi_phpstdlib.test);
    ("ide/ide_check", run Test_ide_check.test);
    ("ide/ide_close", run Test_ide_close.test);
    ("ide/ide_consistency", run Test_ide_consistency.test);
    ( "ide/ide_diagnostic_subscription",
      run Test_ide_diagnostic_subscription.test );
    ("ide/ide_disk", run Test_ide_disk.test);
    ("ide/ide_file_sync", run Test_ide_file_sync.test);
    ("ide/identify_symbol", run Test_identify_symbol.test);
    ("ide/ide_parsing_errors", run Test_ide_parsing_errors.test);
    ("ide/ide_redecl", run Test_ide_redecl.test);
    ("ide/ide_status", run Test_ide_status.test);
    ("ide/ide_typing_deps", run Test_ide_typing_deps.test);
    ("ide/max_errors", run Test_max_errors.test);
    ("ide/naming_errors", run Test_naming_errors.test);
    ("ide/override", run Test_override.test);
    ("ide/remove_function", run Test_remove_function.test);
    ("ide/remove_parent", run Test_remove_parent.test);
    ("ide/status_single", run Test_status_single.test);
    ("ide/unsaved_changes", run Test_unsaved_changes.test);
    ("added_parent", run Test_added_parent.test);
    ("capitalization", run Test_capitalization.test);
    ("coverage_counts", run Test_coverage_counts.test);
    ("coverage_levels_checked", run Test_coverage_levels_checked.test);
    ("coverage_levels_multi", run Test_coverage_levels_multi.test);
    ("coverage_levels_partial", run Test_coverage_levels_partial.test);
    ("coverage_levels_unchecked", run Test_coverage_levels_unchecked.test);
    ("decl_decl", run Test_decl_decl.test);
    ("delete_file", run Test_delete_file.test);
    ("duplicated_file", run Test_duplicated_file.test);
    ("duplicate_parent", run Test_duplicate_parent.test);
    ("failed_decl", run Test_failed_decl.test);
    ("failed_naming", run Test_failed_naming.test);
    ( "function_arg_rx_if_implements1",
      run Test_function_arg_rx_if_implements1.test );
    ( "function_arg_rx_if_implements2",
      run Test_function_arg_rx_if_implements2.test );
    ("gconst_file", run Test_gconst_file.test);
    ("get_dependent_classes", run Test_get_dependent_classes.test);
    ("getfundeps", run Test_getfundeps.test);
    ("infer_type", run Test_infer_type.test);
    ("interrupt2", run Test_interrupt2.test);
    ("interrupt", run Test_interrupt.test);
    ("isfunlocallable", run Test_isfunlocallable.test);
    ("lazy_decl_idempotence", run Test_lazy_decl_idempotence.test);
    ("modify_file", run Test_modify_file.test);
    ("new_file", run Test_new_file.test);
    ("rx_if_implements", run Test_rx_if_implements.test);
    ("server_config_overrides", run Test_serverConfig_overrides.test);
    ("server_hover", run Test_server_hover.test);
    ("unbound_name", run Test_unbound_name.test);
    ("unbound_name_2", run Test_unbound_name_2.test);
  ]

let () = Runner_base.go tests
