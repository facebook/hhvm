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
            try Sys_utils.redirect_stdout_and_stderr_to_file "." with
            | _ -> Printf.eprintf "exn.%!"
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

let test_ncores () =
  (* Several positive examples taken from https://doc.callmematthi.eu/static/webArticles/Understanding%20Linux%20_proc_cpuinfo.pdf *)
  let cpuinfo_1socket_1core_hyper =
    {|processor : 0
model name : Intel(R) Pentium(R) 4 CPU 2.80GHz
cache size : 1024 KB
physical id : 0
siblings : 2
core id : 0
cpu cores : 1

processor : 1
model name : Intel(R) Pentium(R) 4 CPU 2.80GHz
cache size : 1024 KB
physical id : 0
siblings : 2
core id : 0
cpu cores : 1|}
  in
  Asserter.Int_asserter.assert_equals
    1
    (Sys_utils.ncores_linux_only cpuinfo_1socket_1core_hyper)
    "cpuinfo_1socket_1core_hyper should have this many cores";

  let cpuinfo_1socket_4core_nothyper =
    "processor : 0
model name : Intel(R) Xeon(R) CPU E5410 @ 2.33GHz
cache size : 6144 KB
physical id : 0
siblings : 4
core id : 0
cpu cores : 4

processor : 1
model name : Intel(R) Xeon(R) CPU E5410 @ 2.33GHz
cache size : 6144 KB
physical id : 0
siblings : 4
core id : 1
cpu cores : 4

processor : 2
model name : Intel(R) Xeon(R) CPU E5410 @ 2.33GHz
cache size : 6144 KB
physical id : 0
siblings : 4
core id : 2
cpu cores : 4

processor : 3
model name : Intel(R) Xeon(R) CPU E5410 @ 2.33GHz
cache size : 6144 KB
physical id : 0
siblings : 4
core id : 3
cpu cores : 4"
  in
  Asserter.Int_asserter.assert_equals
    4
    (Sys_utils.ncores_linux_only cpuinfo_1socket_4core_nothyper)
    "cpuinfo_1socket_4core_nothyper should have this many cores";

  let cpuinfo_1socket_2core_nothyper =
    "processor : 0
model name : Intel(R) Pentium(R) D CPU 3.00GHz
cache size : 2048 KB
physical id : 0
siblings : 2
core id : 0
cpu cores : 2

processor : 1
model name : Intel(R) Pentium(R) D CPU 3.00GHz
cache size : 2048 KB
physical id : 0
siblings : 2
core id : 1
cpu cores : 2"
  in
  Asserter.Int_asserter.assert_equals
    2
    (Sys_utils.ncores_linux_only cpuinfo_1socket_2core_nothyper)
    "cpuinfo_1socket_2core_nothyper should have this many cores";

  let cpuinfo_2socket_1core_hyper =
    "processor : 0
model name : Intel(R) Xeon(TM) CPU 3.60GHz
cache size : 1024 KB
physical id : 0
siblings : 2
core id : 0
cpu cores : 1

processor : 1
model name : Intel(R) Xeon(TM) CPU 3.60GHz
cache size : 1024 KB
physical id : 3
siblings : 2
core id : 0
cpu cores : 1

processor : 2
model name : Intel(R) Xeon(TM) CPU 3.60GHz
cache size : 1024 KB
physical id : 0
siblings : 2
core id : 0
cpu cores : 1

processor : 3
model name : Intel(R) Xeon(TM) CPU 3.60GHz
cache size : 1024 KB
physical id : 3
siblings : 2
core id : 0
cpu cores : 1"
  in
  Asserter.Int_asserter.assert_equals
    2
    (Sys_utils.ncores_linux_only cpuinfo_2socket_1core_hyper)
    "cpuinfo_2socket_1core_hyper should have this many cores";

  let cpuinfo_2socket_2core_nothyper =
    "processor : 0
model name : Intel(R) Xeon(R) CPU 5160 @ 3.00GHz
cache size : 4096 KB
physical id : 0
siblings : 2
core id : 0
cpu cores : 2

processor : 1
model name : Intel(R) Xeon(R) CPU 5160 @ 3.00GHz
cache size : 4096 KB
physical id : 0
siblings : 2
core id : 1
cpu cores : 2

processor : 2
model name : Intel(R) Xeon(R) CPU 5160 @ 3.00GHz
cache size : 4096 KB
physical id : 3
siblings : 2
core id : 0
cpu cores : 2

processor : 3
model name : Intel(R) Xeon(R) CPU 5160 @ 3.00GHz
cache size : 4096 KB
physical id : 3
siblings : 2
core id : 1
cpu cores : 2"
  in
  Asserter.Int_asserter.assert_equals
    4
    (Sys_utils.ncores_linux_only cpuinfo_2socket_2core_nothyper)
    "cpuinfo_2socket_2core_nothyper should have this many cores";
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
    ("test_ncores", test_ncores);
  ]

let () = Unit_test.run_all tests
