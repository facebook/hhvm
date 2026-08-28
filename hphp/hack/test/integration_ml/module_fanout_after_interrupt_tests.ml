(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

open Hh_prelude
open Asserter
module Harness = Server_interrupt_test_harness

let consumer_baseline =
  ("a_consumer.php", "<?hh\nfinal class ModuleConsumer {}\n")

let consumer_with_missing_module =
  ("a_consumer.php", "<?hh\nmodule foo.bar;\nfinal class ModuleConsumer {}\n")

let blocker_baseline =
  ("z_blocker.php", "<?hh\nfunction test_blocker(): void {}\n")

let blocker_infinite =
  ( "z_blocker.php",
    "<?hh\nfunction test_blocker(): void { hh_loop_forever(); }\n" )

let module_definition = ("module.php", "<?hh\nnew module foo.bar {}\n")

let second_wave ~(changed_file_count : int) : (string * string) list =
  (* Every file defines a symbol, so this count is also the number of entries in
     [redo_type_decl]'s defs map: fewer than 10 run on the master, while 10 or
     more use workers. *)
  let filler_count = changed_file_count - 2 in
  let fillers =
    List.init filler_count ~f:(fun index ->
        ( Printf.sprintf "filler_%02d.php" index,
          Printf.sprintf
            "<?hh\nfunction module_fanout_filler_%02d(): void {}\n"
            index ))
  in
  module_definition :: blocker_baseline :: fillers

let start_server (env : Harness.t) : string Lwt.t =
  Harness.hh
    env
    [|
      "--no-load";
      "--config";
      "max_workers=4";
      "--config";
      "produce_streaming_errors=true";
      "--config";
      "interrupt_on_client=true";
      "--custom-hhi-path";
      Path.to_string env.hhi;
    |]

let assert_contains text substring =
  if not (String.is_substring text ~substring) then
    failwith (Printf.sprintf "Expected to find %S in:\n%s" substring text)

let assert_interruption_shape
    (env : Harness.t) ~(log_offset : int) ~(changed_file_count : int) : unit =
  let log = Harness.server_log_since env ~offset:log_offset in
  assert_contains log "Begin typechecking 2 files.";
  assert_contains log "Typechecked 1 files [1 errors]";
  assert_contains
    log
    (Printf.sprintf
       "Interrupted by file watcher sync query: %d files changed"
       changed_file_count);
  assert_contains log "Processing deferred typechecking for 1 file(s)";
  assert_contains
    log
    (Printf.sprintf "Processing changes to %d files" changed_file_count)

let files_to_recheck_from_log (log : string) : string list =
  let headers = ["Files to recheck:"; "First 10 files to recheck:"] in
  let rec latest_section latest = function
    | [] -> Option.value_exn latest
    | line :: rest ->
      if
        List.exists headers ~f:(fun header ->
            String.is_suffix line ~suffix:header)
      then
        let files = List.take_while rest ~f:(String.is_suffix ~suffix:".php") in
        latest_section (Some files) rest
      else
        latest_section latest rest
  in
  String.split_lines log |> latest_section None

let test_files_to_recheck_uses_latest_section () : bool Lwt.t =
  let log =
    "[old] Files to recheck:\nold.php\n[new] First 10 files to recheck:\na_consumer.php\nfiller.php\n"
  in
  Bool_asserter.assert_equals
    true
    (List.equal
       String.equal
       (files_to_recheck_from_log log)
       ["a_consumer.php"; "filler.php"])
    "the latest timestamp-prefixed recheck section should be selected";
  Lwt.return_true

let assert_consumer_fanout
    (env : Harness.t) ~(log_offset : int) ~(expect_consumer : bool) : unit =
  let files =
    Harness.server_log_since env ~offset:log_offset |> files_to_recheck_from_log
  in
  Bool_asserter.assert_equals
    expect_consumer
    (List.mem files "a_consumer.php" ~equal:String.equal)
    (Printf.sprintf
       "unexpected second-wave files-to-recheck set: [%s]"
       (String.concat files ~sep:", "))

let run_case ~(changed_file_count : int) : unit Lwt.t =
  Harness.with_server
    ~hhconfig:"allowed_files_for_module_declarations = *\n"
    ~files:[consumer_baseline; blocker_baseline]
  @@ fun env ->
  let%lwt initial = start_server env in
  String_asserter.assert_equals
    "No errors!\n"
    initial
    "the server should finish its baseline check before the first wave";
  let log_offset = Harness.server_log_offset env in
  Harness.write_files env [consumer_with_missing_module; blocker_infinite];
  Harness.with_hh_process
    env
    [| "check"; "--search"; "this_is_just_to_check_liveness_of_hh_server" |]
  @@ fun _first_priority_query ->
  let%lwt () =
    Harness.wait_for_server_log
      env
      ~offset:log_offset
      ~description:"the first-wave two-file typecheck"
      ~matches:(String.is_substring ~substring:"Begin typechecking 2 files.")
  in
  Harness.with_current_errors_stream env @@ fun current_errors ->
  let%lwt () =
    Harness.wait_for_event
      current_errors
      ~description:"consumer.php's missing-module diagnostic"
      ~matches:(String.equal "Errors [a_consumer.php=1]")
  in
  (* The diagnostic is streamed before the worker's dependency edges are
     registered. A fresh priority query can only complete after the server has
     returned from that merge callback, so awaiting it establishes the barrier
     before the second wave without relying on timing. *)
  let%lwt _ =
    Harness.hh
      env
      [| "check"; "--search"; "this_is_just_to_check_liveness_of_hh_server" |]
  in
  let second_wave_log_offset = Harness.server_log_offset env in
  Harness.write_files env (second_wave ~changed_file_count);
  let%lwt _ =
    Harness.hh
      env
      [| "check"; "--search"; "this_is_just_to_check_liveness_of_hh_server" |]
  in
  let%lwt () =
    Harness.wait_for_event
      current_errors
      ~description:"the interrupted typecheck restart"
      ~matches:(String.is_substring ~substring:"typecheck restarted")
  in
  let%lwt final = Harness.hh env [| "check"; "--error-format"; "plain" |] in
  assert_interruption_shape env ~log_offset ~changed_file_count;
  String_asserter.assert_equals
    "No errors!\n"
    final
    "adding the module should clear the unbound-module diagnostic";
  assert_consumer_fanout
    env
    ~log_offset:second_wave_log_offset
    ~expect_consumer:true;
  Lwt.return_unit

let test_nine_changed_files () : bool Lwt.t =
  let%lwt () = run_case ~changed_file_count:9 in
  Lwt.return_true

let test_ten_changed_files () : bool Lwt.t =
  let%lwt () = run_case ~changed_file_count:10 in
  Lwt.return_true

let () =
  Printexc.record_backtrace true;
  EventLogger.init_fake ();
  let tests =
    [
      ( "test_files_to_recheck_uses_latest_section",
        test_files_to_recheck_uses_latest_section );
      ("test_nine_changed_files", test_nine_changed_files);
      ("test_ten_changed_files", test_ten_changed_files);
    ]
    |> List.map ~f:(fun (name, test) ->
           (name, (fun () -> Lwt_main.run (test ()))))
  in
  Unit_test.run_all tests
