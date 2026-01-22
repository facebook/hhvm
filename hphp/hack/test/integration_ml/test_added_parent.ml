(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Integration_test_base_types
module Test = Integration_test_base

let foo_name = "foo.php"

let foo_contents =
  "<?hh // strict

class Foo {
  /* HH_FIXME[4336] */
  public function f() : string {
  }
}
"

let foo_child_name = "foo_child.php"

let foo_child_contents =
  "<?hh // strict

/* HH_FIXME[2049] */
/* HH_FIXME[4123] */
class FooChild extends Foo {}
"

let bar_name = "bar.php"

let bar_contents =
  "<?hh // strict

function take_int(int $x) : void {}

function test(FooChild $foo_child) : void {
  /* HH_FIXME[4053] */
  take_int($foo_child->f());
}
"

let bar_errors =
  "ERROR: File \"/bar.php\", line 7, characters 12-26:
Invalid argument (Typing[4110])
  File \"/bar.php\", line 3, characters 19-21:
  Expected `int`
  File \"/foo.php\", line 5, characters 25-30:
  But got `string`
"

let root = "/"

let hhconfig_filename = Filename.concat root ".hhconfig"

let hhconfig_contents =
  "
allowed_fixme_codes_strict = 2049,4053,4110,4123,4336
allowed_decl_fixme_codes = 2049,4123,4336
"

let test () =
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  TestDisk.set hhconfig_filename hhconfig_contents;
  let (custom_config, _) =
    ServerConfig.load ~silent:false ~from:"" ~cli_config_overrides:[]
  in
  let env =
    Test.setup_server
      ~custom_config
      ~hhi_files:(Hhi.get_raw_hhi_contents () |> Array.to_list)
      ()
  in
  let env =
    Test.setup_disk
      env
      [
        (foo_name, "");
        (foo_child_name, foo_child_contents);
        (bar_name, bar_contents);
      ]
  in
  (* We need to suppress all the errors (see HH_FIXMEs above), otherwise the
   * logic that always rechecks the files with errors kicks in and does the
   * same job as phase2 fanout. We want to test the latter one in this test. *)
  Test.assert_no_diagnostics env;

  (* restore parent, but with a mismatching return type of f() *)
  let (env, _) =
    Test.(
      run_loop_once
        env
        { default_loop_input with disk_changes = [(foo_name, foo_contents)] })
  in
  Test.assertSingleDiagnostic
    bar_errors
    (Diagnostics.get_diagnostic_list env.ServerEnv.diagnostics)
