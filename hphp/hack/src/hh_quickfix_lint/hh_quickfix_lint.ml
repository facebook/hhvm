(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module JSON = Hh_json

type patch = {
  path: string;
  start_offset: int;
  line: int;
  column: int;
  width: int;
  original: string;
  replacement: string;
}

type error_count = {
  overlapping: int;
  original_not_found: int;
}

type input =
  | Stdin
  | File of string

type options = {
  verbose: bool;
  overwrite: bool;
  input: input;
}

let log_dropped ~verbose error_count patch =
  if verbose then
    Printf.eprintf
      "An overlapping patch is dropped %s:%d:%d\n"
      patch.path
      patch.line
      patch.column;
  { error_count with overlapping = error_count.overlapping + 1 }

let log_original_not_found ~verbose error_count patch =
  if verbose then
    Printf.eprintf
      "Could not find the original pattern at %s:%d:%d.\n"
      patch.path
      patch.line
      patch.column;
  { error_count with original_not_found = error_count.original_not_found + 1 }

let usage_str =
  let str =
    Format.sprintf
      "usage: %s [--input-file FILE] [--overwrite] [--verbose]\n"
      Sys.argv.(0)
  in
  let str =
    Format.sprintf
      "%s  The input file can be a JSON array of lints or an object with an `errors` field containing that array. \n"
      str
  in
  str

let usage_err () =
  Printf.eprintf "%s" usage_str;
  exit 1

let path = "path"

let start_offset = "start_offset"

let line = "line"

let start = "start"

let width = "width"

let original = "original"

let replacement = "replacement"

let field_type_err field ty =
  Printf.eprintf
    "There is an object with a '%s' field that is not an %s\n\n"
    field
    ty;
  usage_err ()

let int_field_opt = function
  | JSON.JSON_Number n -> begin
    try Some (Int.of_string n) with
    | _ -> None
  end
  | _ -> None

let int_field field json =
  match int_field_opt json with
  | Some value -> value
  | None -> field_type_err field "int"

let string_field_opt = function
  | JSON.JSON_String n -> Some n
  | _ -> None

let string_field field json =
  match string_field_opt json with
  | Some str -> str
  | None -> field_type_err field "string"

let find_field_value obj target =
  let value_opt =
    List.find_map obj ~f:(function
        | (field, value) when String.equal field target -> Some value
        | _ -> None)
  in
  match value_opt with
  | Some value -> value
  | None ->
    Printf.eprintf
      "Input JSON has an object with a missing '%s' field\n\n"
      target;
    usage_err ()

let patch_of_object json =
  let obj =
    match json with
    | JSON.JSON_Object obj -> obj
    | _ ->
      Printf.eprintf "Input JSON array has a non-object.\n\n";
      usage_err ()
  in
  let find_field_value = find_field_value obj in
  let path = find_field_value path |> string_field path in
  let line = find_field_value line |> int_field line in
  let column = find_field_value start |> int_field start in
  let start_offset_opt = find_field_value start_offset |> int_field_opt in
  let width_opt = find_field_value width |> int_field_opt in
  let original_opt = find_field_value original |> string_field_opt in
  let replacement_opt = find_field_value replacement |> string_field_opt in
  match (start_offset_opt, width_opt, original_opt, replacement_opt) with
  | (Some start_offset, Some width, Some original, Some replacement) ->
    Some { path; start_offset; line; column; width; original; replacement }
  | _ -> None

let sanitise_and_extract_patches json =
  let err () =
    Printf.eprintf
      "Input JSON is not an array or an object with 'errors' field.\n\n";
    usage_err ()
  in
  let jsonl =
    match json with
    | JSON.JSON_Object fields ->
      let errors_opt =
        List.find fields ~f:(fun (field, _) -> String.equal field "errors")
      in
      begin
        match errors_opt with
        | Some (_, JSON.JSON_Array jsonl) -> jsonl
        | _ -> err ()
      end
    | JSON.JSON_Array jsonl -> jsonl
    | _ -> err ()
  in
  List.filter_map jsonl ~f:patch_of_object

let apply_patch
    ~verbose
    (error_count, contents)
    ({ start_offset; original; replacement; _ } as patch) =
  match String.substr_index contents ~pos:start_offset ~pattern:original with
  | Some n when n = start_offset ->
    let contents =
      String.substr_replace_first
        contents
        ~pos:start_offset
        ~pattern:original
        ~with_:replacement
    in
    (error_count, contents)
  | _ ->
    let error_count = log_original_not_found ~verbose error_count patch in
    (error_count, contents)

let apply_patch_group ~verbose error_count contents group =
  let group =
    let compare a b = Int.compare a.start_offset b.start_offset in
    List.sort ~compare group
  in
  let rec drop_super_patches acc error_count = function
    | patch :: patch' :: patches ->
      if patch'.start_offset <= patch.start_offset + patch.width then begin
        let error_count = log_dropped ~verbose error_count patch in
        drop_super_patches acc error_count (patch' :: patches)
      end else
        drop_super_patches (patch :: acc) error_count (patch' :: patches)
    | [patch] -> (error_count, patch :: acc)
    | [] -> (error_count, acc)
  in
  let (error_count, group) = drop_super_patches [] error_count group in
  List.fold ~f:(apply_patch ~verbose) ~init:(error_count, contents) group

let parse_and_validate_options () =
  let verbose = ref false in
  let overwrite = ref false in
  let input = ref Stdin in
  let rec process = function
    | "--verbose" :: rest ->
      verbose := true;
      process rest
    | "--overwrite" :: rest ->
      overwrite := true;
      process rest
    | "--input-file" :: path :: rest ->
      input := File path;
      process rest
    | arg :: _ ->
      Printf.eprintf "Unrecognised CLI argument %s\n\n" arg;
      usage_err ()
    | [] -> ()
  in
  process (List.tl_exn @@ Array.to_list Sys.argv);
  { verbose = !verbose; overwrite = !overwrite; input = !input }

let output ~overwrite path contents =
  if overwrite then begin
    let open Out_channel in
    with_file path ~f:(fun fd -> output_string fd contents)
  end else begin
    Format.printf "//// %s\n" path;
    Format.printf "%s\n\n" contents
  end

let () =
  let args = parse_and_validate_options () in
  let json_str =
    let open In_channel in
    match args.input with
    | Stdin -> input_all stdin
    | File json_path -> with_file json_path ~f:input_all
  in
  let json = JSON.json_of_string json_str in
  let patches = sanitise_and_extract_patches json in
  let patch_groups =
    patches
    |> List.sort ~compare:(fun a b -> String.compare a.path b.path)
    |> List.group ~break:(fun a b -> not @@ String.equal a.path b.path)
  in
  let error_count = { overlapping = 0; original_not_found = 0 } in
  let error_count =
    List.fold patch_groups ~init:error_count ~f:(fun error_count group ->
        let path = (List.hd_exn group).path in
        let open In_channel in
        let (error_count, patched_contents) =
          with_file path ~f:(fun fd ->
              let contents = input_all fd in
              apply_patch_group ~verbose:args.verbose error_count contents group)
        in
        output ~overwrite:args.overwrite path patched_contents;
        error_count)
  in
  Format.printf
    "There were input %d patches for %d files.\n"
    (List.length patches)
    (List.length patch_groups);
  Format.printf
    "Dropped patch count: %d (%d overlapping, %d original not found)\n"
    (error_count.overlapping + error_count.original_not_found)
    error_count.overlapping
    error_count.original_not_found
