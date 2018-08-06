(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Utils
open Reordered_argument_collections
open String_utils

type error_code = int
(* We use `Pos.t message` on the server and convert to `Pos.absolute message`
 * before sending it to the client *)
type 'a message = 'a * string

type error_phase = Init | Parsing | Naming | Decl | Typing
type error_severity = Warning | Error

(* For callers that don't care about tracking error origins *)
let default_context = (Relative_path.default, Typing)

(* The file and phase of analysis being currently performed *)
let current_context: (Relative_path.t * error_phase) ref = ref default_context

let allow_errors_in_default_path = ref true

module PhaseMap = Reordered_argument_map(MyMap.Make(struct
  type t = error_phase

  let rank = function
    | Init -> 0
    | Parsing -> 1
    | Naming -> 2
    | Decl -> 3
    | Typing -> 4

  let compare x y = (rank x) - (rank y)
end))

(* Results of single file analysis. *)
type 'a file_t = 'a list PhaseMap.t
(* Results of multi-file analysis. *)
type 'a files_t = ('a file_t) Relative_path.Map.t

let files_t_fold v ~f ~init =
  Relative_path.Map.fold v ~init ~f:begin fun path v acc ->
    PhaseMap.fold v ~init:acc ~f:begin fun phase v acc ->
      f path phase v acc
    end
  end

let files_t_map v ~f =
  Relative_path.Map.map v ~f:begin fun v ->
    PhaseMap.map v ~f
  end

let files_t_merge ~f x y =
  (* Using fold instead of merge to make the runtime proportional to the size
   * of first argument (like List.rev_append ) *)
  Relative_path.Map.fold x ~init:y ~f:begin fun k x acc ->
    let y = Option.value (Relative_path.Map.get y k) ~default:PhaseMap.empty in
    Relative_path.Map.add acc k (
      PhaseMap.merge x y ~f:(fun phase x y -> f phase k x y)
    )
  end

let files_t_to_list x =
  files_t_fold x ~f:(fun _ _ x acc -> List.rev_append x acc) ~init:[] |>
  List.rev

let list_to_files_t = function
  | [] -> Relative_path.Map.empty
  | x ->
    (* Values constructed here should not be used with incremental mode.
     * See assert in incremental_update. *)
    Relative_path.Map.singleton Relative_path.default
      (PhaseMap.singleton Typing x)

let get_code_severity code =
  if code = Error_codes.Init.err_code Error_codes.Init.ForwardCompatibilityNotCurrent
  then Warning
  else Error

module Common = struct

  (* Get most recently-ish added error. *)
  let get_last error_map =
    (* If this map has more than one element, we pick an arbitrary file. Because
     * of that, we might not end up with the most recent error and generate a
     * less-specific error message. This should be rare. *)
    match Relative_path.Map.max_binding error_map with
    | None -> None
    | Some (_, phase_map) -> begin
      let error_list = PhaseMap.max_binding phase_map
        |> Option.value_map ~f:snd ~default:[]
      in
      match List.rev error_list with
        | [] -> None
        | e::_ -> Some e
      end

  let try_with_result f1 f2 error_map accumulate_errors =
    let error_map_copy = !error_map in
    let accumulate_errors_copy = !accumulate_errors in
    error_map := Relative_path.Map.empty;
    accumulate_errors := true;
    let result, errors = Utils.try_finally
      ~f:begin fun () ->
        let result = f1 () in
        result, !error_map
      end
      ~finally:begin fun () ->
        error_map := error_map_copy;
        accumulate_errors := accumulate_errors_copy;
      end
    in
    match get_last errors with
    | None -> result
    | Some l -> f2 result l

  let do_ f error_map accumulate_errors applied_fixmes  =
    let error_map_copy = !error_map in
    let accumulate_errors_copy = !accumulate_errors in
    let applied_fixmes_copy = !applied_fixmes in
    error_map := Relative_path.Map.empty;
    applied_fixmes := Relative_path.Map.empty;
    accumulate_errors := true;
    let result, out_errors, out_applied_fixmes = Utils.try_finally
      ~f:begin fun () ->
        let result = f () in
        result, !error_map, !applied_fixmes
      end
      ~finally:begin fun () ->
        error_map := error_map_copy;
        applied_fixmes := applied_fixmes_copy;
        accumulate_errors := accumulate_errors_copy;
      end
    in
    let out_errors = files_t_map ~f:(List.rev) out_errors in
    (out_errors, out_applied_fixmes), result

  let run_in_context path phase f =
    let context_copy = !current_context in
    current_context := (path, phase);
    Utils.try_finally ~f ~finally:begin fun () ->
      current_context := context_copy;
    end

  (* Log important data if lazy_decl triggers a crash *)
  let lazy_decl_error_logging error error_map to_absolute to_string =
    let error_list = files_t_to_list !error_map in
    (* Print the current error list, which should be empty *)
    Printf.eprintf "%s" "Error list(should be empty):\n";
    List.iter error_list ~f:(fun err ->
        let msg = err |> to_absolute |> to_string in Printf.eprintf "%s\n" msg);
    Printf.eprintf "%s" "Offending error:\n";
    Printf.eprintf "%s" error;

    (* Print out a larger stack trace *)
    Printf.eprintf "%s" "Callstack:\n";
    Printf.eprintf "%s" (Printexc.raw_backtrace_to_string
      (Printexc.get_callstack 500));
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
    error_kind^"["^error_number^"]"

  let sort get_pos err =
    List.sort ~cmp:begin fun x y ->
      Pos.compare (get_pos x) (get_pos y)
    end err
    |> List.remove_consecutive_duplicates ~equal:(=)

  let get_sorted_error_list get_pos (err,_) =
    sort get_pos (files_t_to_list err)

  (* Getters and setter for passed-in map, based on current context *)
  let get_current_file_t file_t_map =
     let current_file = fst !current_context in
     Relative_path.Map.get file_t_map current_file |>
     Option.value ~default:PhaseMap.empty

  let get_current_list file_t_map =
    let current_phase = snd !current_context in
    get_current_file_t file_t_map |> fun x ->
    PhaseMap.get x current_phase |>
    Option.value ~default:[]

  let set_current_list file_t_map new_list =
    let current_file, current_phase = !current_context in
    file_t_map := Relative_path.Map.add
      !file_t_map
      current_file
      (PhaseMap.add
        (get_current_file_t !file_t_map)
        current_phase
        new_list
      )
end

(** The mode abstracts away the underlying errors type so errors can be
 * stored either with backtraces (TracingErrors) or without
 * (NonTracingErrors). *)
module type Errors_modes = sig

  type 'a error_
  type error = Pos.t error_
  type applied_fixme = Pos.t * int

  val applied_fixmes: applied_fixme files_t ref
  val error_map: error files_t ref

  val try_with_result: (unit -> 'a) -> ('a -> error -> 'a) -> 'a
  val do_: (unit -> 'a) -> (error files_t * applied_fixme files_t) * 'a
  val do_with_context: Relative_path.t -> error_phase ->
    (unit -> 'a) -> (error files_t * applied_fixme files_t) * 'a
  val run_in_context: Relative_path.t -> error_phase -> (unit -> 'a) -> 'a
  val run_in_decl_mode: Relative_path.t -> (unit -> 'a) -> 'a
  val add_error: error -> unit
  val make_error: error_code -> (Pos.t message) list -> error

  val get_code: 'a error_ -> error_code
  val get_pos: error -> Pos.t
  val get_severity: 'a error_ -> error_severity
  val to_list: 'a error_ -> 'a message list
  val to_absolute : error -> Pos.absolute error_

  val to_string : ?indent:bool -> Pos.absolute error_ -> string

  val get_sorted_error_list: error files_t * applied_fixme files_t -> error list
  val sort: error list -> error list
  val currently_has_errors : unit -> bool

end

(** Errors don't hace backtraces embedded. *)
module NonTracingErrors: Errors_modes = struct
  type 'a error_ = error_code * 'a message list
  type error = Pos.t error_
  type applied_fixme = Pos.t * int

  let applied_fixmes: applied_fixme files_t ref = ref Relative_path.Map.empty
  let (error_map: error files_t ref) = ref Relative_path.Map.empty
  let accumulate_errors = ref false
  (* Some filename when declaring *)
  let in_lazy_decl = ref None

  let try_with_result f1 f2 =
    Common.try_with_result f1 f2 error_map accumulate_errors

  let do_ f =
    Common.do_ f error_map accumulate_errors applied_fixmes

  let do_with_context path phase f =
    Common.run_in_context path phase (fun () -> do_ f)

  let run_in_context = Common.run_in_context

  (* Turn on lazy decl mode for the duration of the closure.
     This runs without returning the original state,
     since we collect it later in do_with_lazy_decls_
  *)
  let run_in_decl_mode filename f =
    let old_in_lazy_decl = !in_lazy_decl in
    in_lazy_decl := Some filename;
    Utils.try_finally ~f ~finally:begin fun () ->
      in_lazy_decl := old_in_lazy_decl;
    end

  and make_error code (x: (Pos.t * string) list) = ((code, x): error)

  (*****************************************************************************)
  (* Accessors. *)
  (*****************************************************************************)

  and get_code (error: 'a error_) = ((fst error): error_code)
  let get_pos (error : error) = fst (List.hd_exn (snd error))

  let get_severity (error: 'a error_) = get_code_severity (get_code error)

  let to_list (error : 'a error_) = snd error
  let to_absolute error =
    let code, msg_l = (get_code error), (to_list error) in
    let msg_l = List.map msg_l (fun (p, s) -> Pos.to_absolute p, s) in
    code, msg_l

  let to_string ?(indent=false) (error : Pos.absolute error_) : string =
    let error_code, msgl = (get_code error), (to_list error) in
    let buf = Buffer.create 50 in
    (match msgl with
    | [] -> assert false
    | (pos1, msg1) :: rest_of_error ->
        Buffer.add_string buf begin
          let error_code = Common.error_code_to_string error_code in
          Printf.sprintf "%s\n%s (%s)\n"
            (Pos.string pos1) msg1 error_code
      end;
        let indentstr = if indent then "  " else "" in
        List.iter rest_of_error begin fun (p, w) ->
          let msg = Printf.sprintf "%s%s\n%s%s\n"
              indentstr (Pos.string p) indentstr w in
          Buffer.add_string buf msg
        end
    );
    Buffer.contents buf

  let sort = Common.sort get_pos
  let get_sorted_error_list = Common.get_sorted_error_list get_pos

  let add_error error =
    if !accumulate_errors then
      let () = match !current_context with
        | (path, _) when path = Relative_path.default &&
            (not !allow_errors_in_default_path) ->
            Hh_logger.log "WARNING: adding an error in default path\n%s\n"
              (Printexc.raw_backtrace_to_string (Printexc.get_callstack 100))
        | _ -> ()
      in
      (* Cheap test to avoid duplicating most recent error *)
      let error_list = Common.get_current_list !error_map in
      match error_list with
      | old_error :: _ when error = old_error -> ()
      | _ -> Common.set_current_list error_map (error :: error_list)
    else
      (* We have an error, but haven't handled it in any way *)
      let msg = error |> to_absolute |> to_string in
      match !in_lazy_decl with
      | Some _ ->
        Common.lazy_decl_error_logging msg error_map to_absolute to_string
      | None -> assert_false_log_backtrace (Some msg)
  let currently_has_errors () =
    Common.get_current_list !error_map <> []

end

(** Errors with backtraces embedded. They are revealed with to_string. *)
module TracingErrors: Errors_modes = struct
  type 'a error_ = (Printexc.raw_backtrace * error_code * 'a message list)
  type error = Pos.t error_
  type applied_fixme = Pos.t * int

  let applied_fixmes: applied_fixme files_t ref = ref Relative_path.Map.empty
  let (error_map: error files_t ref) = ref Relative_path.Map.empty

  let accumulate_errors = ref false
  let in_lazy_decl = ref None

  let try_with_result f1 f2 =
    Common.try_with_result f1 f2 error_map accumulate_errors

  let do_ f =
    Common.do_ f error_map accumulate_errors applied_fixmes

  let run_in_context = Common.run_in_context

  let do_with_context path phase f =
    run_in_context path phase (fun () -> do_ f)

  (* Turn on lazy decl mode for the duration of the closure.
     This runs without returning the original state,
     since we collect it later in do_with_lazy_decls_
  *)
  let run_in_decl_mode filename f =
    in_lazy_decl := Some filename;
    Utils.try_finally ~f ~finally:begin fun () ->
      in_lazy_decl := None;
    end

  let make_error code (x: (Pos.t * string) list) =
    let bt = Printexc.get_callstack 25 in
    ((bt, code, x): error)

  let get_code ((_, c, _): 'a error_) = c

  let get_pos ((_, _, msg_l): error) =
    fst (List.hd_exn msg_l)

  let get_severity (error: 'a error_) = get_code_severity (get_code error)

  let get_bt ((bt, _, _): 'a error_) = bt

  let to_list ((_, _, l): 'a error_) = l

  let to_absolute (error: error) =
    let bt, code, msg_l = (get_bt error), (get_code error), (to_list error) in
    let msg_l = List.map msg_l (fun (p, s) -> Pos.to_absolute p, s) in
    bt, code, msg_l

  (** TODO: Much of this is copy-pasta. *)
  let to_string ?(indent=false) (error : Pos.absolute error_) : string =
    let bt, error_code, msgl = (get_bt error),
      (get_code error), (to_list error) in
    let buf = Buffer.create 50 in
    (match msgl with
    | [] -> assert false
    | (pos1, msg1) :: rest_of_error ->
        Buffer.add_string buf begin
          let error_code = Common.error_code_to_string error_code in
          Printf.sprintf "%s\n%s%s (%s)\n"
            (Pos.string pos1) (Printexc.raw_backtrace_to_string bt)
            msg1 error_code
        end;
        let indentstr = if indent then "  " else "" in
        List.iter rest_of_error begin fun (p, w) ->
          let msg = Printf.sprintf "%s%s\n%s%s\n"
              indentstr (Pos.string p) indentstr w in
          Buffer.add_string buf msg
        end
    );
    Buffer.contents buf

  let add_error error =
    if !accumulate_errors then
      begin
        let error_list = Common.get_current_list !error_map in
        Common.set_current_list error_map (error :: error_list)
      end
    else
    (* We have an error, but haven't handled it in any way *)
      let msg = error |> to_absolute |> to_string in
      match !in_lazy_decl with
      | Some _ ->
        Common.lazy_decl_error_logging msg error_map to_absolute to_string
      | None -> assert_false_log_backtrace (Some msg)

  let sort = Common.sort get_pos
  let get_sorted_error_list = Common.get_sorted_error_list get_pos
  let currently_has_errors () =
    Common.get_current_list !error_map <> []
end

(** The Errors functor which produces the Errors module.
 * Omitting gratuitous indentation. *)
module Errors_with_mode(M: Errors_modes) = struct

module Temporary = Error_codes.Temporary
module Parsing = Error_codes.Parsing
module Naming = Error_codes.Naming
module NastCheck = Error_codes.NastCheck
module Typing = Error_codes.Typing
module Init = Error_codes.Init

(*****************************************************************************)
(* Types *)
(*****************************************************************************)

type 'a error_ = 'a M.error_
type error = Pos.t error_
type applied_fixme = M.applied_fixme
type t = error files_t * applied_fixme files_t

type phase = error_phase = Init | Parsing | Naming | Decl | Typing
type severity = error_severity = Warning | Error

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

let applied_fixmes = M.applied_fixmes
let error_map = M.error_map

let default_ignored_fixme_codes = ISet.of_list [
  Typing.err_code Typing.InvalidIsAsExpressionHint;
]
let ignored_fixme_codes = ref default_ignored_fixme_codes

let set_allow_errors_in_default_path x = allow_errors_in_default_path := x

let is_ignored_code code = ISet.mem code !ignored_fixme_codes

let is_ignored_fixme code = is_ignored_code code

let (is_hh_fixme: (Pos.t -> error_code -> bool) ref) = ref (fun _ _ -> false)
let (get_hh_fixme_pos: (Pos.t -> error_code -> Pos.t option) ref) =
  ref (fun _ _ -> None)

let add_ignored_fixme_code_error pos code =
  if !is_hh_fixme pos code && is_ignored_code code then
    let pos = Option.value (!get_hh_fixme_pos pos code) ~default:pos in
    M.add_error (M.make_error code
      [pos, Printf.sprintf "HH_FIXME cannot be used for error %d" code])

(*****************************************************************************)
(* Errors accumulator. *)
(*****************************************************************************)

let add_applied_fixme code pos =
  if ServerLoadFlag.get_no_load () then
    let applied_fixmes_list = Common.get_current_list !applied_fixmes in
    Common.set_current_list applied_fixmes ((pos, code) :: applied_fixmes_list)
  else ()

let rec add_error = M.add_error

and add code pos msg =
  if not (is_ignored_fixme code) && !is_hh_fixme pos code
  then add_applied_fixme code pos
  else add_error (M.make_error code [pos, msg]);
  add_ignored_fixme_code_error pos code

and add_list code pos_msg_l =
  let pos = fst (List.hd_exn pos_msg_l) in
  if not (is_ignored_fixme code) && !is_hh_fixme pos code
  then add_applied_fixme code pos
  else add_error (make_error code pos_msg_l);
  add_ignored_fixme_code_error pos code;

and merge (err',fixmes') (err,fixmes) =
  let append = fun _ _ x y ->
    let x = Option.value x ~default: [] in
    let y = Option.value y ~default: [] in
    Some (List.rev_append x y)
  in
  files_t_merge ~f:append err' err,
  files_t_merge ~f:append fixmes' fixmes

and merge_into_current errors =
  let merged = merge errors (!error_map, !applied_fixmes) in
  error_map := fst merged;
  applied_fixmes := snd merged

and incremental_update :
  (* Need to write out the entire ugly type to convince OCaml it's polymorphic
   * and can update both error_map as well as applied_fixmes map *)
  type a .
    a files_t ->
    a files_t ->
    (* function folding over paths of rechecked files *)
    (a files_t -> (Relative_path.t -> a files_t -> a files_t) -> a files_t) ->
    phase ->
    a files_t
  = fun old new_ fold phase ->
  (* Helper to remove acc[path][phase]. If acc[path] becomes empty afterwards,
   * remove it too (i.e do not store empty maps or lists ever). *)
  let remove path phase acc =
    let new_phase_map = match Relative_path.Map.get acc path with
      | None -> None
      | Some phase_map ->
        let new_phase_map = PhaseMap.remove phase_map phase in
        if PhaseMap.is_empty new_phase_map then None else Some new_phase_map
    in
    match new_phase_map with
    | None -> Relative_path.Map.remove acc path
    | Some x -> Relative_path.Map.add acc path x
  in
  (* Replace old errors with new *)
  let res = files_t_merge new_ old ~f:begin fun phase path new_ old ->
    if path = Relative_path.default then begin
      let phase = match phase with
        | Init -> "Init"
        | Parsing -> "Parsing"
        | Naming -> "Naming"
        | Decl -> "Decl"
        | Typing -> "Typing"
      in
      Utils.assert_false_log_backtrace (Some(
        "Default (untracked) error sources should not get into incremental " ^
        "mode. There might be a missing call to Errors.do_with_context/" ^
        "run_in_context somwhere or incorrectly used Errors.from_error_list." ^
        "Phase: " ^ phase
      ))
    end;
    match new_ with
    | Some new_ -> Some (List.rev new_)
    | None -> old
  end in
  (* For files that were rechecked, but had no errors - remove them from maps *)
  fold res begin fun path acc ->
    let has_errors =
      match Relative_path.Map.get new_ path with
      | None -> false
      | Some phase_map -> PhaseMap.mem phase_map phase
    in
    if has_errors then acc
    else remove path phase acc
  end

and incremental_update_set ~old ~new_ ~rechecked phase =
  let fold = fun init g -> Relative_path.Set.fold  ~f:begin fun path acc ->
    g path acc
  end ~init rechecked in
  (incremental_update (fst old) (fst new_) fold phase),
  (incremental_update (snd old) (snd new_) fold phase)

and incremental_update_map ~old ~new_ ~rechecked phase =
  let fold = fun init g -> Relative_path.Map.fold ~f:begin fun path _ acc ->
    g path acc
  end ~init rechecked in
  (incremental_update (fst old) (fst new_) fold phase),
  (incremental_update (snd old) (snd new_) fold phase)

and empty = (Relative_path.Map.empty, Relative_path.Map.empty)
and is_empty (err, _fixmes) = Relative_path.Map.is_empty err

and count (err, _fixmes) = files_t_fold err ~f:(fun _ _ x acc -> acc + List.length x) ~init:0
and get_error_list (err, _fixmes) = files_t_to_list err
and get_applied_fixmes (_err, fixmes) = files_t_to_list fixmes
and from_error_list err = (list_to_files_t err, Relative_path.Map.empty)

(*****************************************************************************)
(* Accessors. (All methods delegated to the parameterized module.) *)
(*****************************************************************************)

and get_code = M.get_code
and get_pos = M.get_pos
and get_severity = M.get_severity
and to_list = M.to_list

and make_error = M.make_error

let sort = M.sort
let get_sorted_error_list = M.get_sorted_error_list
let iter_error_list f err = List.iter ~f:f (get_sorted_error_list err)

let fold_errors ?phase err ~init ~f =
  match phase with
  | None ->
    files_t_fold (fst err)
      ~init
      ~f:begin fun source _ errors acc ->
        List.fold_right errors ~init:acc ~f:(f source)
      end
  | Some phase ->
    Relative_path.Map.fold (fst err) ~init ~f:begin fun source phases acc ->
      match PhaseMap.get phases phase with
      | None -> acc
      | Some errors -> List.fold_right errors ~init:acc ~f:(f source)
    end

let fold_errors_in ?phase err ~source ~init ~f =
  Relative_path.Map.get (fst err) source |>
  Option.value ~default:PhaseMap.empty |>
  PhaseMap.fold ~init ~f:begin fun p errors acc ->
    if phase <> None && phase <> Some p then acc
    else List.fold_right errors ~init:acc ~f
  end

let get_failed_files err phase =
  files_t_fold (fst err)
  ~init:Relative_path.Set.empty
  ~f:begin fun source p _ acc ->
    if phase <> p then acc else Relative_path.Set.add acc source
  end

(*****************************************************************************)
(* Error code printing. *)
(*****************************************************************************)

let error_code_to_string = Common.error_code_to_string

let internal_error pos msg =
  add 0 pos ("Internal error: "^msg)

let unimplemented_feature pos msg =
  add 0 pos ("Feature not implemented: " ^ msg)

let experimental_feature pos msg =
  add 0 pos ("Cannot use experimental feature: " ^ msg)

(*****************************************************************************)
(* Temporary errors. *)
(*****************************************************************************)

let darray_not_supported pos =
  add Temporary.darray_not_supported pos "darray is not supported."

let varray_not_supported pos =
  add Temporary.varray_not_supported pos "varray is not supported."

let varray_or_darray_not_supported pos =
  add
    Temporary.varray_or_darray_not_supported
    pos
    "varray_or_darray is not supported."

let nonnull_not_supported pos =
  add Temporary.nonnull_not_supported pos "nonnull is not supported."

(*****************************************************************************)
(* Parsing errors. *)
(*****************************************************************************)

let fixme_format pos =
  add (Parsing.err_code Parsing.FixmeFormat) pos
    "HH_FIXME wrong format, expected '/* HH_FIXME[ERROR_NUMBER] */'"

let unexpected_eof pos =
  add (Parsing.err_code Parsing.UnexpectedEof) pos "Unexpected end of file"

let unterminated_comment pos =
  add (Parsing.err_code Parsing.UnterminatedComment) pos "unterminated comment"

let unterminated_xhp_comment pos =
  add (Parsing.err_code Parsing.UnterminatedXhpComment) pos "unterminated xhp comment"

let parsing_error (p, msg) =
  add (Parsing.err_code Parsing.ParsingError) p msg

(*****************************************************************************)
(* Naming errors *)
(*****************************************************************************)

let typeparam_alok (pos, x) =
  add (Naming.err_code Naming.TypeparamAlok) pos (
  "You probably forgot to bind this type parameter right?\nAdd <"^x^
  "> somewhere (after the function name definition, \
    or after the class name)\nExamples: "^"function foo<T> or class A<T>")

let generic_class_var pos =
  add (Naming.err_code Naming.GenericClassVar) pos
    "A class variable cannot be generic"

let unexpected_arrow pos cname =
  add (Naming.err_code Naming.UnexpectedArrow) pos (
  "Keys may not be specified for "^cname^" initialization"
 )

let missing_arrow pos cname =
  add (Naming.err_code Naming.MissingArrow) pos (
  "Keys must be specified for "^cname^" initialization"
 )

let disallowed_xhp_type pos name =
  add (Naming.err_code Naming.DisallowedXhpType) pos (
  name^" is not a valid type. Use :xhp or XHPChild."
 )

let name_already_bound name pos1 pos2 =
  let name = Utils.strip_ns name in
  add_list (Naming.err_code Naming.NameAlreadyBound) [
    pos1, "Name already bound: "^name;
    pos2, "Previous definition is here"
]

let name_is_reserved name pos =
  let name = Utils.strip_all_ns name in
  add (Naming.err_code Naming.NameIsReserved) pos (
  name^" cannot be used as it is reserved."
 )

let dollardollar_unused pos =
  add (Naming.err_code Naming.DollardollarUnused) pos ("This expression does not contain a "^
    "usage of the special pipe variable. Did you forget to use the ($$) "^
    "variable?")

let method_name_already_bound pos name =
  add (Naming.err_code Naming.MethodNameAlreadyBound) pos (
  "Method name already bound: "^name
 )

let reference_in_rx pos =
  add (Naming.err_code Naming.ReferenceInRx) pos (
    "References are not allowed in reactive code."
  )
let error_name_already_bound name name_prev p p_prev =
  let name = Utils.strip_ns name in
  let name_prev = Utils.strip_ns name_prev in
  let errs = [
    p, "Name already bound: "^name;
    p_prev, (if String.compare name name_prev == 0
      then "Previous definition is here"
      else "Previous definition "^name_prev^" differs only in capitalization ")
  ] in
  let hhi_msg =
    "This appears to be defined in an hhi file included in your project "^
    "root. The hhi files for the standard library are now a part of the "^
    "typechecker and must be removed from your project. Typically, you can "^
    "do this by deleting the \"hhi\" directory you copied into your "^
    "project when first starting with Hack." in
  let errs =
    if (Relative_path.prefix (Pos.filename p)) = Relative_path.Hhi
    then errs @ [p_prev, hhi_msg]
    else if (Relative_path.prefix (Pos.filename p_prev)) = Relative_path.Hhi
    then errs @ [p, hhi_msg]
    else errs in
  add_list (Naming.err_code Naming.ErrorNameAlreadyBound) errs

let error_class_attribute_already_bound name name_prev p p_prev =
  let name = Utils.strip_ns name in
  let name_prev = Utils.strip_ns name_prev in
  let errs = [
    p, "A class and an attribute class cannot share the same name. Conflicting class: "^name;
    p_prev, "Previous definition: "^name_prev
  ] in
  add_list (Naming.err_code Naming.AttributeClassNameConflict) errs

let unbound_name pos name kind =
  let kind_str = match kind with
    | `cls -> "an object type"
    | `func -> "a global function"
    | `const -> "a global constant"
  in
  add (Naming.err_code Naming.UnboundName) pos
    ("Unbound name: "^(strip_ns name)^" ("^kind_str^")")

let different_scope pos var_name pos' =
  add_list (Naming.err_code Naming.DifferentScope) [
  pos, ("The variable "^ var_name ^" is defined");
  pos', ("But in a different scope")
]

let undefined pos var_name =
  add (Naming.err_code Naming.Undefined) pos ("Undefined variable: "^var_name)

let this_reserved pos =
  add (Naming.err_code Naming.ThisReserved) pos
    "The type parameter \"this\" is reserved"

let start_with_T pos =
  add (Naming.err_code Naming.StartWith_T) pos
    "Please make your type parameter start with the letter T (capital)"

let already_bound pos name =
  add (Naming.err_code Naming.NameAlreadyBound) pos ("Argument already bound: "^name)

let unexpected_typedef pos def_pos =
  add_list (Naming.err_code Naming.UnexpectedTypedef) [
  pos, "Unexpected typedef";
  def_pos, "Definition is here";
]

let fd_name_already_bound pos =
  add (Naming.err_code Naming.FdNameAlreadyBound) pos
    "Field name already bound"

let primitive_toplevel pos =
  add (Naming.err_code Naming.PrimitiveToplevel) pos (
  "Primitive type annotations are always available and may no \
    longer be referred to in the toplevel namespace."
)

let primitive_invalid_alias pos used valid =
  add (Naming.err_code Naming.PrimitiveInvalidAlias) pos
    ("Invalid Hack type. Using '"^used^"' in Hack is considered \
    an error. Use '"^valid^"' instead, to keep the codebase \
    consistent.")

let dynamic_new_in_strict_mode pos =
  add (Naming.err_code Naming.DynamicNewInStrictMode) pos
  "Cannot use dynamic new in strict mode"

let invalid_type_access_root (pos, id) =
  add (Naming.err_code Naming.InvalidTypeAccessRoot) pos
  (id^" must be an identifier for a class, \"self\", or \"this\"")

let duplicate_user_attribute (pos, name) existing_attr_pos =
  add_list (Naming.err_code Naming.DuplicateUserAttribute) [
    pos, "You cannot reuse the attribute "^name;
    existing_attr_pos, name^" was already used here";
  ]

let misplaced_rx_of_scope pos =
  add (Naming.err_code Naming.MisplacedRxOfScope) pos (
    "<<__RxOfScope>> attribute is only allowed on lambdas."
  )
let rx_of_scope_and_explicit_rx pos =
  add (Naming.err_code Naming.RxOfScopeAndExplicitRx) pos (
    "<<__RxOfScope>> attribute cannot be used with explicit reactivity annotations."
  )
let unbound_attribute_name pos name =
  let reason = if (string_starts_with name "__")
    then "starts with __ but is not a standard attribute"
    else "does not have a class and is not listed in .hhconfig"
  in add (Naming.err_code Naming.UnboundName) pos
    ("Unrecognized user attribute: "^name^" "^reason)

let this_no_argument pos =
  add (Naming.err_code Naming.ThisNoArgument) pos "\"this\" expects no arguments"

let void_cast pos =
  add (Naming.err_code Naming.VoidCast) pos "Cannot cast to void."

let unset_cast pos =
  add (Naming.err_code Naming.UnsetCast) pos "Don't use (unset), just assign null!"

let object_cast pos x =
  add (Naming.err_code Naming.ObjectCast) pos ("Object casts are unsupported. "^
    "Try 'if ($var instanceof "^x^")' or "^
    "'invariant($var instanceof "^x^", ...)'.")

let this_hint_outside_class pos =
   add (Naming.err_code Naming.ThisHintOutsideClass) pos
    "Cannot use \"this\" outside of a class"

let this_type_forbidden pos =
 add (Naming.err_code Naming.ThisMustBeReturn) pos
    "The type \"this\" cannot be used as a constraint on a class' generic, \
     or as the type of a static member variable"

let lowercase_this pos type_ =
  add (Naming.err_code Naming.LowercaseThis) pos (
  "Invalid Hack type \""^type_^"\". Use \"this\" instead"
 )

let classname_param pos =
  add (Naming.err_code Naming.ClassnameParam) pos
    ("Missing type parameter to classname; classname is entirely"
     ^" meaningless without one")

let invalid_instanceof pos =
  add (Naming.err_code Naming.InvalidInstanceof) pos
    "This instanceof has an invalid right operand. Only class identifiers, \
    local variables, accesses of objects / classes / arrays, and function / \
    method calls are allowed."

let tparam_with_tparam pos x =
  add (Naming.err_code Naming.TparamWithTparam) pos (
  Printf.sprintf "%s is a type parameter. Type parameters cannot \
    themselves take type parameters (e.g. %s<int> doesn't make sense)" x x
 )

let shadowed_type_param p pos name =
  add_list (Naming.err_code Naming.ShadowedTypeParam) [
    p, Printf.sprintf "You cannot re-bind the type parameter %s" name;
    pos, Printf.sprintf "%s is already bound here" name
  ]

let missing_typehint pos =
  add (Naming.err_code Naming.MissingTypehint) pos
    "Please add a type hint"

let expected_variable pos =
  add (Naming.err_code Naming.ExpectedVariable) pos
    "Was expecting a variable name"

let clone_too_many_arguments pos =
  add (Naming.err_code Naming.NamingTooManyArguments) pos
    "__clone method cannot take arguments"

let naming_too_few_arguments pos =
  add (Naming.err_code Naming.NamingTooFewArguments) pos
    "Too few arguments"

let naming_too_many_arguments pos =
  add (Naming.err_code Naming.NamingTooManyArguments) pos
    "Too many arguments"

let expected_collection pos cn =
  add (Naming.err_code Naming.ExpectedCollection) pos (
  "Unexpected collection type " ^ (Utils.strip_ns cn)
 )

let illegal_CLASS pos =
  add (Naming.err_code Naming.IllegalClass) pos
    "Using __CLASS__ outside a class or trait"

let illegal_TRAIT pos =
  add (Naming.err_code Naming.IllegalTrait) pos
    "Using __TRAIT__ outside a trait"

let dynamic_method_call pos =
  add (Naming.err_code Naming.DynamicMethodCall) pos
    "Dynamic method call"

let nullsafe_property_write_context pos =
  add (Typing.err_code Typing.NullsafePropertyWriteContext) pos
  "?-> syntax not supported here, this function effectively does a write"

let illegal_fun pos =
  let msg = "The argument to fun() must be a single-quoted, constant "^
    "literal string representing a valid function name." in
  add (Naming.err_code Naming.IllegalFun) pos msg

let illegal_member_variable_class pos =
  let msg = "Cannot declare a constant named 'class'. \
             The name 'class' is reserved for the class \
             constant that represents the name of the class" in
  add (Naming.err_code Naming.IllegalMemberVariableClass) pos msg

let illegal_meth_fun pos =
  let msg = "String argument to fun() contains ':';"^
    " for static class methods, use"^
    " class_meth(Cls::class, 'method_name'), not fun('Cls::method_name')" in
  add (Naming.err_code Naming.IllegalMethFun) pos msg

let illegal_inst_meth pos =
  let msg = "The argument to inst_meth() must be an expression and a "^
    "constant literal string representing a valid method name." in
  add (Naming.err_code Naming.IllegalInstMeth) pos msg

let illegal_meth_caller pos =
  let msg =
    "The two arguments to meth_caller() must be:"
    ^"\n - first: ClassOrInterface::class"
    ^"\n - second: a single-quoted string literal containing the name"
    ^" of a non-static method of that class" in
  add (Naming.err_code Naming.IllegalMethCaller) pos msg

let illegal_class_meth pos =
  let msg =
    "The two arguments to class_meth() must be:"
    ^"\n - first: ValidClassname::class"
    ^"\n - second: a single-quoted string literal containing the name"
    ^" of a static method of that class" in
  add (Naming.err_code Naming.IllegalClassMeth) pos msg

let assert_arity pos =
  add (Naming.err_code Naming.AssertArity) pos
    "assert expects exactly one argument"

let gena_arity pos =
  add (Naming.err_code Naming.GenaArity) pos
    "gena() expects exactly 1 argument"

let genva_arity pos =
  add (Naming.err_code Naming.GenvaArity) pos
    "genva() expects at least 1 argument"

let gen_array_rec_arity pos =
  add (Naming.err_code Naming.GenArrayRecArity) pos
    "gen_array_rec() expects exactly 1 argument"

let uninstantiable_class usage_pos decl_pos name reason_msgl =
  let name = strip_ns name in
  let msgl = [
    usage_pos, (name^" is uninstantiable");
    decl_pos, "Declaration is here"
  ] in
  let msgl = match reason_msgl with
    | (reason_pos, reason_str) :: tail ->
      (reason_pos, reason_str^" which must be instantiable") :: tail @ msgl
    | _ -> msgl in
  add_list (Typing.err_code Typing.UninstantiableClass) msgl

let abstract_const_usage usage_pos decl_pos name =
  let name = strip_ns name in
  add_list (Typing.err_code Typing.AbstractConstUsage) [
    usage_pos, ("Cannot reference abstract constant "^name^" directly");
    decl_pos, "Declaration is here"
  ]

let add_a_typehint pos =
  add (Naming.err_code Naming.AddATypehint) pos
    "Please add a type hint"

let local_const var_pos =
  add (Naming.err_code Naming.LocalConst) var_pos
    "You cannot use a local variable in a constant definition"

let illegal_constant pos =
  add (Naming.err_code Naming.IllegalConstant) pos
    "Illegal constant value"

let invalid_req_implements pos =
  add (Naming.err_code Naming.InvalidReqImplements) pos
    "Only traits may use 'require implements'"

let invalid_req_extends pos =
  add (Naming.err_code Naming.InvalidReqExtends) pos
    "Only traits and interfaces may use 'require extends'"

let did_you_mean_naming pos name suggest_pos suggest_name =
  add_list (Naming.err_code Naming.DidYouMeanNaming) [
  pos, "Could not find "^(strip_ns name);
  suggest_pos, "Did you mean "^(strip_ns suggest_name)^"?"
]

let using_internal_class pos name =
  add (Naming.err_code Naming.UsingInternalClass) pos (
  name^" is an implementation internal class that cannot be used directly"
 )

 let too_few_type_arguments p =
   add (Naming.err_code Naming.TooFewTypeArguments) p
     ("Too few type arguments for this type")

let goto_label_already_defined
    label_name
    redeclaration_pos
    original_delcaration_pos =
  add_list
    (Naming.err_code Naming.GotoLabelAlreadyDefined)
    [
      redeclaration_pos, "Cannot redeclare the goto label '" ^ label_name ^ "'";
      original_delcaration_pos, "Declaration is here";
    ]

let goto_label_undefined pos label_name =
  add (Naming.err_code Naming.GotoLabelUndefined) pos ("Undefined goto label: " ^ label_name)

let goto_label_defined_in_finally pos =
  add (Naming.err_code Naming.GotoLabelDefinedInFinally)
    pos
    "It is illegal to define a goto label within a finally block."

let unsupported_feature pos name =
  add (Naming.err_code Naming.UnsupportedFeature) pos (name ^ " is not supported in Hack.")

let goto_invoked_in_finally pos =
  add (Naming.err_code Naming.GotoInvokedInFinally)
    pos
    "It is illegal to invoke goto within a finally block."

let method_needs_visibility pos =
  add (Naming.err_code Naming.MethodNeedsVisibility)
    pos ("Methods need to be marked public, private, or protected.")

let dynamic_class_property_name_in_strict_mode pos =
  add (Naming.err_code Naming.DynamicClassPropertyNameInStrictMode)
    pos
    "Cannot use dynamic class property name in strict mode"

let dynamic_class_name_in_strict_mode pos =
  add (Naming.err_code Naming.DynamicClassNameInStrictMode)
    pos
    "Cannot use dynamic class name in strict mode"

let xhp_optional_required_attr pos id =
  add (Naming.err_code Naming.XhpOptionalRequiredAttr)
    pos
    ("XHP attribute " ^ id ^ " cannot be marked as nullable and required")

let xhp_required_with_default pos id =
  add (Naming.err_code Naming.XhpRequiredWithDefault)
    pos
    ("XHP attribute " ^ id ^ " cannot be marked as required and provide a default")

let variable_variables_disallowed pos =
  add (Naming.err_code Naming.VariableVariablesDisallowed) pos
    "Variable variables are not legal; all variable identifiers must be static strings."

let array_typehints_disallowed pos =
  add (Naming.err_code Naming.ArrayTypehintsDisallowed) pos
    "Array typehints are no longer legal; use varray or darray instead"

let array_literals_disallowed pos =
  add (Naming.err_code Naming.ArrayLiteralsDisallowed) pos
    "Array literals are no longer legal; use varray or darray instead"

let wildcard_disallowed pos =
  add (Naming.err_code Naming.WildcardDisallowed) pos
    "Wildcard typehints are not allowed in this position"

let reference_in_strict_mode pos =
  add (Naming.err_code Naming.ReferenceInStrictMode) pos
    "Don't use references!"

(*****************************************************************************)
(* Init check errors *)
(*****************************************************************************)

let no_construct_parent pos =
  add (NastCheck.err_code NastCheck.NoConstructParent) pos (
  sl["You are extending a class that needs to be initialized\n";
     "Make sure you call parent::__construct.\n"
   ]
 )

let constructor_required (pos, name) prop_names =
  let name = Utils.strip_ns name in
  let props_str = SSet.fold ~f:(fun x acc -> x^" "^acc) prop_names ~init:"" in
  add (NastCheck.err_code NastCheck.ConstructorRequired) pos
    ("Lacking __construct, class "^name^" does not initialize its private member(s): "^props_str)

let not_initialized (pos, cname) prop_names =
  let cname = Utils.strip_ns cname in
  let props_str = SSet.fold ~f:(fun x acc -> x^" "^acc) prop_names ~init:"" in
  let members, verb = if 1 == SSet.cardinal prop_names then "member", "is"
    else "members", "are" in
  let setters_str =
    SSet.fold ~f:(fun x acc -> "$this->"^x^" "^acc) prop_names ~init:"" in
  add (NastCheck.err_code NastCheck.NotInitialized) pos (
    sl[
      "Class "; cname ; " does not initialize all of its members; ";
      props_str; verb; " not always initialized.";
      "\nMake sure you systematically set "; setters_str;
      "when the method __construct is called.";
      "\nAlternatively, you can define the "; members ;" as optional (?...)\n"
    ])

let call_before_init pos cv =
  add (NastCheck.err_code NastCheck.CallBeforeInit) pos (
  sl([
     "Until the initialization of $this is over,";
     " you can only call private methods\n";
     "The initialization is not over because ";
   ] @
     if cv = "parent::__construct"
     then ["you forgot to call parent::__construct"]
     else ["$this->"; cv; " can still potentially be null"])
 )

(*****************************************************************************)
(* Nast errors check *)
(*****************************************************************************)

let type_arity pos name nargs =
  add (Typing.err_code Typing.TypeArityMismatch) pos (
  sl["The type ";(Utils.strip_ns name);
     " expects ";nargs;" type parameter(s)"]
 )

let abstract_with_body (p, _) =
  add (NastCheck.err_code NastCheck.AbstractWithBody) p
    "This method is declared as abstract, but has a body"

let not_abstract_without_body (p, _) =
  add (NastCheck.err_code NastCheck.NotAbstractWithoutBody) p
    "This method is not declared as abstract, it must have a body"

let not_abstract_without_typeconst (p, _) =
  add (NastCheck.err_code NastCheck.NotAbstractWithoutTypeconst) p
    ("This type constant is not declared as abstract, it must have"^
     " an assigned type")

let abstract_with_typeconst (p, _) =
  add (NastCheck.err_code NastCheck.AbstractWithTypeconst) p
    ("This type constant is declared as abstract, it cannot be assigned a type")

let typeconst_depends_on_external_tparam pos ext_pos ext_name =
  add_list (NastCheck.err_code NastCheck.TypeconstDependsOnExternalTparam) [
    pos, ("A type constant can only use type parameters declared in its own"^
      " type parameter list");
    ext_pos, (ext_name ^ " was declared as a type parameter here");
  ]

let typeconst_assigned_tparam pos tp_name =
  add (NastCheck.err_code NastCheck.TypeconstAssignedTparam) pos
    (tp_name ^" is a type parameter. It cannot be assigned to a type constant")

let interface_with_partial_typeconst tconst_pos =
  add (NastCheck.err_code NastCheck.InterfaceWithPartialTypeconst) tconst_pos
    "An interface cannot contain a partially abstract type constant"

let multiple_xhp_category pos =
  add (NastCheck.err_code NastCheck.MultipleXhpCategory) pos
    "XHP classes can only contain one category declaration"

let return_in_gen p =
  add (NastCheck.err_code NastCheck.ReturnInGen) p
    ("You cannot return a value in a generator (a generator"^
     " is a function that uses yield)")

let return_in_finally p =
  add (NastCheck.err_code NastCheck.ReturnInFinally) p
    ("Don't use return in a finally block;"^
     " there's nothing to receive the return value")

let toplevel_break p =
  add (NastCheck.err_code NastCheck.ToplevelBreak) p
    "break can only be used inside loops or switch statements"

let toplevel_continue p =
  add (NastCheck.err_code NastCheck.ToplevelContinue) p
    "continue can only be used inside loops"

let continue_in_switch p =
  add (NastCheck.err_code NastCheck.ContinueInSwitch) p
    ("In PHP, 'continue;' inside a switch \
       statement is equivalent to 'break;'."^
  " Hack does not support this; use 'break' if that is what you meant.")

let await_in_sync_function p =
  add (NastCheck.err_code NastCheck.AwaitInSyncFunction) p
    "await can only be used inside async functions"

let await_not_allowed p =
  add (NastCheck.err_code NastCheck.AwaitNotAllowed) p
    "await is only permitted as a statement, expression in a return statement \
      or as a right hand side in top level assignment."

let async_in_interface p =
  add (NastCheck.err_code NastCheck.AsyncInInterface) p
    "async is only meaningful when it modifies a method body"

let await_in_coroutine p =
  add (NastCheck.err_code NastCheck.AwaitInCoroutine) p
    "await is not allowed in coroutines."

let yield_in_coroutine p =
  add (NastCheck.err_code NastCheck.YieldInCoroutine) p
    "yield is not allowed in coroutines."

let suspend_outside_of_coroutine p =
  add (NastCheck.err_code NastCheck.SuspendOutsideOfCoroutine) p
    "suspend is only allowed in coroutines."

let suspend_in_finally p =
  add (NastCheck.err_code NastCheck.SuspendInFinally) p
    "suspend is not allowed inside finally blocks."

let break_continue_n_not_supported p =
  add (NastCheck.err_code NastCheck.BreakContinueNNotSupported) p
    "Break/continue N operators are not supported."

let static_memoized_function p =
  add (NastCheck.err_code NastCheck.StaticMemoizedFunction) p
    "memoize is not allowed on static methods in classes that aren't final "

let magic (p, s) =
  add (NastCheck.err_code NastCheck.Magic) p
    ("Don't call "^s^" it's one of these magic things we want to avoid")

let non_interface (p : Pos.t) (c2: string) (verb: string): 'a =
  add (NastCheck.err_code NastCheck.NonInterface) p
    ("Cannot " ^ verb ^ " " ^ (strip_ns c2) ^ " - it is not an interface")

let toString_returns_string pos =
  add (NastCheck.err_code NastCheck.ToStringReturnsString) pos "__toString should return a string"

let toString_visibility pos =
  add (NastCheck.err_code NastCheck.ToStringVisibility) pos
    "__toString must have public visibility and cannot be static"

let uses_non_trait (p: Pos.t) (n: string) (t: string) =
  add (NastCheck.err_code NastCheck.UsesNonTrait) p
    ((Utils.strip_ns n) ^ " is not a trait. It is " ^ t ^ ".")

let requires_non_class (p: Pos.t) (n: string) (t: string) =
  add (NastCheck.err_code NastCheck.RequiresNonClass) p
    ((Utils.strip_ns n) ^ " is not a class. It is " ^ t ^ ".")

let abstract_body pos =
  add (NastCheck.err_code NastCheck.AbstractBody) pos "This method shouldn't have a body"

let not_public_or_protected_interface pos =
  add (NastCheck.err_code NastCheck.NotPublicInterface) pos
    "Access type for interface method must be public or protected."

let interface_with_member_variable pos =
  add (NastCheck.err_code NastCheck.InterfaceWithMemberVariable) pos
    "Interfaces cannot have member variables"

let interface_with_static_member_variable pos =
  add (NastCheck.err_code NastCheck.InterfaceWithStaticMemberVariable) pos
    "Interfaces cannot have static variables"

let illegal_function_name pos mname =
  add (NastCheck.err_code NastCheck.IllegalFunctionName) pos
    ("Illegal function name: " ^ strip_ns mname)

let dangerous_method_name pos =
  add (NastCheck.err_code NastCheck.DangerousMethodName) pos (
  "This is a dangerous method name, "^
  "if you want to define a constructor, use "^
  "__construct"
)

let inout_params_outside_of_sync pos =
  add (NastCheck.err_code NastCheck.InoutParamsOutsideOfSync) pos (
    "Inout parameters cannot be defined on async functions, "^
    "generators or coroutines."
  )

let mutable_params_outside_of_sync pos fpos name fname =
  add_list (NastCheck.err_code NastCheck.MutableParamsOutsideOfSync) [
    pos, "Mutable parameters are not allowed on async functions";
    pos, "This parameter "^ (strip_ns name) ^" is marked mutable.";
    fpos, "The function "^ (strip_ns fname) ^" is marked async.";
  ]

let mutable_async_method pos =
  add (NastCheck.err_code NastCheck.MutableAsyncMethod) pos (
    "Mutable methods must be synchronous. Try removing the async tag from the function."
  )

let mutable_attribute_on_function pos =
  add (NastCheck.err_code NastCheck.MutableAttributeOnFunction) pos (
    "<<__Mutable>> only makes sense on methods, or parameters on functions or methods."
  )

let maybe_mutable_attribute_on_function pos =
  add (NastCheck.err_code NastCheck.MaybeMutableAttributeOnFunction) pos (
    "<<__MaybeMutable>> only makes sense on methods, or parameters on functions or methods."
  )

let conflicting_mutable_and_maybe_mutable_attributes pos =
  add  (NastCheck.err_code NastCheck.ConflictingMutableAndMaybeMutableAttributes) pos (
    "Declaration cannot have both <<__Mutable>> and <<__MaybeMutable>> attributtes."
  )

let mutable_methods_must_be_reactive pos name =
  add (NastCheck.err_code NastCheck.MutableMethodsMustBeReactive) pos (
    "The method " ^ (strip_ns name) ^ " has a mutable parameter" ^
    " (or mutable this), so it must be marked reactive with <<__Rx>>."
  )

let mutable_return_annotated_decls_must_be_reactive kind pos name =
  add (NastCheck.err_code NastCheck.MutableReturnAnnotatedDeclsMustBeReactive) pos (
    "The " ^ kind ^ " " ^ (strip_ns name) ^ " is annotated with <<__MutableReturn>>, " ^
    " so it must be marked reactive with <<__Rx>>."
  )

let maybe_mutable_methods_must_be_reactive pos name =
  add (NastCheck.err_code NastCheck.MaybeMutableMethodsMustBeReactive) pos (
    "The method " ^ (strip_ns name) ^ " is annotated with <<__MaybeMutable> attribute, \
    or has this attribute on one of parameters so it must be marked reactive."
  )


let inout_params_special pos =
  add (NastCheck.err_code NastCheck.InoutParamsSpecial) pos
  "Methods with special semantics cannot have inout parameters."

let inout_params_mix_byref pos1 pos2 =
  if pos1 <> pos2 then begin
    let msg1 = pos1, "Cannot mix inout and byRef parameters" in
    let msg2 = pos2, "This parameter is passed by reference" in
    add_list (NastCheck.err_code NastCheck.InoutParamsMixByref) [msg1; msg2]
  end

let inout_params_memoize fpos pos =
  let msg1 = fpos, "Functions with inout parameters cannot be memoized" in
  let msg2 = pos, "This is an inout parameter" in
  add_list (NastCheck.err_code NastCheck.InoutParamsMemoize) [msg1; msg2]

let inout_params_ret_by_ref fpos pos =
  let msg1 = fpos,
    "Functions with inout parameters cannot return by reference (&)" in
  let msg2 = pos, "This is an inout parameter" in
  add_list (NastCheck.err_code NastCheck.InoutParamsRetByRef) [msg1; msg2]

let reading_from_append pos =
  add (NastCheck.err_code NastCheck.ReadingFromAppend) pos "Cannot use [] for reading"

let const_attribute_prohibited pos kind =
  add (NastCheck.err_code NastCheck.ConstAttributeProhibited) pos
    ("Cannot apply __Const attribute to " ^ kind)

let missing_reactivity_for_condition pos =
  add (NastCheck.err_code NastCheck.MissingReactivityForCondition) pos (
    "__OnlyRxIfImpl annotation cannot be used without __Rx, __RxShallow " ^
    "or __RxLocal."
  )

let multiple_reactivity_annotations pos =
  add (NastCheck.err_code NastCheck.MultipleReactivityAnnotations) pos (
    "Multiple <<__Rx>>, <<__RxLocal>> or <<__RxShallow>> annotations are not " ^
    "allowed."
  )

let inout_argument_bad_expr pos =
  add (NastCheck.err_code NastCheck.InoutArgumentBadExpr) pos (
    "Arguments for inout parameters must be local variables or simple " ^
    "subscript expressions on vecs, dicts, keysets, or arrays"
  )

let illegal_destructor pos =
  add (NastCheck.err_code NastCheck.IllegalDestructor) pos (
    "Destructors are not supported in Hack; use other patterns like " ^
    "IDisposable/using or try/catch instead."
  )

let conditionally_reactive_function pos =
  add (NastCheck.err_code NastCheck.ConditionallyReactiveFunction) pos (
    "Function cannot be marked with <<__OnlyRxIfImpl>> annotation."
  )

let multiple_conditionally_reactive_annotations pos name =
  add (NastCheck.err_code NastCheck.MultipleConditionallyReactiveAnnotations) pos (
    "Method '" ^ name ^ "' has multiple <<__OnlyRxIfImpl>> annotations."
  )

let rx_is_enabled_invalid_location pos =
  add (NastCheck.err_code NastCheck.RxIsEnabledInvalidLocation) pos (
    "HH\\Rx\\IS_ENABLED must be the only condition in an if-statement, " ^
    "and that if-statement must be the only statement in the function body."
  )

let onlyrx_if_rxfunc_invalid_location pos =
  add (NastCheck.err_code NastCheck.MaybeRxInvalidLocation) pos (
    "<<__OnlyRxIfRxFunc>> attribute can only be put on parameters of \
    conditionally reactive function or method annotated with \
    <<__OnlyRxIfArgs>> attribute."
  )

let no_onlyrx_if_rxfunc_for_rx_if_args pos =
  add (NastCheck.err_code NastCheck.NoOnlyrxIfRxfuncForRxIfArgs) pos (
    "Function or method annotated with <<__OnlyRxIfArgs>> attribute \
    should have at least one parameter with <<__OnlyRxIfRxFunc>> or \
    <<__OnlyRxIfImpl>> annotations."
  )

let conditionally_reactive_annotation_invalid_arguments ~is_method pos =
  let loc = if is_method then "Method" else "Parameter" in
  add (NastCheck.err_code NastCheck.ConditionallyReactiveAnnotationInvalidArguments) pos (
    loc ^ " is marked with <<__OnlyRxIfImpl>> attribute that have " ^
    "invalid arguments. This attribute must have one argument and it should be " ^
    "'::class' class constant."
  )

let coroutine_in_constructor pos =
  add (NastCheck.err_code NastCheck.CoroutineInConstructor) pos
    "A class constructor may not be a coroutine"

let illegal_return_by_ref pos =
  add (NastCheck.err_code NastCheck.IllegalReturnByRef) pos
    "Returning by reference from a function is no longer supported in Hack."

let illegal_by_ref_expr pos str =
  add (NastCheck.err_code NastCheck.IllegalByRefExpr) pos
  (str ^ " cannot be passed by reference")

let variadic_byref_param pos =
  add (NastCheck.err_code NastCheck.VariadicByRefParam) pos
    "Variadic parameters should not be taken by reference"

(*****************************************************************************)
(* Nast terminality *)
(*****************************************************************************)

let case_fallthrough pos1 pos2 =
  add_list (NastCheck.err_code NastCheck.CaseFallthrough) [
  pos1, ("This switch has a case that implicitly falls through and is "^
      "not annotated with // FALLTHROUGH");
  pos2, "This case implicitly falls through"
]

let default_fallthrough pos =
  add (NastCheck.err_code NastCheck.DefaultFallthrough) pos
    ("This switch has a default case that implicitly falls "^
     "through and is not annotated with // FALLTHROUGH")

(*****************************************************************************)
(* Typing errors *)
(*****************************************************************************)

let visibility_extends vis pos parent_pos parent_vis =
  let msg1 = pos, "This member visibility is: " ^ vis in
  let msg2 = parent_pos, parent_vis ^ " was expected" in
  add_list (Typing.err_code Typing.VisibilityExtends) [msg1; msg2]

let member_not_implemented member_name parent_pos pos defn_pos =
  let msg1 = pos, "This type doesn't implement the method "^member_name in
  let msg2 = parent_pos, "Which is required by this interface" in
  let msg3 = defn_pos, "As defined here" in
  add_list (Typing.err_code Typing.MemberNotImplemented) [msg1; msg2; msg3]

let bad_decl_override parent_pos parent_name pos name (error: error) =
  (* TODO: T24193254 reword this error message, says methods when error can
   * occur on member field *)
  let msg1 = pos, ("Class " ^ (strip_ns name)
      ^ " does not correctly implement all required methods ") in
  let msg2 = parent_pos,
    ("Some methods are incompatible with those declared in type "
     ^ (strip_ns parent_name) ^
     "\nRead the following to see why:"
    ) in
  (* This is a cascading error message *)
  let code, msgl = (get_code error), (to_list error) in
  add_list code (msg1 :: msg2 :: msgl)

let bad_method_override pos member_name (error: error) =
  let msg = pos, ("Member " ^ (strip_ns member_name)
      ^ " has the wrong type") in
  (* This is a cascading error message *)
  let code, msgl = (get_code error), (to_list error) in
  add_list code (msg :: msgl)

let bad_enum_decl pos (error: error) =
  let msg = pos,
    "This enum declaration is invalid.\n\
    Read the following to see why:"
  in
  (* This is a cascading error message *)
  let code, msgl = (get_code error), (to_list error) in
  add_list code (msg :: msgl)

let missing_constructor pos =
  add (Typing.err_code Typing.MissingConstructor) pos
    "The constructor is not implemented"

let typedef_trail_entry pos =
  pos, "Typedef definition comes from here"

let add_with_trail code errs trail =
  add_list code (errs @ List.map trail typedef_trail_entry)

let enum_constant_type_bad pos ty_pos ty trail =
  add_with_trail (Typing.err_code Typing.EnumConstantTypeBad)
    [pos, "Enum constants must be an int or string";
     ty_pos, "Not " ^ ty]
    trail

let enum_type_bad pos ty trail =
  add_with_trail (Typing.err_code Typing.EnumTypeBad)
    [pos, "Enums must be int or string, not " ^ ty]
    trail

let enum_type_typedef_mixed pos =
  add (Typing.err_code Typing.EnumTypeTypedefMixed) pos
    "Can't use typedef that resolves to mixed in enum"

let enum_type_typedef_nonnull pos =
  add (Typing.err_code Typing.EnumTypeTypedefNonnull) pos
    "Can't use typedef that resolves to nonnull in enum"

let enum_switch_redundant const first_pos second_pos =
  add_list (Typing.err_code Typing.EnumSwitchRedundant) [
    second_pos, "Redundant case statement";
    first_pos, const ^ " already handled here"
  ]

let enum_switch_nonexhaustive pos missing enum_pos =
  add_list (Typing.err_code Typing.EnumSwitchNonexhaustive) [
    pos, "Switch statement nonexhaustive; the following cases are missing: " ^
            String.concat ", " missing;
    enum_pos, "Enum declared here"
  ]

let enum_switch_redundant_default pos enum_pos =
  add_list (Typing.err_code Typing.EnumSwitchRedundantDefault) [
    pos, "All cases already covered; a redundant default case prevents "^
         "detecting future errors";
    enum_pos, "Enum declared here"
  ]

let enum_switch_not_const pos =
  add (Typing.err_code Typing.EnumSwitchNotConst) pos
    "Case in switch on enum is not an enum constant"

let enum_switch_wrong_class pos expected got =
  add (Typing.err_code Typing.EnumSwitchWrongClass) pos
    ("Switching on enum " ^ expected ^ " but using constant from " ^ got)

let invalid_shape_field_name p =
  add (Typing.err_code Typing.InvalidShapeFieldName) p
    "Was expecting a constant string, class constant, or int (for shape access)"

let invalid_shape_field_name_empty p =
  add (Typing.err_code Typing.InvalidShapeFieldNameEmpty) p
    "A shape field name cannot be an empty string"

let invalid_shape_field_name_number p =
  add (Typing.err_code Typing.InvalidShapeFieldNameNumber) p
    "A shape field name cannot start with numbers"

let invalid_shape_field_type pos ty_pos ty trail =
  add_with_trail (Typing.err_code Typing.InvalidShapeFieldType)
    [pos, "A shape field name must be an int or string";
     ty_pos, "Not " ^ ty]
    trail

let invalid_shape_field_literal key_pos witness_pos =
  add_list (Typing.err_code Typing.InvalidShapeFieldLiteral)
    [key_pos, "Shape uses literal string as field name";
     witness_pos, "But expected a class constant"]

let invalid_shape_field_const key_pos witness_pos =
  add_list (Typing.err_code Typing.InvalidShapeFieldConst)
    [key_pos, "Shape uses class constant as field name";
     witness_pos, "But expected a literal string"]

let shape_field_class_mismatch key_pos witness_pos key_class witness_class =
  add_list (Typing.err_code Typing.ShapeFieldClassMismatch)
    [key_pos, "Shape field name is class constant from " ^ key_class;
     witness_pos, "But expected constant from " ^ witness_class]

let shape_field_type_mismatch key_pos witness_pos key_ty witness_ty =
  add_list (Typing.err_code Typing.ShapeFieldTypeMismatch)
    [key_pos, "Shape field name is " ^ key_ty ^ " class constant";
     witness_pos, "But expected " ^ witness_ty]

let missing_field pos1 pos2 name =
  add_list (Typing.err_code Typing.MissingField) (
    (pos1, "The field '"^name^"' is missing")::
    [pos2, "The field '"^name^"' is defined"])

let unknown_field_disallowed_in_shape pos1 pos2 name =
  add_list
    (Typing.err_code Typing.UnknownFieldDisallowedInShape)
    [
      pos1,
      "The field '" ^ name ^ "' is not defined in this shape type, and \
      this shape type does not allow unknown fields.";
      pos2,
      "The field '" ^ name ^ "' is set in the shape.";
    ]

let shape_fields_unknown pos1 pos2 =
  add_list (Typing.err_code Typing.ShapeFieldsUnknown)
    [
      pos1,
      "This shape type allows unknown fields, and so it may contain fields \
      other than those explicitly declared in its declaration.";
      pos2,
      "It is incompatible with a shape that does not allow unknown fields.";
    ]

let shape_field_unset pos1 pos2 name =
  add_list (Typing.err_code Typing.ShapeFieldUnset) (
    [(pos1, "The field '"^name^"' was unset here");
     (pos2, "The field '"^name^"' might be present in this shape because of " ^
            "structural subtyping")]
  )

let invalid_shape_remove_key p =
  add (Typing.err_code Typing.InvalidShapeRemoveKey) p
    "You can only unset fields of local variables"

let unification_cycle pos ty =
  add_list (Typing.err_code Typing.UnificationCycle)
    [pos, "Type circularity: in order to type-check this expression it " ^
       "is necessary for a type [rec] to be equal to type " ^ ty]


let explain_constraint ~use_pos ~definition_pos ~param_name (error : error) =
  let inst_msg = "Some type constraint(s) here are violated" in
  let code, msgl = (get_code error), (to_list error) in
  (* There may be multiple constraints instantiated at one spot; avoid
   * duplicating the instantiation message *)
  let msgl = match msgl with
    | (p, x) :: rest when x = inst_msg && p = use_pos -> rest
    | _ -> msgl in
  let name = Utils.strip_ns param_name in
  add_list code begin
    [use_pos, inst_msg;
     definition_pos, "'" ^ name ^ "' is a constrained type parameter"] @ msgl
  end

let explain_where_constraint ~use_pos ~definition_pos (error : error) =
  let inst_msg = "A 'where' type constraint is violated here" in
  let code, msgl = (get_code error), (to_list error) in
  add_list code begin
    [use_pos, inst_msg;
     definition_pos, "This is the method with 'where' type constraints"] @ msgl
  end

let explain_tconst_where_constraint ~use_pos ~definition_pos (error: error) =
  let inst_msg = "A 'where' type constraint is violated here" in
  let code, msgl = (get_code error), (to_list error) in
  add_list code begin
    [use_pos, inst_msg;
     definition_pos,
     "This method's where constraints contain a generic type access"] @ msgl
  end

let explain_type_constant reason_msgl (error: error) =
  let code, msgl = (get_code error), (to_list error) in
  add_list code (msgl @ reason_msgl)

let overflow p =
  add (Typing.err_code Typing.Overflow) p "Value is too large"

let format_string pos snippet s class_pos fname class_suggest =
  add_list (Typing.err_code Typing.FormatString) [
  (pos, "I don't understand the format string " ^ snippet ^ " in " ^ s);
  (class_pos,
   "You can add a new format specifier by adding "
   ^fname^"() to "^class_suggest)]

let expected_literal_string pos =
  add (Typing.err_code Typing.ExpectedLiteralString) pos
    "This argument must be a literal string"

let re_prefixed_non_string pos non_strings =
  add (Typing.err_code Typing.RePrefixedNonString) pos
    (non_strings^" are not allowed to be to be `re`-prefixed")

let bad_regex_pattern pos s =
  add (Typing.err_code Typing.BadRegexPattern) pos
    ("Bad regex pattern; "^s^".")

let generic_array_strict p =
  add (Typing.err_code Typing.GenericArrayStrict) p
    "You cannot have an array without generics in strict mode"

let strict_members_not_known p name =
  let name = Utils.strip_ns name in
  add (Typing.err_code Typing.StrictMembersNotKnown) p
    (name^" has a non-<?hh grandparent; this is not allowed in strict mode"
     ^" because that parent may define methods of unknowable name and type")

let option_return_only_typehint p kind =
  let (typehint, reason) = match kind with
    | `void -> ("?void", "only return implicitly")
    | `noreturn -> ("?noreturn", "never return")
  in
  add (Typing.err_code Typing.OptionReturnOnlyTypehint) p
    (typehint^" is a nonsensical typehint; a function cannot both "^reason
     ^" and return null.")

let tuple_syntax p =
  add (Typing.err_code Typing.TupleSyntax) p
    ("Did you want a tuple? Try (X,Y), not tuple<X,Y>")

let class_arity usage_pos class_pos class_name arity =
  add_list (Typing.err_code Typing.ClassArity)
    [usage_pos, ("The class "^(Utils.strip_ns class_name)^" expects "^
                    soi arity^" arguments");
     class_pos, "Definition is here"]

let expecting_type_hint p =
  add (Typing.err_code Typing.ExpectingTypeHint) p "Was expecting a type hint"

let expecting_type_hint_suggest p ty =
  add (Typing.err_code Typing.ExpectingTypeHintSuggest) p
    ("Was expecting a type hint (what about: "^ty^")")

let expecting_return_type_hint p =
  add (Typing.err_code Typing.ExpectingReturnTypeHint) p
    "Was expecting a return type hint"

let expecting_return_type_hint_suggest p ty =
  add (Typing.err_code Typing.ExpectingReturnTypeHintSuggest) p
    ("Was expecting a return type hint (what about: ': "^ty^"')")

let expecting_awaitable_return_type_hint p =
  add (Typing.err_code Typing.ExpectingAwaitableReturnTypeHint) p
    "Was expecting an Awaitable return type hint"

let duplicate_using_var pos =
  add (Typing.err_code Typing.DuplicateUsingVar) pos
    "Local variable already used in 'using' statement"

let illegal_disposable pos verb =
  add (Typing.err_code Typing.IllegalDisposable) pos
    ("Disposable objects must only be " ^ verb ^ " in a 'using' statement")

let escaping_disposable pos =
  add (Typing.err_code Typing.EscapingDisposable) pos
    "Variable from 'using' clause may only be used as receiver in method invocation or \
    passed to function with <<__AcceptDisposable>> parameter attribute"

let escaping_disposable_parameter pos =
  add (Typing.err_code Typing.EscapingDisposableParameter) pos
    "Parameter with <<__AcceptDisposable>> attribute may only be used as receiver in method \
    invocation or passed to another function with <<__AcceptDisposable>> parameter attribute"

let escaping_this pos =
  add (Typing.err_code Typing.EscapingThis) pos
    "$this implementing IDisposable or IAsyncDisposable may only be used as receiver in method \
    invocation or passed to another function with <<__AcceptDisposable>> parameter attribute"

let must_extend_disposable pos =
  add (Typing.err_code Typing.MustExtendDisposable) pos
    "A disposable type may not extend a class or use a trait that is not disposable"

let accept_disposable_invariant pos1 pos2 =
  let msg1 = pos1, "This parameter is marked <<__AcceptDisposable>>" in
  let msg2 = pos2, "This parameter is not marked <<__AcceptDisposable>>" in
  add_list (Typing.err_code Typing.AcceptDisposableInvariant) [msg1; msg2]

let field_kinds pos1 pos2 =
  add_list (Typing.err_code Typing.FieldKinds)
    [pos1, "You cannot use this kind of field (value)";
     pos2, "Mixed with this kind of field (key => value)"]

let unbound_name_typing pos name =
  add (Typing.err_code Typing.UnboundNameTyping) pos
    ("Unbound name (typing): "^(strip_ns name))

let previous_default p =
  add (Typing.err_code Typing.PreviousDefault) p
    ("A previous parameter has a default value.\n"^
     "Remove all the default values for the preceding parameters,\n"^
     "or add a default value to this one.")

let return_only_typehint p kind =
  let msg = match kind with
    | `void -> "void"
    | `noreturn -> "noreturn" in
  add (Naming.err_code Naming.ReturnOnlyTypehint) p
    ("The "^msg^" typehint can only be used to describe a function return type")

let unexpected_type_arguments p =
  add (Naming.err_code Naming.UnexpectedTypeArguments) p
    ("Type arguments are not expected for this type")

let too_many_type_arguments p =
  add (Naming.err_code Naming.TooManyTypeArguments) p
    ("Too many type arguments for this type")

let declare_statement_in_hack p =
  add (Naming.err_code Naming.DeclareStatement) p
    ("Declare statements are disallowed in Hack code.")
let return_in_void pos1 pos2 =
  add_list (Typing.err_code Typing.ReturnInVoid) [
  pos1,
  "You cannot return a value";
  pos2,
  "This is a void function"
]

let this_in_static p =
  add (Typing.err_code Typing.ThisInStatic) p "Don't use $this in a static method"

let this_var_outside_class p =
  add (Typing.err_code Typing.ThisVarOutsideClass) p "Can't use $this outside of a class"

let unbound_global cst_pos =
  add (Typing.err_code Typing.UnboundGlobal) cst_pos "Unbound global constant (Typing)"

let private_inst_meth method_pos p =
  add_list (Typing.err_code Typing.PrivateInstMeth) [
  method_pos, "This is a private method";
  p, "you cannot use it with inst_meth \
    (whether you are in the same class or not)."
]

let protected_inst_meth method_pos p =
  add_list (Typing.err_code Typing.ProtectedInstMeth) [
  method_pos, "This is a protected method";
  p, "you cannot use it with inst_meth \
    (whether you are in the same class hierarchy or not)."
]

let private_class_meth pos1 pos2 =
  add_list (Typing.err_code Typing.PrivateClassMeth) [
  pos1, "This is a private method";
  pos2, "you cannot use it with class_meth \
    (whether you are in the same class or not)."
]

let protected_class_meth pos1 pos2 =
  add_list (Typing.err_code Typing.ProtectedClassMeth) [
  pos1, "This is a protected method";
  pos2, "you cannot use it with class_meth \
    (whether you are in the same class hierarchy or not)."
]

let array_cast pos =
  add (Typing.err_code Typing.ArrayCast) pos
    "(array) cast forbidden; arrays with unspecified \
    key and value types are not allowed"

let nullable_cast pos ty ty_pos =
  add_list (Typing.err_code Typing.NullableCast) [
    pos, "Casting from a nullable type is forbidden";
    ty_pos, "This is "^ty;
  ]

let anonymous_recursive pos =
  add (Typing.err_code Typing.AnonymousRecursive) pos
    "Anonymous functions cannot be recursive"

let static_outside_class pos =
  add (Typing.err_code Typing.StaticOutsideClass) pos
    "'static' is undefined outside of a class"

let self_outside_class pos =
  add (Typing.err_code Typing.SelfOutsideClass) pos
    "'self' is undefined outside of a class"

let new_inconsistent_construct new_pos (cpos, cname) kind =
  let name = Utils.strip_ns cname in
  let preamble = match kind with
    | `static -> "Can't use new static() for "^name
    | `classname -> "Can't use new on classname<"^name^">"
  in
  add_list (Typing.err_code Typing.NewStaticInconsistent) [
    new_pos, preamble^"; __construct arguments are not \
    guaranteed to be consistent in child classes";
    cpos, ("This declaration neither defines an abstract/final __construct"
           ^" nor uses <<__ConsistentConstruct>> attribute")]

let pair_arity pos =
  add (Typing.err_code Typing.PairArity) pos "A pair has exactly 2 elements"

let tuple_arity pos2 size2 pos1 size1 =
  add_list (Typing.err_code Typing.TupleArity) [
  pos2, "This tuple has "^ string_of_int size2^" elements";
  pos1, string_of_int size1 ^ " were expected"]

let undefined_parent pos =
  add (Typing.err_code Typing.UndefinedParent) pos
    "The parent class is undefined"

let parent_outside_class pos =
  add (Typing.err_code Typing.ParentOutsideClass) pos
    "'parent' is undefined outside of a class"

let parent_abstract_call meth_name call_pos decl_pos =
  add_list (Typing.err_code Typing.AbstractCall) [
    call_pos, ("Cannot call parent::"^meth_name^"(); it is abstract");
    decl_pos, "Declaration is here"
  ]

let self_abstract_call meth_name call_pos decl_pos =
  add_list (Typing.err_code Typing.AbstractCall) [
    call_pos, ("Cannot call self::"^meth_name^"(); it is abstract. Did you mean static::"^meth_name^"()?");
    decl_pos, "Declaration is here"
  ]

let classname_abstract_call cname meth_name call_pos decl_pos =
  let cname = Utils.strip_ns cname in
  add_list (Typing.err_code Typing.AbstractCall) [
    call_pos, ("Cannot call "^cname^"::"^meth_name^"(); it is abstract");
    decl_pos, "Declaration is here"
  ]

let static_synthetic_method cname meth_name call_pos decl_pos =
  let cname = Utils.strip_ns cname in
  add_list (Typing.err_code Typing.StaticSyntheticMethod) [
    call_pos, ("Cannot call "^cname^"::"^meth_name^"(); "^meth_name^" is not defined in "^cname);
    decl_pos, "Declaration is here"
  ]

let empty_in_strict pos =
  add (Typing.err_code Typing.IssetEmptyInStrict) pos
    ("empty cannot be used in a completely type safe way and so is banned in "
     ^"strict mode")

let isset_in_strict pos =
  add (Typing.err_code Typing.IssetEmptyInStrict) pos
    ("isset cannot be used in a completely type safe way and so is banned in "
     ^"strict mode; try using array_key_exists instead")

let unset_nonidx_in_strict pos msgs =
  add_list (Typing.err_code Typing.UnsetNonidxInStrict)
    ([pos, "In strict mode, unset is banned except on array, keyset, "^
           "or dict indexing"] @
     msgs)

let unpacking_disallowed_builtin_function pos name =
  let name = Utils.strip_ns name in
  add (Typing.err_code Typing.UnpackingDisallowed) pos
    ("Arg unpacking is disallowed for "^name)

let array_get_arity pos1 name pos2 =
  add_list (Typing.err_code Typing.ArrayGetArity) [
  pos1, "You cannot use this "^(Utils.strip_ns name);
  pos2, "It is missing its type parameters"
]

let typing_error pos msg =
  add (Typing.err_code Typing.GenericUnify) pos msg

let typing_error_l err =
  add_error err

let undefined_field ~use_pos ~name ~shape_type_pos =
  add_list (Typing.err_code Typing.UndefinedField) [
    use_pos, "The field "^name^" is undefined";
    shape_type_pos, "You might want to check this out"
  ]

let array_access pos1 pos2 ty =
  add_list (Typing.err_code Typing.ArrayAccess)
    ((pos1, "This is not an object of type KeyedContainer, this is "^ty) ::
     if pos2 != Pos.none
     then [pos2, "You might want to check this out"]
     else [])

let keyset_set pos1 pos2 =
  add_list (Typing.err_code Typing.KeysetSet)
    ((pos1, "Keysets entries cannot be set, use append instead.") ::
     if pos2 != Pos.none
     then [pos2, "You might want to check this out"]
     else [])

let array_append pos1 pos2 ty =
  add_list (Typing.err_code Typing.ArrayAppend)
    ((pos1, ty^" does not allow array append") ::
     if pos2 != Pos.none
     then [pos2, "You might want to check this out"]
     else [])

let const_mutation pos1 pos2 ty =
  add_list (Typing.err_code Typing.ConstMutation)
    ((pos1, "You cannot mutate this") ::
     if pos2 != Pos.none
     then [(pos2, "This is " ^ ty)]
     else [])

let expected_class ?(suffix="") pos =
  add (Typing.err_code Typing.ExpectedClass) pos ("Was expecting a class"^suffix)

let snot_found_hint = function
  | `no_hint ->
      []
  | `closest (pos, v) ->
      [pos, "The closest thing is "^v^" but it's not a static method"]
  | `did_you_mean (pos, v) ->
      [pos, "Did you mean: "^v]

let string_of_class_member_kind = function
  | `class_constant -> "class constant"
  | `static_method  -> "static method"
  | `class_variable -> "class variable"
  | `class_typeconst -> "type constant"

let smember_not_found kind pos (cpos, class_name) member_name hint =
  let kind = string_of_class_member_kind kind in
  let class_name = strip_ns class_name in
  let msg = "Could not find "^kind^" "^member_name^" in type "^class_name in
  add_list (Typing.err_code Typing.SmemberNotFound)
    ((pos, msg) :: (snot_found_hint hint
                    @ [(cpos, "Declaration of "^class_name^" is here")]))

let not_found_hint = function
  | `no_hint ->
      []
  | `closest (pos, v) ->
      [pos, "The closest thing is "^v^" but it's a static method"]
  | `did_you_mean (pos, v) ->
      [pos, "Did you mean: "^v]

let member_not_found kind pos (cpos, type_name) member_name hint reason =
  let type_name = strip_ns type_name in
  let kind =
    match kind with
    | `method_ -> "method"
    | `member -> "member"
  in
  let msg = "Could not find "^kind^" "^member_name^" in an object of type "^
    type_name in
  add_list (Typing.err_code Typing.MemberNotFound)
    ((pos, msg) :: (not_found_hint hint @ reason
                    @ [(cpos, "Declaration of "^type_name^" is here")]))

let parent_in_trait pos =
  add (Typing.err_code Typing.ParentInTrait) pos
    ("parent:: inside a trait is undefined"
     ^" without 'require extends' of a class defined in <?hh")

let parent_undefined pos =
  add (Typing.err_code Typing.ParentUndefined) pos
    "parent is undefined"

let constructor_no_args pos =
  add (Typing.err_code Typing.ConstructorNoArgs) pos
    "This constructor expects no argument"

let visibility p msg1 p_vis msg2 =
  add_list (Typing.err_code Typing.Visibility) [p, msg1; p_vis, msg2]

let typing_too_many_args pos pos_def =
  add_list (Typing.err_code Typing.TypingTooManyArgs)
    [pos, "Too many arguments"; pos_def, "Definition is here"]

let typing_too_few_args pos pos_def =
  add_list (Typing.err_code Typing.TypingTooFewArgs)
    [pos, "Too few arguments"; pos_def, "Definition is here"]

let anonymous_recursive_call pos =
  add (Typing.err_code Typing.AnonymousRecursiveCall) pos
    "recursive call to anonymous function"

let bad_call pos ty =
  add (Typing.err_code Typing.BadCall) pos
    ("This call is invalid, this is not a function, it is "^ty)

let sketchy_null_check pos name kind =
  let name = Option.value name ~default:"$x" in
  add (Typing.err_code Typing.SketchyNullCheck) pos @@
    "You are using a sketchy null check ...\n"^
    match kind with
    | `Coalesce ->
      Printf.sprintf "Use %s ?? $default instead of %s ?: $default" name name
    | `Eq ->
      Printf.sprintf "Use %s === null instead" name
    | `Neq ->
      Printf.sprintf "Use %s !== null instead" name

let sketchy_null_check_primitive pos name kind =
  let name = Option.value name ~default:"$x" in
  add (Typing.err_code Typing.SketchyNullCheckPrimitive) pos @@
    "You are using a sketchy null check on a primitive type ...\n"^
    match kind with
    | `Coalesce ->
      Printf.sprintf "Use %s ?? $default instead of %s ?: $default" name name
    | `Eq ->
      Printf.sprintf "Use %s === null instead" name
    | `Neq ->
      Printf.sprintf "Use %s !== null instead" name

let extend_final extend_pos decl_pos name =
  let name = (strip_ns name) in
  add_list (Typing.err_code Typing.ExtendFinal) [
    extend_pos, ("You cannot extend final class "^name);
    decl_pos, "Declaration is here"
  ]

let extend_sealed child_pos parent_pos parent_name parent_kind verb =
  let name = (strip_ns parent_name) in
  add_list (Typing.err_code Typing.ExtendSealed) [
    child_pos, ("You cannot "^verb^" sealed "^parent_kind^" "^name);
    parent_pos, "Declaration is here"
  ]

let trait_implement_sealed child_pos parent_pos parent_name =
  let name = (strip_ns parent_name) in
  add_list (Typing.err_code Typing.ExtendSealed) [
    child_pos, (
      "A trait cannot implement sealed interface "^name^
      ". Use `require implements` instead");
    parent_pos, "Declaration is here"
  ]

let extend_ppl
  child_pos child_class_type child_is_ppl parent_pos parent_class_type parent_name verb =
  let name = (strip_ns parent_name) in
  let warning =
    if child_is_ppl
    then child_class_type^" annotated with <<__PPL>> cannot "^verb^
      " non <<__PPL>> "^parent_class_type^": "^name
    else child_class_type^" must be annotated with <<__PPL>> to "^verb^
      " <<__PPL>> "^parent_class_type^": "^name in
  add_list (Typing.err_code Typing.ExtendPPL) [
    child_pos, warning;
    parent_pos, "Declaration is here";
  ]

let sealed_final pos name =
  let name = (strip_ns name) in
  add (Typing.err_code Typing.SealedFinal) pos ("Sealed class "^name^" cannot be marked final")

let unsealable pos kind =
  add (Typing.err_code Typing.Unsealable) pos (kind^" cannot be sealed")

let read_before_write (pos, v) =
  add (Typing.err_code Typing.ReadBeforeWrite) pos (
  sl[
  "Read access to $this->"; v; " before initialization"
])

let interface_final pos =
  add (Typing.err_code Typing.InterfaceFinal) pos
    "Interfaces cannot be final"

let trait_final pos =
  add (Typing.err_code Typing.TraitFinal) pos
    "Traits cannot be final"

let final_property pos =
  add (Typing.err_code Typing.FinalProperty) pos "Properties cannot be declared final"

let implement_abstract ~is_final pos1 pos2 kind x =
  let name = "abstract "^kind^" '"^x^"'" in
  let msg1 =
    if is_final then
      "This class was declared as final. It must provide an implementation \
       for the "^name
    else
      "This class must be declared abstract, or provide an implementation \
       for the "^name in
  add_list (Typing.err_code Typing.ImplementAbstract) [
    pos1, msg1;
    pos2, "Declaration is here";
  ]

let generic_static pos x =
  add (Typing.err_code Typing.GenericStatic) pos (
  "This static variable cannot use the type parameter "^x^"."
 )

let fun_too_many_args pos1 pos2 =
  add_list (Typing.err_code Typing.FunTooManyArgs) [
  pos1, "Too many mandatory arguments";
  pos2, "Because of this definition";
]

let fun_too_few_args pos1 pos2 =
  add_list (Typing.err_code Typing.FunTooFewArgs) [
  pos1, "Too few arguments";
  pos2, "Because of this definition";
]

let fun_unexpected_nonvariadic pos1 pos2 =
  add_list (Typing.err_code Typing.FunUnexpectedNonvariadic) [
  pos1, "Should have a variadic argument";
  pos2, "Because of this definition";
]

let fun_variadicity_hh_vs_php56 pos1 pos2 =
  add_list (Typing.err_code Typing.FunVariadicityHhVsPhp56) [
  pos1, "Variadic arguments: ...-style is not a subtype of ...$args";
  pos2, "Because of this definition";
]

let ellipsis_strict_mode ~require pos =
  let msg = match require with
  | `Type -> "Cannot use ... without a type hint in strict mode. Please add a type hint."
  | `Param_name ->
    "Cannot use ... without a parameter name in strict mode. Please add a parameter name."
  | `Type_and_param_name ->
    "Cannot use ... without a type hint and parameter name in strict mode. \
    Please add a type hint and parameter name."
  in
  add (Typing.err_code Typing.EllipsisStrictMode) pos msg

let untyped_lambda_strict_mode pos =
  let msg =
    "Cannot determine types of lambda parameters in strict mode. \
     Please add type hints on parameters."
  in
    add (Typing.err_code Typing.UntypedLambdaStrictMode) pos msg

let echo_in_reactive_context pos =
  add (Typing.err_code Typing.EchoInReactiveContext) pos (
    "'echo' or 'print' are not allowed in reactive or shallow-reactive functions."
  )

let expected_tparam pos n =
  add (Typing.err_code Typing.ExpectedTparam) pos (
  "Expected " ^
  (match n with
  | 0 -> "no type parameter"
  | 1 -> "a type parameter"
  | n -> string_of_int n ^ " type parameters"
  )
 )

let object_string pos1 pos2 =
  add_list (Typing.err_code Typing.ObjectString) [
  pos1, "You cannot use this object as a string";
  pos2, "This object doesn't implement __toString";
]

let type_param_arity pos x n =
  add (Typing.err_code Typing.TypeParamArity) pos (
  "The type "^x^" expects "^n^" parameters"
 )

let cyclic_typedef p =
  add (Typing.err_code Typing.CyclicTypedef) p
    "Cyclic typedef"

let type_arity_mismatch pos1 n1 pos2 n2 =
  add_list (Typing.err_code Typing.TypeArityMismatch) [
  pos1, "This type has "^n1^" arguments";
  pos2, "This one has "^n2;
]

let this_final id pos2 (error: error) =
  let n = Utils.strip_ns (snd id) in
  let message1 = "Since "^n^" is not final" in
  let message2 = "this might not be a "^n in
  let code, msgl = (get_code error), (to_list error) in
  add_list code (msgl @ [(fst id, message1); (pos2, message2)])

let exact_class_final id pos2 (error: error) =
  let n = Utils.strip_ns (snd id) in
  let message1 = "This requires the late-bound type to be exactly "^n in
  let message2 =
    "Since " ^n^" is not final this might be an instance of a child class" in
  let code, msgl = (get_code error), (to_list error) in
  add_list code (msgl @ [(fst id, message1); (pos2, message2)])

let tuple_arity_mismatch pos1 n1 pos2 n2 =
  add_list (Typing.err_code Typing.TupleArityMismatch) [
  pos1, "This tuple has "^n1^" elements";
  pos2, "This one has "^n2^" elements"
]

let fun_arity_mismatch pos1 pos2 =
  add_list (Typing.err_code Typing.FunArityMismatch) [
  pos1, "Number of arguments doesn't match";
  pos2, "Because of this definition";
]

let fun_reactivity_mismatch pos1 kind1 pos2 kind2 =
  let f k = "This function is " ^ k ^ "." in
  add_list
    (Typing.err_code Typing.FunReactivityMismatch)
    [
      pos1, f kind1;
      pos2, f kind2
    ]

let frozen_in_incorrect_scope pos1 =
  add (Typing.err_code Typing.FrozenInIncorrectScope) pos1
  ("This variable is frozen in one scope but not the other")


let reassign_mutable_var pos1 =
  add (Typing.err_code Typing.ReassignMutableVar) pos1
  ("This variable is mutable. You cannot create a new reference to it.")

let reassign_mutable_this pos1 =
  add (Typing.err_code Typing.ReassignMutableThis) pos1
  ("$this here is mutable. You cannot create a new reference to it.")

let mutable_expression_as_multiple_mutable_arguments pos param_kind prev_pos prev_param_kind =
  add_list (Typing.err_code Typing.MutableExpressionAsMultipleMutableArguments) [
    pos, "A mutable expression may not be passed as multiple arguments where \
    at least one matching parameter is mutable. Matching parameter here is " ^ param_kind;
    prev_pos, "This is where it was used before, being passed as " ^ prev_param_kind
  ]

let reassign_maybe_mutable_var pos1 =
  add (Typing.err_code Typing.ReassignMaybeMutableVar) pos1
  ("This variable is maybe mutable. You cannot create a new reference to it.")


let mutable_call_on_immutable fpos pos1 =
  add_list (Typing.err_code Typing.MutableCallOnImmutable)
  [
    pos1, "Cannot call mutable function on immutable expression";
    fpos, "This function is marked <<__Mutable>>, so it has a mutable $this.";
  ]

let immutable_call_on_mutable fpos pos1 =
  add_list (Typing.err_code Typing.ImmutableCallOnMutable)
  [
    pos1, "Cannot call non-mutable function on mutable expression";
    fpos, "This function is not marked as <<__Mutable>>.";
  ]

let mutability_mismatch ~is_receiver pos1 mut1 pos2 mut2 =
  let msg mut =
    let msg = if is_receiver then "Receiver of this function" else "This parameter" in
    msg ^ " is " ^ mut in
  add_list (Typing.err_code Typing.MutabilityMismatch)
  [
    pos1, "Incompatible mutabilities:";
    pos1, msg mut1;
    pos2, msg mut2;
  ]

let invalid_call_on_maybe_mutable ~fun_is_mutable pos fpos =
  let msg =
    "Cannot call " ^ (if fun_is_mutable then "mutable" else "non-mutable") ^ " \
    function on maybe mutable value." in
  add_list (Typing.err_code Typing.InvalidCallMaybeMutable)
  [
    pos, msg;
    fpos, "This function is not marked as <<__MaybeMutable>>."
  ]

let mutable_argument_mismatch param_pos arg_pos =
  add_list (Typing.err_code Typing.MutableArgumentMismatch)
  [
    arg_pos, "Invalid argument";
    param_pos, "This parameter is marked mutable";
    arg_pos, "But this expression is not";
  ]

let immutable_argument_mismatch param_pos arg_pos =
  add_list (Typing.err_code Typing.ImmutableArgumentMismatch)
  [
    arg_pos, "Invalid argument";
    param_pos, "This parameter is not marked as mutable";
    arg_pos, "But this expression is mutable";
  ]

let maybe_mutable_argument_mismatch param_pos arg_pos =
  add_list (Typing.err_code Typing.MaybeMutableArgumentMismatch)
  [
    arg_pos, "Invalid argument";
    param_pos, "This parameter is not marked <<__MaybeMutable>>";
    arg_pos, "But this expression is maybe mutable"
  ]

let invalid_mutable_return_result error_pos function_pos value_kind =
  add_list (Typing.err_code Typing.InvalidMutableReturnResult)
  [
    error_pos, "Functions marked <<__MutableReturn>> must return mutably owned values";
    function_pos, "This function is marked <<__MutableReturn>>";
    error_pos, "This expression is (" ^ value_kind ^ ")"
  ]

let freeze_in_nonreactive_context pos1 =
  add (Typing.err_code Typing.FreezeInNonreactiveContext) pos1
  ("\\HH\\Rx\\freeze can only be used in reactive functions")

let mutable_in_nonreactive_context pos =
  add (Typing.err_code Typing.MutableInNonreactiveContext) pos
  ("\\HH\\Rx\\mutable can only be used in reactive functions")

let invalid_argument_type_for_condition_in_rx
  ~is_receiver f_pos def_pos arg_pos expected_type actual_type =
  let arg_msg =
    if is_receiver then "Receiver type" else "Argument type" in
  let arg_msg =
    arg_msg ^ " must be a subtype of " ^ expected_type ^
    ", now " ^ actual_type ^ "." in
  add_list (Typing.err_code Typing.InvalidConditionallyReactiveCall) [
    f_pos, "Cannot invoke conditionally reactive function in reactive context, \
    because at least one reactivity condition is not met.";
    arg_pos, arg_msg;
    def_pos, "This is the function declaration";
  ]

let callsite_reactivity_mismatch f_pos def_pos callee_reactivity caller_reactivity =
  add_list (Typing.err_code Typing.CallSiteReactivityMismatch) [
    f_pos, "Reactivity mismatch: " ^ caller_reactivity ^ " function cannot call " ^
           callee_reactivity ^ " function.";
    def_pos, "This is declaration of the function being called."
  ]

let invalid_function_type_for_condition_in_rx
  f_pos def_pos arg_pos actual_reactivity expected_reactivity =
  let arg_msg =
    "Argument type is must be " ^ expected_reactivity ^ " function, " ^
    actual_reactivity ^ " given." in
  add_list (Typing.err_code Typing.InvalidConditionallyReactiveCall) [
    f_pos, "Cannot invoke conditionally reactive function in reactive context, \
    because at least one reactivity condition is not met.";
    arg_pos, arg_msg;
    def_pos, "This is the function declaration";
  ]


let invalid_argument_of_rx_mutable_function pos =
  add (Typing.err_code Typing.InvalidArgumentOfRxMutableFunction) pos (
      "Single argument to \\HH\\Rx\\mutable should be an expression that yields new \
       mutably-owned value, like 'new A()', Hack collection literal or 'f()' where f is function \
       annotated with <<__MutableReturn>> attribute."
  )

let invalid_freeze_use pos1 =
  add (Typing.err_code Typing.InvalidFreezeUse) pos1
  ("freeze takes a single mutably-owned local variable as an argument")

let invalid_freeze_target pos1 var_pos var_mutability_str =
  add_list (Typing.err_code Typing.InvalidFreezeTarget)
  [
    pos1, "Invalid argument - freeze() takes a single mutable variable";
    var_pos, "This variable is "^var_mutability_str;
  ]

let discarded_awaitable pos1 pos2 =
  add_list (Typing.err_code Typing.DiscardedAwaitable) [
  pos1, "This expression is of type Awaitable, but it's "^
  "either being discarded or used in a dangerous way before "^
  "being awaited";
  pos2, "This is why I think it is Awaitable"
]

let gena_expects_array pos1 pos2 ty_str =
  add_list (Typing.err_code Typing.GenaExpectsArray) [
  pos1, "gena expects an array";
  pos2, "It is incompatible with " ^ ty_str;
]

let unify_error left right =
  add_list (Typing.err_code Typing.UnifyError) (left @ right)

let static_dynamic static_position dyn_position method_name =
  let msg_static = "The function "^method_name^" is static" in
  let msg_dynamic = "It is defined as dynamic here" in
  add_list (Typing.err_code Typing.StaticDynamic) [
  static_position, msg_static;
  dyn_position, msg_dynamic
]

let null_member s pos r =
  add_list (Typing.err_code Typing.NullMember) ([
  pos,
  "You are trying to access the member "^s^
  " but this object can be null. "
] @ r
)

let non_object_member s pos1 ty pos2 =
  add_list (Typing.err_code Typing.NonObjectMember) [
  pos1,
  ("You are trying to access the member "^s^
   " but this is not an object, it is "^
   ty);
  pos2,
  "Check this out"
  ]

let non_class_member s pos1 ty pos2 =
  add_list (Typing.err_code Typing.NonClassMember) [
  pos1,
  ("You are trying to access the member "^s^
   " but this is not a class, it is "^
   ty);
  pos2,
  "Check this out"
  ]

let ambiguous_member s pos1 ty pos2 =
  add_list (Typing.err_code Typing.AmbiguousMember) [
  pos1,
  ("You are trying to access the member "^s^
   " but there is more than one implementation on "^
   ty);
  pos2,
  "Check this out"
  ]

let null_container p null_witness =
  add_list (Typing.err_code Typing.NullContainer) (
  [
   p,
   "You are trying to access an element of this container"^
   " but the container could be null. "
 ] @ null_witness)

let option_mixed pos =
  add (Typing.err_code Typing.OptionMixed) pos
    "?mixed is a redundant typehint - just use mixed"

let option_void pos =
  add (Typing.err_code Typing.OptionVoid) pos
    "?void is a redundant typehint - just use void"

let declared_covariant pos1 pos2 emsg =
  add_list (Typing.err_code Typing.DeclaredCovariant) (
  [pos2, "Illegal usage of a covariant type parameter";
   pos1, "This is where the parameter was declared as covariant (+)"
 ] @ emsg
 )

let declared_contravariant pos1 pos2 emsg =
  add_list (Typing.err_code Typing.DeclaredContravariant) (
  [pos2, "Illegal usage of a contravariant type parameter";
   pos1, "This is where the parameter was declared as contravariant (-)"
 ] @ emsg
 )

let contravariant_this pos class_name tp =
  add (Typing.err_code Typing.ContravariantThis) pos (
    "The \"this\" type cannot be used in this " ^
    "contravariant position because its enclosing class \"" ^ class_name ^
    "\" " ^ "is final and has a variant type parameter \"" ^ tp ^ "\"")

let cyclic_typeconst pos sl =
  let sl = List.map sl strip_ns in
  add (Typing.err_code Typing.CyclicTypeconst) pos
    ("Cyclic type constant:\n  "^String.concat " -> " sl)

let this_lvalue pos =
  add (Typing.err_code Typing.ThisLvalue) pos "Cannot assign a value to $this"

let abstract_concrete_override pos parent_pos kind =
  let kind_str = match kind with
    | `method_ -> "method"
    | `typeconst -> "type constant"
    | `constant -> "constant" in
  add_list (Typing.err_code Typing.AbstractConcreteOverride) ([
    pos, "Cannot re-declare this " ^ kind_str ^ " as abstract";
    parent_pos, "Previously defined here"
  ])

let instanceof_generic_classname pos name =
  add (Typing.err_code Typing.InstanceofGenericClassname) pos
    ("'instanceof' cannot be used on 'classname<" ^ name ^ ">' because '" ^
    name ^ "' may be instantiated with a type such as \
     'C<int>' that cannot be checked at runtime")

let required_field_is_optional pos1 pos2 name =
  add_list (Typing.err_code Typing.RequiredFieldIsOptional)
    [
      pos1, "The field '"^name^"' is optional";
      pos2, "The field '"^name^"' is defined as required"
    ]

let array_get_with_optional_field pos1 pos2 name =
  add_list
    (Typing.err_code Typing.ArrayGetWithOptionalField)
    [
      pos1,
      "Invalid index operation: '" ^ name ^ "' is marked as an optional shape \
      field. It may not be present in the shape. Use the `??` operator \
      instead.";
      pos2,
      "This is where the field was declared as optional."
    ]

let non_call_argument_in_suspend pos msgs =
  add_list
    (Typing.err_code Typing.NonCallArgumentInSuspend) (
    [
      pos,
      "'suspend' operator expects call to a coroutine as an argument."
    ] @ msgs
  )
let non_coroutine_call_in_suspend pos msgs =
  add_list
    (Typing.err_code Typing.NonCoroutineCallInSuspend) (
    [
      pos,
      "Only coroutine functions are allowed to be called in \
      'suspend' operator."
    ] @ msgs
  )

let coroutine_call_outside_of_suspend pos =
  add_list
    (Typing.err_code Typing.CoroutineCallOutsideOfSuspend)
    [
      pos,
      "Coroutine calls are only allowed when they are arguments to \
      'suspend' operator"
    ]

let function_is_not_coroutine pos name =
  add_list
    (Typing.err_code Typing.FunctionIsNotCoroutine)
    [
      pos,
      "Function '" ^ name ^ "' is not a coroutine and cannot be \
       used in as an argument of 'suspend' operator."
    ]

let coroutinness_mismatch pos1_is_coroutine pos1 pos2 =
  let m1 = "This is a coroutine." in
  let m2 = "This is not a coroutine." in
  add_list
    (Typing.err_code Typing.CoroutinnessMismatch)
    [
      pos1, if pos1_is_coroutine then m1 else m2;
      pos2, if pos1_is_coroutine then m2 else m1;
    ]

let invalid_ppl_call pos context =
  let error_msg = "Cannot call a method on an object of a <<__PPL>> class "^context in
  add (Typing.err_code Typing.InvalidPPLCall) pos error_msg

let invalid_ppl_static_call pos reason =
  let error_msg = "Cannot call a static method on a <<__PPL>> class "^reason in
  add (Typing.err_code Typing.InvalidPPLStaticCall) pos error_msg

let ppl_meth_pointer pos func =
  let error_msg = func^" cannot be used with a <<__PPL>> class" in
  add (Typing.err_code Typing.PPLMethPointer) pos error_msg

let coroutine_outside_experimental pos =
  add (Typing.err_code Typing.CoroutineOutsideExperimental) pos
    Coroutine_errors.error_message

let return_disposable_mismatch pos1_return_disposable pos1 pos2 =
  let m1 = "This is marked <<__ReturnDisposable>>." in
  let m2 = "This is not marked <<__ReturnDisposable>>." in
  add_list
    (Typing.err_code Typing.ReturnDisposableMismatch)
    [
      pos1, if pos1_return_disposable then m1 else m2;
      pos2, if pos1_return_disposable then m2 else m1;
    ]

let return_void_to_rx_mismatch ~pos1_has_attribute pos1 pos2 =
  let m1 = "This is marked <<__ReturnsVoidToRx>>." in
  let m2 = "This is not marked <<__ReturnsVoidToRx>>." in
  add_list
    (Typing.err_code Typing.ReturnVoidToRxMismatch)
    [
      pos1, if pos1_has_attribute then m1 else m2;
      pos2, if pos1_has_attribute then m2 else m1;
    ]

let this_as_lexical_variable pos =
  add (Naming.err_code Naming.ThisAsLexicalVariable) pos "Cannot use $this as lexical variable"

let dollardollar_lvalue pos =
  add (Typing.err_code Typing.DollardollarLvalue) pos
    "Cannot assign a value to the special pipe variable ($$)"

let assigning_to_const pos =
  add (Typing.err_code Typing.AssigningToConst) pos
    "Cannot assign to a __Const property"

let self_const_parent_not pos =
  add (Typing.err_code Typing.SelfConstParentNot) pos
    "A __Const class may only extend other __Const classes"

let parent_const_self_not pos =
  add (Typing.err_code Typing.ParentConstSelfNot) pos
    "Only __Const classes may extend a __Const class"

let overriding_prop_const_mismatch parent_pos parent_const child_pos child_const =
  let m1 = "This property is __Const" in
  let m2 = "This property is not __Const" in
  add_list (Typing.err_code Typing.OverridingPropConstMismatch)
  [
    parent_pos, if parent_const then m1 else m2;
    child_pos, if child_const then m1 else m2;
  ]

let mutable_return_result_mismatch pos1_has_mutable_return pos1 pos2 =
  let m1 = "This is marked <<__MutableReturn>>." in
  let m2 = "This is not marked <<__MutableReturn>>." in
  add_list
    (Typing.err_code Typing.MutableReturnResultMismatch)
    [
      pos1, if pos1_has_mutable_return then m1 else m2;
      pos2, if pos1_has_mutable_return then m2 else m1;
    ]

(*****************************************************************************)
(* Typing decl errors *)
(*****************************************************************************)

let wrong_extend_kind child_pos child parent_pos parent =
  let msg1 = child_pos, child^" cannot extend "^parent in
  let msg2 = parent_pos, "This is "^parent in
  add_list (Typing.err_code Typing.WrongExtendKind) [msg1; msg2]

let unsatisfied_req parent_pos req_name req_pos =
  let s1 = "Failure to satisfy requirement: "^(Utils.strip_ns req_name) in
  let s2 = "Required here" in
  if req_pos = parent_pos
  then add (Typing.err_code Typing.UnsatisfiedReq) parent_pos s1
  else add_list (Typing.err_code Typing.UnsatisfiedReq) [parent_pos, s1; req_pos, s2]

let cyclic_class_def stack pos =
  let stack =
    SSet.fold ~f:(fun x y -> (Utils.strip_ns x)^" "^y) stack ~init:"" in
  add (Typing.err_code Typing.CyclicClassDef) pos ("Cyclic class definition : "^stack)

let trait_reuse p_pos p_name class_name trait =
  let c_pos, c_name = class_name in
  let c_name = Utils.strip_ns c_name in
  let trait = Utils.strip_ns trait in
  let err = "Class "^c_name^" reuses trait "^trait^" in its hierarchy" in
  let err' = "It is already used through "^(Utils.strip_ns p_name) in
  add_list (Typing.err_code Typing.TraitReuse) [c_pos, err; p_pos, err']

(**
 * This error should be unfixmeable, because the `is` expression does not
 * support it at all.
 *)
let invalid_is_as_expression_hint op hint_pos ty_pos ty_str =
  add_list (Typing.err_code Typing.InvalidIsAsExpressionHint) [
    hint_pos, ("Invalid \"" ^ op ^ "\" expression hint");
    ty_pos, ("The \"" ^ op ^ "\" operator cannot be used with " ^ ty_str);
  ]

(**
 * This error is fixmeable, because the typechecker will still refine the type
 * despite the hint not being completely valid.
 *)
let partially_valid_is_as_expression_hint op hint_pos ty_pos ty_str =
  add_list (Typing.err_code Typing.PartiallyValidIsAsExpressionHint) [
    hint_pos, ("Invalid \"" ^ op ^ "\" expression hint");
    ty_pos, ("The \"" ^ op ^ "\" operator should not be used with " ^ ty_str);
  ]

let override_final ~parent ~child =
  add_list (Typing.err_code Typing.OverrideFinal) [child, "You cannot override this method";
            parent, "It was declared as final"]

let should_be_override pos class_id id =
  add (Typing.err_code Typing.ShouldBeOverride) pos
    ((Utils.strip_ns class_id)^"::"^id^"() is marked as override; \
       no non-private parent definition found \
       or overridden parent is defined in non-<?hh code")

let override_per_trait class_name id m_pos =
    let c_pos, c_name = class_name in
    let err_msg =
      ("Method "^(Utils.strip_ns c_name)^"::"^id^" should be an override \
        per the declaring trait; no non-private parent definition found \
        or overridden parent is defined in non-<?hh code")
    in add_list (Typing.err_code Typing.OverridePerTrait) [
      c_pos, err_msg;
      m_pos, "Declaration of "^id^"() is here"
    ]

let missing_assign pos =
  add (Typing.err_code Typing.MissingAssign) pos "Please assign a value"

let private_override pos class_id id =
  add (Typing.err_code Typing.PrivateOverride) pos ((Utils.strip_ns class_id)^"::"^id
          ^": combining private and override is nonsensical")

let invalid_memoized_param pos ty_reason_msg =
  add_list (Typing.err_code Typing.InvalidMemoizedParam) (
    ty_reason_msg @ [pos,
      "Parameters to memoized function must be null, bool, int, float, string, \
      an object deriving IMemoizeParam, or a Container thereof. See also \
      http://docs.hhvm.com/hack/attributes/special#__memoize"])

let invalid_disposable_hint pos class_name =
  add (Typing.err_code Typing.InvalidDisposableHint) pos
    ("Parameter with type '" ^ class_name ^ "' must not \
    implement IDisposable or IAsyncDisposable. Please use <<__AcceptDisposable>> attribute or \
    create disposable object with 'using' statement instead.")

let invalid_disposable_return_hint pos class_name =
  add (Typing.err_code Typing.InvalidDisposableReturnHint) pos
    ("Return type '" ^ class_name ^ "' must not \
    implement IDisposable or IAsyncDisposable. Please add <<__ReturnDisposable>> attribute.")

let xhp_required pos why_xhp ty_reason_msg =
  let msg = "An XHP instance was expected" in
  add_list (Typing.err_code Typing.XhpRequired) ((pos, msg)::(pos, why_xhp)::ty_reason_msg)

let illegal_xhp_child pos ty_reason_msg =
  let msg = "XHP children must be compatible with XHPChild" in
  add_list (Typing.err_code Typing.IllegalXhpChild) ((pos, msg)::ty_reason_msg)

let nullsafe_not_needed p nonnull_witness =
  add_list (Typing.err_code Typing.NullsafeNotNeeded) (
  [
   p,
   "You are using the ?-> operator but this object cannot be null. "
 ] @ nonnull_witness)

let generic_at_runtime p =
  add (Typing.err_code Typing.GenericAtRuntime) p
    "Generics can only be used in type hints since they are erased at runtime."

let trivial_strict_eq p b left right left_trail right_trail =
  let msg = "This expression is always "^b in
  let left_trail = List.map left_trail typedef_trail_entry in
  let right_trail = List.map right_trail typedef_trail_entry in
  add_list (Typing.err_code Typing.TrivialStrictEq)
    ((p, msg) :: left @ left_trail @ right @ right_trail)

let trivial_strict_not_nullable_compare_null p result type_reason =
  let msg = "This expression is always "^result in
  add_list (Typing.err_code Typing.NotNullableCompareNullTrivial)
    ((p, msg) :: type_reason)

let eq_incompatible_types p left right =
  let msg = "This equality test has incompatible types" in
  add_list (Typing.err_code Typing.EqIncompatibleTypes)
    ((p, msg) :: left @ right)

let comparison_invalid_types p left right =
  let msg = "This comparison has invalid types.  Only comparisons in which \
             both arguments are strings, nums, DateTime, or DateTimeImmutable \
             are allowed" in
  add_list (Typing.err_code Typing.ComparisonInvalidTypes) ((p, msg) :: left @ right)

let void_usage p void_witness =
  let msg = "You are using the return value of a void function" in
  add_list (Typing.err_code Typing.VoidUsage) ((p, msg) :: void_witness)

let noreturn_usage p noreturn_witness =
  let msg = "You are using the return value of a noreturn function" in
  add_list (Typing.err_code Typing.NoreturnUsage) ((p, msg) :: noreturn_witness)

let attribute_too_few_arguments pos x n =
  let n = string_of_int n in
  add (Typing.err_code Typing.AttributeTooFewArguments) pos (
    "The attribute "^x^" expects at least "^n^" arguments"
  )

let attribute_too_many_arguments pos x n =
  let n = string_of_int n in
  add (Typing.err_code Typing.AttributeTooManyArguments) pos (
    "The attribute "^x^" expects at most "^n^" arguments"
  )

let attribute_param_type pos x =
  add (Typing.err_code Typing.AttributeParamType) pos (
    "This attribute parameter should be "^x
  )

let deprecated_use pos pos_def msg =
  add_list (Typing.err_code Typing.DeprecatedUse) [
    pos, msg;
    pos_def, "Definition is here";
  ]

let cannot_declare_constant kind pos (class_pos, class_name) =
  let kind_str =
    match kind with
    | `enum -> "an enum"
    | `trait -> "a trait"
  in
  add_list (Typing.err_code Typing.CannotDeclareConstant) [
    pos, "Cannot declare a constant in "^kind_str;
    class_pos, (strip_ns class_name)^" was defined as "^kind_str^" here";
  ]

let ambiguous_inheritance pos class_ origin (error: error) =
  let origin = strip_ns origin in
  let class_ = strip_ns class_ in
  let message = "This declaration was inherited from an object of type "^origin^
    ". Redeclare this member in "^class_^" with a compatible signature." in
  let code, msgl = (get_code error), (to_list error) in
  add_list code (msgl @ [pos, message])

let explain_contravariance pos c_name error =
  let message = "Considering that this type argument is contravariant "^
                "with respect to " ^ strip_ns c_name in
  let code, msgl = (get_code error), (to_list error) in
  add_list code (msgl @ [pos, message])

let explain_invariance pos c_name suggestion error =
  let message = "Considering that this type argument is invariant "^
                  "with respect to " ^ strip_ns c_name ^ suggestion in
  let code, msgl = (get_code error), (to_list error) in
  add_list code (msgl @ [pos, message])

let local_variable_modified_and_used pos_modified pos_used_l =
  let used_msg p = p, "And accessed here" in
  add_list (Typing.err_code Typing.LocalVariableModifedAndUsed)
           ((pos_modified, "Unsequenced modification and access to local \
                            variable. Modified here") ::
            List.map pos_used_l used_msg)

let local_variable_modified_twice pos_modified pos_modified_l =
  let modified_msg p = p, "And also modified here" in
  add_list (Typing.err_code Typing.LocalVariableModifedTwice)
           ((pos_modified, "Unsequenced modifications to local variable. \
                            Modified here") ::
            List.map pos_modified_l modified_msg)

let assign_during_case p =
  add (Typing.err_code Typing.AssignDuringCase) p
    "Don't assign to variables inside of case labels"

let cyclic_enum_constraint pos =
  add (Typing.err_code Typing.CyclicEnumConstraint) pos "Cyclic enum constraint"

let invalid_classname p =
  add (Typing.err_code Typing.InvalidClassname) p "Not a valid class name"

let illegal_type_structure pos errmsg =
  let msg =
    "The two arguments to type_structure() must be:"
    ^"\n - first: ValidClassname::class or an object of that class"
    ^"\n - second: a single-quoted string literal containing the name"
    ^" of a type constant of that class"
    ^"\n"^errmsg in
  add (Typing.err_code Typing.IllegalTypeStructure) pos msg

let illegal_typeconst_direct_access pos =
  let msg =
    "Type constants cannot be directly accessed. "
    ^"Use type_structure(ValidClassname::class, 'TypeConstName') instead" in
  add (Typing.err_code Typing.IllegalTypeStructure) pos msg

let class_property_only_static_literal pos =
  let msg =
    "Initialization of class property must be a static literal expression." in
  add (Typing.err_code Typing.ClassPropertyOnlyStaticLiteral) pos msg

let reference_expr pos =
  let msg = "Cannot take a value by reference in strict mode." in
  add (Typing.err_code Typing.ReferenceExpr) pos msg

let pass_by_ref_annotation_missing pos1 pos2 =
  let msg1 = pos1, "This argument should be annotated with &" in
  let msg2 = pos2, "Because this parameter is passed by reference" in
  add_list (Typing.err_code Typing.PassByRefAnnotationMissing) [msg1; msg2]

let pass_by_ref_annotation_unexpected pos1 pos2 pos2_is_variadic =
  let msg1 = pos1, "This argument should not be annotated with &" in
  let param_str = if pos2_is_variadic
    then "variadic parameters are"
    else "this parameter is" in
  let msg2 = pos2, "Because " ^ param_str ^ " passed by value" in
  add_list (Typing.err_code Typing.PassByRefAnnotationUnexpected) [msg1; msg2]

let reffiness_invariant pos1 pos2 mode2 =
  let msg1 = pos1, "This parameter is passed by reference" in
  let mode_str = match mode2 with
    | `normal -> "a normal parameter"
    | `inout -> "an inout parameter" in
  let msg2 = pos2, "It is incompatible with " ^ mode_str in
  add_list (Typing.err_code Typing.ReffinessInvariant) [msg1; msg2]

let inout_annotation_missing pos1 pos2 =
  let msg1 = pos1, "This argument should be annotated with 'inout'" in
  let msg2 = pos2, "Because this is an inout parameter" in
  add_list (Typing.err_code Typing.InoutAnnotationMissing) [msg1; msg2]

let inout_annotation_unexpected pos1 pos2 pos2_is_variadic =
  let msg1 = pos1, "Unexpected inout annotation for argument" in
  let msg2 = pos2, if pos2_is_variadic
    then "A variadic parameter can never be inout"
    else "This is a normal parameter (does not have 'inout')" in
  add_list (Typing.err_code Typing.InoutAnnotationUnexpected) [msg1; msg2]

let inoutness_mismatch pos1 pos2 =
  let msg1 = pos1, "This is an inout parameter" in
  let msg2 = pos2, "It is incompatible with a normal parameter" in
  add_list (Typing.err_code Typing.InoutnessMismatch) [msg1; msg2]

let invalid_new_disposable pos =
  let msg =
    "Disposable objects may only be created in a 'using' statement or 'return' from function marked <<__ReturnDisposable>>" in
  add (Typing.err_code Typing.InvalidNewDisposable) pos msg

let invalid_return_disposable pos =
  let msg =
    "Return expression must be new disposable in function marked <<__ReturnDisposable>>" in
  add (Typing.err_code Typing.InvalidReturnDisposable) pos msg

let nonreactive_function_call pos decl_pos =
  add_list (Typing.err_code Typing.NonreactiveFunctionCall) [
    pos, "Reactive functions can only call other reactive functions.";
    decl_pos, "This function is not reactive."
  ]

let nonreactive_call_from_shallow pos decl_pos =
  add_list (Typing.err_code Typing.NonreactiveCallFromShallow) [
      pos, "Shallow reactive functions cannot call non-reactive functions.";
      decl_pos, "This function is not reactive."
  ]

let rx_enabled_in_non_rx_context pos =
  add (Typing.err_code Typing.RxEnabledInNonRxContext) pos (
    "\\HH\\Rx\\IS_ENABLED can only be used in reactive functions."
  )

let rx_enabled_in_lambdas pos =
  add (Typing.err_code Typing.RxEnabledInLambdas) pos (
    "\\HH\\Rx\\IS_ENABLED cannot be used inside lambdas."
  )

let rx_parameter_condition_mismatch cond pos def_pos =
  add_list (Typing.err_code Typing.RxParameterConditionMismatch) [
    pos, "This parameter does not satisfy "^ cond ^ " condition defined on \
    matching parameter in function super type.";
    def_pos, "This is parameter declaration from the function super type."
  ]
let nonreactive_append pos =
  let msg = "Cannot append to a Hack Collection object in a reactive context" in
  add (Typing.err_code Typing.NonreactiveAppend) pos msg

let obj_set_reactive pos =
  let msg = ("This object's property is being mutated(used as an lvalue)" ^
  "\nYou cannot set non-mutable object properties in reactive functions") in
  add (Typing.err_code Typing.ObjSetReactive) pos msg

let invalid_unset_target_rx pos =
  add (Typing.err_code Typing.InvalidUnsetTargetInRx) pos (
    "Non-mutable argument for 'unset' is not allowed in reactive functions."
  )

let inout_argument_bad_type pos msgl =
  let msg =
    "Expected argument marked inout to be contained in a local or " ^
    "a value-typed container (e.g. vec, dict, keyset, array). " ^
    "To use inout here, assign to/from a temporary local variable." in
  add_list (Typing.err_code Typing.InoutArgumentBadType) ((pos, msg) :: msgl)

let ambiguous_lambda pos uses =
  let msg1 =
    "Lambda has parameter types that could not be determined at definition site." in
  let msg2 =
    Printf.sprintf
    "%d distinct use types were determined: please add type hints to lambda parameters."
      (List.length uses) in
  add_list (Typing.err_code Typing.AmbiguousLambda) ([(pos, msg1); (pos, msg2)] @
    List.map uses (fun (pos, ty) -> (pos, "This use has type " ^ ty)))

let wrong_expression_kind_attribute expr_kind pos attr attr_class_pos attr_class_name intf_name =
  let msg1 =
    Printf.sprintf "The %s attribute cannot be used on %s." attr expr_kind in
  let msg2 =
    Printf.sprintf "The attribute's class is defined here. To be available for use on \
                    %s, the %s class must implement %s." expr_kind
                    (String_utils.string_after attr_class_name 1)
                    (String_utils.string_after intf_name 1) in
  add_list (Typing.err_code Typing.WrongExpressionKindAttribute) [
    pos, msg1;
    attr_class_pos, msg2
  ]

let attribute_class_no_constructor_args pos def_pos =
  let msg =
    "The class associated with this attribute has no constructor. " ^
    "Please add a constructor to use arguments with this attribute." in
  add_list (Typing.err_code Typing.AttributeClassNoConstructorArgs) [
      pos, msg;
      def_pos, "The attribute's class is defined here."
  ]

let cannot_return_borrowed_value_as_immutable fun_pos value_pos =
  add_list (Typing.err_code Typing.CannotReturnBorrowedValueAsImmutable) [
    fun_pos, "Values returned from reactive function by default are treated \
    as immutable.";
    value_pos, "This value is mutably borrowed and cannot be returned as immutable"
  ]

let decl_override_missing_hint pos =
  add (Typing.err_code Typing.DeclOverrideMissingHint) pos
    "When redeclaring class members, both declarations must have a typehint"

let let_var_immutability_violation pos id =
  add (Typing.err_code Typing.LetVarImmutabilityViolation) pos
    ("Let variables are immutable. Using let variable " ^ id ^ " in write context is not allowed.")

let invalid_type_for_onlyrx_if_rxfunc_parameter pos type_str =
  add (Typing.err_code Typing.InvalidTypeForOnlyrxIfRxfuncParameter) pos (
    "Parameter annotated with <<__OnlyRxIfRxFunc>> attribute must be function, \
    now '" ^ type_str ^ "'."
  )

let missing_annotation_for_onlyrx_if_rxfunc_parameter pos =
  add (Typing.err_code Typing.MissingAnnotationForOnlyrxIfRxfuncParameter) pos (
    "Missing function type annotation on parameter marked with <<__OnlyRxIfRxFunc>> attribute."
  )

let binding_ref_in_array pos =
  let msg = "Binding a reference in an array is no longer supported in Hack." in
  add (Typing.err_code Typing.BindingRefInArray) pos msg

let return_ref_in_array pos =
  let msg = "Returning a reference to an element in an array is no longer " ^
    "supported in Hack." in
  add (Typing.err_code Typing.BindingRefInArray) pos msg

let passing_array_cell_by_ref pos =
  let msg = "Passing array cells by reference is no longer supported; " ^
    "use 'inout' instead" in
  add (Typing.err_code Typing.PassingArrayCellByRef) pos msg

let superglobal_in_reactive_context pos name =
  add (Typing.err_code Typing.SuperglobalInReactiveContext) pos (
    "Superglobal "^  name ^ " cannot be used in a reactive context."
  )

let global_in_reactive_context pos name =
  add (Typing.err_code Typing.GlobalInReactiveContext) pos (
    "Global " ^ name ^ " cannot be used in a reactive context."
  )

let static_property_in_reactive_context pos =
  add (Typing.err_code Typing.StaticPropertyInReactiveContext) pos (
    "Static property cannot be used in a reactive context."
  )

let static_in_reactive_context pos name =
  add (Typing.err_code Typing.StaticInReactiveContext) pos (
    "Static " ^ name ^ " cannot be used in a reactive context."
  )

let returns_void_to_rx_function_as_non_expression_statement pos fpos =
  add_list (Typing.err_code Typing.ReturnsVoidToRxAsNonExpressionStatement) [
    pos, "Cannot use result of function annotated with <<__ReturnsVoidToRx>> \
    in reactive context";
    fpos, "This is function declaration."
  ]

let non_awaited_awaitable_in_rx pos =
  add (Typing.err_code Typing.NonawaitedAwaitableInReactiveContext) pos (
    "This value has Awaitable type. Awaitable typed values in reactive code \
    must be either immediately await'ed or passed as arguments to 'genva' function."
  )

let shapes_key_exists_always_true pos1 name pos2 =
  add_list (Typing.err_code Typing.ShapesKeyExistsAlwaysTrue) [
    pos1, "This Shapes::keyExists() check is always true";
    pos2, "The field '" ^ name ^ "' exists because of this definition"
  ]

let shape_field_non_existence_reason name = function
  | `Undefined ->
    "The field '" ^ name ^ "' is not defined in this shape"
  | `Unset ->
    "The field '" ^ name ^ "' was unset here"

let shapes_key_exists_always_false pos1 name pos2 reason =
  add_list (Typing.err_code Typing.ShapesKeyExistsAlwaysFalse) [
    pos1, "This Shapes::keyExists() check is always false";
    pos2, shape_field_non_existence_reason name reason
  ]

let shapes_idx_with_non_existent_field pos1 name pos2 reason =
  add_list (Typing.err_code Typing.ShapesIdxWithNonExistentField) [
    pos1, "You are calling Shapes::idx() on a field known to not exist";
    pos2, shape_field_non_existence_reason name reason
  ]

let ambiguous_object_access pos name self_pos vis subclass_pos class_self class_subclass =
  let class_self = Utils.strip_ns class_self in
  let class_subclass = Utils.strip_ns class_subclass in
  add_list (Typing.err_code Typing.AmbiguousObjectAccess) [
    pos, "This object access to " ^ name ^ " is ambiguous";
    self_pos, "You will access the private instance declared in " ^ class_self;
    subclass_pos, "Instead of the " ^ vis ^ " instance declared in " ^ class_subclass;
  ]

let invalid_traversable_in_rx pos =
  add (Typing.err_code Typing.InvalidTraversableInRx) pos (
    "Cannot traverse over non-reactive traversable in reactive code."
  )

let lateinit_with_default pos =
  add (Typing.err_code Typing.LateInitWithDefault) pos
    "A late-initialized property cannot have a default value"

let forward_compatibility_not_current pos value =
  let current = ForwardCompatibilityLevel.current in
  add (Init.err_code Init.ForwardCompatibilityNotCurrent)
    pos
    (Printf.sprintf
      "forward_compatibility_level is set to '%s' (%d), which is stale; current is '%s' (%d). Errors may be missing."
      (ForwardCompatibilityLevel.as_string value)
      (ForwardCompatibilityLevel.as_int value)
      (ForwardCompatibilityLevel.as_string current)
      (ForwardCompatibilityLevel.as_int current)
    )

let forward_compatibility_below_minimum pos value =
  let minimum = ForwardCompatibilityLevel.minimum in
  let current = ForwardCompatibilityLevel.current in
  add (Init.err_code Init.ForwardCompatibilityBelowMinimum)
    pos
    (Printf.sprintf
      "forward_compatibility_level is set to '%s' (%d), which is below the minimum of '%s' (%d); current is '%s' (%d)"
      (ForwardCompatibilityLevel.as_string value)
      (ForwardCompatibilityLevel.as_int value)
      (ForwardCompatibilityLevel.as_string minimum)
      (ForwardCompatibilityLevel.as_int minimum)
      (ForwardCompatibilityLevel.as_string current)
      (ForwardCompatibilityLevel.as_int current)
    )

(*****************************************************************************)
(* Convert relative paths to absolute. *)
(*****************************************************************************)

let to_absolute = M.to_absolute

(*****************************************************************************)
(* Printing *)
(*****************************************************************************)

let to_json (error : Pos.absolute error_) =
  let error_code, msgl = (get_code error), (to_list error) in
  let elts = List.map msgl begin fun (p, w) ->
    let line, scol, ecol = Pos.info_pos p in
    Hh_json.JSON_Object [
        "descr", Hh_json.JSON_String w;
        "path",  Hh_json.JSON_String (Pos.filename p);
        "line",  Hh_json.int_ line;
        "start", Hh_json.int_ scol;
        "end",   Hh_json.int_ ecol;
        "code",  Hh_json.int_ error_code
    ]
  end in
  Hh_json.JSON_Object [ "message", Hh_json.JSON_Array elts ]

let to_string = M.to_string

(*****************************************************************************)
(* Try if errors. *)
(*****************************************************************************)

let try_ f1 f2 =
  M.try_with_result f1 (fun _ l -> f2 l)

let try_with_error f1 f2 =
  try_ f1 (fun err -> add_error err; f2())

let try_add_err pos err f1 f2 =
  try_ f1 begin fun error ->
    let error_code, l = (get_code error), (to_list error) in
    add_list error_code ((pos, err) :: l);
    f2()
  end

let has_no_errors f =
  try_ (fun () -> f(); true) (fun _ -> false)

(*****************************************************************************)
(* Do. *)
(*****************************************************************************)

let do_ = M.do_
let do_with_context = M.do_with_context

let run_in_context = M.run_in_context
let run_in_decl_mode = M.run_in_decl_mode

let ignore_ f =
  let allow_errors_in_default_path_copy = !allow_errors_in_default_path in
  set_allow_errors_in_default_path true;
  let _, result =  (do_ f) in
  set_allow_errors_in_default_path allow_errors_in_default_path_copy;
  result

let try_when f ~when_ ~do_ =
  M.try_with_result f begin fun result (error: error) ->
    if when_()
    then do_ error
    else add_error error;
    result
  end

(* Whether we've found at least one error *)
let currently_has_errors = M.currently_has_errors

(* Runs the first function that is expected to produce an error. If it doesn't
 * then we run the second function we are given
 *)
let must_error f error_fun =
  let had_no_errors = try_with_error (fun () -> f(); true) (fun _ -> false) in
  if had_no_errors then error_fun();

end

let errors_without_tracing =
  (module Errors_with_mode(NonTracingErrors) : Errors_sig.S)
let errors_with_tracing =
  (module Errors_with_mode(TracingErrors) : Errors_sig.S)

include (val (if Injector_config.use_error_tracing
  then errors_with_tracing
  else errors_without_tracing))
