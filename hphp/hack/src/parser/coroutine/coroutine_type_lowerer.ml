(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *)

 module Syntax = Full_fidelity_editable_syntax
 module Token = Full_fidelity_editable_token
 module TokenKind = Full_fidelity_token_kind
 open Coroutine_syntax
 open Syntax

(**
 * We rewrite "coroutine function() : void" as
 * "function(Cont<Unit>) : Result<Unit>", so as to avoid having to construct
 * generic types with void as a type argument.
*)
let is_void node =
  match syntax node with
  | SimpleTypeSpecifier { simple_type_specifier }  ->
    begin
    match syntax simple_type_specifier with
    | Token token -> Token.kind token = TokenKind.Void
    | _ -> false
    end
  | _ -> false

let rewrite_return_type return_type =
  if is_missing return_type then mixed_type
  else if is_void return_type then unit_type_syntax
  else return_type

(**
 * A void function becomes a unit function; if the annotation is missing it is
 * added.
 *)
let rewrite_header_node ({ function_colon; function_type; _; } as node) =
  let function_type = rewrite_return_type function_type in
  let function_colon =
    if is_missing function_colon then colon_syntax else function_colon in
  { node with function_type; function_colon; }
