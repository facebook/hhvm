(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
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
"<?hh

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
    ~tco_unsafe_xhp: false
    ~tco_user_attrs: None
    ~tco_experimental_features: GlobalOptions.tco_experimental_all
    ~po_auto_namespace_map:
      [("ShortName", "HH\\LongName\\EvenLonger\\ShortName")]
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
