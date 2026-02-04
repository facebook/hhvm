(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

(**
 * Test that OCaml and Rust configuration parsers produce equivalent results.
 *)

open Hh_prelude

(* FFI binding to the Rust config parser - returns full HhConfig *)
external rust_parse_config_to_hh_config :
  string -> string -> (Hh_config.t, string) result
  = "rust_parse_config_to_hh_config"

(* Read file contents, with error handling *)
let read_file path =
  try Sys_utils.cat path with
  | exn ->
    failwith
      (Printf.sprintf "Failed to read file %s: %s" path (Exn.to_string exn))

(* Build OCaml Hh_config from parsed config contents.
   This parses the config directly without reading from /etc/hh.conf,
   using ServerLocalConfigLoad.load_from_config for consistent parsing. *)
let build_ocaml_hh_config contents =
  let config = Config_file_common.parse_contents contents in
  (* Load GlobalOptions via ServerConfig.load_config *)
  let opts = ServerConfig.load_config config GlobalOptions.default in
  (* Load ServerLocalConfig from parsed config *)
  let version = Config_file_version.parse_version None in
  let local_config =
    ServerLocalConfigLoad.load_from_config
      ~silent:true
      ~current_version:version
      ~current_rolled_out_flag_idx:0
      ~deactivate_saved_state_rollout:false
      config
  in
  {
    Hh_config.version = local_config.ServerLocalConfig.config_version;
    ignored_paths = local_config.ServerLocalConfig.ignored_paths;
    hash = "";
    opts;
    gc_minor_heap_size = local_config.ServerLocalConfig.gc_minor_heap_size;
    gc_space_overhead = local_config.ServerLocalConfig.gc_space_overhead;
    hackfmt_version = local_config.ServerLocalConfig.hackfmt_version;
    sharedmem_dep_table_pow =
      local_config.ServerLocalConfig.sharedmem_dep_table_pow;
    sharedmem_global_size = local_config.ServerLocalConfig.sharedmem_global_size;
    sharedmem_hash_table_pow =
      local_config.ServerLocalConfig.sharedmem_hash_table_pow;
    sharedmem_heap_size = local_config.ServerLocalConfig.sharedmem_heap_size;
    ide_fall_back_to_full_index =
      local_config.ServerLocalConfig.ide_fall_back_to_full_index;
    hh_distc_should_disable_trace_store =
      opts.GlobalOptions.hh_distc_should_disable_trace_store;
    hh_distc_exponential_backoff_num_retries =
      local_config.ServerLocalConfig.hh_distc_exponential_backoff_num_retries;
    naming_table_compression_level =
      local_config.ServerLocalConfig.naming_table_compression_level;
    naming_table_compression_threads =
      local_config.ServerLocalConfig.naming_table_compression_threads;
    eden_fetch_parallelism =
      local_config.ServerLocalConfig.eden_fetch_parallelism;
    use_distc_crawl_dircache =
      local_config.ServerLocalConfig.use_distc_crawl_dircache;
    distc_avoid_unnecessary_saved_state_work =
      local_config.ServerLocalConfig.distc_avoid_unnecessary_saved_state_work;
    distc_write_trace_during_save_state_creation_only =
      local_config
        .ServerLocalConfig.distc_write_trace_during_save_state_creation_only;
  }

(* Test: OCaml and Rust produce equivalent full HhConfig from hh.conf *)
let test_hh_config_equivalence hh_conf_path () =
  Printf.printf "\n=== Test: hh.conf Full HhConfig equivalence ===\n";
  Printf.printf "  hh.conf: %s\n\n" hh_conf_path;
  let contents = read_file hh_conf_path in
  let empty_hh_conf = "" in
  let ocaml_config = build_ocaml_hh_config contents in
  let rust_config =
    match rust_parse_config_to_hh_config contents empty_hh_conf with
    | Ok c -> c
    | Error msg -> failwith ("Rust parse failed: " ^ msg)
  in
  (* Compare with hash excluded *)
  let ocaml_no_hash = { ocaml_config with Hh_config.hash = "" } in
  let rust_no_hash = { rust_config with Hh_config.hash = "" } in
  if Hh_config.equal ocaml_no_hash rust_no_hash then begin
    Printf.printf "SUCCESS: OCaml and Rust produce equivalent HhConfig\n";
    true
  end else begin
    Printf.printf "FAILURE: HhConfig differs between OCaml and Rust\n";
    Printf.printf "\nOCaml HhConfig:\n%s\n" (Hh_config.show ocaml_config);
    Printf.printf "\nRust HhConfig:\n%s\n" (Hh_config.show rust_config);
    false
  end

let () =
  (* Get config file paths from command-line arguments *)
  let args = Sys.argv in
  if Array.length args < 2 then begin
    Printf.eprintf "Usage: %s <hh.conf> [<hh.conf> ...]\n" args.(0);
    Stdlib.exit 1
  end;
  let config_files =
    Array.to_list args
    |> List.tl_exn (* Skip program name *)
    |> List.sort ~compare:String.compare
  in
  (* Generate tests for each config file *)
  let tests =
    List.map config_files ~f:(fun hh_conf_path ->
        let name = Filename.basename hh_conf_path in

        ( Printf.sprintf "%s:hh_config_equivalence" name,
          test_hh_config_equivalence hh_conf_path ))
  in
  Unit_test.run_all tests
