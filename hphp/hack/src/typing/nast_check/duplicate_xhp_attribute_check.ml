(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

open Aast

(* Produce a syntax error on XHP expressions of the form:
 * <foo x={1} x={2} />
 *
 * This is not currently enforced in the parser because syntax errors cannot
 * be HH_FIXME'd. Once codebases are clean, we can move this check to the
 * parser itself.
 *)

let error_if_repeated_attribute (attribs : ('ex, 'en) xhp_attribute list) =
  let rec loop attribs (seen : SSet.t) =
    match attribs with
    | Xhp_simple { xs_name = (pos, name); _ } :: _ when SSet.mem name seen ->
      Errors.add_error
        Parsing_error.(
          to_user_error
          @@ Xhp_parsing_error
               { pos; msg = Printf.sprintf "Cannot redeclare %s" name })
    | Xhp_simple { xs_name = (_, name); _ } :: attribs ->
      loop attribs (SSet.add name seen)
    | Xhp_spread _ :: attribs -> loop attribs seen
    | [] -> ()
  in
  loop attribs SSet.empty

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_expr _ (_, _, e) =
      (match e with
      | Aast.Xml (_, attribs, _) -> error_if_repeated_attribute attribs
      | _ -> ());
      ()
  end
