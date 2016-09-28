(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open File_content

let expect_has_content fc content =
  let content_fc = get_content fc in
  content = content_fc

let test_create () =
  let content = "for test\n" in
  let fc = of_content ~content in
  expect_has_content fc content

let test_basic_edit () =
  let content = "for test\n" in
  let fc = of_content ~content in
  let edit = {
    range = Some {
      st = {line = 1; column = 1};
      ed = {line = 1; column = 1}};
    text = "just "} in
  let edited_fc = edit_file_unsafe fc [edit] in
  expect_has_content edited_fc "just for test\n"

let test_basic_edit2 () =
  let content = "for test\n" in
  let fc = of_content ~content in
  let edit = {
    range = Some {
      st = {line = 1; column = 2};
      ed = {line = 1; column = 4}};
    text = "ree"} in
  let edited_fc = edit_file_unsafe fc [edit] in
  expect_has_content edited_fc "free test\n"

let test_multi_line_edit () =
  let content = "aaaa\ncccc\n" in
  let fc = of_content ~content in
  let edit = {
    range = Some {
      st = {line = 1; column = 4};
      ed = {line = 2; column = 2}};
    text = "b\nbbbb\nb"} in
  let edited_fc = edit_file_unsafe fc [edit] in
  expect_has_content edited_fc "aaab\nbbbb\nbccc\n"

let test_multi_line_edit2 () =
  let content = "aaaa\ncccc\n" in
  let fc = of_content ~content in
  let edit = {
    range = Some {
      st = {line = 1; column = 1};
      ed = {line = 3; column = 1}};
    text = ""} in
  let edited_fc = edit_file_unsafe fc [edit] in
  expect_has_content edited_fc ""

let test_special_edit () =
  let content = "\n\n\n" in
  let fc = of_content ~content in
  let edit = {
    range = Some {
      st = {line = 2; column = 1};
      ed = {line = 3; column = 1}};
    text = "aaa\nbbb\n"} in
  let edited_fc = edit_file_unsafe fc [edit] in
  expect_has_content edited_fc "\naaa\nbbb\n\n"

let test_multiple_edits () =
  let content = "a\nc" in
  let fc = of_content ~content in
  let edit1 = {
    range = Some {
      st = {line = 1; column = 2};
      ed = {line = 3; column = 1}};
    text = "aaa\ncccc\n"} in
  let edit2 = {
    range = Some {
      st = {line = 1; column = 4};
      ed = {line = 2; column = 2}};
    text = "b\nbbbb\nb"} in
  let edited_fc = edit_file_unsafe fc [edit1;edit2] in
  expect_has_content edited_fc "aaab\nbbbb\nbccc\n"

let test_invalid_edit () =
  let content = "for test\n" in
  let fc = of_content ~content in
  let edit = {
    range = Some {
      st = {line = 1; column = 15};
      ed = {line = 2; column = 1}};
    text = "just "} in
  match edit_file fc [edit] with
  | Result.Error _ -> true
  | Result.Ok _ -> false

let test_empty_edit () =
  let content = "" in
  let fc = of_content ~content in
  let edit = {
    range = Some {
      st = { line = 1; column = 1};
      ed = { line = 1; column = 1};
    };
    text = "lol";
  } in
  let edited_fc = edit_file_unsafe fc [edit] in
  expect_has_content edited_fc "lol"

let tests = [
  "test_create", test_create;
  "test_basic_edit", test_basic_edit;
  "test_basic_edit2", test_basic_edit2;
  "test_multi_line_edit", test_multi_line_edit;
  "test_multi_line_edit2", test_multi_line_edit2;
  "test_special_edit", test_special_edit;
  "test_multiple_edits", test_multiple_edits;
  "test_invalid_edit", test_invalid_edit;
  "test_empty_edit", test_empty_edit;
]

let () =
  Unit_test.run_all tests
