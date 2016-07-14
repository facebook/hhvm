(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module PositionedSyntax = Full_fidelity_positioned_syntax
module SyntaxUtilities =
  Full_fidelity_syntax_utilities.WithSyntax(PositionedSyntax)
module SyntaxError = Full_fidelity_syntax_error

open PositionedSyntax

type accumulator = {
  errors : SyntaxError.t list;
}

(* True or false: the first item in this list matches the predicate? *)
let matches_first f items =
  match items with
  | h :: _ when f h -> true
  | _ -> false

let parent_is_function parents =
  matches_first is_function parents

let statement_directly_in_switch parents =
  match parents with
  | l :: c :: s :: _ when (is_compound_statement c) && (is_list l) &&
    (is_switch_statement s) ->
    true
  | c :: s :: _ when (is_compound_statement c) && (is_switch_statement s) ->
    true
  | _ -> false

let is_bad_xhp_attribute_name name =
  (String.contains name ':') || (String.contains name '-')

let xhp_errors node _parents =
(* An attribute name cannot contain - or :, but we allow this in the lexer
   because it's easier to have one rule for tokenizing both attribute and
   element names. *)
  match syntax node with
  |  XHPAttribute attr when
    (is_bad_xhp_attribute_name (PositionedSyntax.text attr.xhp_attr_name)) ->
      let s = start_offset attr.xhp_attr_name in
      let e = end_offset attr.xhp_attr_name in
      [ SyntaxError.make s e SyntaxError.error2002 ]
  | _ -> [ ]

let parameter_errors node parents is_strict =
  match syntax node with
  | ParameterDeclaration p ->
    if is_strict &&
        (parent_is_function parents) &&
        is_missing (param_type p) then
      let s = start_offset node in
      let e = end_offset node in
      [ SyntaxError.make s e SyntaxError.error2001 ]
    else
      [ ]
  | _ -> [ ]

let function_errors node _parents is_strict =
  match syntax node with
  | FunctionDeclaration f ->
    if is_strict && is_missing (function_type f) then
      (* Where do we want to report the error? Probably on the right paren. *)
      let rparen = function_right_paren f in
      let s = start_offset rparen in
      let e = end_offset rparen in
      [ SyntaxError.make s e SyntaxError.error2001 ]
    else
      [ ]
  | _ -> [ ]

let case_errors node parents =
  match syntax node with
  | CaseStatement _ when not (statement_directly_in_switch parents) ->
    let s = start_offset node in
    let e = end_offset node in
    [ SyntaxError.make s e SyntaxError.error2003 ]
  | _ -> [ ]

let default_errors node parents =
  match syntax node with
  | DefaultStatement _ when not (statement_directly_in_switch parents) ->
    let s = start_offset node in
    let e = end_offset node in
    [ SyntaxError.make s e SyntaxError.error2004 ]
  | _ -> [ ]

let find_syntax_errors node is_strict =
  let folder acc node parents =
    let param_errors = parameter_errors node parents is_strict in
    let func_errors = function_errors node parents is_strict in
    let x_errors = xhp_errors node parents in
    let c_errors = case_errors node parents in
    let d_errors = default_errors node parents in
    let errors = func_errors @ param_errors @ acc.errors @
      x_errors @ c_errors @ d_errors in
    { errors } in
  let acc = SyntaxUtilities.parented_fold_pre folder { errors = [] } node in
  acc.errors
