module Test = Integration_test_base

let make_bar = Printf.sprintf {|<?hh // %s

5 + 5;

function bar(): void {}
|}

let bar_error =
  "File \"/bar.php\", line 3, characters 1-6:\n"
  ^ "Toplevel statements are not allowed. Use __EntryPoint attribute instead (Parsing[1002])\n\n"

let test_parsing_error () : unit =
  Tempfile.with_real_tempdir @@ fun temp_dir ->
  let temp_dir = Path.to_string temp_dir in
  let disk_state = [("foo.php", "<?hh\nfunction foo(): void {}")] in
  let before_disk_state = ("bar.php", make_bar "partial") :: disk_state in
  let after_disk_state = ("bar.php", make_bar "strict") :: disk_state in
  Test.save_state_with_errors before_disk_state temp_dir "";

  let env =
    Test.load_state
      temp_dir
      ~disk_state:after_disk_state
      ~master_changes:["bar.php"]
      ~local_changes:[]
      ~use_precheked_files:false
  in
  (* Make sure there's only the errors we expect *)
  Test.assert_env_errors env bar_error;

  let (_ : ServerEnv.env) = Test.assert_errors_in_phase env 1 Errors.Parsing in
  ()

let test () = Test.in_daemon @@ test_parsing_error
