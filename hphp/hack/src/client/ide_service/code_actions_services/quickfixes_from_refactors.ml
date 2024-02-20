(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

(*
 * Make sure to write tests any time you match against an error here
 * as correct behavior is dependent on matching specific strings and error codes (which can change).
 *)

let select_refactoring_from_parse_error (e : Errors.error) :
    Code_action_types.Refactor.find option =
  let msg_str =
    let User_error.{ claim; _ } = e in
    Message.get_message_str claim
  in
  match Error_codes.Parsing.of_enum e.User_error.code with
  | Some Error_codes.Parsing.ParsingError -> begin
    match msg_str with
    | "`await` cannot be used as an expression in this location because it's conditionally executed."
      ->
      Some Extract_variable.find
    | _ -> None
  end
  | _ -> None

let select_refactoring_from_type_error (e : Errors.error) :
    Code_action_types.Refactor.find option =
  match Error_codes.Typing.of_enum e.User_error.code with
  | Some Error_codes.Typing.DiscardedAwaitable -> Some Await_expression.find
  | _ -> None

let select_refactoring (e : Errors.error) :
    Code_action_types.Refactor.find option =
  match select_refactoring_from_parse_error e with
  | Some fn -> Some fn
  | None -> select_refactoring_from_type_error e

let find ctx entry (e : Errors.error) =
  select_refactoring e
  |> Option.map ~f:(fun find_refactors ->
         let e_pos = User_error.get_pos e in
         let refactors = find_refactors ~entry e_pos ctx in
         List.map
           refactors
           ~f:
             Code_action_types.(
               (fun Refactor.{ title; edits } -> Quickfix.{ title; edits })))
  |> Option.value ~default:[]
