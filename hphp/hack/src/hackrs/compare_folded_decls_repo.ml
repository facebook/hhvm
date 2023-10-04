(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let purpose = "Compare Rust and OCaml Decl Folding on www repo"

let usage =
  Printf.sprintf "Usage: %s <options> www-root\n%s" Sys.argv.(0) purpose

let make_workers
    (root : Path.t)
    (server_config : ServerConfig.t)
    (server_local_config : ServerLocalConfig.t) : MultiWorker.worker list =
  let num_workers = Sys_utils.nbr_procs in
  let gc_control = Gc.get () in
  let hhconfig_version =
    server_config |> ServerConfig.version |> Config_file.version_to_string_opt
  in
  let shmem_config = ServerConfig.sharedmem_config server_config in
  let heap_handle = SharedMem.init ~num_workers shmem_config in
  ServerWorker.make
    ~longlived_workers:true
    ~nbr_procs:num_workers
    gc_control
    heap_handle
    ~logging_init:(fun () ->
      HackEventLogger.init_worker
        ~root
        ~always_add_sandcastle_info:
          server_local_config.ServerLocalConfig.log_events_with_sandcastle_info
        ~custom_columns:[]
        ~rollout_flags:(ServerLocalConfig.to_rollout_flags server_local_config)
        ~rollout_group:server_local_config.ServerLocalConfig.rollout_group
        ~hhconfig_version
        ~init_id:(Random_id.short_string ())
        ~time:(Unix.gettimeofday ())
        ~per_file_profiling:
          server_local_config.ServerLocalConfig.per_file_profiling)

let measure_time (action : string) (f : unit -> 'a) : 'a =
  Hh_logger.log "Start %s..." action;
  let start_time = Unix.gettimeofday () in
  let result = f () in
  let _ =
    Hh_logger.log_duration (Printf.sprintf "Finished %s" action) start_time
  in
  result

let init (root : Path.t) (naming_table_path : string option) :
    Provider_context.t * Naming_table.t option * MultiWorker.worker list option
    =
  Relative_path.set_path_prefix Relative_path.Root root;
  Relative_path.set_path_prefix Relative_path.Tmp (Path.make "tmpdir_NOT_USED");
  Relative_path.set_path_prefix Relative_path.Hhi (Hhi.get_hhi_root ());

  let server_args = ServerArgs.default_options ~root:(Path.to_string root) in
  let (server_config, server_local_config) =
    ServerConfig.load ~silent:true server_args
  in
  let popt = ServerConfig.parser_options server_config in
  let tcopt = { popt with GlobalOptions.tco_higher_kinded_types = true } in
  let ctx =
    Provider_context.empty_for_tool
      ~popt
      ~tcopt
      ~backend:Provider_backend.Shared_memory
      ~deps_mode:(Typing_deps_mode.InMemoryMode None)
  in
  let workers = make_workers root server_config server_local_config in
  Hh_logger.log
    "About to decl %s with %d workers %s"
    (Path.to_string root)
    (List.length workers)
    (match naming_table_path with
    | Some path -> Printf.sprintf "with a naming table at %s" path
    | None -> "without a naming table");
  let naming_table =
    Option.map naming_table_path ~f:(fun path ->
        Naming_table.load_from_sqlite ctx path)
  in
  (ctx, naming_table, Some workers)

let parse_repo
    (ctx : Provider_context.t)
    (root : Path.t)
    (workers : MultiWorker.worker list option) : FileInfo.t Relative_path.Map.t
    =
  let get_next =
    ServerUtils.make_next
      ~hhi_filter:(fun _ -> true)
      ~indexer:
        (Find.make_next_files ~name:"root" ~filter:FindUtils.is_hack root)
      ~extra_roots:(ServerConfig.extra_paths ServerConfig.default_config)
  in
  measure_time "parsing repo" @@ fun () ->
  Direct_decl_service.go ctx workers ~get_next ~trace:false ~cache_decls:true

let fold_and_compare_single_decl
    output_dir should_print (ctx : Provider_context.t) decl_class_name rust_decl
    : bool =
  let ocaml_decl_opt =
    Decl_folded_class.class_decl_if_missing
      ~sh:SharedMem.Uses
      ctx
      decl_class_name
  in
  (* Incidentally test here that Rust and OCaml produce the same marshaled
     bytes. *)
  let ocaml_marshaled = Marshal.to_string ocaml_decl_opt [] in
  let rust_marshaled = Ocamlrep_marshal_ffi.to_string ocaml_decl_opt [] in
  let _ =
    if not (String.equal rust_marshaled ocaml_marshaled) then
      failwith
        (Printf.sprintf
           "Marshaling of '%s' differs between Rust and OCaml. This indicates 'ocamlrep_marshal_output_value_to_string' is broken."
           decl_class_name)
    else
      ()
  in
  (* Another incidental test here, this time that Rust unmarshaling works as
     expected. *)
  let rust_decl_opt = Ocamlrep_marshal_ffi.from_string rust_marshaled 0 in
  let rust_read_back_matched =
    match (ocaml_decl_opt, rust_decl_opt) with
    | (Some (ocaml_decl, _), Some (rust_decl, _)) ->
      String.equal
        (Decl_defs.show_decl_class_type ocaml_decl)
        (Decl_defs.show_decl_class_type rust_decl)
    | (None, None) -> true
    | _ -> false
  in
  let _ =
    if not rust_read_back_matched then begin
      Printf.printf "Rust decl unmarshaling failed:\n%!";
      if Option.is_some ocaml_decl_opt then
        Printf.printf
          "ocaml:\n%s\n!"
          (Decl_defs.show_decl_class_type
             (fst (Option.value_exn ocaml_decl_opt)))
      else
        Printf.printf "ocaml:\nNone\n";
      if Option.is_some rust_decl_opt then
        Printf.printf
          "rust:\n%s\n!"
          (Decl_defs.show_decl_class_type
             (fst (Option.value_exn rust_decl_opt)))
      else
        Printf.printf "ocaml:\nNone\n"
    end
  in
  (* The real test: are the Rust decl and OCaml decl the same? *)
  match ocaml_decl_opt with
  | Some (ocaml_decl, _) ->
    let is_identical =
      Decl_folded_class_rupro.decls_equal ocaml_decl rust_decl
    in
    let () =
      if (not is_identical) && should_print then
        let ocaml_decl_str =
          Decl_folded_class_rupro.show_decl_class_type ocaml_decl
        in
        let rust_decl_str =
          Decl_folded_class_rupro.show_decl_class_type rust_decl
        in
        let file_path =
          let file_path_len_limit = 250 in
          let file_path = decl_class_name ^ ".rust.folded_decl" in
          let file_path_len = String.length file_path in
          let file_path =
            if file_path_len > file_path_len_limit then
              String.sub
                file_path
                ~pos:(file_path_len - file_path_len_limit)
                ~len:file_path_len_limit
            else
              file_path
          in
          Path.to_string (Path.concat output_dir file_path)
        in
        Disk.write_file
          ~file:file_path
          ~contents:
            ("OCaml Decl:\n"
            ^ ocaml_decl_str
            ^ "\n"
            ^ "Rust Decl:\n"
            ^ rust_decl_str
            ^ "\n")
    in
    is_identical
  | None ->
    let _ = Hh_logger.error "[OCaml] Could not fold class %s" decl_class_name in
    false

let fold_repo
    (ctx : Provider_context.t)
    (workers : MultiWorker.worker list option)
    rust_decl_map
    output_dir
    print_limit =
  let rust_decls = SMap.bindings rust_decl_map in
  measure_time "folding repo" @@ fun () ->
  let (num_correct, _num_checked) =
    MultiWorker.call
      workers
      ~job:(fun _ batch ->
        List.fold
          ~f:(fun (num_correct, num_checked) (rust_class_name, rust_decl) ->
            let same =
              fold_and_compare_single_decl
                output_dir
                (num_checked - num_correct < print_limit)
                ctx
                rust_class_name
                rust_decl
            in
            if same then
              (num_correct + 1, num_checked + 1)
            else
              (num_correct, num_checked + 1))
          ~init:(0, 0)
          batch)
      ~merge:(fun (i_correct, i_total) (j_correct, j_total) ->
        (i_correct + j_correct, i_total + j_total))
      ~neutral:(0, 0)
      ~next:(MultiWorker.next workers rust_decls)
  in
  (num_correct, List.length rust_decls)

let () =
  Daemon.check_entry_point ();
  Folly.ensure_folly_init ();
  let repo = ref None in
  let naming_table_path = ref None in
  let output_dir = ref None in
  let num_partitions = ref 1 in
  let partition_index = ref 0 in
  let print_limit = ref 10 in
  let args =
    [
      ( "--naming-table",
        Arg.String (fun s -> naming_table_path := Some s),
        " Path to a SQLite naming table (allowing parsing to be done lazily instead of up-front)"
      );
      ( "--output-dir",
        Arg.String (fun s -> output_dir := Some s),
        " Path to write decl differences to" );
      ( "--partition-index",
        Arg.Int (fun i -> partition_index := i),
        " What section of folded decls we compare" );
      ( "--num-partitions",
        Arg.Int (fun i -> num_partitions := i),
        " How many ranges there should be" );
      ( "--print-limit",
        Arg.Int (fun i -> print_limit := i),
        " How many diffs should be written per OCaml process" );
    ]
  in
  Arg.parse args (fun filepath -> repo := Some filepath) usage;

  let (www_root, output_dir) =
    match (!repo, !output_dir) with
    | (Some root, Some path_to_write) ->
      (Path.make root, Path.make path_to_write)
    | _ ->
      Arg.usage args usage;
      Exit.exit Exit_status.Input_error
  in
  let rust_popt_init () =
    Relative_path.set_path_prefix Relative_path.Root www_root;
    Relative_path.set_path_prefix
      Relative_path.Tmp
      (Path.make "tmpdir_NOT_USED");
    Relative_path.set_path_prefix Relative_path.Hhi (Hhi.get_hhi_root ());
    let server_args =
      ServerArgs.default_options ~root:(Path.to_string www_root)
    in
    let (server_config, _server_local_config) =
      ServerConfig.load ~silent:true server_args
    in
    let popt = ServerConfig.parser_options server_config in
    popt
  in
  let popt = rust_popt_init () in
  let rust_decl_map =
    Decl_folded_class_rupro.partition_and_fold_dir
      ~www_root:(Path.to_string www_root)
      popt
      !num_partitions
      !partition_index
  in
  let (ctx, _naming_table_opt, workers) = init www_root !naming_table_path in
  let _ = parse_repo ctx www_root workers in
  let (number_correct, number_total) =
    fold_repo ctx workers rust_decl_map output_dir !print_limit
  in
  let _ =
    Hh_logger.log "num correct: %d out of %d" number_correct number_total
  in
  if Int.equal number_correct number_total then
    exit 0
  else
    exit 1
