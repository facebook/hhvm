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
  json_chunk_size: int;
  custom_service: string option;
  custom_repo_name: string option;
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
        Hh_logger.log "IndexBuilder exception: %s. Failed to parse [%s]"
          (Caml.Printexc.to_string exn)
          file;
        acc
    else (Hh_logger.log "File [%s] does not exist." file; acc)
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

(* Let's use the unix find command which seems to be really quick at this sort of thing *)
let gather_file_list (path: string): string list =
  let cmdline = Printf.sprintf "find %s \\( -name \"*.php\" -o -name \"*.hhi\" \\)" path in
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
  Hh_logger.log "%s [%0.1f secs]" name (end_time -. start_time);
  result
;;

(* Run the index builder project *)
let go (ctxt: index_builder_context) (workers: MultiWorker.worker list option): unit =

  (* Figure out what global revision we are on *)
  let hg_process =
    Process.exec ~cwd:(ctxt.repo_folder) "hg" ["log";"-r";".";"-T";"{globalrev}"] in
  let globalrev =
    match Process.read_and_wait_pid ~timeout:30 hg_process with
    | Ok { Process_types.stdout; _ } ->
      stdout
    | Error _ ->
      "Unknown"
  in
  Hh_logger.log "Repository [%s] is on globalrev [%s]"
    ctxt.repo_folder globalrev;

  (* Gather list of files *)
  let name = Printf.sprintf "Scanned repository folder [%s] in " ctxt.repo_folder in
  let repo_files = measure_time ~f:(fun () -> gather_file_list ctxt.repo_folder) ~name in

  (* Next, get the HHI root folder and add all HHI files from there *)
  let hhi_root_folder = Hhi.get_hhi_root () in
  let hhi_root_folder_path = Path.to_string hhi_root_folder in
  let name = Printf.sprintf "Scanned HHI folder [%s] in " hhi_root_folder_path in
  let hhi_files = measure_time ~f:(fun () -> gather_file_list hhi_root_folder_path) ~name in

  (* Merge lists *)
  let files = List.append repo_files hhi_files in

  (* Spawn the parallel parser *)
  let name = Printf.sprintf "Parsed %d files in " (List.length files) in
  let results = measure_time ~f:(fun () -> parallel_parse ~workers files) ~name in

  (* Are we exporting a sqlite file? *)
  begin
    match ctxt.sqlite_filename with
    | None ->
      ()
    | Some filename ->
      let name = Printf.sprintf "Wrote %d symbols to sqlite in "
        (List.length results) in
      measure_time ~f:(fun () ->
          SqliteSymbolIndexWriter.record_in_db filename results;
        ) ~name;
  end;

  (* Are we exporting a text file? *)
  begin
    match ctxt.text_filename with
    | None ->
      ()
    | Some filename ->
      let name = Printf.sprintf "Wrote %d symbols to text in "
        (List.length results) in
      measure_time ~f:(fun () ->
          TextSymbolIndexWriter.record_in_textfile filename results;
        ) ~name;
  end;

  (* Are we exporting a json file? *)
  let json_exported_files =
    match ctxt.json_filename with
    | None ->
      []
    | Some filename ->
      let name = Printf.sprintf "Wrote %d symbols to json in "
        (List.length results) in
      measure_time ~f:(fun () ->
          JsonSymbolIndexWriter.record_in_jsonfiles
            ctxt.json_chunk_size filename results;
        ) ~name
  in

  (* Are we exporting to a custom writer? *)
  begin
    match (ctxt.custom_service, ctxt.custom_repo_name) with
    | Some _, None
    | None, Some _ ->
      print_endline "API export requires both a service and a repo name.";
    | None, None ->
      ()
    | Some service, Some repo_name ->
      let name = Printf.sprintf "Exported to custom symbol index writer [%s] [%s] in "
        service repo_name in
      measure_time ~f:(fun () ->
          CustomSymbolIndexWriter.send_to_custom_writer
            json_exported_files service repo_name globalrev;
        ) ~name;
  end
;;
