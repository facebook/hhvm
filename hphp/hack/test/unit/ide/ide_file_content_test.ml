(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core
open Ide_api_types
open File_content

let expect_has_content fc content =
  content = fc

let test_basic_edit () =
  let content = "for test\n" in
  let edit = {
    range = Some {
      st = {line = 1; column = 1};
      ed = {line = 1; column = 1}};
    text = "just "} in
  let edited_fc = edit_file_unsafe content [edit] in
  expect_has_content edited_fc "just for test\n"

let test_basic_edit2 () =
  let content = "for test\n" in
  let edit = {
    range = Some {
      st = {line = 1; column = 2};
      ed = {line = 1; column = 4}};
    text = "ree"} in
  let edited_fc = edit_file_unsafe content [edit] in
  expect_has_content edited_fc "free test\n"

let test_multi_line_edit () =
  let content = "aaaa\ncccc\n" in
  let edit = {
    range = Some {
      st = {line = 1; column = 4};
      ed = {line = 2; column = 2}};
    text = "b\nbbbb\nb"} in
  let edited_fc = edit_file_unsafe content [edit] in
  expect_has_content edited_fc "aaab\nbbbb\nbccc\n"

let test_multi_line_edit2 () =
  let content = "aaaa\ncccc\n" in
  let edit = {
    range = Some {
      st = {line = 1; column = 1};
      ed = {line = 3; column = 1}};
    text = ""} in
  let edited_fc = edit_file_unsafe content [edit] in
  expect_has_content edited_fc ""

let test_special_edit () =
  let content = "\n\n\n" in
  let edit = {
    range = Some {
      st = {line = 2; column = 1};
      ed = {line = 3; column = 1}};
    text = "aaa\nbbb\n"} in
  let edited_fc = edit_file_unsafe content [edit] in
  expect_has_content edited_fc "\naaa\nbbb\n\n"

let test_multiple_edits () =
  let content = "a\nc\n" in
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
  let edited_fc = edit_file_unsafe content [edit1;edit2] in
  expect_has_content edited_fc "aaab\nbbbb\nbccc\n"

let test_invalid_edit () =
  let content = "for test\n" in
  let edit = {
    range = Some {
      st = {line = 1; column = 15};
      ed = {line = 2; column = 1}};
    text = "just "} in
  match edit_file content [edit] with
  | Result.Error _ -> true
  | Result.Ok _ -> false

let test_empty_edit () =
  let content = "" in
  let edit = {
    range = Some {
      st = { line = 1; column = 1};
      ed = { line = 1; column = 1};
    };
    text = "lol";
  } in
  let edited_fc = edit_file_unsafe content [edit] in
  expect_has_content edited_fc "lol"

let test_end_of_line_edit () =
  let content = "a" in
  let edit = {
    range = Some {
      st = { line = 1; column = 2};
      ed = { line = 1; column = 2};
    };
    text = "a";
  } in
  let edited_fc = edit_file_unsafe content [edit] in
  expect_has_content edited_fc "aa"

let utf8 x =
  String.concat "" (List.map x ~f:(fun x -> String.make 1 (Char.chr x)))

let delete_nth x =
  {
    range = Some {
      st = { line = 1; column = x};
      ed = { line = 1; column = x+1};
    };
    text = "";
  }

let test_utf8 () =
  let c1 = "a" in
  (* 'INVERTED EXCLAMATION MARK' (U+00A1) *)
  let c2 = utf8 [0xC2; 0xA1;] in
  (* TELUGU LETTER GHA' (U+0C18) *)
  let c3 = utf8 [0xE0; 0xB0; 0x98;] in
  (* Unicode Han Character 'U+26604' *)
  let c4 = utf8 [0xF0; 0xA6; 0x98; 0x84;] in

  let content = c1^c2^c3^c4 in

  let edit = delete_nth 1 in
  let edited_content = edit_file_unsafe content [edit] in
  let r1 = expect_has_content edited_content (c2^c3^c4) in

  let edit = delete_nth 2 in
  let edited_content = edit_file_unsafe content [edit] in
  let r2 = expect_has_content edited_content (c1^c3^c4) in

  let edit = delete_nth 3 in
  let edited_content = edit_file_unsafe content [edit] in
  let r3 = expect_has_content edited_content (c1^c2^c4) in

  let edit = delete_nth 4 in
  let edited_content = edit_file_unsafe content [edit] in
  let r4 = expect_has_content edited_content (c1^c2^c3) in
  r1 && r2 && r3 && r4

let test_large () =
  let len = 100000000 in
  let content = String.make len 'c' in
  let content_to_edit = content ^ "c" in
  let edit = delete_nth (len/2) in
  let edited_content = edit_file_unsafe content_to_edit [edit] in
  expect_has_content edited_content content

let tests = [
  "test_basic_edit", test_basic_edit;
  "test_basic_edit2", test_basic_edit2;
  "test_multi_line_edit", test_multi_line_edit;
  "test_multi_line_edit2", test_multi_line_edit2;
  "test_special_edit", test_special_edit;
  "test_multiple_edits", test_multiple_edits;
  "test_invalid_edit", test_invalid_edit;
  "test_empty_edit", test_empty_edit;
  "test_end_of_line_edit", test_end_of_line_edit;
  "test-utf8", test_utf8;
  "test_large", test_large;
]

let () =
  Unit_test.run_all tests
