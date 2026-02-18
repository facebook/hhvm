(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let human_formatter_for error_format =
  Diagnostics.(
    match error_format with
    | Extended -> Some Extended_diagnostic_formatter.to_string
    | Context
    | Raw
    | Highlighted
    | Plain ->
      None)

let print_diagnostic
    ~(error_format : Diagnostics.format) (e : Diagnostics.finalized_diagnostic)
    : unit =
  let human_formatter = human_formatter_for error_format in
  let hh_json =
    User_diagnostic.to_json ~human_formatter ~filename_to_string:Fun.id e
  in
  let yojson = Hh_json.to_yojson hh_json in
  let assoc =
    match yojson with
    | `Assoc fields -> ("kind", `String "diagnostic") :: fields
    | _ -> [("kind", `String "diagnostic")]
  in
  Printf.printf "%s\n%!" (Yojson.Safe.to_string (`Assoc assoc))

let print_summary ~passed ~error_count ~warning_count : unit =
  let obj =
    `Assoc
      [
        ("kind", `String "summary");
        ("passed", `Bool passed);
        ("version", `String Hh_version.version);
        ("error_count", `Int error_count);
        ("warning_count", `Int warning_count);
      ]
  in
  Printf.printf "%s\n%!" (Yojson.Safe.to_string obj)

let print_restarted ~message : unit =
  let obj =
    `Assoc [("kind", `String "restarted"); ("message", `String message)]
  in
  Printf.printf "%s\n%!" (Yojson.Safe.to_string obj)

let print_stopped ~message : unit =
  let obj =
    `Assoc [("kind", `String "stopped"); ("message", `String message)]
  in
  Printf.printf "%s\n%!" (Yojson.Safe.to_string obj)
