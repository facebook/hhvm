(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

module Test = Integration_test_base

let foo_name = "foo.php"

let foo_contents =
"<?hh // strict

namespace HH\\LongName\\EvenLonger\\ShortName;

function foo() : void {}
"

let autocomplete_contents1 =
"<?hh // strict

function testTypecheck(): void {
  ShortName\\foAUTO332;
}
"

let autocomplete_contents2 =
"<?hh

function testTypecheck(): void {
  \\ShortName\\foAUTO332;
}
"

let () =

  let global_opts = GlobalOptions.make
    ~tco_assume_php: false
    ~tco_safe_array: false
    ~tco_safe_vector_array: false
    ~tco_user_attrs: None
    ~tco_experimental_features: GlobalOptions.tco_experimental_all
    ~tco_migration_flags: SSet.empty
    ~tco_dynamic_view: false
    ~tco_disallow_unsafe_comparisons: false
    ~tco_disallow_non_arraykey_keys: false
    ~tco_disallow_array_as_tuple: false
    ~po_auto_namespace_map:
      [("ShortName", "HH\\LongName\\EvenLonger\\ShortName")]
    ~po_deregister_php_stdlib: true
    ~po_use_full_fidelity:true
    ~tco_disallow_ambiguous_lambda:false
    ~tco_disallow_array_typehint:false
    ~tco_disallow_array_literal:false
    ~tco_language_feature_logging:false
    ~po_disallow_elvis_space:false
    ~ignored_fixme_codes: ISet.empty
  in
  let custom_config = ServerConfig.default_config in
  let custom_config = ServerConfig.set_parser_options
    custom_config
    global_opts in
  let env = Test.setup_server ~custom_config () in
  let env = Test.setup_disk env [
    foo_name, foo_contents;
  ] in

  let env = Test.connect_persistent_client env in

  let _, loop_output = Test.autocomplete env autocomplete_contents1 in
  Test.assert_autocomplete loop_output ["ShortName\\foo"];

  let _, loop_output = Test.autocomplete env autocomplete_contents2 in
  Test.assert_autocomplete loop_output ["ShortName\\foo"]
