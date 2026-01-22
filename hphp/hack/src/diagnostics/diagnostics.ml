(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Watch out!
  * - Errors consist of both the things we call errors (severity = Err) and the things we call warnings (errors with severity = Warning)
  * - We never do 'try' logic with warnings (if we do, it's a bug). See `try_with_result` logic
  * - For fixmes/ignore: The things we call warnings (errors with severity = Warning) can sometimes have error (non-warning) codes: see `try_apply_fixme `
*)

open Hh_prelude

type error_code = int

(** see .mli **)
type format =
  | Context
  | Raw
  | Highlighted
  | Plain
  | Extended

let claim_as_reason : Pos.t Message.t -> Pos_or_decl.t Message.t =
 (fun (p, m) -> (Pos_or_decl.of_raw_pos p, m))

(* The file of analysis being currently performed *)
let current_file : Relative_path.t ref = ref Relative_path.default

let current_span : Pos.t ref = ref Pos.none

let ignore_pos_outside_current_span : bool ref = ref false

let allow_errors_in_default_path = ref true

type finalized_diagnostic = (Pos.absolute, Pos.absolute) User_diagnostic.t
[@@deriving eq, ord, show]

type diagnostic = (Pos.t, (Pos_or_decl.t[@hash.ignore])) User_diagnostic.t
[@@deriving eq, hash, ord, show]

type per_file_diagnostics = diagnostic list

type t = diagnostic list Relative_path.Map.t [@@deriving eq, show]

type severity = User_diagnostic.severity

let files_t_fold v ~f ~init =
  Relative_path.Map.fold v ~init ~f:(fun path v acc -> f path v acc)

let files_t_map v ~f = Relative_path.Map.map v ~f

(** [files_t_merge f x y] is a merge by [file], i.e. given "x:t" and "y:t" it
goes through every (file) mentioned in either of them, and combines
the error-lists using the provided callback. Cost is O(x). *)
let files_t_merge
    ~(f :
       Relative_path.t ->
       diagnostic list option ->
       diagnostic list option ->
       diagnostic list option)
    (x : t)
    (y : t) : t =
  (* Using fold instead of merge to make the runtime proportional to the size
   * of first argument (like List.rev_append ) *)
  Relative_path.Map.fold x ~init:y ~f:(fun path x acc ->
      let y = Relative_path.Map.find_opt y path in
      match f path (Some x) y with
      | None -> Relative_path.Map.remove acc path
      | Some data -> Relative_path.Map.add acc ~key:path ~data)

let files_t_to_list x =
  files_t_fold x ~f:(fun _ x acc -> List.rev_append x acc) ~init:[] |> List.rev

(** Values constructed here should not be used with incremental mode.
See assert in incremental_update. *)
let from_diagnostic_list errors =
  if List.is_empty errors then
    Relative_path.Map.empty
  else
    Relative_path.Map.singleton Relative_path.default errors

(* Get most recently-ish added error. *)
let get_last error_map =
  (* If this map has more than one element, we pick an arbitrary file. Because
   * of that, we might not end up with the most recent error and generate a
   * less-specific error message. This should be rare. *)
  match Relative_path.Map.max_binding_opt error_map with
  | None -> None
  | Some (_, error_list) ->
    (match List.rev error_list with
    | [] -> None
    | e :: _ -> Some e)

module Error = struct
  type t = diagnostic [@@deriving ord]

  let hash_for_saved_state (error : t) : Warnings_saved_state.ErrorHash.t =
    User_diagnostic.hash_diagnostic_for_saved_state error
end

module ErrorSet = Stdlib.Set.Make (Error)

module FinalizedError = struct
  type t = finalized_diagnostic [@@deriving ord]
end

module FinalizedErrorSet = Stdlib.Set.Make (FinalizedError)

let (get_hh_fixme_pos : (Pos.t -> error_code -> Pos.t option) ref) =
  ref (fun _ _ -> None)

let (get_disallowed_fixme_pos : (Pos.t -> error_code -> Pos.t option) ref) =
  ref (fun _ _ -> None)

let (get_ignore_pos : (Pos.t -> error_code -> Pos.t option) ref) =
  ref (fun _ _ -> None)

let drop_fixmed_errors (errs : ('a, 'b) User_diagnostic.t list) :
    ('a, 'b) User_diagnostic.t list =
  List.filter errs ~f:(fun e -> not e.User_diagnostic.is_fixmed)

let drop_fixmed_errors_in_files (t : t) : t =
  Relative_path.Map.fold
    t
    ~init:Relative_path.Map.empty
    ~f:(fun file errs acc ->
      let errs_without_fixmes = drop_fixmed_errors errs in
      if List.is_empty errs_without_fixmes then
        acc
      else
        Relative_path.Map.add acc ~key:file ~data:errs_without_fixmes)

let drop_fixmes_if (err : t) (drop : bool) : t =
  if drop then
    drop_fixmed_errors_in_files err
  else
    err

(** Get all errors emitted. Drops errors with valid HH_FIXME comments
    unless ~drop_fixmed:false is set. *)
let get_diagnostic_list ?(drop_fixmed = true) err =
  files_t_to_list (drop_fixmes_if err drop_fixmed)

let (error_map : diagnostic list Relative_path.Map.t ref) =
  ref Relative_path.Map.empty

let accumulate_errors = ref false

(* Are we in the middle of folding a decl? ("lazy" means it happens on-demand
   during the course of the typecheck, rather than all upfront). *)
let in_lazy_decl = ref false

let badpos_message =
  Printf.sprintf
    "Incomplete position information! Your type error is in this file, but we could only find related positions in another file. %s"
    Error_message_sentinel.please_file_a_bug_message

let badpos_message_2 =
  Printf.sprintf
    "Incomplete position information! We couldn't find the exact line of your type error in this definition. %s"
    Error_message_sentinel.please_file_a_bug_message

let try_with_result (f1 : unit -> 'res) (f2 : 'res -> diagnostic -> 'res) : 'res
    =
  let error_map_copy = !error_map in
  let accumulate_errors_copy = !accumulate_errors in
  let get_hh_fixme_pos_copy = !get_hh_fixme_pos in
  (get_hh_fixme_pos := (fun _ _ -> None));
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
            get_hh_fixme_pos := get_hh_fixme_pos_copy
        end
  in
  match get_last errors with
  | None -> result
  | Some
      User_diagnostic.
        {
          severity;
          code;
          claim;
          reasons;
          explanation;
          quickfixes;
          custom_msgs;
          function_pos;
          is_fixmed = _;
        } ->
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
    let error =
      User_diagnostic.make
        severity
        code
        claim
        reasons
        explanation
        ~quickfixes
        ~custom_msgs
        ?function_pos
    in
    (* We ensure warnings do not affect typechecker behavior and are not lost *)
    (match severity with
    | User_diagnostic.Err -> f2 result error
    | User_diagnostic.Warning ->
      if !accumulate_errors then begin
        let current_list =
          Relative_path.Map.find_opt !error_map !current_file
          |> Option.value ~default:[]
        in
        error_map :=
          Relative_path.Map.add
            !error_map
            ~key:!current_file
            ~data:(error :: current_list)
      end;
      result)

(* Reset errors before running [f] so that we can return the errors
 * caused by f. These errors are not added in the global list of errors. *)
let do_ ?(drop_fixmed = true) f =
  let error_map_copy = !error_map in
  let accumulate_errors_copy = !accumulate_errors in
  error_map := Relative_path.Map.empty;
  accumulate_errors := true;
  let get_hh_fixme_pos_copy = !get_hh_fixme_pos in
  let (result, out_errors) =
    Utils.try_finally
      ~f:
        begin
          fun () ->
            let result = f () in
            (result, !error_map)
        end
      ~finally:
        begin
          fun () ->
            error_map := error_map_copy;
            accumulate_errors := accumulate_errors_copy;
            get_hh_fixme_pos := get_hh_fixme_pos_copy
        end
  in
  let out_errors = files_t_map ~f:List.rev out_errors in
  (drop_fixmes_if out_errors drop_fixmed, result)

let run_in_context path f =
  let context_copy = !current_file in
  current_file := path;
  Utils.try_finally ~f ~finally:(fun () -> current_file := context_copy)

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
    (Stdlib.Printexc.raw_backtrace_to_string
       (Stdlib.Printexc.get_callstack 500));

  (* Exit with special error code so we can see the log after *)
  Exit.exit Exit_status.Lazy_decl_bug

(*****************************************************************************)
(* Error code printing. *)
(*****************************************************************************)
let compare_internal
    (x : ('a, 'b) User_diagnostic.t) (y : ('a, 'b) User_diagnostic.t) : int =
  let User_diagnostic.
        {
          severity = x_severity;
          code = x_code;
          claim = x_claim;
          reasons = x_messages;
          explanation = _;
          custom_msgs = _;
          quickfixes = _;
          is_fixmed = _;
          function_pos = _;
        } =
    x
  in
  let User_diagnostic.
        {
          severity = y_severity;
          code = y_code;
          claim = y_claim;
          reasons = y_messages;
          explanation = _;
          custom_msgs = _;
          quickfixes = _;
          is_fixmed = _;
          function_pos = _;
        } =
    y
  in
  (* /!\ KEEP THIS IN SYNC WITH errors_impl.rs, `fn cmp_impl` *)
  let comparison = 0 in
  (* order by severity *)
  let comparison =
    if comparison = 0 then
      User_diagnostic.compare_severity x_severity y_severity
    else
      comparison
  in
  (* order by file *)
  let comparison =
    if comparison = 0 then
      Relative_path.compare
        (fst x_claim |> Pos.filename)
        (fst y_claim |> Pos.filename)
    else
      comparison
  in
  (* Then within each file, sort by phase *)
  let comparison =
    let x_phase = x_code / 1000 in
    let y_phase = y_code / 1000 in
    if comparison = 0 then
      Int.compare x_phase y_phase
    else
      comparison
  in
  (* If the phases are the same, sort by position *)
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
  (* If the message text is also the same, then continue comparing
     the reason messages (which indicate the reason why Hack believes
     there is an error reported in the claim message) *)
  let comparison =
    if comparison = 0 then
      (List.compare (Message.compare Pos_or_decl.compare)) x_messages y_messages
    else
      comparison
  in
  (* If everything else is the same, compare by code *)
  let comparison =
    if comparison = 0 then
      Int.compare x_code y_code
    else
      comparison
  in
  comparison

let sort (err : diagnostic list) : diagnostic list =
  List.sort ~compare:compare_internal err
  |> List.remove_consecutive_duplicates
       ~equal:(fun x y ->
         (* If two errors have the exact same messages and positions but different codes,
            consider them equal for deduplication *)
         compare_internal
           {
             x with
             User_diagnostic.code = x.User_diagnostic.code / 1000 * 1000;
           }
           {
             y with
             User_diagnostic.code = y.User_diagnostic.code / 1000 * 1000;
           }
         |> Int.equal 0)
       ~which_to_keep:`First (* Match Rust dedup_by *)

let get_sorted_diagnostic_list ?(drop_fixmed = true) err =
  sort (files_t_to_list (drop_fixmes_if err drop_fixmed))

let sort_and_finalize (errors : t) : finalized_diagnostic list =
  errors
  |> get_sorted_diagnostic_list
  |> List.map ~f:User_diagnostic.to_absolute

(* Getters and setter for passed-in map, based on current context *)
let get_current_list t =
  Relative_path.Map.find_opt t !current_file |> Option.value ~default:[]

let set_current_list file_t_map new_list =
  file_t_map :=
    Relative_path.Map.add !file_t_map ~key:!current_file ~data:new_list

let run_and_check_for_errors (f : unit -> 'res) : 'res * bool =
  let old_error_list = get_current_list !error_map in

  let old_count = List.length old_error_list in
  let result = f () in
  let new_error_list = get_current_list !error_map in
  let new_count = List.length new_error_list in
  (result, not (Int.equal old_count new_count))

let do_with_context ?(drop_fixmed = true) path f =
  run_in_context path (fun () -> do_ ~drop_fixmed f)

(** Turn on lazy decl mode for the duration of the closure.
    This runs without returning the original state,
    since we collect it later in do_with_lazy_decls_ *)
let run_in_decl_mode f =
  let old_in_lazy_decl = !in_lazy_decl in
  in_lazy_decl := true;
  Utils.try_finally ~f ~finally:(fun () -> in_lazy_decl := old_in_lazy_decl)

let read_lines path =
  try In_channel.read_lines path with
  | Sys_error _ ->
    (try Multifile.read_file_from_multifile path with
    | Sys_error _ -> [])

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
      (match Map.find grouped key with
      | Some members ->
        let grouped = Map.set grouped ~key ~data:(members @ [x]) in
        build_map xs grouped keys
      | None ->
        let grouped = Map.set grouped ~key ~data:[x] in
        build_map xs grouped (key :: keys))
    | [] -> (grouped, keys)
  in
  let (grouped, keys) = build_map xs String.Map.empty [] in
  List.concat_map (List.rev keys) ~f:(fun fn -> Map.find_exn grouped fn)

let format_or_default = function
  | Some error_format -> error_format
  | None -> Highlighted

(* E.g. "10 errors, 11 warnings found." *)
let format_summary
    format
    ~(error_count : int)
    ~(warning_count : int)
    ~(dropped_count : int option)
    ~(max_errors : int option) : string option =
  let no_errors_string = "No errors!" in
  match format with
  | Context
  | Highlighted ->
    let error_count_message =
      if Int.( = ) error_count 0 then
        no_errors_string
      else
        Printf.sprintf
          "%d error%s"
          error_count
          (if error_count = 1 then
            ""
          else
            "s")
    in
    let warning_count_message =
      if Int.( = ) warning_count 0 then
        ""
      else
        Printf.sprintf
          "%s %d warning%s"
          (if Int.( = ) error_count 0 then
            ""
          else
            ",")
          warning_count
          (if warning_count = 1 then
            ""
          else
            "s")
    in
    let found_s =
      if Int.( > ) (error_count + warning_count) 0 then
        " found"
      else
        ""
    in
    let truncated_message =
      match (max_errors, dropped_count) with
      | (Some max_errors, Some dropped_count) when dropped_count > 0 ->
        Printf.sprintf
          " (only showing first %d, dropped %d)"
          max_errors
          dropped_count
      | (Some max_errors, None) when error_count + warning_count >= max_errors
        ->
        Printf.sprintf " (max-errors is %d; more errors may exist)" max_errors
      | _ -> ""
    in
    Some
      (error_count_message
      ^ warning_count_message
      ^ found_s
      ^ truncated_message
      ^ "\n")
  | Extended
  | Raw
  | Plain ->
    if Int.equal 0 (error_count + warning_count) then
      Some (no_errors_string ^ "\n")
    else
      None

let to_string error = User_diagnostic.to_string error

let log_unexpected error path desc =
  HackEventLogger.invariant_violation_bug
    desc
    ~path
    ~pos:
      (error
      |> User_diagnostic.to_absolute
      |> User_diagnostic.get_pos
      |> Pos.show_absolute);
  Hh_logger.log
    "UNEXPECTED: %s\n%s"
    desc
    (Exception.get_current_callstack_string 99 |> Exception.clean_stack);
  ()

let add_error_impl error =
  if !accumulate_errors then
    let () =
      match !current_file with
      | path
        when Relative_path.equal path Relative_path.default
             && not !allow_errors_in_default_path ->
        log_unexpected error path "adding an error in default path"
      | _ -> ()
    in
    (* Cheap test to avoid duplicating most recent error *)
    let error_list = get_current_list !error_map in
    match error_list with
    | old_error :: _ when equal_diagnostic error old_error -> ()
    | _ -> set_current_list error_map (error :: error_list)
  else
    (* We have an error, but haven't handled it in any way *)
    let msg = error |> User_diagnostic.to_absolute |> to_string in
    if !in_lazy_decl then
      lazy_decl_error_logging
        msg
        error_map
        User_diagnostic.to_absolute
        to_string
    else if error.User_diagnostic.is_fixmed then
      ()
    else
      Utils.assert_false_log_backtrace (Some msg)

(* Whether we've found at least one error *)
let currently_has_errors () = not (List.is_empty (get_current_list !error_map))

module Parsing = Error_codes.Parsing
module Naming = Error_codes.Naming
module NastCheck = Error_codes.NastCheck
module Typing = Error_codes.Typing
module GlobalAccessCheck = Error_codes.GlobalAccessCheck

(*****************************************************************************)
(* Types *)
(*****************************************************************************)

module type Error_category = Error_category.S

(*****************************************************************************)
(* HH_FIXMEs hook *)
(*****************************************************************************)

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

let set_allow_errors_in_default_path x = allow_errors_in_default_path := x

let is_allowed_code_strict (code : error_code) =
  ISet.mem code !allowed_fixme_codes_strict

let code_agnostic_fixme = ref false

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
    Pos.t Message.t * Pos_or_decl.t Message.t list ->
    Pos.t Message.t * Pos_or_decl.t Message.t list =
 fun (claim, reasons) ->
  let pos = fst claim in
  let current_file = !current_file in
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

let add_error_with_fixme_error error explanation ~fixme_pos =
  let User_diagnostic.
        {
          severity;
          code;
          claim = _;
          reasons = _;
          explanation = _;
          quickfixes;
          custom_msgs;
          function_pos;
          is_fixmed = _;
        } =
    error
  in
  add_error_impl error;
  add_error_impl
  @@ User_diagnostic.make
       severity
       code
       (fixme_pos, explanation)
       []
       Explanation.empty
       ~quickfixes
       ~custom_msgs
       ?function_pos

let add_applied_fixme error =
  let error = { error with User_diagnostic.is_fixmed = true } in
  add_error_impl error

type suppression_kind =
  | Fixme of { forbidden_decl_fixme: bool }
  | Ignore

let get_fixme (pos : Pos.t) code : (Pos.t * suppression_kind) option =
  match !get_hh_fixme_pos pos code with
  | Some pos -> Some (pos, Fixme { forbidden_decl_fixme = false })
  | None ->
    (match !get_ignore_pos pos code with
    | Some pos -> Some (pos, Ignore)
    | None ->
      (match !get_disallowed_fixme_pos pos code with
      | Some pos -> Some (pos, Fixme { forbidden_decl_fixme = true })
      | None -> None))

type fixme_error = {
  explanation: string;
  fixme_pos: Pos.t;
}

type fixme_outcome =
  | Not_fixmed of fixme_error option
  | Fixmed

let try_apply_fixme pos code severity : fixme_outcome =
  match get_fixme pos code with
  | None ->
    (* Fixmes and banned decl fixmes are separated by the parser because Errors can't recover
     * the position information after the fact. This is the default case, where an HH_FIXME
     * comment is not present. Therefore, the remaining cases are variations on behavior when
     * a fixme is present *)
    Not_fixmed None
  | Some (fixme_pos, fixme_kind) ->
    let severity_based_on_code =
      (* We use severity based on code for warning-to-error migration scenarios.
         Sometimes we want to introduce an new error as a warning first. In that case,
         to avoid having to codemod any HH_IGNORE along with their codes when the
         warnings become errors, we'll first make the errors have a non-warning code
         but have warning severity at first, and we'll force suppression using HH_FIXME instead of HH_IGNORE. *)
      match Error_codes.Warning.of_enum code with
      | Some _ -> User_diagnostic.Warning
      | None -> User_diagnostic.Err
    in
    (match (severity_based_on_code, fixme_kind) with
    | (User_diagnostic.Err, Fixme { forbidden_decl_fixme }) ->
      if ISet.mem code hard_banned_codes then
        let explanation =
          Printf.sprintf
            "You cannot use `HH_FIXME` or `HH_IGNORE_ERROR` comments to suppress error %d, and this cannot be enabled by configuration"
            code
        in
        Not_fixmed (Some { explanation; fixme_pos })
      else if Relative_path.(is_hhi (prefix (Pos.filename pos))) then
        Fixmed
      else if forbidden_decl_fixme then
        let explanation =
          Printf.sprintf
            "You cannot use `HH_FIXME` or `HH_IGNORE_ERROR` comments to suppress error %d in declarations"
            code
        in
        Not_fixmed (Some { explanation; fixme_pos })
      else if is_allowed_code_strict code then
        Fixmed
      else
        let explanation =
          Printf.sprintf
            "You cannot use `HH_FIXME` or `HH_IGNORE_ERROR` comments to suppress error %d"
            code
        in
        Not_fixmed (Some { explanation; fixme_pos })
    | (User_diagnostic.Warning, Fixme _) ->
      let explanation =
        Printf.sprintf
          "You cannot use `HH_FIXME` comments to suppress warnings. Use `HH_IGNORE[%d]` instead."
          code
      in
      Not_fixmed (Some { explanation; fixme_pos })
    | (User_diagnostic.Warning, Ignore) -> Fixmed
    | (User_diagnostic.Err, Ignore) ->
      let explanation =
        match severity with
        | User_diagnostic.Err ->
          Printf.sprintf
            "You cannot use `HH_IGNORE` to suppress non-warning code %d. Hack errors should not be suppressed. You should try your best to fix the code causing the error, or incidents will happen."
            code
        | User_diagnostic.Warning ->
          Printf.sprintf
            "This warning code %d will soon be migrated to be an error. Try your best to fix it, or if not possible, suppress it with `HH_FIXME` instead of `HH_IGNORE` and report your case to us."
            code
      in
      Not_fixmed (Some { explanation; fixme_pos }))

let is_suppressed error =
  let User_diagnostic.{ severity; code; claim = (pos, _); _ } = error in
  match try_apply_fixme pos code severity with
  | Fixmed -> true
  | Not_fixmed _ -> false

let add_diagnostic (diag : diagnostic) =
  let User_diagnostic.
        {
          severity;
          code;
          claim;
          reasons;
          explanation;
          quickfixes;
          custom_msgs;
          function_pos;
          is_fixmed = _;
        } =
    diag
  in
  let (claim, reasons) = check_pos_msg (claim, reasons) in
  let diag =
    User_diagnostic.make
      severity
      code
      claim
      reasons
      explanation
      ~quickfixes
      ~custom_msgs
      ?function_pos
  in

  let pos = fst claim in

  match try_apply_fixme pos code severity with
  | Not_fixmed None -> add_error_impl diag
  | Not_fixmed (Some { explanation; fixme_pos }) ->
    add_error_with_fixme_error diag explanation ~fixme_pos
  | Fixmed -> add_applied_fixme diag

let merge err' err =
  let append _ x y =
    let x = Option.value x ~default:[] in
    let y = Option.value y ~default:[] in
    Some (List.rev_append x y)
  in
  files_t_merge ~f:append err' err

let incremental_update ~(old : t) ~(new_ : t) ~(rechecked : Relative_path.Set.t)
    : t =
  (* Helper to detect coding oversight *)
  let assert_path_is_real (path : Relative_path.t) : unit =
    if Relative_path.equal path Relative_path.default then
      Utils.assert_false_log_backtrace
        (Some
           ("Default (untracked) error sources should not get into incremental "
           ^ "mode. There might be a missing call to `Diagnostics.do_with_context`/"
           ^ "`run_in_context` somwhere or incorrectly used `Diagnostics.from_diagnostic_list`."
           ))
  in

  (* Replace old errors with new *)
  let res : t =
    files_t_merge new_ old ~f:(fun path new_ old ->
        assert_path_is_real path;
        match new_ with
        | Some new_ -> Some new_
        | None -> old)
  in
  (* For files that were rechecked, but had no errors - remove them from maps *)
  let res : t =
    Relative_path.Set.fold rechecked ~init:res ~f:(fun path acc ->
        if Relative_path.Map.mem new_ path then
          acc
        else
          Relative_path.Map.remove acc path)
  in
  res

let add_list severity code ?quickfixes (claim : _ Message.t) reasons explanation
    =
  add_diagnostic
  @@ User_diagnostic.make severity code claim reasons explanation ?quickfixes

let add severity code pos msg =
  add_list severity code (pos, msg) [] Explanation.empty

let count ?(drop_fixmed = true) err =
  files_t_fold
    (drop_fixmes_if err drop_fixmed)
    ~f:(fun _ x acc -> acc + List.length x)
    ~init:0

let is_empty ?(drop_fixmed = true) err =
  Relative_path.Map.is_empty (drop_fixmes_if err drop_fixmed)

let get_current_span () = !current_span

and empty = Relative_path.Map.empty

let from_file_diagnostic_list (errors : (Relative_path.t * diagnostic) list) : t
    =
  List.fold errors ~init:Relative_path.Map.empty ~f:(fun errors (file, diag) ->
      let errors_for_file =
        Relative_path.Map.find_opt errors file |> Option.value ~default:[]
      in
      let errors_for_file = diag :: errors_for_file in
      Relative_path.Map.add errors ~key:file ~data:errors_for_file)

let as_map (t : t) : diagnostic list Relative_path.Map.t = t

let merge_into_current errors = error_map := merge errors !error_map

(*****************************************************************************)
(* Accessors. (All methods delegated to the parameterized module.) *)
(*****************************************************************************)

let per_file_diagnostic_count
    ?(drop_fixmed = true) (errors : per_file_diagnostics) : int =
  let errors =
    if drop_fixmed then
      drop_fixmed_errors errors
    else
      errors
  in
  List.length errors

let per_file_warning_count ?(drop_fixmed = true) (errors : per_file_diagnostics)
    : int =
  let errors =
    if drop_fixmed then
      drop_fixmed_errors errors
    else
      errors
  in
  List.fold errors ~init:0 ~f:(fun acc (diag : diagnostic) ->
      acc
      +
      match diag.User_diagnostic.severity with
      | User_diagnostic.Warning -> 1
      | User_diagnostic.Err -> 0)

let get_file_diagnostics ?(drop_fixmed = true) (t : t) (file : Relative_path.t)
    : per_file_diagnostics =
  match Relative_path.Map.find_opt t file with
  | None -> []
  | Some errors ->
    if drop_fixmed then
      drop_fixmed_errors errors
    else
      errors

let fold_per_file_diagnostics (errors : per_file_diagnostics) ~init ~f =
  List.fold errors ~init ~f

let iter_diagnostic_list ?(drop_fixmed = true) f err =
  List.iter ~f (get_sorted_diagnostic_list ~drop_fixmed err)

let fold_errors
    ?(drop_fixmed = true)
    (t : t)
    ~(init : 'a)
    ~(f : Relative_path.t -> diagnostic -> 'a -> 'a) =
  let t = drop_fixmes_if t drop_fixmed in
  files_t_fold t ~init ~f:(fun source errors acc ->
      List.fold_right errors ~init:acc ~f:(f source))

(** Get paths that have errors which haven't been HH_FIXME'd. *)
let get_failed_files (err : t) =
  let err = drop_fixmed_errors_in_files err in
  files_t_fold err ~init:Relative_path.Set.empty ~f:(fun source _ acc ->
      Relative_path.Set.add acc source)

(** Count errors which haven't been HH_FIXME'd. *)
let error_count : t -> int =
 fun errors ->
  Relative_path.Map.fold errors ~init:0 ~f:(fun _path errors count ->
      count + per_file_diagnostic_count ~drop_fixmed:true errors)

let warning_count : t -> int =
 fun errors ->
  Relative_path.Map.fold errors ~init:0 ~f:(fun _path errors count ->
      count + per_file_warning_count ~drop_fixmed:true errors)

exception Done of ISet.t

(** This ignores warnings. *)
let first_n_distinct_error_codes ~(n : int) (t : t) : error_code list =
  let codes =
    try
      Relative_path.Map.fold t ~init:ISet.empty ~f:(fun _path errors codes ->
          List.fold
            errors
            ~init:codes
            ~f:(fun codes User_diagnostic.{ severity; code; _ } ->
              match severity with
              | User_diagnostic.Err ->
                let codes = ISet.add code codes in
                if ISet.cardinal codes >= n then
                  raise (Done codes)
                else
                  codes
              | User_diagnostic.Warning -> codes))
    with
    | Done codes -> codes
  in
  ISet.elements codes

(** Get the error code of the first error which hasn't been HH_FIXME'd. *)
let choose_code_opt (t : t) : int option =
  drop_fixmed_errors_in_files t |> first_n_distinct_error_codes ~n:1 |> List.hd

let as_telemetry_summary : t -> Telemetry.t =
 fun errors ->
  Telemetry.create ()
  |> Telemetry.int_ ~key:"count" ~value:(error_count errors)
  |> Telemetry.int_ ~key:"warning_count" ~value:(warning_count errors)
  |> Telemetry.int_list
       ~key:"first_5_distinct_error_codes"
       ~value:
         (first_n_distinct_error_codes
            ~n:5
            (drop_fixmed_errors_in_files errors))

let as_telemetry
    ~limit
    ~with_context_limit
    ~(error_to_string : finalized_diagnostic -> string)
    (errors : t) : Telemetry.t =
  let module L = struct
    type error_to_process = {
      error: diagnostic;
      include_context: bool;
    }
  end in
  let errors = drop_fixmed_errors_in_files errors in
  let total_count = error_count errors in
  let ((errors : L.error_to_process list Relative_path.Map.t), _, is_truncated)
      =
    Relative_path.Map.fold
      errors
      ~init:(Relative_path.Map.empty, 0, false)
      ~f:(fun
           path
           file_errors
           ((errors_acc, errors_acc_count, is_truncated) as acc)
         ->
        if is_truncated then
          acc
        else
          let file_errors_count = List.length file_errors in
          let remaining_capacity = limit - errors_acc_count in
          let is_truncated = remaining_capacity < file_errors_count in
          let file_errors =
            if is_truncated then
              List.take file_errors remaining_capacity
            else
              file_errors
          in
          let file_errors =
            let remaining_with_context =
              with_context_limit - errors_acc_count
            in
            List.mapi file_errors ~f:(fun i error ->
                L.{ error; include_context = i < remaining_with_context })
          in
          ( Relative_path.Map.add errors_acc ~key:path ~data:file_errors,
            errors_acc_count + file_errors_count,
            is_truncated ))
  in
  let error_to_telemetry ({ L.error; include_context } : L.error_to_process) :
      Telemetry.t =
    let rel_err = User_diagnostic.to_relative error in
    let { User_diagnostic.severity; code; _ } = rel_err in
    let telemetry =
      Telemetry.create ()
      |> Telemetry.string_
           ~key:"error_severity"
           ~value:(User_diagnostic.Severity.to_all_caps_string severity)
      |> Telemetry.int_ ~key:"error_code" ~value:code
      |> Telemetry.string_
           ~key:"line_agnostic_hash"
           ~value:(Printf.sprintf "%x" (Error.hash_for_saved_state error))
      |> Telemetry.json_
           ~key:"error_json"
           ~value:
             (User_diagnostic.to_json
                ~human_formatter:None
                ~filename_to_string:Relative_path.suffix
                rel_err)
    in
    if include_context then
      telemetry
      |> Telemetry.string_
           ~key:"error_context"
           ~value:(error_to_string @@ User_diagnostic.to_absolute error)
    else
      telemetry
  in
  let by_file =
    Relative_path.Map.fold
      errors
      ~init:(Telemetry.create ())
      ~f:(fun path errors t ->
        Telemetry.object_list
          t
          ~key:(Relative_path.suffix path)
          ~value:(List.map errors ~f:error_to_telemetry))
  in
  Telemetry.create ()
  |> Telemetry.int_ ~key:"total_count" ~value:total_count
  |> Telemetry.bool_ ~key:"is_truncated" ~value:is_truncated
  |> Telemetry.object_ ~key:"by_file" ~value:by_file

(*****************************************************************************)
(* Error code printing. *)
(*****************************************************************************)

let experimental_feature pos msg =
  add User_diagnostic.Err 0 pos ("Cannot use experimental feature: " ^ msg)

(*****************************************************************************)
(* Typing errors *)
(*****************************************************************************)
let log_exception_occurred pos e =
  let pos_str = pos |> Pos.to_absolute |> Pos.string in
  HackEventLogger.type_check_exn_bug ~path:(Pos.filename pos) ~pos:pos_str ~e;
  Hh_logger.error
    "Exception while typechecking at position %s\n%s"
    pos_str
    (Exception.to_string e)

let log_invariant_violation ~desc pos telemetry =
  let pos_str = pos |> Pos.to_absolute |> Pos.string in
  HackEventLogger.invariant_violation_bug
    desc
    ~path:(Pos.filename pos)
    ~pos:pos_str
    ~telemetry;
  Hh_logger.error
    "Invariant violation at position %s\n%s"
    pos_str
    Telemetry.(telemetry |> string_ ~key:"desc" ~value:desc |> to_string)

(* The contents of the following error message can trigger `hh rage` in
 * sandcastle. If you change it, please make sure the existing trigger in
 * `flib/intern/sandcastle/hack/SandcastleCheckHackOnDiffsStep.php` still
 * catches this error message. *)
let please_file_a_bug_message =
  let hacklang_feedback_support_link =
    "https://fb.workplace.com/groups/hackforhiphop/"
  in
  let hh_rage = Markdown_lite.md_codify "hh rage" in
  Printf.sprintf
    "Please run %s and post in %s."
    hh_rage
    hacklang_feedback_support_link

let remediation_message =
  Printf.sprintf
    "Addressing other errors, restarting the typechecker with %s, or rebasing might resolve this error."
    (Markdown_lite.md_codify "hh restart")

let internal_compiler_error_msg =
  Printf.sprintf
    "Encountered an internal compiler error while typechecking this. %s %s"
    remediation_message
    please_file_a_bug_message

let invariant_violation pos telemetry desc ~report_to_user =
  log_invariant_violation ~desc pos telemetry;
  if report_to_user then
    add_diagnostic
    @@ User_diagnostic.make_err
         Error_codes.Typing.(to_enum InvariantViolated)
         (pos, internal_compiler_error_msg)
         []
         Explanation.empty

let exception_occurred pos exn =
  log_exception_occurred pos exn;
  add_diagnostic
  @@ User_diagnostic.make_err
       Error_codes.Typing.(to_enum ExceptionOccurred)
       (pos, internal_compiler_error_msg)
       []
       Explanation.empty

let internal_error pos msg =
  add_diagnostic
  @@ User_diagnostic.make_err
       Error_codes.Typing.(to_enum InternalError)
       (pos, "Internal error: " ^ msg)
       []
       Explanation.empty

let typechecker_timeout pos fn_name seconds =
  let claim =
    ( pos,
      Printf.sprintf
        "Type checker timed out after %d seconds whilst checking function %s"
        seconds
        fn_name )
  in
  add_diagnostic
  @@ User_diagnostic.make_err
       Error_codes.Typing.(to_enum TypecheckerTimeout)
       claim
       []
       Explanation.empty

(*****************************************************************************)
(* Typing decl errors *)
(*****************************************************************************)

(** TODO: Remove use of `User_diagnostic.t` representation for nested error  *)
let function_is_not_dynamically_callable function_name error =
  let function_name = Markdown_lite.md_codify (Render.strip_ns function_name) in
  let nested_error_reason = User_diagnostic.to_list_ error in
  add_list
    User_diagnostic.Err
    (User_diagnostic.get_code error)
    ( User_diagnostic.get_pos error,
      "Function  " ^ function_name ^ " is not dynamically callable." )
    nested_error_reason
    Explanation.empty

(** TODO: Remove use of `User_diagnostic.t` representation for nested error  *)
let method_is_not_dynamically_callable
    pos
    method_name
    class_name
    support_dynamic_type_attribute
    parent_class_opt
    error_opt =
  let method_name = Markdown_lite.md_codify (Render.strip_ns method_name) in
  let class_name = Markdown_lite.md_codify (Render.strip_ns class_name) in

  let (code, pos, nested_error_reason) =
    match error_opt with
    | None -> (Typing.err_code Typing.ImplementsDynamic, pos, [])
    | Some error ->
      ( User_diagnostic.get_code error,
        User_diagnostic.get_pos error,
        User_diagnostic.to_list_ error )
  in

  let parent_class_reason =
    match parent_class_opt with
    | None -> []
    | Some (parent_class_pos, parent_class_name) ->
      let parent_class_name =
        Markdown_lite.md_codify (Render.strip_ns parent_class_name)
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
    User_diagnostic.Err
    code
    ( pos,
      "Method  "
      ^ method_name
      ^ " in class "
      ^ class_name
      ^ " is not dynamically callable." )
    (parent_class_reason @ attribute_reason @ nested_error_reason)
    Explanation.empty

(* Raise different types of error for accessing global variables. *)
let global_access_error error_code pos message =
  add User_diagnostic.Err (GlobalAccessCheck.err_code error_code) pos message

(*****************************************************************************)
(* Printing *)
(*****************************************************************************)

let convert_errors_to_string
    ?(include_filename = false) (errors : diagnostic list) : string list =
  List.fold_right
    ~init:[]
    ~f:(fun err acc_out ->
      let User_diagnostic.
            {
              severity = _;
              code = _;
              claim;
              reasons;
              custom_msgs;
              explanation = _;
              quickfixes = _;
              is_fixmed = _;
              function_pos = _;
            } =
        err
      in
      let acc_out = custom_msgs @ acc_out in
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

let has_no_errors (f : unit -> 'a) : bool =
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
  let ignore_pos_outside_current_span_copy = !ignore_pos_outside_current_span in
  set_allow_errors_in_default_path true;
  ignore_pos_outside_current_span := true;
  let (_, result) = do_ f in
  set_allow_errors_in_default_path allow_errors_in_default_path_copy;
  ignore_pos_outside_current_span := ignore_pos_outside_current_span_copy;
  result

let try_when f ~if_error_and:condition ~then_:do_ =
  try_with_result f (fun result error ->
      (* Note: we only reach here for errors proper (severity = Err) *)
      if condition () then
        do_ error
      else
        add_diagnostic error;
      result)

let filter (errors : t) ~(f : Relative_path.t -> diagnostic -> bool) : t =
  Relative_path.Map.filter_map errors ~f:(fun path errors ->
      let errors = List.filter errors ~f:(f path) in
      Option.some_if (not (List.is_empty errors)) errors)

let fold (errors : t) ~(init : 'acc) ~f =
  Relative_path.Map.fold errors ~init ~f:(fun path errors acc ->
      List.fold errors ~init:acc ~f:(fun acc error -> f path error acc))

let count_errors_and_warnings (error_list : (_, _) User_diagnostic.t list) :
    int * int =
  List.fold
    error_list
    ~init:(0, 0)
    ~f:(fun (err_count, warn_count) { User_diagnostic.severity; _ } ->
      match severity with
      | User_diagnostic.Warning -> (err_count, warn_count + 1)
      | User_diagnostic.Err -> (err_count + 1, warn_count))

let filter_out_mergebase_warnings
    (mergebase_warning_hashes : Warnings_saved_state.t option) (errors : t) : t
    =
  match mergebase_warning_hashes with
  | None -> errors
  | Some mergebase_warning_hashes ->
    filter errors ~f:(fun _path error ->
        match error.User_diagnostic.severity with
        | User_diagnostic.Err -> true
        | User_diagnostic.Warning ->
          not
            (Warnings_saved_state.mem
               (Error.hash_for_saved_state error)
               mergebase_warning_hashes))

let filter_out_warnings (errors : t) : t =
  filter errors ~f:(fun _path error ->
      match error.User_diagnostic.severity with
      | User_diagnostic.Err -> true
      | User_diagnostic.Warning -> false)

let make_warning_saved_state (errors : t) : Warnings_saved_state.t =
  fold errors ~init:Warnings_saved_state.empty ~f:(fun _path error ss ->
      match error.User_diagnostic.severity with
      | User_diagnostic.Err -> ss
      | User_diagnostic.Warning ->
        let hash = Error.hash_for_saved_state error in
        Warnings_saved_state.add hash ss)
