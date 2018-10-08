open Core_kernel

module Test = Integration_test_base

(* Set up foo.php and 10 files that depend on it, generate and load saved state for
 * this repo. *)
let foo_name = "foo.php"
let foo_contents = {|<?hh //strict
function foo(): int {
  return 4;
}
|}

let foo_user_name = Printf.sprintf "foo_user_%d.php"
let foo_user_contents = Printf.sprintf {|<?hh //strict
  function foo_user_%d(): int {
    return foo();
  }
|}


let foo_users =
  List.init 10
  ~f:(fun n -> foo_user_name n, foo_user_contents n)

let go f = Tempfile.with_real_tempdir @@ fun temp_dir ->
  let temp_dir = Path.to_string temp_dir in

  let disk_state = (foo_name, foo_contents)::foo_users in

  (* No changes between saving and loading state *)
  Test.save_state disk_state temp_dir;
  let env = Test.load_state
    temp_dir
    ~disk_state
    ~master_changes:[]
    ~local_changes:[]
    ~use_precheked_files:false
  in
  f env
