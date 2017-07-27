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
module Token = Full_fidelity_editable_token
module Syntax = Full_fidelity_editable_syntax
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

let get_params node =
  let rec aux node =
    match syntax node with
    | FunctionDeclaration { function_declaration_header; _ } ->
      aux function_declaration_header
    | FunctionDeclarationHeader { function_parameter_list; _ } ->
      function_parameter_list
    | LambdaExpression { lambda_signature; _ } ->
      aux lambda_signature
    | LambdaSignature { lambda_parameters; _ } ->
      lambda_parameters
    | AnonymousFunction { anonymous_parameters; _ } ->
      anonymous_parameters
    | _ -> make_missing() in
  let param_list = aux node in
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
  | _ -> make_missing()

(* TODO: This does not consider situations like "${x}" as the use of a local.*)
let add_local acc node =
  match syntax node with
  | VariableExpression { variable_expression =
      { syntax = Token { EditableToken.text; _}; _ } } ->
    SSet.add text acc
  | _ -> acc

let local_variables acc node =
  let params = get_params node in
  let body = get_body node in
  let locals = fold_no_lambdas add_local params body in
  SSet.union acc locals

(*
Second problem: what variables appear inside a lambda, including nested lambdas,
which are *not* parameters of any lambda?
*)

let used_non_params node =
  let folder acc node parents =
    (* Note that the parent chain here only goes up to the originally-passed-in
    node; it does not include the parents of *that* node. Typically the node
    will be a lambda. We want to examine all children of that lambda, looking
    for local variables which are not parameters of the current lambda. If
    there are local variables which are *closed-over parameters of an outer
    lambda*, that's great; we don't want to exclude them. *)
    match syntax node with
    | VariableExpression { variable_expression =
        { syntax = Token { EditableToken.text; _}; _ } } ->
      (* TODO: This is very expensive and inefficient. Basically on every
      encounter of a local variable we reconstruct the set of parameters
      in scope from this parent chain. A more efficient approach would be
      to have a scope-aware folder which, instead of passing in a list
      of parents, passes in a set of parameters. *)
      let params = all_params parents in
      if SSet.mem text params then
        acc (* It's a parameter *)
      else
        SSet.add text acc (* Not any parameter. Maybe an outer variable. *)
    | _ -> acc in
  parented_fold_post folder SSet.empty node

let outer_variables parents lambda =
  let all_outer = List.fold_left parents ~f:local_variables ~init:SSet.empty in
  let all_used = used_non_params lambda in
  (* We now know all the local variables created outside the lambda, and we
     know all the variables that are used that are not any of the lambda's
     parameters, or child lambda's parameters.
     The set of outer variables of the lambda is the intersection.  *)
  let outer = SSet.inter all_outer all_used in
  SSet.elements outer
  (* Note that we are guaranteed that the list is sorted. *)

let partition_used_locals parents params body =
  let all_used = fold_no_lambdas add_local SSet.empty body in
  let all_used = SSet.remove "$this" all_used in
  let decls = syntax_node_to_list params in
  let all_params = List.filter_map decls ~f:param_name in
  let all_params = SSet.of_list all_params in
  let used_params = SSet.inter all_used all_params in
  let all_outer = List.fold_left parents ~f:local_variables ~init:SSet.empty in
  let used_outer = SSet.inter all_used all_outer in
  let inner = SSet.diff all_used (SSet.union used_params used_outer) in
  (inner, used_outer, used_params)
