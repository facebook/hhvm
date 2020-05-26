(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

let expect file stack =
  let matches s = String_utils.string_starts_with s file in
  if List.exists matches (String_utils.split_on_newlines stack) then
    ()
  else
    let () =
      Printf.eprintf "expected '%s' but didn't find it:\n%s" file stack
    in
    failwith (Printf.sprintf "didn't find '%s'" file)

let test_current_stack () =
  let stack =
    Exception.get_current_callstack_string 99 |> Exception.clean_stack
  in
  expect "test/unit/utils/exception_test.ml" stack;
  expect "test/unit/unit_test.ml" stack;
  true

let bar b =
  let _ = Printf.sprintf "%s" "no inlining" in
  Exception_test_b.intentional_raise b;
  let _ = Printf.sprintf "%s" "no inlining" in
  ()

let foo () =
  let _ = Printf.sprintf "%s" "no inlining" in
  let () = bar true in
  let _ = Printf.sprintf "%s" "no inlining" in
  ()

let test_exception_stack () =
  try
    foo ();
    failwith "Expected foo to throw an exception"
  with e ->
    let e = Exception.wrap e in
    let stack = Exception.get_backtrace_string e |> Exception.clean_stack in
    expect "test/unit/utils/exception_test_b.ml" stack;
    expect "test/unit/utils/exception_test.ml" stack;
    true

let test_clean_stack () =
  let stack =
    {|Raised by primitive operation at file "stdlib.ml", line 390, characters 28-54
Called from file "stdlib.ml" (inlined), line 398, characters 2-47
Called from file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/utils/disk/realDisk.ml", line 5, characters 11-31
Called from file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/utils/core/utils.ml", line 257, characters 8-12
Re-raised at file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/utils/core/utils.ml", line 260, characters 6-13
Called from file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/providers/provider_context.ml", line 114, characters 22-74
Re-raised at file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/utils/core/exception.ml", line 36, characters 33-76
Called from file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/providers/ast_provider.ml", line 127, characters 19-64
Called from file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/providers/ast_provider.ml", line 148, characters 22-47
Called from file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/providers/tast_provider.ml", line 61, characters 6-116
Called from file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/providers/tast_provider.ml" (inlined), line 174, characters 2-96
Called from file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/server/findRefsService.ml", line 249, characters 10-62
Called from file "src/list.ml", line 343, characters 13-17
Called from file "src/list.ml" (inlined), line 350, characters 15-31
Called from file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/server/findRefsService.ml", line 246, characters 4-263
Called from file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/server/findRefsService.ml", line 364, characters 6-35
Called from file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/server/serverFindRefs.ml", line 49, characters 4-78
Called from file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/server/serverFindRefs.ml", line 64, characters 26-30
Called from file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/server/serverRpc.ml", line 248, characters 10-55
Called from file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/utils/core/utils.ml", line 257, characters 8-12
Re-raised at file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/utils/core/utils.ml", line 260, characters 6-13
Called from file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/errors/errors.ml", line 198, characters 23-27
Called from file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/utils/core/utils.ml", line 257, characters 8-12
Re-raised at file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/utils/core/utils.ml", line 260, characters 6-13
Called from file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/errors/errors.ml", line 194, characters 4-360
Called from file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/providers/provider_utils.ml", line 164, characters 4-171
Called from file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/server/serverCommand.ml", line 243, characters 14-18
Re-raised at file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/server/serverCommand.ml", line 248, characters 4-46
Called from file "/data/sandcastle/boxes/eden-trunk-hg-fbcode-fbsource/fbcode/hphp/hack/src/server/serverCommand.ml", line 276, characters 8-79|}
  in
  Asserter.String_asserter.assert_equals
    {|src/utils/disk/realDisk.ml @ 5
src/utils/core/utils.ml @ 257
src/utils/core/utils.ml @ 260
src/providers/provider_context.ml @ 114
src/utils/core/exception.ml @ 36
src/providers/ast_provider.ml @ 127
src/providers/ast_provider.ml @ 148
src/providers/tast_provider.ml @ 61
src/providers/tast_provider.ml @ 174
src/server/findRefsService.ml @ 249
src/server/findRefsService.ml @ 246
src/server/findRefsService.ml @ 364
src/server/serverFindRefs.ml @ 49
src/server/serverFindRefs.ml @ 64
src/server/serverRpc.ml @ 248
src/utils/core/utils.ml @ 257
src/utils/core/utils.ml @ 260
src/errors/errors.ml @ 198
src/utils/core/utils.ml @ 257
src/utils/core/utils.ml @ 260
src/errors/errors.ml @ 194
src/providers/provider_utils.ml @ 164
src/server/serverCommand.ml @ 243
src/server/serverCommand.ml @ 248
src/server/serverCommand.ml @ 276|}
    (Exception.clean_stack stack)
    "Expected the stack to be cleaned like this";
  true

let tests =
  [
    ("test_current_stack", test_current_stack);
    ("test_exception_stack", test_exception_stack);
    ("test_clean_stack", test_clean_stack);
  ]

let () = Unit_test.run_all tests
