(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Reordered_argument_collections
open String_utils

type error_code = int [@@deriving eq, ord, show]

(** We use `Pos.t message` and `Pos_or_decl.t message` on the server
    and convert to `Pos.absolute message` before sending it to the client *)
type 'a message = 'a * string [@@deriving eq, ord, show]

let get_message_pos (msg : 'a message) = fst msg

let get_message_str (msg : 'a message) = snd msg

type phase =
  | Init
  | Parsing
  | Naming
  | Decl
  | Typing
[@@deriving eq]

type severity =
  | Warning
  | Error

type format =
  | Context
  | Raw
  | Highlighted

type typing_error_callback =
  ?code:int -> Pos.t * string -> (Pos_or_decl.t * string) list -> unit

type error_from_reasons_callback =
  ?code:int -> (Pos_or_decl.t * string) list -> unit

let claim_as_reason : Pos.t message -> Pos_or_decl.t message =
 (fun (p, m) -> (Pos_or_decl.of_raw_pos p, m))

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

let current_span : Pos.t ref = ref Pos.none

let ignore_pos_outside_current_span : bool ref = ref false

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
        ~key:k
        ~data:(PhaseMap.merge x y ~f:(fun phase x y -> f phase k x y)))

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

type ('prim_pos, 'pos) error_ = {
  code: error_code;
  claim: 'prim_pos message;
  reasons: 'pos message list;
}
[@@deriving eq, ord, show]

type finalized_error = (Pos.absolute, Pos.absolute) error_
[@@deriving eq, ord, show]

type error = (Pos.t, Pos_or_decl.t) error_ [@@deriving eq, ord, show]

type applied_fixme = Pos.t * int [@@deriving eq]

type per_file_errors = error file_t

type t = error files_t * applied_fixme files_t [@@deriving eq]

module Error = struct
  type t = error [@@deriving ord]
end

module ErrorSet = Caml.Set.Make (Error)

module FinalizedError = struct
  type t = finalized_error [@@deriving ord]
end

module FinalizedErrorSet = Caml.Set.Make (FinalizedError)

let applied_fixmes : applied_fixme files_t ref = ref Relative_path.Map.empty

let (error_map : error files_t ref) = ref Relative_path.Map.empty

let accumulate_errors = ref false

(* Some filename when declaring *)
let in_lazy_decl = ref None

let (is_hh_fixme : (Pos.t -> error_code -> bool) ref) = ref (fun _ _ -> false)

let badpos_message =
  Printf.sprintf
    "There is an error somewhere in this file. However the type checker reports that the error is in another file. %s"
    Error_message_sentinel.please_file_a_bug_message

let badpos_message_2 =
  Printf.sprintf
    "There is an error somewhere in this definition. However the type checker reports that the error is elsewhere. %s"
    Error_message_sentinel.please_file_a_bug_message

let try_with_result f1 (f2 : 'res -> error -> 'res) : 'res =
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
  | Some { code; claim; reasons } ->
    (* Remove bad position sentinel if present: we might be about to add a new primary
     * error position. *)
    let (claim, reasons) =
      let (prim_pos, msg) = claim in
      if String.equal msg badpos_message || String.equal msg badpos_message_2
      then
        match reasons with
        | [] -> failwith "in try_with_result"
        | (_p, claim) :: reasons -> ((prim_pos, claim), reasons)
      else
        (claim, reasons)
    in
    f2 result { code; claim; reasons }

(* Reset errors before running [f] so that we can return the errors
 * caused by f. These errors are not added in the global list of errors. *)
let do_ f =
  let error_map_copy = !error_map in
  let applied_fixmes_copy = !applied_fixmes in
  let accumulate_errors_copy = !accumulate_errors in
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

let run_with_span span f =
  let old_span = !current_span in
  current_span := span;
  Utils.try_finally ~f ~finally:(fun () -> current_span := old_span)

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
  Exit.exit Exit_status.Lazy_decl_bug

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

let phase_of_string (value : string) : phase option =
  match Caml.String.lowercase_ascii value with
  | "init" -> Some Init
  | "parsing" -> Some Parsing
  | "naming" -> Some Naming
  | "decl" -> Some Decl
  | "typing" -> Some Typing
  | _ -> None

let (name_context_to_string : name_context -> string) = function
  | FunctionNamespace -> "function"
  | ConstantNamespace -> "constant"
  | TypeNamespace -> "type"
  | TraitContext -> "trait"
  | ClassContext -> "class"
  | RecordContext -> "record"

let get_pos { claim; _ } = fst claim

let sort : error list -> error list =
 fun err ->
  let compare_reasons = List.compare (compare_message Pos_or_decl.compare) in
  let compare
      { code = x_code; claim = x_claim; reasons = x_messages }
      { code = y_code; claim = y_claim; reasons = y_messages } =
    (* The primary sort order is by file *)
    let comparison =
      Relative_path.compare
        (fst x_claim |> Pos.filename)
        (fst y_claim |> Pos.filename)
    in
    (* Then within each file, sort by phase *)
    let comparison =
      if comparison = 0 then
        Int.compare (x_code / 1000) (y_code / 1000)
      else
        comparison
    in
    (* If the error codes are the same, sort by position *)
    let comparison =
      if comparison = 0 then
        Pos.compare (fst x_claim) (fst y_claim)
      else
        comparison
    in
    (* If the primary positions are also the same, sort by message text *)
    let comparison =
      if comparison = 0 then
        String.compare (snd x_claim) (snd y_claim)
      else
        comparison
    in
    (* Finally, if the message text is also the same, then continue comparing
    the reason messages (which indicate the reason why Hack believes
    there is an error reported in the claim message) *)
    if comparison = 0 then
      compare_reasons x_messages y_messages
    else
      comparison
  in
  let equal x y = compare x y = 0 in
  List.sort ~compare:(fun x y -> compare x y) err
  |> List.remove_consecutive_duplicates ~equal:(fun x y -> equal x y)

let get_sorted_error_list : t -> error list =
 (fun (err, _) -> sort (files_t_to_list err))

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
      ~key:current_file
      ~data:
        (PhaseMap.add
           (get_current_file_t !file_t_map)
           ~key:current_phase
           ~data:new_list)

let do_with_context path phase f = run_in_context path phase (fun () -> do_ f)

(** Turn on lazy decl mode for the duration of the closure.
    This runs without returning the original state,
    since we collect it later in do_with_lazy_decls_ *)
let run_in_decl_mode filename f =
  let old_in_lazy_decl = !in_lazy_decl in
  in_lazy_decl := Some filename;
  Utils.try_finally ~f ~finally:(fun () -> in_lazy_decl := old_in_lazy_decl)

and make_error code claim reasons : error = { code; claim; reasons }

(*****************************************************************************)
(* Accessors. *)
(*****************************************************************************)
and get_code ({ code; _ } : ('pp, 'p) error_) = (code : error_code)

let get_severity (error : ('pp, 'p) error_) = get_code_severity (get_code error)

let to_list : ('p, 'p) error_ -> 'p message list =
 (fun { claim; reasons; _ } -> claim :: reasons)

let to_list_ : error -> Pos_or_decl.t message list =
 fun { claim = (pos, claim); reasons; _ } ->
  (Pos_or_decl.of_raw_pos pos, claim) :: reasons

let get_messages = to_list

let to_absolute : error -> finalized_error =
 fun { code; claim; reasons } ->
  let claim = (fst claim |> Pos.to_absolute, snd claim) in
  let reasons =
    List.map reasons ~f:(fun (p, s) ->
        (p |> Pos_or_decl.unsafe_to_raw_pos |> Pos.to_absolute, s))
  in
  { code; claim; reasons }

let make_absolute_error code (x : (Pos.absolute * string) list) :
    finalized_error =
  match x with
  | [] -> failwith "an error must have at least one message"
  | claim :: reasons -> { code; claim; reasons }

let read_lines path =
  try In_channel.read_lines path
  with Sys_error _ ->
    (try Multifile.read_file_from_multifile path with Sys_error _ -> [])

let num_digits x = int_of_float (Float.log10 (float_of_int x)) + 1

(* Sort [xs] such that all the values that return the same
   value for (f x) are consecutive. For any two elements
   x1 and x2 in xs where (f x1) = (f x2), if x1 occurs before x2 in
   xs, then x1 also occurs before x2 in the returned list. *)
let combining_sort (xs : 'a list) ~(f : 'a -> string) : _ list =
  let rec build_map xs grouped keys =
    match xs with
    | x :: xs ->
      let key = f x in
      (match String.Map.find grouped key with
      | Some members ->
        let grouped = String.Map.set grouped ~key ~data:(members @ [x]) in
        build_map xs grouped keys
      | None ->
        let grouped = String.Map.set grouped ~key ~data:[x] in
        build_map xs grouped (key :: keys))
    | [] -> (grouped, keys)
  in
  let (grouped, keys) = build_map xs String.Map.empty [] in
  List.concat_map (List.rev keys) ~f:(fun fn -> String.Map.find_exn grouped fn)

(* E.g. "10 errors found." *)
let format_summary format errors dropped_count max_errors : string option =
  match format with
  | Context
  | Highlighted ->
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

let to_absolute_for_test : error -> finalized_error =
 fun { code; claim; reasons } ->
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
  let claim = f @@ claim_as_reason claim in
  let reasons = List.map ~f reasons in
  { code; claim; reasons }

let report_pos_from_reason = ref false

let to_string ({ code; claim; reasons } : finalized_error) : string =
  let buf = Buffer.create 50 in
  let (pos1, msg1) = claim in
  Buffer.add_string
    buf
    begin
      let error_code = error_code_to_string code in
      let reason_msg =
        if !report_pos_from_reason && Pos.get_from_reason pos1 then
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
  Buffer.contents buf

let add_error_impl error =
  if !accumulate_errors then
    let () =
      match !current_context with
      | (path, _)
        when Relative_path.equal path Relative_path.default
             && not !allow_errors_in_default_path ->
        Hh_logger.log
          "WARNING: adding an error in default path\n%s\n"
          (Caml.Printexc.raw_backtrace_to_string
             (Caml.Printexc.get_callstack 100))
      | _ -> ()
    in
    (* Cheap test to avoid duplicating most recent error *)
    let error_list = get_current_list !error_map in
    match error_list with
    | old_error :: _ when equal_error error old_error -> ()
    | _ -> set_current_list error_map (error :: error_list)
  else
    (* We have an error, but haven't handled it in any way *)
    let msg = error |> to_absolute |> to_string in
    match !in_lazy_decl with
    | Some _ -> lazy_decl_error_logging msg error_map to_absolute to_string
    | None -> Utils.assert_false_log_backtrace (Some msg)

(* Whether we've found at least one error *)
let currently_has_errors () = not (List.is_empty (get_current_list !error_map))

module Parsing = Error_codes.Parsing
module Naming = Error_codes.Naming
module NastCheck = Error_codes.NastCheck
module Typing = Error_codes.Typing

(*****************************************************************************)
(* Types *)
(*****************************************************************************)

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
let hard_banned_codes =
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
      Typing.err_code Typing.NewClassReified;
      Typing.err_code Typing.MemoizeReified;
      Typing.err_code Typing.ClassGetReified;
    ]

let allowed_fixme_codes_strict = ref ISet.empty

let allowed_fixme_codes_partial = ref ISet.empty

let codes_not_raised_partial = ref ISet.empty

let set_allow_errors_in_default_path x = allow_errors_in_default_path := x

let is_allowed_code_strict code = ISet.mem code !allowed_fixme_codes_strict

let is_allowed_code_partial code = ISet.mem code !allowed_fixme_codes_partial

let is_not_raised_partial code = ISet.mem code !codes_not_raised_partial

let (get_hh_fixme_pos : (Pos.t -> error_code -> Pos.t option) ref) =
  ref (fun _ _ -> None)

let (is_hh_fixme_disallowed : (Pos.t -> error_code -> bool) ref) =
  ref (fun _ _ -> false)

(*****************************************************************************)
(* Errors accumulator. *)
(*****************************************************************************)

(* We still want to error somewhere to avoid silencing any kind of errors, which
 * could cause errors to creep into the codebase. *)
let wrap_error_in_different_file ~current_file ~current_span claim reasons =
  let reasons = claim :: reasons in
  let message =
    List.map reasons ~f:(fun (pos, msg) ->
        Pos.print_verbose_relative (Pos_or_decl.unsafe_to_raw_pos pos)
        ^ ": "
        ^ msg)
  in
  let stack =
    Exception.get_current_callstack_string 99 |> Exception.clean_stack
  in
  HackEventLogger.type_check_primary_position_bug ~current_file ~message ~stack;
  let claim =
    if Pos.equal current_span Pos.none then
      (Pos.make_from current_file, badpos_message)
    else
      (current_span, badpos_message_2)
  in
  (claim, reasons)

(* If primary position in error list isn't in current file, wrap with a sentinel error *)
let check_pos_msg :
    Pos.t message * Pos_or_decl.t message list ->
    Pos.t message * Pos_or_decl.t message list =
 fun (claim, reasons) ->
  let pos = fst claim in
  let current_file = fst !current_context in
  let current_span = !current_span in
  (* If error is reported inside the current span, or no span has been set but the error
   * is reported in the current file, then accept the error *)
  if
    !ignore_pos_outside_current_span
    || Pos.contains current_span pos
    || Pos.equal current_span Pos.none
       && Relative_path.equal (Pos.filename pos) current_file
    || Relative_path.equal current_file Relative_path.default
  then
    (claim, reasons)
  else
    wrap_error_in_different_file
      ~current_file
      ~current_span
      (claim_as_reason claim)
      reasons

let add_error_with_fixme_error error explanation =
  let { code; claim; reasons = _ } = error in
  let (pos, _) = claim in
  let pos = Option.value (!get_hh_fixme_pos pos code) ~default:pos in
  add_error_impl error;
  add_error_impl { code; claim = (pos, explanation); reasons = [] }

let rec add_applied_fixme code (pos : Pos.t) =
  if ServerLoadFlag.get_no_load () then
    let applied_fixmes_list = get_current_list !applied_fixmes in
    set_current_list applied_fixmes ((pos, code) :: applied_fixmes_list)
  else
    ()

and add code pos msg = add_list code (pos, msg) []

and fixme_present (pos : Pos.t) code =
  !is_hh_fixme pos code || !is_hh_fixme_disallowed pos code

and add_list code (claim : _ message) reasons =
  add_error { code; claim; reasons }

and add_error (error : error) =
  let { code; claim; reasons } = error in
  let (claim, reasons) = check_pos_msg (claim, reasons) in
  let error = { code; claim; reasons } in

  let pos = fst claim in

  if ISet.mem code hard_banned_codes then
    if fixme_present pos code then
      let explanation =
        Printf.sprintf
          "You cannot use `HH_FIXME` or `HH_IGNORE_ERROR` comments to suppress error %d, and this cannot be enabled by configuration"
          code
      in
      add_error_with_fixme_error error explanation
    else
      add_error_impl error
  else if
    is_not_raised_partial code && Relative_path.is_partial (Pos.filename pos)
  then
    ()
  else if not (fixme_present pos code) then
    (* Fixmes and banned decl fixmes are separated by the parser because Errors can't recover
     * the position information after the fact. This is the default case, where an HH_FIXME
     * comment is not present. Therefore, the remaining cases are variations on behavior when
     * a fixme is present *)
    add_error_impl error
  else if Relative_path.(is_hhi (prefix (Pos.filename pos))) then
    add_applied_fixme code pos
  else if !report_pos_from_reason && Pos.get_from_reason pos then
    let explanation =
      "You cannot use `HH_FIXME` or `HH_IGNORE_ERROR` comments to suppress an error whose position was derived from reason information"
    in
    add_error_with_fixme_error error explanation
  else if !is_hh_fixme_disallowed pos code then
    let explanation =
      Printf.sprintf
        "You cannot use `HH_FIXME` or `HH_IGNORE_ERROR` comments to suppress error %d in declarations"
        code
    in
    add_error_with_fixme_error error explanation
  else
    let whitelist =
      if
        (not (ISet.is_empty !allowed_fixme_codes_partial))
        && Relative_path.is_partial (Pos.filename pos)
      then
        is_allowed_code_partial
      else
        is_allowed_code_strict
    in
    if whitelist code then
      add_applied_fixme code pos
    else
      let explanation =
        Printf.sprintf
          "You cannot use `HH_FIXME` or `HH_IGNORE_ERROR` comments to suppress error %d"
          code
      in
      add_error_with_fixme_error error explanation

and merge (err', fixmes') (err, fixmes) =
  let append _ _ x y =
    let x = Option.value x ~default:[] in
    let y = Option.value y ~default:[] in
    Some (List.rev_append x y)
  in
  (files_t_merge ~f:append err' err, files_t_merge ~f:append fixmes' fixmes)

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
    | Some x -> Relative_path.Map.add acc ~key:path ~data:x
  in
  (* Replace old errors with new *)
  let res =
    files_t_merge new_ old ~f:(fun phase path new_ old ->
        ( if Relative_path.equal path Relative_path.default then
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
               ^ "mode. There might be a missing call to `Errors.do_with_context`/"
               ^ "`run_in_context` somwhere or incorrectly used `Errors.from_error_list`."
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

let from_file_error_list : ?phase:phase -> (Relative_path.t * error) list -> t =
 fun ?(phase = Typing) errors ->
  let errors =
    List.fold
      errors
      ~init:Relative_path.Map.empty
      ~f:(fun errors (file, error) ->
        let errors_for_file =
          Relative_path.Map.find_opt errors file
          |> Option.value ~default:PhaseMap.empty
        in
        let errors_for_phase =
          PhaseMap.find_opt errors_for_file phase |> Option.value ~default:[]
        in
        let errors_for_phase = error :: errors_for_phase in
        let errors_for_file =
          PhaseMap.add errors_for_file ~key:phase ~data:errors_for_phase
        in
        Relative_path.Map.add errors ~key:file ~data:errors_for_file)
  in
  (errors, Relative_path.Map.empty)

let as_map : t -> error list Relative_path.Map.t =
 fun (errors, _fixmes) ->
  Relative_path.Map.map
    errors
    ~f:(PhaseMap.fold ~init:[] ~f:(fun _ -> List.append))

let add_error_assert_primary_pos_in_current_decl :
    current_decl_and_file:Pos_or_decl.ctx ->
    error_code ->
    Pos_or_decl.t message list ->
    unit =
 fun ~current_decl_and_file code reasons ->
  match reasons with
  | [] -> ()
  | ((primary_pos, claim_) as claim) :: remaining_reasons ->
    let (claim, reasons) =
      match
        Pos_or_decl.fill_in_filename_if_in_current_decl
          ~current_decl_and_file
          primary_pos
      with
      | Some primary_pos -> ((primary_pos, claim_), remaining_reasons)
      | None ->
        wrap_error_in_different_file
          ~current_file:current_decl_and_file.Pos_or_decl.file
          ~current_span:!current_span
          claim
          reasons
    in
    add_list code claim reasons

let error_assert_primary_pos_in_current_decl_callback :
    default_code:Typing.t ->
    current_decl_and_file:Pos_or_decl.ctx ->
    error_from_reasons_callback =
 fun ~default_code ~current_decl_and_file ?code reasons ->
  let code = Option.value code ~default:(Typing.err_code default_code) in
  add_error_assert_primary_pos_in_current_decl
    ~current_decl_and_file
    code
    reasons

let unify_error_assert_primary_pos_in_current_decl :
    current_decl_and_file:Pos_or_decl.ctx -> error_from_reasons_callback =
  error_assert_primary_pos_in_current_decl_callback
    ~default_code:Typing.UnifyError

let invalid_type_hint_assert_primary_pos_in_current_decl :
    current_decl_and_file:Pos_or_decl.ctx -> error_from_reasons_callback =
  error_assert_primary_pos_in_current_decl_callback
    ~default_code:Typing.InvalidTypeHint

let merge_into_current errors =
  let merged = merge errors (!error_map, !applied_fixmes) in
  error_map := fst merged;
  applied_fixmes := snd merged

let apply_callback_to_errors : t -> error_from_reasons_callback -> unit =
 fun (errors, _fixmes) on_error ->
  let on_error { code; claim; reasons } =
    on_error ~code (claim_as_reason claim :: reasons)
  in
  Relative_path.Map.iter errors ~f:(fun _ ->
      PhaseMap.iter ~f:(fun _ -> List.iter ~f:on_error))

(*****************************************************************************)
(* Accessors. (All methods delegated to the parameterized module.) *)
(*****************************************************************************)

let errors_in_file : t -> Relative_path.t -> _ =
 fun (errors, _) file ->
  Relative_path.Map.find_opt errors file
  |> Option.value ~default:PhaseMap.empty
  |> PhaseMap.fold ~init:[] ~f:(fun _phase errors acc -> errors @ acc)

let per_file_error_count : per_file_errors -> int =
  PhaseMap.fold ~init:0 ~f:(fun _phase errors count ->
      List.length errors + count)

let get_file_errors : t -> Relative_path.t -> per_file_errors =
 fun (errors, _) file ->
  Relative_path.Map.find_opt errors file |> Option.value ~default:PhaseMap.empty

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

let fold_errors_in ?phase err ~file ~init ~f =
  Relative_path.Map.find_opt (fst err) file
  |> Option.value ~default:PhaseMap.empty
  |> PhaseMap.fold ~init ~f:(fun p errors acc ->
         match phase with
         | Some x when not (equal_phase x p) -> acc
         | _ -> List.fold_right errors ~init:acc ~f)

let fold_per_file :
    t ->
    init:'acc ->
    f:(Relative_path.t -> per_file_errors -> 'acc -> 'acc) ->
    'acc =
 (fun (errors, _fixmes) -> Relative_path.Map.fold errors)

let get_failed_files err phase =
  files_t_fold (fst err) ~init:Relative_path.Set.empty ~f:(fun source p _ acc ->
      if not (equal_phase phase p) then
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

let on_error_or_add
    (on_error : error_from_reasons_callback option) code claim reasons =
  match on_error with
  | None -> add_error { code; claim; reasons }
  | Some f -> f ~code (claim_as_reason claim :: reasons)

(*****************************************************************************)
(* Parsing errors. *)
(*****************************************************************************)

let fixme_format pos =
  add
    (Parsing.err_code Parsing.FixmeFormat)
    pos
    "`HH_FIXME` wrong format, expected `/* HH_FIXME[ERROR_NUMBER] */`"

let parsing_error (p, msg) = add (Parsing.err_code Parsing.ParsingError) p msg

let xhp_parsing_error (p, msg) =
  add (Parsing.err_code Parsing.XhpParsingError) p msg

(*****************************************************************************)
(* Legacy AST / AAST errors *)
(*****************************************************************************)

let mk_unsupported_trait_use_as pos =
  {
    code = Naming.err_code Naming.UnsupportedTraitUseAs;
    claim =
      ( pos,
        "Aliasing with `as` within a trait `use` is a PHP feature that is unsupported in Hack"
      );
    reasons = [];
  }

let unsupported_trait_use_as pos = add_error (mk_unsupported_trait_use_as pos)

let mk_unsupported_instead_of pos =
  {
    code = Naming.err_code Naming.UnsupportedInsteadOf;
    claim = (pos, "`insteadof` is a PHP feature that is unsupported in Hack");
    reasons = [];
  }

let unsupported_instead_of pos = add_error (mk_unsupported_instead_of pos)

let mk_invalid_trait_use_as_visibility pos =
  {
    code = Naming.err_code Naming.InvalidTraitUseAsVisibility;
    claim = (pos, "Cannot redeclare trait method's visibility in this manner");
    reasons = [];
  }

let invalid_trait_use_as_visibility pos =
  add_error (mk_invalid_trait_use_as_visibility pos)

(*****************************************************************************)
(* Naming errors *)
(*****************************************************************************)

let unexpected_arrow pos cname =
  add
    (Naming.err_code Naming.UnexpectedArrow)
    pos
    ( "Keys may not be specified for "
    ^ Markdown_lite.md_codify cname
    ^ " initialization" )

let missing_arrow pos cname =
  add
    (Naming.err_code Naming.MissingArrow)
    pos
    ( "Keys must be specified for "
    ^ Markdown_lite.md_codify cname
    ^ " initialization" )

let disallowed_xhp_type pos name =
  add
    (Naming.err_code Naming.DisallowedXhpType)
    pos
    ( Markdown_lite.md_codify name
    ^ " is not a valid type. Use `:xhp` or `XHPChild`." )

let name_is_reserved name pos =
  let name = Utils.strip_all_ns name in
  add
    (Naming.err_code Naming.NameIsReserved)
    pos
    (Markdown_lite.md_codify name ^ " cannot be used as it is reserved.")

let dollardollar_unused pos =
  add
    (Naming.err_code Naming.DollardollarUnused)
    pos
    ( "This expression does not contain a "
    ^ "usage of the special pipe variable. Did you forget to use the `$$` "
    ^ "variable?" )

let method_name_already_bound pos name =
  add
    (Naming.err_code Naming.MethodNameAlreadyBound)
    pos
    ("Method name already bound: " ^ Markdown_lite.md_codify name)

(* Given two equal-length strings, highlights the characters in
   the second that differ from the first *)
let highlight_differences base to_highlight =
  match List.zip (String.to_list base) (String.to_list to_highlight) with
  | List.Or_unequal_lengths.Ok l ->
    List.group l ~break:(fun (o1, s1) (o2, s2) ->
        not (Bool.equal (Char.equal o1 s1) (Char.equal o2 s2)))
    |> List.map ~f:(fun cs ->
           let s = List.map cs ~f:snd |> String.of_char_list in
           let (c1, c2) = List.hd_exn cs in
           if Char.equal c1 c2 then
             s
           else
             Markdown_lite.md_highlight s)
    |> String.concat
  | List.Or_unequal_lengths.Unequal_lengths -> to_highlight

let error_name_already_bound name name_prev (p : Pos.t) (p_prev : Pos.t) =
  let name = strip_ns name in
  let name_prev = strip_ns name_prev in
  let (claim, reasons) =
    ( (p, "Name already bound: " ^ Markdown_lite.md_codify name),
      [
        ( Pos_or_decl.of_raw_pos p_prev,
          if String.equal name name_prev then
            "Previous definition is here"
          else
            "Previous definition "
            ^ (highlight_differences name name_prev |> Markdown_lite.md_codify)
            ^ " differs only by case " );
      ] )
  in
  let hhi_msg =
    "This appears to be defined in an hhi file included in your project "
    ^ "root. The hhi files for the standard library are now a part of the "
    ^ "typechecker and must be removed from your project. Typically, you can "
    ^ "do this by deleting the \"hhi\" directory you copied into your "
    ^ "project when first starting with Hack."
  in
  let reasons =
    if Relative_path.(is_hhi (prefix (Pos.filename p))) then
      reasons @ [(Pos_or_decl.of_raw_pos p_prev, hhi_msg)]
    else if Pos.is_hhi p_prev then
      reasons @ [(Pos_or_decl.of_raw_pos p, hhi_msg)]
    else
      reasons
  in
  add_list (Naming.err_code Naming.ErrorNameAlreadyBound) claim reasons

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
    ("Unbound name: " ^ Markdown_lite.md_codify (strip_ns name) ^ kind_str)

let invalid_fun_pointer pos name =
  add
    (Naming.err_code Naming.InvalidFunPointer)
    pos
    ( "Unbound global function: "
    ^ Markdown_lite.md_codify (strip_ns name)
    ^ " is not a valid name for fun()" )

let hint_message ?(modifier = "") orig hint hint_pos =
  let s =
    if
      (not (String.equal orig hint))
      && String.equal (String.lowercase orig) (String.lowercase hint)
    then
      Printf.sprintf
        "Did you mean %s%s instead (which only differs by case)?"
        modifier
        (highlight_differences orig hint |> Markdown_lite.md_codify)
    else
      Printf.sprintf
        "Did you mean %s%s instead?"
        modifier
        (Markdown_lite.md_codify hint)
  in
  (hint_pos, s)

let undefined pos var_name did_you_mean =
  let msg =
    Printf.sprintf
      "Variable %s is undefined, or not always defined."
      (Markdown_lite.md_codify var_name)
  in
  let suggestion =
    match did_you_mean with
    | Some (did_you_mean, pos) ->
      [hint_message var_name did_you_mean pos |> claim_as_reason]
    | None -> []
  in
  add_list (Naming.err_code Naming.Undefined) (pos, msg) suggestion

let this_reserved pos =
  add
    (Naming.err_code Naming.ThisReserved)
    pos
    "The type parameter `this` is reserved"

let start_with_T pos =
  add
    (Naming.err_code Naming.StartWith_T)
    pos
    "Please make your type parameter start with the letter `T` (capital)"

let already_bound pos name =
  add
    (Naming.err_code Naming.NameAlreadyBound)
    pos
    ("Argument already bound: " ^ Markdown_lite.md_codify name)

let unexpected_typedef pos def_pos expected_kind =
  let expected_type = name_context_to_string expected_kind in
  add_list
    (Naming.err_code Naming.UnexpectedTypedef)
    ( pos,
      Printf.sprintf
        "Expected a %s but got a type alias."
        (Markdown_lite.md_codify expected_type) )
    [(Pos_or_decl.of_raw_pos def_pos, "Alias definition is here.")]

let mk_fd_name_already_bound pos =
  {
    code = Naming.err_code Naming.FdNameAlreadyBound;
    claim = (pos, "Field name already bound");
    reasons = [];
  }

let fd_name_already_bound pos = add_error (mk_fd_name_already_bound pos)

let repeated_record_field name pos prev_pos =
  let msg =
    Printf.sprintf "Duplicate record field %s" (Markdown_lite.md_codify name)
  in
  add_list
    (NastCheck.err_code NastCheck.RepeatedRecordFieldName)
    (pos, msg)
    [(prev_pos, "Previous field is here")]

let unexpected_record_field_name ~field_name ~field_pos ~record_name ~decl_pos =
  let msg =
    Printf.sprintf
      "Record %s has no field %s"
      (strip_ns record_name |> Markdown_lite.md_codify)
      (Markdown_lite.md_codify field_name)
  in
  add_list
    (Typing.err_code Typing.RecordUnknownField)
    (field_pos, msg)
    [(decl_pos, "Definition is here")]

let missing_record_field_name ~field_name ~new_pos ~record_name ~field_decl_pos
    =
  let msg =
    Printf.sprintf
      "Mising required field %s in %s"
      (Markdown_lite.md_codify field_name)
      (strip_ns record_name |> Markdown_lite.md_codify)
  in
  add_list
    (Typing.err_code Typing.RecordMissingRequiredField)
    (new_pos, msg)
    [(field_decl_pos, "Field definition is here")]

let type_not_record id pos =
  add
    (Typing.err_code Typing.NotARecord)
    pos
    (Printf.sprintf
       "Expected a record type, but got %s."
       (strip_ns id |> Markdown_lite.md_codify))

let primitive_toplevel pos =
  add
    (Naming.err_code Naming.PrimitiveToplevel)
    pos
    "Primitive type annotations are always available and may no longer be referred to in the toplevel namespace."

let primitive_invalid_alias pos used valid =
  add
    (Naming.err_code Naming.PrimitiveInvalidAlias)
    pos
    ( "Invalid Hack type. Using "
    ^ Markdown_lite.md_codify used
    ^ " in Hack is considered an error. Use "
    ^ Markdown_lite.md_codify valid
    ^ " instead, to keep the codebase consistent." )

let dynamic_new_in_strict_mode pos =
  add
    (Naming.err_code Naming.DynamicNewInStrictMode)
    pos
    "Cannot use dynamic `new`."

let invalid_type_access_root (pos, id) =
  add
    (Naming.err_code Naming.InvalidTypeAccessRoot)
    pos
    ( Markdown_lite.md_codify id
    ^ " must be an identifier for a class, `self`, or `this`" )

let duplicate_user_attribute (pos, name) existing_attr_pos =
  add_list
    (Naming.err_code Naming.DuplicateUserAttribute)
    (pos, "You cannot reuse the attribute " ^ Markdown_lite.md_codify name)
    [
      ( Pos_or_decl.of_raw_pos existing_attr_pos,
        Markdown_lite.md_codify name ^ " was already used here" );
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
    ( "Unrecognized user attribute: "
    ^ (strip_ns name |> Markdown_lite.md_codify)
    ^ " "
    ^ reason )

let this_no_argument pos =
  add (Naming.err_code Naming.ThisNoArgument) pos "`this` expects no arguments"

let object_cast pos =
  add
    (Naming.err_code Naming.ObjectCast)
    pos
    "Casts are only supported for `bool`, `int`, `float` and `string`."

let this_hint_outside_class pos =
  add
    (Naming.err_code Naming.ThisHintOutsideClass)
    pos
    "Cannot use `this` outside of a class"

let this_type_forbidden pos =
  add
    (Naming.err_code Naming.ThisMustBeReturn)
    pos
    "The type `this` cannot be used as a constraint on a class generic, or as the type of a static member variable"

let nonstatic_property_with_lsb pos =
  add
    (Naming.err_code Naming.NonstaticPropertyWithLSB)
    pos
    "`__LSB` attribute may only be used on static properties"

let lowercase_this pos type_ =
  add
    (Naming.err_code Naming.LowercaseThis)
    pos
    ( "Invalid Hack type "
    ^ Markdown_lite.md_codify type_
    ^ ". Use `this` instead" )

let classname_param pos =
  add
    (Naming.err_code Naming.ClassnameParam)
    pos
    ( "Missing type parameter to `classname`; `classname` is entirely"
    ^ " meaningless without one" )

(** Used if higher-kinded types are disabled *)
let typaram_applied_to_type pos x =
  add
    (Naming.err_code Naming.HigherKindedTypesUnsupportedFeature)
    pos
    (Printf.sprintf
       "`%s` is a type parameter. Type parameters cannot take type arguments (e.g. `%s<int>` isn't allowed)"
       x
       x)

(** Used if higher-kinded types are disabled *)
let tparam_with_tparam pos x =
  let param_desc =
    match x with
    | "_" -> ""
    | _ -> Markdown_lite.md_codify x ^ " is a type parameter. "
  in
  add
    (Naming.err_code Naming.HigherKindedTypesUnsupportedFeature)
    pos
    (Printf.sprintf
       "%sType parameters cannot themselves have type parameters"
       param_desc)

let shadowed_type_param p pos name =
  add_list
    (Naming.err_code Naming.ShadowedTypeParam)
    ( p,
      Printf.sprintf
        "You cannot re-bind the type parameter %s"
        (Markdown_lite.md_codify name) )
    [
      ( Pos_or_decl.of_raw_pos pos,
        Printf.sprintf "%s is already bound here" (Markdown_lite.md_codify name)
      );
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
    "`__clone` method cannot take arguments"

let naming_too_few_arguments pos =
  add (Naming.err_code Naming.NamingTooFewArguments) pos "Too few arguments"

let naming_too_many_arguments pos =
  add (Naming.err_code Naming.NamingTooManyArguments) pos "Too many arguments"

let expected_collection pos cn =
  add
    (Naming.err_code Naming.ExpectedCollection)
    pos
    ("Unexpected collection type " ^ (strip_ns cn |> Markdown_lite.md_codify))

let illegal_CLASS pos =
  add
    (Naming.err_code Naming.IllegalClass)
    pos
    "Using `__CLASS__` outside a class or trait"

let illegal_TRAIT pos =
  add
    (Naming.err_code Naming.IllegalTrait)
    pos
    "Using `__TRAIT__` outside a trait"

let lvar_in_obj_get pos =
  add
    (Naming.err_code Naming.LvarInObjGet)
    pos
    "Dynamic method or attribute access is not allowed on a non-dynamic value."

let nullsafe_property_write_context pos =
  add
    (Typing.err_code Typing.NullsafePropertyWriteContext)
    pos
    "`?->` syntax not supported here, this function effectively does a write"

let illegal_fun pos =
  let msg =
    "The argument to `fun()` must be a single-quoted, constant "
    ^ "literal string representing a valid function name."
  in
  add (Naming.err_code Naming.IllegalFun) pos msg

let illegal_member_variable_class pos =
  let msg =
    "Cannot declare a constant named `class`. The name `class` is reserved for the class constant that represents the name of the class"
  in
  add (Naming.err_code Naming.IllegalMemberVariableClass) pos msg

let illegal_meth_fun pos =
  let msg =
    "String argument to `fun()` contains `:`;"
    ^ " for static class methods, use"
    ^ " `class_meth(Cls::class, 'method_name')`, not `fun('Cls::method_name')`"
  in
  add (Naming.err_code Naming.IllegalMethFun) pos msg

let illegal_inst_meth pos =
  let msg =
    "The argument to `inst_meth()` must be an expression and a "
    ^ "constant literal string representing a valid method name."
  in
  add (Naming.err_code Naming.IllegalInstMeth) pos msg

let illegal_meth_caller pos =
  let msg =
    "The two arguments to `meth_caller()` must be:"
    ^ "\n - first: `ClassOrInterface::class`"
    ^ "\n - second: a single-quoted string literal containing the name"
    ^ " of a non-static method of that class"
  in
  add (Naming.err_code Naming.IllegalMethCaller) pos msg

let illegal_class_meth pos =
  let msg =
    "The two arguments to `class_meth()` must be:"
    ^ "\n - first: `ValidClassname::class`"
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

let class_meth_non_final_CLASS pos is_trait class_name =
  let suggestion =
    if not is_trait then
      "Use `" ^ strip_ns class_name ^ "::class` explicitly"
    else
      ""
  in
  let msg =
    "`class_meth` with `__CLASS__` in non-final classes is not allowed.\n"
    ^ suggestion
  in
  add (Naming.err_code Naming.ClassMethNonFinalCLASS) pos msg

let unexpected_ty_in_tast pos ~actual_ty ~expected_ty =
  add
    (Typing.err_code Typing.UnexpectedTy)
    pos
    ( "Unexpected type in TAST: expected "
    ^ Markdown_lite.md_codify expected_ty
    ^ ", got "
    ^ Markdown_lite.md_codify actual_ty )

let uninstantiable_class :
    Pos.t -> Pos_or_decl.t -> string -> (Pos.t * string) option -> unit =
 fun (usage_pos : Pos.t) (decl_pos : Pos_or_decl.t) name reason_msgl ->
  let name = strip_ns name in
  let (claim, reasons) =
    ( (usage_pos, Markdown_lite.md_codify name ^ " is uninstantiable"),
      [(decl_pos, "Declaration is here")] )
  in
  let (claim, reasons) =
    match reason_msgl with
    | Some (prefix_claim_pos, prefix_claim_msg) ->
      let reasons = claim_as_reason claim :: reasons in
      let claim =
        (prefix_claim_pos, prefix_claim_msg ^ " which must be instantiable")
      in
      (claim, reasons)
    | None -> (claim, reasons)
  in
  add_list (Typing.err_code Typing.UninstantiableClass) claim reasons

let new_abstract_record (pos, name) =
  let name = strip_ns name in
  add
    (Typing.err_code Typing.NewAbstractRecord)
    pos
    (Printf.sprintf
       "Cannot create instance of abstract record %s"
       (Markdown_lite.md_codify name))

let abstract_const_usage usage_pos decl_pos name =
  let name = strip_ns name in
  add_list
    (Typing.err_code Typing.AbstractConstUsage)
    ( usage_pos,
      "Cannot reference abstract constant "
      ^ Markdown_lite.md_codify name
      ^ " directly" )
    [(decl_pos, "Declaration is here")]

let concrete_const_interface_override
    child_pos
    parent_pos
    parent_origin
    name
    (on_error : error_from_reasons_callback) =
  let parent_origin = strip_ns parent_origin in
  on_error
    ~code:(Typing.err_code Typing.ConcreteConstInterfaceOverride)
    [
      ( child_pos,
        "Non-abstract constants defined in an interface cannot be overridden when implementing or extending that interface."
      );
      ( parent_pos,
        "You could make "
        ^ Markdown_lite.md_codify name
        ^ " abstract in "
        ^ Markdown_lite.md_codify parent_origin
        ^ "." );
    ]

let interface_const_multiple_defs
    child_pos
    parent_pos
    child_origin
    parent_origin
    name
    (on_error : error_from_reasons_callback) =
  let parent_origin = strip_ns parent_origin in
  let child_origin = strip_ns child_origin in
  on_error
    ~code:(Typing.err_code Typing.ConcreteConstInterfaceOverride)
    [
      ( child_pos,
        "Non-abstract constants defined in an interface cannot conflict with other inherited constants."
      );
      ( parent_pos,
        Markdown_lite.md_codify name
        ^ " inherited from "
        ^ Markdown_lite.md_codify parent_origin );
      ( child_pos,
        "conflicts with constant "
        ^ Markdown_lite.md_codify name
        ^ " inherited from "
        ^ Markdown_lite.md_codify child_origin
        ^ "." );
    ]

let interface_typeconst_multiple_defs
    child_pos
    parent_pos
    child_origin
    parent_origin
    name
    child_is_abstract
    (on_error : error_from_reasons_callback) =
  let parent_origin = strip_ns parent_origin in
  let child_origin = strip_ns child_origin in
  let child =
    if child_is_abstract then
      "abstract type constant with default value"
    else
      "concrete type constant"
  in
  on_error
    ~code:(Typing.err_code Typing.ConcreteConstInterfaceOverride)
    [
      ( child_pos,
        "Concrete and abstract type constants with default values in an interface cannot conflict with inherited"
        ^ " concrete or abstract type constants with default values." );
      ( parent_pos,
        Markdown_lite.md_codify name
        ^ " inherited from "
        ^ Markdown_lite.md_codify parent_origin );
      ( child_pos,
        "conflicts with "
        ^ Markdown_lite.md_codify child
        ^ " "
        ^ Markdown_lite.md_codify name
        ^ " inherited from "
        ^ Markdown_lite.md_codify child_origin
        ^ "." );
    ]

let typeconst_concrete_concrete_override pos parent_pos =
  add_error_assert_primary_pos_in_current_decl
    (Typing.err_code Typing.TypeconstConcreteConcreteOverride)
    [
      (pos, "Cannot re-declare this type constant");
      (parent_pos, "Previously defined here");
    ]

let const_without_typehint sid =
  let (pos, name) = sid in
  let msg =
    Printf.sprintf
      "Please add a type hint `const SomeType %s`"
      (Utils.strip_all_ns name)
  in
  add (Naming.err_code Naming.AddATypehint) pos msg

let prop_without_typehint visibility sid =
  let (pos, name) = sid in
  let msg =
    Printf.sprintf "Please add a type hint `%s SomeType %s`" visibility name
  in
  add (Naming.err_code Naming.AddATypehint) pos msg

let illegal_constant pos =
  add (Naming.err_code Naming.IllegalConstant) pos "Illegal constant value"

let invalid_req_implements pos =
  add
    (Naming.err_code Naming.InvalidReqImplements)
    pos
    "Only traits may use `require implements`"

let invalid_req_extends pos =
  add
    (Naming.err_code Naming.InvalidReqExtends)
    pos
    "Only traits and interfaces may use `require extends`"

let did_you_mean_naming pos name suggest_pos suggest_name =
  let name = strip_ns name in
  let suggest_name = strip_ns suggest_name in
  add_list
    (Naming.err_code Naming.DidYouMeanNaming)
    (pos, "Could not find " ^ Markdown_lite.md_codify name ^ ".")
    [hint_message name suggest_name (Pos_or_decl.of_raw_pos suggest_pos)]

let using_internal_class pos name =
  add
    (Naming.err_code Naming.UsingInternalClass)
    pos
    ( Markdown_lite.md_codify name
    ^ " is an implementation internal class that cannot be used directly" )

let too_few_type_arguments p =
  add
    (Naming.err_code Naming.TooFewTypeArguments)
    p
    "Too few type arguments for this type"

let mk_method_needs_visibility pos =
  {
    code = Naming.err_code Naming.MethodNeedsVisibility;
    claim =
      (pos, "Methods need to be marked `public`, `private`, or `protected`.");
    reasons = [];
  }

let method_needs_visibility pos = add_error (mk_method_needs_visibility pos)

let dynamic_class_name_in_strict_mode pos =
  add
    (Naming.err_code Naming.DynamicClassNameInStrictMode)
    pos
    "Cannot use dynamic class name in strict mode"

let xhp_optional_required_attr pos id =
  add
    (Naming.err_code Naming.XhpOptionalRequiredAttr)
    pos
    ( "XHP attribute "
    ^ Markdown_lite.md_codify id
    ^ " cannot be marked as nullable and `@required`" )

let xhp_required_with_default pos id =
  add
    (Naming.err_code Naming.XhpRequiredWithDefault)
    pos
    ( "XHP attribute "
    ^ Markdown_lite.md_codify id
    ^ " cannot be marked `@required` and provide a default" )

let array_typehints_disallowed pos =
  add
    (Naming.err_code Naming.ArrayTypehintsDisallowed)
    pos
    "Array typehints are no longer legal; use `varray` or `darray` instead"

let wildcard_hint_disallowed pos =
  add
    (Naming.err_code Naming.WildcardHintDisallowed)
    pos
    "Wildcard typehints are not allowed in this position"

let wildcard_param_disallowed pos =
  add
    (Naming.err_code Naming.WildcardTypeParamDisallowed)
    pos
    "Cannot use anonymous type parameter in this position."

let illegal_use_of_dynamically_callable attr_pos meth_pos visibility =
  add_list
    (Naming.err_code Naming.IllegalUseOfDynamicallyCallable)
    (attr_pos, "`__DynamicallyCallable` can only be used on public methods")
    [
      ( Pos_or_decl.of_raw_pos meth_pos,
        sprintf "But this method is %s" (Markdown_lite.md_codify visibility) );
    ]

let dynamically_callable_reified attr_pos =
  add
    (NastCheck.err_code NastCheck.DynamicallyCallableReified)
    attr_pos
    "`__DynamicallyCallable` cannot be used on reified functions or methods"

let parent_in_function_pointer pos parento meth_name =
  let suggestion =
    match parento with
    | None -> "Consider using the name of the parent class explicitly."
    | Some id ->
      let name = Markdown_lite.md_codify (strip_ns id ^ "::" ^ meth_name) in
      "Consider using " ^ name ^ " instead"
  in
  add
    (Naming.err_code Naming.ParentInFunctionPointer)
    pos
    ( "Cannot use `parent::` in a function pointer due to class context ambiguity. "
    ^ suggestion )

let self_in_non_final_function_pointer pos cido meth_name =
  let suggestion =
    match cido with
    | None -> ""
    | Some id ->
      let name = Markdown_lite.md_codify (strip_ns id ^ "::" ^ meth_name) in
      "Consider using " ^ name ^ " instead"
  in
  add
    (Naming.err_code Naming.SelfInNonFinalFunctionPointer)
    pos
    ( "Cannot use `self::` in a function pointer in a non-final class due to class context ambiguity. "
    ^ suggestion )

let invalid_wildcard_context pos =
  add
    (Naming.err_code Naming.InvalidWildcardContext)
    pos
    "A wildcard can only be used as a context when it is the sole context of a callable parameter in a higher-order function. The parameter must also be referenced with `ctx` in the higher-order function's context list, e.g. `function hof((function ()[_]: void) $f)[ctx $f]: void {}`"

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
         "Make sure you call `parent::__construct`.\n";
       ])

let nonstatic_method_in_abstract_final_class pos =
  add
    (NastCheck.err_code NastCheck.NonstaticMethodInAbstractFinalClass)
    pos
    "Abstract final classes cannot have nonstatic methods or constructors."

let constructor_required (pos, name) prop_names =
  let name = strip_ns name in
  let props_str =
    List.map ~f:Markdown_lite.md_codify prop_names |> String.concat ~sep:" "
  in
  add
    (NastCheck.err_code NastCheck.ConstructorRequired)
    pos
    ( "Lacking `__construct`, class "
    ^ Markdown_lite.md_codify name
    ^ " does not initialize its private member(s): "
    ^ props_str )

let not_initialized (pos, cname) props =
  let cname = strip_ns cname in
  let prop_msgs =
    List.map props ~f:(fun (pos, prop) ->
        ( pos,
          Markdown_lite.md_codify ("$this->" ^ prop) ^ " is not initialized." ))
  in
  add_list
    (NastCheck.err_code NastCheck.NotInitialized)
    ( pos,
      "Class "
      ^ Markdown_lite.md_codify cname
      ^ " has properties that cannot be null and aren't always set in `__construct`."
    )
    prop_msgs

let call_before_init pos cv =
  add
    (NastCheck.err_code NastCheck.CallBeforeInit)
    pos
    (Utils.sl
       ( [
           "Until the initialization of `$this` is over,";
           " you can only call private methods\n";
           "The initialization is not over because ";
         ]
       @
       if String.equal cv "parent::__construct" then
         ["you forgot to call `parent::__construct`"]
       else
         [
           Markdown_lite.md_codify ("$this->" ^ cv);
           " can still potentially be null";
         ] ))

(*****************************************************************************)
(* Nast errors check *)
(*****************************************************************************)

let type_arity use_pos def_pos ~expected ~actual =
  add_list
    (Typing.err_code Typing.TypeArityMismatch)
    ( use_pos,
      Printf.sprintf
        "Wrong number of type arguments (expected %d, got %d)"
        expected
        actual )
    [(def_pos, "Definition is here")]

let abstract_with_body (p, _) =
  add
    (NastCheck.err_code NastCheck.AbstractWithBody)
    p
    "This method is declared as abstract, but has a body"

let mk_not_abstract_without_typeconst (p, _) =
  {
    code = NastCheck.err_code NastCheck.NotAbstractWithoutTypeconst;
    claim =
      ( p,
        "This type constant is not declared as abstract, it must have"
        ^ " an assigned type" );
    reasons = [];
  }

let not_abstract_without_typeconst node =
  add_error (mk_not_abstract_without_typeconst node)

let typeconst_depends_on_external_tparam pos ext_pos ext_name =
  add_list
    (NastCheck.err_code NastCheck.TypeconstDependsOnExternalTparam)
    ( pos,
      "A type constant can only use type parameters declared in its own"
      ^ " type parameter list" )
    [
      ( Pos_or_decl.of_raw_pos ext_pos,
        Markdown_lite.md_codify ext_name
        ^ " was declared as a type parameter here" );
    ]

let interface_with_partial_typeconst tconst_pos =
  add
    (NastCheck.err_code NastCheck.InterfaceWithPartialTypeconst)
    tconst_pos
    "An interface cannot contain a partially abstract type constant"

let partially_abstract_typeconst_definition pos =
  add
    (NastCheck.err_code NastCheck.PartiallyAbstractTypeconstDefinition)
    pos
    "`as` constraints are only legal on abstract type constants"

let mk_multiple_xhp_category pos =
  {
    code = NastCheck.err_code NastCheck.MultipleXhpCategory;
    claim = (pos, "XHP classes can only contain one category declaration");
    reasons = [];
  }

let multiple_xhp_category pos = add_error (mk_multiple_xhp_category pos)

let return_in_gen p =
  add
    (NastCheck.err_code NastCheck.ReturnInGen)
    p
    ( "You cannot return a value in a generator (a generator"
    ^ " is a function that uses `yield`)" )

let return_in_finally p =
  add
    (NastCheck.err_code NastCheck.ReturnInFinally)
    p
    ( "Don't use `return` in a `finally` block;"
    ^ " there's nothing to receive the return value" )

let toplevel_break p =
  add
    (NastCheck.err_code NastCheck.ToplevelBreak)
    p
    "`break` can only be used inside loops or `switch` statements"

let toplevel_continue p =
  add
    (NastCheck.err_code NastCheck.ToplevelContinue)
    p
    "`continue` can only be used inside loops"

let continue_in_switch p =
  add
    (NastCheck.err_code NastCheck.ContinueInSwitch)
    p
    ( "In PHP, `continue;` inside a switch statement is equivalent to `break;`."
    ^ " Hack does not support this; use `break` if that is what you meant." )

let await_in_sync_function p =
  add
    (NastCheck.err_code NastCheck.AwaitInSyncFunction)
    p
    "`await` can only be used inside `async` functions"

let interface_use_trait p =
  add
    (NastCheck.err_code NastCheck.InterfaceUsesTrait)
    p
    "Interfaces cannot use traits"

let static_memoized_function p =
  add
    (NastCheck.err_code NastCheck.StaticMemoizedFunction)
    p
    "`memoize` is not allowed on static methods in classes that aren't final "

let magic (p, s) =
  add
    (NastCheck.err_code NastCheck.Magic)
    p
    ( Markdown_lite.md_codify s
    ^ " is a magic method and cannot be called directly" )

let non_interface (p : Pos.t) (c2 : string) (verb : string) : 'a =
  add
    (NastCheck.err_code NastCheck.NonInterface)
    p
    ( "Cannot "
    ^ verb
    ^ " "
    ^ (strip_ns c2 |> Markdown_lite.md_codify)
    ^ " - it is not an interface" )

let toString_returns_string pos =
  add
    (NastCheck.err_code NastCheck.ToStringReturnsString)
    pos
    "`__toString` should return a string"

let toString_visibility pos =
  add
    (NastCheck.err_code NastCheck.ToStringVisibility)
    pos
    "`__toString` must have public visibility and cannot be static"

let uses_non_trait (p : Pos.t) (n : string) (t : string) =
  add
    (NastCheck.err_code NastCheck.UsesNonTrait)
    p
    ( (strip_ns n |> Markdown_lite.md_codify)
    ^ " is not a trait. It is "
    ^ t
    ^ "." )

let requires_non_class (p : Pos.t) (n : string) (t : string) =
  add
    (NastCheck.err_code NastCheck.RequiresNonClass)
    p
    ( (strip_ns n |> Markdown_lite.md_codify)
    ^ " is not a class. It is "
    ^ t
    ^ "." )

let requires_final_class (p : Pos.t) (n : string) =
  add
    (NastCheck.err_code NastCheck.RequiresFinalClass)
    p
    ((strip_ns n |> Markdown_lite.md_codify) ^ " is not an extendable class.")

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
    ("Illegal function name: " ^ (strip_ns mname |> Markdown_lite.md_codify))

let entrypoint_arguments pos =
  add
    (NastCheck.err_code NastCheck.EntryPointArguments)
    pos
    "`__EntryPoint` functions cannot take arguments."

let entrypoint_generics pos =
  add
    (NastCheck.err_code NastCheck.EntryPointGenerics)
    pos
    "`__EntryPoint` functions cannot have generic parameters."

let variadic_memoize pos =
  add
    (NastCheck.err_code NastCheck.VariadicMemoize)
    pos
    "Memoized functions cannot be variadic."

let abstract_method_memoize pos =
  add
    (NastCheck.err_code NastCheck.AbstractMethodMemoize)
    pos
    "Abstract methods cannot be memoized."

let instance_property_in_abstract_final_class pos =
  add
    (NastCheck.err_code NastCheck.InstancePropertyInAbstractFinalClass)
    pos
    "Abstract final classes cannot have instance properties."

let inout_params_special pos =
  add
    (NastCheck.err_code NastCheck.InoutParamsSpecial)
    pos
    "Methods with special semantics cannot have `inout` parameters."

let inout_params_memoize fpos pos =
  let msg1 = (fpos, "Functions with `inout` parameters cannot be memoized") in
  let msg2 = (Pos_or_decl.of_raw_pos pos, "This is an `inout` parameter") in
  add_list (NastCheck.err_code NastCheck.InoutParamsMemoize) msg1 [msg2]

let reading_from_append pos =
  add
    (NastCheck.err_code NastCheck.ReadingFromAppend)
    pos
    "Cannot use `[]` for reading"

let list_rvalue pos =
  add
    (NastCheck.err_code NastCheck.ListRvalue)
    pos
    "`list()` can only be used for destructuring assignment. Did you mean `tuple()` or `vec[]`?"

let inout_argument_bad_expr pos =
  add
    (NastCheck.err_code NastCheck.InoutArgumentBadExpr)
    pos
    ( "Arguments for `inout` parameters must be local variables or simple "
    ^ "subscript expressions on vecs, dicts, keysets, or arrays" )

let illegal_destructor pos =
  add
    (NastCheck.err_code NastCheck.IllegalDestructor)
    pos
    ( "Destructors are not supported in Hack; use other patterns like "
    ^ "`IDisposable`/`using` or `try`/`catch` instead." )

let switch_non_terminal_default pos =
  add
    (NastCheck.err_code NastCheck.SwitchNonTerminalDefault)
    pos
    "Default case in `switch` must be terminal"

let switch_multiple_default pos =
  add
    (NastCheck.err_code NastCheck.SwitchMultipleDefault)
    pos
    "There can be only one `default` case in `switch`"

let context_definitions_msg () =
  (* Notes:
   * - needs to be a thunk because path_of_prefix resolves a reference that is populated at runtime
   *   - points to the hh_server tmp hhi directory
   * - magic numbers are inteded to provide a nicer IDE experience,
   * - a pos is constructed in order to make the link to contexts.hhi clickable
   *)
  let path =
    Relative_path.(
      let path =
        Path.concat (Path.make (path_of_prefix Hhi)) "coeffect/contexts.hhi"
      in
      create Hhi (Path.to_string path))
  in
  ( Pos.make_from_lnum_bol_cnum
      ~pos_file:path
      ~pos_start:(28, 0, 0)
      ~pos_end:(28, 0, 23),
    "Hack provides a list of supported contexts here" )
  |> claim_as_reason

let illegal_context pos name =
  add_list
    NastCheck.(err_code IllegalContext)
    ( pos,
      "Illegal context: "
      ^ (name |> Markdown_lite.md_codify)
      ^ "\nCannot use a context defined outside namespace "
      ^ Naming_special_names.Coeffects.contexts )
    [context_definitions_msg ()]

(*****************************************************************************)
(* Nast terminality *)
(*****************************************************************************)

let case_fallthrough pos1 pos2 =
  add_list
    (NastCheck.err_code NastCheck.CaseFallthrough)
    ( pos1,
      "This `switch` has a `case` that implicitly falls through and is "
      ^ "not annotated with `// FALLTHROUGH`" )
    [
      ( Pos_or_decl.of_raw_pos pos2,
        "This `case` implicitly falls through. Did you forget to add `break` or `return`?"
      );
    ]

let default_fallthrough pos =
  add
    (NastCheck.err_code NastCheck.DefaultFallthrough)
    pos
    ( "This `switch` has a default case that implicitly falls "
    ^ "through and is not annotated with `// FALLTHROUGH`" )

(*****************************************************************************)
(* Typing errors *)
(*****************************************************************************)

let visibility_extends
    vis pos parent_pos parent_vis (on_error : error_from_reasons_callback) =
  let msg1 =
    (pos, "This member visibility is: " ^ Markdown_lite.md_codify vis)
  in
  let msg2 =
    (parent_pos, Markdown_lite.md_codify parent_vis ^ " was expected")
  in
  on_error ~code:(Typing.err_code Typing.VisibilityExtends) [msg1; msg2]

let visibility_override_internal
    pos parent_pos m parent_m (on_error : error_from_reasons_callback) =
  let msg1 =
    (pos, Printf.sprintf "Cannot override this member in module `%s`" m)
  in
  let msg2 =
    ( parent_pos,
      Printf.sprintf "This member is internal to module `%s`" parent_m )
  in
  on_error ~code:(Typing.err_code Typing.ModuleError) [msg1; msg2]

let member_not_implemented member_name parent_pos pos defn_pos =
  let msg1 =
    ( pos,
      "This type doesn't implement the method "
      ^ Markdown_lite.md_codify member_name )
  in
  let msg2 = (parent_pos, "Which is required by this interface") in
  let msg3 = (defn_pos, "As defined here") in
  add_list (Typing.err_code Typing.MemberNotImplemented) msg1 [msg2; msg3]

let bad_decl_override parent_pos parent_name pos name msgl =
  let msg1 =
    ( pos,
      "Class "
      ^ (strip_ns name |> Markdown_lite.md_codify)
      ^ " does not correctly implement all required members " )
  in
  let msg2 =
    ( parent_pos,
      "Some members are incompatible with those declared in type "
      ^ (strip_ns parent_name |> Markdown_lite.md_codify) )
  in
  (* This is a cascading error message *)
  add_list (Typing.err_code Typing.BadDeclOverride) msg1 (msg2 :: msgl)

let method_import_via_diamond
    pos class_name method_pos method_name trace1 trace2 =
  let class_name = Markdown_lite.md_codify (strip_ns class_name) in
  let method_name = Markdown_lite.md_codify (strip_ns method_name) in
  let msg1 =
    ( pos,
      "Class "
      ^ class_name
      ^ " inherits trait method "
      ^ method_name
      ^ " via multiple traits.  Remove the multiple paths or override the method"
    )
  in
  let msg2 = (method_pos, "Trait method is defined here") in
  add_list
    (Typing.err_code Typing.DiamondTraitMethod)
    msg1
    ((msg2 :: trace1) @ trace2)

let bad_method_override
    pos member_name msgl (on_error : error_from_reasons_callback) =
  let msg =
    ( pos,
      "The method "
      ^ (strip_ns member_name |> Markdown_lite.md_codify)
      ^ " is not compatible with the overridden method" )
  in
  (* This is a cascading error message *)
  on_error ~code:(Typing.err_code Typing.BadMethodOverride) (msg :: msgl)

let bad_prop_override
    pos member_name msgl (on_error : error_from_reasons_callback) =
  let msg =
    ( pos,
      "The property "
      ^ (strip_ns member_name |> Markdown_lite.md_codify)
      ^ " has the wrong type" )
  in
  (* This is a cascading error message *)
  on_error ~code:(Typing.err_code Typing.BadMethodOverride) (msg :: msgl)

let bad_enum_decl pos msgl =
  let msg = (pos, "This enum declaration is invalid.") in
  (* This is a cascading error message *)
  add_list (Typing.err_code Typing.BadEnumExtends) msg msgl

let missing_constructor pos (on_error : error_from_reasons_callback) =
  on_error
    ~code:(Typing.err_code Typing.MissingConstructor)
    [(pos, "The constructor is not implemented")]

let typedef_trail_entry pos = (pos, "Typedef definition comes from here")

let abstract_tconst_not_allowed
    pos (p, tconst_name) (on_error : error_from_reasons_callback) =
  on_error
    ~code:(Typing.err_code Typing.AbstractTconstNotAllowed)
    [
      (pos, "An abstract type constant is not allowed in this position.");
      ( p,
        Printf.sprintf
          "%s is abstract here."
          (Markdown_lite.md_codify tconst_name) );
    ]

let add_with_trail code claim reasons trail =
  add_list code claim (reasons @ List.map trail ~f:typedef_trail_entry)

let enum_constant_type_bad pos ty_pos ty trail =
  add_with_trail
    (Typing.err_code Typing.EnumConstantTypeBad)
    (pos, "Enum constants must be an `int` or `string`")
    [(ty_pos, "Not " ^ Markdown_lite.md_codify ty)]
    trail

let enum_type_bad pos ty_dependent ty trail =
  let ty = Markdown_lite.md_codify ty in
  let msg =
    if ty_dependent then
      "Invalid base type for an enum class: "
    else
      "Enums must be `int` or `string` or `arraykey`, not "
  in
  add_with_trail (Typing.err_code Typing.EnumTypeBad) (pos, msg ^ ty) [] trail

let enum_type_typedef_nonnull pos =
  add
    (Typing.err_code Typing.EnumTypeTypedefNonnull)
    pos
    "Can't use `typedef` that resolves to nonnull in enum"

let enum_switch_redundant const first_pos second_pos =
  add_list
    (Typing.err_code Typing.EnumSwitchRedundant)
    (second_pos, "Redundant `case` statement")
    [
      ( Pos_or_decl.of_raw_pos first_pos,
        Markdown_lite.md_codify const ^ " already handled here" );
    ]

let enum_switch_nonexhaustive pos missing enum_pos =
  add_list
    (Typing.err_code Typing.EnumSwitchNonexhaustive)
    ( pos,
      "`switch` statement nonexhaustive; the following cases are missing: "
      ^ (List.map ~f:Markdown_lite.md_codify missing |> String.concat ~sep:", ")
    )
    [(enum_pos, "Enum declared here")]

let enum_switch_redundant_default pos enum_pos =
  add_list
    (Typing.err_code Typing.EnumSwitchRedundantDefault)
    ( pos,
      "All cases already covered; a redundant `default` case prevents "
      ^ "detecting future errors. If your goal is to guard against "
      ^ "invalid values for this type, do an `is` check before the switch." )
    [(enum_pos, "Enum declared here")]

let enum_switch_not_const pos =
  add
    (Typing.err_code Typing.EnumSwitchNotConst)
    pos
    "Case in `switch` on enum is not an enum constant"

let enum_switch_wrong_class pos expected got =
  add
    (Typing.err_code Typing.EnumSwitchWrongClass)
    pos
    ( "Switching on enum "
    ^ Markdown_lite.md_codify expected
    ^ " but using constant from "
    ^ Markdown_lite.md_codify got )

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
    (pos, "A shape field name must be an `int` or `string`")
    [(ty_pos, "Not " ^ ty)]
    trail

let invalid_shape_field_literal key_pos witness_pos =
  add_list
    (Typing.err_code Typing.InvalidShapeFieldLiteral)
    (key_pos, "Shape uses literal string as field name")
    [(Pos_or_decl.of_raw_pos witness_pos, "But expected a class constant")]

let invalid_shape_field_const key_pos witness_pos =
  add_list
    (Typing.err_code Typing.InvalidShapeFieldConst)
    (key_pos, "Shape uses class constant as field name")
    [(Pos_or_decl.of_raw_pos witness_pos, "But expected a literal string")]

let shape_field_class_mismatch key_pos witness_pos key_class witness_class =
  add_list
    (Typing.err_code Typing.ShapeFieldClassMismatch)
    ( key_pos,
      "Shape field name is class constant from "
      ^ Markdown_lite.md_codify key_class )
    [
      ( Pos_or_decl.of_raw_pos witness_pos,
        "But expected constant from " ^ Markdown_lite.md_codify witness_class );
    ]

let shape_field_type_mismatch key_pos witness_pos key_ty witness_ty =
  add_list
    (Typing.err_code Typing.ShapeFieldTypeMismatch)
    (key_pos, "Shape field name is " ^ key_ty ^ " class constant")
    [(Pos_or_decl.of_raw_pos witness_pos, "But expected " ^ witness_ty)]

let missing_field pos1 pos2 name (on_error : error_from_reasons_callback) =
  on_error
    ~code:(Typing.err_code Typing.MissingField)
    [
      (pos1, "The field " ^ Markdown_lite.md_codify name ^ " is missing");
      (pos2, "The field " ^ Markdown_lite.md_codify name ^ " is defined");
    ]

let shape_fields_unknown pos1 pos2 (on_error : error_from_reasons_callback) =
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
    "You can only unset fields of **local** variables"

let unification_cycle pos ty =
  add_list
    (Typing.err_code Typing.UnificationCycle)
    ( pos,
      "Type circularity: in order to type-check this expression it "
      ^ "is necessary for a type [rec] to be equal to type "
      ^ Markdown_lite.md_codify ty )
    []

let violated_constraint
    violated_constraints reasons (on_error : error_from_reasons_callback) =
  let msgs =
    List.concat_map violated_constraints ~f:(fun (p_cstr, (p_tparam, tparam)) ->
        [
          ( p_tparam,
            Printf.sprintf
              "%s is a constrained type parameter"
              (Markdown_lite.md_codify tparam) );
          (p_cstr, "This type constraint is violated");
        ])
  in
  on_error
    ~code:(Typing.err_code Typing.TypeConstraintViolation)
    (msgs @ reasons)

let method_variance pos =
  add
    (Typing.err_code Typing.MethodVariance)
    pos
    "Covariance or contravariance is not allowed in type parameter of method or function."

let explain_constraint ~(use_pos : Pos.t) : error_from_reasons_callback =
 fun ?code:_ reasons ->
  add_list
    (Typing.err_code Typing.TypeConstraintViolation)
    (use_pos, "Some type arguments violate their constraints")
    reasons

let explain_where_constraint ~in_class ~use_pos ~definition_pos reasons =
  let callsite_ty =
    if in_class then
      "class"
    else
      "method"
  in
  let definition_head =
    Printf.sprintf "This is the %s with `where` type constraints" callsite_ty
  in
  let inst_msg = "A `where` type constraint is violated here" in
  add_list
    (Typing.err_code Typing.TypeConstraintViolation)
    (use_pos, inst_msg)
    ([(definition_pos, definition_head)] @ reasons)

let explain_tconst_where_constraint ~use_pos ~definition_pos msgl =
  let inst_msg = "A `where` type constraint is violated here" in
  add_list
    (Typing.err_code Typing.TypeConstraintViolation)
    (use_pos, inst_msg)
    ( [
        ( definition_pos,
          "This method's `where` constraints contain a generic type access" );
      ]
    @ msgl )

let format_string pos snippet s class_pos fname class_suggest =
  add_list
    (Typing.err_code Typing.FormatString)
    ( pos,
      "Invalid format string "
      ^ Markdown_lite.md_codify snippet
      ^ " in "
      ^ Markdown_lite.md_codify ("\"" ^ s ^ "\"") )
    [
      ( class_pos,
        "You can add a new format specifier by adding "
        ^ Markdown_lite.md_codify (fname ^ "()")
        ^ " to "
        ^ Markdown_lite.md_codify class_suggest );
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
    ( Markdown_lite.md_codify typehint
    ^ " is a nonsensical typehint; a function cannot both "
    ^ reason
    ^ " and return null." )

let tuple_syntax p =
  add
    (Typing.err_code Typing.TupleSyntax)
    p
    "Did you want a *tuple*? Try `(X,Y)`, not `tuple<X,Y>`"

let redeclaring_missing_method p trait_method =
  add
    (Typing.err_code Typing.RedeclaringMissingMethod)
    p
    ( "Attempting to redeclare a trait method "
    ^ Markdown_lite.md_codify trait_method
    ^ " which was never inherited. "
    ^ "You might be trying to redeclare a non-static method as `static` or vice-versa."
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

let duplicate_using_var pos =
  add
    (Typing.err_code Typing.DuplicateUsingVar)
    pos
    "Local variable already used in `using` statement"

let illegal_disposable pos verb =
  add
    (Typing.err_code Typing.IllegalDisposable)
    pos
    ("Disposable objects must only be " ^ verb ^ " in a `using` statement")

let escaping_disposable pos =
  add
    (Typing.err_code Typing.EscapingDisposable)
    pos
    ( "Variable from `using` clause may only be used as receiver in method invocation "
    ^ "or passed to function with `<<__AcceptDisposable>>` parameter attribute"
    )

let escaping_disposable_parameter pos =
  add
    (Typing.err_code Typing.EscapingDisposableParameter)
    pos
    ( "Parameter with `<<__AcceptDisposable>>` attribute may only be used as receiver in method invocation "
    ^ "or passed to another function with `<<__AcceptDisposable>>` parameter attribute"
    )

let escaping_this pos =
  add
    (Typing.err_code Typing.EscapingThis)
    pos
    ( "`$this` implementing `IDisposable` or `IAsyncDisposable` may only be used as receiver in method invocation "
    ^ "or passed to another function with `<<__AcceptDisposable>>` parameter attribute"
    )

let must_extend_disposable pos =
  add
    (Typing.err_code Typing.MustExtendDisposable)
    pos
    "A disposable type may not extend a class or use a trait that is not disposable"

let accept_disposable_invariant
    pos1 pos2 (on_error : error_from_reasons_callback) =
  let msg1 = (pos1, "This parameter is marked `<<__AcceptDisposable>>`") in
  let msg2 = (pos2, "This parameter is not marked `<<__AcceptDisposable>>`") in
  on_error ~code:(Typing.err_code Typing.AcceptDisposableInvariant) [msg1; msg2]

let ifc_external_contravariant
    pos1 pos2 (on_error : error_from_reasons_callback) =
  let msg1 =
    ( pos1,
      "Parameters with `<<__External>>` must be overridden by other parameters with <<__External>>. This parameter is marked `<<__External>>`"
    )
  in
  let msg2 = (pos2, "But this parameter is not marked `<<__External>>`") in
  on_error ~code:(Typing.err_code Typing.IFCExternalContravariant) [msg1; msg2]

let field_kinds pos1 pos2 =
  add_list
    (Typing.err_code Typing.FieldKinds)
    (pos1, "You cannot use this kind of field (value)")
    [(pos2, "Mixed with this kind of field (key => value)")]

let unbound_name_typing pos name =
  add
    (Typing.err_code Typing.UnboundNameTyping)
    pos
    ("Unbound name (typing): " ^ Markdown_lite.md_codify (strip_ns name))

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
    ^ Markdown_lite.md_codify msg
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
    (pos1, "You cannot return a value")
    [(Pos_or_decl.of_raw_pos pos2, "This is a `void` function")]

let returns_with_and_without_value
    ~fun_pos ~with_value_pos ~without_value_pos_opt =
  let without_value_message =
    match without_value_pos_opt with
    | None ->
      ( Pos_or_decl.of_raw_pos fun_pos,
        "This function does not always return a value" )
    | Some retun_without_val_pos ->
      ( Pos_or_decl.of_raw_pos retun_without_val_pos,
        "Returning without a value here" )
  in
  add_list
    (Typing.err_code Typing.ReturnsWithAndWithoutValue)
    (fun_pos, "This function can exit with and without returning a value")
    [
      (Pos_or_decl.of_raw_pos with_value_pos, "Returning a value here.");
      without_value_message;
    ]

let this_var_outside_class p =
  add
    (Typing.err_code Typing.ThisVarOutsideClass)
    p
    "Can't use `$this` outside of a class"

let unbound_global cst_pos =
  add
    (Typing.err_code Typing.UnboundGlobal)
    cst_pos
    "Unbound global constant (Typing)"

let private_inst_meth ~def_pos ~use_pos =
  add_list
    (Typing.err_code Typing.PrivateInstMeth)
    ( use_pos,
      "You cannot use this method with `inst_meth` (whether you are in the same class or not)."
    )
    [(def_pos, "It is declared as `private` here")]

let protected_inst_meth ~def_pos ~use_pos =
  add_list
    (Typing.err_code Typing.ProtectedInstMeth)
    ( use_pos,
      "You cannot use this method with `inst_meth` (whether you are in the same class hierarchy or not)."
    )
    [(def_pos, "It is declared as `protected` here")]

let private_meth_caller ~def_pos ~use_pos =
  add_list
    (Typing.err_code Typing.PrivateMethCaller)
    ( use_pos,
      "You cannot access this method with `meth_caller` (even from the same class hierarchy)"
    )
    [(def_pos, "It is declared as `private` here")]

let protected_meth_caller ~def_pos ~use_pos =
  add_list
    (Typing.err_code Typing.ProtectedMethCaller)
    ( use_pos,
      "You cannot access this method with `meth_caller` (even from the same class hierarchy)"
    )
    [(def_pos, "It is declared as `protected` here")]

let private_class_meth ~def_pos ~use_pos =
  add_list
    (Typing.err_code Typing.PrivateClassMeth)
    ( use_pos,
      "You cannot use this method with `class_meth` (whether you are in the same class or not)."
    )
    [(def_pos, "It is declared as `private` here")]

let protected_class_meth ~def_pos ~use_pos =
  add_list
    (Typing.err_code Typing.ProtectedClassMeth)
    ( use_pos,
      "You cannot use this method with `class_meth` (whether you are in the same class hierarchy or not)."
    )
    [(def_pos, "It is declared as `protected` here")]

let array_cast pos =
  add
    (Typing.err_code Typing.ArrayCast)
    pos
    "(array) cast forbidden; arrays with unspecified key and value types are not allowed"

let string_cast pos ty =
  add (Typing.err_code Typing.StringCast) pos
  @@ Printf.sprintf
       "Cannot cast a value of type %s to string. Only primitives may be used in a `(string)` cast."
       (Markdown_lite.md_codify ty)

let nullable_cast pos ty ty_pos =
  add_list
    (Typing.err_code Typing.NullableCast)
    (pos, "Casting from a nullable type is forbidden")
    [(ty_pos, "This is " ^ Markdown_lite.md_codify ty)]

let static_outside_class pos =
  add
    (Typing.err_code Typing.StaticOutsideClass)
    pos
    "`static` is undefined outside of a class"

let self_outside_class pos =
  add
    (Typing.err_code Typing.SelfOutsideClass)
    pos
    "`self` is undefined outside of a class"

let new_inconsistent_construct new_pos (cpos, cname) kind =
  let name = strip_ns cname in
  let preamble =
    match kind with
    | `static -> "Can't use `new static()` for " ^ Markdown_lite.md_codify name
    | `classname ->
      "Can't use `new` on " ^ Markdown_lite.md_codify ("classname<" ^ name ^ ">")
  in
  add_list
    (Typing.err_code Typing.NewStaticInconsistent)
    ( new_pos,
      preamble
      ^ "; `__construct` arguments are not guaranteed to be consistent in child classes"
    )
    [
      ( cpos,
        "This declaration is neither `final` nor uses the `<<__ConsistentConstruct>>` attribute"
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
    "`parent` is undefined outside of a class"

let parent_abstract_call meth_name call_pos decl_pos =
  add_list
    (Typing.err_code Typing.AbstractCall)
    ( call_pos,
      "Cannot call "
      ^ Markdown_lite.md_codify ("parent::" ^ meth_name ^ "()")
      ^ "; it is abstract" )
    [(decl_pos, "Declaration is here")]

let self_abstract_call meth_name call_pos decl_pos =
  add_list
    (Typing.err_code Typing.AbstractCall)
    ( call_pos,
      "Cannot call "
      ^ Markdown_lite.md_codify ("self::" ^ meth_name ^ "()")
      ^ "; it is abstract. Did you mean "
      ^ Markdown_lite.md_codify ("static::" ^ meth_name ^ "()")
      ^ "?" )
    [(decl_pos, "Declaration is here")]

let classname_abstract_call cname meth_name call_pos decl_pos =
  let cname = strip_ns cname in
  add_list
    (Typing.err_code Typing.AbstractCall)
    ( call_pos,
      "Cannot call "
      ^ Markdown_lite.md_codify (cname ^ "::" ^ meth_name ^ "()")
      ^ "; it is abstract" )
    [(decl_pos, "Declaration is here")]

let static_synthetic_method cname meth_name call_pos decl_pos =
  let cname = strip_ns cname in
  add_list
    (Typing.err_code Typing.StaticSyntheticMethod)
    ( call_pos,
      "Cannot call "
      ^ Markdown_lite.md_codify (cname ^ "::" ^ meth_name ^ "()")
      ^ "; "
      ^ Markdown_lite.md_codify meth_name
      ^ " is not defined in "
      ^ Markdown_lite.md_codify cname )
    [(decl_pos, "Declaration is here")]

let isset_in_strict pos =
  add
    (Typing.err_code Typing.IssetEmptyInStrict)
    pos
    ( "`isset` tends to hide errors due to variable typos and so is limited to dynamic checks in "
    ^ "`strict` mode" )

let unset_nonidx_in_strict pos msgs =
  add_list
    (Typing.err_code Typing.UnsetNonidxInStrict)
    ( pos,
      "In `strict` mode, `unset` is banned except on dynamic, "
      ^ "darray, keyset, or dict indexing" )
    msgs

let unpacking_disallowed_builtin_function pos name =
  let name = strip_ns name in
  add
    (Typing.err_code Typing.UnpackingDisallowed)
    pos
    ("Arg unpacking is disallowed for " ^ Markdown_lite.md_codify name)

let invalid_destructure pos1 pos2 ty (on_error : error_from_reasons_callback) =
  on_error
    ~code:(Typing.err_code Typing.InvalidDestructure)
    [
      ( pos1,
        "This expression cannot be destructured with a `list(...)` expression"
      );
      (pos2, "This is " ^ Markdown_lite.md_codify ty);
    ]

let unpack_array_required_argument p fp (on_error : error_from_reasons_callback)
    =
  on_error
    ~code:(Typing.err_code Typing.SplatArrayRequired)
    [
      ( p,
        "An array cannot be unpacked into the required arguments of a function"
      );
      (fp, "Definition is here");
    ]

let unpack_array_variadic_argument p fp (on_error : error_from_reasons_callback)
    =
  on_error
    ~code:(Typing.err_code Typing.SplatArrayRequired)
    [
      ( p,
        "A function that receives an unpacked array as an argument must have a variadic parameter to accept the elements of the array"
      );
      (fp, "Definition is here");
    ]

let array_get_arity pos1 name pos2 =
  add_list
    (Typing.err_code Typing.ArrayGetArity)
    (pos1, "You cannot use this " ^ (strip_ns name |> Markdown_lite.md_codify))
    [(pos2, "It is missing its type parameters")]

let typing_error pos msg = add (Typing.err_code Typing.GenericUnify) pos msg

let undefined_field ~use_pos ~name ~shape_type_pos =
  add_list
    (Typing.err_code Typing.UndefinedField)
    (use_pos, "The field " ^ Markdown_lite.md_codify name ^ " is undefined")
    [(shape_type_pos, "Definition is here")]

let array_access code pos1 pos2 ty =
  add_list
    (Typing.err_code code)
    (pos1, "This is not an object of type `KeyedContainer`, this is " ^ ty)
    ( if not (phys_equal pos2 Pos_or_decl.none) then
      [(pos2, "Definition is here")]
    else
      [] )

let array_access_read = array_access Typing.ArrayAccessRead

let array_access_write = array_access Typing.ArrayAccessWrite

let keyset_set pos1 pos2 =
  add_list
    (Typing.err_code Typing.KeysetSet)
    (pos1, "Elements in a keyset cannot be assigned, use append instead.")
    ( if not (phys_equal pos2 Pos_or_decl.none) then
      [(pos2, "Definition is here")]
    else
      [] )

let array_append pos1 pos2 ty =
  add_list
    (Typing.err_code Typing.ArrayAppend)
    (pos1, ty ^ " does not allow array append")
    ( if not (phys_equal pos2 Pos_or_decl.none) then
      [(pos2, "Definition is here")]
    else
      [] )

let const_mutation pos1 pos2 ty =
  add_list
    (Typing.err_code Typing.ConstMutation)
    (pos1, "You cannot mutate this")
    ( if not (phys_equal pos2 Pos_or_decl.none) then
      [(pos2, "This is " ^ ty)]
    else
      [] )

let expected_class ?(suffix = "") pos =
  add
    (Typing.err_code Typing.ExpectedClass)
    pos
    ("Was expecting a class" ^ suffix)

let unknown_type description pos r =
  let msg = "Was expecting " ^ description ^ " but type is unknown" in
  add_list (Typing.err_code Typing.UnknownType) (pos, msg) r

let not_found_hint orig hint =
  match hint with
  | `no_hint -> None
  | `closest (pos, v) ->
    Some (hint_message ~modifier:"static method " orig v pos)
  | `did_you_mean (pos, v) -> Some (hint_message orig v pos)

let snot_found_hint orig hint =
  match hint with
  | `no_hint -> None
  | `closest (pos, v) ->
    Some (hint_message ~modifier:"instance method " orig v pos)
  | `did_you_mean (pos, v) -> Some (hint_message orig v pos)

let string_of_class_member_kind = function
  | `class_constant -> "class constant"
  | `static_method -> "static method"
  | `class_variable -> "class variable"
  | `class_typeconst -> "type constant"
  | `method_ -> "method"
  | `property -> "property"

let smember_not_found_messages kind (cpos, class_name) member_name hint =
  let kind = string_of_class_member_kind kind in
  let class_name = strip_ns class_name in
  let msg =
    Printf.sprintf
      "No %s %s in %s"
      kind
      (Markdown_lite.md_codify member_name)
      (Markdown_lite.md_codify class_name)
  in
  let hint =
    match snot_found_hint member_name hint with
    | None -> []
    | Some hint -> [hint]
  in
  ( Typing.err_code Typing.SmemberNotFound,
    msg,
    hint
    @ [
        ( cpos,
          "Declaration of " ^ Markdown_lite.md_codify class_name ^ " is here" );
      ] )

let smember_not_found
    kind pos class_ member_name hint (on_error : typing_error_callback) =
  let (code, claim, reasons) =
    smember_not_found_messages kind class_ member_name hint
  in
  on_error ~code (pos, claim) reasons

let smember_not_found_
    kind pos class_ member_name hint (on_error : error_from_reasons_callback) =
  let (code, claim, reasons) =
    smember_not_found_messages kind class_ member_name hint
  in
  on_error ~code ((pos, claim) :: reasons)

let member_not_found
    kind
    pos
    (cpos, type_name)
    member_name
    hint
    reason
    (on_error : typing_error_callback) =
  let type_name = strip_ns type_name |> Markdown_lite.md_codify in
  let kind =
    match kind with
    | `method_ -> "instance method"
    | `property -> "property"
  in
  let msg =
    Printf.sprintf
      "No %s %s in %s"
      kind
      (Markdown_lite.md_codify member_name)
      type_name
  in
  on_error
    ~code:(Typing.err_code Typing.MemberNotFound)
    (pos, msg)
    (let hint =
       match not_found_hint member_name hint with
       | None -> []
       | Some hint -> [hint]
     in
     hint @ reason @ [(cpos, "Declaration of " ^ type_name ^ " is here")])

let expr_tree_unsupported_operator cls_name meth_name pos =
  let msg =
    match meth_name with
    | "__bool" ->
      (* If the user writes `if ($not_bool)`, provide a more specific
         error rather than a generic missing method error for
         `$not_bool->__bool()`. *)
      Printf.sprintf
        "`%s` cannot be used as a boolean (it has no instance method named `__bool`)"
        cls_name
    | _ ->
      (* If the user writes `$not_int + ...`, provide a more specific
         error rather than a generic missing method error for
         `$not_int->__plus(...)`. *)
      Printf.sprintf
        "`%s` does not support this operator (it has no instance method named `%s`)"
        cls_name
        meth_name
  in
  add (Typing.err_code Typing.MemberNotFound) pos msg

let parent_in_trait pos =
  add
    (Typing.err_code Typing.ParentInTrait)
    pos
    "You can only use `parent::` in traits that specify `require extends SomeClass`"

let parent_undefined pos =
  add (Typing.err_code Typing.ParentUndefined) pos "parent is undefined"

let constructor_no_args pos =
  add
    (Typing.err_code Typing.ConstructorNoArgs)
    pos
    "This constructor expects no argument"

let visibility p msg1 p_vis msg2 =
  add_list (Typing.err_code Typing.Visibility) (p, msg1) [(p_vis, msg2)]

let typing_too_many_args_error expected actual pos_def =
  ( Typing.err_code Typing.TypingTooManyArgs,
    Printf.sprintf "Too many arguments (expected %d but got %d)" expected actual,
    [(pos_def, "Definition is here")] )

let typing_too_many_args expected actual pos pos_def =
  let (code, claim, reasons) =
    typing_too_many_args_error expected actual pos_def
  in
  add_list code (pos, claim) reasons

let typing_too_many_args_w_callback :
    int ->
    int ->
    Pos_or_decl.t ->
    Pos_or_decl.t ->
    error_from_reasons_callback ->
    unit =
 fun expected actual pos pos_def on_error ->
  let (code, claim, reasons) =
    typing_too_many_args_error expected actual pos_def
  in
  on_error ~code ((pos, claim) :: reasons)

let typing_too_few_args_error required actual pos_def =
  ( Typing.err_code Typing.TypingTooFewArgs,
    Printf.sprintf "Too few arguments (required %d but got %d)" required actual,
    [(pos_def, "Definition is here")] )

let typing_too_few_args required actual pos pos_def =
  let (code, claim, reasons) =
    typing_too_few_args_error required actual pos_def
  in
  add_list code (pos, claim) reasons

let typing_too_few_args_w_callback :
    error_code ->
    error_code ->
    Pos_or_decl.t ->
    Pos_or_decl.t ->
    error_from_reasons_callback ->
    unit =
 fun required actual pos pos_def on_error ->
  let (code, claim, reasons) =
    typing_too_few_args_error required actual pos_def
  in
  on_error ~code ((pos, claim) :: reasons)

let bad_call pos ty =
  add
    (Typing.err_code Typing.BadCall)
    pos
    ("This call is invalid, this is not a function, it is " ^ ty)

let extend_final extend_pos decl_pos name =
  let name = strip_ns name in
  add_list
    (Typing.err_code Typing.ExtendFinal)
    (extend_pos, "You cannot extend final class " ^ Markdown_lite.md_codify name)
    [(decl_pos, "Declaration is here")]

let extend_non_abstract_record name extend_pos decl_pos =
  let name = strip_ns name in
  let msg =
    Printf.sprintf
      "Cannot extend record %s because it isn't abstract"
      (Markdown_lite.md_codify name)
  in
  add_list
    (Typing.err_code Typing.ExtendFinal)
    (extend_pos, msg)
    [(decl_pos, "Declaration is here")]

let extend_sealed child_pos parent_pos parent_name parent_kind verb =
  let name = strip_ns parent_name in
  add_list
    (Typing.err_code Typing.ExtendSealed)
    ( child_pos,
      "You cannot "
      ^ verb
      ^ " sealed "
      ^ parent_kind
      ^ " "
      ^ Markdown_lite.md_codify name )
    [(parent_pos, "Declaration is here")]

let sealed_not_subtype
    verb parent_pos child_pos parent_name child_name child_kind =
  let parent_name = strip_ns parent_name in
  let child_name = strip_ns child_name in
  add_list
    (Typing.err_code Typing.SealedNotSubtype)
    ( parent_pos,
      child_kind
      ^ " "
      ^ Markdown_lite.md_codify child_name
      ^ " in sealed whitelist for "
      ^ Markdown_lite.md_codify parent_name
      ^ ", but does not "
      ^ verb
      ^ " "
      ^ Markdown_lite.md_codify parent_name )
    [(child_pos, "Definition is here")]

let trait_prop_const_class pos x =
  add
    (Typing.err_code Typing.TraitPropConstClass)
    pos
    ( "Trait declaration of non-const property "
    ^ Markdown_lite.md_codify x
    ^ " is incompatible with a const class" )

let read_before_write (pos, v) =
  add
    (Typing.err_code Typing.ReadBeforeWrite)
    pos
    (Utils.sl
       [
         "Read access to ";
         Markdown_lite.md_codify ("$this->" ^ v);
         " before initialization";
       ])

let implement_abstract ~is_final pos1 pos2 kind x =
  let name = "abstract " ^ kind ^ " " ^ Markdown_lite.md_codify x in
  let msg1 =
    if is_final then
      "This class was declared as `final`. It must provide an implementation for the "
      ^ name
    else
      "This class must be declared `abstract`, or provide an implementation for the "
      ^ name
  in
  add_list
    (Typing.err_code Typing.ImplementAbstract)
    (pos1, msg1)
    [(pos2, "Declaration is here")]

let generic_static pos x =
  add
    (Typing.err_code Typing.GenericStatic)
    pos
    ( "This static variable cannot use the type parameter "
    ^ Markdown_lite.md_codify x
    ^ "." )

let fun_too_many_args
    required actual pos1 pos2 (on_error : error_from_reasons_callback) =
  on_error
    ~code:(Typing.err_code Typing.FunTooManyArgs)
    [
      ( pos1,
        Printf.sprintf
          "Too many mandatory arguments (expected %d but got %d)"
          required
          actual );
      (pos2, "Because of this definition");
    ]

let fun_too_few_args
    required actual pos1 pos2 (on_error : error_from_reasons_callback) =
  on_error
    ~code:(Typing.err_code Typing.FunTooFewArgs)
    [
      ( pos1,
        Printf.sprintf
          "Too few arguments (required %d but got %d)"
          required
          actual );
      (pos2, "Because of this definition");
    ]

let fun_unexpected_nonvariadic
    pos1 pos2 (on_error : error_from_reasons_callback) =
  on_error
    ~code:(Typing.err_code Typing.FunUnexpectedNonvariadic)
    [
      (pos1, "Should have a variadic argument");
      (pos2, "Because of this definition");
    ]

let fun_variadicity_hh_vs_php56
    pos1 pos2 (on_error : error_from_reasons_callback) =
  on_error
    ~code:(Typing.err_code Typing.FunVariadicityHhVsPhp56)
    [
      (pos1, "Variadic arguments: `...`-style is not a subtype of `...$args`");
      (pos2, "Because of this definition");
    ]

let ellipsis_strict_mode ~require pos =
  let msg =
    match require with
    | `Param_name ->
      "Variadic function arguments require a name in strict mode, e.g. `...$args`."
    | `Type_and_param_name ->
      "Variadic function arguments require a name and type in strict mode, e.g. `int ...$args`."
  in
  add (Typing.err_code Typing.EllipsisStrictMode) pos msg

let untyped_lambda_strict_mode pos =
  let msg =
    "Cannot determine types of lambda parameters in strict mode. Please add type hints on parameters."
  in
  add (Typing.err_code Typing.UntypedLambdaStrictMode) pos msg

let expected_tparam
    ~use_pos ~definition_pos n (on_error : error_from_reasons_callback option) =
  let claim =
    ( use_pos,
      "Expected "
      ^
      match n with
      | 0 -> "no type parameters"
      | 1 -> "exactly one type parameter"
      | n -> string_of_int n ^ " type parameters" )
  in
  let reasons = [(definition_pos, "Definition is here")] in
  on_error_or_add on_error (Typing.err_code Typing.ExpectedTparam) claim reasons

let object_string pos1 pos2 =
  add_list
    (Typing.err_code Typing.ObjectString)
    (pos1, "You cannot use this object as a string")
    [(pos2, "This object doesn't implement `__toString`")]

let object_string_deprecated pos =
  add
    (Typing.err_code Typing.ObjectString)
    pos
    "You cannot use this object as a string\nImplicit conversions of Stringish objects to string are deprecated."

let cyclic_typedef def_pos use_pos =
  add_list
    (Typing.err_code Typing.CyclicTypedef)
    (def_pos, "Cyclic type definition")
    [(use_pos, "Cyclic use is here")]

let type_arity_mismatch pos1 n1 pos2 n2 (on_error : error_from_reasons_callback)
    =
  on_error
    ~code:(Typing.err_code Typing.TypeArityMismatch)
    [(pos1, "This type has " ^ n1 ^ " arguments"); (pos2, "This one has " ^ n2)]

let this_final (id_pos, id) pos2 =
  let n = strip_ns id |> Markdown_lite.md_codify in
  let message1 = "Since " ^ n ^ " is not final" in
  let message2 = "this might not be a " ^ n in
  [(id_pos, message1); (pos2, message2)]

let exact_class_final id pos2 =
  let n = strip_ns (snd id) |> Markdown_lite.md_codify in
  let message1 = "This requires the late-bound type to be exactly " ^ n in
  let message2 =
    "Since " ^ n ^ " is not final this might be an instance of a child class"
  in
  [(fst id, message1); (pos2, message2)]

let require_args_reify def_pos arg_pos =
  add_list
    (Typing.err_code Typing.RequireArgsReify)
    ( arg_pos,
      "All type arguments must be specified because a type parameter is reified"
    )
    [(def_pos, "Definition is here")]

let require_generic_explicit (def_pos, def_name) arg_pos =
  add_list
    (Typing.err_code Typing.RequireGenericExplicit)
    ( arg_pos,
      "Generic type parameter "
      ^ Markdown_lite.md_codify def_name
      ^ " must be specified explicitly" )
    [(def_pos, "Definition is here")]

let invalid_reified_argument (def_pos, def_name) hint_pos arg_info =
  let (arg_pos, arg_kind) = List.hd_exn arg_info in
  add_list
    (Typing.err_code Typing.InvalidReifiedArgument)
    (hint_pos, "Invalid reified hint")
    [
      ( arg_pos,
        "This is " ^ arg_kind ^ ", it cannot be used as a reified type argument"
      );
      (def_pos, Markdown_lite.md_codify def_name ^ " is reified");
    ]

let invalid_reified_argument_reifiable (def_pos, def_name) arg_pos ty_pos ty_msg
    =
  add_list
    (Typing.err_code Typing.InvalidReifiedArgument)
    (arg_pos, "PHP arrays cannot be used as a reified type argument")
    [
      (ty_pos, String.capitalize ty_msg);
      (def_pos, Markdown_lite.md_codify def_name ^ " is reified");
    ]

let new_class_reified pos class_type suggested_class =
  let suggestion =
    match suggested_class with
    | Some s ->
      let s = strip_ns s in
      sprintf ". Try `new %s` instead." s
    | None -> ""
  in
  add
    (Typing.err_code Typing.NewClassReified)
    pos
    (sprintf
       "Cannot call `new %s` because the current class has reified generics%s"
       class_type
       suggestion)

let class_get_reified pos =
  add
    (Typing.err_code Typing.ClassGetReified)
    pos
    "Cannot access static properties on reified generics"

let static_meth_with_class_reified_generic meth_pos generic_pos =
  add_list
    (Typing.err_code Typing.StaticMethWithClassReifiedGeneric)
    ( meth_pos,
      "Static methods cannot use generics reified at the class level. Try reifying them at the static method itself."
    )
    [
      ( Pos_or_decl.of_raw_pos generic_pos,
        "Class-level reified generic used here." );
    ]

let consistent_construct_reified pos =
  add
    (Typing.err_code Typing.ConsistentConstructReified)
    pos
    "This class or one of its ancestors is annotated with `<<__ConsistentConstruct>>`. It cannot have reified generics."

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
    ( Markdown_lite.md_codify name
    ^ " cannot be used with `new` because it does not have the `<<__Newable>>` attribute"
    )

let discarded_awaitable pos1 pos2 =
  add_list
    (Typing.err_code Typing.DiscardedAwaitable)
    ( pos1,
      "This expression is of type `Awaitable`, but it's "
      ^ "either being discarded or used in a dangerous way before "
      ^ "being awaited" )
    [(pos2, "This is why I think it is `Awaitable`")]

let ignore_error : error_from_reasons_callback = (fun ?code:_ _ -> ())

let unify_error ?code claim reasons =
  add_list
    (Option.value code ~default:(Typing.err_code Typing.UnifyError))
    claim
    reasons

let unify_error_at : Pos.t -> error_from_reasons_callback =
 (fun pos ?code reasons -> unify_error ?code (pos, "Typing error") reasons)

let rigid_tvar_escape_at : Pos.t -> string -> error_from_reasons_callback =
 fun pos what ?code:_ reasons ->
  add_list
    (Typing.err_code Typing.RigidTVarEscape)
    (pos, "Rigid type variable escapes its " ^ what)
    reasons

let invalid_type_hint : Pos.t -> error_from_reasons_callback =
 fun pos ?code reasons ->
  add_list
    (Option.value code ~default:(Typing.err_code Typing.InvalidTypeHint))
    (pos, "Invalid type hint")
    reasons

let maybe_unify_error specific_code ?code errl =
  add_list (Option.value code ~default:(Typing.err_code specific_code)) errl

let index_type_mismatch = maybe_unify_error Typing.IndexTypeMismatch

let index_type_mismatch_at pos ?code reasons =
  add_list
    (Option.value code ~default:(Typing.err_code Typing.IndexTypeMismatch))
    (pos, "Invalid index expression")
    reasons

let expected_stringlike = maybe_unify_error Typing.ExpectedStringlike

let type_constant_mismatch :
    error_from_reasons_callback -> error_from_reasons_callback =
 fun (on_error : error_from_reasons_callback) ?code errl ->
  let code =
    Option.value code ~default:(Typing.err_code Typing.TypeConstantMismatch)
  in
  on_error ~code errl

let class_constant_type_mismatch
    (on_error : error_from_reasons_callback) ?code errl =
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

let missing_return = maybe_unify_error Typing.MissingReturnInNonVoidFunction

let inout_return_type_mismatch =
  maybe_unify_error Typing.InoutReturnTypeMismatch

let class_constant_value_does_not_match_hint =
  maybe_unify_error Typing.ClassConstantValueDoesNotMatchHint

let class_property_initializer_type_does_not_match_hint =
  maybe_unify_error Typing.ClassPropertyInitializerTypeDoesNotMatchHint

let xhp_attribute_does_not_match_hint =
  maybe_unify_error Typing.XhpAttributeValueDoesNotMatchHint

let record_init_value_does_not_match_hint =
  maybe_unify_error Typing.RecordInitValueDoesNotMatchHint

let strict_str_concat_type_mismatch =
  maybe_unify_error Typing.StrictStrConcatTypeMismatch

let strict_str_interp_type_mismatch =
  maybe_unify_error Typing.StrictStrInterpTypeMismatch

let bitwise_math_invalid_argument =
  maybe_unify_error Typing.BitwiseMathInvalidArgument

let inc_dec_invalid_argument = maybe_unify_error Typing.IncDecInvalidArgument

let math_invalid_argument = maybe_unify_error Typing.MathInvalidArgument

let using_error pos has_await ?code:_ claim reasons =
  let (note, cls) =
    if has_await then
      (" with await", Naming_special_names.Classes.cIAsyncDisposable)
    else
      ("", Naming_special_names.Classes.cIDisposable)
  in
  add_list
    (Typing.err_code Typing.UnifyError)
    ( pos,
      Printf.sprintf
        "This expression is used in a `using` clause%s so it must have type `%s`"
        note
        cls )
    (claim_as_reason claim :: reasons)

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
    ^ Markdown_lite.md_codify (dollar ^ member_name)
    ^ " is declared here as non-static"
  in
  let msg_static =
    "But it conflicts with an inherited static declaration here"
  in
  add_list
    (Typing.err_code Typing.StaticDynamic)
    (dyn_position, msg_dynamic)
    [(static_position, msg_static)]

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
    ^ Markdown_lite.md_codify (dollar ^ member_name)
    ^ " is declared here as static"
  in
  let msg_dynamic =
    "But it conflicts with an inherited non-static declaration here"
  in
  add_list
    (Typing.err_code Typing.StaticDynamic)
    (static_position, msg_static)
    [(dyn_position, msg_dynamic)]

let null_member code ~is_method s pos r =
  let msg =
    Printf.sprintf
      "You are trying to access the %s %s but this object can be null."
      ( if is_method then
        "method"
      else
        "property" )
      (Markdown_lite.md_codify s)
  in
  add_list (Typing.err_code code) (pos, msg) r

let null_member_read = null_member Typing.NullMemberRead

let null_member_write = null_member Typing.NullMemberWrite

(* Trying to access a member on a mixed or nonnull value. *)
let top_member null_code nonnull_code ~is_method ~is_nullable s pos1 ty pos2 =
  let msg =
    Printf.sprintf
      "You are trying to access the %s %s but this is %s. Use a **specific** class or interface name."
      ( if is_method then
        "method"
      else
        "property" )
      (Markdown_lite.md_codify s)
      ty
  in
  add_list
    (Typing.err_code
       ( if is_nullable then
         null_code
       else
         nonnull_code ))
    (pos1, msg)
    [(pos2, "Definition is here")]

let top_member_read =
  top_member Typing.NullMemberRead Typing.NonObjectMemberRead

let top_member_write =
  top_member Typing.NullMemberWrite Typing.NonObjectMemberWrite

let non_object_member_messages kind s ty pos2 =
  let msg_start =
    Printf.sprintf
      "You are trying to access the %s %s but this is %s"
      (string_of_class_member_kind kind)
      (Markdown_lite.md_codify s)
      ty
  in
  let msg =
    if String.equal ty "a shape" then
      msg_start ^ ". Did you mean `$foo['" ^ s ^ "']` instead?"
    else
      msg_start
  in
  (msg, [(pos2, "Definition is here")])

let non_object_member_
    code ~kind s pos1 ty pos2 (on_error : error_from_reasons_callback) =
  let (claim, reasons) = non_object_member_messages kind s ty pos2 in
  on_error ~code:(Typing.err_code code) ((pos1, claim) :: reasons)

let non_object_member
    code ~kind s pos1 ty pos2 (on_error : typing_error_callback) =
  let (claim, reasons) = non_object_member_messages kind s ty pos2 in
  on_error ~code:(Typing.err_code code) (pos1, claim) reasons

let non_object_member_read_ = non_object_member_ Typing.NonObjectMemberRead

let non_object_member_write_ = non_object_member_ Typing.NonObjectMemberRead

let non_object_member_read = non_object_member Typing.NonObjectMemberRead

let non_object_member_write = non_object_member Typing.NonObjectMemberRead

let unknown_object_member ~is_method s pos r =
  let msg =
    Printf.sprintf
      "You are trying to access the %s %s on a value whose class is unknown."
      ( if is_method then
        "method"
      else
        "property" )
      (Markdown_lite.md_codify s)
  in
  add_list (Typing.err_code Typing.UnknownObjectMember) (pos, msg) r

let non_class_member ~is_method s pos1 ty pos2 =
  let msg =
    Printf.sprintf
      "You are trying to access the static %s %s but this is %s"
      ( if is_method then
        "method"
      else
        "property" )
      (Markdown_lite.md_codify s)
      ty
  in
  add_list
    (Typing.err_code Typing.NonClassMember)
    (pos1, msg)
    [(pos2, "Definition is here")]

let null_container p null_witness =
  add_list
    (Typing.err_code Typing.NullContainer)
    ( p,
      "You are trying to access an element of this container"
      ^ " but the container could be `null`. " )
    null_witness

let option_mixed pos =
  add
    (Typing.err_code Typing.OptionMixed)
    pos
    "`?mixed` is a redundant typehint - just use `mixed`"

let option_null pos =
  add
    (Typing.err_code Typing.OptionNull)
    pos
    "`?null` is a redundant typehint - just use `null`"

let declared_covariant pos1 pos2 emsg =
  add_list
    (Typing.err_code Typing.DeclaredCovariant)
    (pos2, "Illegal usage of a covariant type parameter")
    ( [
        ( Pos_or_decl.of_raw_pos pos1,
          "This is where the parameter was declared as covariant `+`" );
      ]
    @ List.map emsg ~f:claim_as_reason )

let declared_contravariant pos1 pos2 emsg =
  add_list
    (Typing.err_code Typing.DeclaredContravariant)
    (pos2, "Illegal usage of a contravariant type parameter")
    ( [
        ( Pos_or_decl.of_raw_pos pos1,
          "This is where the parameter was declared as contravariant `-`" );
      ]
    @ List.map emsg ~f:claim_as_reason )

let static_property_type_generic_param ~class_pos ~var_type_pos ~generic_pos =
  add_list
    (Typing.err_code Typing.ClassVarTypeGenericParam)
    ( generic_pos,
      "A generic parameter cannot be used in the type of a static property" )
    [
      ( var_type_pos,
        "This is where the type of the static property was declared" );
      (class_pos, "This is the class containing the static property");
    ]

let contravariant_this pos class_name tp =
  add
    (Typing.err_code Typing.ContravariantThis)
    pos
    ( "The `this` type cannot be used in this "
    ^ "contravariant position because its enclosing class "
    ^ Markdown_lite.md_codify class_name
    ^ " "
    ^ "is final and has a variant type parameter "
    ^ Markdown_lite.md_codify tp )

let cyclic_typeconst pos sl =
  let sl = List.map sl ~f:(fun s -> strip_ns s |> Markdown_lite.md_codify) in
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
  add_error_assert_primary_pos_in_current_decl
    (Typing.err_code Typing.AbstractConcreteOverride)
    [
      (pos, "Cannot re-declare this " ^ kind_str ^ " as abstract");
      (parent_pos, "Previously defined here");
    ]

let required_field_is_optional
    pos1 pos2 name (on_error : error_from_reasons_callback) =
  on_error
    ~code:(Typing.err_code Typing.RequiredFieldIsOptional)
    [
      (pos1, "The field " ^ Markdown_lite.md_codify name ^ " is **optional**");
      ( pos2,
        "The field "
        ^ Markdown_lite.md_codify name
        ^ " is defined as **required**" );
    ]

let array_get_with_optional_field pos1 pos2 name =
  add_list
    (Typing.err_code Typing.ArrayGetWithOptionalField)
    ( pos1,
      Printf.sprintf
        "The field %s may not be present in this shape. Use `Shapes::idx()` instead."
        (Markdown_lite.md_codify name) )
    [(pos2, "This is where the field was declared as optional.")]

let return_disposable_mismatch
    pos1_return_disposable pos1 pos2 (on_error : error_from_reasons_callback) =
  let m1 = "This is marked `<<__ReturnDisposable>>`." in
  let m2 = "This is not marked `<<__ReturnDisposable>>`." in
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

let ifc_policy_mismatch
    pos_sub
    pos_super
    policy_sub
    policy_super
    (on_error : error_from_reasons_callback) =
  let m1 =
    "IFC policies must be invariant with respect to inheritance. This method is policied with "
    ^ policy_sub
  in
  let m2 =
    "This is incompatible with its inherited policy, which is " ^ policy_super
  in
  on_error
    ~code:(Typing.err_code Typing.IFCPolicyMismatch)
    [(pos_sub, m1); (pos_super, m2)]

let this_as_lexical_variable pos =
  add
    (Naming.err_code Naming.ThisAsLexicalVariable)
    pos
    "Cannot use `$this` as lexical variable"

let dollardollar_lvalue pos =
  add
    (Typing.err_code Typing.DollardollarLvalue)
    pos
    "Cannot assign a value to the special pipe variable `$$`"

let mutating_const_property pos =
  add
    (Typing.err_code Typing.AssigningToConst)
    pos
    "Cannot mutate a `__Const` property"

let self_const_parent_not pos =
  add
    (Typing.err_code Typing.SelfConstParentNot)
    pos
    "A `__Const` class may only extend other `__Const` classes"

let overriding_prop_const_mismatch
    parent_pos
    parent_const
    child_pos
    child_const
    (on_error : error_from_reasons_callback) =
  let m1 = "This property is `__Const`" in
  let m2 = "This property is not `__Const`" in
  on_error
    ~code:(Typing.err_code Typing.OverridingPropConstMismatch)
    [
      ( child_pos,
        if child_const then
          m1
        else
          m2 );
      ( parent_pos,
        if parent_const then
          m1
        else
          m2 );
    ]

let php_lambda_disallowed pos =
  add
    (NastCheck.err_code NastCheck.PhpLambdaDisallowed)
    pos
    "PHP style anonymous functions are not allowed."

(*****************************************************************************)
(* Typing decl errors *)
(*****************************************************************************)

let wrong_extend_kind
    ~parent_pos
    ~parent_kind
    ~parent_name
    ~parent_is_enum_class
    ~child_pos
    ~child_kind
    ~child_name
    ~child_is_enum_class =
  let parent_kind_str =
    Ast_defs.string_of_class_kind
      parent_kind
      ~is_enum_class:parent_is_enum_class
  in
  let parent_name = strip_ns parent_name in
  let child_name = strip_ns child_name in
  let use_msg =
    Printf.sprintf
      " Did you mean to add `use %s;` within the body of %s?"
      parent_name
      (Markdown_lite.md_codify child_name)
  in
  let child_msg =
    match child_kind with
    | Ast_defs.Cabstract
    | Ast_defs.Cnormal ->
      let extends_msg = "Classes can only extend other classes." in
      let suggestion =
        if Ast_defs.is_c_interface parent_kind then
          " Did you mean `implements " ^ parent_name ^ "`?"
        else if Ast_defs.is_c_trait parent_kind then
          use_msg
        else
          ""
      in
      extends_msg ^ suggestion
    | Ast_defs.Cinterface ->
      let extends_msg = "Interfaces can only extend other interfaces." in
      let suggestion =
        if Ast_defs.is_c_trait parent_kind then
          use_msg
        else
          ""
      in
      extends_msg ^ suggestion
    | Ast_defs.Cenum ->
      if child_is_enum_class then
        "Enum classes can only extend other enum classes."
      else
        (* This case should never happen, as the type checker will have already caught
          it with EnumTypeBad. But just in case, report this error here too. *)
        "Enums can only extend int, string, or arraykey."
    | Ast_defs.Ctrait ->
      (* This case should never happen, as the parser will have caught it before
          we get here. *)
      "A trait cannot use `extends`. This is a parser error."
  in
  let msg1 = (child_pos, child_msg) in
  let msg2 = (parent_pos, "This is " ^ parent_kind_str ^ ".") in
  add_list (Typing.err_code Typing.WrongExtendKind) msg1 [msg2]

let unsatisfied_req_callback
    ~class_pos ~trait_pos ~req_pos req_name ?code:_ reasons =
  add_list
    (Typing.err_code Typing.UnsatisfiedReq)
    ( class_pos,
      "This class does not satisfy all the requirements of its traits or interfaces."
    )
    ( (trait_pos, "This requires to extend or implement " ^ strip_ns req_name)
      ::
      ( if Pos_or_decl.equal req_pos trait_pos then
        []
      else
        [(req_pos, "Required here")] )
    @ reasons )

let unsatisfied_req ~class_pos ~trait_pos ~req_pos req_name =
  unsatisfied_req_callback ~class_pos ~trait_pos ~req_pos req_name ?code:None []

let cyclic_class_def stack pos =
  let stack =
    SSet.fold
      ~f:(fun x y -> (strip_ns x |> Markdown_lite.md_codify) ^ " " ^ y)
      stack
      ~init:""
  in
  add
    (Typing.err_code Typing.CyclicClassDef)
    pos
    ("Cyclic class definition : " ^ stack)

let cyclic_record_def names pos =
  let names =
    List.map ~f:(fun n -> strip_ns n |> Markdown_lite.md_codify) names
  in
  add
    (Typing.err_code Typing.CyclicRecordDef)
    pos
    (Printf.sprintf
       "Record inheritance cycle: %s"
       (String.concat ~sep:" " names))

let trait_reuse_with_final_method use_pos trait_name parent_cls_name trace =
  let msg =
    Printf.sprintf
      "Traits with final methods cannot be reused, and `%s` is already used by `%s`."
      (strip_ns trait_name)
      (strip_ns parent_cls_name)
  in
  add_list (Typing.err_code Typing.TraitReuse) (use_pos, msg) trace

let trait_reuse p_pos p_name class_name trait =
  let (c_pos, c_name) = class_name in
  let c_name = strip_ns c_name |> Markdown_lite.md_codify in
  let trait = strip_ns trait |> Markdown_lite.md_codify in
  let err =
    "Class " ^ c_name ^ " reuses trait " ^ trait ^ " in its hierarchy"
  in
  let err' =
    "It is already used through " ^ (strip_ns p_name |> Markdown_lite.md_codify)
  in
  add_list (Typing.err_code Typing.TraitReuse) (c_pos, err) [(p_pos, err')]

let trait_reuse_inside_class class_name trait occurrences =
  let (c_pos, c_name) = class_name in
  let c_name = strip_ns c_name |> Markdown_lite.md_codify in
  let trait = strip_ns trait |> Markdown_lite.md_codify in
  let err = "Class " ^ c_name ^ " uses trait " ^ trait ^ " multiple times" in
  add_list
    (Typing.err_code Typing.TraitReuseInsideClass)
    (c_pos, err)
    (List.map ~f:(fun p -> (p, "used here")) occurrences)

let invalid_is_as_expression_hint op hint_pos reasons =
  add_list
    (Typing.err_code Typing.InvalidIsAsExpressionHint)
    (hint_pos, "Invalid " ^ Markdown_lite.md_codify op ^ " expression hint")
    (List.map reasons ~f:(fun (ty_pos, ty_str) ->
         ( ty_pos,
           "The "
           ^ Markdown_lite.md_codify op
           ^ " operator cannot be used with "
           ^ ty_str )))

let invalid_enforceable_type kind_str (tp_pos, tp_name) targ_pos ty_info =
  let (ty_pos, ty_str) = List.hd_exn ty_info in
  add_list
    (Typing.err_code Typing.InvalidEnforceableTypeArgument)
    (targ_pos, "Invalid type")
    [
      ( tp_pos,
        "Type "
        ^ kind_str
        ^ " "
        ^ Markdown_lite.md_codify tp_name
        ^ " was declared `__Enforceable` here" );
      (ty_pos, "This type is not enforceable because it has " ^ ty_str);
    ]

let reifiable_attr attr_pos decl_kind decl_pos ty_info =
  let (ty_pos, ty_msg) = List.hd_exn ty_info in
  add_list
    (Typing.err_code Typing.DisallowPHPArraysAttr)
    (decl_pos, "Invalid " ^ decl_kind)
    [
      (attr_pos, "This type constant has the `__Reifiable` attribute");
      (ty_pos, "It cannot contain " ^ ty_msg);
    ]

let invalid_newable_type_argument (tp_pos, tp_name) ta_pos =
  add_list
    (Typing.err_code Typing.InvalidNewableTypeArgument)
    ( ta_pos,
      "A newable type argument must be a concrete class or a newable type parameter."
    )
    [
      ( tp_pos,
        "Type parameter "
        ^ Markdown_lite.md_codify tp_name
        ^ " was declared `__Newable` here" );
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
    ^ Markdown_lite.md_codify tparam_name
    ^ " has the `<<__Newable>>` attribute. "
    ^ "Newable type parameters must be constrained with `as`, and exactly one of those constraints must be a valid newable class. "
    ^ "The class must either be final, or it must have the `<<__ConsistentConstruct>>` attribute or extend a class that has it. "
    ^ partial
    ^ " are valid newable classes"
  in
  add (Typing.err_code Typing.InvalidNewableTypeParamConstraints) tparam_pos msg

let override_final ~parent ~child ~(on_error : error_from_reasons_callback) =
  let msg1 = (child, "You cannot override this method") in
  let msg2 = (parent, "It was declared as final") in
  on_error ~code:(Typing.err_code Typing.OverrideFinal) [msg1; msg2]

let override_lsb
    ~member_name ~parent ~child (on_error : error_from_reasons_callback) =
  on_error
    ~code:(Typing.err_code Typing.OverrideLSB)
    [
      ( child,
        "Member "
        ^ Markdown_lite.md_codify member_name
        ^ " may not override `__LSB` member of parent" );
      (parent, "This is being overridden");
    ]

let should_be_override pos class_id id =
  add_error_assert_primary_pos_in_current_decl
    (Typing.err_code Typing.ShouldBeOverride)
    [
      ( pos,
        Printf.sprintf
          "%s has no parent class with a method %s to override"
          (strip_ns class_id |> Markdown_lite.md_codify)
          (Markdown_lite.md_codify id) );
    ]

let override_per_trait class_name meth_name trait_name m_pos =
  let (c_pos, c_name) = class_name in
  let err_msg =
    Printf.sprintf
      "`%s::%s` is marked `__Override` but `%s` does not define or inherit a `%s` method."
      (strip_ns trait_name)
      meth_name
      (strip_ns c_name)
      meth_name
  in
  add_list
    (Typing.err_code Typing.OverridePerTrait)
    (c_pos, err_msg)
    [
      (m_pos, "Declaration of " ^ Markdown_lite.md_codify meth_name ^ " is here");
    ]

let missing_assign pos =
  add (Typing.err_code Typing.MissingAssign) pos "Please assign a value"

let invalid_memoized_param pos ty_reason_msg =
  add_list
    (Typing.err_code Typing.InvalidMemoizedParam)
    ( pos,
      "Parameters to memoized function must be null, bool, int, float, string, an object deriving IMemoizeParam, or a Container thereof. See also http://docs.hhvm.com/hack/attributes/special#__memoize"
    )
    ty_reason_msg

let invalid_disposable_hint pos class_name =
  add
    (Typing.err_code Typing.InvalidDisposableHint)
    pos
    ( "Parameter with type "
    ^ Markdown_lite.md_codify class_name
    ^ " must not implement `IDisposable` or `IAsyncDisposable`. "
    ^ "Please use `<<__AcceptDisposable>>` attribute or create disposable object with `using` statement instead."
    )

let invalid_disposable_return_hint pos class_name =
  add
    (Typing.err_code Typing.InvalidDisposableReturnHint)
    pos
    ( "Return type "
    ^ Markdown_lite.md_codify class_name
    ^ " must not implement `IDisposable` or `IAsyncDisposable`. Please add `<<__ReturnDisposable>>` attribute."
    )

let xhp_required pos why_xhp ty_reason_msg =
  let msg = "An XHP instance was expected" in
  add_list
    (Typing.err_code Typing.XhpRequired)
    (pos, msg)
    ((Pos_or_decl.of_raw_pos pos, why_xhp) :: ty_reason_msg)

let illegal_xhp_child pos ty_reason_msg =
  let msg = "XHP children must be compatible with XHPChild" in
  add_list (Typing.err_code Typing.IllegalXhpChild) (pos, msg) ty_reason_msg

let missing_xhp_required_attr pos attr ty_reason_msg =
  let msg =
    "Required attribute " ^ Markdown_lite.md_codify attr ^ " is missing."
  in
  add_list
    (Typing.err_code Typing.MissingXhpRequiredAttr)
    (pos, msg)
    ty_reason_msg

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
  let left_trail = List.map left_trail ~f:typedef_trail_entry in
  let right_trail = List.map right_trail ~f:typedef_trail_entry in
  add_list
    (Typing.err_code Typing.TrivialStrictEq)
    (p, msg)
    (left @ left_trail @ right @ right_trail)

let trivial_strict_not_nullable_compare_null p result type_reason =
  let msg = "This expression is always " ^ result in
  add_list
    (Typing.err_code Typing.NotNullableCompareNullTrivial)
    (p, msg)
    type_reason

let eq_incompatible_types p left right =
  let msg = "This equality test has incompatible types" in
  add_list (Typing.err_code Typing.EqIncompatibleTypes) (p, msg) (left @ right)

let comparison_invalid_types p left right =
  let msg =
    "This comparison has invalid types.  Only comparisons in which both arguments are strings, nums, DateTime, or DateTimeImmutable are allowed"
  in
  add_list
    (Typing.err_code Typing.ComparisonInvalidTypes)
    (p, msg)
    (left @ right)

let strict_eq_value_incompatible_types p left right =
  let msg =
    "The arguments to this value equality test are not the same types or are not the allowed types (int, bool, float, string, vec, keyset, dict). The behavior for this test is changing and will soon either be universally false or throw an exception."
  in
  add_list
    (Typing.err_code Typing.StrictEqValueIncompatibleTypes)
    (p, msg)
    (left @ right)

let void_usage p void_witness =
  let msg = "You are using the return value of a `void` function" in
  add_list (Typing.err_code Typing.VoidUsage) (p, msg) void_witness

let noreturn_usage p noreturn_witness =
  let msg = "You are using the return value of a `noreturn` function" in
  add_list (Typing.err_code Typing.NoreturnUsage) (p, msg) noreturn_witness

let attribute_too_few_arguments pos x n =
  let n = string_of_int n in
  add
    (Typing.err_code Typing.AttributeTooFewArguments)
    pos
    ( "The attribute "
    ^ Markdown_lite.md_codify x
    ^ " expects at least "
    ^ n
    ^ " arguments" )

let attribute_too_many_arguments pos x n =
  let n = string_of_int n in
  add
    (Typing.err_code Typing.AttributeTooManyArguments)
    pos
    ( "The attribute "
    ^ Markdown_lite.md_codify x
    ^ " expects at most "
    ^ n
    ^ " arguments" )

let attribute_param_type pos x =
  add
    (Typing.err_code Typing.AttributeParamType)
    pos
    ("This attribute parameter should be " ^ x)

let deprecated_use pos ?(pos_def = None) msg =
  let def_message =
    match pos_def with
    | Some pos_def -> [(pos_def, "Definition is here")]
    | None -> []
  in
  add_list (Typing.err_code Typing.DeprecatedUse) (pos, msg) def_message

let cannot_declare_constant kind pos (class_pos, class_name) =
  let kind_str =
    match kind with
    | `enum -> "an enum"
    | `record -> "a record"
  in
  add_list
    (Typing.err_code Typing.CannotDeclareConstant)
    (pos, "Cannot declare a constant in " ^ kind_str)
    [
      ( Pos_or_decl.of_raw_pos class_pos,
        (strip_ns class_name |> Markdown_lite.md_codify)
        ^ " was defined as "
        ^ kind_str
        ^ " here" );
    ]

let ambiguous_inheritance
    pos class_ origin error (on_error : error_from_reasons_callback) =
  let { code; claim; reasons } = error in
  let origin = strip_ns origin in
  let class_ = strip_ns class_ in
  let message =
    "This declaration was inherited from an object of type "
    ^ Markdown_lite.md_codify origin
    ^ ". Redeclare this member in "
    ^ Markdown_lite.md_codify class_
    ^ " with a compatible signature."
  in
  on_error ~code ((claim_as_reason claim :: reasons) @ [(pos, message)])

let multiple_concrete_defs
    child_pos
    parent_pos
    child_origin
    parent_origin
    name
    class_
    (on_error : error_from_reasons_callback) =
  let child_origin = strip_ns child_origin in
  let parent_origin = strip_ns parent_origin in
  let class_ = strip_ns class_ in
  on_error
    ~code:(Typing.err_code Typing.MultipleConcreteDefs)
    [
      ( child_pos,
        Markdown_lite.md_codify child_origin
        ^ " and "
        ^ Markdown_lite.md_codify parent_origin
        ^ " both declare ambiguous implementations of "
        ^ Markdown_lite.md_codify name
        ^ "." );
      ( child_pos,
        Markdown_lite.md_codify child_origin ^ "'s definition is here." );
      ( parent_pos,
        Markdown_lite.md_codify parent_origin ^ "'s definition is here." );
      ( child_pos,
        "Redeclare "
        ^ Markdown_lite.md_codify name
        ^ " in "
        ^ Markdown_lite.md_codify class_
        ^ " with a compatible signature." );
    ]

let local_variable_modified_and_used pos_modified pos_used_l =
  let used_msg p = (Pos_or_decl.of_raw_pos p, "And accessed here") in
  add_list
    (Typing.err_code Typing.LocalVariableModifedAndUsed)
    ( pos_modified,
      "Unsequenced modification and access to local variable. Modified here" )
    (List.map pos_used_l ~f:used_msg)

let local_variable_modified_twice pos_modified pos_modified_l =
  let modified_msg p = (Pos_or_decl.of_raw_pos p, "And also modified here") in
  add_list
    (Typing.err_code Typing.LocalVariableModifedTwice)
    (pos_modified, "Unsequenced modifications to local variable. Modified here")
    (List.map pos_modified_l ~f:modified_msg)

let assign_during_case p =
  add
    (Typing.err_code Typing.AssignDuringCase)
    p
    "Don't assign to variables inside of case labels"

let cyclic_enum_constraint pos (on_error : error_from_reasons_callback) =
  on_error
    ~code:(Typing.err_code Typing.CyclicEnumConstraint)
    [(pos, "Cyclic enum constraint")]

let invalid_classname p =
  add (Typing.err_code Typing.InvalidClassname) p "Not a valid class name"

let illegal_type_structure pos errmsg =
  let msg =
    "The two arguments to `type_structure()` must be:"
    ^ "\n - first: `ValidClassname::class` or an object of that class"
    ^ "\n - second: a single-quoted string literal containing the name"
    ^ " of a type constant of that class"
    ^ "\n"
    ^ errmsg
  in
  add (Typing.err_code Typing.IllegalTypeStructure) pos msg

let illegal_typeconst_direct_access pos =
  let msg =
    "Type constants cannot be directly accessed. "
    ^ "Use `type_structure(ValidClassname::class, 'TypeConstName')` instead"
  in
  add (Typing.err_code Typing.IllegalTypeStructure) pos msg

let override_no_default_typeconst pos_child pos_parent =
  add_error_assert_primary_pos_in_current_decl
    (Typing.err_code Typing.OverrideNoDefaultTypeconst)
    [
      (pos_child, "This abstract type constant does not have a default type");
      ( pos_parent,
        "It cannot override an abstract type constant that has a default type"
      );
    ]

let inout_annotation_missing pos1 pos2 =
  let msg1 = (pos1, "This argument should be annotated with `inout`") in
  let msg2 = (pos2, "Because this is an `inout` parameter") in
  add_list (Typing.err_code Typing.InoutAnnotationMissing) msg1 [msg2]

let inout_annotation_unexpected pos1 pos2 pos2_is_variadic =
  let msg1 = (pos1, "Unexpected `inout` annotation for argument") in
  let msg2 =
    ( pos2,
      if pos2_is_variadic then
        "A variadic parameter can never be `inout`"
      else
        "This is a normal parameter (does not have `inout`)" )
  in
  add_list (Typing.err_code Typing.InoutAnnotationUnexpected) msg1 [msg2]

let inoutness_mismatch pos1 pos2 (on_error : error_from_reasons_callback) =
  let msg1 = (pos1, "This is an `inout` parameter") in
  let msg2 = (pos2, "It is incompatible with a normal parameter") in
  on_error ~code:(Typing.err_code Typing.InoutnessMismatch) [msg1; msg2]

let invalid_new_disposable pos =
  let msg =
    "Disposable objects may only be created in a `using` statement or `return` from function marked `<<__ReturnDisposable>>`"
  in
  add (Typing.err_code Typing.InvalidNewDisposable) pos msg

let invalid_return_disposable pos =
  let msg =
    "Return expression must be new disposable in function marked `<<__ReturnDisposable>>`"
  in
  add (Typing.err_code Typing.InvalidReturnDisposable) pos msg

let inout_argument_bad_type pos msgl =
  let msg =
    "Expected argument marked `inout` to be contained in a local or "
    ^ "a value-typed container (e.g. vec, dict, keyset, array). "
    ^ "To use `inout` here, assign to/from a temporary local variable."
  in
  add_list (Typing.err_code Typing.InoutArgumentBadType) (pos, msg) msgl

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
    (pos, msg1)
    ( (Pos_or_decl.of_raw_pos pos, msg2)
    :: List.map uses ~f:(fun (pos, ty) ->
           (pos, "This use has type " ^ Markdown_lite.md_codify ty)) )

let wrong_expression_kind_attribute
    expr_kind pos attr attr_class_pos attr_class_name intf_name =
  let msg1 =
    Printf.sprintf
      "The %s attribute cannot be used on %s."
      (strip_ns attr |> Markdown_lite.md_codify)
      expr_kind
  in
  let msg2 =
    Printf.sprintf
      "The attribute's class is defined here. To be available for use on %s, the %s class must implement %s."
      expr_kind
      (strip_ns attr_class_name |> Markdown_lite.md_codify)
      (strip_ns intf_name |> Markdown_lite.md_codify)
  in
  add_list
    (Typing.err_code Typing.WrongExpressionKindAttribute)
    (pos, msg1)
    [(attr_class_pos, msg2)]

let wrong_expression_kind_builtin_attribute expr_kind pos attr =
  let msg1 =
    Printf.sprintf
      "The %s attribute cannot be used on %s."
      (strip_ns attr |> Markdown_lite.md_codify)
      expr_kind
  in
  add_list (Typing.err_code Typing.WrongExpressionKindAttribute) (pos, msg1) []

let decl_override_missing_hint pos (on_error : error_from_reasons_callback) =
  on_error
    ~code:(Typing.err_code Typing.DeclOverrideMissingHint)
    [
      ( pos,
        "When redeclaring class members, both declarations must have a typehint"
      );
    ]

let shapes_key_exists_always_true pos1 name pos2 =
  add_list
    (Typing.err_code Typing.ShapesKeyExistsAlwaysTrue)
    (pos1, "This `Shapes::keyExists()` check is always true")
    [
      ( pos2,
        "The field "
        ^ Markdown_lite.md_codify name
        ^ " exists because of this definition" );
    ]

let shape_field_non_existence_reason pos name = function
  | `Undefined ->
    [
      ( pos,
        "The field "
        ^ Markdown_lite.md_codify name
        ^ " is not defined in this shape" );
    ]
  | `Nothing reason ->
    ( pos,
      "The type of the field "
      ^ Markdown_lite.md_codify name
      ^ " in this shape doesn't allow any values" )
    :: reason

let shapes_key_exists_always_false pos1 name pos2 reason =
  add_list
    (Typing.err_code Typing.ShapesKeyExistsAlwaysFalse)
    (pos1, "This `Shapes::keyExists()` check is always false")
  @@ shape_field_non_existence_reason pos2 name reason

let shapes_method_access_with_non_existent_field
    pos1 name pos2 method_name reason =
  add_list
    (Typing.err_code Typing.ShapesMethodAccessWithNonExistentField)
    ( pos1,
      "You are calling "
      ^ Markdown_lite.md_codify ("Shapes::" ^ method_name ^ "()")
      ^ " on a field known to not exist" )
  @@ shape_field_non_existence_reason pos2 name reason

let shape_access_with_non_existent_field pos1 name pos2 reason =
  add_list
    (Typing.err_code Typing.ShapeAccessWithNonExistentField)
    (pos1, "You are accessing a field known to not exist")
  @@ shape_field_non_existence_reason pos2 name reason

let ambiguous_object_access
    pos name self_pos vis subclass_pos class_self class_subclass =
  let class_self = strip_ns class_self in
  let class_subclass = strip_ns class_subclass in
  add_list
    (Typing.err_code Typing.AmbiguousObjectAccess)
    ( pos,
      "This object access to " ^ Markdown_lite.md_codify name ^ " is ambiguous"
    )
    [
      ( self_pos,
        "You will access the private instance declared in "
        ^ Markdown_lite.md_codify class_self );
      ( subclass_pos,
        "Instead of the "
        ^ vis
        ^ " instance declared in "
        ^ Markdown_lite.md_codify class_subclass );
    ]

let lateinit_with_default pos =
  add
    (Typing.err_code Typing.LateInitWithDefault)
    pos
    "A late-initialized property cannot have a default value"

let bad_lateinit_override
    parent_is_lateinit
    parent_pos
    child_pos
    (on_error : error_from_reasons_callback) =
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
        "Redeclared properties must be consistently declared `__LateInit`" );
      (parent_pos, "The property " ^ verb ^ " declared `__LateInit` here");
    ]

let bad_xhp_attr_required_override
    parent_tag
    child_tag
    parent_pos
    child_pos
    (on_error : error_from_reasons_callback) =
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
       "Switch statements use `==` equality, so comparing values of type %s with %s may not give the desired result."
       (case_value_ty |> Markdown_lite.md_codify)
       (scrutinee_ty |> Markdown_lite.md_codify)

let unserializable_type pos message =
  add
    (Typing.err_code Typing.UnserializableType)
    pos
    ( "Unserializable type (could not be converted to JSON and back again): "
    ^ message )

let invalid_arraykey code pos (cpos, ctype) (kpos, ktype) =
  add_list
    (Typing.err_code code)
    (pos, "This value is not a valid key type for this container")
    [
      (cpos, "This container is " ^ ctype);
      (kpos, String.capitalize ktype ^ " cannot be used as a key for " ^ ctype);
    ]

let invalid_arraykey_read = invalid_arraykey Typing.InvalidArrayKeyRead

let invalid_arraykey_write = invalid_arraykey Typing.InvalidArrayKeyWrite

let invalid_keyset_value pos (cpos, ctype) (vpos, vtype) =
  add_list
    (Typing.err_code Typing.InvalidKeysetValue)
    (pos, "Keyset values must be arraykeys")
    [
      (cpos, "This container is " ^ ctype);
      (vpos, String.capitalize vtype ^ " is not an arraykey");
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

let internal_compiler_error_msg =
  Printf.sprintf
    "Encountered an internal compiler error while typechecking this. %s %s"
    Error_message_sentinel.remediation_message
    Error_message_sentinel.please_file_a_bug_message

let exception_occurred ~typechecking_is_deferring pos e =
  let pos_str = pos |> Pos.to_absolute |> Pos.string in
  HackEventLogger.type_check_exn_bug
    ~typechecking_is_deferring
    ~path:(Pos.filename pos)
    ~pos:pos_str
    ~e;
  Hh_logger.error
    "Exception while typechecking at position %s\n%s"
    pos_str
    (Exception.to_string e);
  add (Typing.err_code Typing.ExceptionOccurred) pos internal_compiler_error_msg

let invariant_violation
    ~typechecking_is_deferring ~report_to_user ~desc pos telemetry =
  let pos_str = pos |> Pos.to_absolute |> Pos.string in
  HackEventLogger.invariant_violation_bug
    ~typechecking_is_deferring
    ~path:(Pos.filename pos)
    ~pos:pos_str
    ~desc
    telemetry;
  Hh_logger.error
    "Invariant violation at position %s\n%s"
    pos_str
    Telemetry.(telemetry |> string_ ~key:"desc" ~value:desc |> to_string);
  if report_to_user then
    add
      (Typing.err_code Typing.InvariantViolated)
      pos
      internal_compiler_error_msg

let redundant_covariant pos msg suggest =
  add
    (Typing.err_code Typing.RedundantGeneric)
    pos
    ( "This generic parameter is redundant because it only appears in a covariant (output) position"
    ^ msg
    ^ ". Consider replacing uses of generic parameter with "
    ^ Markdown_lite.md_codify suggest
    ^ " or specifying `<<__Explicit>>` on the generic parameter" )

let meth_caller_trait pos trait_name =
  add
    (Typing.err_code Typing.MethCallerTrait)
    pos
    ( (strip_ns trait_name |> Markdown_lite.md_codify)
    ^ " is a trait which cannot be used with `meth_caller`. Use a class instead."
    )

let duplicate_interface pos name others =
  add_list
    (Typing.err_code Typing.DuplicateInterface)
    ( pos,
      Printf.sprintf
        "Interface %s is used more than once in this declaration."
        (strip_ns name |> Markdown_lite.md_codify) )
    (List.map others ~f:(fun pos -> (pos, "Here is another occurrence")))

let hk_var_description because_nested var_name =
  if because_nested then
    Markdown_lite.md_codify var_name
    ^ " is a generic parameter of another (higher-kinded) generic parameter. "
  else
    Markdown_lite.md_codify var_name
    ^ " is a higher-kinded type parameter, standing for a type that has type parameters itself. "

let unsupported_hk_feature ~because_nested pos var_name feature_description =
  let var_description = hk_var_description because_nested var_name in
  add
    (Naming.err_code Naming.HigherKindedTypesUnsupportedFeature)
    pos
    ( var_description
    ^ "We don't support "
    ^ feature_description
    ^ " parameters like "
    ^ Markdown_lite.md_codify var_name
    ^ "." )

let tparam_non_shadowing_reuse pos var_name =
  add
    (Typing.err_code Typing.TypeParameterNameAlreadyUsedNonShadow)
    pos
    ( "The name "
    ^ Markdown_lite.md_codify var_name
    ^ " was already used for another generic parameter. Please use a different name to avoid confusion."
    )

let illegal_information_flow
    primary secondaries (source_poss, source) (sink_poss, sink) =
  let explain poss node printer reasons =
    let msg = printer node in
    List.map ~f:(fun pos -> (pos, msg)) poss @ reasons
  in
  let source = Markdown_lite.md_codify source in
  let sink = Markdown_lite.md_codify sink in
  let sprintf_main = sprintf "Data with policy %s appears in context %s." in
  let claim = (primary, sprintf_main source sink) in
  let reasons =
    let sprintf = Printf.sprintf in
    let sprintf_source = sprintf "This may be the data source with policy %s" in
    let sprintf_sink = sprintf "This may be the data sink with policy %s" in
    let other_occurrences =
      let f p = (p, "Another program point contributing to the illegal flow") in
      List.map ~f secondaries
    in
    []
    |> explain source_poss source sprintf_source
    |> explain sink_poss sink sprintf_sink
    |> List.append other_occurrences
    |> List.rev
  in
  add_list (Typing.err_code Typing.IllegalInformationFlow) claim reasons

let context_implicit_policy_leakage
    primary secondaries (source_poss, source) (sink_poss, sink) =
  let program_point p =
    (p, "Another program point contributing to the leakage")
  in
  let explain_source p = (p, "Leakage source") in
  let explain_sink p = (p, "Leakage sink") in
  let claim =
    ( primary,
      Printf.sprintf
        "Context-implicit policy leaks into %s via %s."
        (Markdown_lite.md_codify sink)
        (Markdown_lite.md_codify source) )
  in
  let reasons =
    List.map ~f:program_point secondaries
    @ List.map ~f:explain_source source_poss
    @ List.map ~f:explain_sink sink_poss
  in
  add_list (Typing.err_code Typing.ContextImplicitPolicyLeakage) claim reasons

let unknown_information_flow pos str =
  add
    (Typing.err_code Typing.UnknownInformationFlow)
    pos
    ("Unable to analyze information flow for " ^ str ^ ". This might be unsafe.")

let reified_function_reference call_pos =
  add
    (Typing.err_code Typing.ReifiedFunctionReference)
    call_pos
    "Invalid function reference. This function requires reified generics. Prefer using a lambda instead."

let class_meth_abstract_call cname meth_name call_pos decl_pos =
  let cname = strip_ns cname in
  add_list
    (Typing.err_code Typing.ClassMethAbstractCall)
    ( call_pos,
      "Cannot create a class_meth of "
      ^ cname
      ^ "::"
      ^ meth_name
      ^ "; it is abstract." )
    [(decl_pos, "Declaration is here")]

let higher_kinded_partial_application pos count =
  add
    (Naming.err_code Naming.HigherKindedTypesUnsupportedFeature)
    pos
    ( "A higher-kinded type is expected here."
    ^ " We do not not support partial applications to yield higher-kinded types, but you are providing "
    ^ string_of_int count
    ^ " type argument(s)." )

let wildcard_for_higher_kinded_type pos =
  add
    (Naming.err_code Naming.HigherKindedTypesUnsupportedFeature)
    pos
    ( "You are supplying _ where a higher-kinded type is expected."
    ^ " We cannot infer higher-kinded type arguments at this time, please state the actual type."
    )

let implicit_type_argument_for_higher_kinded_type ~use_pos ~def_pos param_name =
  let param_desc =
    (* This should be Naming_special_names.Typehints.wildcard, but its not available in this
       module *)
    if String.equal param_name "_" then
      "the anonymous generic parameter"
    else
      "the generic parameter " ^ param_name
  in
  add_list
    (Naming.err_code Naming.HigherKindedTypesUnsupportedFeature)
    ( use_pos,
      "You left out the type arguments here such that they may be inferred."
      ^ " However, a higher-kinded type is expected in place of "
      ^ param_desc
      ^ ", meaning that the type arguments cannot be inferred."
      ^ " Please provide the type arguments explicitly." )
    [(def_pos, param_desc ^ " was declared to be higher-kinded here.")]

(* This is only to be used in a context where we expect something higher-kinded,
  meaning that expected_kind_repr should never just be * *)
let kind_mismatch
    ~use_pos ~def_pos ~tparam_name ~expected_kind_repr ~actual_kind_repr =
  add_list
    (Typing.err_code Typing.KindMismatch)
    ( use_pos,
      "This is "
      ^ actual_kind_repr
      ^ ", but "
      ^ expected_kind_repr
      ^ " was expected here." )
    [
      ( def_pos,
        "We are expecting "
        ^ expected_kind_repr
        ^ " due to the definition of "
        ^ tparam_name
        ^ " here." );
    ]

let class_with_constraints_used_as_hk_type use_pos class_name =
  add
    (Naming.err_code Naming.HigherKindedTypesUnsupportedFeature)
    use_pos
    ( "The class "
    ^ strip_ns class_name
    ^ " imposes constraints on some of its type parameters. Classes that do this cannot be used as higher-kinded types at this time."
    )

let alias_with_implicit_constraints_as_hk_type
    ~use_pos
    ~typedef_pos
    ~used_class_in_def_pos
    ~typedef_name
    ~typedef_tparam_name
    ~used_class_in_def_name
    ~used_class_tparam_name =
  add_list
    (Naming.err_code Naming.HigherKindedTypesUnsupportedFeature)
    ( use_pos,
      "The type "
      ^ strip_ns typedef_name
      ^ " implicitly imposes constraints on its type parameters. Therefore, it cannot be used as a higher-kinded type at this time."
    )
    [
      (typedef_pos, "The definition of " ^ strip_ns typedef_name ^ " is here.");
      ( used_class_in_def_pos,
        "The definition of "
        ^ strip_ns typedef_name
        ^ " relies on "
        ^ strip_ns used_class_in_def_name
        ^ " and the constraints that "
        ^ strip_ns used_class_in_def_name
        ^ " imposes on its type parameter "
        ^ strip_ns used_class_tparam_name
        ^ " then become implicit constraints on the type parameter "
        ^ typedef_tparam_name
        ^ " of "
        ^ strip_ns typedef_name
        ^ "." );
    ]

let reinheriting_classish_const
    dest_classish_pos
    dest_classish_name
    src_classish_pos
    src_classish_name
    existing_const_origin
    const_name =
  add_list
    (Typing.err_code Typing.RedeclaringClassishConstant)
    ( src_classish_pos,
      strip_ns dest_classish_name
      ^ " cannot re-inherit constant "
      ^ const_name
      ^ " from "
      ^ strip_ns src_classish_name )
    [
      ( Pos_or_decl.of_raw_pos dest_classish_pos,
        "because it already inherited it via " ^ strip_ns existing_const_origin
      );
    ]

let redeclaring_classish_const
    classish_pos
    classish_name
    redeclaration_pos
    existing_const_origin
    const_name =
  add_list
    (Typing.err_code Typing.RedeclaringClassishConstant)
    ( redeclaration_pos,
      strip_ns classish_name ^ " cannot re-declare constant " ^ const_name )
    [
      ( Pos_or_decl.of_raw_pos classish_pos,
        "because it already inherited it via " ^ strip_ns existing_const_origin
      );
    ]

let incompatible_enum_inclusion_base
    dest_classish_pos dest_classish_name src_classish_name =
  add_list
    (Typing.err_code Typing.IncompatibleEnumInclusion)
    ( dest_classish_pos,
      "Enum "
      ^ strip_ns dest_classish_name
      ^ " includes enum "
      ^ strip_ns src_classish_name
      ^ " but their base types are incompatible" )
    []

let incompatible_enum_inclusion_constraint
    dest_classish_pos dest_classish_name src_classish_name =
  add_list
    (Typing.err_code Typing.IncompatibleEnumInclusion)
    ( dest_classish_pos,
      "Enum "
      ^ strip_ns dest_classish_name
      ^ " includes enum "
      ^ strip_ns src_classish_name
      ^ " but their constraints are incompatible" )
    []

let enum_inclusion_not_enum
    dest_classish_pos dest_classish_name src_classish_name =
  add_list
    (Typing.err_code Typing.IncompatibleEnumInclusion)
    ( dest_classish_pos,
      "Enum "
      ^ strip_ns dest_classish_name
      ^ " includes "
      ^ strip_ns src_classish_name
      ^ " which is not an enum" )
    []

(* This is meant for subtyping functions with incompatible coeffects, i.e.
 * (function ()[io]: void) </: (function ()[]: void) *)
let coeffect_subtyping_error
    pos_expected
    cap_expected
    pos_got
    cap_got
    (on_error : error_from_reasons_callback) =
  on_error
    ~code:(Typing.err_code Typing.SubtypeCoeffects)
    [
      (pos_expected, "Expected a function that requires " ^ cap_expected);
      (pos_got, "But got a function that requires " ^ cap_got);
    ]

let call_coeffect_error
    ~available_incl_unsafe ~available_pos ~required ~required_pos call_pos =
  add_list
    (Typing.err_code Typing.CallCoeffects)
    ( call_pos,
      "This call is not allowed because its coeffects are incompatible with the context"
    )
    [
      ( available_pos,
        "From this declaration, the context of this function body provides "
        ^ available_incl_unsafe );
      (required_pos, "But the function being called requires " ^ required);
      context_definitions_msg ();
    ]

let op_coeffect_error
    ~locally_available
    ~available_pos
    ~err_code
    ~required
    ?(suggestion = [])
    op
    op_pos =
  add_list
    err_code
    ( op_pos,
      op ^ " requires " ^ required ^ ", which is not provided by the context."
    )
    ( [
        ( available_pos,
          "The local (enclosing) context provides " ^ locally_available );
      ]
    @ suggestion
    @ [context_definitions_msg ()] )

let abstract_function_pointer cname meth_name call_pos decl_pos =
  let cname = strip_ns cname in
  add_list
    (Typing.err_code Typing.AbstractFunctionPointer)
    ( call_pos,
      "Cannot create a function pointer to "
      ^ Markdown_lite.md_codify (cname ^ "::" ^ meth_name)
      ^ "; it is abstract" )
    [(decl_pos, "Declaration is here")]

let unnecessary_attribute pos ~attr ~reason ~suggestion =
  let attr = strip_ns attr in
  let (reason_pos, reason_msg) = reason in
  let suggestion =
    match suggestion with
    | None -> "Try deleting this attribute"
    | Some s -> s
  in
  add_list
    (Typing.err_code Typing.UnnecessaryAttribute)
    (pos, sprintf "The attribute `%s` is unnecessary" attr)
    [
      ( Pos_or_decl.of_raw_pos reason_pos,
        "It is unnecessary because " ^ reason_msg );
      (Pos_or_decl.of_raw_pos pos, suggestion);
    ]

let inherited_class_member_with_different_case
    member_type name name_prev p child_class prev_class prev_class_pos =
  let name = strip_ns name in
  let name_prev = strip_ns name_prev in
  let child_class = strip_ns child_class in
  let prev_class = strip_ns prev_class in
  let claim =
    ( p,
      child_class
      ^ " inherits a "
      ^ member_type
      ^ " named "
      ^ Markdown_lite.md_codify name_prev
      ^ " whose name differs from this one ("
      ^ Markdown_lite.md_codify name
      ^ ") only by case." )
  in
  let reasons =
    [
      ( prev_class_pos,
        "It was inherited from "
        ^ prev_class
        ^ " as "
        ^ (highlight_differences name name_prev |> Markdown_lite.md_codify)
        ^ ". If you meant to override it, please use the same casing as the inherited "
        ^ member_type
        ^ "."
        ^ " Otherwise, please choose a different name for the new "
        ^ member_type );
    ]
  in
  add_list (Typing.err_code Typing.InheritedMethodCaseDiffers) claim reasons

let multiple_inherited_class_member_with_different_case
    ~member_type ~name1 ~name2 ~class1 ~class2 ~child_class ~child_p ~p1 ~p2 =
  let name1 = strip_ns name1 in
  let name2 = strip_ns name2 in
  let class1 = strip_ns class1 in
  let class2 = strip_ns class2 in
  let child_class = strip_ns child_class in
  let claim =
    ( child_p,
      Markdown_lite.md_codify child_class
      ^ " inherited two versions of the "
      ^ member_type
      ^ " "
      ^ Markdown_lite.md_codify name1
      ^ " whose names differ only by case." )
  in
  let reasons =
    [
      ( p1,
        "It inherited "
        ^ Markdown_lite.md_codify name1
        ^ " from "
        ^ class1
        ^ " here." );
      ( p2,
        "And "
        ^ Markdown_lite.md_codify name2
        ^ " from "
        ^ class2
        ^ " here. Please rename these "
        ^ member_type
        ^ "s to the same casing." );
    ]
  in
  add_list (Typing.err_code Typing.InheritedMethodCaseDiffers) claim reasons

let via_label_invalid_parameter pos =
  add_list
    (Typing.err_code Typing.ViaLabelInvalidParameter)
    ( pos,
      "Attribute "
      ^ Naming_special_names.UserAttributes.uaViaLabel
      ^ " is only allowed on "
      ^ Naming_special_names.Classes.cMemberOf )
    []

let via_label_invalid_parameter_in_enum_class pos =
  add_list
    (Typing.err_code Typing.ViaLabelInvalidParameter)
    ( pos,
      "When using "
      ^ Naming_special_names.UserAttributes.uaViaLabel
      ^ ", only type parameters bounded by enum classes and "
      ^ "enum classes are allowed as the first parameters of "
      ^ Naming_special_names.Classes.cMemberOf )
    []

let via_label_invalid_generic pos name =
  add_list
    (Typing.err_code Typing.ViaLabelInvalidParameter)
    ( pos,
      "The type "
      ^ name
      ^ " must be a type constant or a reified generic "
      ^ "in order to be used with "
      ^ Naming_special_names.UserAttributes.uaViaLabel )
    []

let enum_class_label_unknown pos label_name class_name =
  let class_name = strip_ns class_name in
  add_list
    (Typing.err_code Typing.EnumClassLabelUnknown)
    (pos, "Unknown constant " ^ label_name ^ " in " ^ class_name)
    []

let enum_class_label_as_expr pos =
  add_list
    (Typing.err_code Typing.EnumClassLabelAsExpression)
    ( pos,
      "Enum class labels are not allowed in this position. They are only allowed "
      ^ "in function calls)" )
    []

let enum_class_label_invalid_argument pos ~is_proj =
  let msg =
    if is_proj then
      ", not a class constant projection."
    else
      "."
  in
  add_list
    (Typing.err_code Typing.EnumClassLabelInvalidArgument)
    (pos, "An enum class label is required here" ^ msg)
    []

let ifc_internal_error pos reason =
  add
    (Typing.err_code Typing.IFCInternalError)
    pos
    ( "IFC Internal Error: "
    ^ reason
    ^ ". If you see this error and aren't expecting it, please `hh rage` and let the Hack team know."
    )

let override_method_support_dynamic_type
    child_pos
    parent_pos
    parent_origin
    name
    (on_error : error_from_reasons_callback) =
  let parent_origin = strip_ns parent_origin in
  on_error
    ~code:(Typing.err_code Typing.ImplementsDynamic)
    [
      ( child_pos,
        "Method "
        ^ name
        ^ " must be declared <<__SupportDynamicType>> because it overrides method in class "
        ^ parent_origin
        ^ " which does." );
      (parent_pos, "Overridden method is defined here.");
    ]

let parent_support_dynamic_type
    pos
    (child_name, child_kind)
    (parent_name, parent_kind)
    child_support_dynamic_type =
  let kind_to_string = function
    | Ast_defs.Cabstract
    | Ast_defs.Cnormal ->
      "class "
    | Ast_defs.Ctrait -> "trait "
    | Ast_defs.Cinterface -> "interface "
    | Ast_defs.Cenum -> (* cannot happen *) ""
  in
  let kinds_to_use child_kind parent_kind =
    match (child_kind, parent_kind) with
    | (_, Ast_defs.Cabstract)
    | (_, Ast_defs.Cnormal) ->
      "extends "
    | (_, Ast_defs.Ctrait) -> "uses "
    | (Ast_defs.Cinterface, Ast_defs.Cinterface) -> "extends "
    | (_, Ast_defs.Cinterface) -> "implements "
    | (_, _) -> ""
  in
  let child_name = Markdown_lite.md_codify (strip_ns child_name) in
  let child_kind_s = kind_to_string child_kind in
  let parent_name = Markdown_lite.md_codify (strip_ns parent_name) in
  let parent_kind_s = kind_to_string parent_kind in
  add
    (Typing.err_code Typing.ImplementsDynamic)
    pos
    ( String.capitalize child_kind_s
    ^ child_name
    ^ ( if child_support_dynamic_type then
        " cannot "
      else
        " must " )
    ^ "declare <<__SupportDynamicType>> because it "
    ^ kinds_to_use child_kind parent_kind
    ^ parent_kind_s
    ^ parent_name
    ^ " which does"
    ^
    if child_support_dynamic_type then
      " not"
    else
      "" )

let function_is_not_dynamically_callable pos function_name error =
  let function_name = Markdown_lite.md_codify (strip_ns function_name) in
  let nested_error_reason = claim_as_reason error.claim :: error.reasons in
  add_list
    (Typing.err_code Typing.ImplementsDynamic)
    (pos, "Function  " ^ function_name ^ " is not dynamically callable.")
    nested_error_reason

let method_is_not_dynamically_callable
    pos
    method_name
    class_name
    support_dynamic_type_attribute
    parent_class_opt
    error_opt =
  let method_name = Markdown_lite.md_codify (strip_ns method_name) in
  let class_name = Markdown_lite.md_codify (strip_ns class_name) in

  let nested_error_reason =
    match error_opt with
    | None -> []
    | Some e -> claim_as_reason e.claim :: e.reasons
  in

  let parent_class_reason =
    match parent_class_opt with
    | None -> []
    | Some (parent_class_pos, parent_class_name) ->
      let parent_class_name =
        Markdown_lite.md_codify (strip_ns parent_class_name)
      in
      [
        ( parent_class_pos,
          "Method "
          ^ method_name
          ^ " is defined in the super class "
          ^ parent_class_name
          ^ " and is not overridden" );
      ]
  in

  let attribute_reason =
    match support_dynamic_type_attribute with
    | true -> []
    | false ->
      [
        ( Pos_or_decl.of_raw_pos pos,
          "A parameter of "
          ^ method_name
          ^ " is not enforceable. "
          ^ "Try adding the <<"
          ^ Naming_special_names.UserAttributes.uaSupportDynamicType
          ^ ">> attribute to "
          ^ "the method to check if its code is safe for dynamic calling." );
      ]
  in

  add_list
    (Typing.err_code Typing.ImplementsDynamic)
    ( pos,
      "Method  "
      ^ method_name
      ^ " in class "
      ^ class_name
      ^ " is not dynamically callable." )
    (parent_class_reason @ attribute_reason @ nested_error_reason)

let property_is_not_enforceable pos prop_name class_name (prop_pos, prop_type) =
  let class_name = Markdown_lite.md_codify (strip_ns class_name) in
  let prop_name = Markdown_lite.md_codify prop_name in
  let prop_type = Markdown_lite.md_codify prop_type in
  add_list
    (Typing.err_code Typing.ImplementsDynamic)
    ( pos,
      "Class "
      ^ class_name
      ^ " cannot support dynamic because property "
      ^ prop_name
      ^ " does not have an enforceable type" )
    [(prop_pos, "Property " ^ prop_name ^ " has type " ^ prop_type)]

let property_is_not_dynamic pos prop_name class_name (prop_pos, prop_type) =
  let class_name = Markdown_lite.md_codify (strip_ns class_name) in
  let prop_name = Markdown_lite.md_codify prop_name in
  let prop_type = Markdown_lite.md_codify prop_type in
  add_list
    (Typing.err_code Typing.ImplementsDynamic)
    ( pos,
      "Class "
      ^ class_name
      ^ " cannot support dynamic because property "
      ^ prop_name
      ^ " cannot be assigned to dynamic" )
    [(prop_pos, "Property " ^ prop_name ^ " has type " ^ prop_type)]

let private_property_is_not_enforceable
    pos prop_name class_name (prop_pos, prop_type) =
  let class_name = Markdown_lite.md_codify (strip_ns class_name) in
  let prop_name = Markdown_lite.md_codify prop_name in
  let prop_type = Markdown_lite.md_codify prop_type in
  add_list
    (Typing.err_code Typing.PrivateDynamicWrite)
    ( pos,
      "Cannot write to property "
      ^ prop_name
      ^ " through dynamic type because private property in "
      ^ class_name
      ^ " does not have an enforceable type" )
    [(prop_pos, "Property " ^ prop_name ^ " has type " ^ prop_type)]

let private_property_is_not_dynamic
    pos prop_name class_name (prop_pos, prop_type) =
  let class_name = Markdown_lite.md_codify (strip_ns class_name) in
  let prop_name = Markdown_lite.md_codify prop_name in
  let prop_type = Markdown_lite.md_codify prop_type in
  add_list
    (Typing.err_code Typing.PrivateDynamicRead)
    ( pos,
      "Cannot read from property "
      ^ prop_name
      ^ " through dynamic type because private property in "
      ^ class_name
      ^ " cannot be assigned to dynamic" )
    [(prop_pos, "Property " ^ prop_name ^ " has type " ^ prop_type)]

let immutable_local pos =
  add
    (Typing.err_code Typing.ImmutableLocal)
    pos
    (* TODO: generalize this error message in the future for arbitrary immutable locals *)
    "This variable cannot be reassigned because it is used for a dependent context"

let enum_classes_reserved_syntax pos =
  add
    (Typing.err_code Typing.EnumClassesReservedSyntax)
    pos
    ( "This syntax is reserved for the Enum Classes feature.\n"
    ^ "Enable it with the enable_enum_classes option in .hhconfig" )

let nonsense_member_selection pos kind =
  add
    (Typing.err_code Typing.NonsenseMemberSelection)
    pos
    ("Dynamic member access requires a local variable, not `" ^ kind ^ "`.")

let consider_meth_caller pos class_name meth_name =
  add
    (Typing.err_code Typing.ConsiderMethCaller)
    pos
    ( "Function pointer syntax requires a static method. "
    ^ "Use `meth_caller("
    ^ strip_ns class_name
    ^ "::class, '"
    ^ meth_name
    ^ "')` to create a function pointer to the instance method" )

let enum_supertyping_reserved_syntax pos =
  add
    (Typing.err_code Typing.EnumSupertypingReservedSyntax)
    pos
    ( "This Enum uses syntax reserved for the Enum Supertyping feature.\n"
    ^ "Enable it with the enable_enum_supertyping option in .hhconfig" )

let readonly_modified ?reason pos =
  match reason with
  | Some (rpos, rmsg) ->
    add_list
      (Typing.err_code Typing.ReadonlyValueModified)
      (pos, "This value is readonly, its properties cannot be modified")
      [(rpos, rmsg)]
  | None ->
    add
      (Typing.err_code Typing.ReadonlyValueModified)
      pos
      "This value is readonly, its properties cannot be modified"

let readonly_mismatch prefix pos ~reason_sub ~reason_super =
  add_list
    (Typing.err_code Typing.ReadonlyMismatch)
    (pos, prefix)
    (reason_sub @ reason_super)

let readonly_invalid_as_mut pos =
  add
    (Typing.err_code Typing.ReadonlyInvalidAsMut)
    pos
    "Only primitive value types can be converted to mutable."

let readonly_exception pos =
  add
    (Typing.err_code Typing.ReadonlyException)
    pos
    "This exception is readonly; throwing readonly exceptions is not currently supported."

let readonly_mismatch_on_error
    prefix
    pos
    ~reason_sub
    ~reason_super
    (on_error : error_from_reasons_callback) =
  on_error
    ~code:(Typing.err_code Typing.ReadonlyMismatch)
    ((pos, prefix) :: (reason_sub @ reason_super))

let explicit_readonly_cast kind pos origin_pos =
  add_list
    (Typing.err_code Typing.ExplicitReadonlyCast)
    ( pos,
      "This "
      ^ kind
      ^ " returns a readonly value. It must be explicitly wrapped in a readonly expression."
    )
    [(origin_pos, "The " ^ kind ^ " is defined here.")]

let readonly_method_call receiver_pos method_pos =
  add_list
    (Typing.err_code Typing.ReadonlyMethodCall)
    ( receiver_pos,
      "This expression is readonly, so it can only call readonly methods" )
    [(method_pos, "This method is not readonly")]

let invalid_meth_caller_calling_convention call_pos param_pos convention =
  add_list
    (Typing.err_code Typing.InvalidMethCallerCallingConvention)
    ( call_pos,
      "`meth_caller` does not support methods with the "
      ^ convention
      ^ " calling convention" )
    [
      ( param_pos,
        "This is why I think this method uses the `inout` calling convention" );
    ]

let readonly_closure_call pos f_pos suggestion =
  add_list
    (Typing.err_code Typing.ReadonlyClosureCall)
    ( pos,
      "This function is readonly, so it must be marked readonly at declaration time to be called."
    )
    [(f_pos, "Did you mean to " ^ suggestion ^ "?")]

let unsafe_cast pos =
  add
    (Typing.err_code Typing.UnsafeCast)
    pos
    "This cast violates type safety and may lead to unexpected behavior at runtime."

let experimental_expression_trees pos =
  add
    (Typing.err_code Typing.ExperimentalExpressionTrees)
    pos
    ( "To enable Expression Trees for local experimentation use the "
    ^ "`<<file:__EnableUnstableFeatures('expression_trees')>>` file attribute."
    )

let non_void_annotation_on_return_void_function is_async p =
  let (async_indicator, return_type) =
    if is_async then
      ("Async f", "Awaitable<void>")
    else
      ("F", "void")
  in
  let message =
    Printf.sprintf
      "%sunctions that do not return a value must have a type of %s"
      async_indicator
      return_type
  in
  add (Typing.err_code Typing.NonVoidAnnotationOnReturnVoidFun) p message

let cyclic_class_constant pos class_name constant_name =
  add_list
    (Typing.err_code Typing.CyclicClassConstant)
    ( pos,
      "Cannot declare self-referencing constant "
      ^ constant_name
      ^ " in "
      ^ strip_ns class_name )
    []

let bad_conditional_support_dynamic pos ~child ~parent statement reasons =
  add_list
    (Typing.err_code Typing.BadConditionalSupportDynamic)
    ( pos,
      "Class "
      ^ strip_ns child
      ^ " must support dynamic at least as often as "
      ^ strip_ns parent
      ^ ":\n"
      ^ statement )
    reasons

let unresolved_type_variable_projection pos tname ~proj_pos =
  add_list
    (Typing.err_code Typing.UnresolvedTypeVariableProjection)
    (pos, "Can't access a type constant " ^ tname ^ " from an unresolved type")
    [
      (proj_pos, "Access happens here");
      ( Pos_or_decl.of_raw_pos pos,
        "Disambiguate the types using explicit type annotations here." );
    ]

let function_pointer_with_via_label pos fpos =
  let via_label = Naming_special_names.UserAttributes.uaViaLabel in
  add_list
    (Typing.err_code Typing.FunctionPointerWithViaLabel)
    ( pos,
      "Function pointer on functions/methods using "
      ^ via_label
      ^ " is not supported." )
    [(fpos, via_label ^ " is used here.")]

let reified_static_method_in_expr_tree pos =
  add
    (Typing.err_code Typing.ReifiedStaticMethodInExprTree)
    pos
    "Static method calls on reified generics are not permitted in Expression Trees."

let invalid_echo_argument_at pos ?code reasons =
  let msg =
    "Invalid "
    ^ Markdown_lite.md_codify "echo"
    ^ "/"
    ^ Markdown_lite.md_codify "print"
    ^ " argument"
  in
  add_list
    (Option.value code ~default:(Typing.err_code Typing.InvalidEchoArgument))
    (pos, msg)
    reasons

let module_mismatch pos m2_pos m1 m2 =
  add_list
    (Typing.err_code Typing.ModuleError)
    ( pos,
      Printf.sprintf
        "Cannot access an internal element from module `%s` %s"
        m2
        (match m1 with
        | Some m -> Printf.sprintf "in module `%s`" m
        | None -> "outside of a module") )
    [(m2_pos, Printf.sprintf "This is from module `%s`" m2)]

let module_hint ~def_pos ~use_pos =
  add_list
    (Typing.err_code Typing.ModuleHintError)
    (use_pos, "You cannot use this type in a public declaration.")
    [(def_pos, "It is declared as `internal` here")]

(*****************************************************************************)
(* Printing *)
(*****************************************************************************)

let to_json (error : finalized_error) =
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
  Hh_json.JSON_Object [("message", Hh_json.JSON_Array elts)]

let convert_errors_to_string ?(include_filename = false) (errors : error list) :
    string list =
  List.fold_right
    ~init:[]
    ~f:(fun err acc_out ->
      let { code = _; claim; reasons } = err in
      List.fold_right
        ~init:acc_out
        ~f:(fun (pos, msg) acc_in ->
          let pos = Pos_or_decl.unsafe_to_raw_pos pos in
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
        (claim_as_reason claim :: reasons))
    errors

(*****************************************************************************)
(* Try if errors. *)
(*****************************************************************************)

let try_ f1 f2 = try_with_result f1 (fun _ err -> f2 err)

let try_with_error f1 f2 =
  try_ f1 (fun error ->
      add_error error;
      f2 ())

let has_no_errors (f : unit -> 'a) : bool =
  try_
    (fun () ->
      let _ = f () in
      true)
    (fun _ -> false)

(* Runs f2 on the result only if f1 returns has no errors. *)
let try_if_no_errors f1 f2 =
  let (result, error_opt) =
    try_with_result
      (fun () ->
        let result = f1 () in
        (result, None))
      (fun (result, _none) error -> (result, Some error))
  in
  match error_opt with
  | Some err ->
    add_error err;
    result
  | None -> f2 result

(*****************************************************************************)
(* Do. *)
(*****************************************************************************)

let ignore_ f =
  let allow_errors_in_default_path_copy = !allow_errors_in_default_path in
  let ignore_pos_outside_current_span_copy = !ignore_pos_outside_current_span in
  set_allow_errors_in_default_path true;
  ignore_pos_outside_current_span := true;
  let (_, result) = do_ f in
  set_allow_errors_in_default_path allow_errors_in_default_path_copy;
  ignore_pos_outside_current_span := ignore_pos_outside_current_span_copy;
  result

let try_when f ~when_ ~do_ =
  try_with_result f (fun result error ->
      if when_ () then
        do_ error
      else
        add_error error;
      result)
