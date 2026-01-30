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

(* FFI binding to the Rust config parser *)
external rust_parse_config_to_global_options :
  string -> string -> (GlobalOptions.t, string) result
  = "rust_parse_config_to_global_options"

(* Read file contents, with error handling *)
let read_file path =
  try Sys_utils.cat path with
  | exn ->
    failwith
      (Printf.sprintf "Failed to read file %s: %s" path (Exn.to_string exn))

(* Test: OCaml and Rust produce equivalent GlobalOptions from hh.conf *)
let test_hh_conf_ocaml_rust_equivalence hh_conf_path () =
  Printf.printf "\n=== Test: hh.conf OCaml vs Rust equivalence ===\n";
  Printf.printf "  hh.conf: %s\n\n" hh_conf_path;
  let hh_conf_contents = read_file hh_conf_path in
  let empty_hh_conf = "" in
  (* Parse hh.conf using OCaml's Config_file_common *)
  let hh_conf_config = Config_file_common.parse_contents hh_conf_contents in
  (* Apply config to GlobalOptions using OCaml's ServerConfig.load_config *)
  let ocaml_opts =
    ServerConfig.load_config hh_conf_config GlobalOptions.default
  in
  (* Parse using Rust via FFI *)
  let rust_opts =
    match
      rust_parse_config_to_global_options hh_conf_contents empty_hh_conf
    with
    | Ok opts -> opts
    | Error msg -> failwith ("Rust parse failed: " ^ msg)
  in
  (* Compare entire GlobalOptions structures *)
  if GlobalOptions.equal ocaml_opts rust_opts then begin
    Printf.printf "SUCCESS: OCaml and Rust produce equivalent GlobalOptions\n";
    true
  end else begin
    Printf.printf "FAILURE: GlobalOptions differ between OCaml and Rust\n";
    Printf.printf "\nOCaml GlobalOptions:\n%s\n" (GlobalOptions.show ocaml_opts);
    Printf.printf "\nRust GlobalOptions:\n%s\n" (GlobalOptions.show rust_opts);
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
        ( Printf.sprintf "%s:ocaml_rust_equivalence" name,
          test_hh_conf_ocaml_rust_equivalence hh_conf_path ))
  in
  Unit_test.run_all tests
