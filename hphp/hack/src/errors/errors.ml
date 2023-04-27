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

type error_code = int

type phase =
  | Parsing
  | Naming
  | Decl
  | Typing
[@@deriving eq, show, enum]

let _ = max_phase

let phases_up_to_excl : phase -> phase list =
 fun phase ->
  let range_incl_excl i j =
    let rec aux n acc =
      if n < i then
        acc
      else
        aux (n - 1) (n :: acc)
    in
    aux (j - 1) []
  in
  range_incl_excl min_phase (phase_to_enum phase)
  |> List.map ~f:(fun enum -> Option.value_exn (phase_of_enum enum))

type format =
  | Context  (** Underlined references and color *)
  | Raw  (** Compact format with color but no references *)
  | Highlighted  (** Numbered and colored references *)
  | Plain  (** Verbose positions and no color *)

let claim_as_reason : Pos.t Message.t -> Pos_or_decl.t Message.t =
 (fun (p, m) -> (Pos_or_decl.of_raw_pos p, m))

(* The file and phase of analysis being currently performed *)
let current_context : (Relative_path.t * phase) ref =
  ref (Relative_path.default, Typing)

let current_span : Pos.t ref = ref Pos.none

let ignore_pos_outside_current_span : bool ref = ref false

let allow_errors_in_default_path = ref true

module PhaseMap = struct
  include Reordered_argument_map (WrappedMap.Make (struct
    type t = phase

    let rank = function
      | Parsing -> 1
      | Naming -> 2
      | Decl -> 3
      | Typing -> 4

    let compare x y = rank x - rank y
  end))

  let pp pp_data = make_pp pp_phase pp_data

  let show pp_data x = Format.asprintf "%a" (pp pp_data) x
end

(** Results of single file analysis. *)
type 'a file_t = 'a list PhaseMap.t [@@deriving eq, show]

(** Results of multi-file analysis. *)
type 'a files_t = 'a file_t Relative_path.Map.t [@@deriving eq, show]

type finalized_error = (Pos.absolute, Pos.absolute) User_error.t
[@@deriving eq, ord, show]

type error = (Pos.t, Pos_or_decl.t) User_error.t [@@deriving eq, ord, show]

type per_file_errors = error file_t

type t = error files_t [@@deriving eq, show]

let files_t_fold v ~f ~init =
  Relative_path.Map.fold v ~init ~f:(fun path v acc ->
      PhaseMap.fold v ~init:acc ~f:(fun phase v acc -> f path phase v acc))

let files_t_map v ~f = Relative_path.Map.map v ~f:(fun v -> PhaseMap.map v ~f)

(** [files_t_merge f x y] is a merge by [file] & [phase], i.e. given "x:t" and "y:t" it
goes through every (file * phase) mentioned in either of them, and combines
the error-lists using the provided callback. Cost is O(x). *)
let files_t_merge
    ~(f :
       phase ->
       Relative_path.t ->
       error list option ->
       error list option ->
       error list option)
    (x : t)
    (y : t) : t =
  (* Using fold instead of merge to make the runtime proportional to the size
   * of first argument (like List.rev_append ) *)
  Relative_path.Map.fold x ~init:y ~f:(fun path x acc ->
      let y =
        Option.value (Relative_path.Map.find_opt y path) ~default:PhaseMap.empty
      in
      Relative_path.Map.add
        acc
        ~key:path
        ~data:(PhaseMap.merge x y ~f:(fun phase x y -> f phase path x y)))

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

module Error = struct
  type t = error [@@deriving ord]
end

module ErrorSet = Caml.Set.Make (Error)

module FinalizedError = struct
  type t = finalized_error [@@deriving ord]
end

module FinalizedErrorSet = Caml.Set.Make (FinalizedError)

let drop_fixmed_errors (errs : ('a, 'b) User_error.t list) :
    ('a, 'b) User_error.t list =
  List.filter errs ~f:(fun e -> not e.User_error.is_fixmed)

let drop_fixmed_errors_in_file (ef : ('a, 'b) User_error.t file_t) :
    ('a, 'b) User_error.t file_t =
  PhaseMap.fold ef ~init:PhaseMap.empty ~f:(fun phase errs acc ->
      match drop_fixmed_errors errs with
      | [] -> acc
      | errs -> PhaseMap.add acc ~key:phase ~data:errs)

