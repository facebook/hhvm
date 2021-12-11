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

let mock_realworld_cpuinfo =
  "processor\t: 0
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 0
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 0
initial apicid\t: 0
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4788.99
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 1
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 1
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 1
initial apicid\t: 1
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4926.31
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 2
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 2
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 2
initial apicid\t: 2
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4882.84
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 3
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 3
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 3
initial apicid\t: 3
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4874.06
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 4
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 4
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 4
initial apicid\t: 4
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4936.32
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 5
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 5
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 5
initial apicid\t: 5
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4929.05
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 6
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 6
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 6
initial apicid\t: 6
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4809.68
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 7
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 7
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 7
initial apicid\t: 7
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4824.12
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 8
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 8
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 8
initial apicid\t: 8
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4853.07
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 9
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 9
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 9
initial apicid\t: 9
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4855.92
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 10
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 10
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 10
initial apicid\t: 10
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4918.44
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 11
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 11
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 11
initial apicid\t: 11
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4907.30
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 12
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 12
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 12
initial apicid\t: 12
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4892.13
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 13
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 13
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 13
initial apicid\t: 13
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
flags\t\t: fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush mmx fxsr sse sse2 syscall nx rdtscp lm constant_tsc rep_good nopl xtopology cpuid pni pclmulqdq ssse3 fma cx16 pcid sse4_1 sse4_2 movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand hypervisor lahf_lm abm 3dnowprefetch cpuid_fault invpcid_single pti fsgsbase bmi1 hle avx2 smep bmi2 erms invpcid rtm rdseed adx smap xsaveopt arat
bugs\t\t: cpu_meltdown spectre_v1 spectre_v2 spec_store_bypass l1tf mds swapgs
bogomips\t: 4940.92
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 14
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 14
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 14
initial apicid\t: 14
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4868.87
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 15
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 15
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 15
initial apicid\t: 15
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4831.04
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 16
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 16
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 16
initial apicid\t: 16
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4827.98
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 17
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 17
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 17
initial apicid\t: 17
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4813.93
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 18
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 18
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 18
initial apicid\t: 18
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4832.41
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 19
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 19
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 19
initial apicid\t: 19
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4818.89
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 20
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 20
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 20
initial apicid\t: 20
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4844.89
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 21
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 21
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 21
initial apicid\t: 21
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4849.74
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 22
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 22
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 22
initial apicid\t: 22
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4872.98
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:

processor\t: 23
vendor_id\t: GenuineIntel
cpu family\t: 6
model\t\t: 61
model name\t: Intel Core Processor (Broadwell)
stepping\t: 2
microcode\t: 0x1
cpu MHz\t\t: 2394.499
cache size\t: 16384 KB
physical id\t: 23
siblings\t: 1
core id\t\t: 0
cpu cores\t: 1
apicid\t\t: 23
initial apicid\t: 23
fpu\t\t: yes
fpu_exception\t: yes
cpuid level\t: 13
wp\t\t: yes
bogomips\t: 4878.43
clflush size\t: 64
cache_alignment\t: 64
address sizes\t: 40 bits physical, 48 bits virtual
power management:"

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
  (* Positive real-world example *)
  Asserter.Int_asserter.assert_equals
    24
    (Sys_utils.ncores_linux_only mock_realworld_cpuinfo)
    "mock-real-world cpuinfo should have this many cores";

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
