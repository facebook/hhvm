(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type t = {
  warning_code: int;
  pos: Pos.t;
}

type parse_result = {
  warnings: t list Relative_path.Map.t;
  input_diagnostics: int;
  matching_diagnostics: int;
  codemoddable_diagnostics: int;
  unique_targets: int;
}

type raw_warning = {
  warning_code: int;
  line: int;
  start: int;
  end_: int;
}

type parsed_warning =
  | Unrelated
  | Uncodemoddable
  | Codemoddable of Relative_path.t * raw_warning

let parse_raw_warning_json (warning_json : Yojson.Safe.t) :
    (parsed_warning, string) result =
  let open Result.Let_syntax in
  let extract_key key = function
    | `Assoc assoc -> begin
      match List.Assoc.find assoc ~equal:String.equal key with
      | Some value -> Ok value
      | None -> Error (Printf.sprintf "missing member %s" key)
    end
    | _ -> Error "expected an object"
  in
  let extract_int = function
    | `Int i -> Ok i
    | _ -> Error "expected an integer"
  in
  let extract_string = function
    | `String s -> Ok s
    | _ -> Error "expected a string"
  in
  let extract_list = function
    | `List l -> Ok l
    | _ -> Error "expected an array"
  in
  let field key extract json =
    let* value = extract_key key json in
    Result.map_error (extract value) ~f:(fun error ->
        Printf.sprintf "%s: %s" key error)
  in
  let* messages = field "message" extract_list warning_json in
  let* first_message =
    match messages with
    | first :: _ -> Ok first
    | [] -> Error "message array is empty"
  in
  let* raw_error_code = field "code" extract_int first_message in
  let* descr = field "descr" extract_string first_message in
  (* Override, body, and forwarding-call violations are typing errors;
     class-call violations are warnings. The validated code is carried
     through as an integer. *)
  let codemoddable warning_code message =
    let* path = field "path" extract_string message in
    let root = Relative_path.path_of_prefix Relative_path.Root in
    let* path =
      if String.is_prefix path ~prefix:root then
        Ok (Relative_path.create Relative_path.Root path)
      else
        Error
          (Printf.sprintf "path: expected a path under %s, got %S" root path)
    in
    let* line = field "line" extract_int message in
    let* start = field "start" extract_int message in
    let* end_ = field "end" extract_int message in
    Ok (Codemoddable (path, { warning_code; line; start; end_ }))
  in
  match
    ( Error_codes.Warning.of_enum raw_error_code,
      Error_codes.Typing.of_enum raw_error_code )
  with
  | (Some Error_codes.Warning.CallNeedsConcrete, _)
  | (_, Some Error_codes.Typing.CallNeedsConcrete) ->
    if
      List.exists ["self"; "parent"; "static"] ~f:(fun receiver ->
          String.is_substring
            descr
            ~substring:(Printf.sprintf " via `%s`." receiver))
    then
      codemoddable raw_error_code first_message
    else
      Ok Uncodemoddable
  | ( _,
      Some
        ( Error_codes.Typing.AbstractAccessViaStatic
        | Error_codes.Typing.UninstantiableClassViaStatic ) ) ->
    codemoddable raw_error_code first_message
  | (_, Some Error_codes.Typing.NeedsConcreteOverride) ->
    let rec find_targets targets = function
      | [] -> Ok targets
      | message :: rest ->
        let* descr = field "descr" extract_string message in
        let targets =
          if String.equal descr "Previously defined here" then
            message :: targets
          else
            targets
        in
        find_targets targets rest
    in
    let* targets = find_targets [] messages in
    begin
      match targets with
      | [target] -> codemoddable raw_error_code target
      | _ ->
        Error
          (Printf.sprintf
             "expected exactly one override target, got %d"
             (List.length targets))
    end
  | _ -> Ok Unrelated

let warning_of_raw
    (path : Relative_path.t)
    (source_text : Full_fidelity_source_text.t)
    ({ warning_code; line; start; end_ } : raw_warning) : t =
  let beginning_of_line =
    Full_fidelity_source_text.position_to_offset source_text (line, 0)
  in
  let pos =
    Pos.make_from_lnum_bol_offset
      ~pos_file:path
      ~pos_start:(line, beginning_of_line, beginning_of_line + start)
      ~pos_end:(line, beginning_of_line, beginning_of_line + end_)
  in
  { warning_code; pos }

let parse_warnings_json_file (path : string) : (parse_result, string) result =
  let channel = Stdlib.open_in_bin path in
  Exn.protect
    ~f:(fun () ->
      let open Result.Let_syntax in
      let raw_warnings = ref Relative_path.Map.empty in
      let input_diagnostics = ref 0 in
      let matching_diagnostics = ref 0 in
      let codemoddable_diagnostics = ref 0 in
      let seen_errors = ref false in
      let add_warning warning_path warning =
        raw_warnings :=
          Relative_path.Map.update
            warning_path
            (fun warnings ->
              let warnings = Option.value warnings ~default:Set.Poly.empty in
              Some (Set.add warnings warning))
            !raw_warnings
      in
      let read_warning () lexer lexbuf =
        incr input_diagnostics;
        let warning_json = Yojson.Safe.read_json lexer lexbuf in
        match parse_raw_warning_json warning_json with
        | Error error ->
          raise
            (Yojson.Json_error
               (Printf.sprintf
                  "unexpected diagnostic %d: %s"
                  !input_diagnostics
                  error))
        | Ok Unrelated -> ()
        | Ok Uncodemoddable -> incr matching_diagnostics
        | Ok (Codemoddable (warning_path, warning)) ->
          incr matching_diagnostics;
          incr codemoddable_diagnostics;
          add_warning warning_path warning
      in
      let read_field () name lexer lexbuf =
        if String.equal name "errors" then begin
          if !seen_errors then
            raise (Yojson.Json_error "duplicate errors member");
          seen_errors := true;
          Yojson.Safe.read_sequence read_warning () lexer lexbuf
        end else
          Yojson.Safe.skip_json lexer lexbuf
      in
      let* () =
        try
          let lexer = Yojson.init_lexer () in
          let lexbuf = Lexing.from_channel channel in
          Yojson.Safe.read_space lexer lexbuf;
          Yojson.Safe.read_abstract_fields
            Yojson.Safe.read_string
            read_field
            ()
            lexer
            lexbuf;
          Yojson.Safe.read_space lexer lexbuf;
          if not (Yojson.Safe.read_eof lexbuf) then
            Error "unexpected data after top-level object"
          else if not !seen_errors then
            Error "missing errors member"
          else
            Ok ()
        with
        | Yojson.Json_error error -> Error error
      in
      let unique_targets =
        Relative_path.Map.fold !raw_warnings ~init:0 ~f:(fun _ warnings total ->
            total + Set.length warnings)
      in
      let warnings =
        Relative_path.Map.mapi !raw_warnings ~f:(fun warning_path warnings ->
            let source_text =
              Full_fidelity_source_text.make
                warning_path
                (Disk.cat (Relative_path.to_absolute warning_path))
            in
            Set.to_list warnings
            |> List.map ~f:(warning_of_raw warning_path source_text))
      in
      Ok
        {
          warnings;
          input_diagnostics = !input_diagnostics;
          matching_diagnostics = !matching_diagnostics;
          codemoddable_diagnostics = !codemoddable_diagnostics;
          unique_targets;
        })
    ~finally:(fun () -> Stdlib.close_in_noerr channel)
  |> Result.map_error ~f:(fun error ->
         Printf.sprintf "invalid hh JSON in %s: %s" path error)
