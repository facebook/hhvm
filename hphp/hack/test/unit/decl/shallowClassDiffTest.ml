(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open OUnit2

let parser_options =
  let parser_options =
    ParserOptions.{ default with keep_user_attributes = true }
  in
  Decl_parser_options.from_parser_options parser_options

let parse_class source =
  let { Direct_decl_parser.pf_decls; _ } =
    Direct_decl_parser.parse_decls parser_options Relative_path.default source
  in
  match
    List.find_map pf_decls ~f:(fun (_, decl) ->
        match decl with
        | Shallow_decl_defs.Class class_ -> Some class_
        | _ -> None)
  with
  | Some class_ -> class_
  | None -> assert_failure "Expected a class declaration"

let diff_class ~enable_annotation_agnostic_decl_diffing old_source new_source =
  Shallow_class_diff.diff_class
    ~enable_annotation_agnostic_decl_diffing
    (parse_class old_source)
    (parse_class new_source)

let assert_major_change = function
  | Some (Class_diff.Major_change _) -> ()
  | Some (Class_diff.Minor_change _) ->
    assert_failure "Expected a major change, got a minor change"
  | None -> assert_failure "Expected a major change, got no change"

let assert_no_change = function
  | None -> ()
  | Some _ -> assert_failure "Expected no change"

let test_custom_attribute_change_is_major_when_disabled _ =
  diff_class
    ~enable_annotation_agnostic_decl_diffing:false
    "<?hh\n<<Oncalls('old')>>\nclass C {}\n"
    "<?hh\n<<Oncalls('new')>>\nclass C {}\n"
  |> assert_major_change

let test_custom_attribute_change_is_ignored_when_enabled _ =
  diff_class
    ~enable_annotation_agnostic_decl_diffing:true
    "<?hh\n<<Oncalls('old')>>\nclass C {}\n"
    "<?hh\n<<Oncalls('new')>>\nclass C {}\n"
  |> assert_no_change

let test_custom_attribute_addition_is_ignored_when_enabled _ =
  diff_class
    ~enable_annotation_agnostic_decl_diffing:true
    "<?hh\nclass C {}\n"
    "<?hh\n<<Oncalls('team')>>\nclass C {}\n"
  |> assert_no_change

let test_builtin_attribute_change_remains_major_when_enabled _ =
  diff_class
    ~enable_annotation_agnostic_decl_diffing:true
    "<?hh\nclass C {}\n"
    "<?hh\n<<__Const>>\nclass C {}\n"
  |> assert_major_change

let test_enable_annotation_agnostic_decl_diffing_is_a_typechecker_option _ =
  let options =
    GlobalOptions.set
      ~tco_enable_annotation_agnostic_decl_diffing:true
      GlobalOptions.default
  in
  assert_bool
    "Expected annotation-agnostic decl diffing to be enabled"
    (TypecheckerOptions.enable_annotation_agnostic_decl_diffing options)

let () =
  "shallowClassDiffTest"
  >::: [
         "custom_attribute_change_is_major_when_disabled"
         >:: test_custom_attribute_change_is_major_when_disabled;
         "custom_attribute_change_is_ignored_when_enabled"
         >:: test_custom_attribute_change_is_ignored_when_enabled;
         "custom_attribute_addition_is_ignored_when_enabled"
         >:: test_custom_attribute_addition_is_ignored_when_enabled;
         "builtin_attribute_change_remains_major_when_enabled"
         >:: test_builtin_attribute_change_remains_major_when_enabled;
         "enable_annotation_agnostic_decl_diffing_is_a_typechecker_option"
         >:: test_enable_annotation_agnostic_decl_diffing_is_a_typechecker_option;
       ]
  |> run_test_tt_main
