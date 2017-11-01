let get_or_throw m =
  let open Hh_json.Access in
  m |> counit_with @@ fun f -> raise @@ Failure
  (Printf.sprintf "%s\n%!" @@ Hh_json.Access.access_failure_to_string f)

let test_open_header_width_constant () =
  let a = Recorder.Header.header_open_tag 1 in
  let b = Recorder.Header.header_open_tag 90000 in
  Asserter.Int_asserter.assert_equals (String.length a)
    Recorder.Header.open_tag_width "";
  Asserter.Int_asserter.assert_equals (String.length a)
    (String.length b) "";
  true

let test_make_header () =
  let header = Recorder.Header.make_header ~for_revision:(Hg.Hg_rev "hello")
    "fake_build_id" in
  Asserter.String_asserter.assert_equals "<hack-recording-header size=00069>"
    (String.sub header 0 Recorder.Header.open_tag_width) "";
  let blob = String.sub header Recorder.Header.open_tag_width 76 in
  let json = Hh_json.json_of_string ~strict:true blob in
  let open Hh_json.Access in
  let rev = (return json)
    >>= get_string "loaded_state_for_hg_rev"
    |> get_or_throw
  in
  Asserter.String_asserter.assert_equals "hello" rev "";
  let build_id = (return json)
    >>= get_string "build_id"
    |> get_or_throw
  in
  Asserter.String_asserter.assert_equals "fake_build_id" build_id "";
  true

let sample_recorder_init_env =
  {
    Recorder_types.root_path = Path.make Sys_utils.temp_dir_name;
    hhi_path = Path.make Sys_utils.temp_dir_name;
    lock_file = Path.concat (Path.make Sys_utils.temp_dir_name) @@
      "sample_recorder.lock";
  }

(** Make a recording and return a channel to that recording. *)
let make_sample_recording init_env =
  let recorder = Recorder.start init_env in
  let recorder = Recorder.add_event Debug_event.Typecheck recorder in
  let recorder = Recorder.add_event Debug_event.Stop_recording recorder in
  let in_fd, out_fd = Unix.pipe () in
  let ic = Unix.in_channel_of_descr in_fd in
  let oc = Unix.out_channel_of_descr out_fd in
  let header = Recorder.get_header recorder in
  output_string oc header;
  let events = Recorder.get_events recorder in
  List.iter (fun event -> Marshal.to_channel oc event []) events;
  flush oc;
  close_out oc;
  ic

let rec read_events ic acc =
  try
    let event = Marshal.from_channel ic in
    read_events ic (event :: acc)
  with
  | End_of_file ->
    acc
  | Failure msg when msg = "input_value: truncated object" ->
    acc

let read_events ic =
  List.rev (read_events ic [])

let test_basic_recorder () =
  let ic = make_sample_recording sample_recorder_init_env in
  let json = Recorder.Header.parse_header ic in
  let open Hh_json.Access in
  let build_id = (return json)
    >>= get_string "build_id"
    |> get_or_throw
  in
  Asserter.String_asserter.assert_equals Build_id.build_id_ohai build_id "";
  let events = read_events ic in
  let open Recorder_types in
  let expected_events = [
    Start_recording sample_recorder_init_env;
    Typecheck;
  ] in
  Asserter.Recorder_event_asserter.assert_list_equals expected_events events "";
  true

let tests = [
  ("test_open_header_width_constant", test_open_header_width_constant);
  ("test_make_header", test_make_header);
  ("test_basic_recorder", test_basic_recorder);
]

let () =
  Unit_test.run_all tests
