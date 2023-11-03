(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type ('prim_pos, 'pos) t = {
  code: int;
  claim: 'prim_pos Message.t;
  reasons: 'pos Message.t list;
  quickfixes: 'prim_pos Quickfix.t list; [@hash.ignore]
  custom_msgs: string list;
  is_fixmed: bool;
}
[@@deriving eq, hash, ord, show]

let make
    code
    ?(is_fixmed = false)
    ?(quickfixes = [])
    ?(custom_msgs = [])
    claim
    reasons =
  { code; claim; reasons; quickfixes; custom_msgs; is_fixmed }

let get_code { code; _ } = code

let get_pos { claim; _ } = fst claim

let quickfixes { quickfixes; _ } = quickfixes

let to_list { claim; reasons; _ } = claim :: reasons

let to_list_ { claim = (pos, claim); reasons; _ } =
  (Pos_or_decl.of_raw_pos pos, claim) :: reasons

let get_messages = to_list

let to_absolute { code; claim; reasons; quickfixes; custom_msgs; is_fixmed } =
  let claim = (fst claim |> Pos.to_absolute, snd claim) in
  let reasons =
    List.map reasons ~f:(fun (p, s) ->
        (p |> Pos_or_decl.unsafe_to_raw_pos |> Pos.to_absolute, s))
  in
  let quickfixes = List.map quickfixes ~f:Quickfix.to_absolute in
  { code; claim; reasons; quickfixes; custom_msgs; is_fixmed }

let make_absolute code = function
  | [] -> failwith "an error must have at least one message"
  | claim :: reasons ->
    {
      code;
      claim;
      reasons;
      quickfixes = [];
      custom_msgs = [];
      is_fixmed = false;
    }

let to_absolute_for_test
    {
      code;
      claim = (claim_pos, claim_msg);
      reasons;
      quickfixes;
      custom_msgs;
      is_fixmed;
    } =
  let f (p, s) =
    let p = Pos_or_decl.unsafe_to_raw_pos p in
    let path = Pos.filename p in
    let path_without_prefix = Relative_path.suffix path in
    let p =
      Pos.set_file
        (Relative_path.create Relative_path.Dummy path_without_prefix)
        p
    in
    (Pos.to_absolute p, s)
  in
  let claim = f (Pos_or_decl.of_raw_pos claim_pos, claim_msg) in
  let reasons = List.map ~f reasons in
  let quickfixes = List.map quickfixes ~f:Quickfix.to_absolute in
  { code; claim; reasons; quickfixes; custom_msgs; is_fixmed }

let error_kind error_code =
  match error_code / 1000 with
  | 1 -> "Parsing"
  | 2 -> "Naming"
  | 3 -> "NastCheck"
  | 4 -> "Typing"
  | 5 -> "Lint"
  | 8 -> "Init"
  | _ -> "Other"

let error_code_to_string error_code =
  let error_kind = error_kind error_code in
  let error_number = Printf.sprintf "%04d" error_code in
  error_kind ^ "[" ^ error_number ^ "]"

let to_string
    report_pos_from_reason
    { code; claim; reasons; custom_msgs; quickfixes = _; is_fixmed = _ } =
  let buf = Buffer.create 50 in
  let (pos1, msg1) = claim in
  Buffer.add_string
    buf
    begin
      let error_code = error_code_to_string code in
      let reason_msg =
        if report_pos_from_reason && Pos.get_from_reason pos1 then
          " [FROM REASON INFO]"
        else
          ""
      in
      Printf.sprintf
        "%s\n%s (%s)%s\n"
        (Pos.string pos1)
        msg1
        error_code
        reason_msg
    end;
  List.iter reasons ~f:(fun (p, w) ->
      let msg = Printf.sprintf "  %s\n  %s\n" (Pos.string p) w in
      Buffer.add_string buf msg);
  List.iter custom_msgs ~f:(fun w ->
      let msg = Printf.sprintf "  %s\n" w in
      Buffer.add_string buf msg);
  Buffer.contents buf

let to_json error =
  let (error_code, msgl) = (get_code error, to_list error) in
  let elts =
    List.map msgl ~f:(fun (p, w) ->
        let (line, scol, ecol) = Pos.info_pos p in
        Hh_json.JSON_Object
          [
            ("descr", Hh_json.JSON_String w);
            ("path", Hh_json.JSON_String (Pos.filename p));
            ("line", Hh_json.int_ line);
            ("start", Hh_json.int_ scol);
            ("end", Hh_json.int_ ecol);
            ("code", Hh_json.int_ error_code);
          ])
  in
  let custom_msgs =
    List.map ~f:(fun msg -> Hh_json.JSON_String msg) error.custom_msgs
  in
  Hh_json.JSON_Object
    [
      ("message", Hh_json.JSON_Array elts);
      ("custom_messages", Hh_json.JSON_Array custom_msgs);
    ]
