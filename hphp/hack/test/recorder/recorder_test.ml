open Recorder_types
open Core

module DE = Debug_event

let transcriber_consumer list_ref =
  Recorder.Transcribe_to_consumer (fun events ->
    list_ref := events
  )

let verify_result actual expected =
  let zipped = List.zip actual expected in
  match zipped with
  | None ->
    let () = Printf.eprintf "actual and expected lengths differ.\n" in
    let () = Printf.eprintf "Actual length: %d. Expected length: %d"
      (List.length actual) (List.length expected) in
    false
  | Some zipped ->
    List.for_all zipped ~f:(fun (actual, expected) ->
      let result = actual = expected in
      if not result then
        Printf.eprintf "Found but not expected. Actual: %s. Expected: %s\n"
          (to_string actual) (to_string expected)
      else
        ();
      result)

let ignore_events_before_fresh_state () =
  let result = ref [] in
  let recorder = Recorder.start {
    Recorder.transcriber = transcriber_consumer result;
  } in
  let _recorder = recorder
    (** First typecheck is ignored. *)
    |> Recorder.add_event DE.Typecheck
    |> Recorder.add_event (DE.Fresh_vcs_state "abcdefg")
    |> Recorder.add_event DE.Stop_recording
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
