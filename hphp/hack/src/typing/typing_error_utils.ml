(*
 * Copyrighd (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core

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

let is_suppressed User_error.{ claim; code; _ } =
  Errors.fixme_present Message.(get_message_pos claim) code

let add_typing_error err =
  Typing_error.(
    iter ~on_prim:log_primary_typing_error ~on_snd:(fun _ -> ()) err;
    Eval_result.iter ~f:Errors.add_error
    @@ Eval_result.suppress_intersection ~is_suppressed
    @@ to_user_error err ~current_span:(Errors.get_current_span ()))

(* Until we return a list of errors from typing, we have to apply
   'client errors' to a callback for using in subtyping *)
let apply_callback_to_errors :
    Errors.t -> Typing_error.Reasons_callback.t -> unit =
 fun errors on_error ->
  let on_error
      User_error.{ code; claim; reasons; quickfixes = _; is_fixmed = _ } =
    Typing_error.(
      let code = Option.value_exn (Error_code.of_enum code) in
      Eval_result.iter ~f:Errors.add_error
      @@ Eval_result.suppress_intersection ~is_suppressed
      @@ Reasons_callback.apply
           on_error
           ~code
           ~claim:(lazy claim)
           ~reasons:(lazy reasons)
           ~current_span:(Errors.get_current_span ()))
  in
  Errors.iter errors ~f:on_error

let apply_error_from_reasons_callback ?code ?claim ?reasons ?quickfixes err =
  Typing_error.(
    Eval_result.iter ~f:Errors.add_error
    @@ Eval_result.suppress_intersection ~is_suppressed
    @@ Reasons_callback.apply
         ?code
         ?claim
         ?reasons
         ?quickfixes
         err
         ~current_span:(Errors.get_current_span ()))

let claim_as_reason : Pos.t Message.t -> Pos_or_decl.t Message.t =
 (fun (p, m) -> (Pos_or_decl.of_raw_pos p, m))

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
