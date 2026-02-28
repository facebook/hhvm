open Integration_test_base_types
module Test = Integration_test_base

let root = "/"

let hhconfig_filename = Filename.concat root ".hhconfig"

let hhconfig_contents = "
new_exhaustivity_check = true
"

let e_contents = "<?hh

enum E: int {
  A = 1;
  B = 2;
}
"

let e_changes = "<?hh

enum E: int {
  A = 1;
}
"

let f_contents =
  "<?hh

function f(E $e): int {
  switch ($e) {
    case E::A:
      return 4;
  }
  return 42;
}
"

let f_errors =
  "ERROR: File \"/f.php\", line 4, characters 3-50:
This `switch` statement is nonexhaustive. At least the following cases are missing: `E::B` (Typing[4019])
  File \"/f.php\", line 4, characters 11-12:
  The scrutinee has type `E`.
"

let test () =
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  TestDisk.set hhconfig_filename hhconfig_contents;
  let (custom_config, _) =
    ServerConfig.load ~silent:false ~from:"" ~cli_config_overrides:[]
  in
  let env = Test.setup_server ~custom_config () in
  let env =
    Test.setup_disk env [("e.php", e_contents); ("f.php", f_contents)]
  in
  Test.assertSingleDiagnostic
    f_errors
    (Diagnostics.get_diagnostic_list env.ServerEnv.diagnostics);

  let (env, _) =
    Test.(
      run_loop_once
        env
        { default_loop_input with disk_changes = [("e.php", e_changes)] })
  in
  Test.assert_no_diagnostics env
