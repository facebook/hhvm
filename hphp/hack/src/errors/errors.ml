(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type error_code = int

type format =
  | Context  (** Underlined references and color *)
  | Raw  (** Compact format with color but no references *)
  | Highlighted  (** Numbered and colored references *)
  | Plain  (** Verbose positions and no color *)

let claim_as_reason : Pos.t Message.t -> Pos_or_decl.t Message.t =
 (fun (p, m) -> (Pos_or_decl.of_raw_pos p, m))

(* The file of analysis being currently performed *)
let current_file : Relative_path.t ref = ref Relative_path.default

let current_span : Pos.t ref = ref Pos.none

let ignore_pos_outside_current_span : bool ref = ref false

let allow_errors_in_default_path = ref true

type finalized_error = (Pos.absolute, Pos.absolute) User_error.t
[@@deriving eq, ord, show]

type error = (Pos.t, Pos_or_decl.t) User_error.t
[@@deriving eq, hash, ord, show]

type per_file_errors = error list

type t = error list Relative_path.Map.t [@@deriving eq, show]

let files_t_fold v ~f ~init =
  Relative_path.Map.fold v ~init ~f:(fun path v acc -> f path v acc)

let files_t_map v ~f = Relative_path.Map.map v ~f

(** [files_t_merge f x y] is a merge by [file], i.e. given "x:t" and "y:t" it
goes through every (file) mentioned in either of them, and combines
the error-lists using the provided callback. Cost is O(x). *)
let files_t_merge
    ~(f :
       Relative_path.t ->
       error list option ->
       error list option ->
       error list option)
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
let from_error_list errors =
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

let iter t ~f =
  Relative_path.Map.iter t ~f:(fun _path errors -> List.iter errors ~f)

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
let get_error_list ?(drop_fixmed = true) err =
  files_t_to_list (drop_fixmes_if err drop_fixmed)

let (error_map : error list Relative_path.Map.t ref) =
  ref Relative_path.Map.empty

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
  | Some
      User_error.
        { code; claim; reasons; quickfixes; custom_msgs; is_fixmed = _ } ->
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
    f2 result @@ User_error.make code claim reasons ~quickfixes ~custom_msgs

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
    (Caml.Printexc.raw_backtrace_to_string (Caml.Printexc.get_callstack 500));

  (* Exit with special error code so we can see the log after *)
  Exit.exit Exit_status.Lazy_decl_bug

(*****************************************************************************)
(* Error code printing. *)
(*****************************************************************************)
let compare_internal
    (x : ('a, 'b) User_error.t)
    (y : ('a, 'b) User_error.t)
    ~compare_code
    ~compare_claim_fn
    ~compare_claim
    ~compare_reason : int =
  let User_error.
        {
          code = x_code;
          claim = x_claim;
          reasons = x_messages;
          custom_msgs = _;
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
          custom_msgs = _;
          quickfixes = _;
          is_fixmed = _;
        } =
    y
  in
  (* The primary sort order is by file *)
  let comparison =
    compare_claim_fn (fst x_claim |> Pos.filename) (fst y_claim |> Pos.filename)
  in
  (* Then within each file, sort by category *)
  let comparison =
    if comparison = 0 then
      compare_code x_code y_code
    else
      comparison
  in
  (* If the error codes are the same, sort by position *)
  let comparison =
    if comparison = 0 then
      compare_claim (fst x_claim) (fst y_claim)
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
    (List.compare (Message.compare compare_reason)) x_messages y_messages
  else
    comparison

let compare (x : error) (y : error) : int =
  compare_internal
    x
    y
    ~compare_code:Int.compare
    ~compare_claim_fn:Relative_path.compare
    ~compare_claim:Pos.compare
    ~compare_reason:Pos_or_decl.compare

let compare_finalized (x : finalized_error) (y : finalized_error) : int =
  compare_internal
    x
    y
    ~compare_code:Int.compare
    ~compare_claim_fn:String.compare
    ~compare_claim:Pos.compare_absolute
    ~compare_reason:Pos.compare_absolute

let sort (err : error list) : error list =
  let compare_exact_code = Int.compare in
  let compare_category x_code y_code =
    Int.compare (x_code / 1000) (y_code / 1000)
  in
  let compare_claim_fn = Relative_path.compare in
  let compare_claim = Pos.compare in
  let compare_reason = Pos_or_decl.compare in
  (* Sort using the exact code to ensure sort stability, but use the category to deduplicate *)
  let equal x y =
    compare_internal
      ~compare_code:compare_category
      ~compare_claim_fn
      ~compare_claim
      ~compare_reason
      x
      y
    = 0
  in
  List.sort
    ~compare:(fun x y ->
      compare_internal
        ~compare_code:compare_exact_code
        ~compare_claim_fn
        ~compare_claim
        ~compare_reason
        x
        y)
    err
  |> List.remove_consecutive_duplicates
       ~equal:(fun x y -> equal x y)
       ~which_to_keep:`First (* Match Rust dedup_by *)

let get_sorted_error_list ?(drop_fixmed = true) err =
  sort (files_t_to_list (drop_fixmes_if err drop_fixmed))

let sort_and_finalize (errors : t) : finalized_error list =
  errors |> get_sorted_error_list |> List.map ~f:User_error.to_absolute

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

let add_error_with_fixme_error error explanation =
  let User_error.
        { code; claim; reasons = _; quickfixes; custom_msgs; is_fixmed = _ } =
    error
  in
  let (pos, _) = claim in
  let pos = Option.value (!get_hh_fixme_pos pos code) ~default:pos in
  add_error_impl error;
  add_error_impl
  @@ User_error.make code (pos, explanation) [] ~quickfixes ~custom_msgs

let add_applied_fixme error =
  let error = { error with User_error.is_fixmed = true } in
  add_error_impl error

let rec add code pos msg = add_list code (pos, msg) []

and fixme_present (pos : Pos.t) code =
  !is_hh_fixme pos code || !is_hh_fixme_disallowed pos code

and add_list code ?quickfixes (claim : _ Message.t) reasons =
  add_error @@ User_error.make code claim reasons ?quickfixes

and add_error (error : error) =
  let User_error.
        { code; claim; reasons; quickfixes; custom_msgs; is_fixmed = _ } =
    error
  in
  let (claim, reasons) = check_pos_msg (claim, reasons) in
  let error = User_error.make code claim reasons ~quickfixes ~custom_msgs in

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
  let append _ x y =
    let x = Option.value x ~default:[] in
    let y = Option.value y ~default:[] in
    Some (List.rev_append x y)
  in
  files_t_merge ~f:append err' err

and incremental_update ~(old : t) ~(new_ : t) ~(rechecked : Relative_path.Set.t)
    : t =
  (* Helper to detect coding oversight *)
  let assert_path_is_real (path : Relative_path.t) : unit =
    if Relative_path.equal path Relative_path.default then
      Utils.assert_false_log_backtrace
        (Some
           ("Default (untracked) error sources should not get into incremental "
           ^ "mode. There might be a missing call to `Errors.do_with_context`/"
           ^ "`run_in_context` somwhere or incorrectly used `Errors.from_error_list`."
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

let count ?(drop_fixmed = true) err =
  files_t_fold
    (drop_fixmes_if err drop_fixmed)
    ~f:(fun _ x acc -> acc + List.length x)
    ~init:0

let is_empty ?(drop_fixmed = true) err =
  Relative_path.Map.is_empty (drop_fixmes_if err drop_fixmed)

let get_current_span () = !current_span

and empty = Relative_path.Map.empty

let from_file_error_list (errors : (Relative_path.t * error) list) : t =
  List.fold errors ~init:Relative_path.Map.empty ~f:(fun errors (file, error) ->
      let errors_for_file =
        Relative_path.Map.find_opt errors file |> Option.value ~default:[]
      in
      let errors_for_file = error :: errors_for_file in
      Relative_path.Map.add errors ~key:file ~data:errors_for_file)

let as_map (t : t) : error list Relative_path.Map.t = t

let merge_into_current errors = error_map := merge errors !error_map

(*****************************************************************************)
(* Accessors. (All methods delegated to the parameterized module.) *)
(*****************************************************************************)

let per_file_error_count ?(drop_fixmed = true) (errors : per_file_errors) : int
    =
  let errors =
    if drop_fixmed then
      drop_fixmed_errors errors
    else
      errors
  in
  List.length errors

let get_file_errors ?(drop_fixmed = true) (t : t) (file : Relative_path.t) :
    per_file_errors =
  match Relative_path.Map.find_opt t file with
  | None -> []
  | Some errors ->
    if drop_fixmed then
      drop_fixmed_errors errors
    else
      errors

let fold_per_file_errors (errors : per_file_errors) ~init ~f =
  List.fold errors ~init ~f

let iter_error_list ?(drop_fixmed = true) f err =
  List.iter ~f (get_sorted_error_list ~drop_fixmed err)

let fold_errors
    ?(drop_fixmed = true)
    (t : t)
    ~(init : 'a)
    ~(f : Relative_path.t -> error -> 'a -> 'a) =
  let t = drop_fixmes_if t drop_fixmed in
  files_t_fold t ~init ~f:(fun source errors acc ->
      List.fold_right errors ~init:acc ~f:(f source))

let fold_errors_in
    ?(drop_fixmed = true)
    (t : t)
    ~(file : Relative_path.t)
    ~(init : 'a)
    ~(f : error -> 'a -> 'a) =
  let t = drop_fixmes_if t drop_fixmed in
  match Relative_path.Map.find_opt t file with
  | None -> init
  | Some errors -> List.fold_right errors ~init ~f

(* Get paths that have errors which haven't been HH_FIXME'd. *)
let get_failed_files (err : t) =
  let err = drop_fixmed_errors_in_files err in
  files_t_fold err ~init:Relative_path.Set.empty ~f:(fun source _ acc ->
      Relative_path.Set.add acc source)

(* Count errors which haven't been HH_FIXME'd. *)
let error_count : t -> int =
 fun errors ->
  Relative_path.Map.fold errors ~init:0 ~f:(fun _path errors count ->
      count + per_file_error_count ~drop_fixmed:true errors)

exception Done of ISet.t

let first_n_distinct_error_codes ~(n : int) (t : t) : error_code list =
  let codes =
    try
      Relative_path.Map.fold t ~init:ISet.empty ~f:(fun _path errors codes ->
          List.fold errors ~init:codes ~f:(fun codes User_error.{ code; _ } ->
              let codes = ISet.add code codes in
              if ISet.cardinal codes >= n then
                raise (Done codes)
              else
                codes))
    with
    | Done codes -> codes
  in
  ISet.elements codes

(* Get the error code of the first error which hasn't been HH_FIXME'd. *)
let choose_code_opt (t : t) : int option =
  drop_fixmed_errors_in_files t |> first_n_distinct_error_codes ~n:1 |> List.hd

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

let unimplemented_feature pos msg = add 0 pos ("Feature not implemented: " ^ msg)

let experimental_feature pos msg =
  add 0 pos ("Cannot use experimental feature: " ^ msg)

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
    add_error
    @@ User_error.make
         Error_codes.Typing.(to_enum InvariantViolated)
         (pos, internal_compiler_error_msg)
         []

let exception_occurred pos exn =
  log_exception_occurred pos exn;
  add_error
  @@ User_error.make
       Error_codes.Typing.(to_enum ExceptionOccurred)
       (pos, internal_compiler_error_msg)
       []

let internal_error pos msg =
  add_error
  @@ User_error.make
       Error_codes.Typing.(to_enum InternalError)
       (pos, "Internal error: " ^ msg)
       []

let typechecker_timeout pos fn_name seconds =
  let claim =
    ( pos,
      Printf.sprintf
        "Type checker timed out after %d seconds whilst checking function %s"
        seconds
        fn_name )
  in
  add_error
  @@ User_error.make Error_codes.Typing.(to_enum TypecheckerTimeout) claim []

(*****************************************************************************)
(* Typing decl errors *)
(*****************************************************************************)

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
      let User_error.
            {
              code = _;
              claim;
              reasons;
              custom_msgs;
              quickfixes = _;
              is_fixmed = _;
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
