open Integration_test_base_types
open ServerEnv
module Test = Integration_test_base

let init_base_content =
  "<?hh // partial
enum DynamicTemplateField : string {
  BRAND = 'brand';
  DESCRIPTION = 'description';
  // NAME = 'name';
}"

let err_base_content =
  "<?hh // partial
enum DynamicTemplateField : string {
  BRAND = 'brand';
  DESCRIPTION = 'description';
  NAME = 'name';
}"

let make_disk_changes base_content =
  [
    ("base.php", base_content);
    ( "test.php",
      "<?hh // strict

function test(
  DynamicTemplateField $field,
): string {
  // Switch enforces enum exhaustiveness
  switch ($field) {
    case DynamicTemplateField::BRAND:
      return 'Brand';
    case DynamicTemplateField::DESCRIPTION:
      return 'Description';
  }
}"
    );
  ]

let errors_to_string errors =
  List.fold_left
    (fun str error -> str ^ Errors.(error |> to_absolute |> to_string))
    ""
  @@ errors

let () =
  let env = Test.setup_server () in
  (* Initially we expect no errors *)
  let (env, _) =
    Test.(
      run_loop_once
        env
        {
          default_loop_input with
          disk_changes = make_disk_changes init_base_content;
        })
  in
  let errors = Errors.get_sorted_error_list env.errorl in
  if errors <> [] then
    Test.fail ("Expected no errors. Got:\n" ^ errors_to_string errors);

  (* We expect errors when we change base.php to err_base_content *)
  let (env, _) =
    Test.(
      run_loop_once
        env
        {
          default_loop_input with
          disk_changes = make_disk_changes err_base_content;
        })
  in
  let expected_errors = Errors.get_sorted_error_list env.errorl in
  if expected_errors = [] then Test.fail "Expected there to be errors!";

  (* We reset the disk changes to the initial state. Should be no errors *)
  let (env, _) =
    Test.(
      run_loop_once
        env
        {
          default_loop_input with
          disk_changes = make_disk_changes init_base_content;
        })
  in
  let errors = Errors.get_sorted_error_list env.errorl in
  if errors <> [] then
    Test.fail ("Expected no errors. Got:\n" ^ errors_to_string errors);

  (* We now change only base.php. We expect the same errors as before *)
  let (env, _) =
    Test.(
      run_loop_once
        env
        {
          default_loop_input with
          disk_changes = [("base.php", err_base_content)];
        })
  in
  let incremental_errors = Errors.get_sorted_error_list env.errorl in
  if incremental_errors <> expected_errors then
    Test.fail
      ( "Incremental mode gave different errors than a full type check.\n\n"
      ^ "Full Type Check Errors:\n"
      ^ errors_to_string expected_errors
      ^ "\n"
      ^ "Incremental Mode Errors:\n"
      ^ errors_to_string incremental_errors )
