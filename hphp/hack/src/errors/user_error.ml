(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

(* The order matters here, as it will determine which errors/warnings will
   be output first. *)
type severity =
  | Warning
  | Err
[@@deriving eq, hash, show, ord]

module Severity = struct
  let to_string = function
    | Err -> "error"
    | Warning -> "warning"

  let to_all_caps_string = function
    | Err -> "ERROR"
    | Warning -> "WARN"

  let tty_color = function
    | Err -> Tty.Red
    | Warning -> Tty.Yellow
end

type ('prim_pos, 'pos) t = {
  severity: severity;
  code: int;
  claim: 'prim_pos Message.t;
  reasons: 'pos Message.t list;
  explanation: 'pos Explanation.t;
      [@hash.ignore] [@equal (fun _ _ -> true)] [@compare (fun _ _ -> 0)]
  quickfixes: 'prim_pos Quickfix.t list; [@hash.ignore]
  custom_msgs: string list;
  is_fixmed: bool;
  flags: User_error_flags.t;
}
[@@deriving eq, hash, ord, show]

let make
    severity
    code
    ?(is_fixmed = false)
    ?(quickfixes = [])
    ?(custom_msgs = [])
    ?(flags = User_error_flags.empty)
    claim
    reasons
    explanation =
  {
    severity;
    code;
    claim;
    reasons;
    explanation;
    quickfixes;
    custom_msgs;
    is_fixmed;
    flags;
  }

let make_err
    code ?is_fixmed ?quickfixes ?custom_msgs ?flags claim reasons explanation =
  make
    Err
    code
    ?is_fixmed
    ?quickfixes
    ?custom_msgs
    ?flags
    claim
    reasons
    explanation

let make_warning code ?is_fixmed ?quickfixes ?custom_msgs ?flags claim reasons =
  make
    Warning
    code
    ?is_fixmed
    ?quickfixes
    ?custom_msgs
    ?flags
    claim
    reasons
    Explanation.empty

let get_code { code; _ } = code

let get_pos { claim; _ } = fst claim

let quickfixes { quickfixes; _ } = quickfixes

let to_list { claim; reasons; _ } = claim :: reasons

let to_list_ { claim = (pos, claim); reasons; _ } =
  (Pos_or_decl.of_raw_pos pos, claim) :: reasons

let get_messages { claim; reasons; _ } = claim :: reasons

let to_absolute
    {
      severity;
      code;
      claim;
      reasons;
      explanation;
      quickfixes;
      custom_msgs;
      is_fixmed;
      flags;
    } =
  let pos_or_decl_to_absolute_pos pos_or_decl =
    let pos = Pos_or_decl.unsafe_to_raw_pos pos_or_decl in
    Pos.to_absolute pos
  in
  let claim = (fst claim |> Pos.to_absolute, snd claim) in
  let reasons =
    List.map reasons ~f:(fun (p, s) -> (pos_or_decl_to_absolute_pos p, s))
  in
  let explanation =
    Explanation.map explanation ~f:pos_or_decl_to_absolute_pos
  in

  let quickfixes = List.map quickfixes ~f:Quickfix.to_absolute in
  {
    severity;
    code;
    claim;
    reasons;
    explanation;
    quickfixes;
    custom_msgs;
    is_fixmed;
    flags;
  }

let to_relative
    {
      severity;
      code;
      claim;
      reasons;
      explanation;
      quickfixes;
      custom_msgs;
      is_fixmed;
      flags;
    } : (Pos.t, Pos.t) t =
  let claim = (fst claim, snd claim) in
  let reasons =
    List.map reasons ~f:(fun (p, s) -> (p |> Pos_or_decl.unsafe_to_raw_pos, s))
  in
  let explanation =
    Explanation.map explanation ~f:(fun pos ->
        Pos_or_decl.unsafe_to_raw_pos pos)
  in
  {
    severity;
    code;
    claim;
    reasons;
    explanation;
    quickfixes;
    custom_msgs;
    is_fixmed;
    flags;
  }

let make_absolute severity code = function
  | [] -> failwith "an error must have at least one message"
  | claim :: reasons ->
    {
      severity;
      code;
      claim;
      reasons;
      explanation = Explanation.empty;
      quickfixes = [];
      custom_msgs = [];
      is_fixmed = false;
      flags = User_error_flags.empty;
    }

let to_absolute_for_test
    {
      severity;
      code;
      claim = (claim_pos, claim_msg);
      reasons;
      explanation;
      quickfixes;
      custom_msgs;
      is_fixmed;
      flags;
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

  let explanation =
    Explanation.map explanation ~f:(fun pos ->
        Pos.to_absolute @@ Pos_or_decl.unsafe_to_raw_pos pos)
  in
  let quickfixes = List.map quickfixes ~f:Quickfix.to_absolute in
  {
    severity;
    code;
    claim;
    reasons;
    explanation;
    quickfixes;
    custom_msgs;
    is_fixmed;
    flags;
  }

let error_kind error_code =
  match error_code / 1000 with
  | 1 -> "Parsing"
  | 2 -> "Naming"
  | 3 -> "NastCheck"
  | 4 -> "Typing"
  | 5 -> "Lint"
  | 8 -> "Init"
  | 12 -> "Warn"
  | _ -> "Other"

let error_code_to_string error_code =
  let error_kind = error_kind error_code in
  let error_number = Printf.sprintf "%04d" error_code in
  error_kind ^ "[" ^ error_number ^ "]"

(** The string looks like:

    ERROR/WARN: path/to/file.php line ...
    You have a problem here (Typing4110)
      path/to/other/file.php ...
      Because this is wrong
      path/to/other/file.php ...
      And this too
  *)
let to_string
    report_pos_from_reason
    {
      severity;
      code;
      claim;
      reasons;
      explanation = _;
      custom_msgs;
      quickfixes = _;
      is_fixmed = _;
      flags = _;
    } =
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
        (* /!\ WARNING!!!!!
           Changing this might break emacs and vim.
           These are not officially supported, but breaking will piss off users.
           Fix Emacs at fbcode/emacs_config/emacs-packages/compile-mode-regexes.el
           Fix Vim at fbcode/shellconfigs/rc_deprecated/vim/plugin/hack.vim, variable hack_errorformat *)
        "%s: %s\n%s (%s)%s\n"
        (Severity.to_all_caps_string severity)
        (Pos.string pos1)
        msg1
        error_code
        reason_msg
    end;
  let (_ : unit) =
    List.iter reasons ~f:(fun (p, w) ->
        let msg = Printf.sprintf "  %s\n  %s\n" (Pos.string p) w in
        Buffer.add_string buf msg)
  in
  let (_ : unit) =
    List.iter custom_msgs ~f:(fun w ->
        let msg = Printf.sprintf "  %s\n" w in
        Buffer.add_string buf msg)
  in
  Buffer.contents buf

let to_json ~(human_formatter : (_ -> string) option) ~filename_to_string error
    =
  let {
    severity;
    code;
    claim;
    reasons;
    explanation = _;
    custom_msgs;
    quickfixes = _;
    is_fixmed = _;
    flags;
  } =
    error
  in
  let msgl = claim :: reasons in
  let elts =
    List.map msgl ~f:(fun (p, msg) ->
        let (line, scol, ecol) = Pos.info_pos p in
        Hh_json.JSON_Object
          [
            ("descr", Hh_json.JSON_String msg);
            ("path", Hh_json.JSON_String (Pos.filename p |> filename_to_string));
            ("line", Hh_json.int_ line);
            ("start", Hh_json.int_ scol);
            ("end", Hh_json.int_ ecol);
            ("code", Hh_json.int_ code);
          ])
  in
  let custom_msgs =
    List.map ~f:(fun msg -> Hh_json.JSON_String msg) custom_msgs
  in
  let flags = User_error_flags.to_json flags in
  let human_format = Option.map ~f:(fun f -> f error) human_formatter in
  let obj =
    [
      ("severity", Hh_json.JSON_String (Severity.to_string severity));
      ("message", Hh_json.JSON_Array elts);
      ("custom_messages", Hh_json.JSON_Array custom_msgs);
      ("flags", flags);
    ]
  in
  let obj =
    match human_format with
    | None -> obj
    | Some human_format ->
      obj @ [("human_format", Hh_json.JSON_String human_format)]
  in
  Hh_json.JSON_Object obj
