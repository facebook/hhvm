(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

let check_multiple_default casel =
  let is_default = function
    | Default _ -> true
    | _ -> false
  in
  match List.filter casel ~f:is_default with
  | _ :: Default (pos, _) :: _ -> Errors.switch_multiple_default pos
  | _ -> ()

let check_non_terminal_default casel =
  let raise_if_non_terminal (has_default, already_raised) = function
    | Default _ -> (true, already_raised)
    | Case ((pos, _), _) when has_default && not already_raised ->
      Errors.switch_non_terminal_default pos;
      (has_default, true)
    | _ -> (has_default, already_raised)
  in
  List.fold casel ~init:(false, false) ~f:raise_if_non_terminal |> ignore

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_stmt _ s =
      match snd s with
      | Switch (_, casel) ->
        check_multiple_default casel;
        check_non_terminal_default casel
      | _ -> ()
  end
