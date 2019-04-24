(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)
open IndexBuilder

(* Basic help text *)
let usage =
  Printf.sprintf
    "Usage: %s [--sqlite file] [--text file] [--json file] [repository]\n"
    Sys.argv.(0)
;;

(* Parse command line options *)
let parse_options (): index_builder_context =
  let sqlite_filename = ref None in
  let text_filename = ref None in
  let json_filename = ref None in
  let json_chunk_size = ref 500_000 in
  let glean_service = ref None in
  let glean_repo_name = ref None in
  let repository = ref "." in
  let options = ref [
      "--sqlite",
      Arg.String (fun x -> sqlite_filename := (Some x)),
      "[filename]  Save the global index in a Sqlite database";

      "--text",
      Arg.String (fun x -> text_filename := (Some x)),
      "[filename]  Save the global index in a finite-state transducer (FST) file";

      "--json",
      Arg.String (fun x -> json_filename := (Some x)),
      "[filename]  Save the global index in a JSON file";

      "--chunk-size",
      Arg.Int (fun x -> json_chunk_size := x),
      "[number]    Split the JSON file into chunks of a specified size";

      "--glean-service",
      Arg.String (fun x -> glean_service := (Some x)),
      "[service]  Use this specified glean service";

      "--glean-repo-name",
      Arg.String (fun x -> glean_repo_name := (Some x)),
      "[repo-name]  Use this specified glean repo name";

    ] in
  Arg.parse_dynamic options (fun anonymous_arg -> repository := anonymous_arg) usage;

  (* Print what we're about to do *)
  Printf.printf "Building global symbol index for [%s]\n%!"
    !repository;

  (* Parameters for this execution *)
  {
    repo_folder = !repository;
    sqlite_filename = !sqlite_filename;
    text_filename = !text_filename;
    json_filename = !json_filename;
    json_chunk_size = !json_chunk_size;
    glean_service = !glean_service;
    glean_repo_name = !glean_repo_name;
  }
;;

(* Run the application *)
let main (): unit =
  Daemon.check_entry_point ();
  PidLog.init "/tmp/hh_server/global_index_builder.pids";
  PidLog.log ~reason:"main" (Unix.getpid ());
  let ctxt = parse_options () in
  IndexBuilder.go ctxt;
;;

(* Main entry point *)
let () =
  let _ = IndexBuilder.measure_time ~f:(fun () -> main ()) ~name:"\n\nGlobal Index Built successfully:" in
  Printf.printf "Done%s" "";
