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

let pos_to_one_based_json
    ?(timestamp : float option)
    ~(half_open_interval : bool)
    ((name, pos) : string * Pos.absolute) : Hh_json.json =
  let (st_line, st_column, _ed_line, ed_column) =
    if half_open_interval then
      Pos.destruct_range pos
    else
      let (line, start, end_) = Pos.info_pos pos in
      (line, start, line, end_)
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
        ("filename", Hh_json.JSON_String (Pos.filename pos));
        ("line", Hh_json.int_ st_line);
        ("char_start", Hh_json.int_ st_column);
        ("char_end", Hh_json.int_ ed_column);
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
  }

  (**
   * Output strings in the form of
   * SYMBOL_NAME|COMMA_SEPARATED_ACTION_STRING
   * For example,
   * HackTypecheckerQueryBase::getWWWDir|Member,\HackTypecheckerQueryBase,Method,getWWWDir
   * Must be manually kept in sync with string_to_symbol_and_action_exn.
  *)
  let to_string { symbol_name; action } : string =
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
    { symbol_name; action }
end
