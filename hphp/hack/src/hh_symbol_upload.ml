(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type parse_options = {
  service: string;
  json_repo: string;
  glean_repo: string;
  www_repo: string;
}

type parse_result = (parse_options, string) result

let usage =
  Printf.sprintf
    "Usage: %s [opts] [repository]\n"
    Sys.argv.(0)

let parse_options () =
  let service = ref None in
  let json_repo = ref None in
  let glean_repo = ref None in
  let www_repo = ref None in

  let options = ref [
    "--www-repo",
    Arg.String (fun x -> www_repo := (Some x)),
    "WWW Repository path";

    "--service",
    Arg.String (fun x -> service := (Some x)),
    "Glean service path";

    "--glean-repo",
    Arg.String (fun x -> glean_repo := (Some x)),
    "Glean repo name";
  ] in

  Arg.parse_dynamic options (fun arg -> json_repo := (Some arg)) usage;
  match !json_repo, !service, !glean_repo, !www_repo with
  | None, _, _, _
  | _, None, _, _
  | _, _, None, _
  | _, _, _, None ->
    Error (Arg.usage_string !options usage)
  | Some json_repo, Some service, Some glean_repo, Some www_repo ->
    Ok
    { service = service; json_repo = json_repo; glean_repo = glean_repo; www_repo = www_repo }

let send_to_service ~glean_service ~json_repo ~glean_repo ~www_repo =
  (* making sure we are only getting the json files in the repository *)
  let files = Sys_utils.collect_paths begin fun filename ->
    Str.string_match
      ( Str.regexp "[./a-zA-Z0-9_]+.json" )
      filename
      0
  end json_repo in
  CustomSymbolIndexWriter.send_to_custom_writer
    files
    glean_service
    glean_repo
    www_repo

(* entry point *)
let () =
  Daemon.check_entry_point ();
  let parse_result = parse_options () in
  match parse_result with
  | Error message ->
    Printf.eprintf "%s" message;
  | Ok parse_options ->
    let glean_service, json_repo, glean_repo, www_repo =
      parse_options.service,
      parse_options.json_repo,
      parse_options.glean_repo,
      parse_options.www_repo
    in

    Hh_logger.log
      "Sending JSON files from %s to glean service: %s/%s"
      glean_service json_repo glean_repo;

    send_to_service ~glean_service ~json_repo ~glean_repo ~www_repo
