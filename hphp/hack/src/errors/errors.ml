(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Reordered_argument_collections
open String_utils

type error_code = int [@@deriving eq]

(** We use `Pos.t message` on the server and convert to `Pos.absolute message`
 * before sending it to the client *)
type 'a message = 'a * string [@@deriving eq]

type phase =
  | Init
  | Parsing
  | Naming
  | Decl
  | Typing

type severity =
  | Warning
  | Error

type format =
  | Context
  | Raw

type typing_error_callback = ?code:int -> (Pos.t * string) list -> unit

type name_context =
  | FunctionNamespace
  | ConstantNamespace
  | TypeNamespace  (** Classes, interfaces, traits, records and type aliases.*)
  (* The following are all subsets of TypeNamespace, used when we can
     give a more specific naming error. E.g. `use Foo;` only allows
     traits. *)
  | TraitContext
  | ClassContext
  | RecordContext

(* The file and phase of analysis being currently performed *)
let current_context : (Relative_path.t * phase) ref =
  ref (Relative_path.default, Typing)

let allow_errors_in_default_path = ref true

module PhaseMap = Reordered_argument_map (WrappedMap.Make (struct
  type t = phase

  let rank = function
    | Init -> 0
    | Parsing -> 1
    | Naming -> 2
    | Decl -> 3
    | Typing -> 4

  let compare x y = rank x - rank y
end))

(** Results of single file analysis. *)
type 'a file_t = 'a list PhaseMap.t [@@deriving eq]

(** Results of multi-file analysis. *)
type 'a files_t = 'a file_t Relative_path.Map.t [@@deriving eq]

let files_t_fold v ~f ~init =
  Relative_path.Map.fold v ~init ~f:(fun path v acc ->
      PhaseMap.fold v ~init:acc ~f:(fun phase v acc -> f path phase v acc))

let files_t_map v ~f = Relative_path.Map.map v ~f:(fun v -> PhaseMap.map v ~f)

let files_t_merge ~f x y =
  (* Using fold instead of merge to make the runtime proportional to the size
   * of first argument (like List.rev_append ) *)
  Relative_path.Map.fold x ~init:y ~f:(fun k x acc ->
      let y =
        Option.value (Relative_path.Map.find_opt y k) ~default:PhaseMap.empty
      in
      Relative_path.Map.add
        acc
        k
        (PhaseMap.merge x y ~f:(fun phase x y -> f phase k x y)))

let files_t_to_list x =
  files_t_fold x ~f:(fun _ _ x acc -> List.rev_append x acc) ~init:[]
  |> List.rev

let list_to_files_t = function
  | [] -> Relative_path.Map.empty
  | x ->
    (* Values constructed here should not be used with incremental mode.
     * See assert in incremental_update. *)
    Relative_path.Map.singleton
      Relative_path.default
      (PhaseMap.singleton Typing x)

let get_code_severity code =
  if
    code
    = Error_codes.Init.err_code Error_codes.Init.ForwardCompatibilityNotCurrent
  then
    Warning
  else
    Error

(* Get most recently-ish added error. *)
let get_last error_map =
  (* If this map has more than one element, we pick an arbitrary file. Because
   * of that, we might not end up with the most recent error and generate a
   * less-specific error message. This should be rare. *)
  match Relative_path.Map.max_binding_opt error_map with
  | None -> None
  | Some (_, phase_map) ->
    let error_list =
      PhaseMap.max_binding_opt phase_map |> Option.value_map ~f:snd ~default:[]
    in
    (match List.rev error_list with
    | [] -> None
    | e :: _ -> Some e)

type 'a error_ = error_code * 'a message list [@@deriving eq]

type error = Pos.t error_ [@@deriving eq]

type applied_fixme = Pos.t * int [@@deriving eq]

let applied_fixmes : applied_fixme files_t ref = ref Relative_path.Map.empty

let (error_map : error files_t ref) = ref Relative_path.Map.empty

let accumulate_errors = ref false

(* Some filename when declaring *)
let in_lazy_decl = ref None

let (is_hh_fixme : (Pos.t -> error_code -> bool) ref) = ref (fun _ _ -> false)

let badpos_message =
  Printf.sprintf
    "Type checking of this file produced an error with the primary position in a different file. %s"
    Error_message_sentinel.please_file_a_bug_message

let try_with_result f1 f2 =
  let error_map_copy = !error_map in
  let accumulate_errors_copy = !accumulate_errors in
  let is_hh_fixme_copy = !is_hh_fixme in
  (is_hh_fixme := (fun _ _ -> false));
  error_map := Relative_path.Map.empty;
  accumulate_errors := true;
  let (result, errors) =
    Utils.try_finally
      ~f:
        begin
          fun () ->
          let result = f1 () in
          (result, !error_map)
        end
      ~finally:
        begin
          fun () ->
          error_map := error_map_copy;
          accumulate_errors := accumulate_errors_copy;
          is_hh_fixme := is_hh_fixme_copy
        end
  in
  match get_last errors with
  | None -> result
  | Some (code, l) ->
    (* Remove bad position sentinel if present: we might be about to add a new primary
     * error position*)
    let l =
      match l with
      | (_, msg) :: l when msg = badpos_message -> l
      | _ -> l
    in
    f2 result (code, l)

let do_ f =
  let error_map_copy = !error_map in
  let accumulate_errors_copy = !accumulate_errors in
  let applied_fixmes_copy = !applied_fixmes in
  error_map := Relative_path.Map.empty;
  applied_fixmes := Relative_path.Map.empty;
  accumulate_errors := true;
  let (result, out_errors, out_applied_fixmes) =
    Utils.try_finally
      ~f:
        begin
          fun () ->
          let result = f () in
          (result, !error_map, !applied_fixmes)
        end
      ~finally:
        begin
          fun () ->
          error_map := error_map_copy;
          applied_fixmes := applied_fixmes_copy;
          accumulate_errors := accumulate_errors_copy
        end
  in
  let out_errors = files_t_map ~f:List.rev out_errors in
  ((out_errors, out_applied_fixmes), result)

let run_in_context path phase f =
  let context_copy = !current_context in
  current_context := (path, phase);
  Utils.try_finally ~f ~finally:(fun () -> current_context := context_copy)

(* Log important data if lazy_decl triggers a crash *)
let lazy_decl_error_logging error error_map to_absolute to_string =
  let error_list = files_t_to_list !error_map in
  (* Print the current error list, which should be empty *)
  Printf.eprintf "%s" "Error list(should be empty):\n";
  List.iter error_list ~f:(fun err ->
      let msg = err |> to_absolute |> to_string in
      Printf.eprintf "%s\n" msg);
  Printf.eprintf "%s" "Offending error:\n";
  Printf.eprintf "%s" error;

  (* Print out a larger stack trace *)
  Printf.eprintf "%s" "Callstack:\n";
  Printf.eprintf
    "%s"
    (Caml.Printexc.raw_backtrace_to_string (Caml.Printexc.get_callstack 500));

  (* Exit with special error code so we can see the log after *)
  Exit_status.exit Exit_status.Lazy_decl_bug

(*****************************************************************************)
(* Error code printing. *)
(*****************************************************************************)

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

let phase_to_string (phase : phase) : string =
  match phase with
  | Init -> "Init"
  | Parsing -> "Parsing"
  | Naming -> "Naming"
  | Decl -> "Decl"
  | Typing -> "Typing"

let get_message (error : error) =
  (* We get the position of the first item in the message error list, which
    represents the location of the error (error claim). Other messages represent
    the reason for the error (the warrant and the grounds). *)
  let message_list = snd error in
  let first_message = List.hd_exn message_list in
  first_message

let get_pos (error : error) = fst (get_message error)

let sort err =
  let rec compare (x_code, x_messages) (y_code, y_messages) =
    match (x_messages, y_messages) with
    | ([], []) -> 0
    | (_x_messages, []) -> -1
    | ([], _y_messages) -> 1
    | (x_message :: x_messages, y_message :: y_messages) ->
      (* The primary sort order is position *)
      let comparison = Pos.compare (fst x_message) (fst y_message) in
      (* If the positions are the same, sort by error code *)
      let comparison =
        if comparison = 0 then
          Int.compare x_code y_code
        else
          comparison
      in
      (* If the error codes are also the same, sort by message text *)
      let comparison =
        if comparison = 0 then
          String.compare (snd x_message) (snd y_message)
        else
          comparison
      in
      (* Finally, if the message text is also the same, then continue comparing
      the rest of the messages (which indicate the reason why Hack believes
      there is an error reported in the 1st message) *)
      if comparison = 0 then
        compare (x_code, x_messages) (y_code, y_messages)
      else
        comparison
  in
  List.sort ~compare err |> List.remove_consecutive_duplicates ~equal:( = )

let get_sorted_error_list (err, _) = sort (files_t_to_list err)

(* Getters and setter for passed-in map, based on current context *)
let get_current_file_t file_t_map =
  let current_file = fst !current_context in
  Relative_path.Map.find_opt file_t_map current_file
  |> Option.value ~default:PhaseMap.empty

let get_current_list file_t_map =
  let current_phase = snd !current_context in
  get_current_file_t file_t_map |> fun x ->
  PhaseMap.find_opt x current_phase |> Option.value ~default:[]

let set_current_list file_t_map new_list =
  let (current_file, current_phase) = !current_context in
  file_t_map :=
    Relative_path.Map.add
      !file_t_map
      current_file
      (PhaseMap.add (get_current_file_t !file_t_map) current_phase new_list)

let do_with_context path phase f = run_in_context path phase (fun () -> do_ f)

(* Turn on lazy decl mode for the duration of the closure.
   This runs without returning the original state,
   since we collect it later in do_with_lazy_decls_
*)
let run_in_decl_mode filename f =
  let old_in_lazy_decl = !in_lazy_decl in
  in_lazy_decl := Some filename;
  Utils.try_finally ~f ~finally:(fun () -> in_lazy_decl := old_in_lazy_decl)

and make_error code (x : (Pos.t * string) list) : error = (code, x)

(*****************************************************************************)
(* Accessors. *)
(*****************************************************************************)
and get_code (error : 'a error_) = (fst error : error_code)

let get_severity (error : 'a error_) = get_code_severity (get_code error)

let to_list (error : 'a error_) = snd error

let to_absolute error =
  let (code, msg_l) = (get_code error, to_list error) in
  let msg_l = List.map msg_l (fun (p, s) -> (Pos.to_absolute p, s)) in
  (code, msg_l)

let read_lines path = In_channel.read_lines path

let line_margin (line_num : int option) col_width : string =
  let padded_num =
    match line_num with
    | Some line_num -> Printf.sprintf "%*d" col_width line_num
    | None -> String.make col_width ' '
  in
  Tty.apply_color (Tty.Normal Tty.Cyan) (padded_num ^ " |")

(* Get the lines of source code associated with this position. *)
let load_context_lines (pos : Pos.absolute) : string list =
  let path = Pos.filename pos in
  let line = Pos.line pos in
  let end_line = Pos.end_line pos in
  let lines = (try read_lines path with Sys_error _ -> []) in
  (* Line numbers are 1-indexed. *)
  List.filteri lines ~f:(fun i _ -> i + 1 >= line && i + 1 <= end_line)

let format_context_lines (pos : Pos.absolute) (lines : string list) col_width :
    string =
  let lines =
    match lines with
    | [] -> [Tty.apply_color (Tty.Dim Tty.Default) "No source found"]
    | ls -> ls
  in
  let line_num = Pos.line pos in
  let format_line i (line : string) =
    Printf.sprintf "%s %s" (line_margin (Some (line_num + i)) col_width) line
  in
  let formatted_lines = List.mapi ~f:format_line lines in
  (* TODO: display all the lines, showing the underline on all of them. *)
  List.hd_exn formatted_lines

(* Format this message as "  ^^^ You did something wrong here". *)
let format_substring_underline
    (pos : Pos.absolute)
    (msg : string)
    (first_context_line : string option)
    is_first
    col_width : string =
  let (start_line, start_column) = Pos.line_column pos in
  let (end_line, end_column) = Pos.end_line_column pos in
  let underline_width =
    match first_context_line with
    | None -> 4 (* Arbitrary choice when source isn't available. *)
    | Some first_context_line ->
      if start_line = end_line then
        end_column - start_column
      else
        String.length first_context_line - start_column
  in
  let underline = String.make (max underline_width 1) '^' in
  let underline_padding =
    if Option.is_some first_context_line then
      String.make start_column ' '
    else
      ""
  in
  let color =
    if is_first then
      Tty.Bold Tty.Red
    else
      Tty.Dim Tty.Default
  in
  Printf.sprintf
    "%s %s%s"
    (line_margin None col_width)
    underline_padding
    (Tty.apply_color
       color
       ( if is_first then
         underline
       else
         underline ^ " " ^ msg ))

let format_filename (pos : Pos.absolute) : string =
  let relative_path path =
    let cwd = Filename.concat (Sys.getcwd ()) "" in
    lstrip path cwd
  in
  let filename = relative_path (Pos.filename pos) in
  Printf.sprintf
    "   %s %s"
    (Tty.apply_color (Tty.Normal Tty.Cyan) "-->")
    (Tty.apply_color (Tty.Normal Tty.Green) filename)

let column_width line_number =
  let num_digits x = int_of_float (Float.log10 (float_of_int x)) + 1 in
  max 3 (num_digits line_number)

(* Format the line of code associated with this message, and the message itself. *)
let format_message (msg : string) (pos : Pos.absolute) ~is_first ~col_width :
    string * string =
  let col_width =
    Option.value col_width ~default:(column_width (Pos.line pos))
  in
  let context_lines = load_context_lines pos in
  let pretty_ctx = format_context_lines pos context_lines col_width in
  let pretty_msg =
    format_substring_underline
      pos
      msg
      (List.hd context_lines)
      is_first
      col_width
  in
  (pretty_ctx, pretty_msg)

(** Sort messages such that messages in the same file are together.
    Do not reorder the files or messages within a file.
 *)
let group_by_file (msgs : Pos.absolute message list) : Pos.absolute message list
    =
  let rec build_map msgs grouped filenames =
    match msgs with
    | msg :: msgs ->
      let filename = Pos.filename (fst msg) in
      (match String.Map.find grouped filename with
      | Some file_msgs ->
        let grouped =
          String.Map.set grouped ~key:filename ~data:(file_msgs @ [msg])
        in
        build_map msgs grouped filenames
      | None ->
        let grouped = String.Map.set grouped ~key:filename ~data:[msg] in
        build_map msgs grouped (filename :: filenames))
    | [] -> (grouped, filenames)
  in
  let (grouped, filenames) = build_map msgs String.Map.empty [] in
  List.concat_map (List.rev filenames) ~f:(fun fn ->
      String.Map.find_exn grouped fn)

(* Work out the column width needed for each file. Files with many
   lines need a wider column due to the higher line numbers. *)
let col_widths (msgs : Pos.absolute message list) : int Core_kernel.String.Map.t
    =
  (* Find the longest line number for every file in msgs. *)
  let longest_lines =
    List.fold msgs ~init:String.Map.empty ~f:(fun acc msg ->
        let filename = Pos.filename (fst msg) in
        let current_max =
          Option.value (String.Map.find acc filename) ~default:0
        in
        String.Map.set
          acc
          ~key:filename
          ~data:(max current_max (Pos.line (fst msg))))
  in
  String.Map.map longest_lines ~f:column_width

(** Given a list of error messages, format them with context.
    The list may not be ordered, and multiple messages may occur on one line.
 *)
let format_messages (msgs : Pos.absolute message list) : string =
  let msgs = group_by_file msgs in
  (* The first message is the 'primary' message, so add a boolean to distinguish it. *)
  let rec label_first msgs is_first =
    match msgs with
    | msg :: msgs -> (msg, is_first) :: label_first msgs false
    | [] -> []
  in
  let labelled_msgs = label_first msgs true in
  (* Sort messages by line number, so we can display with context. *)
  let cmp (m1, _) (m2, _) =
    match compare (Pos.filename (fst m1)) (Pos.filename (fst m2)) with
    | 0 -> compare (Pos.line (fst m1)) (Pos.line (fst m2))
    | _ -> 0
  in
  let sorted_msgs = List.stable_sort cmp labelled_msgs in
  (* For every message, show it alongside the relevant line. If there
     are multiple messages associated with the line, only show it once. *)
  let col_widths = col_widths msgs in
  let rec aux msgs prev : string list =
    match msgs with
    | (msg, is_first) :: msgs ->
      let (pos, err_msg) = msg in
      let filename = Pos.filename pos in
      let line = Pos.line pos in
      let col_width = String.Map.find col_widths filename in
      let (pretty_ctx, pretty_msg) =
        format_message err_msg pos ~is_first ~col_width
      in
      let formatted : string list =
        match prev with
        | Some (prev_filename, prev_line)
          when prev_filename = filename && prev_line = line ->
          (* Previous message was on this line too, just show the message itself*)
          [pretty_msg]
        | Some (prev_filename, _) when prev_filename = filename ->
          (* Previous message was this file, but an earlier line. *)
          [pretty_ctx; pretty_msg]
        | _ -> [format_filename pos; pretty_ctx; pretty_msg]
      in
      formatted @ aux msgs (Some (filename, line))
    | [] -> []
  in
  String.concat ~sep:"\n" (aux sorted_msgs None) ^ "\n"

(* E.g. "10 errors found." *)
let format_summary format errors dropped_count max_errors : string option =
  match format with
  | Context ->
    let total = List.length errors + dropped_count in
    let formatted_total =
      Printf.sprintf
        "%d error%s found"
        total
        ( if total = 1 then
          ""
        else
          "s" )
    in
    let truncated =
      match max_errors with
      | Some max_errors when dropped_count > 0 ->
        Printf.sprintf
          " (only showing first %d, dropped %d).\n"
          max_errors
          dropped_count
      | _ -> ".\n"
    in
    Some (formatted_total ^ truncated)
  | Raw -> None

let to_contextual_string (error : Pos.absolute error_) : string =
  let error_code = get_code error in
  let msgl = to_list error in
  let buf = Buffer.create 50 in
  (match msgl with
  | [] -> failwith "Impossible: an error always has non-empty list of messages"
  | (_, msg) :: _ ->
    Buffer.add_string
      buf
      (Printf.sprintf
         "%s %s\n"
         (Tty.apply_color (Tty.Bold Tty.Red) (error_code_to_string error_code))
         (Tty.apply_color (Tty.Bold Tty.Default) msg)));
  (try Buffer.add_string buf (format_messages msgl)
   with _ ->
     Buffer.add_string
       buf
       "Error could not be pretty-printed. Please file a bug.");
  Buffer.add_string buf "\n";
  Buffer.contents buf

let to_absolute_for_test error =
  let (code, msg_l) = (get_code error, to_list error) in
  let msg_l =
    List.map msg_l (fun (p, s) ->
        let path = Pos.filename p in
        let path_without_prefix = Relative_path.suffix path in
        let p =
          Pos.set_file
            (Relative_path.create Relative_path.Dummy path_without_prefix)
            p
        in
        (Pos.to_absolute p, s))
  in
  (code, msg_l)

let to_string ?(indent = false) (error : Pos.absolute error_) : string =
  let (error_code, msgl) = (get_code error, to_list error) in
  let buf = Buffer.create 50 in
  (match msgl with
  | [] -> assert false
  | (pos1, msg1) :: rest_of_error ->
    Buffer.add_string
      buf
      begin
        let error_code = error_code_to_string error_code in
        Printf.sprintf "%s\n%s (%s)\n" (Pos.string pos1) msg1 error_code
      end;
    let indentstr =
      if indent then
        "  "
      else
        ""
    in
    List.iter rest_of_error (fun (p, w) ->
        let msg =
          Printf.sprintf "%s%s\n%s%s\n" indentstr (Pos.string p) indentstr w
        in
        Buffer.add_string buf msg));
  Buffer.contents buf

let add_error_impl error =
  if !accumulate_errors then
    let () =
      match !current_context with
      | (path, _)
        when path = Relative_path.default && not !allow_errors_in_default_path
        ->
        Hh_logger.log
          "WARNING: adding an error in default path\n%s\n"
          (Caml.Printexc.raw_backtrace_to_string
             (Caml.Printexc.get_callstack 100))
      | _ -> ()
    in
    (* Cheap test to avoid duplicating most recent error *)
    let error_list = get_current_list !error_map in
    match error_list with
    | old_error :: _ when error = old_error -> ()
    | _ -> set_current_list error_map (error :: error_list)
  else
    (* We have an error, but haven't handled it in any way *)
    let msg = error |> to_absolute |> to_string in
    match !in_lazy_decl with
    | Some _ -> lazy_decl_error_logging msg error_map to_absolute to_string
    | None -> Utils.assert_false_log_backtrace (Some msg)

(* Whether we've found at least one error *)
let currently_has_errors () = get_current_list !error_map <> []

module Parsing = Error_codes.Parsing
module Naming = Error_codes.Naming
module NastCheck = Error_codes.NastCheck
module Typing = Error_codes.Typing

(*****************************************************************************)
(* Types *)
(*****************************************************************************)

type t = error files_t * applied_fixme files_t [@@deriving eq]

module type Error_category = sig
  type t

  val min : int

  val max : int

  val of_enum : int -> t option

  val show : t -> string

  val err_code : t -> int
end

(*****************************************************************************)
(* HH_FIXMEs hook *)
(*****************************************************************************)

let error_codes_treated_strictly = ref (ISet.of_list [])

let is_strict_code code = ISet.mem code !error_codes_treated_strictly

(* The 'phps FixmeAllHackErrors' tool must be kept in sync with this list *)
let default_ignored_fixme_codes =
  ISet.of_list
    [
      Typing.err_code Typing.InvalidIsAsExpressionHint;
      Typing.err_code Typing.InvalidEnforceableTypeArgument;
      Typing.err_code Typing.RequireArgsReify;
      Typing.err_code Typing.InvalidReifiedArgument;
      Typing.err_code Typing.GenericsNotAllowed;
      Typing.err_code Typing.InvalidNewableTypeArgument;
      Typing.err_code Typing.InvalidNewableTypeParamConstraints;
      Typing.err_code Typing.NewWithoutNewable;
      Typing.err_code Typing.NewStaticClassReified;
      Typing.err_code Typing.MemoizeReified;
      Typing.err_code Typing.ClassGetReified;
    ]

let ignored_fixme_codes = ref default_ignored_fixme_codes

let set_allow_errors_in_default_path x = allow_errors_in_default_path := x

let is_ignored_code code = ISet.mem code !ignored_fixme_codes || code / 1000 = 5

let is_ignored_fixme code = is_ignored_code code

let (get_hh_fixme_pos : (Pos.t -> error_code -> Pos.t option) ref) =
  ref (fun _ _ -> None)

let (is_hh_fixme_disallowed : (Pos.t -> error_code -> bool) ref) =
  ref (fun _ _ -> false)

let add_ignored_fixme_code_error pos code =
  if !is_hh_fixme_disallowed pos code then
    add_error_impl
      (make_error
         code
         [
           ( pos,
             Printf.sprintf
               "You cannot use HH_FIXME or HH_IGNORE_ERROR comments to suppress error %d in declarations"
               code );
         ])
  else if !is_hh_fixme pos code && is_ignored_code code then
    let pos = Option.value (!get_hh_fixme_pos pos code) ~default:pos in
    if code / 1000 = 5 then
      add_error_impl
        (make_error
           code
           [
             ( pos,
               Printf.sprintf
                 "You cannot use HH_FIXME or HH_IGNORE_ERROR comments to suppress error %d.Please use @lint-ignore."
                 code );
           ])
    else
      add_error_impl
        (make_error
           code
           [
             ( pos,
               Printf.sprintf
                 "You cannot use HH_FIXME or HH_IGNORE_ERROR comments to suppress error %d"
                 code );
           ])

(*****************************************************************************)
(* Errors accumulator. *)
(*****************************************************************************)

(* If primary position in error list isn't in current file, wrap with a sentinel error *)
let check_pos_msg pos_msg_l =
  let pos = fst (List.hd_exn pos_msg_l) in
  let current_file = fst !current_context in
  if current_file <> Relative_path.default && Pos.filename pos <> current_file
  then
    (Pos.make_from current_file, badpos_message) :: pos_msg_l
  else
    pos_msg_l

let rec add_applied_fixme code pos =
  if ServerLoadFlag.get_no_load () then
    let applied_fixmes_list = get_current_list !applied_fixmes in
    set_current_list applied_fixmes ((pos, code) :: applied_fixmes_list)
  else
    ()

and add code pos msg =
  let pos_msg_l = check_pos_msg [(pos, msg)] in
  if (not (is_ignored_fixme code)) && !is_hh_fixme pos code then
    add_applied_fixme code pos
  else
    add_error_impl (make_error code pos_msg_l);
  add_ignored_fixme_code_error pos code

and add_error_with_check (error : error) : unit =
  add_list (fst error) (snd error)

and add_list code pos_msg_l =
  let pos = fst (List.hd_exn pos_msg_l) in
  let pos_msg_l = check_pos_msg pos_msg_l in
  if (not (is_ignored_fixme code)) && !is_hh_fixme pos code then
    add_applied_fixme code pos
  else
    add_error_impl (make_error code pos_msg_l);
  add_ignored_fixme_code_error pos code

and add_error (code, pos_msg_l) = add_list code pos_msg_l

and merge (err', fixmes') (err, fixmes) =
  let append _ _ x y =
    let x = Option.value x ~default:[] in
    let y = Option.value y ~default:[] in
    Some (List.rev_append x y)
  in
  (files_t_merge ~f:append err' err, files_t_merge ~f:append fixmes' fixmes)

and merge_into_current errors =
  let merged = merge errors (!error_map, !applied_fixmes) in
  error_map := fst merged;
  applied_fixmes := snd merged

and incremental_update :
    type (* Need to write out the entire ugly type to convince OCaml it's polymorphic
          * and can update both error_map as well as applied_fixmes map *)
    a.
    a files_t ->
    a files_t ->
    ((* function folding over paths of rechecked files *)
     a files_t ->
    (Relative_path.t -> a files_t -> a files_t) ->
    a files_t) ->
    phase ->
    a files_t =
 fun old new_ fold phase ->
  (* Helper to remove acc[path][phase]. If acc[path] becomes empty afterwards,
   * remove it too (i.e do not store empty maps or lists ever). *)
  let remove path phase acc =
    let new_phase_map =
      match Relative_path.Map.find_opt acc path with
      | None -> None
      | Some phase_map ->
        let new_phase_map = PhaseMap.remove phase_map phase in
        if PhaseMap.is_empty new_phase_map then
          None
        else
          Some new_phase_map
    in
    match new_phase_map with
    | None -> Relative_path.Map.remove acc path
    | Some x -> Relative_path.Map.add acc path x
  in
  (* Replace old errors with new *)
  let res =
    files_t_merge new_ old ~f:(fun phase path new_ old ->
        ( if path = Relative_path.default then
          let phase =
            match phase with
            | Init -> "Init"
            | Parsing -> "Parsing"
            | Naming -> "Naming"
            | Decl -> "Decl"
            | Typing -> "Typing"
          in
          Utils.assert_false_log_backtrace
            (Some
               ( "Default (untracked) error sources should not get into incremental "
               ^ "mode. There might be a missing call to Errors.do_with_context/"
               ^ "run_in_context somwhere or incorrectly used Errors.from_error_list."
               ^ "Phase: "
               ^ phase )) );
        match new_ with
        | Some new_ -> Some (List.rev new_)
        | None -> old)
  in
  (* For files that were rechecked, but had no errors - remove them from maps *)
  fold res (fun path acc ->
      let has_errors =
        match Relative_path.Map.find_opt new_ path with
        | None -> false
        | Some phase_map -> PhaseMap.mem phase_map phase
      in
      if has_errors then
        acc
      else
        remove path phase acc)

and incremental_update_set ~old ~new_ ~rechecked phase =
  let fold init g =
    Relative_path.Set.fold
      ~f:
        begin
          fun path acc ->
          g path acc
        end
      ~init
      rechecked
  in
  ( incremental_update (fst old) (fst new_) fold phase,
    incremental_update (snd old) (snd new_) fold phase )

and incremental_update_map ~old ~new_ ~rechecked phase =
  let fold init g =
    Relative_path.Map.fold
      ~f:
        begin
          fun path _ acc ->
          g path acc
        end
      ~init
      rechecked
  in
  ( incremental_update (fst old) (fst new_) fold phase,
    incremental_update (snd old) (snd new_) fold phase )

and empty = (Relative_path.Map.empty, Relative_path.Map.empty)

and is_empty (err, _fixmes) = Relative_path.Map.is_empty err

and count (err, _fixmes) =
  files_t_fold err ~f:(fun _ _ x acc -> acc + List.length x) ~init:0

and get_error_list (err, _fixmes) = files_t_to_list err

and get_applied_fixmes (_err, fixmes) = files_t_to_list fixmes

and from_error_list err = (list_to_files_t err, Relative_path.Map.empty)

(*****************************************************************************)
(* Accessors. (All methods delegated to the parameterized module.) *)
(*****************************************************************************)

let iter_error_list f err = List.iter ~f (get_sorted_error_list err)

let fold_errors ?phase err ~init ~f =
  match phase with
  | None ->
    files_t_fold (fst err) ~init ~f:(fun source _ errors acc ->
        List.fold_right errors ~init:acc ~f:(f source))
  | Some phase ->
    Relative_path.Map.fold (fst err) ~init ~f:(fun source phases acc ->
        match PhaseMap.find_opt phases phase with
        | None -> acc
        | Some errors -> List.fold_right errors ~init:acc ~f:(f source))

let fold_errors_in ?phase err ~source ~init ~f =
  Relative_path.Map.find_opt (fst err) source
  |> Option.value ~default:PhaseMap.empty
  |> PhaseMap.fold ~init ~f:(fun p errors acc ->
         if phase <> None && phase <> Some p then
           acc
         else
           List.fold_right errors ~init:acc ~f)

let get_failed_files err phase =
  files_t_fold (fst err) ~init:Relative_path.Set.empty ~f:(fun source p _ acc ->
      if phase <> p then
        acc
      else
        Relative_path.Set.add acc source)

(*****************************************************************************)
(* Error code printing. *)
(*****************************************************************************)

let internal_error pos msg = add 0 pos ("Internal error: " ^ msg)

let unimplemented_feature pos msg = add 0 pos ("Feature not implemented: " ^ msg)

let experimental_feature pos msg =
  add 0 pos ("Cannot use experimental feature: " ^ msg)

let strip_ns id = id |> Utils.strip_ns |> Hh_autoimport.reverse_type

(*****************************************************************************)
(* Parsing errors. *)
(*****************************************************************************)

let fixme_format pos =
  add
    (Parsing.err_code Parsing.FixmeFormat)
    pos
    "HH_FIXME wrong format, expected '/* HH_FIXME[ERROR_NUMBER] */'"

let parsing_error (p, msg) = add (Parsing.err_code Parsing.ParsingError) p msg

let xhp_parsing_error (p, msg) =
  add (Parsing.err_code Parsing.XhpParsingError) p msg

(*****************************************************************************)
(* Legacy AST / AAST errors *)
(*****************************************************************************)

let mk_unsupported_trait_use_as pos =
  ( Naming.err_code Naming.UnsupportedTraitUseAs,
    [(pos, "Trait use as is a PHP feature that is unsupported in Hack")] )

let unsupported_trait_use_as pos =
  add_error_with_check (mk_unsupported_trait_use_as pos)

let mk_unsupported_instead_of pos =
  ( Naming.err_code Naming.UnsupportedInsteadOf,
    [(pos, "insteadof is a PHP feature that is unsupported in Hack")] )

let unsupported_instead_of pos =
  add_error_with_check (mk_unsupported_instead_of pos)

let mk_invalid_trait_use_as_visibility pos =
  ( Naming.err_code Naming.InvalidTraitUseAsVisibility,
    [(pos, "Cannot redeclare trait method's visibility in this manner")] )

let invalid_trait_use_as_visibility pos =
  add_error_with_check (mk_invalid_trait_use_as_visibility pos)

(*****************************************************************************)
(* Naming errors *)
(*****************************************************************************)

let unexpected_arrow pos cname =
  add
    (Naming.err_code Naming.UnexpectedArrow)
    pos
    ("Keys may not be specified for " ^ cname ^ " initialization")

let missing_arrow pos cname =
  add
    (Naming.err_code Naming.MissingArrow)
    pos
    ("Keys must be specified for " ^ cname ^ " initialization")

let disallowed_xhp_type pos name =
  add
    (Naming.err_code Naming.DisallowedXhpType)
    pos
    (name ^ " is not a valid type. Use :xhp or XHPChild.")

let name_is_reserved name pos =
  let name = Utils.strip_all_ns name in
  add
    (Naming.err_code Naming.NameIsReserved)
    pos
    (name ^ " cannot be used as it is reserved.")

let dollardollar_unused pos =
  add
    (Naming.err_code Naming.DollardollarUnused)
    pos
    ( "This expression does not contain a "
    ^ "usage of the special pipe variable. Did you forget to use the ($$) "
    ^ "variable?" )

let method_name_already_bound pos name =
  add
    (Naming.err_code Naming.MethodNameAlreadyBound)
    pos
    ("Method name already bound: " ^ name)

let error_name_already_bound name name_prev p p_prev =
  let name = strip_ns name in
  let name_prev = strip_ns name_prev in
  let errs =
    [
      (p, "Name already bound: " ^ name);
      ( p_prev,
        if String.compare name name_prev = 0 then
          "Previous definition is here"
        else
          "Previous definition "
          ^ name_prev
          ^ " differs only in capitalization " );
    ]
  in
  let hhi_msg =
    "This appears to be defined in an hhi file included in your project "
    ^ "root. The hhi files for the standard library are now a part of the "
    ^ "typechecker and must be removed from your project. Typically, you can "
    ^ "do this by deleting the \"hhi\" directory you copied into your "
    ^ "project when first starting with Hack."
  in
  let errs =
    if Relative_path.prefix (Pos.filename p) = Relative_path.Hhi then
      errs @ [(p_prev, hhi_msg)]
    else if Relative_path.prefix (Pos.filename p_prev) = Relative_path.Hhi then
      errs @ [(p, hhi_msg)]
    else
      errs
  in
  add_list (Naming.err_code Naming.ErrorNameAlreadyBound) errs

let error_class_attribute_already_bound name name_prev p p_prev =
  let name = strip_ns name in
  let name_prev = strip_ns name_prev in
  let errs =
    [
      ( p,
        "A class and an attribute class cannot share the same name. Conflicting class: "
        ^ name );
      (p_prev, "Previous definition: " ^ name_prev);
    ]
  in
  add_list (Naming.err_code Naming.AttributeClassNameConflict) errs

let unbound_name pos name kind =
  let kind_str =
    match kind with
    | ConstantNamespace -> " (a global constant)"
    | FunctionNamespace -> " (a global function)"
    | TypeNamespace -> ""
    | ClassContext -> " (an object type)"
    | TraitContext -> " (a trait)"
    | RecordContext -> " (a record type)"
  in
  add
    (Naming.err_code Naming.UnboundName)
    pos
    ("Unbound name: " ^ strip_ns name ^ kind_str)

let invalid_fun_pointer pos name =
  add
    (Naming.err_code Naming.InvalidFunPointer)
    pos
    ( "Unbound global function: '"
    ^ strip_ns name
    ^ "' is not a valid name for fun()" )

let rx_move_invalid_location pos =
  add
    (Naming.err_code Naming.RxMoveInvalidLocation)
    pos
    "Rx\\move is only allowed in argument position or as right hand side of the assignment."

let undefined ~in_rx_scope pos var_name did_you_mean =
  let msg =
    if in_rx_scope then
      Printf.sprintf
        "Variable %s is undefined, not always defined, or unset afterwards"
        var_name
    else
      Printf.sprintf "Variable %s is undefined, or not always defined" var_name
  in
  let suggestion =
    match did_you_mean with
    | Some var_name -> Printf.sprintf " (did you mean %s instead?)" var_name
    | None -> ""
  in
  add_list (Naming.err_code Naming.Undefined) [(pos, msg ^ suggestion)]

let this_reserved pos =
  add
    (Naming.err_code Naming.ThisReserved)
    pos
    "The type parameter \"this\" is reserved"

let start_with_T pos =
  add
    (Naming.err_code Naming.StartWith_T)
    pos
    "Please make your type parameter start with the letter T (capital)"

let already_bound pos name =
  add
    (Naming.err_code Naming.NameAlreadyBound)
    pos
    ("Argument already bound: " ^ name)

let unexpected_typedef pos def_pos =
  add_list
    (Naming.err_code Naming.UnexpectedTypedef)
    [(pos, "Unexpected typedef"); (def_pos, "Definition is here")]

let mk_fd_name_already_bound pos =
  ( Naming.err_code Naming.FdNameAlreadyBound,
    [(pos, "Field name already bound")] )

let fd_name_already_bound pos =
  add_error_with_check (mk_fd_name_already_bound pos)

let repeated_record_field name pos prev_pos =
  let msg = Printf.sprintf "Duplicate record field `%s`" name in
  add_list
    (NastCheck.err_code NastCheck.RepeatedRecordFieldName)
    [(pos, msg); (prev_pos, "Previous field is here")]

let unexpected_record_field_name ~field_name ~field_pos ~record_name ~decl_pos =
  let msg =
    Printf.sprintf
      "Record `%s` has no field `%s`"
      (strip_ns record_name)
      field_name
  in
  add_list
    (Typing.err_code Typing.RecordUnknownField)
    [(field_pos, msg); (decl_pos, "Definition is here")]

let missing_record_field_name ~field_name ~new_pos ~record_name ~field_decl_pos
    =
  let msg =
    Printf.sprintf
      "Mising required field `%s` in `%s`"
      field_name
      (strip_ns record_name)
  in
  add_list
    (Typing.err_code Typing.RecordMissingRequiredField)
    [(new_pos, msg); (field_decl_pos, "Field definition is here")]

let type_not_record id pos =
  add
    (Typing.err_code Typing.NotARecord)
    pos
    (Printf.sprintf "Expected a record type, but got `%s`." (strip_ns id))

let primitive_toplevel pos =
  add
    (Naming.err_code Naming.PrimitiveToplevel)
    pos
    "Primitive type annotations are always available and may no longer be referred to in the toplevel namespace."

let primitive_invalid_alias pos used valid =
  add
    (Naming.err_code Naming.PrimitiveInvalidAlias)
    pos
    ( "Invalid Hack type. Using '"
    ^ used
    ^ "' in Hack is considered an error. Use '"
    ^ valid
    ^ "' instead, to keep the codebase consistent." )

let dynamic_new_in_strict_mode pos =
  add
    (Naming.err_code Naming.DynamicNewInStrictMode)
    pos
    "Cannot use dynamic new."

let invalid_type_access_root (pos, id) =
  add
    (Naming.err_code Naming.InvalidTypeAccessRoot)
    pos
    (id ^ " must be an identifier for a class, \"self\", or \"this\"")

let duplicate_user_attribute (pos, name) existing_attr_pos =
  add_list
    (Naming.err_code Naming.DuplicateUserAttribute)
    [
      (pos, "You cannot reuse the attribute " ^ name);
      (existing_attr_pos, name ^ " was already used here");
    ]

let unbound_attribute_name pos name =
  let reason =
    if string_starts_with name "__" then
      "starts with __ but is not a standard attribute"
    else
      "does not have a class. Please declare a class for the attribute."
  in
  add
    (Naming.err_code Naming.UnboundName)
    pos
    ("Unrecognized user attribute: " ^ strip_ns name ^ " " ^ reason)

let this_no_argument pos =
  add
    (Naming.err_code Naming.ThisNoArgument)
    pos
    "\"this\" expects no arguments"

let object_cast pos =
  add
    (Naming.err_code Naming.ObjectCast)
    pos
    "Casts are only supported for bool, int, float and string."

let this_hint_outside_class pos =
  add
    (Naming.err_code Naming.ThisHintOutsideClass)
    pos
    "Cannot use \"this\" outside of a class"

let this_type_forbidden pos =
  add
    (Naming.err_code Naming.ThisMustBeReturn)
    pos
    "The type \"this\" cannot be used as a constraint on a class' generic, or as the type of a static member variable"

let nonstatic_property_with_lsb pos =
  add
    (Naming.err_code Naming.NonstaticPropertyWithLSB)
    pos
    "__LSB attribute may only be used on static properties"

let lowercase_this pos type_ =
  add
    (Naming.err_code Naming.LowercaseThis)
    pos
    ("Invalid Hack type \"" ^ type_ ^ "\". Use \"this\" instead")

let classname_param pos =
  add
    (Naming.err_code Naming.ClassnameParam)
    pos
    ( "Missing type parameter to classname; classname is entirely"
    ^ " meaningless without one" )

let tparam_with_tparam pos x =
  add
    (Naming.err_code Naming.TparamWithTparam)
    pos
    (Printf.sprintf
       "%s is a type parameter. Type parameters cannot themselves take type parameters (e.g. %s<int> doesn't make sense)"
       x
       x)

let shadowed_type_param p pos name =
  add_list
    (Naming.err_code Naming.ShadowedTypeParam)
    [
      (p, Printf.sprintf "You cannot re-bind the type parameter %s" name);
      (pos, Printf.sprintf "%s is already bound here" name);
    ]

let missing_typehint pos =
  add (Naming.err_code Naming.MissingTypehint) pos "Please add a type hint"

let expected_variable pos =
  add
    (Naming.err_code Naming.ExpectedVariable)
    pos
    "Was expecting a variable name"

let clone_too_many_arguments pos =
  add
    (Naming.err_code Naming.NamingTooManyArguments)
    pos
    "__clone method cannot take arguments"

let naming_too_few_arguments pos =
  add (Naming.err_code Naming.NamingTooFewArguments) pos "Too few arguments"

let naming_too_many_arguments pos =
  add (Naming.err_code Naming.NamingTooManyArguments) pos "Too many arguments"

let expected_collection pos cn =
  add
    (Naming.err_code Naming.ExpectedCollection)
    pos
    ("Unexpected collection type " ^ strip_ns cn)

let illegal_CLASS pos =
  add
    (Naming.err_code Naming.IllegalClass)
    pos
    "Using __CLASS__ outside a class or trait"

let illegal_TRAIT pos =
  add
    (Naming.err_code Naming.IllegalTrait)
    pos
    "Using __TRAIT__ outside a trait"

let lvar_in_obj_get pos =
  add
    (Naming.err_code Naming.LvarInObjGet)
    pos
    "Dynamic method or attribute access is not allowed on a non-dynamic value."

let nullsafe_property_write_context pos =
  add
    (Typing.err_code Typing.NullsafePropertyWriteContext)
    pos
    "?-> syntax not supported here, this function effectively does a write"

let illegal_fun pos =
  let msg =
    "The argument to fun() must be a single-quoted, constant "
    ^ "literal string representing a valid function name."
  in
  add (Naming.err_code Naming.IllegalFun) pos msg

let illegal_member_variable_class pos =
  let msg =
    "Cannot declare a constant named 'class'. The name 'class' is reserved for the class constant that represents the name of the class"
  in
  add (Naming.err_code Naming.IllegalMemberVariableClass) pos msg

let illegal_meth_fun pos =
  let msg =
    "String argument to fun() contains ':';"
    ^ " for static class methods, use"
    ^ " class_meth(Cls::class, 'method_name'), not fun('Cls::method_name')"
  in
  add (Naming.err_code Naming.IllegalMethFun) pos msg

let illegal_inst_meth pos =
  let msg =
    "The argument to inst_meth() must be an expression and a "
    ^ "constant literal string representing a valid method name."
  in
  add (Naming.err_code Naming.IllegalInstMeth) pos msg

let illegal_meth_caller pos =
  let msg =
    "The two arguments to meth_caller() must be:"
    ^ "\n - first: ClassOrInterface::class"
    ^ "\n - second: a single-quoted string literal containing the name"
    ^ " of a non-static method of that class"
  in
  add (Naming.err_code Naming.IllegalMethCaller) pos msg

let illegal_class_meth pos =
  let msg =
    "The two arguments to class_meth() must be:"
    ^ "\n - first: ValidClassname::class"
    ^ "\n - second: a single-quoted string literal containing the name"
    ^ " of a static method of that class"
  in
  add (Naming.err_code Naming.IllegalClassMeth) pos msg

let class_meth_non_final_self pos class_name =
  let msg =
    "`class_meth` with `self::class` does not preserve class calling context.\n"
    ^ "Use `static::class`, or `"
    ^ strip_ns class_name
    ^ "::class` explicitly"
  in
  add (Naming.err_code Naming.ClassMethNonFinalSelf) pos msg

let assert_arity pos =
  add
    (Naming.err_code Naming.AssertArity)
    pos
    "assert expects exactly one argument"

let unexpected_ty_in_tast pos ~actual_ty ~expected_ty =
  add
    (Typing.err_code Typing.UnexpectedTy)
    pos
    ("Unexpected type in TAST: expected " ^ expected_ty ^ ", got " ^ actual_ty)

let uninstantiable_class usage_pos decl_pos name reason_msgl =
  let name = strip_ns name in
  let msgl =
    [
      (usage_pos, name ^ " is uninstantiable"); (decl_pos, "Declaration is here");
    ]
  in
  let msgl =
    match reason_msgl with
    | (reason_pos, reason_str) :: tail ->
      ((reason_pos, reason_str ^ " which must be instantiable") :: tail) @ msgl
    | _ -> msgl
  in
  add_list (Typing.err_code Typing.UninstantiableClass) msgl

let new_abstract_record (pos, name) =
  let name = strip_ns name in
  add
    (Typing.err_code Typing.NewAbstractRecord)
    pos
    (Printf.sprintf "Cannot create instance of abstract record `%s`" name)

let abstract_const_usage usage_pos decl_pos name =
  let name = strip_ns name in
  add_list
    (Typing.err_code Typing.AbstractConstUsage)
    [
      (usage_pos, "Cannot reference abstract constant " ^ name ^ " directly");
      (decl_pos, "Declaration is here");
    ]

let const_without_typehint sid =
  let (pos, name) = sid in
  let msg =
    Printf.sprintf
      "Please add a type hint 'const SomeType %s'"
      (Utils.strip_all_ns name)
  in
  add (Naming.err_code Naming.AddATypehint) pos msg

let prop_without_typehint visibility sid =
  let (pos, name) = sid in
  let msg =
    Printf.sprintf "Please add a type hint '%s SomeType %s'" visibility name
  in
  add (Naming.err_code Naming.AddATypehint) pos msg

let illegal_constant pos =
  add (Naming.err_code Naming.IllegalConstant) pos "Illegal constant value"

let invalid_req_implements pos =
  add
    (Naming.err_code Naming.InvalidReqImplements)
    pos
    "Only traits may use 'require implements'"

let invalid_req_extends pos =
  add
    (Naming.err_code Naming.InvalidReqExtends)
    pos
    "Only traits and interfaces may use 'require extends'"

let did_you_mean_naming pos name suggest_pos suggest_name =
  add_list
    (Naming.err_code Naming.DidYouMeanNaming)
    [
      (pos, "Could not find " ^ strip_ns name);
      (suggest_pos, "Did you mean " ^ strip_ns suggest_name ^ "?");
    ]

let using_internal_class pos name =
  add
    (Naming.err_code Naming.UsingInternalClass)
    pos
    (name ^ " is an implementation internal class that cannot be used directly")

let too_few_type_arguments p =
  add
    (Naming.err_code Naming.TooFewTypeArguments)
    p
    "Too few type arguments for this type"

let goto_label_already_defined
    label_name redeclaration_pos original_delcaration_pos =
  add_list
    (Naming.err_code Naming.GotoLabelAlreadyDefined)
    [
      (redeclaration_pos, "Cannot redeclare the goto label '" ^ label_name ^ "'");
      (original_delcaration_pos, "Declaration is here");
    ]

let goto_label_undefined pos label_name =
  add
    (Naming.err_code Naming.GotoLabelUndefined)
    pos
    ("Undefined goto label: " ^ label_name)

let goto_label_defined_in_finally pos =
  add
    (Naming.err_code Naming.GotoLabelDefinedInFinally)
    pos
    "It is illegal to define a goto label within a finally block."

let goto_invoked_in_finally pos =
  add
    (Naming.err_code Naming.GotoInvokedInFinally)
    pos
    "It is illegal to invoke goto within a finally block."

let mk_method_needs_visibility pos =
  ( Naming.err_code Naming.MethodNeedsVisibility,
    [(pos, "Methods need to be marked public, private, or protected.")] )

let method_needs_visibility pos =
  add_error_with_check (mk_method_needs_visibility pos)

let dynamic_class_name_in_strict_mode pos =
  add
    (Naming.err_code Naming.DynamicClassNameInStrictMode)
    pos
    "Cannot use dynamic class name in strict mode"

let xhp_optional_required_attr pos id =
  add
    (Naming.err_code Naming.XhpOptionalRequiredAttr)
    pos
    ("XHP attribute " ^ id ^ " cannot be marked as nullable and required")

let xhp_required_with_default pos id =
  add
    (Naming.err_code Naming.XhpRequiredWithDefault)
    pos
    ( "XHP attribute "
    ^ id
    ^ " cannot be marked as required and provide a default" )

let array_typehints_disallowed pos =
  add
    (Naming.err_code Naming.ArrayTypehintsDisallowed)
    pos
    "Array typehints are no longer legal; use varray or darray instead"

let array_literals_disallowed pos =
  add
    (Naming.err_code Naming.ArrayLiteralsDisallowed)
    pos
    "Array literals are no longer legal; use varray or darray instead"

let wildcard_disallowed pos =
  add
    (Naming.err_code Naming.WildcardDisallowed)
    pos
    "Wildcard typehints are not allowed in this position"

let misplaced_mutability_hint pos =
  add
    (Naming.err_code Naming.MisplacedMutabilityHint)
    pos
    "Setting mutability via type hints is only allowed for parameters of reactive function types. For other cases consider using attributes."

let mutability_hint_in_non_rx_function pos =
  add
    (Naming.err_code Naming.MutabilityHintInNonRx)
    pos
    "Parameter with mutability hint cannot appear in non-reactive function type."

let invalid_mutability_in_return_type_hint pos =
  add
    (Naming.err_code Naming.InvalidReturnMutableHint)
    pos
    "OwnedMutable is the only mutability related hint allowed in return type annotation for reactive function types."

let pu_duplication pos kind name seen =
  let name = strip_ns name in
  let seen = strip_ns seen in
  add
    (Naming.err_code Naming.PocketUniversesDuplication)
    pos
    (sprintf "Pocket Universe %s %s is already declared in %s" kind name seen)

let pu_not_in_class pos name loc =
  let name = strip_ns name in
  let loc = strip_ns loc in
  add
    (Naming.err_code Naming.PocketUniversesNotInClass)
    pos
    (sprintf "Pocket Universe %s is defined outside a class (%s)" name loc)

let pu_atom_missing pos name kind loc missing =
  let name = strip_ns name in
  let loc = strip_ns loc in
  add
    (Naming.err_code Naming.PocketUniversesAtomMissing)
    pos
    (sprintf
       "In Pocket Universe %s, atom %s is missing %s %s"
       loc
       name
       kind
       missing)

let pu_atom_unknown pos name kind loc unk =
  let name = strip_ns name in
  let loc = strip_ns loc in
  add
    (Naming.err_code Naming.PocketUniversesAtomUnknown)
    pos
    (sprintf
       "In Pocket Universe %s, atom %s declares unknown %s %s"
       loc
       name
       kind
       unk)

let pu_localize pos pu member =
  let pu = strip_ns pu in
  let member = strip_ns member in
  add
    (Naming.err_code Naming.PocketUniversesLocalization)
    pos
    (sprintf
       "In %s, member %s is neither an atom nor a generic type parameter"
       pu
       member)

let pu_localize_unknown pos msg =
  let msg = strip_ns msg in
  add
    (Naming.err_code Naming.PocketUniversesLocalization)
    pos
    (sprintf "Illegal Pocket Universe invocation %s" msg)

let illegal_use_of_dynamically_callable attr_pos meth_pos visibility =
  add_list
    (Naming.err_code Naming.IllegalUseOfDynamicallyCallable)
    [
      (attr_pos, "__DynamicallyCallable can only be used on public methods");
      (meth_pos, sprintf "But this method is %s" visibility);
    ]

(*****************************************************************************)
(* Init check errors *)
(*****************************************************************************)

let no_construct_parent pos =
  add
    (NastCheck.err_code NastCheck.NoConstructParent)
    pos
    (Utils.sl
       [
         "You are extending a class that needs to be initialized\n";
         "Make sure you call parent::__construct.\n";
       ])

let nonstatic_method_in_abstract_final_class pos =
  add
    (NastCheck.err_code NastCheck.NonstaticMethodInAbstractFinalClass)
    pos
    "Abstract final classes cannot have nonstatic methods or constructors."

let constructor_required (pos, name) prop_names =
  let name = strip_ns name in
  let props_str =
    SSet.fold ~f:(fun x acc -> x ^ " " ^ acc) prop_names ~init:""
  in
  add
    (NastCheck.err_code NastCheck.ConstructorRequired)
    pos
    ( "Lacking __construct, class "
    ^ name
    ^ " does not initialize its private member(s): "
    ^ props_str )

let not_initialized (pos, cname) prop_names =
  let cname = strip_ns cname in
  let props_str =
    List.fold_right prop_names ~f:(fun x acc -> x ^ " " ^ acc) ~init:""
  in
  let (members, verb) =
    if 1 = List.length prop_names then
      ("member", "is")
    else
      ("members", "are")
  in
  let setters_str =
    List.fold_right
      prop_names
      ~f:(fun x acc -> "$this->" ^ x ^ " " ^ acc)
      ~init:""
  in
  add
    (NastCheck.err_code NastCheck.NotInitialized)
    pos
    (Utils.sl
       [
         "Class ";
         cname;
         " does not initialize all of its members; ";
         props_str;
         verb;
         " not always initialized.";
         "\nMake sure you systematically set ";
         setters_str;
         "when the method __construct is called.";
         "\nAlternatively, you can define the ";
         members;
         " as nullable (?...)\n";
       ])

let call_before_init pos cv =
  add
    (NastCheck.err_code NastCheck.CallBeforeInit)
    pos
    (Utils.sl
       ( [
           "Until the initialization of $this is over,";
           " you can only call private methods\n";
           "The initialization is not over because ";
         ]
       @
       if cv = "parent::__construct" then
         ["you forgot to call parent::__construct"]
       else
         ["$this->"; cv; " can still potentially be null"] ))

(*****************************************************************************)
(* Nast errors check *)
(*****************************************************************************)

let type_arity use_pos def_pos ~expected ~actual =
  add_list
    (Typing.err_code Typing.TypeArityMismatch)
    [
      ( use_pos,
        Printf.sprintf
          "Wrong number of type arguments (expected %d, got %d)"
          expected
          actual );
      (def_pos, "Definition is here");
    ]

let abstract_with_body (p, _) =
  add
    (NastCheck.err_code NastCheck.AbstractWithBody)
    p
    "This method is declared as abstract, but has a body"

let not_abstract_without_body (p, _) =
  add
    (NastCheck.err_code NastCheck.NotAbstractWithoutBody)
    p
    "This method is not declared as abstract, it must have a body"

let mk_not_abstract_without_typeconst (p, _) =
  ( NastCheck.err_code NastCheck.NotAbstractWithoutTypeconst,
    [
      ( p,
        "This type constant is not declared as abstract, it must have"
        ^ " an assigned type" );
    ] )

let not_abstract_without_typeconst node =
  add_error_with_check (mk_not_abstract_without_typeconst node)

let typeconst_depends_on_external_tparam pos ext_pos ext_name =
  add_list
    (NastCheck.err_code NastCheck.TypeconstDependsOnExternalTparam)
    [
      ( pos,
        "A type constant can only use type parameters declared in its own"
        ^ " type parameter list" );
      (ext_pos, ext_name ^ " was declared as a type parameter here");
    ]

let interface_with_partial_typeconst tconst_pos =
  add
    (NastCheck.err_code NastCheck.InterfaceWithPartialTypeconst)
    tconst_pos
    "An interface cannot contain a partially abstract type constant"

let mk_multiple_xhp_category pos =
  ( NastCheck.err_code NastCheck.MultipleXhpCategory,
    [(pos, "XHP classes can only contain one category declaration")] )

let multiple_xhp_category pos =
  add_error_with_check (mk_multiple_xhp_category pos)

let return_in_gen p =
  add
    (NastCheck.err_code NastCheck.ReturnInGen)
    p
    ( "You cannot return a value in a generator (a generator"
    ^ " is a function that uses yield)" )

let return_in_finally p =
  add
    (NastCheck.err_code NastCheck.ReturnInFinally)
    p
    ( "Don't use return in a finally block;"
    ^ " there's nothing to receive the return value" )

let toplevel_break p =
  add
    (NastCheck.err_code NastCheck.ToplevelBreak)
    p
    "break can only be used inside loops or switch statements"

let toplevel_continue p =
  add
    (NastCheck.err_code NastCheck.ToplevelContinue)
    p
    "continue can only be used inside loops"

let continue_in_switch p =
  add
    (NastCheck.err_code NastCheck.ContinueInSwitch)
    p
    ( "In PHP, 'continue;' inside a switch statement is equivalent to 'break;'."
    ^ " Hack does not support this; use 'break' if that is what you meant." )

let await_in_sync_function p =
  add
    (NastCheck.err_code NastCheck.AwaitInSyncFunction)
    p
    "await can only be used inside async functions"

let interface_use_trait p =
  add
    (NastCheck.err_code NastCheck.InterfaceUsesTrait)
    p
    "Interfaces cannot use traits"

let await_in_coroutine p =
  add
    (NastCheck.err_code NastCheck.AwaitInCoroutine)
    p
    "await is not allowed in coroutines."

let yield_in_coroutine p =
  add
    (NastCheck.err_code NastCheck.YieldInCoroutine)
    p
    "yield is not allowed in coroutines."

let suspend_outside_of_coroutine p =
  add
    (NastCheck.err_code NastCheck.SuspendOutsideOfCoroutine)
    p
    "suspend is only allowed in coroutines."

let suspend_in_finally p =
  add
    (NastCheck.err_code NastCheck.SuspendInFinally)
    p
    "suspend is not allowed inside finally blocks."

let static_memoized_function p =
  add
    (NastCheck.err_code NastCheck.StaticMemoizedFunction)
    p
    "memoize is not allowed on static methods in classes that aren't final "

let magic (p, s) =
  add
    (NastCheck.err_code NastCheck.Magic)
    p
    (s ^ " is a magic method and cannot be called directly")

let non_interface (p : Pos.t) (c2 : string) (verb : string) : 'a =
  add
    (NastCheck.err_code NastCheck.NonInterface)
    p
    ("Cannot " ^ verb ^ " " ^ strip_ns c2 ^ " - it is not an interface")

let toString_returns_string pos =
  add
    (NastCheck.err_code NastCheck.ToStringReturnsString)
    pos
    "__toString should return a string"

let toString_visibility pos =
  add
    (NastCheck.err_code NastCheck.ToStringVisibility)
    pos
    "__toString must have public visibility and cannot be static"

let uses_non_trait (p : Pos.t) (n : string) (t : string) =
  add
    (NastCheck.err_code NastCheck.UsesNonTrait)
    p
    (strip_ns n ^ " is not a trait. It is " ^ t ^ ".")

let requires_non_class (p : Pos.t) (n : string) (t : string) =
  add
    (NastCheck.err_code NastCheck.RequiresNonClass)
    p
    (strip_ns n ^ " is not a class. It is " ^ t ^ ".")

let requires_final_class (p : Pos.t) (n : string) =
  add
    (NastCheck.err_code NastCheck.RequiresFinalClass)
    p
    (strip_ns n ^ " is not an extendable class.")

let abstract_body pos =
  add
    (NastCheck.err_code NastCheck.AbstractBody)
    pos
    "This method shouldn't have a body"

let interface_with_member_variable pos =
  add
    (NastCheck.err_code NastCheck.InterfaceWithMemberVariable)
    pos
    "Interfaces cannot have member variables"

let interface_with_static_member_variable pos =
  add
    (NastCheck.err_code NastCheck.InterfaceWithStaticMemberVariable)
    pos
    "Interfaces cannot have static variables"

let illegal_function_name pos mname =
  add
    (NastCheck.err_code NastCheck.IllegalFunctionName)
    pos
    ("Illegal function name: " ^ strip_ns mname)

let inout_params_outside_of_sync pos =
  add
    (NastCheck.err_code NastCheck.InoutParamsOutsideOfSync)
    pos
    ( "Inout parameters cannot be defined on async functions, "
    ^ "generators or coroutines." )

let mutable_attribute_on_function pos =
  add
    (NastCheck.err_code NastCheck.MutableAttributeOnFunction)
    pos
    "<<__Mutable>> only makes sense on methods, or parameters on functions or methods."

let maybe_mutable_attribute_on_function pos =
  add
    (NastCheck.err_code NastCheck.MaybeMutableAttributeOnFunction)
    pos
    "<<__MaybeMutable>> only makes sense on methods, or parameters on functions or methods."

let conflicting_mutable_and_maybe_mutable_attributes pos =
  add
    (NastCheck.err_code NastCheck.ConflictingMutableAndMaybeMutableAttributes)
    pos
    "Declaration cannot have both <<__Mutable>> and <<__MaybeMutable>> attributtes."

let mutable_methods_must_be_reactive pos name =
  add
    (NastCheck.err_code NastCheck.MutableMethodsMustBeReactive)
    pos
    ( "The method "
    ^ strip_ns name
    ^ " has a mutable parameter"
    ^ " (or mutable this), so it must be marked reactive with <<__Rx>>." )

let mutable_return_annotated_decls_must_be_reactive kind pos name =
  add
    (NastCheck.err_code NastCheck.MutableReturnAnnotatedDeclsMustBeReactive)
    pos
    ( "The "
    ^ kind
    ^ " "
    ^ strip_ns name
    ^ " is annotated with <<__MutableReturn>>, "
    ^ " so it must be marked reactive with <<__Rx>>." )

let maybe_mutable_methods_must_be_reactive pos name =
  add
    (NastCheck.err_code NastCheck.MaybeMutableMethodsMustBeReactive)
    pos
    ( "The method "
    ^ strip_ns name
    ^ " is annotated with <<__MaybeMutable> attribute, or has this attribute on one of parameters so it must be marked reactive."
    )

let inout_params_special pos =
  add
    (NastCheck.err_code NastCheck.InoutParamsSpecial)
    pos
    "Methods with special semantics cannot have inout parameters."

let inout_params_memoize fpos pos =
  let msg1 = (fpos, "Functions with inout parameters cannot be memoized") in
  let msg2 = (pos, "This is an inout parameter") in
  add_list (NastCheck.err_code NastCheck.InoutParamsMemoize) [msg1; msg2]

let reading_from_append pos =
  add
    (NastCheck.err_code NastCheck.ReadingFromAppend)
    pos
    "Cannot use [] for reading"

let inout_argument_bad_expr pos =
  add
    (NastCheck.err_code NastCheck.InoutArgumentBadExpr)
    pos
    ( "Arguments for inout parameters must be local variables or simple "
    ^ "subscript expressions on vecs, dicts, keysets, or arrays" )

let illegal_destructor pos =
  add
    (NastCheck.err_code NastCheck.IllegalDestructor)
    pos
    ( "Destructors are not supported in Hack; use other patterns like "
    ^ "IDisposable/using or try/catch instead." )

let multiple_conditionally_reactive_annotations pos name =
  add
    (NastCheck.err_code NastCheck.MultipleConditionallyReactiveAnnotations)
    pos
    ("Method '" ^ name ^ "' has multiple <<__OnlyRxIfImpl>> annotations.")

let rx_is_enabled_invalid_location pos =
  add
    (NastCheck.err_code NastCheck.RxIsEnabledInvalidLocation)
    pos
    ( "HH\\Rx\\IS_ENABLED must be the only condition in an if-statement, "
    ^ "and that if-statement must be the only statement in the function body."
    )

let atmost_rx_as_rxfunc_invalid_location pos =
  add
    (NastCheck.err_code NastCheck.MaybeRxInvalidLocation)
    pos
    "<<__AtMostRxAsFunc>> attribute can only be put on parameters of conditionally reactive function or method annotated with <<__AtMostRxAsArgs>> attribute."

let no_atmost_rx_as_rxfunc_for_rx_if_args pos =
  add
    (NastCheck.err_code NastCheck.NoOnlyrxIfRxfuncForRxIfArgs)
    pos
    "Function or method annotated with <<__AtMostRxAsArgs>> attribute should have at least one parameter with <<__AtMostRxAsFunc>> or <<__OnlyRxIfImpl>> annotations."

let conditionally_reactive_annotation_invalid_arguments ~is_method pos =
  let loc =
    if is_method then
      "Method"
    else
      "Parameter"
  in
  add
    (NastCheck.err_code
       NastCheck.ConditionallyReactiveAnnotationInvalidArguments)
    pos
    ( loc
    ^ " is marked with <<__OnlyRxIfImpl>> attribute that have "
    ^ "invalid arguments. This attribute must have one argument and it should be "
    ^ "'::class' class constant." )

let coroutine_in_constructor pos =
  add
    (NastCheck.err_code NastCheck.CoroutineInConstructor)
    pos
    "A class constructor may not be a coroutine"

let switch_non_terminal_default pos =
  add
    (NastCheck.err_code NastCheck.SwitchNonTerminalDefault)
    pos
    "Default case in switch must be terminal"

let switch_multiple_default pos =
  add
    (NastCheck.err_code NastCheck.SwitchMultipleDefault)
    pos
    "There can be only one default case in switch"

(*****************************************************************************)
(* Nast terminality *)
(*****************************************************************************)

let case_fallthrough pos1 pos2 =
  add_list
    (NastCheck.err_code NastCheck.CaseFallthrough)
    [
      ( pos1,
        "This switch has a case that implicitly falls through and is "
        ^ "not annotated with // FALLTHROUGH" );
      (pos2, "This case implicitly falls through");
    ]

let default_fallthrough pos =
  add
    (NastCheck.err_code NastCheck.DefaultFallthrough)
    pos
    ( "This switch has a default case that implicitly falls "
    ^ "through and is not annotated with // FALLTHROUGH" )

(*****************************************************************************)
(* Typing errors *)
(*****************************************************************************)

let visibility_extends
    vis pos parent_pos parent_vis (on_error : typing_error_callback) =
  let msg1 = (pos, "This member visibility is: " ^ vis) in
  let msg2 = (parent_pos, parent_vis ^ " was expected") in
  on_error ~code:(Typing.err_code Typing.VisibilityExtends) [msg1; msg2]

let member_not_implemented member_name parent_pos pos defn_pos =
  let msg1 = (pos, "This type doesn't implement the method " ^ member_name) in
  let msg2 = (parent_pos, "Which is required by this interface") in
  let msg3 = (defn_pos, "As defined here") in
  add_list (Typing.err_code Typing.MemberNotImplemented) [msg1; msg2; msg3]

let bad_decl_override parent_pos parent_name pos name msgl =
  let msg1 =
    ( pos,
      "Class "
      ^ strip_ns name
      ^ " does not correctly implement all required members " )
  in
  let msg2 =
    ( parent_pos,
      "Some members are incompatible with those declared in type "
      ^ strip_ns parent_name
      ^ "\nRead the following to see why:" )
  in
  (* This is a cascading error message *)
  add_list (Typing.err_code Typing.BadDeclOverride) (msg1 :: msg2 :: msgl)

let bad_method_override pos member_name msgl (on_error : typing_error_callback)
    =
  let msg = (pos, "Member " ^ strip_ns member_name ^ " has the wrong type") in
  (* This is a cascading error message *)
  on_error ~code:(Typing.err_code Typing.BadMethodOverride) (msg :: msgl)

let bad_enum_decl pos msgl =
  let msg =
    (pos, "This enum declaration is invalid.\nRead the following to see why:")
  in
  (* This is a cascading error message *)
  add_list (Typing.err_code Typing.BadEnumExtends) (msg :: msgl)

let missing_constructor pos (on_error : typing_error_callback) =
  on_error
    ~code:(Typing.err_code Typing.MissingConstructor)
    [(pos, "The constructor is not implemented")]

let typedef_trail_entry pos = (pos, "Typedef definition comes from here")

let abstract_tconst_not_allowed pos (p, tconst_name) =
  add_list
    (Typing.err_code Typing.AbstractTconstNotAllowed)
    [
      (pos, "An abstract type constant is not allowed in this position.");
      (p, Printf.sprintf "%s is abstract here." tconst_name);
    ]

let add_with_trail code errs trail =
  add_list code (errs @ List.map trail typedef_trail_entry)

let enum_constant_type_bad pos ty_pos ty trail =
  add_with_trail
    (Typing.err_code Typing.EnumConstantTypeBad)
    [(pos, "Enum constants must be an int or string"); (ty_pos, "Not " ^ ty)]
    trail

let enum_type_bad pos ty trail =
  add_with_trail
    (Typing.err_code Typing.EnumTypeBad)
    [(pos, "Enums must be int or string or arraykey, not " ^ ty)]
    trail

let enum_type_typedef_nonnull pos =
  add
    (Typing.err_code Typing.EnumTypeTypedefNonnull)
    pos
    "Can't use typedef that resolves to nonnull in enum"

let enum_switch_redundant const first_pos second_pos =
  add_list
    (Typing.err_code Typing.EnumSwitchRedundant)
    [
      (second_pos, "Redundant case statement");
      (first_pos, const ^ " already handled here");
    ]

let enum_switch_nonexhaustive pos missing enum_pos =
  add_list
    (Typing.err_code Typing.EnumSwitchNonexhaustive)
    [
      ( pos,
        "Switch statement nonexhaustive; the following cases are missing: "
        ^ String.concat ~sep:", " missing );
      (enum_pos, "Enum declared here");
    ]

let enum_switch_redundant_default pos enum_pos =
  add_list
    (Typing.err_code Typing.EnumSwitchRedundantDefault)
    [
      ( pos,
        "All cases already covered; a redundant default case prevents "
        ^ "detecting future errors. If your goal is to guard against "
        ^ "invalid values for this type, do an `is` check before the switch." );
      (enum_pos, "Enum declared here");
    ]

let enum_switch_not_const pos =
  add
    (Typing.err_code Typing.EnumSwitchNotConst)
    pos
    "Case in switch on enum is not an enum constant"

let enum_switch_wrong_class pos expected got =
  add
    (Typing.err_code Typing.EnumSwitchWrongClass)
    pos
    ("Switching on enum " ^ expected ^ " but using constant from " ^ got)

let invalid_shape_field_name p =
  add
    (Typing.err_code Typing.InvalidShapeFieldName)
    p
    "Was expecting a constant string, class constant, or int (for shape access)"

let invalid_shape_field_name_empty p =
  add
    (Typing.err_code Typing.InvalidShapeFieldNameEmpty)
    p
    "A shape field name cannot be an empty string"

let invalid_shape_field_type pos ty_pos ty trail =
  add_with_trail
    (Typing.err_code Typing.InvalidShapeFieldType)
    [
      (pos, "A shape field name must be an int or string"); (ty_pos, "Not " ^ ty);
    ]
    trail

let invalid_shape_field_literal key_pos witness_pos =
  add_list
    (Typing.err_code Typing.InvalidShapeFieldLiteral)
    [
      (key_pos, "Shape uses literal string as field name");
      (witness_pos, "But expected a class constant");
    ]

let invalid_shape_field_const key_pos witness_pos =
  add_list
    (Typing.err_code Typing.InvalidShapeFieldConst)
    [
      (key_pos, "Shape uses class constant as field name");
      (witness_pos, "But expected a literal string");
    ]

let shape_field_class_mismatch key_pos witness_pos key_class witness_class =
  add_list
    (Typing.err_code Typing.ShapeFieldClassMismatch)
    [
      (key_pos, "Shape field name is class constant from " ^ key_class);
      (witness_pos, "But expected constant from " ^ witness_class);
    ]

let shape_field_type_mismatch key_pos witness_pos key_ty witness_ty =
  add_list
    (Typing.err_code Typing.ShapeFieldTypeMismatch)
    [
      (key_pos, "Shape field name is " ^ key_ty ^ " class constant");
      (witness_pos, "But expected " ^ witness_ty);
    ]

let missing_field pos1 pos2 name (on_error : typing_error_callback) =
  on_error
    ~code:(Typing.err_code Typing.MissingField)
    [
      (pos1, "The field '" ^ name ^ "' is missing");
      (pos2, "The field '" ^ name ^ "' is defined");
    ]

let shape_fields_unknown pos1 pos2 (on_error : typing_error_callback) =
  on_error
    ~code:(Typing.err_code Typing.ShapeFieldsUnknown)
    [
      ( pos1,
        "This shape type allows unknown fields, and so it may contain fields other than those explicitly declared in its declaration."
      );
      ( pos2,
        "It is incompatible with a shape that does not allow unknown fields." );
    ]

let invalid_shape_remove_key p =
  add
    (Typing.err_code Typing.InvalidShapeRemoveKey)
    p
    "You can only unset fields of local variables"

let unification_cycle pos ty =
  add_list
    (Typing.err_code Typing.UnificationCycle)
    [
      ( pos,
        "Type circularity: in order to type-check this expression it "
        ^ "is necessary for a type [rec] to be equal to type "
        ^ ty );
    ]

let violated_constraint
    p_cstr (p_tparam, tparam) left right (on_error : typing_error_callback) =
  on_error
    ~code:(Typing.err_code Typing.TypeConstraintViolation)
    ( [
        (p_cstr, "Some type constraint(s) are violated here");
        (p_tparam, Printf.sprintf "%s is a constrained type parameter" tparam);
      ]
    @ left
    @ right )

let method_variance pos =
  add
    (Typing.err_code Typing.MethodVariance)
    pos
    "Covariance or contravariance is not allowed in type parameter of method or function."

let explain_constraint ~use_pos ~definition_pos ~param_name msgl =
  let inst_msg = "Some type constraint(s) here are violated" in
  (* There may be multiple constraints instantiated at one spot; avoid
   * duplicating the instantiation message *)
  let msgl =
    match msgl with
    | (p, x) :: rest when x = inst_msg && p = use_pos -> rest
    | _ -> msgl
  in
  let name = strip_ns param_name in
  add_list
    (Typing.err_code Typing.TypeConstraintViolation)
    ( [
        (use_pos, inst_msg);
        (definition_pos, "'" ^ name ^ "' is a constrained type parameter");
      ]
    @ msgl )

let explain_where_constraint ~in_class ~use_pos ~definition_pos msgl =
  let callsite_ty =
    if in_class then
      "class"
    else
      "method"
  in
  let definition_head =
    Printf.sprintf "This is the %s with 'where' type constraints" callsite_ty
  in
  let inst_msg = "A 'where' type constraint is violated here" in
  add_list
    (Typing.err_code Typing.TypeConstraintViolation)
    ([(use_pos, inst_msg); (definition_pos, definition_head)] @ msgl)

let explain_tconst_where_constraint ~use_pos ~definition_pos msgl =
  let inst_msg = "A 'where' type constraint is violated here" in
  add_list
    (Typing.err_code Typing.TypeConstraintViolation)
    ( [
        (use_pos, inst_msg);
        ( definition_pos,
          "This method's where constraints contain a generic type access" );
      ]
    @ msgl )

let format_string pos snippet s class_pos fname class_suggest =
  add_list
    (Typing.err_code Typing.FormatString)
    [
      (pos, "Invalid format string " ^ snippet ^ " in " ^ s);
      ( class_pos,
        "You can add a new format specifier by adding "
        ^ fname
        ^ "() to "
        ^ class_suggest );
    ]

let expected_literal_format_string pos =
  add
    (Typing.err_code Typing.ExpectedLiteralFormatString)
    pos
    "This argument must be a literal format string"

let re_prefixed_non_string pos non_strings =
  add
    (Typing.err_code Typing.RePrefixedNonString)
    pos
    (non_strings ^ " are not allowed to be to be `re`-prefixed")

let bad_regex_pattern pos s =
  add
    (Typing.err_code Typing.BadRegexPattern)
    pos
    ("Bad regex pattern; " ^ s ^ ".")

let generic_array_strict p =
  add
    (Typing.err_code Typing.GenericArrayStrict)
    p
    "You cannot have an array without generics in strict mode"

let option_return_only_typehint p kind =
  let (typehint, reason) =
    match kind with
    | `void -> ("?void", "only return implicitly")
    | `noreturn -> ("?noreturn", "never return")
  in
  add
    (Typing.err_code Typing.OptionReturnOnlyTypehint)
    p
    ( typehint
    ^ " is a nonsensical typehint; a function cannot both "
    ^ reason
    ^ " and return null." )

let tuple_syntax p =
  add
    (Typing.err_code Typing.TupleSyntax)
    p
    "Did you want a tuple? Try (X,Y), not tuple<X,Y>"

let redeclaring_missing_method p trait_method =
  add
    (Typing.err_code Typing.RedeclaringMissingMethod)
    p
    ( "Attempting to redeclare a trait method "
    ^ trait_method
    ^ " which was never inherited. "
    ^ "You might be trying to redeclare a non-static method as static or vice-versa."
    )

let expecting_type_hint p =
  add (Typing.err_code Typing.ExpectingTypeHint) p "Was expecting a type hint"

let expecting_type_hint_variadic p =
  add
    (Typing.err_code Typing.ExpectingTypeHintVariadic)
    p
    "Was expecting a type hint on this variadic parameter"

let expecting_return_type_hint p =
  add
    (Typing.err_code Typing.ExpectingReturnTypeHint)
    p
    "Was expecting a return type hint"

let expecting_awaitable_return_type_hint p =
  add
    (Typing.err_code Typing.ExpectingAwaitableReturnTypeHint)
    p
    "Was expecting an Awaitable return type hint"

let duplicate_using_var pos =
  add
    (Typing.err_code Typing.DuplicateUsingVar)
    pos
    "Local variable already used in 'using' statement"

let illegal_disposable pos verb =
  add
    (Typing.err_code Typing.IllegalDisposable)
    pos
    ("Disposable objects must only be " ^ verb ^ " in a 'using' statement")

let escaping_disposable pos =
  add
    (Typing.err_code Typing.EscapingDisposable)
    pos
    "Variable from 'using' clause may only be used as receiver in method invocation or passed to function with <<__AcceptDisposable>> parameter attribute"

let escaping_disposable_parameter pos =
  add
    (Typing.err_code Typing.EscapingDisposableParameter)
    pos
    "Parameter with <<__AcceptDisposable>> attribute may only be used as receiver in method invocation or passed to another function with <<__AcceptDisposable>> parameter attribute"

let escaping_this pos =
  add
    (Typing.err_code Typing.EscapingThis)
    pos
    "$this implementing IDisposable or IAsyncDisposable may only be used as receiver in method invocation or passed to another function with <<__AcceptDisposable>> parameter attribute"

let escaping_mutable_object pos =
  add
    (Typing.err_code Typing.EscapingMutableObject)
    pos
    "Neither a Mutable nor MaybeMutable object may be captured by an anonymous function."

let must_extend_disposable pos =
  add
    (Typing.err_code Typing.MustExtendDisposable)
    pos
    "A disposable type may not extend a class or use a trait that is not disposable"

let accept_disposable_invariant pos1 pos2 (on_error : typing_error_callback) =
  let msg1 = (pos1, "This parameter is marked <<__AcceptDisposable>>") in
  let msg2 = (pos2, "This parameter is not marked <<__AcceptDisposable>>") in
  on_error ~code:(Typing.err_code Typing.AcceptDisposableInvariant) [msg1; msg2]

let field_kinds pos1 pos2 =
  add_list
    (Typing.err_code Typing.FieldKinds)
    [
      (pos1, "You cannot use this kind of field (value)");
      (pos2, "Mixed with this kind of field (key => value)");
    ]

let unbound_name_typing pos name =
  add
    (Typing.err_code Typing.UnboundNameTyping)
    pos
    ("Unbound name (typing): " ^ strip_ns name)

let previous_default p =
  add
    (Typing.err_code Typing.PreviousDefault)
    p
    ( "A previous parameter has a default value.\n"
    ^ "Remove all the default values for the preceding parameters,\n"
    ^ "or add a default value to this one." )

let return_only_typehint p kind =
  let msg =
    match kind with
    | `void -> "void"
    | `noreturn -> "noreturn"
  in
  add
    (Naming.err_code Naming.ReturnOnlyTypehint)
    p
    ( "The "
    ^ msg
    ^ " typehint can only be used to describe a function return type" )

let unexpected_type_arguments p =
  add
    (Naming.err_code Naming.UnexpectedTypeArguments)
    p
    "Type arguments are not expected for this type"

let too_many_type_arguments p =
  add
    (Naming.err_code Naming.TooManyTypeArguments)
    p
    "Too many type arguments for this type"

let return_in_void pos1 pos2 =
  add_list
    (Typing.err_code Typing.ReturnInVoid)
    [(pos1, "You cannot return a value"); (pos2, "This is a void function")]

let this_var_outside_class p =
  add
    (Typing.err_code Typing.ThisVarOutsideClass)
    p
    "Can't use $this outside of a class"

let unbound_global cst_pos =
  add
    (Typing.err_code Typing.UnboundGlobal)
    cst_pos
    "Unbound global constant (Typing)"

let private_inst_meth ~def_pos ~use_pos =
  add_list
    (Typing.err_code Typing.PrivateInstMeth)
    [
      ( use_pos,
        "You cannot use this method with inst_meth (whether you are in the same class or not)."
      );
      (def_pos, "It is declared as private here");
    ]

let protected_inst_meth ~def_pos ~use_pos =
  add_list
    (Typing.err_code Typing.ProtectedInstMeth)
    [
      ( use_pos,
        "You cannot use this method with inst_meth (whether you are in the same class hierarchy or not)."
      );
      (def_pos, "It is declared as protected here");
    ]

let private_class_meth ~def_pos ~use_pos =
  add_list
    (Typing.err_code Typing.PrivateClassMeth)
    [
      ( use_pos,
        "You cannot use this method with class_meth (whether you are in the same class or not)."
      );
      (def_pos, "It is declared as private here");
    ]

let protected_class_meth ~def_pos ~use_pos =
  add_list
    (Typing.err_code Typing.ProtectedClassMeth)
    [
      ( use_pos,
        "You cannot use this method with class_meth (whether you are in the same class hierarchy or not)."
      );
      (def_pos, "It is declared as protected here");
    ]

let array_cast pos =
  add
    (Typing.err_code Typing.ArrayCast)
    pos
    "(array) cast forbidden; arrays with unspecified key and value types are not allowed"

let string_cast pos ty =
  add (Typing.err_code Typing.StringCast) pos
  @@ Printf.sprintf
       "Cannot cast a value of type %s to string.\nOnly primitives may be used in a (string) cast.\nIf you are trying to cast a Stringish type, please use `stringish_cast`.\nThis functionality is being removed from HHVM."
       ty

let nullable_cast pos ty ty_pos =
  add_list
    (Typing.err_code Typing.NullableCast)
    [
      (pos, "Casting from a nullable type is forbidden");
      (ty_pos, "This is " ^ ty);
    ]

let static_outside_class pos =
  add
    (Typing.err_code Typing.StaticOutsideClass)
    pos
    "'static' is undefined outside of a class"

let self_outside_class pos =
  add
    (Typing.err_code Typing.SelfOutsideClass)
    pos
    "'self' is undefined outside of a class"

let new_inconsistent_construct new_pos (cpos, cname) kind =
  let name = strip_ns cname in
  let preamble =
    match kind with
    | `static -> "Can't use new static() for " ^ name
    | `classname -> "Can't use new on classname<" ^ name ^ ">"
  in
  add_list
    (Typing.err_code Typing.NewStaticInconsistent)
    [
      ( new_pos,
        preamble
        ^ "; __construct arguments are not guaranteed to be consistent in child classes"
      );
      ( cpos,
        "This declaration is neither final nor uses the <<__ConsistentConstruct>> attribute"
      );
    ]

let undefined_parent pos =
  add
    (Typing.err_code Typing.UndefinedParent)
    pos
    "The parent class is undefined"

let parent_outside_class pos =
  add
    (Typing.err_code Typing.ParentOutsideClass)
    pos
    "'parent' is undefined outside of a class"

let parent_abstract_call meth_name call_pos decl_pos =
  add_list
    (Typing.err_code Typing.AbstractCall)
    [
      (call_pos, "Cannot call parent::" ^ meth_name ^ "(); it is abstract");
      (decl_pos, "Declaration is here");
    ]

let self_abstract_call meth_name call_pos decl_pos =
  add_list
    (Typing.err_code Typing.AbstractCall)
    [
      ( call_pos,
        "Cannot call self::"
        ^ meth_name
        ^ "(); it is abstract. Did you mean static::"
        ^ meth_name
        ^ "()?" );
      (decl_pos, "Declaration is here");
    ]

let classname_abstract_call cname meth_name call_pos decl_pos =
  let cname = strip_ns cname in
  add_list
    (Typing.err_code Typing.AbstractCall)
    [
      ( call_pos,
        "Cannot call " ^ cname ^ "::" ^ meth_name ^ "(); it is abstract" );
      (decl_pos, "Declaration is here");
    ]

let static_synthetic_method cname meth_name call_pos decl_pos =
  let cname = strip_ns cname in
  add_list
    (Typing.err_code Typing.StaticSyntheticMethod)
    [
      ( call_pos,
        "Cannot call "
        ^ cname
        ^ "::"
        ^ meth_name
        ^ "(); "
        ^ meth_name
        ^ " is not defined in "
        ^ cname );
      (decl_pos, "Declaration is here");
    ]

let isset_in_strict pos =
  add
    (Typing.err_code Typing.IssetEmptyInStrict)
    pos
    ( "isset tends to hide errors due to variable typos and so is limited to dynamic checks in "
    ^ "strict mode" )

let unset_nonidx_in_strict pos msgs =
  add_list
    (Typing.err_code Typing.UnsetNonidxInStrict)
    ( [
        ( pos,
          "In strict mode, unset is banned except on dynamic, "
          ^ "darray, keyset, or dict indexing" );
      ]
    @ msgs )

let unpacking_disallowed_builtin_function pos name =
  let name = strip_ns name in
  add
    (Typing.err_code Typing.UnpackingDisallowed)
    pos
    ("Arg unpacking is disallowed for " ^ name)

let invalid_destructure pos1 pos2 ty =
  add_list
    (Typing.err_code Typing.InvalidDestructure)
    [
      ( pos1,
        "This expression cannot be destructured with a list(...) expression" );
      (pos2, "This is " ^ ty);
    ]

let unpack_array_required_argument p fp =
  add_list
    (Typing.err_code Typing.SplatArrayRequired)
    [
      ( p,
        "An array cannot be unpacked into the required arguments of a function"
      );
      (fp, "Definition is here");
    ]

let unpack_array_variadic_argument p fp =
  add_list
    (Typing.err_code Typing.SplatArrayRequired)
    [
      ( p,
        "A function that receives an unpacked array as an argument must have a variadic parameter to accept the elements of the array"
      );
      (fp, "Definition is here");
    ]

let array_get_arity pos1 name pos2 =
  add_list
    (Typing.err_code Typing.ArrayGetArity)
    [
      (pos1, "You cannot use this " ^ strip_ns name);
      (pos2, "It is missing its type parameters");
    ]

let typing_error pos msg = add (Typing.err_code Typing.GenericUnify) pos msg

let undefined_field ~use_pos ~name ~shape_type_pos =
  add_list
    (Typing.err_code Typing.UndefinedField)
    [
      (use_pos, "The field " ^ name ^ " is undefined");
      (shape_type_pos, "Definition is here");
    ]

let array_access pos1 pos2 ty =
  add_list
    (Typing.err_code Typing.ArrayAccess)
    ( (pos1, "This is not an object of type KeyedContainer, this is " ^ ty)
    ::
    ( if not (phys_equal pos2 Pos.none) then
      [(pos2, "Definition is here")]
    else
      [] ) )

let keyset_set pos1 pos2 =
  add_list
    (Typing.err_code Typing.KeysetSet)
    ( (pos1, "Elements in a keyset cannot be assigned, use append instead.")
    ::
    ( if not (phys_equal pos2 Pos.none) then
      [(pos2, "Definition is here")]
    else
      [] ) )

let array_append pos1 pos2 ty =
  add_list
    (Typing.err_code Typing.ArrayAppend)
    ( (pos1, ty ^ " does not allow array append")
    ::
    ( if not (phys_equal pos2 Pos.none) then
      [(pos2, "Definition is here")]
    else
      [] ) )

let const_mutation pos1 pos2 ty =
  add_list
    (Typing.err_code Typing.ConstMutation)
    ( (pos1, "You cannot mutate this")
    ::
    ( if not (phys_equal pos2 Pos.none) then
      [(pos2, "This is " ^ ty)]
    else
      [] ) )

let expected_class ?(suffix = "") pos =
  add
    (Typing.err_code Typing.ExpectedClass)
    pos
    ("Was expecting a class" ^ suffix)

let unknown_type description pos r =
  let msg = "Was expecting " ^ description ^ " but type is unknown" in
  add_list (Typing.err_code Typing.UnknownType) ([(pos, msg)] @ r)

let not_found_hint = function
  | `no_hint -> ""
  | `closest (_pos, v) -> Printf.sprintf " (did you mean static method '%s'?)" v
  | `did_you_mean (_pos, v) -> Printf.sprintf " (did you mean '%s'?)" v

let snot_found_hint = function
  | `no_hint -> ""
  | `closest (_pos, v) ->
    Printf.sprintf " (did you mean instance method '%s'?)" v
  | `did_you_mean (_pos, v) -> Printf.sprintf " (did you mean '%s'?)" v

let string_of_class_member_kind = function
  | `class_constant -> "class constant"
  | `static_method -> "static method"
  | `class_variable -> "class variable"
  | `class_typeconst -> "type constant"

let smember_not_found
    kind
    pos
    (cpos, class_name)
    member_name
    hint
    (on_error : typing_error_callback) =
  let kind = string_of_class_member_kind kind in
  let class_name = strip_ns class_name in
  let msg = Printf.sprintf "No %s '%s' in %s" kind member_name class_name in
  on_error
    ~code:(Typing.err_code Typing.SmemberNotFound)
    [
      (pos, msg ^ snot_found_hint hint);
      (cpos, "Declaration of " ^ class_name ^ " is here");
    ]

let member_not_found
    kind
    pos
    (cpos, type_name)
    member_name
    hint
    reason
    (on_error : typing_error_callback) =
  let type_name = strip_ns type_name in
  let kind =
    match kind with
    | `method_ -> "method"
    | `property -> "property"
  in
  let msg = Printf.sprintf "No %s '%s' in %s" kind member_name type_name in
  on_error
    ~code:(Typing.err_code Typing.MemberNotFound)
    ( (pos, msg ^ not_found_hint hint)
    :: (reason @ [(cpos, "Declaration of " ^ type_name ^ " is here")]) )

let parent_in_trait pos =
  add
    (Typing.err_code Typing.ParentInTrait)
    pos
    ( "parent:: inside a trait is undefined"
    ^ " without 'require extends' of a class defined in <?hh" )

let parent_undefined pos =
  add (Typing.err_code Typing.ParentUndefined) pos "parent is undefined"

let constructor_no_args pos =
  add
    (Typing.err_code Typing.ConstructorNoArgs)
    pos
    "This constructor expects no argument"

let visibility p msg1 p_vis msg2 =
  add_list (Typing.err_code Typing.Visibility) [(p, msg1); (p_vis, msg2)]

let typing_too_many_args expected actual pos pos_def =
  add_list
    (Typing.err_code Typing.TypingTooManyArgs)
    [
      ( pos,
        Printf.sprintf
          "Too many arguments (expected %d but got %d)"
          expected
          actual );
      (pos_def, "Definition is here");
    ]

let typing_too_few_args required actual pos pos_def =
  add_list
    (Typing.err_code Typing.TypingTooFewArgs)
    [
      ( pos,
        Printf.sprintf
          "Too few arguments (required %d but got %d)"
          required
          actual );
      (pos_def, "Definition is here");
    ]

let bad_call pos ty =
  add
    (Typing.err_code Typing.BadCall)
    pos
    ("This call is invalid, this is not a function, it is " ^ ty)

let extend_final extend_pos decl_pos name =
  let name = strip_ns name in
  add_list
    (Typing.err_code Typing.ExtendFinal)
    [
      (extend_pos, "You cannot extend final class " ^ name);
      (decl_pos, "Declaration is here");
    ]

let extend_non_abstract_record name extend_pos decl_pos =
  let name = strip_ns name in
  let msg =
    Printf.sprintf "Cannot extend record `%s` because it isn't abstract" name
  in
  add_list
    (Typing.err_code Typing.ExtendFinal)
    [(extend_pos, msg); (decl_pos, "Declaration is here")]

let extend_sealed child_pos parent_pos parent_name parent_kind verb =
  let name = strip_ns parent_name in
  add_list
    (Typing.err_code Typing.ExtendSealed)
    [
      (child_pos, "You cannot " ^ verb ^ " sealed " ^ parent_kind ^ " " ^ name);
      (parent_pos, "Declaration is here");
    ]

let trait_prop_const_class pos x =
  add
    (Typing.err_code Typing.TraitPropConstClass)
    pos
    ( "Trait declaration of non-const property "
    ^ x
    ^ " is incompatible with a const class" )

let extend_ppl
    child_pos
    child_class_type
    child_is_ppl
    parent_pos
    parent_class_type
    parent_name
    verb =
  let name = strip_ns parent_name in
  let warning =
    if child_is_ppl then
      child_class_type
      ^ " annotated with <<__PPL>> cannot "
      ^ verb
      ^ " non <<__PPL>> "
      ^ parent_class_type
      ^ ": "
      ^ name
    else
      child_class_type
      ^ " must be annotated with <<__PPL>> to "
      ^ verb
      ^ " <<__PPL>> "
      ^ parent_class_type
      ^ ": "
      ^ name
  in
  add_list
    (Typing.err_code Typing.ExtendPPL)
    [(child_pos, warning); (parent_pos, "Declaration is here")]

let read_before_write (pos, v) =
  add
    (Typing.err_code Typing.ReadBeforeWrite)
    pos
    (Utils.sl ["Read access to $this->"; v; " before initialization"])

let final_property pos =
  add
    (Typing.err_code Typing.FinalProperty)
    pos
    "Properties cannot be declared final"

let implement_abstract ~is_final pos1 pos2 kind x =
  let name = "abstract " ^ kind ^ " '" ^ x ^ "'" in
  let msg1 =
    if is_final then
      "This class was declared as final. It must provide an implementation for the "
      ^ name
    else
      "This class must be declared abstract, or provide an implementation for the "
      ^ name
  in
  add_list
    (Typing.err_code Typing.ImplementAbstract)
    [(pos1, msg1); (pos2, "Declaration is here")]

let generic_static pos x =
  add
    (Typing.err_code Typing.GenericStatic)
    pos
    ("This static variable cannot use the type parameter " ^ x ^ ".")

let fun_too_many_args pos1 pos2 (on_error : typing_error_callback) =
  on_error
    ~code:(Typing.err_code Typing.FunTooManyArgs)
    [
      (pos1, "Too many mandatory arguments");
      (pos2, "Because of this definition");
    ]

let fun_too_few_args pos1 pos2 (on_error : typing_error_callback) =
  on_error
    ~code:(Typing.err_code Typing.FunTooFewArgs)
    [(pos1, "Too few arguments"); (pos2, "Because of this definition")]

let fun_unexpected_nonvariadic pos1 pos2 (on_error : typing_error_callback) =
  on_error
    ~code:(Typing.err_code Typing.FunUnexpectedNonvariadic)
    [
      (pos1, "Should have a variadic argument");
      (pos2, "Because of this definition");
    ]

let fun_variadicity_hh_vs_php56 pos1 pos2 (on_error : typing_error_callback) =
  on_error
    ~code:(Typing.err_code Typing.FunVariadicityHhVsPhp56)
    [
      (pos1, "Variadic arguments: ...-style is not a subtype of ...$args");
      (pos2, "Because of this definition");
    ]

let ellipsis_strict_mode ~require pos =
  let msg =
    match require with
    | `Type ->
      "Cannot use ... without a type hint in strict mode. Please add a type hint."
    | `Param_name ->
      "Cannot use ... without a parameter name in strict mode. Please add a parameter name."
    | `Type_and_param_name ->
      "Cannot use ... without a type hint and parameter name in strict mode. Please add a type hint and parameter name."
  in
  add (Typing.err_code Typing.EllipsisStrictMode) pos msg

let untyped_lambda_strict_mode pos =
  let msg =
    "Cannot determine types of lambda parameters in strict mode. Please add type hints on parameters."
  in
  add (Typing.err_code Typing.UntypedLambdaStrictMode) pos msg

let echo_in_reactive_context pos =
  add
    (Typing.err_code Typing.EchoInReactiveContext)
    pos
    "'echo' or 'print' are not allowed in reactive functions."

let expected_tparam
    ~use_pos ~definition_pos n (on_error : typing_error_callback option) =
  let errl =
    [
      ( use_pos,
        "Expected "
        ^
        match n with
        | 0 -> "no type parameter"
        | 1 -> "a type parameter"
        | n -> string_of_int n ^ " type parameters" );
      (definition_pos, "Definition is here");
    ]
  in
  match on_error with
  | None -> add_list (Typing.err_code Typing.ExpectedTparam) errl
  | Some f -> f ~code:(Typing.err_code Typing.ExpectedTparam) errl

let object_string pos1 pos2 =
  add_list
    (Typing.err_code Typing.ObjectString)
    [
      (pos1, "You cannot use this object as a string");
      (pos2, "This object doesn't implement __toString");
    ]

let object_string_deprecated pos =
  add
    (Typing.err_code Typing.ObjectString)
    pos
    "You cannot use this object as a string\nImplicit conversions of Stringish objects to string are deprecated."

let cyclic_typedef p =
  add (Typing.err_code Typing.CyclicTypedef) p "Cyclic typedef"

let type_arity_mismatch pos1 n1 pos2 n2 (on_error : typing_error_callback) =
  on_error
    ~code:(Typing.err_code Typing.TypeArityMismatch)
    [(pos1, "This type has " ^ n1 ^ " arguments"); (pos2, "This one has " ^ n2)]

let this_final id pos2 =
  let n = strip_ns (snd id) in
  let message1 = "Since " ^ n ^ " is not final" in
  let message2 = "this might not be a " ^ n in
  [(fst id, message1); (pos2, message2)]

let exact_class_final id pos2 =
  let n = strip_ns (snd id) in
  let message1 = "This requires the late-bound type to be exactly " ^ n in
  let message2 =
    "Since " ^ n ^ " is not final this might be an instance of a child class"
  in
  [(fst id, message1); (pos2, message2)]

let fun_arity_mismatch pos1 pos2 (on_error : typing_error_callback) =
  on_error
    ~code:(Typing.err_code Typing.FunArityMismatch)
    [
      (pos1, "Number of arguments doesn't match");
      (pos2, "Because of this definition");
    ]

let fun_reactivity_mismatch
    pos1 kind1 pos2 kind2 (on_error : typing_error_callback) =
  let f k = "This function is " ^ k ^ "." in
  on_error
    ~code:(Typing.err_code Typing.FunReactivityMismatch)
    [(pos1, f kind1); (pos2, f kind2)]

let inconsistent_mutability pos1 mut1 p2_opt =
  match p2_opt with
  | Some (pos2, mut2) ->
    add_list
      (Typing.err_code Typing.InconsistentMutability)
      [
        ( pos1,
          "Inconsistent mutability of local variable, here local is " ^ mut1 );
        (pos2, "But here it is " ^ mut2);
      ]
  | None ->
    add
      (Typing.err_code Typing.InconsistentMutability)
      pos1
      ("Local is " ^ mut1 ^ " in one scope and immutable in another.")

let inconsistent_mutability_for_conditional p_mut p_other =
  add_list
    (Typing.err_code Typing.InconsistentMutability)
    [
      ( p_mut,
        "Inconsistent mutability of conditional expression, this branch returns owned mutable value"
      );
      (p_other, "But this one does not.");
    ]

let invalid_mutability_flavor pos mut1 mut2 =
  add
    (Typing.err_code Typing.InvalidMutabilityFlavorInAssignment)
    pos
    ( "Cannot assign "
    ^ mut2
    ^ " value to "
    ^ mut1
    ^ " local variable. Mutability flavor of local variable cannot be altered."
    )

let reassign_mutable_var ~in_collection pos1 =
  let msg =
    if in_collection then
      "This variable is mutable. You cannot create a new reference to it by putting it into the collection."
    else
      "This variable is mutable. You cannot create a new reference to it."
  in
  add (Typing.err_code Typing.ReassignMutableVar) pos1 msg

let reassign_mutable_this ~in_collection ~is_maybe_mutable pos1 =
  let kind =
    if is_maybe_mutable then
      "maybe mutable"
    else
      "mutable"
  in
  let msg =
    if in_collection then
      "$this here is "
      ^ kind
      ^ ". You cannot create a new reference to it by putting it into the collection."
    else
      "$this here is " ^ kind ^ ". You cannot create a new reference to it."
  in
  add (Typing.err_code Typing.ReassignMutableThis) pos1 msg

let mutable_expression_as_multiple_mutable_arguments
    pos param_kind prev_pos prev_param_kind =
  add_list
    (Typing.err_code Typing.MutableExpressionAsMultipleMutableArguments)
    [
      ( pos,
        "A mutable expression may not be passed as multiple arguments where at least one matching parameter is mutable. Matching parameter here is "
        ^ param_kind );
      ( prev_pos,
        "This is where it was used before, being passed as " ^ prev_param_kind
      );
    ]

let reassign_maybe_mutable_var ~in_collection pos1 =
  let msg =
    if in_collection then
      "This variable is maybe mutable. You cannot create a new reference to it by putting it into the collection."
    else
      "This variable is maybe mutable. You cannot create a new reference to it."
  in
  add (Typing.err_code Typing.ReassignMaybeMutableVar) pos1 msg

let mutable_call_on_immutable fpos pos1 rx_mutable_hint_pos =
  let l =
    match rx_mutable_hint_pos with
    | Some p ->
      [
        ( p,
          "Consider wrapping this expression with Rx\\mutable to forward mutability."
        );
      ]
    | None -> []
  in
  let l =
    (pos1, "Cannot call mutable function on immutable expression")
    :: ( fpos,
         "This function is marked <<__Mutable>>, so it has a mutable $this." )
    :: l
  in
  add_list (Typing.err_code Typing.MutableCallOnImmutable) l

let immutable_call_on_mutable fpos pos1 =
  add_list
    (Typing.err_code Typing.ImmutableCallOnMutable)
    [
      (pos1, "Cannot call non-mutable function on mutable expression");
      (fpos, "This function is not marked as <<__Mutable>>.");
    ]

let mutability_mismatch
    ~is_receiver pos1 mut1 pos2 mut2 (on_error : typing_error_callback) =
  let msg mut =
    let msg =
      if is_receiver then
        "Receiver of this function"
      else
        "This parameter"
    in
    msg ^ " is " ^ mut
  in
  on_error
    ~code:(Typing.err_code Typing.MutabilityMismatch)
    [(pos1, "Incompatible mutabilities:"); (pos1, msg mut1); (pos2, msg mut2)]

let invalid_call_on_maybe_mutable ~fun_is_mutable pos fpos =
  let msg =
    "Cannot call "
    ^ ( if fun_is_mutable then
        "mutable"
      else
        "non-mutable" )
    ^ " function on maybe mutable value."
  in
  add_list
    (Typing.err_code Typing.InvalidCallMaybeMutable)
    [(pos, msg); (fpos, "This function is not marked as <<__MaybeMutable>>.")]

let mutable_argument_mismatch param_pos arg_pos =
  add_list
    (Typing.err_code Typing.MutableArgumentMismatch)
    [
      (arg_pos, "Invalid argument");
      (param_pos, "This parameter is marked mutable");
      (arg_pos, "But this expression is not");
    ]

let immutable_argument_mismatch param_pos arg_pos =
  add_list
    (Typing.err_code Typing.ImmutableArgumentMismatch)
    [
      (arg_pos, "Invalid argument");
      (param_pos, "This parameter is not marked as mutable");
      (arg_pos, "But this expression is mutable");
    ]

let mutably_owned_argument_mismatch ~arg_is_owned_local param_pos arg_pos =
  let arg_msg =
    if arg_is_owned_local then
      "Owned mutable locals used as argument should be passed via Rx\\move function"
    else
      "But this expression is not owned mutable"
  in
  add_list
    (Typing.err_code Typing.ImmutableArgumentMismatch)
    [
      (arg_pos, "Invalid argument");
      (param_pos, "This parameter is marked with <<__OwnedMutable>>");
      (arg_pos, arg_msg);
    ]

let maybe_mutable_argument_mismatch param_pos arg_pos =
  add_list
    (Typing.err_code Typing.MaybeMutableArgumentMismatch)
    [
      (arg_pos, "Invalid argument");
      (param_pos, "This parameter is not marked <<__MaybeMutable>>");
      (arg_pos, "But this expression is maybe mutable");
    ]

let invalid_mutable_return_result error_pos function_pos value_kind =
  add_list
    (Typing.err_code Typing.InvalidMutableReturnResult)
    [
      ( error_pos,
        "Functions marked <<__MutableReturn>> must return mutably owned values: mutably owned local variables and results of calling Rx\\mutable."
      );
      (function_pos, "This function is marked <<__MutableReturn>>");
      (error_pos, "This expression is " ^ value_kind);
    ]

let freeze_in_nonreactive_context pos1 =
  add
    (Typing.err_code Typing.FreezeInNonreactiveContext)
    pos1
    "\\HH\\Rx\\freeze can only be used in reactive functions"

let mutable_in_nonreactive_context pos =
  add
    (Typing.err_code Typing.MutableInNonreactiveContext)
    pos
    "\\HH\\Rx\\mutable can only be used in reactive functions"

let move_in_nonreactive_context pos =
  add
    (Typing.err_code Typing.MoveInNonreactiveContext)
    pos
    "\\HH\\Rx\\move can only be used in reactive functions"

let invalid_argument_type_for_condition_in_rx
    ~is_receiver f_pos def_pos arg_pos expected_type actual_type =
  let arg_msg =
    if is_receiver then
      "Receiver type"
    else
      "Argument type"
  in
  let arg_msg =
    arg_msg
    ^ " must be a subtype of "
    ^ expected_type
    ^ ", now "
    ^ actual_type
    ^ "."
  in
  add_list
    (Typing.err_code Typing.InvalidConditionallyReactiveCall)
    [
      ( f_pos,
        "Cannot invoke conditionally reactive function in reactive context, because at least one reactivity condition is not met."
      );
      (arg_pos, arg_msg);
      (def_pos, "This is the function declaration");
    ]

let callsite_reactivity_mismatch
    f_pos def_pos callee_reactivity cause_pos_opt caller_reactivity =
  add_list
    (Typing.err_code Typing.CallSiteReactivityMismatch)
    ( [
        ( f_pos,
          "Reactivity mismatch: "
          ^ caller_reactivity
          ^ " function cannot call "
          ^ callee_reactivity
          ^ " function." );
        (def_pos, "This is declaration of the function being called.");
      ]
    @ Option.value_map cause_pos_opt ~default:[] ~f:(fun cause_pos ->
          [
            ( cause_pos,
              "Reactivity of this argument was used as reactivity of the callee."
            );
          ]) )

let invalid_argument_of_rx_mutable_function pos =
  add
    (Typing.err_code Typing.InvalidArgumentOfRxMutableFunction)
    pos
    "Single argument to \\HH\\Rx\\mutable should be an expression that yields new mutably-owned value, like 'new A()', Hack collection literal or 'f()' where f is function annotated with <<__MutableReturn>> attribute."

let invalid_freeze_use pos1 =
  add
    (Typing.err_code Typing.InvalidFreezeUse)
    pos1
    "freeze takes a single mutably-owned local variable as an argument"

let invalid_move_use pos1 =
  add
    (Typing.err_code Typing.InvalidMoveUse)
    pos1
    "move takes a single mutably-owned local variable as an argument"

let require_args_reify def_pos arg_pos =
  add_list
    (Typing.err_code Typing.RequireArgsReify)
    [
      ( arg_pos,
        "All type arguments must be specified because a type parameter is reified"
      );
      (def_pos, "Definition is here");
    ]

let require_generic_explicit (def_pos, def_name) arg_pos =
  add_list
    (Typing.err_code Typing.RequireGenericExplicit)
    [
      ( arg_pos,
        "Generic type parameter " ^ def_name ^ " must be specified explicitly"
      );
      (def_pos, "Definition is here");
    ]

let invalid_reified_argument (def_pos, def_name) hint_pos arg_pos arg_kind =
  add_list
    (Typing.err_code Typing.InvalidReifiedArgument)
    [
      (hint_pos, "Invalid reified hint");
      ( arg_pos,
        "This is " ^ arg_kind ^ ", it cannot be used as a reified type argument"
      );
      (def_pos, def_name ^ " is reified");
    ]

let invalid_reified_argument_reifiable (def_pos, def_name) arg_pos ty_pos ty_msg
    =
  add_list
    (Typing.err_code Typing.InvalidReifiedArgument)
    [
      (arg_pos, "PHP arrays cannot be used as a reified type argument");
      (ty_pos, String.capitalize ty_msg);
      (def_pos, def_name ^ " is reified");
    ]

let new_static_class_reified pos =
  add
    (Typing.err_code Typing.NewStaticClassReified)
    pos
    "Cannot call new static because the current class has reified generics"

let class_get_reified pos =
  add
    (Typing.err_code Typing.ClassGetReified)
    pos
    "Cannot access static properties on reified generics"

let static_call_with_class_level_reified_generic call_pos generic_pos =
  add_list
    (Typing.err_code Typing.StaticCallWithClassLevelReifiedGeneric)
    [
      ( call_pos,
        "Static methods cannot use generics reified at the class level. Try reifying them at the static method itself."
      );
      (generic_pos, "Class-level reified generic used here.");
    ]

let consistent_construct_reified pos =
  add
    (Typing.err_code Typing.ConsistentConstructReified)
    pos
    "This class or one of its ancestors is annotated with <<__ConsistentConstruct>>. It cannot have reified generics."

let bad_function_pointer_construction pos =
  add
    (Typing.err_code Typing.BadFunctionPointerConstruction)
    pos
    "Function pointers must be explicitly named"

let reified_generics_not_allowed pos =
  add
    (Typing.err_code Typing.InvalidReifiedFunctionPointer)
    pos
    "Creating function pointers with reified generics is not currently allowed"

let new_without_newable pos name =
  add
    (Typing.err_code Typing.NewWithoutNewable)
    pos
    ( name
    ^ " cannot be used with `new` because it does not have the <<__Newable>> attribute"
    )

let invalid_freeze_target pos1 var_pos var_mutability_str =
  add_list
    (Typing.err_code Typing.InvalidFreezeTarget)
    [
      (pos1, "Invalid argument - freeze() takes a single mutable variable");
      (var_pos, "This variable is " ^ var_mutability_str);
    ]

let invalid_move_target pos1 var_pos var_mutability_str =
  add_list
    (Typing.err_code Typing.InvalidMoveTarget)
    [
      (pos1, "Invalid argument - move() takes a single mutably-owned variable");
      (var_pos, "This variable is " ^ var_mutability_str);
    ]

let discarded_awaitable pos1 pos2 =
  add_list
    (Typing.err_code Typing.DiscardedAwaitable)
    [
      ( pos1,
        "This expression is of type Awaitable, but it's "
        ^ "either being discarded or used in a dangerous way before "
        ^ "being awaited" );
      (pos2, "This is why I think it is Awaitable");
    ]

let unify_error ?code errl =
  add_list (Option.value code ~default:(Typing.err_code Typing.UnifyError)) errl

let unify_error_at pos ?code errl =
  unify_error ?code ((pos, "Typing error") :: errl)

let maybe_unify_error specific_code ?code errl =
  add_list (Option.value code ~default:(Typing.err_code specific_code)) errl

let index_type_mismatch = maybe_unify_error Typing.IndexTypeMismatch

let expected_stringlike = maybe_unify_error Typing.ExpectedStringlike

let type_constant_mismatch (on_error : typing_error_callback) ?code errl =
  let code =
    Option.value code ~default:(Typing.err_code Typing.TypeConstantMismatch)
  in
  on_error ~code errl

let class_constant_type_mismatch (on_error : typing_error_callback) ?code errl =
  let code =
    Option.value
      code
      ~default:(Typing.err_code Typing.ClassConstantTypeMismatch)
  in
  on_error ~code errl

let constant_does_not_match_enum_type =
  maybe_unify_error Typing.ConstantDoesNotMatchEnumType

let enum_underlying_type_must_be_arraykey =
  maybe_unify_error Typing.EnumUnderlyingTypeMustBeArraykey

let enum_constraint_must_be_arraykey =
  maybe_unify_error Typing.EnumConstraintMustBeArraykey

let enum_subtype_must_have_compatible_constraint =
  maybe_unify_error Typing.EnumSubtypeMustHaveCompatibleConstraint

let parameter_default_value_wrong_type =
  maybe_unify_error Typing.ParameterDefaultValueWrongType

let newtype_alias_must_satisfy_constraint =
  maybe_unify_error Typing.NewtypeAliasMustSatisfyConstraint

let bad_function_typevar = maybe_unify_error Typing.BadFunctionTypevar

let bad_class_typevar = maybe_unify_error Typing.BadClassTypevar

let bad_method_typevar = maybe_unify_error Typing.BadMethodTypevar

let missing_return = maybe_unify_error Typing.MissingReturnInNonVoidFunction

let inout_return_type_mismatch =
  maybe_unify_error Typing.InoutReturnTypeMismatch

let class_constant_value_does_not_match_hint =
  maybe_unify_error Typing.ClassConstantValueDoesNotMatchHint

let class_property_initializer_type_does_not_match_hint =
  maybe_unify_error Typing.ClassPropertyInitializerTypeDoesNotMatchHint

let xhp_attribute_does_not_match_hint =
  maybe_unify_error Typing.XhpAttributeValueDoesNotMatchHint

let pocket_universes_typing = maybe_unify_error Typing.PocketUniversesTyping

let record_init_value_does_not_match_hint =
  maybe_unify_error Typing.RecordInitValueDoesNotMatchHint

let elt_type_to_string = function
  | `Method -> "method"
  | `Property -> "property"

let static_redeclared_as_dynamic
    dyn_position static_position member_name ~elt_type =
  let dollar =
    match elt_type with
    | `Property -> "$"
    | _ -> ""
  in
  let elt_type = elt_type_to_string elt_type in
  let msg_dynamic =
    "The "
    ^ elt_type
    ^ " "
    ^ dollar
    ^ member_name
    ^ " is declared here as non-static"
  in
  let msg_static =
    "But it conflicts with an inherited static declaration here"
  in
  add_list
    (Typing.err_code Typing.StaticDynamic)
    [(dyn_position, msg_dynamic); (static_position, msg_static)]

let dynamic_redeclared_as_static
    static_position dyn_position member_name ~elt_type =
  let dollar =
    match elt_type with
    | `Property -> "$"
    | _ -> ""
  in
  let elt_type = elt_type_to_string elt_type in
  let msg_static =
    "The "
    ^ elt_type
    ^ " "
    ^ dollar
    ^ member_name
    ^ " is declared here as static"
  in
  let msg_dynamic =
    "But it conflicts with an inherited non-static declaration here"
  in
  add_list
    (Typing.err_code Typing.StaticDynamic)
    [(static_position, msg_static); (dyn_position, msg_dynamic)]

let null_member ~is_method s pos r =
  let msg =
    Printf.sprintf
      "You are trying to access the %s '%s' but this object can be null."
      ( if is_method then
        "method"
      else
        "property" )
      s
  in
  add_list (Typing.err_code Typing.NullMember) ([(pos, msg)] @ r)

(* Trying to access a member on a mixed or nonnull value. *)
let top_member ~is_method ~is_nullable s pos1 ty pos2 =
  let msg =
    Printf.sprintf
      "You are trying to access the %s '%s' but this is %s. Use a specific class or interface name."
      ( if is_method then
        "method"
      else
        "property" )
      s
      ty
  in
  add_list
    (Typing.err_code
       ( if is_nullable then
         Typing.NullMember
       else
         Typing.NonObjectMember ))
    [(pos1, msg); (pos2, "Definition is here")]

let non_object_member
    ~is_method s pos1 ty pos2 (on_error : typing_error_callback) =
  let msg_start =
    Printf.sprintf
      "You are trying to access the %s '%s' but this is %s"
      ( if is_method then
        "method"
      else
        "property" )
      s
      ty
  in
  let msg =
    if ty = "a shape" then
      msg_start ^ ". Did you mean $foo['" ^ s ^ "'] instead?"
    else
      msg_start
  in
  on_error
    ~code:(Typing.err_code Typing.NonObjectMember)
    [(pos1, msg); (pos2, "Definition is here")]

let unknown_object_member ~is_method s pos r =
  let msg =
    Printf.sprintf
      "You are trying to access the %s '%s' on a value whose class is unknown."
      ( if is_method then
        "method"
      else
        "property" )
      s
  in
  add_list (Typing.err_code Typing.UnknownObjectMember) ([(pos, msg)] @ r)

let non_class_member ~is_method s pos1 ty pos2 =
  let msg =
    Printf.sprintf
      "You are trying to access the static %s '%s' but this is %s"
      ( if is_method then
        "method"
      else
        "property" )
      s
      ty
  in
  add_list
    (Typing.err_code Typing.NonClassMember)
    [(pos1, msg); (pos2, "Definition is here")]

let ambiguous_member ~is_method s pos1 ty pos2 =
  let msg =
    Printf.sprintf
      "You are trying to access the %s '%s' but there is more than one implementation on %s"
      ( if is_method then
        "method"
      else
        "property" )
      s
      ty
  in
  add_list
    (Typing.err_code Typing.AmbiguousMember)
    [(pos1, msg); (pos2, "Definition is here")]

let null_container p null_witness =
  add_list
    (Typing.err_code Typing.NullContainer)
    ( [
        ( p,
          "You are trying to access an element of this container"
          ^ " but the container could be null. " );
      ]
    @ null_witness )

let option_mixed pos =
  add
    (Typing.err_code Typing.OptionMixed)
    pos
    "?mixed is a redundant typehint - just use mixed"

let option_null pos =
  add
    (Typing.err_code Typing.OptionNull)
    pos
    "?null is a redundant typehint - just use null"

let declared_covariant pos1 pos2 emsg =
  add_list
    (Typing.err_code Typing.DeclaredCovariant)
    ( [
        (pos2, "Illegal usage of a covariant type parameter");
        (pos1, "This is where the parameter was declared as covariant (+)");
      ]
    @ emsg )

let declared_contravariant pos1 pos2 emsg =
  add_list
    (Typing.err_code Typing.DeclaredContravariant)
    ( [
        (pos2, "Illegal usage of a contravariant type parameter");
        (pos1, "This is where the parameter was declared as contravariant (-)");
      ]
    @ emsg )

let static_property_type_generic_param ~class_pos ~var_type_pos ~generic_pos =
  add_list
    (Typing.err_code Typing.ClassVarTypeGenericParam)
    [
      ( generic_pos,
        "A generic parameter cannot be used in the type of a static property" );
      ( var_type_pos,
        "This is where the type of the static property was declared" );
      (class_pos, "This is the class containing the static property");
    ]

let contravariant_this pos class_name tp =
  add
    (Typing.err_code Typing.ContravariantThis)
    pos
    ( "The \"this\" type cannot be used in this "
    ^ "contravariant position because its enclosing class \""
    ^ class_name
    ^ "\" "
    ^ "is final and has a variant type parameter \""
    ^ tp
    ^ "\"" )

let cyclic_typeconst pos sl =
  let sl = List.map sl strip_ns in
  add
    (Typing.err_code Typing.CyclicTypeconst)
    pos
    ("Cyclic type constant:\n  " ^ String.concat ~sep:" -> " sl)

let abstract_concrete_override pos parent_pos kind =
  let kind_str =
    match kind with
    | `method_ -> "method"
    | `typeconst -> "type constant"
    | `constant -> "constant"
    | `property -> "property"
  in
  add_list
    (Typing.err_code Typing.AbstractConcreteOverride)
    [
      (pos, "Cannot re-declare this " ^ kind_str ^ " as abstract");
      (parent_pos, "Previously defined here");
    ]

let required_field_is_optional pos1 pos2 name (on_error : typing_error_callback)
    =
  on_error
    ~code:(Typing.err_code Typing.RequiredFieldIsOptional)
    [
      (pos1, "The field '" ^ name ^ "' is optional");
      (pos2, "The field '" ^ name ^ "' is defined as required");
    ]

let array_get_with_optional_field pos1 pos2 name =
  add_list
    (Typing.err_code Typing.ArrayGetWithOptionalField)
    [
      ( pos1,
        Printf.sprintf
          "The field `%s` may not be present in this shape. Use `Shapes::idx()` instead."
          name );
      (pos2, "This is where the field was declared as optional.");
    ]

let non_call_argument_in_suspend pos msgs =
  add_list
    (Typing.err_code Typing.NonCallArgumentInSuspend)
    ( [(pos, "'suspend' operator expects call to a coroutine as an argument.")]
    @ msgs )

let non_coroutine_call_in_suspend pos msgs =
  add_list
    (Typing.err_code Typing.NonCoroutineCallInSuspend)
    ( [
        ( pos,
          "Only coroutine functions are allowed to be called in 'suspend' operator."
        );
      ]
    @ msgs )

let coroutine_call_outside_of_suspend pos =
  add_list
    (Typing.err_code Typing.CoroutineCallOutsideOfSuspend)
    [
      ( pos,
        "Coroutine calls are only allowed when they are arguments to 'suspend' operator"
      );
    ]

let function_is_not_coroutine pos name =
  add_list
    (Typing.err_code Typing.FunctionIsNotCoroutine)
    [
      ( pos,
        "Function '"
        ^ name
        ^ "' is not a coroutine and cannot be used in as an argument of 'suspend' operator."
      );
    ]

let coroutinness_mismatch
    pos1_is_coroutine pos1 pos2 (on_error : typing_error_callback) =
  let m1 = "This is a coroutine." in
  let m2 = "This is not a coroutine." in
  on_error
    ~code:(Typing.err_code Typing.CoroutinnessMismatch)
    [
      ( pos1,
        if pos1_is_coroutine then
          m1
        else
          m2 );
      ( pos2,
        if pos1_is_coroutine then
          m2
        else
          m1 );
    ]

let invalid_ppl_call pos context =
  let error_msg =
    "Cannot call a method on an object of a <<__PPL>> class " ^ context
  in
  add (Typing.err_code Typing.InvalidPPLCall) pos error_msg

let invalid_ppl_static_call pos reason =
  let error_msg =
    "Cannot call a static method on a <<__PPL>> class " ^ reason
  in
  add (Typing.err_code Typing.InvalidPPLStaticCall) pos error_msg

let ppl_meth_pointer pos func =
  let error_msg = func ^ " cannot be used with a <<__PPL>> class" in
  add (Typing.err_code Typing.PPLMethPointer) pos error_msg

let coroutine_outside_experimental pos =
  add
    (Typing.err_code Typing.CoroutineOutsideExperimental)
    pos
    Coroutine_errors.error_message

let return_disposable_mismatch
    pos1_return_disposable pos1 pos2 (on_error : typing_error_callback) =
  let m1 = "This is marked <<__ReturnDisposable>>." in
  let m2 = "This is not marked <<__ReturnDisposable>>." in
  on_error
    ~code:(Typing.err_code Typing.ReturnDisposableMismatch)
    [
      ( pos1,
        if pos1_return_disposable then
          m1
        else
          m2 );
      ( pos2,
        if pos1_return_disposable then
          m2
        else
          m1 );
    ]

let return_void_to_rx_mismatch
    ~pos1_has_attribute pos1 pos2 (on_error : typing_error_callback) =
  let m1 = "This is marked <<__ReturnsVoidToRx>>." in
  let m2 = "This is not marked <<__ReturnsVoidToRx>>." in
  on_error
    ~code:(Typing.err_code Typing.ReturnVoidToRxMismatch)
    [
      ( pos1,
        if pos1_has_attribute then
          m1
        else
          m2 );
      ( pos2,
        if pos1_has_attribute then
          m2
        else
          m1 );
    ]

let this_as_lexical_variable pos =
  add
    (Naming.err_code Naming.ThisAsLexicalVariable)
    pos
    "Cannot use $this as lexical variable"

let dollardollar_lvalue pos =
  add
    (Typing.err_code Typing.DollardollarLvalue)
    pos
    "Cannot assign a value to the special pipe variable ($$)"

let mutating_const_property pos =
  add
    (Typing.err_code Typing.AssigningToConst)
    pos
    "Cannot mutate a __Const property"

let self_const_parent_not pos =
  add
    (Typing.err_code Typing.SelfConstParentNot)
    pos
    "A __Const class may only extend other __Const classes"

let overriding_prop_const_mismatch
    parent_pos
    parent_const
    child_pos
    child_const
    (on_error : typing_error_callback) =
  let m1 = "This property is __Const" in
  let m2 = "This property is not __Const" in
  on_error
    ~code:(Typing.err_code Typing.OverridingPropConstMismatch)
    [
      ( parent_pos,
        if parent_const then
          m1
        else
          m2 );
      ( child_pos,
        if child_const then
          m1
        else
          m2 );
    ]

let mutable_return_result_mismatch
    pos1_has_mutable_return pos1 pos2 (on_error : typing_error_callback) =
  let m1 = "This is marked <<__MutableReturn>>." in
  let m2 = "This is not marked <<__MutableReturn>>." in
  on_error
    ~code:(Typing.err_code Typing.MutableReturnResultMismatch)
    [
      ( pos1,
        if pos1_has_mutable_return then
          m1
        else
          m2 );
      ( pos2,
        if pos1_has_mutable_return then
          m2
        else
          m1 );
    ]

let pu_expansion pos =
  add
    (Typing.err_code Typing.PocketUniversesExpansion)
    pos
    "[PocketUniverses] Type expansion is not supported."

let pu_typing pos kind msg =
  add
    (Typing.err_code Typing.PocketUniversesTyping)
    pos
    (sprintf "Unexpected Pocket Universes %s %s during typing." kind msg)

let pu_typing_not_supported pos =
  add
    (Typing.err_code Typing.PocketUniversesTyping)
    pos
    "Unsupported Pocket Universes member."

let php_lambda_disallowed pos =
  add
    (NastCheck.err_code NastCheck.PhpLambdaDisallowed)
    pos
    "PHP style anonymous functions are not allowed."

(*****************************************************************************)
(* Typing decl errors *)
(*****************************************************************************)

let wrong_extend_kind child_pos child parent_pos parent =
  let msg1 = (child_pos, child ^ " cannot extend " ^ parent) in
  let msg2 = (parent_pos, "This is " ^ parent) in
  add_list (Typing.err_code Typing.WrongExtendKind) [msg1; msg2]

let unsatisfied_req parent_pos req_name req_pos =
  let s1 = "Failure to satisfy requirement: " ^ strip_ns req_name in
  let s2 = "Required here" in
  if req_pos = parent_pos then
    add (Typing.err_code Typing.UnsatisfiedReq) parent_pos s1
  else
    add_list
      (Typing.err_code Typing.UnsatisfiedReq)
      [(parent_pos, s1); (req_pos, s2)]

let cyclic_class_def stack pos =
  let stack = SSet.fold ~f:(fun x y -> strip_ns x ^ " " ^ y) stack ~init:"" in
  add
    (Typing.err_code Typing.CyclicClassDef)
    pos
    ("Cyclic class definition : " ^ stack)

let cyclic_record_def names pos =
  let names = List.map ~f:strip_ns names in
  add
    (Typing.err_code Typing.CyclicRecordDef)
    pos
    (Printf.sprintf
       "Record inheritance cycle: %s"
       (String.concat ~sep:" " names))

let trait_reuse p_pos p_name class_name trait =
  let (c_pos, c_name) = class_name in
  let c_name = strip_ns c_name in
  let trait = strip_ns trait in
  let err =
    "Class " ^ c_name ^ " reuses trait " ^ trait ^ " in its hierarchy"
  in
  let err' = "It is already used through " ^ strip_ns p_name in
  add_list (Typing.err_code Typing.TraitReuse) [(c_pos, err); (p_pos, err')]

let trait_reuse_inside_class class_name trait occurrences =
  let (c_pos, c_name) = class_name in
  let c_name = strip_ns c_name in
  let trait = strip_ns trait in
  let err = "Class " ^ c_name ^ " uses trait " ^ trait ^ " multiple times" in
  add_list
    (Typing.err_code Typing.TraitReuseInsideClass)
    ([(c_pos, err)] @ List.map ~f:(fun p -> (p, "used here")) occurrences)

let invalid_is_as_expression_hint op hint_pos ty_pos ty_str =
  add_list
    (Typing.err_code Typing.InvalidIsAsExpressionHint)
    [
      (hint_pos, "Invalid \"" ^ op ^ "\" expression hint");
      (ty_pos, "The \"" ^ op ^ "\" operator cannot be used with " ^ ty_str);
    ]

let invalid_enforceable_type kind_str (tp_pos, tp_name) targ_pos ty_pos ty_str =
  add_list
    (Typing.err_code Typing.InvalidEnforceableTypeArgument)
    [
      (targ_pos, "Invalid type");
      ( tp_pos,
        "Type " ^ kind_str ^ " " ^ tp_name ^ " was declared __Enforceable here"
      );
      (ty_pos, "This type is not enforceable because it has " ^ ty_str);
    ]

let reifiable_attr attr_pos decl_kind decl_pos ty_pos ty_msg =
  add_list
    (Typing.err_code Typing.DisallowPHPArraysAttr)
    [
      (decl_pos, "Invalid " ^ decl_kind);
      (attr_pos, "This type constant has the __Reifiable attribute");
      (ty_pos, "It cannot contain " ^ ty_msg);
    ]

let invalid_newable_type_argument (tp_pos, tp_name) ta_pos =
  add_list
    (Typing.err_code Typing.InvalidNewableTypeArgument)
    [
      ( ta_pos,
        "A newable type argument must be a concrete class or a newable type parameter."
      );
      (tp_pos, "Type parameter " ^ tp_name ^ " was declared __Newable here");
    ]

let invalid_newable_type_param_constraints
    (tparam_pos, tparam_name) constraint_list =
  let partial =
    if List.is_empty constraint_list then
      "No constraints"
    else
      "The constraints "
      ^ String.concat ~sep:", " (List.map ~f:strip_ns constraint_list)
  in
  let msg =
    "The type parameter "
    ^ tparam_name
    ^ " has the <<__Newable>> attribute. Newable type parameters must be constrained with `as`, and exactly one of those constraints must be a valid newable class. The class must either be final, or it must have the <<__ConsistentConstruct>> attribute or extend a class that has it. "
    ^ partial
    ^ " are valid newable classes"
  in
  add (Typing.err_code Typing.InvalidNewableTypeParamConstraints) tparam_pos msg

let override_final ~parent ~child ~(on_error : typing_error_callback option) =
  let msg1 = (child, "You cannot override this method") in
  let msg2 = (parent, "It was declared as final") in
  match on_error with
  | Some on_error ->
    on_error ~code:(Typing.err_code Typing.OverrideFinal) [msg1; msg2]
  | None -> add_list (Typing.err_code Typing.OverrideFinal) [msg1; msg2]

let override_memoizelsb ~parent ~child =
  add_list
    (Typing.err_code Typing.OverrideMemoizeLSB)
    [
      ( child,
        "__MemoizeLSB method may not be an override (temporary due to HHVM bug)"
      );
      (parent, "This method is being overridden");
    ]

let override_lsb ~member_name ~parent ~child =
  add_list
    (Typing.err_code Typing.OverrideLSB)
    [
      ( child,
        "Member " ^ member_name ^ " may not override __LSB member of parent" );
      (parent, "This is being overridden");
    ]

let should_be_override pos class_id id =
  add
    (Typing.err_code Typing.ShouldBeOverride)
    pos
    (Printf.sprintf
       "%s has no parent class with a method `%s` to override"
       (strip_ns class_id)
       id)

let override_per_trait class_name id m_pos =
  let (c_pos, c_name) = class_name in
  let err_msg =
    "Method "
    ^ strip_ns c_name
    ^ "::"
    ^ id
    ^ " should be an override per the declaring trait; no non-private parent definition found or overridden parent is defined in non-<?hh code"
  in
  add_list
    (Typing.err_code Typing.OverridePerTrait)
    [(c_pos, err_msg); (m_pos, "Declaration of " ^ id ^ "() is here")]

let missing_assign pos =
  add (Typing.err_code Typing.MissingAssign) pos "Please assign a value"

let private_override pos class_id id =
  add
    (Typing.err_code Typing.PrivateOverride)
    pos
    ( strip_ns class_id
    ^ "::"
    ^ id
    ^ ": combining private and override is nonsensical" )

let invalid_memoized_param pos ty_reason_msg =
  add_list
    (Typing.err_code Typing.InvalidMemoizedParam)
    ( ( pos,
        "Parameters to memoized function must be null, bool, int, float, string, an object deriving IMemoizeParam, or a Container thereof. See also http://docs.hhvm.com/hack/attributes/special#__memoize"
      )
    :: ty_reason_msg )

let invalid_disposable_hint pos class_name =
  add
    (Typing.err_code Typing.InvalidDisposableHint)
    pos
    ( "Parameter with type '"
    ^ class_name
    ^ "' must not implement IDisposable or IAsyncDisposable. Please use <<__AcceptDisposable>> attribute or create disposable object with 'using' statement instead."
    )

let invalid_disposable_return_hint pos class_name =
  add
    (Typing.err_code Typing.InvalidDisposableReturnHint)
    pos
    ( "Return type '"
    ^ class_name
    ^ "' must not implement IDisposable or IAsyncDisposable. Please add <<__ReturnDisposable>> attribute."
    )

let xhp_required pos why_xhp ty_reason_msg =
  let msg = "An XHP instance was expected" in
  add_list
    (Typing.err_code Typing.XhpRequired)
    ((pos, msg) :: (pos, why_xhp) :: ty_reason_msg)

let illegal_xhp_child pos ty_reason_msg =
  let msg = "XHP children must be compatible with XHPChild" in
  add_list (Typing.err_code Typing.IllegalXhpChild) ((pos, msg) :: ty_reason_msg)

let missing_xhp_required_attr pos attr ty_reason_msg =
  let msg = "Required attribute " ^ attr ^ " is missing." in
  add_list
    (Typing.err_code Typing.MissingXhpRequiredAttr)
    ((pos, msg) :: ty_reason_msg)

let nullsafe_not_needed p nonnull_witness =
  add_list
    (Typing.err_code Typing.NullsafeNotNeeded)
    ( [(p, "You are using the ?-> operator but this object cannot be null. ")]
    @ nonnull_witness )

let generic_at_runtime p prefix =
  add
    (Typing.err_code Typing.ErasedGenericAtRuntime)
    p
    ( prefix
    ^ " generics can only be used in type hints because they do not exist at runtime."
    )

let generics_not_allowed p =
  add
    (Typing.err_code Typing.GenericsNotAllowed)
    p
    "Generics are not allowed in this position."

let trivial_strict_eq p b left right left_trail right_trail =
  let msg = "This expression is always " ^ b in
  let left_trail = List.map left_trail typedef_trail_entry in
  let right_trail = List.map right_trail typedef_trail_entry in
  add_list
    (Typing.err_code Typing.TrivialStrictEq)
    (((p, msg) :: left) @ left_trail @ right @ right_trail)

let trivial_strict_not_nullable_compare_null p result type_reason =
  let msg = "This expression is always " ^ result in
  add_list
    (Typing.err_code Typing.NotNullableCompareNullTrivial)
    ((p, msg) :: type_reason)

let eq_incompatible_types p left right =
  let msg = "This equality test has incompatible types" in
  add_list
    (Typing.err_code Typing.EqIncompatibleTypes)
    (((p, msg) :: left) @ right)

let comparison_invalid_types p left right =
  let msg =
    "This comparison has invalid types.  Only comparisons in which both arguments are strings, nums, DateTime, or DateTimeImmutable are allowed"
  in
  add_list
    (Typing.err_code Typing.ComparisonInvalidTypes)
    (((p, msg) :: left) @ right)

let void_usage p void_witness =
  let msg = "You are using the return value of a void function" in
  add_list (Typing.err_code Typing.VoidUsage) ((p, msg) :: void_witness)

let noreturn_usage p noreturn_witness =
  let msg = "You are using the return value of a noreturn function" in
  add_list (Typing.err_code Typing.NoreturnUsage) ((p, msg) :: noreturn_witness)

let attribute_too_few_arguments pos x n =
  let n = string_of_int n in
  add
    (Typing.err_code Typing.AttributeTooFewArguments)
    pos
    ("The attribute " ^ x ^ " expects at least " ^ n ^ " arguments")

let attribute_too_many_arguments pos x n =
  let n = string_of_int n in
  add
    (Typing.err_code Typing.AttributeTooManyArguments)
    pos
    ("The attribute " ^ x ^ " expects at most " ^ n ^ " arguments")

let attribute_param_type pos x =
  add
    (Typing.err_code Typing.AttributeParamType)
    pos
    ("This attribute parameter should be " ^ x)

let deprecated_use pos pos_def msg =
  add_list
    (Typing.err_code Typing.DeprecatedUse)
    [(pos, msg); (pos_def, "Definition is here")]

let cannot_declare_constant kind pos (class_pos, class_name) =
  let kind_str =
    match kind with
    | `enum -> "an enum"
    | `trait -> "a trait"
    | `record -> "a record"
  in
  add_list
    (Typing.err_code Typing.CannotDeclareConstant)
    [
      (pos, "Cannot declare a constant in " ^ kind_str);
      (class_pos, strip_ns class_name ^ " was defined as " ^ kind_str ^ " here");
    ]

let ambiguous_inheritance
    pos class_ origin (error : error) (on_error : typing_error_callback) =
  let origin = strip_ns origin in
  let class_ = strip_ns class_ in
  let message =
    "This declaration was inherited from an object of type "
    ^ origin
    ^ ". Redeclare this member in "
    ^ class_
    ^ " with a compatible signature."
  in
  let (code, msgl) = (get_code error, to_list error) in
  on_error ~code (msgl @ [(pos, message)])

let multiple_concrete_defs
    child_pos
    parent_pos
    child_origin
    parent_origin
    name
    class_
    (on_error : typing_error_callback) =
  let child_origin = strip_ns child_origin in
  let parent_origin = strip_ns parent_origin in
  let class_ = strip_ns class_ in
  on_error
    ~code:(Typing.err_code Typing.MultipleConcreteDefs)
    [
      ( child_pos,
        child_origin
        ^ " and "
        ^ parent_origin
        ^ " both declare ambiguous implementations of "
        ^ name
        ^ "." );
      (child_pos, child_origin ^ "'s definition is here.");
      (parent_pos, parent_origin ^ "'s definition is here.");
      ( child_pos,
        "Redeclare " ^ name ^ " in " ^ class_ ^ " with a compatible signature."
      );
    ]

let local_variable_modified_and_used pos_modified pos_used_l =
  let used_msg p = (p, "And accessed here") in
  add_list
    (Typing.err_code Typing.LocalVariableModifedAndUsed)
    ( ( pos_modified,
        "Unsequenced modification and access to local variable. Modified here"
      )
    :: List.map pos_used_l used_msg )

let local_variable_modified_twice pos_modified pos_modified_l =
  let modified_msg p = (p, "And also modified here") in
  add_list
    (Typing.err_code Typing.LocalVariableModifedTwice)
    ( ( pos_modified,
        "Unsequenced modifications to local variable. Modified here" )
    :: List.map pos_modified_l modified_msg )

let assign_during_case p =
  add
    (Typing.err_code Typing.AssignDuringCase)
    p
    "Don't assign to variables inside of case labels"

let cyclic_enum_constraint pos =
  add (Typing.err_code Typing.CyclicEnumConstraint) pos "Cyclic enum constraint"

let invalid_classname p =
  add (Typing.err_code Typing.InvalidClassname) p "Not a valid class name"

let illegal_type_structure pos errmsg =
  let msg =
    "The two arguments to type_structure() must be:"
    ^ "\n - first: ValidClassname::class or an object of that class"
    ^ "\n - second: a single-quoted string literal containing the name"
    ^ " of a type constant of that class"
    ^ "\n"
    ^ errmsg
  in
  add (Typing.err_code Typing.IllegalTypeStructure) pos msg

let illegal_typeconst_direct_access pos =
  let msg =
    "Type constants cannot be directly accessed. "
    ^ "Use type_structure(ValidClassname::class, 'TypeConstName') instead"
  in
  add (Typing.err_code Typing.IllegalTypeStructure) pos msg

let override_no_default_typeconst pos_child pos_parent =
  add_list
    (Typing.err_code Typing.OverrideNoDefaultTypeconst)
    [
      (pos_child, "This abstract type constant does not have a default type");
      ( pos_parent,
        "It cannot override an abstract type constant that has a default type"
      );
    ]

let inout_annotation_missing pos1 pos2 =
  let msg1 = (pos1, "This argument should be annotated with 'inout'") in
  let msg2 = (pos2, "Because this is an inout parameter") in
  add_list (Typing.err_code Typing.InoutAnnotationMissing) [msg1; msg2]

let inout_annotation_unexpected pos1 pos2 pos2_is_variadic =
  let msg1 = (pos1, "Unexpected inout annotation for argument") in
  let msg2 =
    ( pos2,
      if pos2_is_variadic then
        "A variadic parameter can never be inout"
      else
        "This is a normal parameter (does not have 'inout')" )
  in
  add_list (Typing.err_code Typing.InoutAnnotationUnexpected) [msg1; msg2]

let inoutness_mismatch pos1 pos2 (on_error : typing_error_callback) =
  let msg1 = (pos1, "This is an inout parameter") in
  let msg2 = (pos2, "It is incompatible with a normal parameter") in
  on_error ~code:(Typing.err_code Typing.InoutnessMismatch) [msg1; msg2]

let invalid_new_disposable pos =
  let msg =
    "Disposable objects may only be created in a 'using' statement or 'return' from function marked <<__ReturnDisposable>>"
  in
  add (Typing.err_code Typing.InvalidNewDisposable) pos msg

let invalid_return_disposable pos =
  let msg =
    "Return expression must be new disposable in function marked <<__ReturnDisposable>>"
  in
  add (Typing.err_code Typing.InvalidReturnDisposable) pos msg

let nonreactive_function_call pos decl_pos callee_reactivity cause_pos_opt =
  add_list
    (Typing.err_code Typing.NonreactiveFunctionCall)
    ( [
        (pos, "Reactive functions can only call other reactive functions.");
        (decl_pos, "This function is " ^ callee_reactivity ^ ".");
      ]
    @ Option.value_map cause_pos_opt ~default:[] ~f:(fun cause_pos ->
          [
            ( cause_pos,
              "This argument caused function to be " ^ callee_reactivity ^ "."
            );
          ]) )

let nonreactive_call_from_shallow pos decl_pos callee_reactivity cause_pos_opt =
  add_list
    (Typing.err_code Typing.NonreactiveCallFromShallow)
    ( [
        (pos, "Shallow reactive functions cannot call non-reactive functions.");
        (decl_pos, "This function is " ^ callee_reactivity ^ ".");
      ]
    @ Option.value_map cause_pos_opt ~default:[] ~f:(fun cause_pos ->
          [
            ( cause_pos,
              "This argument caused function to be " ^ callee_reactivity ^ "."
            );
          ]) )

let rx_enabled_in_non_rx_context pos =
  add
    (Typing.err_code Typing.RxEnabledInNonRxContext)
    pos
    "\\HH\\Rx\\IS_ENABLED can only be used in reactive functions."

let rx_parameter_condition_mismatch
    cond pos def_pos (on_error : typing_error_callback) =
  on_error
    ~code:(Typing.err_code Typing.RxParameterConditionMismatch)
    [
      ( pos,
        "This parameter does not satisfy "
        ^ cond
        ^ " condition defined on matching parameter in function super type." );
      (def_pos, "This is parameter declaration from the function super type.");
    ]

let nonreactive_indexing is_append pos =
  let msg =
    if is_append then
      "Cannot append to a Hack Collection object in a reactive context. Instead, use the 'add' method."
    else
      "Cannot assign to element of Hack Collection object via [] in a reactive context. Instead, use the 'set' method."
  in
  add (Typing.err_code Typing.NonreactiveIndexing) pos msg

let obj_set_reactive pos =
  let msg =
    "This object's property is being mutated(used as an lvalue)"
    ^ "\nYou cannot set non-mutable object properties in reactive functions"
  in
  add (Typing.err_code Typing.ObjSetReactive) pos msg

let invalid_unset_target_rx pos =
  add
    (Typing.err_code Typing.InvalidUnsetTargetInRx)
    pos
    "Non-mutable argument for 'unset' is not allowed in reactive functions."

let inout_argument_bad_type pos msgl =
  let msg =
    "Expected argument marked inout to be contained in a local or "
    ^ "a value-typed container (e.g. vec, dict, keyset, array). "
    ^ "To use inout here, assign to/from a temporary local variable."
  in
  add_list (Typing.err_code Typing.InoutArgumentBadType) ((pos, msg) :: msgl)

let ambiguous_lambda pos uses =
  let msg1 =
    "Lambda has parameter types that could not be determined at definition site."
  in
  let msg2 =
    Printf.sprintf
      "%d distinct use types were determined: please add type hints to lambda parameters."
      (List.length uses)
  in
  add_list
    (Typing.err_code Typing.AmbiguousLambda)
    ( [(pos, msg1); (pos, msg2)]
    @ List.map uses (fun (pos, ty) -> (pos, "This use has type " ^ ty)) )

let wrong_expression_kind_attribute
    expr_kind pos attr attr_class_pos attr_class_name intf_name =
  let msg1 =
    Printf.sprintf
      "The %s attribute cannot be used on %s."
      (strip_ns attr)
      expr_kind
  in
  let msg2 =
    Printf.sprintf
      "The attribute's class is defined here. To be available for use on %s, the %s class must implement %s."
      expr_kind
      (strip_ns attr_class_name)
      (strip_ns intf_name)
  in
  add_list
    (Typing.err_code Typing.WrongExpressionKindAttribute)
    [(pos, msg1); (attr_class_pos, msg2)]

let wrong_expression_kind_builtin_attribute expr_kind pos attr =
  let msg1 =
    Printf.sprintf
      "The %s attribute cannot be used on %s."
      (strip_ns attr)
      expr_kind
  in
  add_list (Typing.err_code Typing.WrongExpressionKindAttribute) [(pos, msg1)]

let cannot_return_borrowed_value_as_immutable fun_pos value_pos =
  add_list
    (Typing.err_code Typing.CannotReturnBorrowedValueAsImmutable)
    [
      ( fun_pos,
        "Values returned from reactive function by default are treated as immutable."
      );
      ( value_pos,
        "This value is mutably borrowed and cannot be returned as immutable" );
    ]

let decl_override_missing_hint pos (on_error : typing_error_callback) =
  on_error
    ~code:(Typing.err_code Typing.DeclOverrideMissingHint)
    [
      ( pos,
        "When redeclaring class members, both declarations must have a typehint"
      );
    ]

let invalid_type_for_atmost_rx_as_rxfunc_parameter pos type_str =
  add
    (Typing.err_code Typing.InvalidTypeForOnlyrxIfRxfuncParameter)
    pos
    ( "Parameter annotated with <<__AtMostRxAsFunc>> attribute must be function, now '"
    ^ type_str
    ^ "'." )

let missing_annotation_for_atmost_rx_as_rxfunc_parameter pos =
  add
    (Typing.err_code Typing.MissingAnnotationForOnlyrxIfRxfuncParameter)
    pos
    "Missing function type annotation on parameter marked with <<__AtMostRxAsFunc>> attribute."

let superglobal_in_reactive_context pos name =
  add
    (Typing.err_code Typing.SuperglobalInReactiveContext)
    pos
    ("Superglobal " ^ name ^ " cannot be used in a reactive context.")

let static_property_in_reactive_context pos =
  add
    (Typing.err_code Typing.StaticPropertyInReactiveContext)
    pos
    "Static property cannot be used in a reactive context."

let returns_void_to_rx_function_as_non_expression_statement pos fpos =
  add_list
    (Typing.err_code Typing.ReturnsVoidToRxAsNonExpressionStatement)
    [
      ( pos,
        "Cannot use result of function annotated with <<__ReturnsVoidToRx>> in reactive context"
      );
      (fpos, "This is function declaration.");
    ]

let non_awaited_awaitable_in_rx pos =
  add
    (Typing.err_code Typing.NonawaitedAwaitableInReactiveContext)
    pos
    "This value has Awaitable type. Awaitable typed values in reactive code must be immediately await'ed."

let shapes_key_exists_always_true pos1 name pos2 =
  add_list
    (Typing.err_code Typing.ShapesKeyExistsAlwaysTrue)
    [
      (pos1, "This Shapes::keyExists() check is always true");
      (pos2, "The field '" ^ name ^ "' exists because of this definition");
    ]

let shape_field_non_existence_reason pos name = function
  | `Undefined ->
    [(pos, "The field '" ^ name ^ "' is not defined in this shape")]
  | `Nothing reason ->
    ( pos,
      "The type of the field '"
      ^ name
      ^ "' in this shape doesn't allow any values" )
    :: reason

let shapes_key_exists_always_false pos1 name pos2 reason =
  add_list (Typing.err_code Typing.ShapesKeyExistsAlwaysFalse)
  @@ (pos1, "This Shapes::keyExists() check is always false")
     :: shape_field_non_existence_reason pos2 name reason

let shapes_method_access_with_non_existent_field
    pos1 name pos2 method_name reason =
  add_list (Typing.err_code Typing.ShapesMethodAccessWithNonExistentField)
  @@ ( pos1,
       "You are calling Shapes::"
       ^ method_name
       ^ "() on a field known to not exist" )
     :: shape_field_non_existence_reason pos2 name reason

let shape_access_with_non_existent_field pos1 name pos2 reason =
  add_list (Typing.err_code Typing.ShapeAccessWithNonExistentField)
  @@ (pos1, "You are accessing a field known to not exist")
     :: shape_field_non_existence_reason pos2 name reason

let ambiguous_object_access
    pos name self_pos vis subclass_pos class_self class_subclass =
  let class_self = strip_ns class_self in
  let class_subclass = strip_ns class_subclass in
  add_list
    (Typing.err_code Typing.AmbiguousObjectAccess)
    [
      (pos, "This object access to " ^ name ^ " is ambiguous");
      ( self_pos,
        "You will access the private instance declared in " ^ class_self );
      ( subclass_pos,
        "Instead of the " ^ vis ^ " instance declared in " ^ class_subclass );
    ]

let invalid_traversable_in_rx pos =
  add
    (Typing.err_code Typing.InvalidTraversableInRx)
    pos
    "Cannot traverse over non-reactive traversable in reactive code."

let lateinit_with_default pos =
  add
    (Typing.err_code Typing.LateInitWithDefault)
    pos
    "A late-initialized property cannot have a default value"

let bad_lateinit_override
    parent_is_lateinit parent_pos child_pos (on_error : typing_error_callback) =
  let verb =
    if parent_is_lateinit then
      "is"
    else
      "is not"
  in
  on_error
    ~code:(Typing.err_code Typing.BadLateInitOverride)
    [
      ( child_pos,
        "Redeclared properties must be consistently declared __LateInit" );
      (parent_pos, "The property " ^ verb ^ " declared __LateInit here");
    ]

let bad_xhp_attr_required_override
    parent_tag child_tag parent_pos child_pos (on_error : typing_error_callback)
    =
  on_error
    ~code:(Typing.err_code Typing.BadXhpAttrRequiredOverride)
    [
      (child_pos, "Redeclared attribute must not be less strict");
      ( parent_pos,
        "The attribute is "
        ^ parent_tag
        ^ ", which is stricter than "
        ^ child_tag );
    ]

let invalid_switch_case_value_type case_value_p case_value_ty scrutinee_ty =
  add (Typing.err_code Typing.InvalidSwitchCaseValueType) case_value_p
  @@ Printf.sprintf
       "Switch statements use == equality, so comparing values of type %s with %s may not give the desired result."
       case_value_ty
       scrutinee_ty

let unserializable_type pos message =
  add
    (Typing.err_code Typing.UnserializableType)
    pos
    ( "Unserializable type (could not be converted to JSON and back again): "
    ^ message )

let redundant_rx_condition pos =
  add
    (Typing.err_code Typing.RedundantRxCondition)
    pos
    "Reactivity condition for this method is always true, consider removing it."

let invalid_arraykey pos (cpos, ctype) (kpos, ktype) =
  add_list
    (Typing.err_code Typing.InvalidArrayKey)
    [
      (pos, "This value is not a valid key type for this container");
      (cpos, "This container is " ^ ctype);
      (kpos, String.capitalize ktype ^ " cannot be used as a key for " ^ ctype);
    ]

let invalid_sub_string pos ty =
  add (Typing.err_code Typing.InvalidSubString) pos
  @@ "Expected an object convertible to string but got "
  ^ ty

let typechecker_timeout (pos, fun_name) seconds =
  add
    (Typing.err_code Typing.TypecheckerTimeout)
    pos
    (Printf.sprintf
       "Type checker timed out after %d seconds whilst checking function %s"
       seconds
       fun_name)

let unresolved_type_variable pos =
  add
    (Typing.err_code Typing.UnresolvedTypeVariable)
    pos
    "The type of this expression contains an unresolved type variable"

let invalid_arraykey_constraint pos t =
  add
    (Typing.err_code Typing.InvalidArrayKeyConstraint)
    pos
    ( "This type is "
    ^ t
    ^ ", which cannot be used as an arraykey (string | int)" )

let exception_occurred pos =
  add
    (Typing.err_code Typing.ExceptionOccurred)
    pos
    (Printf.sprintf
       "An exception occurred while typechecking this. %s"
       Error_message_sentinel.please_file_a_bug_message)

let redundant_covariant pos msg suggest =
  add
    (Typing.err_code Typing.RedundantGeneric)
    pos
    ( "This generic parameter is redundant because it only appears in a covariant (output) position"
    ^ msg
    ^ ". Consider replacing uses of generic parameter with `"
    ^ suggest
    ^ "` or specifying <<__Explicit>> on the generic parameter" )

(*****************************************************************************)
(* Printing *)
(*****************************************************************************)

let to_json (error : Pos.absolute error_) =
  let (error_code, msgl) = (get_code error, to_list error) in
  let elts =
    List.map msgl (fun (p, w) ->
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
  Hh_json.JSON_Object [("message", Hh_json.JSON_Array elts)]

let convert_errors_to_string ?(include_filename = false) (errors : error list) :
    string list =
  List.fold_right
    ~init:[]
    ~f:(fun err acc_out ->
      List.fold_right
        ~init:acc_out
        ~f:(fun (pos, msg) acc_in ->
          let result = Format.asprintf "%a %s" Pos.pp pos msg in
          if include_filename then
            let full_result =
              Printf.sprintf
                "%s %s"
                (Pos.to_absolute pos |> Pos.filename)
                result
            in
            full_result :: acc_in
          else
            result :: acc_in)
        (to_list err))
    errors

(*****************************************************************************)
(* Try if errors. *)
(*****************************************************************************)

let try_ f1 f2 = try_with_result f1 (fun _ l -> f2 l)

let try_with_error f1 f2 =
  try_ f1 (fun err ->
      let (error_code, l) = (get_code err, to_list err) in
      add_list error_code l;
      f2 ())

let has_no_errors f =
  try_
    (fun () ->
      let _ = f () in
      true)
    (fun _ -> false)

(*****************************************************************************)
(* Do. *)
(*****************************************************************************)

let ignore_ f =
  let allow_errors_in_default_path_copy = !allow_errors_in_default_path in
  set_allow_errors_in_default_path true;
  let (_, result) = do_ f in
  set_allow_errors_in_default_path allow_errors_in_default_path_copy;
  result

let try_when f ~when_ ~do_ =
  try_with_result f (fun result (error : error) ->
      if when_ () then
        do_ error
      else
        add_error error;
      result)
