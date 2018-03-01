(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
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

type error_phase = Parsing | Naming | Decl | Typing

(* For callers that don't care about tracking error origins *)
let default_context = (Relative_path.default, Typing)

(* The file and phase of analysis being currently performed *)
let current_context: (Relative_path.t * error_phase) ref = ref default_context

module PhaseMap = Reordered_argument_map(MyMap.Make(struct
  type t = error_phase

  let rank = function
    | Parsing -> 0
    | Naming -> 1
    | Decl -> 2
    | Typing -> 3

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
  Relative_path.Map.merge x y ~f:begin fun k x y ->
    let x = Option.value x ~default:PhaseMap.empty in
    let y = Option.value y ~default:PhaseMap.empty in
    Some (PhaseMap.merge x y ~f:(fun _k x y -> f k x y))
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
    let result = f1 () in
    let errors = !error_map in
    error_map := error_map_copy;
    accumulate_errors := accumulate_errors_copy;
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
    let result = f () in
    let out_errors = !error_map in
    let out_applied_fixmes = !applied_fixmes in
    error_map := error_map_copy;
    applied_fixmes := applied_fixmes_copy;
    accumulate_errors := accumulate_errors_copy;
    let out_errors = files_t_map ~f:(List.rev) out_errors in
    (out_errors, out_applied_fixmes), result

  let run_in_context path phase f =
    let context_copy = !current_context in
    current_context := (path, phase);
    let res = f () in
    current_context := context_copy;
    res

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
  val to_list: 'a error_ -> 'a message list
  val to_absolute : error -> Pos.absolute error_

  val to_string : ?indent:bool -> Pos.absolute error_ -> string

  val get_sorted_error_list: error files_t * applied_fixme files_t -> error list
  val sort: error list -> error list

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
    let result = f () in
    in_lazy_decl := old_in_lazy_decl;
    result

  and make_error code (x: (Pos.t * string) list) = ((code, x): error)

  (*****************************************************************************)
  (* Accessors. *)
  (*****************************************************************************)

  and get_code (error: 'a error_) = ((fst error): error_code)
  let get_pos (error : error) = fst (List.hd_exn (snd error))
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
    let result = f () in
    in_lazy_decl := None;
    result


  let make_error code (x: (Pos.t * string) list) =
    let bt = Printexc.get_callstack 25 in
    ((bt, code, x): error)

  let get_code ((_, c, _): 'a error_) = c

  let get_pos ((_, _, msg_l): error) =
    fst (List.hd_exn msg_l)

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

end

(** The Errors functor which produces the Errors module.
 * Omitting gratuitous indentation. *)
module Errors_with_mode(M: Errors_modes) = struct


(*****************************************************************************)
(* Types *)
(*****************************************************************************)

type 'a error_ = 'a M.error_
type error = Pos.t error_
type applied_fixme = M.applied_fixme
type t = error files_t * applied_fixme files_t

type phase = error_phase = Parsing | Naming | Decl | Typing

(*****************************************************************************)
(* HH_FIXMEs hook *)
(*****************************************************************************)

let applied_fixmes = M.applied_fixmes
let error_map = M.error_map

let ignored_fixme_files = ref None
let set_ignored_fixmes files = ignored_fixme_files := files

let ignored_fixme_codes = ref ISet.empty

let is_in_ignored_file pos = match !ignored_fixme_files with
(* No fixme is ignored *)
| None -> false
(* Only the fixmes in gives files are ignored *)
| Some l ->
  List.exists l ~f:(fun x -> x = (Pos.filename pos))

let is_ignored_code code = ISet.mem code !ignored_fixme_codes

let is_ignored_fixme pos code =
  is_in_ignored_file pos || is_ignored_code code

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
  let applied_fixmes_list = Common.get_current_list !applied_fixmes in
  Common.set_current_list applied_fixmes ((pos, code) :: applied_fixmes_list)

let rec add_error = M.add_error

and add code pos msg =
  if not (is_ignored_fixme pos code) && !is_hh_fixme pos code
  then add_applied_fixme code pos
  else add_error (M.make_error code [pos, msg]);
  add_ignored_fixme_code_error pos code

and add_list code pos_msg_l =
  let pos = fst (List.hd_exn pos_msg_l) in
  if not (is_ignored_fixme pos code) && !is_hh_fixme pos code
  then add_applied_fixme code pos
  else add_error (make_error code pos_msg_l);
  add_ignored_fixme_code_error pos code

and merge (err',fixmes') (err,fixmes) =
  let append = fun _ x y ->
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
  let res = files_t_merge old new_ ~f:begin fun path old new_ ->
    if path = Relative_path.default then
      Utils.assert_false_log_backtrace (Some(
        "Default (untracked) error sources should not get into incremental " ^
        "mode. There might be a missing call to Errors.do_with_context/" ^
        "run_in_context somwhere or incorrectly used Errors.from_error_list"
      ));
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

and get_error_list (err, _fixmes) = files_t_to_list err
and get_applied_fixmes (_err, fixmes) = files_t_to_list fixmes
and from_error_list err = (list_to_files_t err, Relative_path.Map.empty)

(*****************************************************************************)
(* Accessors. (All methods delegated to the parameterized module.) *)
(*****************************************************************************)

and get_code = M.get_code
and get_pos = M.get_pos
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

(*****************************************************************************)
(* Error codes.
 * Each error has a unique number associated with it. The following modules
 * define the error code associated with each kind of error.
 * It is ok to extend the codes with new values, it is NOT OK to change the
 * value of an existing error to a different error code!
 * I added some comments to make that extra clear :-)
 *)
(*****************************************************************************)

(* Errors in the Temporary range are for errors that will disappear in the
 * future. *)
module Temporary = struct
  let darray_not_supported = 1
  let varray_not_supported = 2
  (* DEPRECATED let unknown_fields_not_supported = 3 *)
  let varray_or_darray_not_supported = 4
  (* DEPRECATED let goto_not_supported = 5 *)
  let nonnull_not_supported = 6
end

module Parsing = struct
  let fixme_format                          = 1001 (* DONT MODIFY!!!! *)
  let parsing_error                         = 1002 (* DONT MODIFY!!!! *)
  let unexpected_eof                        = 1003 (* DONT MODIFY!!!! *)
  let unterminated_comment                  = 1004 (* DONT MODIFY!!!! *)
  let unterminated_xhp_comment              = 1005 (* DONT MODIFY!!!! *)
  (* DEPRECATED let call_time_pass_by_reference = 1006 *)
  (* EXTEND HERE WITH NEW VALUES IF NEEDED *)
end

module Naming                               = struct
  let add_a_typehint                        = 2001 (* DONT MODIFY!!!! *)
  let typeparam_alok                        = 2002 (* DONT MODIFY!!!! *)
  let assert_arity                          = 2003 (* DONT MODIFY!!!! *)
  let primitive_invalid_alias               = 2004 (* DONT MODIFY!!!! *)
  (* DECPRECATED let cyclic_constraint      = 2005 *)
  let did_you_mean_naming                   = 2006 (* DONT MODIFY!!!! *)
  let different_scope                       = 2007 (* DONT MODIFY!!!! *)
  let disallowed_xhp_type                   = 2008 (* DONT MODIFY!!!! *)
  (* DEPRECATED let double_instead_of_float = 2009 *)
  (* DEPRECATED let dynamic_class           = 2010 *)
  let dynamic_method_call                   = 2011 (* DONT MODIFY!!!! *)
  let error_name_already_bound              = 2012 (* DONT MODIFY!!!! *)
  let expected_collection                   = 2013 (* DONT MODIFY!!!! *)
  let expected_variable                     = 2014 (* DONT MODIFY!!!! *)
  let fd_name_already_bound                 = 2015 (* DONT MODIFY!!!! *)
  let gen_array_rec_arity                   = 2016 (* DONT MODIFY!!!! *)
  (* let gen_array_va_rec_arity             = 2017 *)
  let gena_arity                            = 2018 (* DONT MODIFY!!!! *)
  let generic_class_var                     = 2019 (* DONT MODIFY!!!! *)
  let genva_arity                           = 2020 (* DONT MODIFY!!!! *)
  let illegal_CLASS                         = 2021 (* DONT MODIFY!!!! *)
  let illegal_class_meth                    = 2022 (* DONT MODIFY!!!! *)
  let illegal_constant                      = 2023 (* DONT MODIFY!!!! *)
  let illegal_fun                           = 2024 (* DONT MODIFY!!!! *)
  let illegal_inst_meth                     = 2025 (* DONT MODIFY!!!! *)
  let illegal_meth_caller                   = 2026 (* DONT MODIFY!!!! *)
  let illegal_meth_fun                      = 2027 (* DONT MODIFY!!!! *)
  (* DEPRECATED integer_instead_of_int      = 2028 *)
  let invalid_req_extends                   = 2029 (* DONT MODIFY!!!! *)
  let invalid_req_implements                = 2030 (* DONT MODIFY!!!! *)
  let local_const                           = 2031 (* DONT MODIFY!!!! *)
  let lowercase_this                        = 2032 (* DONT MODIFY!!!! *)
  let method_name_already_bound             = 2033 (* DONT MODIFY!!!! *)
  let missing_arrow                         = 2034 (* DONT MODIFY!!!! *)
  let missing_typehint                      = 2035 (* DONT MODIFY!!!! *)
  let name_already_bound                    = 2036 (* DONT MODIFY!!!! *)
  let naming_too_few_arguments              = 2037 (* DONT MODIFY!!!! *)
  let naming_too_many_arguments             = 2038 (* DONT MODIFY!!!! *)
  let primitive_toplevel                    = 2039 (* DONT MODIFY!!!! *)
  (* DEPRECATED let real_instead_of_float   = 2040 *)
  let shadowed_type_param                   = 2041 (* DONT MODIFY!!!! *)
  let start_with_T                          = 2042 (* DONT MODIFY!!!! *)
  let this_must_be_return                   = 2043 (* DONT MODIFY!!!! *)
  let this_no_argument                      = 2044 (* DONT MODIFY!!!! *)
  let this_hint_outside_class               = 2045 (* DONT MODIFY!!!! *)
  let this_reserved                         = 2046 (* DONT MODIFY!!!! *)
  let tparam_with_tparam                    = 2047 (* DONT MODIFY!!!! *)
  let typedef_constraint                    = 2048 (* DONT MODIFY!!!! *)
  let unbound_name                          = 2049 (* DONT MODIFY!!!! *)
  let undefined                             = 2050 (* DONT MODIFY!!!! *)
  let unexpected_arrow                      = 2051 (* DONT MODIFY!!!! *)
  let unexpected_typedef                    = 2052 (* DONT MODIFY!!!! *)
  let using_internal_class                  = 2053 (* DONT MODIFY!!!! *)
  let void_cast                             = 2054 (* DONT MODIFY!!!! *)
  let object_cast                           = 2055 (* DONT MODIFY!!!! *)
  let unset_cast                            = 2056 (* DONT MODIFY!!!! *)
  (* DEPRECATED let nullsafe_property_access = 2057 *)
  let illegal_TRAIT                         = 2058 (* DONT MODIFY!!!! *)
  (* DEPRECATED let shape_typehint          = 2059  *)
  let dynamic_new_in_strict_mode            = 2060 (* DONT MODIFY!!!! *)
  let invalid_type_access_root              = 2061 (* DONT MODIFY!!!! *)
  let duplicate_user_attribute              = 2062 (* DONT MODIFY!!!! *)
  let return_only_typehint                  = 2063 (* DONT MODIFY!!!! *)
  let unexpected_type_arguments             = 2064 (* DONT MODIFY!!!! *)
  let too_many_type_arguments               = 2065 (* DONT MODIFY!!!! *)
  let classname_param                       = 2066 (* DONT MODIFY!!!! *)
  let invalid_instanceof                    = 2067 (* DONT MODIFY!!!! *)
  let name_is_reserved                      = 2068 (* DONT MODIFY!!!! *)
  let dollardollar_unused                   = 2069 (* DONT MODIFY!!!! *)
  let illegal_member_variable_class         = 2070 (* DONT MODIFY!!!! *)
  let too_few_type_arguments                = 2071 (* DONT MODIFY!!!! *)
  let goto_label_already_defined            = 2072 (* DONT MODIFY!!!! *)
  let goto_label_undefined                  = 2073 (* DONT MODIFY!!!! *)
  let goto_label_defined_in_finally         = 2074 (* DONT MODIFY!!!! *)
  let goto_invoked_in_finally               = 2075 (* DONT MODIFY!!!! *)
  let dynamic_class_property_name_in_strict_mode  = 2076 (* DONT MODIFY!!!! *)
  let this_as_lexical_variable              = 2077 (* DONT MODIFY!!!! *)
  let dynamic_class_name_in_strict_mode     = 2078 (* DONT MODIFY!!!! *)
  let xhp_optional_required_attr            = 2079 (* DONT MODIFY!!!! *)
  let xhp_required_with_default             = 2080 (* DONT MODIFY!!!! *)
  let variable_variables_disallowed         = 2081 (* DONT MODIFY!!!! *)
  (* EXTEND HERE WITH NEW VALUES IF NEEDED *)
end

module NastCheck                            = struct
  let abstract_body                         = 3001 (* DONT MODIFY!!!! *)
  let abstract_with_body                    = 3002 (* DONT MODIFY!!!! *)
  let await_in_sync_function                = 3003 (* DONT MODIFY!!!! *)
  let call_before_init                      = 3004 (* DONT MODIFY!!!! *)
  let case_fallthrough                      = 3005 (* DONT MODIFY!!!! *)
  let continue_in_switch                    = 3006 (* DONT MODIFY!!!! *)
  let dangerous_method_name                 = 3007 (* DONT MODIFY!!!! *)
  let default_fallthrough                   = 3008 (* DONT MODIFY!!!! *)
  let interface_with_member_variable        = 3009 (* DONT MODIFY!!!! *)
  let interface_with_static_member_variable = 3010 (* DONT MODIFY!!!! *)
  let magic                                 = 3011 (* DONT MODIFY!!!! *)
  let no_construct_parent                   = 3012 (* DONT MODIFY!!!! *)
  let non_interface                         = 3013 (* DONT MODIFY!!!! *)
  let not_abstract_without_body             = 3014 (* DONT MODIFY!!!! *)
  let not_initialized                       = 3015 (* DONT MODIFY!!!! *)
  let not_public_interface                  = 3016 (* DONT MODIFY!!!! *)
  let requires_non_class                    = 3017 (* DONT MODIFY!!!! *)
  let return_in_finally                     = 3018 (* DONT MODIFY!!!! *)
  let return_in_gen                         = 3019 (* DONT MODIFY!!!! *)
  let toString_returns_string               = 3020 (* DONT MODIFY!!!! *)
  let toString_visibility                   = 3021 (* DONT MODIFY!!!! *)
  let toplevel_break                        = 3022 (* DONT MODIFY!!!! *)
  let toplevel_continue                     = 3023 (* DONT MODIFY!!!! *)
  let uses_non_trait                        = 3024 (* DONT MODIFY!!!! *)
  let illegal_function_name                 = 3025 (* DONT MODIFY!!!! *)
  let not_abstract_without_typeconst        = 3026 (* DONT MODIFY!!!! *)
  let typeconst_depends_on_external_tparam  = 3027 (* DONT MODIFY!!!! *)
  let typeconst_assigned_tparam             = 3028 (* DONT MODIFY!!!! *)
  let abstract_with_typeconst               = 3029 (* DONT MODIFY!!!! *)
  let constructor_required                  = 3030 (* DONT MODIFY!!!! *)
  let interface_with_partial_typeconst      = 3031 (* DONT MODIFY!!!! *)
  let multiple_xhp_category                 = 3032 (* DONT MODIFY!!!! *)
  (* DEPRECATED
     let optional_shape_fields_not_supported   = 3033 (* DONT MODIFY!!!! *) *)
  let await_not_allowed                     = 3034 (* DONT MODIFY!!!! *)
  let async_in_interface                    = 3035 (* DONT MODIFY!!!! *)
  let await_in_coroutine                    = 3036 (* DONT MODIFY!!!! *)
  let yield_in_coroutine                    = 3037 (* DONT MODIFY!!!! *)
  let suspend_outside_of_coroutine          = 3038 (* DONT MODIFY!!!! *)
  let suspend_in_finally                    = 3039 (* DONT MODIFY!!!! *)
  let break_continue_n_not_supported        = 3040 (* DONT MODIFY!!!! *)
  let static_memoized_function              = 3041 (* DONT MODIFY!!!! *)
  let inout_params_outside_of_sync          = 3042 (* DONT MODIFY!!!! *)
  let inout_params_special                  = 3043 (* DONT MODIFY!!!! *)
  let inout_params_mix_byref                = 3044 (* DONT MODIFY!!!! *)
  let inout_params_memoize                  = 3045 (* DONT MODIFY!!!! *)
  let inout_params_ret_by_ref               = 3046 (* DONT MODIFY!!!! *)
  let reading_from_append                   = 3047 (* DONT MODIFY!!!! *)
  let const_attribute_prohibited            = 3048 (* DONT MODIFY!!!! *)
  let global_in_reactive_context            = 3049 (* DONT MODIFY!!!! *)
  let inout_argument_bad_expr               = 3050 (* DONT MODIFY!!!! *)
  let mutable_params_outside_of_sync        = 3051 (* DONT MODIFY!!!! *)
  let mutable_async_method                  = 3052 (* DONT MODIFY!!!! *)
  let mutable_methods_must_be_reactive      = 3053(* DONT MODIFY!!!! *)
  let mutable_attribute_on_function         = 3054 (* DONT MODIFY!!!! *)
  let mutable_return_annotated_decls_must_be_reactive = 3055 (* DONT MODIFY!!!! *)
  let illegal_destructor                    = 3056 (* DONT MODIFY!!!! *)
  let conditionally_reactive_function       = 3057 (* DONT MODIFY!!!! *)
  let multiple_conditionally_reactive_annotations = 3058 (* DONT MODIFY!!!! *)
  let conditionally_reactive_annotation_invalid_arguments = 3059 (* DONT MODIFY!!!! *)
  let conflicting_reactive_annotations      = 3040 (* DONT MODIFY!!!! *)

  (* EXTEND HERE WITH NEW VALUES IF NEEDED *)
end

module Typing                               = struct
  (* let abstract_class_final                  = 4001 (\* DONT MODIFY!!!! *\) *)
  let uninstantiable_class                  = 4002 (* DONT MODIFY!!!! *)
  let anonymous_recursive                   = 4003 (* DONT MODIFY!!!! *)
  let anonymous_recursive_call              = 4004 (* DONT MODIFY!!!! *)
  let array_access                          = 4005 (* DONT MODIFY!!!! *)
  let array_append                          = 4006 (* DONT MODIFY!!!! *)
  let array_cast                            = 4007 (* DONT MODIFY!!!! *)
  let array_get_arity                       = 4008 (* DONT MODIFY!!!! *)
  let bad_call                              = 4009 (* DONT MODIFY!!!! *)
  let class_arity                           = 4010 (* DONT MODIFY!!!! *)
  let const_mutation                        = 4011 (* DONT MODIFY!!!! *)
  let constructor_no_args                   = 4012 (* DONT MODIFY!!!! *)
  let cyclic_class_def                      = 4013 (* DONT MODIFY!!!! *)
  let cyclic_typedef                        = 4014 (* DONT MODIFY!!!! *)
  let discarded_awaitable                   = 4015 (* DONT MODIFY!!!! *)
  let isset_empty_in_strict                 = 4016 (* DONT MODIFY!!!! *)
  (* DEPRECATED dynamic_yield_private       = 4017 *)
  let enum_constant_type_bad                = 4018 (* DONT MODIFY!!!! *)
  let enum_switch_nonexhaustive             = 4019 (* DONT MODIFY!!!! *)
  let enum_switch_not_const                 = 4020 (* DONT MODIFY!!!! *)
  let enum_switch_redundant                 = 4021 (* DONT MODIFY!!!! *)
  let enum_switch_redundant_default         = 4022 (* DONT MODIFY!!!! *)
  let enum_switch_wrong_class               = 4023 (* DONT MODIFY!!!! *)
  let enum_type_bad                         = 4024 (* DONT MODIFY!!!! *)
  let enum_type_typedef_mixed               = 4025 (* DONT MODIFY!!!! *)
  let expected_class                        = 4026 (* DONT MODIFY!!!! *)
  let expected_literal_string               = 4027 (* DONT MODIFY!!!! *)
  (* DEPRECATED expected_static_int         = 4028 *)
  let expected_tparam                       = 4029 (* DONT MODIFY!!!! *)
  let expecting_return_type_hint            = 4030 (* DONT MODIFY!!!! *)
  let expecting_return_type_hint_suggest    = 4031 (* DONT MODIFY!!!! *)
  let expecting_type_hint                   = 4032 (* DONT MODIFY!!!! *)
  let expecting_type_hint_suggest           = 4033 (* DONT MODIFY!!!! *)
  let extend_final                          = 4035 (* DONT MODIFY!!!! *)
  let field_kinds                           = 4036 (* DONT MODIFY!!!! *)
  (* DEPRECATED field_missing               = 4037 *)
  let format_string                         = 4038 (* DONT MODIFY!!!! *)
  let fun_arity_mismatch                    = 4039 (* DONT MODIFY!!!! *)
  let fun_too_few_args                      = 4040 (* DONT MODIFY!!!! *)
  let fun_too_many_args                     = 4041 (* DONT MODIFY!!!! *)
  let fun_unexpected_nonvariadic            = 4042 (* DONT MODIFY!!!! *)
  let fun_variadicity_hh_vs_php56           = 4043 (* DONT MODIFY!!!! *)
  let gena_expects_array                    = 4044 (* DONT MODIFY!!!! *)
  let generic_array_strict                  = 4045 (* DONT MODIFY!!!! *)
  let generic_static                        = 4046 (* DONT MODIFY!!!! *)
  let implement_abstract                    = 4047 (* DONT MODIFY!!!! *)
  let interface_final                       = 4048 (* DONT MODIFY!!!! *)
  let invalid_shape_field_const             = 4049 (* DONT MODIFY!!!! *)
  let invalid_shape_field_literal           = 4050 (* DONT MODIFY!!!! *)
  let invalid_shape_field_name              = 4051 (* DONT MODIFY!!!! *)
  let invalid_shape_field_type              = 4052 (* DONT MODIFY!!!! *)
  let member_not_found                      = 4053 (* DONT MODIFY!!!! *)
  let member_not_implemented                = 4054 (* DONT MODIFY!!!! *)
  let missing_assign                        = 4055 (* DONT MODIFY!!!! *)
  let missing_constructor                   = 4056 (* DONT MODIFY!!!! *)
  let missing_field                         = 4057 (* DONT MODIFY!!!! *)
  (* DEPRECATED negative_tuple_index        = 4058 *)
  let self_outside_class                    = 4059 (* DONT MODIFY!!!! *)
  let new_static_inconsistent               = 4060 (* DONT MODIFY!!!! *)
  let static_outside_class                  = 4061 (* DONT MODIFY!!!! *)
  let non_object_member                     = 4062 (* DONT MODIFY!!!! *)
  let null_container                        = 4063 (* DONT MODIFY!!!! *)
  let null_member                           = 4064 (* DONT MODIFY!!!! *)
  (*let nullable_parameter                    = 4065 *)
  let option_return_only_typehint           = 4066 (* DONT MODIFY!!!! *)
  let object_string                         = 4067 (* DONT MODIFY!!!! *)
  let option_mixed                          = 4068 (* DONT MODIFY!!!! *)
  let overflow                              = 4069 (* DONT MODIFY!!!! *)
  let override_final                        = 4070 (* DONT MODIFY!!!! *)
  let override_per_trait                    = 4071 (* DONT MODIFY!!!! *)
  let pair_arity                            = 4072 (* DONT MODIFY!!!! *)
  let abstract_call                         = 4073 (* DONT MODIFY!!!! *)
  let parent_in_trait                       = 4074 (* DONT MODIFY!!!! *)
  let parent_outside_class                  = 4075 (* DONT MODIFY!!!! *)
  let parent_undefined                      = 4076 (* DONT MODIFY!!!! *)
  let previous_default                      = 4077 (* DONT MODIFY!!!! *)
  let private_class_meth                    = 4078 (* DONT MODIFY!!!! *)
  let private_inst_meth                     = 4079 (* DONT MODIFY!!!! *)
  let private_override                      = 4080 (* DONT MODIFY!!!! *)
  let protected_class_meth                  = 4081 (* DONT MODIFY!!!! *)
  let protected_inst_meth                   = 4082 (* DONT MODIFY!!!! *)
  let read_before_write                     = 4083 (* DONT MODIFY!!!! *)
  let return_in_void                        = 4084 (* DONT MODIFY!!!! *)
  let shape_field_class_mismatch            = 4085 (* DONT MODIFY!!!! *)
  let shape_field_type_mismatch             = 4086 (* DONT MODIFY!!!! *)
  let should_be_override                    = 4087 (* DONT MODIFY!!!! *)
  let sketchy_null_check                    = 4088 (* DONT MODIFY!!!! *)
  let sketchy_null_check_primitive          = 4089 (* DONT MODIFY!!!! *)
  let smember_not_found                     = 4090 (* DONT MODIFY!!!! *)
  let static_dynamic                        = 4091 (* DONT MODIFY!!!! *)
  (* DEPRECATED let static_overflow         = 4092 *)
  let this_in_static                        = 4094 (* DONT MODIFY!!!! *)
  let this_var_outside_class                = 4095 (* DONT MODIFY!!!! *)
  let trait_final                           = 4096 (* DONT MODIFY!!!! *)
  let tuple_arity                           = 4097 (* DONT MODIFY!!!! *)
  let tuple_arity_mismatch                  = 4098 (* DONT MODIFY!!!! *)
  (* DEPRECATED tuple_index_too_large       = 4099 *)
  let tuple_syntax                          = 4100 (* DONT MODIFY!!!! *)
  let type_arity_mismatch                   = 4101 (* DONT MODIFY!!!! *)
  let type_param_arity                      = 4102 (* DONT MODIFY!!!! *)
  let typing_too_few_args                   = 4104 (* DONT MODIFY!!!! *)
  let typing_too_many_args                  = 4105 (* DONT MODIFY!!!! *)
  let unbound_global                        = 4106 (* DONT MODIFY!!!! *)
  let unbound_name_typing                   = 4107 (* DONT MODIFY!!!! *)
  let undefined_field                       = 4108 (* DONT MODIFY!!!! *)
  let undefined_parent                      = 4109 (* DONT MODIFY!!!! *)
  let unify_error                           = 4110 (* DONT MODIFY!!!! *)
  let unsatisfied_req                       = 4111 (* DONT MODIFY!!!! *)
  let visibility                            = 4112 (* DONT MODIFY!!!! *)
  let visibility_extends                    = 4113 (* DONT MODIFY!!!! *)
  (* DEPRECATED void_parameter              = 4114 *)
  let wrong_extend_kind                     = 4115 (* DONT MODIFY!!!! *)
  let generic_unify                         = 4116 (* DONT MODIFY!!!! *)
  let nullsafe_not_needed                   = 4117 (* DONT MODIFY!!!! *)
  let trivial_strict_eq                     = 4118 (* DONT MODIFY!!!! *)
  let void_usage                            = 4119 (* DONT MODIFY!!!! *)
  let declared_covariant                    = 4120 (* DONT MODIFY!!!! *)
  let declared_contravariant                = 4121 (* DONT MODIFY!!!! *)
  (* DEPRECATED unset_in_strict             = 4122 *)
  let strict_members_not_known              = 4123 (* DONT MODIFY!!!! *)
  let generic_at_runtime                    = 4124 (* DONT MODIFY!!!! *)
  (*let dynamic_class                       = 4125 *)
  let attribute_too_many_arguments          = 4126 (* DONT MODIFY!!!! *)
  let attribute_param_type                  = 4127 (* DONT MODIFY!!!! *)
  let deprecated_use                        = 4128 (* DONT MODIFY!!!! *)
  let abstract_const_usage                  = 4129 (* DONT MODIFY!!!! *)
  let cannot_declare_constant               = 4130 (* DONT MODIFY!!!! *)
  let cyclic_typeconst                      = 4131 (* DONT MODIFY!!!! *)
  let nullsafe_property_write_context       = 4132 (* DONT MODIFY!!!! *)
  let noreturn_usage                        = 4133 (* DONT MODIFY!!!! *)
  let this_lvalue                           = 4134 (* DONT MODIFY!!!! *)
  let unset_nonidx_in_strict                = 4135 (* DONT MODIFY!!!! *)
  let invalid_shape_field_name_empty        = 4136 (* DONT MODIFY!!!! *)
  let invalid_shape_field_name_number       = 4137 (* DONT MODIFY!!!! *)
  let shape_fields_unknown                  = 4138 (* DONT MODIFY!!!! *)
  let invalid_shape_remove_key              = 4139 (* DONT MODIFY!!!! *)
  let missing_optional_field                = 4140 (* DONT MODIFY!!!! *)
  let shape_field_unset                     = 4141 (* DONT MODIFY!!!! *)
  let abstract_concrete_override            = 4142 (* DONT MODIFY!!!! *)
  let local_variable_modifed_and_used       = 4143 (* DONT MODIFY!!!! *)
  let local_variable_modifed_twice          = 4144 (* DONT MODIFY!!!! *)
  let assign_during_case                    = 4145 (* DONT MODIFY!!!! *)
  let cyclic_enum_constraint                = 4146 (* DONT MODIFY!!!! *)
  let unpacking_disallowed                  = 4147 (* DONT MODIFY!!!! *)
  let invalid_classname                     = 4148 (* DONT MODIFY!!!! *)
  let invalid_memoized_param                = 4149 (* DONT MODIFY!!!! *)
  let illegal_type_structure                = 4150 (* DONT MODIFY!!!! *)
  let not_nullable_compare_null_trivial     = 4151 (* DONT MODIFY!!!! *)
  let class_property_only_static_literal    = 4152 (* DONT MODIFY!!!! *)
  let attribute_too_few_arguments           = 4153 (* DONT MODIFY!!!! *)
  let reference_expr                        = 4154 (* DONT MODIFY!!!! *)
  let unification_cycle                     = 4155 (* DONT MODIFY!!!! *)
  let keyset_set                            = 4156 (* DONT MODIFY!!!! *)
  let eq_incompatible_types                 = 4157 (* DONT MODIFY!!!! *)
  let contravariant_this                    = 4158 (* DONT MODIFY!!!! *)
  let instanceof_always_false               = 4159 (* DONT MODIFY!!!! *)
  let instanceof_always_true                = 4160 (* DONT MODIFY!!!! *)
  let ambiguous_member                      = 4161 (* DONT MODIFY!!!! *)
  let instanceof_generic_classname          = 4162 (* DONT MODIFY!!!! *)
  let required_field_is_optional            = 4163 (* DONT MODIFY!!!! *)
  let final_property                        = 4164 (* DONT MODIFY!!!! *)
  let array_get_with_optional_field         = 4165 (* DONT MODIFY!!!! *)
  let unknown_field_disallowed_in_shape     = 4166 (* DONT MODIFY!!!! *)
  let nullable_cast                         = 4167 (* DONT MODIFY!!!! *)
  let pass_by_ref_annotation_missing        = 4168 (* DONT MODIFY!!!! *)
  let non_call_argument_in_suspend          = 4169 (* DONT MODIFY!!!! *)
  let non_coroutine_call_in_suspend         = 4170 (* DONT MODIFY!!!! *)
  let coroutine_call_outside_of_suspend     = 4171 (* DONT MODIFY!!!! *)
  let function_is_not_coroutine             = 4172 (* DONT MODIFY!!!! *)
  let coroutinness_mismatch                 = 4173 (* DONT MODIFY!!!! *)
  let expecting_awaitable_return_type_hint  = 4174 (* DONT MODIFY!!!! *)
  let reffiness_invariant                   = 4175 (* DONT MODIFY!!!! *)
  let dollardollar_lvalue                   = 4176 (* DONT MODIFY!!!! *)
  (* DEPRECATED static_method_on_interface = 4177 *)
  let duplicate_using_var                   = 4178 (* DONT MODIFY!!!! *)
  let illegal_disposable                    = 4179 (* DONT MODIFY!!!! *)
  let escaping_disposable                   = 4180 (* DONT MODIFY!!!! *)
  let pass_by_ref_annotation_unexpected     = 4181 (* DONT MODIFY!!!! *)
  let inout_annotation_missing              = 4182 (* DONT MODIFY!!!! *)
  let inout_annotation_unexpected           = 4183 (* DONT MODIFY!!!! *)
  let inoutness_mismatch                    = 4184 (* DONT MODIFY!!!! *)
  let static_synthetic_method               = 4185 (* DONT MODIFY!!!! *)
  let trait_reuse                           = 4186 (* DONT MODIFY!!!! *)
  let invalid_new_disposable                = 4187 (* DONT MODIFY!!!! *)
  let escaping_disposable_parameter         = 4188 (* DONT MODIFY!!!! *)
  let accept_disposable_invariant           = 4189 (* DONT MODIFY!!!! *)
  let invalid_disposable_hint               = 4190 (* DONT MODIFY!!!! *)
  let xhp_required                          = 4191 (* DONT MODIFY!!!! *)
  let escaping_this                         = 4192 (* DONT MODIFY!!!! *)
  let illegal_xhp_child                     = 4193 (* DONT MODIFY!!!! *)
  let must_extend_disposable                = 4194 (* DONT MODIFY!!!! *)
  let invalid_is_expression_hint            = 4195 (* DONT MODIFY!!!! *)
  let assigning_to_const                    = 4196 (* DONT MODIFY!!!! *)
  let self_const_parent_not                 = 4197 (* DONT MODIFY!!!! *)
  let parent_const_self_not                 = 4198 (* DONT MODIFY!!!! *)
  let partially_valid_is_expression_hint    = 4199 (* DONT MODIFY!!!! *)
  let nonreactive_function_call             = 4200 (* DONT MODIFY!!!! *)
  let nonreactive_append                    = 4201 (* DONT MODIFY!!!! *)
  let obj_set_reactive                      = 4202 (* DONT MODIFY!!!! *)
  let fun_reactivity_mismatch               = 4203 (* DONT MODIFY!!!! *)
  let overriding_prop_const_mismatch        = 4204 (* DONT MODIFY!!!! *)
  let invalid_return_disposable             = 4205 (* DONT MODIFY!!!! *)
  let invalid_disposable_return_hint        = 4206 (* DONT MODIFY!!!! *)
  let return_disposable_mismatch            = 4207 (* DONT MODIFY!!!! *)
  let inout_argument_bad_type               = 4208 (* DONT MODIFY!!!! *)
  let frozen_in_incorrect_scope             = 4209 (* DONT MODIFY!!!! *)
  let reassign_mutable_var                  = 4210 (* DONT MODIFY!!!! *)
  let invalid_freeze_target                 = 4211 (* DONT MODIFY!!!! *)
  let invalid_freeze_use                    = 4212 (* DONT MODIFY!!!! *)
  let freeze_in_nonreactive_context         = 4213 (* DONT MODIFY!!!! *)
  let mutable_call_on_immutable             = 4214 (* DONT MODIFY!!!! *)
  let mutable_argument_mismatch             = 4215 (* DONT MODIFY!!!! *)
  let invalid_mutable_return_result         = 4216 (* DONT MODIFY!!!! *)
  let mutable_return_result_mismatch        = 4217 (* DONT MODIFY!!!! *)
  let nonreactive_call_from_shallow         = 4218 (* DONT MODIFY!!!! *)
  let enum_type_typedef_nonnull             = 4219 (* DONT MODIFy!!!! *)
  let rx_enabled_in_non_rx_context          = 4220 (* DONT MODIFY!!!! *)
  let rx_enabled_in_lambdas                 = 4221 (* DONT MODIFY!!!! *)
  let ambiguous_lambda                      = 4222 (* DONT MODIFY!!!! *)
  let ellipsis_strict_mode                  = 4223 (* DONT MODIFY!!!! *)
  let untyped_lambda_strict_mode            = 4224 (* DONT MODIFY!!!! *)
  let binding_ref_in_array                  = 4225 (* DONT MODIFY!!!! *)
  let invalid_conditionally_reactive_call   = 4225 (* DONT MODIFY!!!! *)
  (* EXTEND HERE WITH NEW VALUES IF NEEDED *)
end

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
  add Parsing.fixme_format pos
    "HH_FIXME wrong format, expected '/* HH_FIXME[ERROR_NUMBER] */'"

let unexpected_eof pos =
  add Parsing.unexpected_eof pos "Unexpected end of file"

let unterminated_comment pos =
  add Parsing.unterminated_comment pos "unterminated comment"

let unterminated_xhp_comment pos =
  add Parsing.unterminated_xhp_comment pos "unterminated xhp comment"

let parsing_error (p, msg) =
  add Parsing.parsing_error p msg

(*****************************************************************************)
(* Naming errors *)
(*****************************************************************************)

let typeparam_alok (pos, x) =
  add Naming.typeparam_alok pos (
  "You probably forgot to bind this type parameter right?\nAdd <"^x^
  "> somewhere (after the function name definition, \
    or after the class name)\nExamples: "^"function foo<T> or class A<T>")

let generic_class_var pos =
  add Naming.generic_class_var pos
    "A class variable cannot be generic"

let unexpected_arrow pos cname =
  add Naming.unexpected_arrow pos (
  "Keys may not be specified for "^cname^" initialization"
 )

let missing_arrow pos cname =
  add Naming.missing_arrow pos (
  "Keys must be specified for "^cname^" initialization"
 )

let disallowed_xhp_type pos name =
  add Naming.disallowed_xhp_type pos (
  name^" is not a valid type. Use :xhp or XHPChild."
 )

let name_already_bound name pos1 pos2 =
  let name = Utils.strip_ns name in
  add_list Naming.name_already_bound [
    pos1, "Name already bound: "^name;
    pos2, "Previous definition is here"
]

let name_is_reserved name pos =
  let name = Utils.strip_all_ns name in
  add Naming.name_is_reserved pos (
  name^" cannot be used as it is reserved."
 )

let dollardollar_unused pos =
  add Naming.dollardollar_unused pos ("This expression does not contain a "^
    "usage of the special pipe variable. Did you forget to use the ($$) "^
    "variable?")

let method_name_already_bound pos name =
  add Naming.method_name_already_bound pos (
  "Method name already bound: "^name
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
  add_list Naming.error_name_already_bound errs

let unbound_name pos name kind =
  let kind_str = match kind with
    | `cls -> "an object type"
    | `func -> "a global function"
    | `const -> "a global constant"
  in
  add Naming.unbound_name pos
    ("Unbound name: "^(strip_ns name)^" ("^kind_str^")")

let different_scope pos var_name pos' =
  add_list Naming.different_scope [
  pos, ("The variable "^ var_name ^" is defined");
  pos', ("But in a different scope")
]

let undefined pos var_name =
  add Naming.undefined pos ("Undefined variable: "^var_name)

let this_reserved pos =
  add Naming.this_reserved pos
    "The type parameter \"this\" is reserved"

let start_with_T pos =
  add Naming.start_with_T pos
    "Please make your type parameter start with the letter T (capital)"

let already_bound pos name =
  add Naming.name_already_bound pos ("Argument already bound: "^name)

let unexpected_typedef pos def_pos =
  add_list Naming.unexpected_typedef [
  pos, "Unexpected typedef";
  def_pos, "Definition is here";
]

let fd_name_already_bound pos =
  add Naming.fd_name_already_bound pos
    "Field name already bound"

let primitive_toplevel pos =
  add Naming.primitive_toplevel pos (
  "Primitive type annotations are always available and may no \
    longer be referred to in the toplevel namespace."
)

let primitive_invalid_alias pos used valid =
  add Naming.primitive_invalid_alias pos
    ("Invalid Hack type. Using '"^used^"' in Hack is considered \
    an error. Use '"^valid^"' instead, to keep the codebase \
    consistent.")

let dynamic_new_in_strict_mode pos =
  add Naming.dynamic_new_in_strict_mode pos
  "Cannot use dynamic new in strict mode"

let invalid_type_access_root (pos, id) =
  add Naming.invalid_type_access_root pos
  (id^" must be an identifier for a class, \"self\", or \"this\"")

let duplicate_user_attribute (pos, name) existing_attr_pos =
  add_list Naming.duplicate_user_attribute [
    pos, "You cannot reuse the attribute "^name;
    existing_attr_pos, name^" was already used here";
  ]

let unbound_attribute_name pos name =
  let reason = if (string_starts_with name "__")
    then "starts with __ but is not a standard attribute"
    else "is not listed in .hhconfig"
  in add Naming.unbound_name pos
    ("Unrecognized user attribute: "^name^" "^reason)

let this_no_argument pos =
  add Naming.this_no_argument pos "\"this\" expects no arguments"

let void_cast pos =
  add Naming.void_cast pos "Cannot cast to void."

let unset_cast pos =
  add Naming.unset_cast pos "Don't use (unset), just assign null!"

let object_cast pos x =
  add Naming.object_cast pos ("Object casts are unsupported. "^
    "Try 'if ($var instanceof "^x^")' or "^
    "'invariant($var instanceof "^x^", ...)'.")

let this_hint_outside_class pos =
   add Naming.this_hint_outside_class pos
    "Cannot use \"this\" outside of a class"

let this_type_forbidden pos =
 add Naming.this_must_be_return pos
    "The type \"this\" cannot be used as a constraint on a class' generic, \
     or as the type of a static member variable"

let lowercase_this pos type_ =
  add Naming.lowercase_this pos (
  "Invalid Hack type \""^type_^"\". Use \"this\" instead"
 )

let classname_param pos =
  add Naming.classname_param pos
    ("Missing type parameter to classname; classname is entirely"
     ^" meaningless without one")

let invalid_instanceof pos =
  add Naming.invalid_instanceof pos
    "This instanceof has an invalid right operand. Only class identifiers, \
    local variables, accesses of objects / classes / arrays, and function / \
    method calls are allowed."

let tparam_with_tparam pos x =
  add Naming.tparam_with_tparam pos (
  Printf.sprintf "%s is a type parameter. Type parameters cannot \
    themselves take type parameters (e.g. %s<int> doesn't make sense)" x x
 )

let shadowed_type_param p pos name =
  add_list Naming.shadowed_type_param [
    p, Printf.sprintf "You cannot re-bind the type parameter %s" name;
    pos, Printf.sprintf "%s is already bound here" name
  ]

let missing_typehint pos =
  add Naming.missing_typehint pos
    "Please add a type hint"

let expected_variable pos =
  add Naming.expected_variable pos
    "Was expecting a variable name"

let clone_too_many_arguments pos =
  add Naming.naming_too_many_arguments pos
    "__clone method cannot take arguments"

let naming_too_few_arguments pos =
  add Naming.naming_too_few_arguments pos
    "Too few arguments"

let naming_too_many_arguments pos =
  add Naming.naming_too_many_arguments pos
    "Too many arguments"

let expected_collection pos cn =
  add Naming.expected_collection pos (
  "Unexpected collection type " ^ (Utils.strip_ns cn)
 )

let illegal_CLASS pos =
  add Naming.illegal_CLASS pos
    "Using __CLASS__ outside a class or trait"

let illegal_TRAIT pos =
  add Naming.illegal_TRAIT pos
    "Using __TRAIT__ outside a trait"

let dynamic_method_call pos =
  add Naming.dynamic_method_call pos
    "Dynamic method call"

let nullsafe_property_write_context pos =
  add Typing.nullsafe_property_write_context pos
  "?-> syntax not supported here, this function effectively does a write"

let illegal_fun pos =
  let msg = "The argument to fun() must be a single-quoted, constant "^
    "literal string representing a valid function name." in
  add Naming.illegal_fun pos msg

let illegal_member_variable_class pos =
  let msg = "Cannot declare a constant named 'class'. \
             The name 'class' is reserved for the class \
             constant that represents the name of the class" in
  add Naming.illegal_member_variable_class pos msg

let illegal_meth_fun pos =
  let msg = "String argument to fun() contains ':';"^
    " for static class methods, use"^
    " class_meth(Cls::class, 'method_name'), not fun('Cls::method_name')" in
  add Naming.illegal_meth_fun pos msg

let illegal_inst_meth pos =
  let msg = "The argument to inst_meth() must be an expression and a "^
    "constant literal string representing a valid method name." in
  add Naming.illegal_inst_meth pos msg

let illegal_meth_caller pos =
  let msg =
    "The two arguments to meth_caller() must be:"
    ^"\n - first: ClassOrInterface::class"
    ^"\n - second: a single-quoted string literal containing the name"
    ^" of a non-static method of that class" in
  add Naming.illegal_meth_caller pos msg

let illegal_class_meth pos =
  let msg =
    "The two arguments to class_meth() must be:"
    ^"\n - first: ValidClassname::class"
    ^"\n - second: a single-quoted string literal containing the name"
    ^" of a static method of that class" in
  add Naming.illegal_class_meth pos msg

let assert_arity pos =
  add Naming.assert_arity pos
    "assert expects exactly one argument"

let gena_arity pos =
  add Naming.gena_arity pos
    "gena() expects exactly 1 argument"

let genva_arity pos =
  add Naming.genva_arity pos
    "genva() expects at least 1 argument"

let gen_array_rec_arity pos =
  add Naming.gen_array_rec_arity pos
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
  add_list Typing.uninstantiable_class msgl

let abstract_const_usage usage_pos decl_pos name =
  let name = strip_ns name in
  add_list Typing.abstract_const_usage [
    usage_pos, ("Cannot reference abstract constant "^name^" directly");
    decl_pos, "Declaration is here"
  ]

let typedef_constraint pos =
  add Naming.typedef_constraint pos
    "Constraints on typedefs are not supported"

let add_a_typehint pos =
  add Naming.add_a_typehint pos
    "Please add a type hint"

let local_const var_pos =
  add Naming.local_const var_pos
    "You cannot use a local variable in a constant definition"

let illegal_constant pos =
  add Naming.illegal_constant pos
    "Illegal constant value"

let invalid_req_implements pos =
  add Naming.invalid_req_implements pos
    "Only traits may use 'require implements'"

let invalid_req_extends pos =
  add Naming.invalid_req_extends pos
    "Only traits and interfaces may use 'require extends'"

let did_you_mean_naming pos name suggest_pos suggest_name =
  add_list Naming.did_you_mean_naming [
  pos, "Could not find "^(strip_ns name);
  suggest_pos, "Did you mean "^(strip_ns suggest_name)^"?"
]

let using_internal_class pos name =
  add Naming.using_internal_class pos (
  name^" is an implementation internal class that cannot be used directly"
 )

 let too_few_type_arguments p =
   add Naming.too_few_type_arguments p
     ("Too few type arguments for this type")

let goto_label_already_defined
    label_name
    redeclaration_pos
    original_delcaration_pos =
  add_list
    Naming.goto_label_already_defined
    [
      redeclaration_pos, "Cannot redeclare the goto label '" ^ label_name ^ "'";
      original_delcaration_pos, "Declaration is here";
    ]

let goto_label_undefined pos label_name =
  add Naming.goto_label_undefined pos ("Undefined goto label: " ^ label_name)

let goto_label_defined_in_finally pos =
  add Naming.goto_label_defined_in_finally
    pos
    "It is illegal to define a goto label within a finally block."

let goto_invoked_in_finally pos =
  add Naming.goto_invoked_in_finally
    pos
    "It is illegal to invoke goto within a finally block."

let dynamic_class_property_name_in_strict_mode pos =
  add Naming.dynamic_class_property_name_in_strict_mode
    pos
    "Cannot use dynamic class property name in strict mode"

let dynamic_class_name_in_strict_mode pos =
  add Naming.dynamic_class_name_in_strict_mode
    pos
    "Cannot use dynamic class name in strict mode"

let xhp_optional_required_attr pos id =
  add Naming.xhp_optional_required_attr
    pos
    ("XHP attribute " ^ id ^ " cannot be marked as nullable and required")

let xhp_required_with_default pos id =
  add Naming.xhp_required_with_default
    pos
    ("XHP attribute " ^ id ^ " cannot be marked as required and provide a default")

let variable_variables_disallowed pos =
  add Naming.variable_variables_disallowed pos
    "Variable variables are not legal; all variable identifiers must be static strings."

(*****************************************************************************)
(* Init check errors *)
(*****************************************************************************)

let no_construct_parent pos =
  add NastCheck.no_construct_parent pos (
  sl["You are extending a class that needs to be initialized\n";
     "Make sure you call parent::__construct.\n"
   ]
 )

let constructor_required (pos, name) prop_names =
  let name = Utils.strip_ns name in
  let props_str = SSet.fold ~f:(fun x acc -> x^" "^acc) prop_names ~init:"" in
  add NastCheck.constructor_required pos
    ("Lacking __construct, class "^name^" does not initialize its private member(s): "^props_str)

let not_initialized (pos, cname) prop_names =
  let cname = Utils.strip_ns cname in
  let props_str = SSet.fold ~f:(fun x acc -> x^" "^acc) prop_names ~init:"" in
  let members, verb = if 1 == SSet.cardinal prop_names then "member", "is"
    else "members", "are" in
  let setters_str =
    SSet.fold ~f:(fun x acc -> "$this->"^x^" "^acc) prop_names ~init:"" in
  add NastCheck.not_initialized pos (
    sl[
      "Class "; cname ; " does not initialize all of its members; ";
      props_str; verb; " not always initialized.";
      "\nMake sure you systematically set "; setters_str;
      "when the method __construct is called.";
      "\nAlternatively, you can define the "; members ;" as optional (?...)\n"
    ])

let call_before_init pos cv =
  add NastCheck.call_before_init pos (
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
  add Typing.type_arity_mismatch pos (
  sl["The type ";(Utils.strip_ns name);
     " expects ";nargs;" type parameter(s)"]
 )

let abstract_with_body (p, _) =
  add NastCheck.abstract_with_body p
    "This method is declared as abstract, but has a body"

let not_abstract_without_body (p, _) =
  add NastCheck.not_abstract_without_body p
    "This method is not declared as abstract, it must have a body"

let not_abstract_without_typeconst (p, _) =
  add NastCheck.not_abstract_without_typeconst p
    ("This type constant is not declared as abstract, it must have"^
     " an assigned type")

let abstract_with_typeconst (p, _) =
  add NastCheck.abstract_with_typeconst p
    ("This type constant is declared as abstract, it cannot be assigned a type")

let typeconst_depends_on_external_tparam pos ext_pos ext_name =
  add_list NastCheck.typeconst_depends_on_external_tparam [
    pos, ("A type constant can only use type parameters declared in its own"^
      " type parameter list");
    ext_pos, (ext_name ^ " was declared as a type parameter here");
  ]

let typeconst_assigned_tparam pos tp_name =
  add NastCheck.typeconst_assigned_tparam pos
    (tp_name ^" is a type parameter. It cannot be assigned to a type constant")

let interface_with_partial_typeconst tconst_pos =
  add NastCheck.interface_with_partial_typeconst tconst_pos
    "An interface cannot contain a partially abstract type constant"

let multiple_xhp_category pos =
  add NastCheck.multiple_xhp_category pos
    "XHP classes can only contain one category declaration"

let return_in_gen p =
  add NastCheck.return_in_gen p
    ("You cannot return a value in a generator (a generator"^
     " is a function that uses yield)")

let return_in_finally p =
  add NastCheck.return_in_finally p
    ("Don't use return in a finally block;"^
     " there's nothing to receive the return value")

let toplevel_break p =
  add NastCheck.toplevel_break p
    "break can only be used inside loops or switch statements"

let toplevel_continue p =
  add NastCheck.toplevel_continue p
    "continue can only be used inside loops"

let continue_in_switch p =
  add NastCheck.continue_in_switch p
    ("In PHP, 'continue;' inside a switch \
       statement is equivalent to 'break;'."^
  " Hack does not support this; use 'break' if that is what you meant.")

let await_in_sync_function p =
  add NastCheck.await_in_sync_function p
    "await can only be used inside async functions"

let await_not_allowed p =
  add NastCheck.await_not_allowed p
    "await is only permitted as a statement, expression in a return statement \
      or as a right hand side in top level assignment."

let async_in_interface p =
  add NastCheck.async_in_interface p
    "async is only meaningful when it modifies a method body"

let await_in_coroutine p =
  add NastCheck.await_in_coroutine p
    "await is not allowed in coroutines."

let yield_in_coroutine p =
  add NastCheck.yield_in_coroutine p
    "yield is not allowed in coroutines."

let suspend_outside_of_coroutine p =
  add NastCheck.suspend_outside_of_coroutine p
    "suspend is only allowed in coroutines."

let suspend_in_finally p =
  add NastCheck.suspend_in_finally p
    "suspend is not allowed inside finally blocks."

let break_continue_n_not_supported p =
  add NastCheck.break_continue_n_not_supported p
    "Break/continue N operators are not supported."

let static_memoized_function p =
  add NastCheck.static_memoized_function p
    "memoize is not allowed on static methods in classes that aren't final "

let magic (p, s) =
  add NastCheck.magic p
    ("Don't call "^s^" it's one of these magic things we want to avoid")

let non_interface (p : Pos.t) (c2: string) (verb: string): 'a =
  add NastCheck.non_interface p
    ("Cannot " ^ verb ^ " " ^ (strip_ns c2) ^ " - it is not an interface")

let toString_returns_string pos =
  add NastCheck.toString_returns_string pos "__toString should return a string"

let toString_visibility pos =
  add NastCheck.toString_visibility pos
    "__toString must have public visibility and cannot be static"

let uses_non_trait (p: Pos.t) (n: string) (t: string) =
  add NastCheck.uses_non_trait p
    ((Utils.strip_ns n) ^ " is not a trait. It is " ^ t ^ ".")

let requires_non_class (p: Pos.t) (n: string) (t: string) =
  add NastCheck.requires_non_class p
    ((Utils.strip_ns n) ^ " is not a class. It is " ^ t ^ ".")

let abstract_body pos =
  add NastCheck.abstract_body pos "This method shouldn't have a body"

let not_public_interface pos =
  add NastCheck.not_public_interface pos
    "Access type for interface method must be public"

let interface_with_member_variable pos =
  add NastCheck.interface_with_member_variable pos
    "Interfaces cannot have member variables"

let interface_with_static_member_variable pos =
  add NastCheck.interface_with_static_member_variable pos
    "Interfaces cannot have static variables"

let illegal_function_name pos mname =
  add NastCheck.illegal_function_name pos
    ("Illegal function name: " ^ strip_ns mname)

let dangerous_method_name pos =
  add NastCheck.dangerous_method_name pos (
  "This is a dangerous method name, "^
  "if you want to define a constructor, use "^
  "__construct"
)

let inout_params_outside_of_sync pos =
  add NastCheck.inout_params_outside_of_sync pos (
    "Inout parameters cannot be defined on async functions, "^
    "generators or coroutines."
  )

let mutable_params_outside_of_sync pos fpos name fname =
  add_list NastCheck.mutable_params_outside_of_sync [
    pos, "Mutable parameters are not allowed on async functions";
    pos, "This parameter "^ (strip_ns name) ^" is marked mutable.";
    fpos, "The function "^ (strip_ns fname) ^" is marked async.";
  ]

let mutable_async_method pos =
  add NastCheck.mutable_async_method pos (
    "Mutable methods must be synchronous. Try removing the async tag from the function."
  )

let mutable_attribute_on_function pos =
  add NastCheck.mutable_attribute_on_function pos (
    "<<__Mutable>> only makes sense on methods, or parameters on functions or methods."
  )


let mutable_methods_must_be_reactive pos name =
  add NastCheck.mutable_methods_must_be_reactive pos (
    "The method " ^ (strip_ns name) ^ " has a mutable parameter" ^
    " (or mutable this), so it must be marked reactive with <<__Rx>>."
  )

let mutable_return_annotated_decls_must_be_reactive kind pos name =
  add NastCheck.mutable_return_annotated_decls_must_be_reactive pos (
    "The " ^ kind ^ " " ^ (strip_ns name) ^ " is annotated with <<__MutableReturn>>, " ^
    " so it must be marked reactive with <<__Rx>>."
  )

let inout_params_special pos =
  add NastCheck.inout_params_special pos
  "Methods with special semantics cannot have inout parameters."

let inout_params_mix_byref pos1 pos2 =
  if pos1 <> pos2 then begin
    let msg1 = pos1, "Cannot mix inout and byRef parameters" in
    let msg2 = pos2, "This parameter is passed by reference" in
    add_list NastCheck.inout_params_mix_byref [msg1; msg2]
  end

let inout_params_memoize fpos pos =
  let msg1 = fpos, "Functions with inout parameters cannot be memoized" in
  let msg2 = pos, "This is an inout parameter" in
  add_list NastCheck.inout_params_memoize [msg1; msg2]

let inout_params_ret_by_ref fpos pos =
  let msg1 = fpos,
    "Functions with inout parameters cannot return by reference (&)" in
  let msg2 = pos, "This is an inout parameter" in
  add_list NastCheck.inout_params_ret_by_ref [msg1; msg2]

let reading_from_append pos =
  add NastCheck.reading_from_append pos "Cannot use [] for reading"

let const_attribute_prohibited pos kind =
  add NastCheck.const_attribute_prohibited pos
    ("Cannot apply __Const attribute to " ^ kind)

let global_in_reactive_context pos =
  add NastCheck.global_in_reactive_context pos
    "This is global; you cannot access or write to globals in a reactive context"

let inout_argument_bad_expr pos =
  add NastCheck.inout_argument_bad_expr pos (
    "Arguments for inout parameters must be local variables or simple " ^
    "subscript expressions on vecs, dicts, keysets, or arrays"
  )

let illegal_destructor pos =
  add NastCheck.illegal_destructor pos (
    "Destructors are not supported in Hack; use other patterns like " ^
    "IDisposable/using or try/catch instead."
  )

let conditionally_reactive_function pos =
  add NastCheck.conditionally_reactive_function pos (
    "Function cannot be marked with <<__RxIfImplements>>, " ^
    "<<__RxShallowIfImplements>> or <<__RxLocalIfImplements>>."
  )

let multiple_conditionally_reactive_annotations pos name =
  add NastCheck.multiple_conditionally_reactive_annotations pos (
    "Method '" ^ name ^ "' has multiple <<__RxIfImplements>>, <<__RxShallowIfImplements>> "  ^
    "or <<__RxLocalIfImplements>> annotations."
  )

let conditionally_reactive_annotation_invalid_arguments pos =
  add NastCheck.conditionally_reactive_annotation_invalid_arguments pos (
    "Method is marked with <<__RxIfImplements>>, <<__RxShallowIfImplements>> " ^
    "or <<__RxLocalIfImplements>> attributes that have invalid arguments." ^
    "These attributes must have one argument and it should be " ^
    "'::class' class constant."
  )

let conflicting_reactive_annotations pos =
  add NastCheck.conflicting_reactive_annotations pos (
    "Method cannot be marked as reactive and conditionally reactive at the same time."
  )

(*****************************************************************************)
(* Nast terminality *)
(*****************************************************************************)

let case_fallthrough pos1 pos2 =
  add_list NastCheck.case_fallthrough [
  pos1, ("This switch has a case that implicitly falls through and is "^
      "not annotated with // FALLTHROUGH");
  pos2, "This case implicitly falls through"
]

let default_fallthrough pos =
  add NastCheck.default_fallthrough pos
    ("This switch has a default case that implicitly falls "^
     "through and is not annotated with // FALLTHROUGH")

(*****************************************************************************)
(* Typing errors *)
(*****************************************************************************)

let visibility_extends vis pos parent_pos parent_vis =
  let msg1 = pos, "This member visibility is: " ^ vis in
  let msg2 = parent_pos, parent_vis ^ " was expected" in
  add_list Typing.visibility_extends [msg1; msg2]

let member_not_implemented member_name parent_pos pos defn_pos =
  let msg1 = pos, "This type doesn't implement the method "^member_name in
  let msg2 = parent_pos, "Which is required by this interface" in
  let msg3 = defn_pos, "As defined here" in
  add_list Typing.member_not_implemented [msg1; msg2; msg3]

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
  add Typing.missing_constructor pos
    "The constructor is not implemented"

let typedef_trail_entry pos =
  pos, "Typedef definition comes from here"

let add_with_trail code errs trail =
  add_list code (errs @ List.map trail typedef_trail_entry)

let enum_constant_type_bad pos ty_pos ty trail =
  add_with_trail Typing.enum_constant_type_bad
    [pos, "Enum constants must be an int or string";
     ty_pos, "Not " ^ ty]
    trail

let enum_type_bad pos ty trail =
  add_with_trail Typing.enum_type_bad
    [pos, "Enums must be int or string, not " ^ ty]
    trail

let enum_type_typedef_mixed pos =
  add Typing.enum_type_typedef_mixed pos
    "Can't use typedef that resolves to mixed in enum"

let enum_type_typedef_nonnull pos =
  add Typing.enum_type_typedef_nonnull pos
    "Can't use typedef that resolves to nonnull in enum"

let enum_switch_redundant const first_pos second_pos =
  add_list Typing.enum_switch_redundant [
    second_pos, "Redundant case statement";
    first_pos, const ^ " already handled here"
  ]

let enum_switch_nonexhaustive pos missing enum_pos =
  add_list Typing.enum_switch_nonexhaustive [
    pos, "Switch statement nonexhaustive; the following cases are missing: " ^
            String.concat ", " missing;
    enum_pos, "Enum declared here"
  ]

let enum_switch_redundant_default pos enum_pos =
  add_list Typing.enum_switch_redundant_default [
    pos, "All cases already covered; a redundant default case prevents "^
         "detecting future errors";
    enum_pos, "Enum declared here"
  ]

let enum_switch_not_const pos =
  add Typing.enum_switch_not_const pos
    "Case in switch on enum is not an enum constant"

let enum_switch_wrong_class pos expected got =
  add Typing.enum_switch_wrong_class pos
    ("Switching on enum " ^ expected ^ " but using constant from " ^ got)

let invalid_shape_field_name p =
  add Typing.invalid_shape_field_name p
    "Was expecting a constant string or class constant (for shape access)"

let invalid_shape_field_name_empty p =
  add Typing.invalid_shape_field_name_empty p
    "A shape field name cannot be an empty string"

let invalid_shape_field_name_number p =
  add Typing.invalid_shape_field_name_number p
    "A shape field name cannot start with numbers"

let invalid_shape_field_type pos ty_pos ty trail =
  add_with_trail Typing.invalid_shape_field_type
    [pos, "A shape field name must be an int or string";
     ty_pos, "Not " ^ ty]
    trail

let invalid_shape_field_literal key_pos witness_pos =
  add_list Typing.invalid_shape_field_literal
    [key_pos, "Shape uses literal string as field name";
     witness_pos, "But expected a class constant"]

let invalid_shape_field_const key_pos witness_pos =
  add_list Typing.invalid_shape_field_const
    [key_pos, "Shape uses class constant as field name";
     witness_pos, "But expected a literal string"]

let shape_field_class_mismatch key_pos witness_pos key_class witness_class =
  add_list Typing.shape_field_class_mismatch
    [key_pos, "Shape field name is class constant from " ^ key_class;
     witness_pos, "But expected constant from " ^ witness_class]

let shape_field_type_mismatch key_pos witness_pos key_ty witness_ty =
  add_list Typing.shape_field_type_mismatch
    [key_pos, "Shape field name is " ^ key_ty ^ " class constant";
     witness_pos, "But expected " ^ witness_ty]

let missing_field pos1 pos2 name =
  add_list Typing.missing_field (
    (pos1, "The field '"^name^"' is missing")::
    [pos2, "The field '"^name^"' is defined"])

let unknown_field_disallowed_in_shape pos1 pos2 name =
  add_list
    Typing.unknown_field_disallowed_in_shape
    [
      pos1,
      "The field '" ^ name ^ "' is not defined in this shape type, and \
      this shape type does not allow unknown fields.";
      pos2,
      "The field '" ^ name ^ "' is set in the shape.";
    ]

let missing_optional_field pos1 pos2 name =
  add_list Typing.missing_optional_field
    (* We have the position of shape type that is marked as optional -
     * explain why we can't omit it despite this.*)
    (if pos2 <> Pos.none then (
      (pos1, "The field '"^name^"' may be set to an unknown type. " ^
              "Explicitly null out the field, or remove it " ^
              "(with Shapes::removeKey(...))")::
      [pos2, "The field '"^name^"' is defined as optional"])
   else
      [pos1, "The field '"^name^"' is missing"])

let shape_fields_unknown pos1 pos2 =
  add_list Typing.shape_fields_unknown
    [
      pos1,
      "This shape type allows unknown fields, and so it may contain fields \
      other than those explicitly declared in its declaration.";
      pos2,
      "It is incompatible with a shape that does not allow unknown fields.";
    ]

let shape_field_unset pos1 pos2 name =
  add_list Typing.shape_field_unset (
    [(pos1, "The field '"^name^"' was unset here");
     (pos2, "The field '"^name^"' might be present in this shape because of " ^
            "structural subtyping")]
  )

let invalid_shape_remove_key p =
  add Typing.invalid_shape_remove_key p
    "You can only unset fields of local variables"

let unification_cycle pos ty =
  add_list Typing.unification_cycle
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
  add Typing.overflow p "Value is too large"

let format_string pos snippet s class_pos fname class_suggest =
  add_list Typing.format_string [
  (pos, "I don't understand the format string " ^ snippet ^ " in " ^ s);
  (class_pos,
   "You can add a new format specifier by adding "
   ^fname^"() to "^class_suggest)]

let expected_literal_string pos =
  add Typing.expected_literal_string pos
    "This argument must be a literal string"

let generic_array_strict p =
  add Typing.generic_array_strict p
    "You cannot have an array without generics in strict mode"

let strict_members_not_known p name =
  let name = Utils.strip_ns name in
  add Typing.strict_members_not_known p
    (name^" has a non-<?hh grandparent; this is not allowed in strict mode"
     ^" because that parent may define methods of unknowable name and type")

let option_return_only_typehint p kind =
  let (typehint, reason) = match kind with
    | `void -> ("?void", "only return implicitly")
    | `noreturn -> ("?noreturn", "never return")
  in
  add Typing.option_return_only_typehint p
    (typehint^" is a nonsensical typehint; a function cannot both "^reason
     ^" and return null.")

let tuple_syntax p =
  add Typing.tuple_syntax p
    ("Did you want a tuple? Try (X,Y), not tuple<X,Y>")

let class_arity usage_pos class_pos class_name arity =
  add_list Typing.class_arity
    [usage_pos, ("The class "^(Utils.strip_ns class_name)^" expects "^
                    soi arity^" arguments");
     class_pos, "Definition is here"]

let expecting_type_hint p =
  add Typing.expecting_type_hint p "Was expecting a type hint"

let expecting_type_hint_suggest p ty =
  add Typing.expecting_type_hint_suggest p
    ("Was expecting a type hint (what about: "^ty^")")

let expecting_return_type_hint p =
  add Typing.expecting_return_type_hint p
    "Was expecting a return type hint"

let expecting_return_type_hint_suggest p ty =
  add Typing.expecting_return_type_hint_suggest p
    ("Was expecting a return type hint (what about: ': "^ty^"')")

let expecting_awaitable_return_type_hint p =
  add Typing.expecting_awaitable_return_type_hint p
    "Was expecting an Awaitable return type hint"

let duplicate_using_var pos =
  add Typing.duplicate_using_var pos "Local variable already used in 'using' statement"

let illegal_disposable pos verb =
  add Typing.illegal_disposable pos
    ("Disposable objects must only be " ^ verb ^ " in a 'using' statement")

let escaping_disposable pos =
  add Typing.escaping_disposable pos
    "Variable from 'using' clause may only be used as receiver in method invocation or \
    passed to function with <<__AcceptDisposable>> parameter attribute"

let escaping_disposable_parameter pos =
  add Typing.escaping_disposable_parameter pos
    "Parameter with <<__AcceptDisposable>> attribute may only be used as receiver in method \
    invocation or passed to another function with <<__AcceptDisposable>> parameter attribute"

let escaping_this pos =
  add Typing.escaping_this pos
    "$this implementing IDisposable or IAsyncDisposable may only be used as receiver in method \
    invocation or passed to another function with <<__AcceptDisposable>> parameter attribute"

let must_extend_disposable pos =
  add Typing.must_extend_disposable pos
    "A disposable type may not extend a class or use a trait that is not disposable"

let accept_disposable_invariant pos1 pos2 =
  let msg1 = pos1, "This parameter is marked <<__AcceptDisposable>>" in
  let msg2 = pos2, "This parameter is not marked <<__AcceptDisposable>>" in
  add_list Typing.accept_disposable_invariant [msg1; msg2]

let field_kinds pos1 pos2 =
  add_list Typing.field_kinds
    [pos1, "You cannot use this kind of field (value)";
     pos2, "Mixed with this kind of field (key => value)"]

let unbound_name_typing pos name =
  add Typing.unbound_name_typing pos
    ("Unbound name (typing): "^(strip_ns name))

let previous_default p =
  add Typing.previous_default p
    ("A previous parameter has a default value.\n"^
     "Remove all the default values for the preceding parameters,\n"^
     "or add a default value to this one.")

let return_only_typehint p kind =
  let msg = match kind with
    | `void -> "void"
    | `noreturn -> "noreturn" in
  add Naming.return_only_typehint p
    ("The "^msg^" typehint can only be used to describe a function return type")

let unexpected_type_arguments p =
  add Naming.unexpected_type_arguments p
    ("Type arguments are not expected for this type")

let too_many_type_arguments p =
  add Naming.too_many_type_arguments p
    ("Too many type arguments for this type")

let return_in_void pos1 pos2 =
  add_list Typing.return_in_void [
  pos1,
  "You cannot return a value";
  pos2,
  "This is a void function"
]

let this_in_static p =
  add Typing.this_in_static p "Don't use $this in a static method"

let this_var_outside_class p =
  add Typing.this_var_outside_class p "Can't use $this outside of a class"

let unbound_global cst_pos =
  add Typing.unbound_global cst_pos "Unbound global constant (Typing)"

let private_inst_meth method_pos p =
  add_list Typing.private_inst_meth [
  method_pos, "This is a private method";
  p, "you cannot use it with inst_meth \
    (whether you are in the same class or not)."
]

let protected_inst_meth method_pos p =
  add_list Typing.protected_inst_meth [
  method_pos, "This is a protected method";
  p, "you cannot use it with inst_meth \
    (whether you are in the same class hierarchy or not)."
]

let private_class_meth pos1 pos2 =
  add_list Typing.private_class_meth [
  pos1, "This is a private method";
  pos2, "you cannot use it with class_meth \
    (whether you are in the same class or not)."
]

let protected_class_meth pos1 pos2 =
  add_list Typing.protected_class_meth [
  pos1, "This is a protected method";
  pos2, "you cannot use it with class_meth \
    (whether you are in the same class hierarchy or not)."
]

let array_cast pos =
  add Typing.array_cast pos
    "(array) cast forbidden; arrays with unspecified \
    key and value types are not allowed"

let nullable_cast pos ty ty_pos =
  add_list Typing.nullable_cast [
    pos, "Casting from a nullable type is forbidden";
    ty_pos, "This is "^ty;
  ]

let anonymous_recursive pos =
  add Typing.anonymous_recursive pos
    "Anonymous functions cannot be recursive"

let static_outside_class pos =
  add Typing.static_outside_class pos
    "'static' is undefined outside of a class"

let self_outside_class pos =
  add Typing.self_outside_class pos
    "'self' is undefined outside of a class"

let new_inconsistent_construct new_pos (cpos, cname) kind =
  let name = Utils.strip_ns cname in
  let preamble = match kind with
    | `static -> "Can't use new static() for "^name
    | `classname -> "Can't use new on classname<"^name^">"
  in
  add_list Typing.new_static_inconsistent [
    new_pos, preamble^"; __construct arguments are not \
    guaranteed to be consistent in child classes";
    cpos, ("This declaration neither defines an abstract/final __construct"
           ^" nor uses <<__ConsistentConstruct>> attribute")]

let pair_arity pos =
  add Typing.pair_arity pos "A pair has exactly 2 elements"

let tuple_arity pos2 size2 pos1 size1 =
  add_list Typing.tuple_arity [
  pos2, "This tuple has "^ string_of_int size2^" elements";
  pos1, string_of_int size1 ^ " were expected"]

let undefined_parent pos =
  add Typing.undefined_parent pos
    "The parent class is undefined"

let parent_outside_class pos =
  add Typing.parent_outside_class pos
    "'parent' is undefined outside of a class"

let parent_abstract_call meth_name call_pos decl_pos =
  add_list Typing.abstract_call [
    call_pos, ("Cannot call parent::"^meth_name^"(); it is abstract");
    decl_pos, "Declaration is here"
  ]

let self_abstract_call meth_name call_pos decl_pos =
  add_list Typing.abstract_call [
    call_pos, ("Cannot call self::"^meth_name^"(); it is abstract. Did you mean static::"^meth_name^"()?");
    decl_pos, "Declaration is here"
  ]

let classname_abstract_call cname meth_name call_pos decl_pos =
  let cname = Utils.strip_ns cname in
  add_list Typing.abstract_call [
    call_pos, ("Cannot call "^cname^"::"^meth_name^"(); it is abstract");
    decl_pos, "Declaration is here"
  ]

let static_synthetic_method cname meth_name call_pos decl_pos =
  let cname = Utils.strip_ns cname in
  add_list Typing.static_synthetic_method [
    call_pos, ("Cannot call "^cname^"::"^meth_name^"(); "^meth_name^" is not defined in "^cname);
    decl_pos, "Declaration is here"
  ]

let empty_in_strict pos =
  add Typing.isset_empty_in_strict pos
    ("empty cannot be used in a completely type safe way and so is banned in "
     ^"strict mode")

let isset_in_strict pos =
  add Typing.isset_empty_in_strict pos
    ("isset cannot be used in a completely type safe way and so is banned in "
     ^"strict mode; try using array_key_exists instead")

let unset_nonidx_in_strict pos msgs =
  add_list Typing.unset_nonidx_in_strict
    ([pos, "In strict mode, unset is banned except on array, keyset, "^
           "or dict indexing"] @
     msgs)

let unpacking_disallowed_builtin_function pos name =
  let name = Utils.strip_ns name in
  add Typing.unpacking_disallowed pos
    ("Arg unpacking is disallowed for "^name)

let array_get_arity pos1 name pos2 =
  add_list Typing.array_get_arity [
  pos1, "You cannot use this "^(Utils.strip_ns name);
  pos2, "It is missing its type parameters"
]

let typing_error pos msg =
  add Typing.generic_unify pos msg

let typing_error_l err =
  add_error err

let undefined_field ~use_pos ~name ~shape_type_pos =
  add_list Typing.undefined_field [
    use_pos, "The field "^name^" is undefined";
    shape_type_pos, "You might want to check this out"
  ]

let array_access pos1 pos2 ty =
  add_list Typing.array_access
    ((pos1, "This is not an object of type KeyedContainer, this is "^ty) ::
     if pos2 != Pos.none
     then [pos2, "You might want to check this out"]
     else [])

let keyset_set pos1 pos2 =
  add_list Typing.keyset_set
    ((pos1, "Keysets entries cannot be set, use append instead.") ::
     if pos2 != Pos.none
     then [pos2, "You might want to check this out"]
     else [])

let array_append pos1 pos2 ty =
  add_list Typing.array_append
    ((pos1, ty^" does not allow array append") ::
     if pos2 != Pos.none
     then [pos2, "You might want to check this out"]
     else [])

let const_mutation pos1 pos2 ty =
  add_list Typing.const_mutation
    ((pos1, "You cannot mutate this") ::
     if pos2 != Pos.none
     then [(pos2, "This is " ^ ty)]
     else [])

let expected_class ?(suffix="") pos =
  add Typing.expected_class pos ("Was expecting a class"^suffix)

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
  add_list Typing.smember_not_found
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
  add_list Typing.member_not_found
    ((pos, msg) :: (not_found_hint hint @ reason
                    @ [(cpos, "Declaration of "^type_name^" is here")]))

let parent_in_trait pos =
  add Typing.parent_in_trait pos
    ("parent:: inside a trait is undefined"
     ^" without 'require extends' of a class defined in <?hh")

let parent_undefined pos =
  add Typing.parent_undefined pos
    "parent is undefined"

let constructor_no_args pos =
  add Typing.constructor_no_args pos
    "This constructor expects no argument"

let visibility p msg1 p_vis msg2 =
  add_list Typing.visibility [p, msg1; p_vis, msg2]

let typing_too_many_args pos pos_def =
  add_list Typing.typing_too_many_args
    [pos, "Too many arguments"; pos_def, "Definition is here"]

let typing_too_few_args pos pos_def =
  add_list Typing.typing_too_few_args
    [pos, "Too few arguments"; pos_def, "Definition is here"]

let anonymous_recursive_call pos =
  add Typing.anonymous_recursive_call pos
    "recursive call to anonymous function"

let bad_call pos ty =
  add Typing.bad_call pos
    ("This call is invalid, this is not a function, it is "^ty)

let sketchy_null_check pos =
  add Typing.sketchy_null_check pos (
  "You are using a sketchy null check ...\n"^
  "Use $x === null instead"
)

let sketchy_null_check_primitive pos =
  add Typing.sketchy_null_check_primitive pos (
  "You are using a sketchy null check on a primitive type ...\n"^
  "Use $x === null instead"
 )

let extend_final extend_pos decl_pos name =
  let name = (strip_ns name) in
  add_list Typing.extend_final [
    extend_pos, ("You cannot extend final class "^name);
    decl_pos, "Declaration is here"
  ]

let read_before_write (pos, v) =
  add Typing.read_before_write pos (
  sl[
  "Read access to $this->"; v; " before initialization"
])

let interface_final pos =
  add Typing.interface_final pos
    "Interfaces cannot be final"

let trait_final pos =
  add Typing.trait_final pos
    "Traits cannot be final"

let final_property pos =
  add Typing.final_property pos "Properties cannot be declared final"

let implement_abstract ~is_final pos1 pos2 kind x =
  let name = "abstract "^kind^" '"^x^"'" in
  let msg1 =
    if is_final then
      "This class was declared as final. It must provide an implementation \
       for the "^name
    else
      "This class must be declared abstract, or provide an implementation \
       for the "^name in
  add_list Typing.implement_abstract [
    pos1, msg1;
    pos2, "Declaration is here";
  ]

let generic_static pos x =
  add Typing.generic_static pos (
  "This static variable cannot use the type parameter "^x^"."
 )

let fun_too_many_args pos1 pos2 =
  add_list Typing.fun_too_many_args [
  pos1, "Too many mandatory arguments";
  pos2, "Because of this definition";
]

let fun_too_few_args pos1 pos2 =
  add_list Typing.fun_too_few_args [
  pos1, "Too few arguments";
  pos2, "Because of this definition";
]

let fun_unexpected_nonvariadic pos1 pos2 =
  add_list Typing.fun_unexpected_nonvariadic [
  pos1, "Should have a variadic argument";
  pos2, "Because of this definition";
]

let fun_variadicity_hh_vs_php56 pos1 pos2 =
  add_list Typing.fun_variadicity_hh_vs_php56 [
  pos1, "Variadic arguments: ...-style is not a subtype of ...$args";
  pos2, "Because of this definition";
]

let ellipsis_strict_mode ~require_param_name pos =
  let msg =
    if require_param_name then
      "Cannot use ... without a type hint and parameter name in strict mode. \
      Please add a type hint and parameter name."
    else
      "Cannot use ... without a type hint in strict mode. Please add a type hint."
    in
  add Typing.ellipsis_strict_mode pos msg

let untyped_lambda_strict_mode pos =
  let msg =
    "Cannot determine types of lambda parameters in strict mode. \
     Please add type hints on parameters."
  in
    add Typing.untyped_lambda_strict_mode pos msg

let invalid_conditionally_reactive_call pos condition_type receiver_type =
  add Typing.invalid_conditionally_reactive_call pos (
    "Conditionally reactive method with condition type '" ^ condition_type ^
    "' cannot be called. Calling such methods is allowed only from conditionally " ^
    "reactive methods with the same condition type or if static type of method " ^
    "receiver is a subtype of condition type, now '" ^ receiver_type ^ "'."
  )

let expected_tparam pos n =
  add Typing.expected_tparam pos (
  "Expected " ^
  (match n with
  | 0 -> "no type parameter"
  | 1 -> "a type parameter"
  | n -> string_of_int n ^ " type parameters"
  )
 )

let object_string pos1 pos2 =
  add_list Typing.object_string [
  pos1, "You cannot use this object as a string";
  pos2, "This object doesn't implement __toString";
]

let type_param_arity pos x n =
  add Typing.type_param_arity pos (
  "The type "^x^" expects "^n^" parameters"
 )

let cyclic_typedef p =
  add Typing.cyclic_typedef p
    "Cyclic typedef"

let type_arity_mismatch pos1 n1 pos2 n2 =
  add_list Typing.type_arity_mismatch [
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
  add_list Typing.tuple_arity_mismatch [
  pos1, "This tuple has "^n1^" elements";
  pos2, "This one has "^n2^" elements"
]

let fun_arity_mismatch pos1 pos2 =
  add_list Typing.fun_arity_mismatch [
  pos1, "Number of arguments doesn't match";
  pos2, "Because of this definition";
]

let fun_reactivity_mismatch pos1 kind1 pos2 kind2 =
  let f k = "This function is " ^ k ^ "." in
  add_list
    Typing.fun_reactivity_mismatch
    [
      pos1, f kind1;
      pos2, f kind2
    ]

let frozen_in_incorrect_scope pos1 =
  add Typing.frozen_in_incorrect_scope pos1
  ("This variable is frozen in one scope but not the other")


let reassign_mutable_var pos1 =
  add Typing.reassign_mutable_var pos1
  ("This variable is mutable. You cannot create a new reference to it.")

let mutable_call_on_immutable fpos pos1 =
  add_list Typing.mutable_call_on_immutable
  [
    pos1, "Cannot call mutable function on immutable expression";
    fpos, "This function is marked <<__Mutable>>, so it has a mutable $this.";
  ]

let mutable_argument_mismatch param_pos arg_pos =
  add_list Typing.mutable_argument_mismatch
  [
    arg_pos, "Invalid argument";
    param_pos, "This parameter is marked mutable";
    arg_pos, "But this expression is not";
  ]

let invalid_mutable_return_result error_pos function_pos value_kind =
  add_list Typing.invalid_mutable_return_result
  [
    error_pos, "Functions marked <<__MutableReturn>> must return mutably owned values";
    function_pos, "This function is marked <<__MutableReturn>>";
    error_pos, "This expression is (" ^ value_kind ^ ")"
  ]

let freeze_in_nonreactive_context pos1 =
  add Typing.freeze_in_nonreactive_context pos1
  ("freeze only makes sense in reactive functions")

let invalid_freeze_use pos1 =
  add Typing.invalid_freeze_use pos1
  ("freeze takes a single mutably-owned local variable as an argument")

let invalid_freeze_target pos1 var_pos var_mutability_str =
  add_list Typing.invalid_freeze_target
  [
    pos1, "Invalid argument - freeze() takes a single mutable variable";
    var_pos, "This variable is "^var_mutability_str;
  ]

let discarded_awaitable pos1 pos2 =
  add_list Typing.discarded_awaitable [
  pos1, "This expression is of type Awaitable, but it's "^
  "either being discarded or used in a dangerous way before "^
  "being awaited";
  pos2, "This is why I think it is Awaitable"
]

let gena_expects_array pos1 pos2 ty_str =
  add_list Typing.gena_expects_array [
  pos1, "gena expects an array";
  pos2, "It is incompatible with " ^ ty_str;
]

let unify_error left right =
  add_list Typing.unify_error (left @ right)

let static_dynamic static_position dyn_position method_name =
  let msg_static = "The function "^method_name^" is static" in
  let msg_dynamic = "It is defined as dynamic here" in
  add_list Typing.static_dynamic [
  static_position, msg_static;
  dyn_position, msg_dynamic
]

let null_member s pos r =
  add_list Typing.null_member ([
  pos,
  "You are trying to access the member "^s^
  " but this object can be null. "
] @ r
)

let non_object_member s pos1 ty pos2 =
  add_list Typing.non_object_member [
  pos1,
  ("You are trying to access the member "^s^
   " but this is not an object, it is "^
   ty);
  pos2,
  "Check this out"
  ]

let ambiguous_member s pos1 ty pos2 =
  add_list Typing.ambiguous_member [
  pos1,
  ("You are trying to access the member "^s^
   " but there is more than one implementation on "^
   ty);
  pos2,
  "Check this out"
  ]

let null_container p null_witness =
  add_list Typing.null_container (
  [
   p,
   "You are trying to access an element of this container"^
   " but the container could be null. "
 ] @ null_witness)

let option_mixed pos =
  add Typing.option_mixed pos
    "?mixed is a redundant typehint - just use mixed"

let declared_covariant pos1 pos2 emsg =
  add_list Typing.declared_covariant (
  [pos2, "Illegal usage of a covariant type parameter";
   pos1, "This is where the parameter was declared as covariant (+)"
 ] @ emsg
 )

let declared_contravariant pos1 pos2 emsg =
  add_list Typing.declared_contravariant (
  [pos2, "Illegal usage of a contravariant type parameter";
   pos1, "This is where the parameter was declared as contravariant (-)"
 ] @ emsg
 )

let contravariant_this pos class_name tp =
  add Typing.contravariant_this pos (
    "The \"this\" type cannot be used in this " ^
    "contravariant position because its enclosing class \"" ^ class_name ^
    "\" " ^ "is final and has a variant type parameter \"" ^ tp ^ "\"")

let cyclic_typeconst pos sl =
  let sl = List.map sl strip_ns in
  add Typing.cyclic_typeconst pos
    ("Cyclic type constant:\n  "^String.concat " -> " sl)

let this_lvalue pos =
  add Typing.this_lvalue pos "Cannot assign a value to $this"

let abstract_concrete_override pos parent_pos kind =
  let kind_str = match kind with
    | `method_ -> "method"
    | `typeconst -> "type constant"
    | `constant -> "constant" in
  add_list Typing.abstract_concrete_override ([
    pos, "Cannot re-declare this " ^ kind_str ^ " as abstract";
    parent_pos, "Previously defined here"
  ])

let instanceof_always_false pos =
  add Typing.instanceof_always_false pos
    "This 'instanceof' test will never succeed"

let instanceof_always_true pos =
  add Typing.instanceof_always_true pos
    "This 'instanceof' test will always succeed"

let instanceof_generic_classname pos name =
  add Typing.instanceof_generic_classname pos
    ("'instanceof' cannot be used on 'classname<" ^ name ^ ">' because '" ^
    name ^ "' may be instantiated with a type such as \
     'C<int>' that cannot be checked at runtime")

let required_field_is_optional pos1 pos2 name =
  add_list Typing.required_field_is_optional
    [
      pos1, "The field '"^name^"' is optional";
      pos2, "The field '"^name^"' is defined as required"
    ]

let array_get_with_optional_field pos1 pos2 name =
  add_list
    Typing.array_get_with_optional_field
    [
      pos1,
      "Invalid index operation: '" ^ name ^ "' is marked as an optional shape \
      field. It may not be present in the shape. Use Shapes::idx instead.";
      pos2,
      "This is where the field was declared as optional."
    ]

let non_call_argument_in_suspend pos msgs =
  add_list
    Typing.non_call_argument_in_suspend (
    [
      pos,
      "'suspend' operator expects call to a coroutine as an argument."
    ] @ msgs
  )
let non_coroutine_call_in_suspend pos msgs =
  add_list
    Typing.non_coroutine_call_in_suspend (
    [
      pos,
      "Only coroutine functions are allowed to be called in \
      'suspend' operator."
    ] @ msgs
  )

let coroutine_call_outside_of_suspend pos =
  add_list
    Typing.coroutine_call_outside_of_suspend
    [
      pos,
      "Coroutine calls are only allowed when they are arguments to \
      'suspend' operator"
    ]

let function_is_not_coroutine pos name =
  add_list
    Typing.function_is_not_coroutine
    [
      pos,
      "Function '" ^ name ^ "' is not a coroutine and cannot be \
       used in as an argument of 'suspend' operator."
    ]

let coroutinness_mismatch pos1_is_coroutine pos1 pos2 =
  let m1 = "This is a coroutine." in
  let m2 = "This is not a coroutine." in
  add_list
    Typing.coroutinness_mismatch
    [
      pos1, if pos1_is_coroutine then m1 else m2;
      pos2, if pos1_is_coroutine then m2 else m1;
    ]

let return_disposable_mismatch pos1_return_disposable pos1 pos2 =
  let m1 = "This is marked <<__ReturnDisposable>>." in
  let m2 = "This is not marked <<__ReturnDisposable>>." in
  add_list
    Typing.return_disposable_mismatch
    [
      pos1, if pos1_return_disposable then m1 else m2;
      pos2, if pos1_return_disposable then m2 else m1;
    ]

let this_as_lexical_variable pos =
  add Naming.this_as_lexical_variable pos "Cannot use $this as lexical variable"

let dollardollar_lvalue pos =
  add Typing.dollardollar_lvalue pos
    "Cannot assign a value to the special pipe variable ($$)"

let assigning_to_const pos =
  add Typing.assigning_to_const pos
    "Cannot assign to a __Const property"

let self_const_parent_not pos =
  add Typing.self_const_parent_not pos
    "A __Const class may only extend other __Const classes"

let parent_const_self_not pos =
  add Typing.parent_const_self_not pos
    "Only __Const classes may extend a __Const class"

let overriding_prop_const_mismatch parent_pos parent_const child_pos child_const =
  let m1 = "This property is __Const" in
  let m2 = "This property is not __Const" in
  add_list Typing.overriding_prop_const_mismatch
  [
    parent_pos, if parent_const then m1 else m2;
    child_pos, if child_const then m1 else m2;
  ]

let mutable_return_result_mismatch pos1_has_mutable_return pos1 pos2 =
  let m1 = "This is marked <<__MutableReturn>>." in
  let m2 = "This is not marked <<__MutableReturn>>." in
  add_list
    Typing.mutable_return_result_mismatch
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
  add_list Typing.wrong_extend_kind [msg1; msg2]

let unsatisfied_req parent_pos req_name req_pos =
  let s1 = "Failure to satisfy requirement: "^(Utils.strip_ns req_name) in
  let s2 = "Required here" in
  if req_pos = parent_pos
  then add Typing.unsatisfied_req parent_pos s1
  else add_list Typing.unsatisfied_req [parent_pos, s1; req_pos, s2]

let cyclic_class_def stack pos =
  let stack =
    SSet.fold ~f:(fun x y -> (Utils.strip_ns x)^" "^y) stack ~init:"" in
  add Typing.cyclic_class_def pos ("Cyclic class definition : "^stack)

let trait_reuse p_pos p_name class_name trait =
  let c_pos, c_name = class_name in
  let c_name = Utils.strip_ns c_name in
  let trait = Utils.strip_ns trait in
  let err = "Class "^c_name^" reuses trait "^trait^" in its hierarchy" in
  let err' = "It is already used through "^(Utils.strip_ns p_name) in
  add_list Typing.trait_reuse [c_pos, err; p_pos, err']

(**
 * This error should be unfixmeable, because the `is` expression does not
 * support it at all.
 *)
let invalid_is_expression_hint pos ty_str =
  add Typing.invalid_is_expression_hint pos
    ("The `is` operator cannot be used with "^ty_str)

(**
 * This error is fixmeable, because the typechecker will still refine the type
 * despite the hint not being completely valid.
 *)
let partially_valid_is_expression_hint pos ty_str=
  add Typing.partially_valid_is_expression_hint pos
    ("The `is` operator should not be used with "^ty_str)

let override_final ~parent ~child =
  add_list Typing.override_final [child, "You cannot override this method";
            parent, "It was declared as final"]

let should_be_override pos class_id id =
  add Typing.should_be_override pos
    ((Utils.strip_ns class_id)^"::"^id^"() is marked as override; \
       no non-private parent definition found \
       or overridden parent is defined in non-<?hh code")

let override_per_trait class_name id m_pos =
    let c_pos, c_name = class_name in
    let err_msg =
      ("Method "^(Utils.strip_ns c_name)^"::"^id^" should be an override \
        per the declaring trait; no non-private parent definition found \
        or overridden parent is defined in non-<?hh code")
    in add_list Typing.override_per_trait [
      c_pos, err_msg;
      m_pos, "Declaration of "^id^"() is here"
    ]

let missing_assign pos =
  add Typing.missing_assign pos "Please assign a value"

let private_override pos class_id id =
  add Typing.private_override pos ((Utils.strip_ns class_id)^"::"^id
          ^": combining private and override is nonsensical")

let invalid_memoized_param pos ty_reason_msg =
  add_list Typing.invalid_memoized_param (
    ty_reason_msg @ [pos,
      "Parameters to memoized function must be null, bool, int, float, string, \
      an object deriving IMemoizeParam, or a Container thereof. See also \
      http://docs.hhvm.com/hack/attributes/special#__memoize"])

let invalid_disposable_hint pos class_name =
  add Typing.invalid_disposable_hint pos ("Parameter with type '" ^ class_name ^ "' must not \
    implement IDisposable or IAsyncDisposable. Please use <<__AcceptDisposable>> attribute or \
    create disposable object with 'using' statement instead.")

let invalid_disposable_return_hint pos class_name =
  add Typing.invalid_disposable_return_hint pos ("Return type '" ^ class_name ^ "' must not \
    implement IDisposable or IAsyncDisposable. Please add <<__ReturnDisposable>> attribute.")

let xhp_required pos why_xhp ty_reason_msg =
  let msg = "An XHP instance was expected" in
  add_list Typing.xhp_required ((pos, msg)::(pos, why_xhp)::ty_reason_msg)

let illegal_xhp_child pos ty_reason_msg =
  let msg = "XHP children must be compatible with XHPChild" in
  add_list Typing.illegal_xhp_child ((pos, msg)::ty_reason_msg)

let nullsafe_not_needed p nonnull_witness =
  add_list Typing.nullsafe_not_needed (
  [
   p,
   "You are using the ?-> operator but this object cannot be null. "
 ] @ nonnull_witness)

let generic_at_runtime p =
  add Typing.generic_at_runtime p
    "Generics can only be used in type hints since they are erased at runtime."

let trivial_strict_eq p b left right left_trail right_trail =
  let msg = "This expression is always "^b in
  let left_trail = List.map left_trail typedef_trail_entry in
  let right_trail = List.map right_trail typedef_trail_entry in
  add_list Typing.trivial_strict_eq
    ((p, msg) :: left @ left_trail @ right @ right_trail)

let trivial_strict_not_nullable_compare_null p result type_reason =
  let msg = "This expression is always "^result in
  add_list Typing.not_nullable_compare_null_trivial
    ((p, msg) :: type_reason)

let eq_incompatible_types p left right =
  let msg = "This equality test has incompatible types" in
  add_list Typing.eq_incompatible_types
    ((p, msg) :: left @ right)

let void_usage p void_witness =
  let msg = "You are using the return value of a void function" in
  add_list Typing.void_usage ((p, msg) :: void_witness)

let noreturn_usage p noreturn_witness =
  let msg = "You are using the return value of a noreturn function" in
  add_list Typing.noreturn_usage ((p, msg) :: noreturn_witness)

let attribute_too_few_arguments pos x n =
  let n = string_of_int n in
  add Typing.attribute_too_few_arguments pos (
    "The attribute "^x^" expects at least "^n^" arguments"
  )

let attribute_too_many_arguments pos x n =
  let n = string_of_int n in
  add Typing.attribute_too_many_arguments pos (
    "The attribute "^x^" expects at most "^n^" arguments"
  )

let attribute_param_type pos x =
  add Typing.attribute_param_type pos (
    "This attribute parameter should be "^x
  )

let deprecated_use pos pos_def msg =
  add_list Typing.deprecated_use [
    pos, msg;
    pos_def, "Definition is here";
  ]

let cannot_declare_constant kind pos (class_pos, class_name) =
  let kind_str =
    match kind with
    | `enum -> "an enum"
    | `trait -> "a trait"
  in
  add_list Typing.cannot_declare_constant [
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
  add_list Typing.local_variable_modifed_and_used
           ((pos_modified, "Unsequenced modification and access to local \
                            variable. Modified here") ::
            List.map pos_used_l used_msg)

let local_variable_modified_twice pos_modified pos_modified_l =
  let modified_msg p = p, "And also modified here" in
  add_list Typing.local_variable_modifed_twice
           ((pos_modified, "Unsequenced modifications to local variable. \
                            Modified here") ::
            List.map pos_modified_l modified_msg)

let assign_during_case p =
  add Typing.assign_during_case p
    "Don't assign to variables inside of case labels"

let cyclic_enum_constraint pos =
  add Typing.cyclic_enum_constraint pos "Cyclic enum constraint"

let invalid_classname p =
  add Typing.invalid_classname p "Not a valid class name"

let illegal_type_structure pos errmsg =
  let msg =
    "The two arguments to type_structure() must be:"
    ^"\n - first: ValidClassname::class or an object of that class"
    ^"\n - second: a single-quoted string literal containing the name"
    ^" of a type constant of that class"
    ^"\n"^errmsg in
  add Typing.illegal_type_structure pos msg

let illegal_typeconst_direct_access pos =
  let msg =
    "Type constants cannot be directly accessed. "
    ^"Use type_structure(ValidClassname::class, 'TypeConstName') instead" in
  add Typing.illegal_type_structure pos msg

let class_property_only_static_literal pos =
  let msg =
    "Initialization of class property must be a static literal expression." in
  add Typing.class_property_only_static_literal pos msg

let reference_expr pos =
  let msg = "Cannot take a value by reference in strict mode." in
  add Typing.reference_expr pos msg

let pass_by_ref_annotation_missing pos1 pos2 =
  let msg1 = pos1, "This argument should be annotated with &" in
  let msg2 = pos2, "Because this parameter is passed by reference" in
  add_list Typing.pass_by_ref_annotation_missing [msg1; msg2]

let pass_by_ref_annotation_unexpected pos1 pos2 =
  let msg1 = pos1, "This argument should not be annotated with &" in
  let msg2 = pos2, "Because this parameter is passed by value" in
  add_list Typing.pass_by_ref_annotation_unexpected [msg1; msg2]

let reffiness_invariant pos1 pos2 mode2 =
  let msg1 = pos1, "This parameter is passed by reference" in
  let mode_str = match mode2 with
    | `normal -> "a normal parameter"
    | `inout -> "an inout parameter" in
  let msg2 = pos2, "It is incompatible with " ^ mode_str in
  add_list Typing.reffiness_invariant [msg1; msg2]

let inout_annotation_missing pos1 pos2 =
  let msg1 = pos1, "This argument should be annotated with 'inout'" in
  let msg2 = pos2, "Because this is an inout parameter" in
  add_list Typing.inout_annotation_missing [msg1; msg2]

let inout_annotation_unexpected pos1 pos2 =
  let msg1 = pos1, "Unexpected inout annotation for argument" in
  let msg2 = pos2, "This is a normal parameter (does not have 'inout')" in
  add_list Typing.inout_annotation_unexpected [msg1; msg2]

let inoutness_mismatch pos1 pos2 =
  let msg1 = pos1, "This is an inout parameter" in
  let msg2 = pos2, "It is incompatible with a normal parameter" in
  add_list Typing.inoutness_mismatch [msg1; msg2]

let invalid_new_disposable pos =
  let msg =
    "Disposable objects may only be created in a 'using' statement or 'return' from function marked <<__ReturnDisposable>>" in
  add Typing.invalid_new_disposable pos msg

let invalid_return_disposable pos =
  let msg =
    "Return expression must be new disposable in function marked <<__ReturnDisposable>>" in
  add Typing.invalid_return_disposable pos msg

let nonreactive_function_call pos decl_pos =
  add_list Typing.nonreactive_function_call [
    pos, "Reactive functions can only call other reactive functions.";
    decl_pos, "This function is not reactive."
  ]

let nonreactive_call_from_shallow pos decl_pos =
  add_list Typing.nonreactive_call_from_shallow [
      pos, "Shallow reactive functions cannot call non-reactive functions.";
      decl_pos, "This function is not reactive."
  ]

let rx_enabled_in_non_rx_context pos =
  add Typing.rx_enabled_in_non_rx_context pos (
    "\\HH\\Rx\\IS_ENABLED can only be used in reactive functions."
  )

let rx_enabled_in_lambdas pos =
  add Typing.rx_enabled_in_lambdas pos (
    "\\HH\\Rx\\IS_ENABLED cannot be used inside lambdas."
  )

let nonreactive_append pos =
  let msg = "Cannot append to a Hack Collection types in a reactive context" in
  add Typing.nonreactive_append pos msg

let obj_set_reactive pos =
  let msg = ("This object's property is being mutated(used as an lvalue)" ^
  "\nYou cannot set non-mutable object properties in reactive functions") in
  add Typing.obj_set_reactive pos msg

let inout_argument_bad_type pos msgl =
  let msg =
    "Expected argument marked inout to be contained in a local or " ^
    "a value-typed container (e.g. vec, dict, keyset, array). " ^
    "To use inout here, assign to/from a temporary local variable." in
  add_list Typing.inout_argument_bad_type ((pos, msg) :: msgl)

let ambiguous_lambda pos n =
  let msg =
    Printf.sprintf
      "Lambda has parameter types that could not be determined at definition-site: \
       body was checked %d times. Please add type hints"
      n in
  add Typing.ambiguous_lambda pos msg

let binding_ref_in_array pos =
  let msg = "Binding a reference in an array is no longer supported in Hack." in
  add Typing.binding_ref_in_array pos msg


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
  let _, result =  (do_ f) in
  result

let try_when f ~when_ ~do_ =
  M.try_with_result f begin fun result (error: error) ->
    if when_()
    then do_ error
    else add_error error;
    result
  end

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
