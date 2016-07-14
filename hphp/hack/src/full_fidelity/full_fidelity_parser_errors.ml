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

let rec break_is_legal parents =
  match parents with
  | h :: _ when is_anonymous_function h -> false
  | h :: _ when is_switch_statement h -> true
  | h :: _ when is_loop_statement h -> true
  | _ :: t -> break_is_legal t
  | [] -> false

let rec continue_is_legal parents =
  match parents with
  | h :: _ when is_anonymous_function h -> false
  | h :: _ when is_loop_statement h -> true
  | _ :: t -> continue_is_legal t
  | [] -> false

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

let statement_errors node parents =
  let result = match syntax node with
  | CaseStatement _
    when not (statement_directly_in_switch parents) ->
    Some (node, SyntaxError.error2003)
  | DefaultStatement _
    when not (statement_directly_in_switch parents) ->
    Some (node, SyntaxError.error2004)
  | BreakStatement _
    when not (break_is_legal parents) ->
    Some (node, SyntaxError.error2005)
  | ContinueStatement _
    when not (continue_is_legal parents) ->
    Some (node, SyntaxError.error2006)
  | TryStatement { catch_clauses; finally_clause; _ }
    when (is_missing catch_clauses) && (is_missing finally_clause) ->
    Some (node, SyntaxError.error2007)
  | _ -> None in
  match result with
  | None -> [ ]
  | Some (error_node, error_message) ->
    let s = start_offset error_node in
    let e = end_offset error_node in
    [ SyntaxError.make s e error_message ]

let find_syntax_errors node is_strict =
  let folder acc node parents =
    let param_errs = parameter_errors node parents is_strict in
    let func_errs = function_errors node parents is_strict in
    let xhp_errs = xhp_errors node parents in
    let statement_errs = statement_errors node parents in
    let errors = acc.errors @ param_errs @ func_errs @
      xhp_errs @ statement_errs in
    { errors } in
  let acc = SyntaxUtilities.parented_fold_pre folder { errors = [] } node in
  List.sort SyntaxError.compare acc.errors
