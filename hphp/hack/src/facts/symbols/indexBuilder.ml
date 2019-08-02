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
open IndexBuilderTypes
open SearchUtils
open Facts

(* Keep track of all references yet to scan *)
let files_scanned = ref 0
let error_count = ref 0


(* Extract kind, abstract, and final flags *)
let get_details_from_info
    (info_opt: Facts.type_facts option): si_kind * bool * bool =
  let open Facts_parser in
  match info_opt with
  | None -> (SI_Unknown, false, false)
  | Some info -> begin
      let k = match info.kind with
      | TKClass -> SI_Class
      | TKInterface -> SI_Interface
      | TKEnum -> SI_Enum
      | TKTrait -> SI_Trait
      | TKMixed -> SI_Mixed
      | _ -> SI_Unknown
      in
      let is_abstract = ((info.flags land flags_abstract) > 0) in
      let is_final = ((info.flags land flags_final) > 0) in
      (k, is_abstract, is_final)
  end
;;

(* Parse one single file and capture information about it *)
let parse_file
    (ctxt: index_builder_context)
    (filename: string): si_capture =
  if Sys.is_directory filename then begin
    []
  end else begin
    let path = String.substr_replace_first
      filename ~pattern:ctxt.repo_folder ~with_:"" in
    let text = In_channel.read_all filename in
    let rp = Relative_path.from_root filename in

    (* Just the facts ma'am *)
    Facts_parser.mangle_xhp_mode := false;
    let fact_opt = Facts_parser.from_text
      ~php5_compat_mode:false
      ~hhvm_compat_mode:true
      ~filename:rp
      ~text in

    (* Iterate through facts and print them out *)
    let result =
      match fact_opt with
      | Some facts ->

        (* Identify all classes in the file *)
        let class_keys = InvSMap.keys facts.types in
        let classes_mapped = List.map class_keys ~f:(fun key -> begin
          let info_opt = InvSMap.get key facts.types in
          let (kind, is_abstract, is_final) = get_details_from_info info_opt in
          {
            sif_name = key;
            sif_kind = kind;
            sif_filepath = path;
            sif_is_abstract = is_abstract;
            sif_is_final = is_final;
          }
        end) in

        (* Identify all functions in the file *)
        let functions_mapped = List.map facts.functions ~f:(fun funcname -> {
          sif_name = funcname;
          sif_kind = SI_Function;
          sif_filepath = path;
          sif_is_abstract = false;
          sif_is_final = false;
        }) in

        (* Handle typedefs *)
        let types_mapped = List.map facts.type_aliases ~f:(fun typename -> {
          sif_name = typename;
          sif_kind = SI_Typedef;
          sif_filepath = path;
          sif_is_abstract = false;
          sif_is_final = false;
        }) in

        (* Handle constants *)
        let constants_mapped = List.map facts.constants ~f:(fun constantname -> {
          sif_name = constantname;
          sif_kind = SI_GlobalConstant;
          sif_filepath = path;
          sif_is_abstract = false;
          sif_is_final = false;
        }) in

        (* Return unified results *)
        let r = List.append classes_mapped functions_mapped in
        let r = List.append r types_mapped in
        let r = List.append r constants_mapped in
        r
      | None ->
        []
    in
    files_scanned := !files_scanned + 1;
    result;
  end
;;

let parse_batch
    (ctxt: index_builder_context)
    (acc: si_capture)
    (files: string list): si_capture =
  List.fold files ~init:acc ~f:begin fun acc file ->
    if Path.file_exists (Path.make file) then
      try
        let res = (parse_file ctxt) file in
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

let parallel_parse
    ~(workers: MultiWorker.worker list option)
    (files: string list)
    (ctxt: index_builder_context): si_capture =
  MultiWorker.call workers
    ~job:(parse_batch ctxt)
    ~neutral:[]
    ~merge:(List.append)
    ~next:(MultiWorker.next workers files)
;;

let entry = WorkerController.register_entry_point ~restore:(fun () -> ())
;;

(* Let's use the unix find command which seems to be really quick at this sort of thing *)
let gather_file_list (path: string): string list =
  let cmdline = Printf.sprintf
    "find %s \\( \\( -name \"*.php\" -o -name \"*.hhi\" \\) -and -not -path \"*/.hg/*\" \\)"
    path
  in
  let channel = Unix.open_process_in cmdline in
  let result = ref [] in
  (try
     while true do
       let line_opt = In_channel.input_line channel in
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

(* All data is ready.  Identify unique namespaces and filepaths *)
let convert_capture (incoming: si_capture): si_scan_result =
  let result = {
    sisr_capture = incoming;
    sisr_namespaces = Caml.Hashtbl.create 0;
    sisr_filepaths = Caml.Hashtbl.create 0;
  } in
  let ns_id = ref 1 in
  List.iter incoming ~f:(fun s ->

    (* Find / add namespace *)
    let (namespace, _name) = Utils.split_ns_from_name s.sif_name in
    if not (Caml.Hashtbl.mem result.sisr_namespaces namespace) then begin
      Caml.Hashtbl.add result.sisr_namespaces namespace !ns_id;
      incr ns_id;
    end;

    (* Find / add filepath hashes *)
    if not (Caml.Hashtbl.mem result.sisr_filepaths s.sif_filepath) then begin
      let path_hash = SharedMem.get_hash s.sif_filepath in
      Caml.Hashtbl.add result.sisr_filepaths s.sif_filepath path_hash;
    end;
  );
  result
;;

let export_to_custom_writer json_exported_files ctxt =
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
          json_exported_files service repo_name ctxt.repo_folder;
      ) ~name;
;;

(* Run the index builder project *)
let go (ctxt: index_builder_context) (workers: MultiWorker.worker list option): unit =
  if ctxt.json_repo_name <> None
  then (* if json repo is specified, just export to custom writer directly *)
    let json_exported_files =
      begin match ctxt.json_repo_name with
        | None -> []
        | Some repo_name ->
          Sys_utils.collect_paths begin fun filename ->
            Str.string_match
              ( Str.regexp "[./a-zA-Z0-9_]+.json" )
              filename
              0
          end repo_name
      end in
    export_to_custom_writer json_exported_files ctxt
  else

  (* Gather list of files *)
  let name = Printf.sprintf "Scanned repository folder [%s] in " ctxt.repo_folder in
  let files = measure_time ~f:(fun () -> gather_file_list ctxt.repo_folder) ~name in

  (* If desired, get the HHI root folder and add all HHI files from there *)
  let files = if ctxt.include_builtins then begin
    let hhi_root_folder = Hhi.get_hhi_root () in
    let hhi_root_folder_path = Path.to_string hhi_root_folder in
    let name = Printf.sprintf "Scanned HHI folder [%s] in " hhi_root_folder_path in
    let hhi_files = measure_time ~f:(fun () -> gather_file_list hhi_root_folder_path) ~name in

    (* Merge lists *)
    List.append files hhi_files
  end else files
  in

  (* Spawn the parallel parser *)
  let name = Printf.sprintf "Parsed %d files in " (List.length files) in
  let capture = measure_time ~f:(fun () -> parallel_parse ~workers files ctxt) ~name in

  (* Convert the raw capture into results *)
  let results = convert_capture capture in

  (* Are we exporting a sqlite file? *)
  begin
    match ctxt.sqlite_filename with
    | None ->
      ()
    | Some filename ->
      let name = Printf.sprintf "Wrote %d symbols to sqlite in "
        (List.length results.sisr_capture) in
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
        (List.length results.sisr_capture) in
      measure_time ~f:(fun () ->
          TextSymbolIndexWriter.record_in_textfile filename results;
        ) ~name;
  end;

  (* Are we exporting a json file? *)
  let json_exported_files =
    match ctxt.json_filename with
    | None -> []
    | Some filename ->
      let name = Printf.sprintf "Wrote %d symbols to json in "
        (List.length results.sisr_capture) in
      measure_time ~f:(fun () ->
          JsonSymbolIndexWriter.record_in_jsonfiles
            ctxt.json_chunk_size filename results;
        ) ~name
  in

  (* Are we exporting to a custom writer? *)
  export_to_custom_writer json_exported_files ctxt
;;
