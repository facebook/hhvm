(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
module List = Core_list
module Syntax = Full_fidelity_editable_positioned_syntax
module Token = Syntax.Token
module SyntaxKind = Full_fidelity_syntax_kind
module Utils = Full_fidelity_syntax_utilities.WithSyntax(Syntax)
open Syntax
open Utils

(*
 We have a lambda in hand, and the chain of parents in its parse tree.
We wish to know all the local variables which appear immediately in any parent
that are used anywhere inside the lambda. These are the "outer variables" of
the lambda.

This is trickier than it sounds.  Consider the first lambda here:

function foo($a, $e) {
  $b = 123;
  m(
    ($b, $c, $f) ==> m($a, $b, $c, () ==> { ... $d = $e ... }),
    () ==> { ... $d = $b; ... });
}

* $a is an outer variable of the lambda; it appears outside and is used inside.
* $b is not an outer variable of the lambda. It appears outside, but the $b used
  inside the lambda is a different $b.
* $c is not an outer variable of the lambda; it does not appear outside.
* $d is not an outer variable of the lambda, even though it appears outside
  the lambda and is used inside the lambda.
* $e is an outer variable; it appears outside, and is used indirectly inside.
* $f is not an outer variable; it does not appear outside.

The first problem we're going to solve is finding all the variables used
*without recursing into child lambdas*. Variables which are declared as formal
parameters are considered used; they are implicitly assigned a value.

For our example above:

* the variables used by foo are $a, $b and $e
* the variables used by the first lambda are $a, $b, $c, $f
* the variables used by the second lambda are $d and $e
* the variables used by the third lambda are $b and $d

TODO: We need to figure out how to make cases like

{
  $x = 123;
  ... () ==> "${x}" ...
}

work correctly. String identity on locals doesn't really work here.

*)

let fold_no_lambdas folder acc node =
  let predicate node =
    match kind node with
    | SyntaxKind.LambdaExpression
    | SyntaxKind.AnonymousFunction -> false
    | _ -> true in
  fold_where folder predicate acc node

let token_to_string node =
  match syntax node with
  | Token t -> Some (Token.text t)
  | _ -> None

let param_name node =
  match syntax node with
  | ListItem { list_item = {
      syntax = ParameterDeclaration { parameter_name; _ }; _
    }; _ } -> token_to_string parameter_name
  | _ -> None

let rec get_params_list node =
  match syntax node with
  | FunctionDeclaration { function_declaration_header; _ } ->
    get_params_list function_declaration_header
  | FunctionDeclarationHeader { function_parameter_list; _ } ->
    function_parameter_list
  | LambdaExpression { lambda_signature; _ } ->
    get_params_list lambda_signature
  | LambdaSignature { lambda_parameters; _ } ->
    lambda_parameters
  | AnonymousFunction { anonymous_parameters; _ } ->
    anonymous_parameters
  | MethodishDeclaration { methodish_function_decl_header; _ } ->
    get_params_list methodish_function_decl_header
  | _ -> make_missing()

let get_params node =
  let param_list = get_params_list node in
  let param_list = syntax_node_to_list param_list in
  let param_list = List.filter_map param_list ~f:param_name in
  SSet.of_list param_list

let all_params parents =
  let folder acc node =
    SSet.union acc (get_params node) in
  List.fold_left parents ~f:folder ~init:SSet.empty

let get_body node =
  match syntax node with
  | FunctionDeclaration { function_body; _ } -> function_body
  | LambdaExpression { lambda_body; _ } -> lambda_body
  | AnonymousFunction { anonymous_body; _ } -> anonymous_body
  | MethodishDeclaration { methodish_function_body; _ } -> methodish_function_body
  | _ -> make_missing()

(* TODO: This does not consider situations like "${x}" as the use of a local.*)
let add_local acc node =
  match syntax node with
  | VariableExpression { variable_expression =
      { syntax = Token token; _ } } ->
    SSet.add (Token.text token) acc
  | _ -> acc

let local_variables acc node =
  let params = get_params node in
  let body = get_body node in
  let locals = fold_no_lambdas add_local params body in
  SSet.union acc locals

let partition_used_locals parents node =
  let params = get_params_list node in
  let body = get_body node in
  let all_used = fold_no_lambdas add_local SSet.empty body in
  let all_used = SSet.remove "$this" all_used in
  let decls = syntax_node_to_list params in
  let all_params = List.filter_map decls ~f:param_name in
  let all_params = SSet.of_list all_params in
  let used_params = SSet.inter all_used all_params in
  (* TODO: This is incorrect in cases where anonymous functions are in play.
  For example:
  { $x = 1; $y = 2; $f = function () use ($y) { M($x, $y); }; }
  We should compute that $y is an outer variable of the lambda and $x is an
  inner variable, but we compute that both are outer variables.
  We also could have more complex situations:
  {
    $x = 1; $y = 2;
    $f = function () use ($y) {
      $z = 3;
      $f2 = () ==> $x + $y + $z;
    };
  }
  In the inner lambda, $y and $z are outer variables of the lambda, but $x is
  not because $x is not in scope inside the anonymous function.
  *)
  let all_outer = List.fold_left parents ~f:local_variables ~init:SSet.empty in
  let used_outer = SSet.inter all_used all_outer in
  let inner = SSet.diff all_used (SSet.union used_params used_outer) in
  (inner, used_outer, used_params)
