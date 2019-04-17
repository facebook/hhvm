(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core_kernel
open SearchUtils
open Facts

(* Keep track of all references yet to scan *)
let files_scanned = ref 0
let error_count = ref 0

type index_builder_context = {
  repo_folder: string;
  sqlite_filename: string option;
  text_filename: string option;
  json_filename: string option;
}

(* Combine two results *)
(* Parse one single file and capture information about it *)
let parse_file (filename: string): si_results =
  if Sys.is_directory filename then begin
    []
  end else begin
    let text = Core_kernel.In_channel.read_all filename in
    let enable_hh_syntax =
      Hhbc_options.enable_hiphop_syntax !Hhbc_options.compiler_options in
    let enable_xhp =
      Hhbc_options.enable_xhp !Hhbc_options.compiler_options in
    let rp = Relative_path.from_root filename in
    (* Just the facts ma'am *)
    let fact_opt = Facts_parser.from_text
        true true enable_hh_syntax enable_xhp rp text in

    (* Iterate through facts and print them out *)
    let result =
      match fact_opt with
      | Some facts ->

        (* Identify all classes in the file *)
        let class_keys = InvSMap.keys facts.types in
        let classes_mapped = Core_kernel.List.map class_keys ~f:(fun key -> begin
              let info_opt = InvSMap.get key facts.types in
              let kind = begin
                match info_opt with
                | None -> SI_Unknown
                | Some info -> begin
                    match info.kind with
                    | TKClass -> SI_Class
                    | TKInterface -> SI_Interface
                    | TKEnum -> SI_Enum
                    | TKTrait -> SI_Trait
                    | TKMixed -> SI_Mixed
                    | _ -> SI_Unknown
                  end
              end in
              {
                si_name = key;
                si_kind = kind;
              }
            end) in

        (* Identify all functions in the file *)
        let functions_mapped = Core_kernel.List.map facts.functions ~f:(fun funcname -> {
              si_name = funcname;
              si_kind = SI_Function;
            }) in

        (* Return unified results *)
        List.append classes_mapped functions_mapped
      | None ->
        []
    in
    files_scanned := !files_scanned + 1;
    result;
  end
;;

let parse_batch acc files =
  List.fold files ~init:acc ~f:begin fun acc file ->
    if Path.file_exists (Path.make file) then
      try
        let res = parse_file file in
        List.append res acc;
      with exn ->
        error_count := !error_count + 1;
        Printf.fprintf stderr "exception: %s\nfailed to parse \"%s\"\n"
          (Caml.Printexc.to_string exn)
          file;
        acc
    else (Printf.fprintf stderr "File %s does not exist.\n" file; acc)
  end
;;

let parallel_parse ~workers files =
  MultiWorker.call workers
    ~job:parse_batch
    ~neutral:[]
    ~merge:(List.append)
    ~next:(MultiWorker.next workers files)
;;

let entry = WorkerController.register_entry_point ~restore:(fun () -> ())
;;
let init_workers () =
  let nbr_procs = Sys_utils.nproc () in
  let gc_control = GlobalConfig.gc_control in
  let config = GlobalConfig.default_sharedmem_config in
  let heap_handle = SharedMem.init config ~num_workers:nbr_procs in
  MultiWorker.make
    ?call_wrapper:None
    ~saved_state:()
    ~entry
    ~nbr_procs
    ~gc_control
    ~heap_handle
;;

(* Basic help text *)
let usage =
  Printf.sprintf
    "Usage: %s [--sqlite file] [--text file] [--json file] [repository]\n"
    Sys.argv.(0)
;;

let parse_options (): index_builder_context =
  let sqlite_filename = ref None in
  let text_filename = ref None in
  let json_filename = ref None in
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
  }
;;

(* Let's use the unix find command which seems to be really quick at this sort of thing *)
let gather_file_list (path: string): string list =
  let cmdline = Printf.sprintf "find %s -name \"*.php\"" path in
  let channel = Unix.open_process_in cmdline in
  let result = ref [] in
  (try
     while true do
       let line_opt = Core_kernel.In_channel.input_line channel in
       match line_opt with
       | Some line -> result := line :: !result
       | None -> raise End_of_file
     done;
   with End_of_file -> ());
  assert (Unix.close_process_in channel = Unix.WEXITED 0);
  !result
;;

(* Run something and measure its duration *)
let measure_time ~f ~(name: string) =
  let start_time = Unix.gettimeofday () in
  let result = f () in
  let end_time = Unix.gettimeofday () in
  Printf.printf "%s [%0.1f secs]\n%!" name (end_time -. start_time);
  result
;;

(* Run the application *)
let main (): unit =
  Daemon.check_entry_point ();
  PidLog.init "/tmp/hh_server/global_index_builder.pids";
  PidLog.log ~reason:"main" (Unix.getpid ());

  (* Gather list of files *)
  let ctxt = parse_options () in
  Printf.printf "Scanning repository %s... %!" ctxt.repo_folder;
  let files = measure_time ~f:(fun () -> gather_file_list ctxt.repo_folder) ~name:"" in

  (* Spawn the parallel parser *)
  Printf.printf "Parsing %d files... %!" (List.length files);
  let workers = Some (init_workers ()) in
  let results = measure_time ~f:(fun () -> parallel_parse ~workers files) ~name:"" in

  (* Are we exporting a sqlite file? *)
  begin
    match ctxt.sqlite_filename with
    | None ->
      ()
    | Some filename ->
      Printf.printf "Writing %d symbols to sqlite... %!"
        (List.length results);
      measure_time ~f:(fun () ->
          SqliteIndexWriter.record_in_db filename results;
        ) ~name:"";
  end;

  (* Are we exporting a text file? *)
  begin
    match ctxt.text_filename with
    | None ->
      ()
    | Some filename ->
      Printf.printf "Writing %d symbols to text... %!"
        (List.length results);
      measure_time ~f:(fun () ->
          TextIndexWriter.record_in_textfile filename results;
        ) ~name:"";
  end;

  (* Are we exporting a json file? *)
  begin
    match ctxt.json_filename with
    | None ->
      ()
    | Some filename ->
      Printf.printf "Writing %d symbols to json... %!"
        (List.length results);
      measure_time ~f:(fun () ->
          JsonIndexWriter.record_in_jsonfile filename results;
        ) ~name:"";
  end;
  ()
;;

let () =
  let _ = measure_time ~f:(fun () -> main ()) ~name:"\n\nGlobal Index Built successfully:" in
  Printf.printf "Done%s" "";
