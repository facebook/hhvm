(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Hh_prelude

let restore_std (f : unit -> 'a) : 'a =
  let old_stdout = Unix.dup Unix.stdout in
  let old_stderr = Unix.dup Unix.stderr in
  let finally () =
    Unix.dup2 old_stdout Unix.stdout;
    Unix.dup2 old_stderr Unix.stderr;
    ()
  in
  Utils.try_finally ~f ~finally

let test_freopen_failure1 () =
  restore_std @@ fun () ->
  try
    Sys_utils.freopen "." "w" Unix.stdout;
    Printf.eprintf "Expected Unix.EISDIR out of freopen . w, not success";
    false
  with
  | Unix.Unix_error (Unix.EISDIR, _, _) -> true
  | e ->
    Printf.eprintf
      "Expected Unix.EISDIR out of freopen . w, not %s"
      (Exn.to_string e);
    false

let test_freopen_failure2 () =
  restore_std @@ fun () ->
  Tempfile.with_real_tempdir (fun path ->
      let fn = Path.concat path "freopen.txt" |> Path.to_string in
      try
        Sys_utils.freopen fn "~az!" Unix.stdout;
        Printf.eprintf "Expected Unix.EINVAL out of freopen . ~az!, not success";
        false
      with
      | Unix.Unix_error (Unix.EINVAL, _, _) -> true
      | e ->
        Printf.eprintf
          "Expected Unix.EINVAL out of freopen . ~az!, not %s"
          (Exn.to_string e);
        false)

let test_freopen_success () =
  Tempfile.with_real_tempdir (fun path ->
      let fn = Path.concat path "freopen.txt" |> Path.to_string in
      Printf.printf "hello\n%!";
      restore_std (fun () ->
          Sys_utils.freopen fn "w" Unix.stdout;
          Printf.printf "there\n%!";
          Printf.eprintf "world\n%!");
      Asserter.String_asserter.assert_equals
        "there\n"
        (Disk.cat fn)
        "Expected stdout to be redirected to file";
      ());
  true

let test_redirect_stdout_and_stderr_to_file_failure () =
  Tempfile.with_real_tempdir (fun path ->
      let fnout = Path.concat path "out.txt" |> Path.to_string in
      let fnerr = Path.concat path "err.txt" |> Path.to_string in
      restore_std (fun () ->
          Sys_utils.freopen fnout "w" Unix.stdout;
          Sys_utils.freopen fnerr "w" Unix.stderr;
          Printf.printf "out.%!";
          Printf.eprintf "err.%!";
          begin
            try Sys_utils.redirect_stdout_and_stderr_to_file "."
            with _ -> Printf.eprintf "exn.%!"
          end;
          Printf.printf "there.%!";
          Printf.eprintf "world.%!";
          ());
      Asserter.String_asserter.assert_equals
        "err.exn.world."
        (Disk.cat fnerr)
        "Stderr should have been restored upon redirect failure";
      Asserter.String_asserter.assert_equals
        "out.there."
        (Disk.cat fnout)
        "Stdout should have been restored upon redirect failure";
      ());
  true

let test_redirect_stdout_and_stderr_to_file_success () =
  Tempfile.with_real_tempdir (fun path ->
      let fn = Path.concat path "freopen.txt" |> Path.to_string in
      Printf.printf "hello.%!";
      restore_std (fun () ->
          Sys_utils.redirect_stdout_and_stderr_to_file fn;
          Printf.printf "there.%!";
          Printf.eprintf "world.%!");
      Asserter.String_asserter.assert_equals
        "there.world."
        (Disk.cat fn)
        "Stdout+stderr should have been redirected to file";
      ());
  true

let tests =
  [
    ("test_freopen_failure1", test_freopen_failure1);
    ("test_freopen_failure2", test_freopen_failure2);
    ("test_freopen_success", test_freopen_success);
    ( "test_redirect_stdout_and_stderr_to_file_failure",
      test_redirect_stdout_and_stderr_to_file_failure );
    ( "test_redirect_stdout_and_stderr_to_file_success",
      test_redirect_stdout_and_stderr_to_file_success );
  ]

let () = Unit_test.run_all tests
