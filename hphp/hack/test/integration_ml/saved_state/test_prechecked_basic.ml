module Test = Integration_test_base
(* Simplest prechecked files scenario: master and local changes are completely
 * independent, and there is no incremental mode involved. *)

let x_contents = Printf.sprintf {|<?hh
function %s(): %s {
  //UNSAFE_BLOCK
}
|}

let y_contents = Printf.sprintf {|<?hh
function %s(): int { return %s(); }
|}

let d_errors = {|
File "/d.php", line 2, characters 28-30:
Invalid return type (Typing[4110])
File "/d.php", line 2, characters 15-17:
This is an int
File "/c.php", line 2, characters 15-20:
It is incompatible with a string
|}

let () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  let temp_dir = Path.to_string temp_dir in
    (* - "b" depends on "a"
     * - "d" depends on "c"
     *   - both pairs are independent from each other *)

  Test.save_state [
    "a.php", x_contents "a" "int";
    "b.php", y_contents "b" "a";
    "c.php", x_contents "c" "int";
    "d.php", y_contents "d" "c";
  ] temp_dir;

  (* "a" and "c" change in a way that generates errors in "b" and "d". But since
   * "a" change is among master, "prechecked" files, we don't fan-out to "b" and
   * only see error in "d". *)
  let env = Test.load_state
    ~saved_state_dir:temp_dir
    ~disk_state:[
      "a.php", x_contents "a" "string";
      "b.php", y_contents "b" "a";
      "c.php", x_contents "c" "string";
      "d.php", y_contents "d" "c";
      ]
    ~master_changes:["a.php"]
    ~local_changes:["c.php"]
    ~use_precheked_files:true
  in

  Test.assert_needs_recheck env "a.php";
  Test.assert_needs_recheck env "c.php";
  Test.assert_needs_no_recheck env "b.php";
  Test.assert_needs_no_recheck env "d.php";

  ServerMain.force_break_recheck_loop_for_test true;
  let env, _ = Test.full_check env in

  Test.assert_needs_no_recheck env "a.php";
  Test.assert_needs_no_recheck env "b.php"; (* important part: no "b" *)
  Test.assert_needs_no_recheck env "c.php";
  Test.assert_needs_recheck env "d.php";

  let env, _ = Test.(run_loop_once env default_loop_input) in

  Test.assert_env_errors env d_errors;
  ()
