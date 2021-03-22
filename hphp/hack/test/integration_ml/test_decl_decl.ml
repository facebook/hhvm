open Integration_test_base_types
module Test = Integration_test_base

(* The bug only occurs when A and B are in different files, are redeclared in
 * the same batch, but B (the child) is redeclared before A (so that decl of B
 * ends up having to call into decl of A). This is achieved here using an
 * implementation detail of files being processed in reverse alphabetical order.
 *)
let a_file_name = "A.php"

let b_file_name = "B.php"

let c_file_name = "C.php"

let a_contents =
  "<?hh // strict

class A {
  /* HH_FIXME[4336] */
  /* HH_FIXME[4422] */
  final public function f() : C {
  }
}
"

let b_contents = "<?hh // strict

class B extends A {}
"

let c_contents = "<?hh // strict

class C {}
"

let errors =
  {|
File "/A.php", line 6, characters 31-31:
Unbound name: `C` (Naming[2049])
|}

let root = "/"

let hhconfig_filename = Filename.concat root ".hhconfig"

let hhconfig_contents =
  "
allowed_fixme_codes_strict = 4336,4422
allowed_decl_fixme_codes = 4336,4422
"

let test () =
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  TestDisk.set hhconfig_filename hhconfig_contents;
  let hhconfig_path =
    Relative_path.create Relative_path.Root hhconfig_filename
  in
  let options = ServerArgs.default_options ~root in
  let (custom_config, _) =
    ServerConfig.load ~silent:false hhconfig_path options
  in
  let env = Test.setup_server ~custom_config () in
  let env =
    Test.setup_disk
      env
      [(a_file_name, a_contents); (b_file_name, b_contents); (c_file_name, "")]
  in
  Test.assert_env_errors env errors;

  let (env, _) =
    Test.(
      run_loop_once
        env
        { default_loop_input with disk_changes = [(c_file_name, c_contents)] })
  in
  Test.assert_no_errors env;
  ()
