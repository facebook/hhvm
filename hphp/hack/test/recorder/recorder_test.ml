open Recorder_types
open Core

let transcriber_consumer list_ref =
  Recorder.Transcribe_to_consumer (fun events ->
    list_ref := events
  )

let verify_result actual expected =
  let zipped = List.zip actual expected in
  match zipped with
  | None ->
    let () = Printf.eprintf "actual and expected lengths differ.\n" in
    false
  | Some zipped ->
    List.for_all zipped ~f:(fun (actual, expected) -> actual = expected)

let ignore_events_before_fresh_state () =
  let result = ref [] in
  let recorder = Recorder.start {
    Recorder.transcriber = transcriber_consumer result;
  } in
  let _recorder = recorder
    (** First typecheck is ignored. *)
    |> Recorder.add_event Typecheck
    |> Recorder.add_event (Fresh_vcs_state "abcdefg")
    |> Recorder.add_event Stop_recording
  in
  if (verify_result !result [Fresh_vcs_state "abcdefg"]) then
    true
  else
    let () = Printf.eprintf "Actual and expected differ.\n" in
    false

let tests = [
  "ignore_events_before_fresh_state", ignore_events_before_fresh_state
]

let () =
  Daemon.check_entry_point (); (* this call might not return *)
  Unit_test.run_all tests
