open Recorder_types
open Core

module DE = Debug_event

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

let ignore_events_before_loading_saved_state () =
  let recorder = Recorder.start () in
  let recorder = recorder
    (** First typecheck is ignored. *)
    |> Recorder.add_event DE.Typecheck
    |> Recorder.add_event (DE.Loaded_saved_state
      ({ DE.filename = "abcde.sqlite";
        dirty_files = Relative_path.Set.empty;
        changed_while_parsing = Relative_path.Set.empty;
        build_targets = Relative_path.Set.empty; },
      ServerGlobalState.fake_state))
    |> Recorder.add_event (DE.Fresh_vcs_state "abcdefg")
    |> Recorder.add_event DE.Stop_recording
  in
  if (verify_result (Recorder.get_events recorder) [
    Loaded_saved_state ({
      filename = "abcde.sqlite";
      dirty_files = Relative_path.Map.empty;
      changed_while_parsing = Relative_path.Map.empty;
      build_targets = Relative_path.Map.empty;
      }, ServerGlobalState.fake_state);
    Fresh_vcs_state "abcdefg";
  ])
  then
    true
  else
    let () = Printf.eprintf "Actual and expected differ.\n" in
    false

let tests = [
  "ignore_events_before_loading_saved_state",
    ignore_events_before_loading_saved_state
]

let () =
  Daemon.check_entry_point (); (* this call might not return *)
  Unit_test.run_all tests