let drop_fixmed_errors_in_files efs =
  Relative_path.Map.fold
    efs
    ~init:Relative_path.Map.empty
    ~f:(fun file errs acc ->
      let errs_without_fixmes = drop_fixmed_errors_in_file errs in
      if PhaseMap.is_empty errs_without_fixmes then
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
let get_error_list ?(drop_fixmed = true) err =
  files_t_to_list (drop_fixmes_if err drop_fixmed)

let (error_map : error files_t ref) = ref Relative_path.Map.empty

let accumulate_errors = ref false

(* Are we in the middle of folding a decl? ("lazy" means it happens on-demand
   during the course of the typecheck, rather than all upfront). *)
let in_lazy_decl = ref false

let (is_hh_fixme : (Pos.t -> error_code -> bool) ref) = ref (fun _ _ -> false)

let badpos_message =
  Printf.sprintf
    "Incomplete position information! Your type error is in this file, but we could only find related positions in another file. %s"
    Error_message_sentinel.please_file_a_bug_message

let badpos_message_2 =
  Printf.sprintf
    "Incomplete position information! We couldn't find the exact line of your type error in this definition. %s"
    Error_message_sentinel.please_file_a_bug_message

let try_with_result (f1 : unit -> 'res) (f2 : 'res -> error -> 'res) : 'res =
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
  | Some User_error.{ code; claim; reasons; quickfixes; is_fixmed = _ } ->
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
    f2 result @@ User_error.make code claim reasons ~quickfixes

let try_with_result_pure ~fail f g =
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
            let result = f () in
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
  | None when not @@ fail result -> result
  | _ -> g result

(* Reset errors before running [f] so that we can return the errors
 * caused by f. These errors are not added in the global list of errors. *)
let do_ ?(apply_fixmes = true) ?(drop_fixmed = true) f =
  let error_map_copy = !error_map in
  let accumulate_errors_copy = !accumulate_errors in
  error_map := Relative_path.Map.empty;
  accumulate_errors := true;
  let is_hh_fixme_copy = !is_hh_fixme in
  (if not apply_fixmes then is_hh_fixme := (fun _ _ -> false));
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
            is_hh_fixme := is_hh_fixme_copy
        end
  in
  let out_errors = files_t_map ~f:List.rev out_errors in
  (drop_fixmes_if out_errors drop_fixmed, result)

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
let phase_to_string (phase : phase) : string =
  match phase with
  | Parsing -> "Parsing"
  | Naming -> "Naming"
  | Decl -> "Decl"
  | Typing -> "Typing"

let phase_of_string (value : string) : phase option =
  match Caml.String.lowercase_ascii value with
  | "parsing" -> Some Parsing
  | "naming" -> Some Naming
  | "decl" -> Some Decl
  | "typing" -> Some Typing
  | _ -> None

let compare_internal (x : error) (y : error) ~compare_code : int =
  let User_error.
        {
          code = x_code;
          claim = x_claim;
          reasons = x_messages;
          quickfixes = _;
          is_fixmed = _;
        } =
    x
  in
  let User_error.
        {
          code = y_code;
          claim = y_claim;
          reasons = y_messages;
          quickfixes = _;
          is_fixmed = _;
        } =
    y
  in
  (* The primary sort order is by file *)
  let comparison =
    Relative_path.compare
      (fst x_claim |> Pos.filename)
      (fst y_claim |> Pos.filename)
  in
  (* Then within each file, sort by phase *)
  let comparison =
    if comparison = 0 then
      compare_code x_code y_code
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
    (List.compare (Message.compare Pos_or_decl.compare)) x_messages y_messages
  else
    comparison

let compare (x : error) (y : error) : int =
  compare_internal x y ~compare_code:Int.compare

let sort (err : error list) : error list =
  let compare_exact_code = Int.compare in
  let compare_phase x_code y_code =
    Int.compare (x_code / 1000) (y_code / 1000)
  in
  (* Sort using the exact code to ensure sort stability, but use the phase to deduplicate *)
  let equal x y = compare_internal ~compare_code:compare_phase x y = 0 in
  List.sort
    ~compare:(fun x y -> compare_internal ~compare_code:compare_exact_code x y)
    err
  |> List.remove_consecutive_duplicates
       ~equal:(fun x y -> equal x y)
       ~which_to_keep:`First (* Match Rust dedup_by *)

let get_sorted_error_list ?(drop_fixmed = true) err =
  sort (files_t_to_list (drop_fixmes_if err drop_fixmed))

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

let do_with_context ?(drop_fixmed = true) path phase f =
  run_in_context path phase (fun () -> do_ ~drop_fixmed f)

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
let format_summary format ~displayed_count ~dropped_count ~max_errors :
    string option =
  match format with
  | Context
  | Highlighted ->
    let found_count = displayed_count + Option.value dropped_count ~default:0 in
    let found_message =
      Printf.sprintf
        "%d error%s found"
        found_count
        (if found_count = 1 then
          ""
        else
          "s")
    in
    let truncated_message =
      match (max_errors, dropped_count) with
      | (Some max_errors, Some dropped_count) when dropped_count > 0 ->
        Printf.sprintf
          " (only showing first %d, dropped %d).\n"
          max_errors
          dropped_count
      | (Some max_errors, None) when displayed_count >= max_errors ->
        Printf.sprintf
          " (max-errors is %d; more errors may exist).\n"
          max_errors
      | _ -> ".\n"
    in
    Some (found_message ^ truncated_message)
  | Raw
  | Plain ->
    None

let report_pos_from_reason = ref false

let to_string error = User_error.to_string !report_pos_from_reason error

let log_unexpected error path desc =
  HackEventLogger.invariant_violation_bug
    desc
    ~path
    ~pos:
      (error
      |> User_error.to_absolute
      |> User_error.get_pos
      |> Pos.show_absolute);
  Hh_logger.log
    "UNEXPECTED: %s\n%s"
    desc
    (Exception.get_current_callstack_string 99 |> Exception.clean_stack);
  ()

let add_error_impl error =
  let () =
    match !current_context with
    | (path, Decl) ->
      log_unexpected error path "didn't expect to see Errors.Decl"
    | _ -> ()
  in
  if !accumulate_errors then
    let () =
      match !current_context with
      | (path, _)
        when Relative_path.equal path Relative_path.default
             && not !allow_errors_in_default_path ->
        log_unexpected error path "adding an error in default path"
      | _ -> ()
    in
    (* Cheap test to avoid duplicating most recent error *)
    let error_list = get_current_list !error_map in
    match error_list with
    | old_error :: _ when equal_error error old_error -> ()
    | _ -> set_current_list error_map (error :: error_list)
  else
    (* We have an error, but haven't handled it in any way *)
    let msg = error |> User_error.to_absolute |> to_string in
    if !in_lazy_decl then
      lazy_decl_error_logging msg error_map User_error.to_absolute to_string
    else if error.User_error.is_fixmed then
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

let set_allow_errors_in_default_path x = allow_errors_in_default_path := x

let is_allowed_code_strict code = ISet.mem code !allowed_fixme_codes_strict

let (get_hh_fixme_pos : (Pos.t -> error_code -> Pos.t option) ref) =
  ref (fun _ _ -> None)

let (is_hh_fixme_disallowed : (Pos.t -> error_code -> bool) ref) =
  ref (fun _ _ -> false)

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
  let User_error.{ code; claim; reasons = _; quickfixes; is_fixmed = _ } =
    error
  in
  let (pos, _) = claim in
  let pos = Option.value (!get_hh_fixme_pos pos code) ~default:pos in
  add_error_impl error;
  add_error_impl @@ User_error.make code (pos, explanation) [] ~quickfixes

let add_applied_fixme error =
  let error = { error with User_error.is_fixmed = true } in
  add_error_impl error

let rec add code pos msg = add_list code (pos, msg) []

and fixme_present (pos : Pos.t) code =
  !is_hh_fixme pos code || !is_hh_fixme_disallowed pos code

and add_list code ?quickfixes (claim : _ Message.t) reasons =
  add_error @@ User_error.make code claim reasons ?quickfixes

and add_error (error : error) =
  let User_error.{ code; claim; reasons; quickfixes; is_fixmed = _ } = error in
  let (claim, reasons) = check_pos_msg (claim, reasons) in
  let error = User_error.make code claim reasons ~quickfixes in

  let pos = fst claim in

  if not (fixme_present pos code) then
    (* Fixmes and banned decl fixmes are separated by the parser because Errors can't recover
     * the position information after the fact. This is the default case, where an HH_FIXME
     * comment is not present. Therefore, the remaining cases are variations on behavior when
     * a fixme is present *)
    add_error_impl error
  else if ISet.mem code hard_banned_codes then
    let explanation =
      Printf.sprintf
        "You cannot use `HH_FIXME` or `HH_IGNORE_ERROR` comments to suppress error %d, and this cannot be enabled by configuration"
        code
    in
    add_error_with_fixme_error error explanation
  else if Relative_path.(is_hhi (prefix (Pos.filename pos))) then
    add_applied_fixme error
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
  else if is_allowed_code_strict code then
    add_applied_fixme error
  else
    let explanation =
      Printf.sprintf
        "You cannot use `HH_FIXME` or `HH_IGNORE_ERROR` comments to suppress error %d"
        code
    in
    add_error_with_fixme_error error explanation

and merge err' err =
  let append _ _ x y =
    let x = Option.value x ~default:[] in
    let y = Option.value y ~default:[] in
    Some (List.rev_append x y)
  in
  files_t_merge ~f:append err' err

and incremental_update
    ~(old : t) ~(new_ : t) ~(rechecked : Relative_path.Set.t) (phase : phase) :
    t =
  (* Helper to detect coding oversight *)
  let assert_path_is_real (path : Relative_path.t) (phase : phase) : unit =
    if Relative_path.equal path Relative_path.default then
      Utils.assert_false_log_backtrace
        (Some
           ("Default (untracked) error sources should not get into incremental "
           ^ "mode. There might be a missing call to `Errors.do_with_context`/"
           ^ "`run_in_context` somwhere or incorrectly used `Errors.from_error_list`."
           ^ "Phase: "
           ^ show_phase phase))
  in

  (* Helper to remove acc[path][phase]. If acc[path] becomes empty afterwards,
   * remove it too (i.e do not store empty maps or lists ever). *)
  let remove (path : Relative_path.t) (phase : phase) (acc : t) : t =
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
  let res : t =
    files_t_merge new_ old ~f:(fun phase path new_ old ->
        assert_path_is_real path phase;
        match new_ with
        | Some new_ -> Some (List.rev new_)
        | None -> old)
  in
  (* For files that were rechecked, but had no errors - remove them from maps *)
  let res : t =
    Relative_path.Set.fold rechecked ~init:res ~f:(fun path acc ->
        let has_errors =
          match Relative_path.Map.find_opt new_ path with
          | None -> false
          | Some phase_map -> PhaseMap.mem phase_map phase
        in
        if has_errors then
          acc
        else
          remove path phase acc)
  in
  res

and from_error_list err = list_to_files_t err

let count ?(drop_fixmed = true) err =
  files_t_fold
    (drop_fixmes_if err drop_fixmed)
    ~f:(fun _ _ x acc -> acc + List.length x)
    ~init:0

let is_empty ?(drop_fixmed = true) err =
  Relative_path.Map.is_empty (drop_fixmes_if err drop_fixmed)

let add_parsing_error err = add_error @@ Parsing_error.to_user_error err

let add_nast_check_error err = add_error @@ Nast_check_error.to_user_error err

let is_suppressed User_error.{ claim; code; _ } =
  fixme_present Message.(get_message_pos claim) code

let apply_error_from_reasons_callback ?code ?claim ?reasons ?quickfixes err =
  Typing_error.(
    Eval_result.iter ~f:add_error
    @@ Eval_result.suppress_intersection ~is_suppressed
    @@ Reasons_callback.apply
         ?code
         ?claim
         ?reasons
         ?quickfixes
         err
         ~current_span:!current_span)

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

let log_primary_typing_error prim_err =
  let open Typing_error.Primary in
  match prim_err with
  | Exception_occurred { pos; exn } -> log_exception_occurred pos exn
  | Invariant_violation { pos; telemetry; desc; _ } ->
    log_invariant_violation ~desc pos telemetry
  | _ -> ()

let add_typing_error err =
  Typing_error.(
    iter ~on_prim:log_primary_typing_error ~on_snd:(fun _ -> ()) err;
    Eval_result.iter ~f:add_error
    @@ Eval_result.suppress_intersection ~is_suppressed
    @@ to_user_error err ~current_span:!current_span)

and empty = Relative_path.Map.empty

let from_file_error_list : ?phase:phase -> (Relative_path.t * error) list -> t =
 fun ?(phase = Typing) errors ->
  List.fold errors ~init:Relative_path.Map.empty ~f:(fun errors (file, error) ->
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

let as_map : t -> error list Relative_path.Map.t =
 fun errors ->
  Relative_path.Map.map
    errors
    ~f:(PhaseMap.fold ~init:[] ~f:(fun _ -> List.append))

let merge_into_current errors = error_map := merge errors !error_map

(* Until we return a list of errors from typing, we have to apply
   'client errors' to a callback for using in subtyping *)
let apply_callback_to_errors : t -> Typing_error.Reasons_callback.t -> unit =
 fun errors on_error ->
  let on_error
      User_error.{ code; claim; reasons; quickfixes = _; is_fixmed = _ } =
    Typing_error.(
      let code = Option.value_exn (Error_code.of_enum code) in
      Eval_result.iter ~f:add_error
      @@ Eval_result.suppress_intersection ~is_suppressed
      @@ Reasons_callback.apply
           on_error
           ~code
           ~claim:(lazy claim)
           ~reasons:(lazy reasons)
           ~current_span:!current_span)
  in
  Relative_path.Map.iter errors ~f:(fun _ ->
      PhaseMap.iter ~f:(fun _ -> List.iter ~f:on_error))

(*****************************************************************************)
(* Accessors. (All methods delegated to the parameterized module.) *)
(*****************************************************************************)

let per_file_error_count : ?drop_fixmed:bool -> per_file_errors -> int =
 fun ?(drop_fixmed = true) ->
  PhaseMap.fold ~init:0 ~f:(fun _phase errors count ->
      let errors =
        if drop_fixmed then
          drop_fixmed_errors errors
        else
          errors
      in
      List.length errors + count)

let get_file_errors :
    ?drop_fixmed:bool -> t -> Relative_path.t -> per_file_errors =
 fun ?(drop_fixmed = true) errors file ->
  Relative_path.Map.find_opt (drop_fixmes_if errors drop_fixmed) file
  |> Option.value ~default:PhaseMap.empty

let iter_error_list ?(drop_fixmed = true) f err =
  List.iter ~f (get_sorted_error_list ~drop_fixmed err)

let fold_errors ?(drop_fixmed = true) ?phase err ~init ~f =
  let err = drop_fixmes_if err drop_fixmed in
  match phase with
  | None ->
    files_t_fold err ~init ~f:(fun source phase errors acc ->
        List.fold_right errors ~init:acc ~f:(f source phase))
  | Some phase ->
    Relative_path.Map.fold err ~init ~f:(fun source phases acc ->
        match PhaseMap.find_opt phases phase with
        | None -> acc
        | Some errors -> List.fold_right errors ~init:acc ~f:(f source phase))

let fold_errors_in ?(drop_fixmed = true) ?phase err ~file ~init ~f =
  let err = drop_fixmes_if err drop_fixmed in
  Relative_path.Map.find_opt err file
  |> Option.value ~default:PhaseMap.empty
  |> PhaseMap.fold ~init ~f:(fun p errors acc ->
         match phase with
         | Some x when not (equal_phase x p) -> acc
         | _ -> List.fold_right errors ~init:acc ~f)

(* Get paths that have errors which haven't been HH_FIXME'd. *)
let get_failed_files (err : t) phase =
  let err = drop_fixmed_errors_in_files err in
  files_t_fold err ~init:Relative_path.Set.empty ~f:(fun source p _ acc ->
      if not (equal_phase phase p) then
        acc
      else
        Relative_path.Set.add acc source)

(* Count errors which haven't been HH_FIXME'd. *)
let error_count : t -> int =
 fun errors ->
  Relative_path.Map.fold errors ~init:0 ~f:(fun _path errors count ->
      count + per_file_error_count ~drop_fixmed:true errors)

exception Done of ISet.t

let first_n_distinct_error_codes : n:int -> t -> error_code list =
 fun ~n (errors : _ files_t) ->
  let codes =
    try
      Relative_path.Map.fold
        errors
        ~init:ISet.empty
        ~f:(fun _path errors codes ->
          PhaseMap.fold errors ~init:codes ~f:(fun _phase errors codes ->
              List.fold
                errors
                ~init:codes
                ~f:(fun codes User_error.{ code; _ } ->
                  let codes = ISet.add code codes in
                  if ISet.cardinal codes >= n then
                    raise (Done codes)
                  else
                    codes)))
    with
    | Done codes -> codes
  in
  ISet.elements codes

(* Get the error code of the first error which hasn't been HH_FIXME'd. *)
let choose_code_opt (errors : t) : int option =
  drop_fixmed_errors_in_files errors
  |> first_n_distinct_error_codes ~n:1
  |> List.hd

let as_telemetry : t -> Telemetry.t =
 fun errors ->
  Telemetry.create ()
  |> Telemetry.int_ ~key:"count" ~value:(error_count errors)
  |> Telemetry.int_list
       ~key:"first_5_distinct_error_codes"
       ~value:
         (first_n_distinct_error_codes
            ~n:5
            (drop_fixmed_errors_in_files errors))

(*****************************************************************************)
(* Error code printing. *)
(*****************************************************************************)

let internal_error pos msg = add 0 pos ("Internal error: " ^ msg)

let unimplemented_feature pos msg = add 0 pos ("Feature not implemented: " ^ msg)

let experimental_feature pos msg =
  add 0 pos ("Cannot use experimental feature: " ^ msg)

(*****************************************************************************)
(* Typing errors *)
(*****************************************************************************)

(*****************************************************************************)
(* Typing decl errors *)
(*****************************************************************************)

(** TODO: Remove use of `User_error.t` representation for nested error &
    callback application *)
let ambiguous_inheritance
    pos class_ origin error (on_error : Typing_error.Reasons_callback.t) =
  let User_error.{ code; claim; reasons; quickfixes = _; is_fixmed = _ } =
    error
  in
  let origin = Render.strip_ns origin in
  let class_ = Render.strip_ns class_ in
  let message =
    "This declaration was inherited from an object of type "
    ^ Markdown_lite.md_codify origin
    ^ ". Redeclare this member in "
    ^ Markdown_lite.md_codify class_
    ^ " with a compatible signature."
  in
  let code = Option.value_exn (Error_codes.Typing.of_enum code) in
  apply_error_from_reasons_callback
    on_error
    ~code
    ~reasons:(lazy ((claim_as_reason claim :: reasons) @ [(pos, message)]))

(** TODO: Remove use of `User_error.t` representation for nested error  *)
let function_is_not_dynamically_callable function_name error =
  let function_name = Markdown_lite.md_codify (Render.strip_ns function_name) in
  let nested_error_reason = User_error.to_list_ error in
  add_list
    (User_error.get_code error)
    ( User_error.get_pos error,
      "Function  " ^ function_name ^ " is not dynamically callable." )
    nested_error_reason

(** TODO: Remove use of `User_error.t` representation for nested error  *)
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
      ( User_error.get_code error,
        User_error.get_pos error,
        User_error.to_list_ error )
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
    code
    ( pos,
      "Method  "
      ^ method_name
      ^ " in class "
      ^ class_name
      ^ " is not dynamically callable." )
    (parent_class_reason @ attribute_reason @ nested_error_reason)

(* Raise different types of error for accessing global variables. *)
let global_access_error error_code pos message =
  add (GlobalAccessCheck.err_code error_code) pos message

(*****************************************************************************)
(* Printing *)
(*****************************************************************************)

let convert_errors_to_string ?(include_filename = false) (errors : error list) :
    string list =
  List.fold_right
    ~init:[]
    ~f:(fun err acc_out ->
      let User_error.{ code = _; claim; reasons; quickfixes = _; is_fixmed = _ }
          =
        err
      in
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

let try_pred ~fail f g = try_with_result_pure ~fail f (fun _ -> g ())

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

let try_when f ~if_error_and:condition ~then_:do_ =
  try_with_result f (fun result error ->
      if condition () then
        do_ error
      else
        add_error error;
      result)
