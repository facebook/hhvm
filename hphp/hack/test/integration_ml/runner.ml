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
    ("ide/added_parent", run Test_added_parent_ide.test);
    ("ide/auto_ns_aliasing", run Test_auto_ns_aliasing.test);
    ("ide/autoclose_xhp", run Test_autoclose_xhp.test);
    ("ide/diagnostics_in_closed_file", run Test_diagnostics_in_closed_file.test);
    ("ide/error_pos", run Test_error_pos.test);
    ("ide/failed_naming", run Test_failed_naming_ide.test);
    ("ide/ide_close", run Test_ide_close.test);
    ("ide/ide_consistency", run Test_ide_consistency.test);
    ( "ide/ide_diagnostic_subscription",
      run Test_ide_diagnostic_subscription.test );
    ("ide/ide_file_sync", run Test_ide_file_sync.test);
    ("ide/identify_symbol", run Test_identify_symbol.test);
    ("ide/ide_parsing_errors", run Test_ide_parsing_errors.test);
    ("ide/naming_errors", run Test_naming_errors.test);
    ("ide/override", run Test_override.test);
    ("ide/remove_function", run Test_remove_function.test);
    ("ide/remove_parent", run Test_remove_parent.test);
    ("ide/unsaved_changes", run Test_unsaved_changes.test);
    ("added_parent", run Test_added_parent.test);
    ("remove_method", run Test_remove_method.test);
    ("capitalization", run Test_capitalization.test);
    ("coeffects", run Test_coeffects.test);
    ("decl_decl", run Test_decl_decl.test);
    ("delete_file", run Test_delete_file.test);
    ("duplicated_file", run Test_duplicated_file.test);
    ("duplicate_parent", run Test_duplicate_parent.test);
    ("failed_naming", run Test_failed_naming.test);
    ("funptr", run Test_funptr.test);
    ("gconst_file", run Test_gconst_file.test);
    ("get_dependent_classes", run Test_get_dependent_classes.test);
    ("getfundeps", run Test_getfundeps.test);
    ("identify", run Test_identify.test);
    ("ignore_fixme_hhi", run Test_ignore_fixme_hhi.test);
    ("infer_type", run Test_infer_type.test);
    ("interrupt2", run Test_interrupt2.test);
    ("interrupt", run Test_interrupt.test);
    ("lazy_decl_idempotence", run Test_lazy_decl_idempotence.test);
    ("modify_file", run Test_modify_file.test);
    ("new_file", run Test_new_file.test);
    ("property_initializer", run Test_property_initializer.test);
    ("property_initializer2", run Test_property_initializer2.test);
    ("server_config_overrides", run Test_serverConfig_overrides.test);
    ("server_hover", run Test_server_hover.test);
    ("unbound_name", run Test_unbound_name.test);
    ("unbound_name_2", run Test_unbound_name_2.test);
    ("unbound_name_3", run Test_unbound_name_3.test);
  ]

let () = Runner_base.go tests
