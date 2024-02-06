(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type half_open_one_based = {
  filename: string;  (** absolute *)
  line: int;  (** 1-based *)
  char_start: int;  (** 1-based *)
  char_end: int;  (** 1-based *)
}

let from_absolute (pos : Pos.absolute) : half_open_one_based =
  let (line, char_start, _line_end, char_end) = Pos.destruct_range pos in
  let filename = Pos.filename pos in
  { filename; line; char_start; char_end }

let pos_to_one_based_json
    ?(timestamp : float option)
    ~(half_open_interval : bool)
    ((name, pos) : string * Pos.absolute) : Hh_json.json =
  let { filename; line; char_start; char_end } =
    if half_open_interval then
      from_absolute pos
    else
      let (line, char_start, char_end) = Pos.info_pos pos in
      { filename = Pos.filename pos; line; char_start; char_end }
  in
  let timestamp =
    match timestamp with
    | None -> []
    | Some t -> [("timestamp", Hh_json.float_ t)]
  in
  Hh_json.JSON_Object
    (timestamp
    @ [
        ("name", Hh_json.JSON_String name);
        ("filename", Hh_json.JSON_String filename);
        ("line", Hh_json.int_ line);
        ("char_start", Hh_json.int_ char_start);
        ("char_end", Hh_json.int_ char_end);
      ])

let half_open_one_based_json_to_pos_exn (json : Hh_json.json) :
    half_open_one_based =
  let open Hh_json_helpers in
  let json = Some json in
  {
    filename = Jget.string_exn json "filename";
    line = Jget.int_exn json "line";
    char_start = Jget.int_exn json "char_start";
    char_end = Jget.int_exn json "char_end";
  }

(** Produced by "hh --ide-find-refs-by-symbol" and parsed by clientLsp *)
module IdeShellout = struct
  let to_string (results : (string * Pos.absolute) list) : string =
    let entries =
      List.map results ~f:(pos_to_one_based_json ~half_open_interval:true)
    in
    Hh_json.JSON_Array entries |> Hh_json.json_to_string

  let from_string_exn (s : string) : half_open_one_based list =
    let json = Hh_json.json_of_string s in
    let entries = Hh_json.get_array_exn json in
    List.map entries ~f:half_open_one_based_json_to_pos_exn
end

(** Used by hh_server's findRefsService to write to a streaming file, read by clientLsp *)
module Ide_stream = struct
  let lock_and_append
      (fd : Unix.file_descr) (results : (string * Pos.absolute) list) : unit =
    if List.is_empty results then
      ()
    else begin
      let bytes =
        List.map results ~f:(fun (name, pos) ->
            Printf.sprintf
              "%s\n"
              (pos_to_one_based_json
                 (name, pos)
                 ~half_open_interval:true
                 ~timestamp:(Unix.gettimeofday ())
              |> Hh_json.json_to_string))
        |> String.concat ~sep:""
        |> Bytes.of_string
      in
      Sys_utils.with_lock fd Unix.F_LOCK ~f:(fun () ->
          let (_ : int) = Unix.lseek fd 0 Unix.SEEK_END in
          Sys_utils.write_non_intr fd bytes 0 (Bytes.length bytes);
          Unix.fsync fd)
    end

  let read_from_locked_file (fd : Unix.file_descr) ~(pos : int) :
      half_open_one_based list * int =
    let end_pos = Unix.lseek fd 0 Unix.SEEK_END in
    if pos = end_pos then
      ([], pos)
    else
      let (_ : int) = Unix.lseek fd pos Unix.SEEK_SET in
      let bytes =
        Sys_utils.read_non_intr fd (end_pos - pos) |> Option.value_exn
      in
      let jsons =
        Bytes.to_string bytes
        |> String_utils.split_on_newlines
        |> List.map ~f:Hh_json.json_of_string
      in
      (List.map jsons ~f:half_open_one_based_json_to_pos_exn, end_pos)
end

(** Used "hh --find-refs --json" and read by HackAst and other tools *)
module HackAst = struct
  let to_string results =
    let entries =
      List.map results ~f:(pos_to_one_based_json ~half_open_interval:false)
    in
    Hh_json.JSON_Array entries |> Hh_json.json_to_string
end

(** Used by "hh --find-refs" *)
module CliHumanReadable = struct
  let print_results results =
    List.iter results ~f:(fun (name, pos) ->
        Printf.printf "%s %s\n" (Pos.string pos) name);
    Printf.printf "%d total results\n" (List.length results)
end

(** CliArgs is produced by clientLsp when it invokes "hh --ide-find-refs-by-symbol <args>"
and consumed by clientArgs when it parses that argument. *)
module CliArgs = struct
  open SearchTypes.Find_refs

  type t = {
    symbol_name: string;
    action: SearchTypes.Find_refs.action;
    stream_file: Path.t option;
    hint_suffixes: string list;
  }

  (**
   * Output strings in the form of
   * SYMBOL_NAME|COMMA_SEPARATED_ACTION_STRING
   * For example,
   * HackTypecheckerQueryBase::getWWWDir|Member,\HackTypecheckerQueryBase,Method,getWWWDir
   * Must be manually kept in sync with string_to_symbol_and_action_exn.
  *)
  let to_string { symbol_name; action; _ } : string =
    let action_serialized =
      match action with
      | Class str -> Printf.sprintf "Class,%s" str
      | ExplicitClass str -> Printf.sprintf "ExplicitClass,%s" str
      | Function str -> Printf.sprintf "Function,%s" str
      | GConst str -> Printf.sprintf "GConst,%s" str
      | Member (str, Method s) ->
        Printf.sprintf "Member,%s,%s,%s" str "Method" s
      | Member (str, Property s) ->
        Printf.sprintf "Member,%s,%s,%s" str "Property" s
      | Member (str, Class_const s) ->
        Printf.sprintf "Member,%s,%s,%s" str "Class_const" s
      | Member (str, Typeconst s) ->
        Printf.sprintf "Member,%s,%s,%s" str "Typeconst" s
      | LocalVar _ ->
        Printf.eprintf "Invalid action\n";
        raise Exit_status.(Exit_with Input_error)
    in
    Printf.sprintf "%s|%s" symbol_name action_serialized

  (**
   * Expects input strings in the form of
   * SYMBOL_NAME|COMMA_SEPARATED_ACTION_STRING
   * For example,
   * HackTypecheckerQueryBase::getWWWDir|Member,\HackTypecheckerQueryBase,Method,getWWWDir
   * The implementation explicitly is tied to whatever is generated from symbol_and_action_to_string_exn
   * and should be kept in sync manually by anybody editing.
  *)
  let from_string_exn (string1 : string) : t =
    let pair = Str.split (Str.regexp "|") string1 in
    let (symbol_name, action_arg) =
      match pair with
      | [symbol_name; action_arg] -> (symbol_name, action_arg)
      | _ ->
        Printf.eprintf "Invalid input\n";
        raise Exit_status.(Exit_with Input_error)
    in
    let action =
      (* Explicitly ignoring localvar support *)
      match Str.split (Str.regexp ",") action_arg with
      | ["Class"; str] -> Class str
      | ["ExplicitClass"; str] -> ExplicitClass str
      | ["Function"; str] -> Function str
      | ["GConst"; str] -> GConst str
      | ["Member"; str; "Method"; member_name] ->
        Member (str, Method member_name)
      | ["Member"; str; "Property"; member_name] ->
        Member (str, Property member_name)
      | ["Member"; str; "Class_const"; member_name] ->
        Member (str, Class_const member_name)
      | ["Member"; str; "Typeconst"; member_name] ->
        Member (str, Typeconst member_name)
      | _ ->
        Printf.eprintf "Invalid input for action, got %s\n" action_arg;
        raise Exit_status.(Exit_with Input_error)
    in
    { symbol_name; action; stream_file = None; hint_suffixes = [] }

  let to_string_triple { symbol_name; action; stream_file; hint_suffixes } :
      string * string * string =
    let action =
      to_string { symbol_name; action; stream_file; hint_suffixes }
    in
    let stream_file =
      Option.value_map stream_file ~default:"-" ~f:Path.to_string
    in
    let hint_suffixes = hint_suffixes |> String.concat ~sep:"|" in
    (action, stream_file, hint_suffixes)

  let from_string_triple_exn
      ((action, stream_file, hints) : string * string * string) : t =
    let { symbol_name; action; _ } = from_string_exn action in
    let stream_file =
      if String.equal stream_file "-" then
        None
      else
        Some (Path.make stream_file)
    in
    let hint_suffixes = String.split_on_chars hints ~on:['|'] in
    { symbol_name; action; stream_file; hint_suffixes }
end
