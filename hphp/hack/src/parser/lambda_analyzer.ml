(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module SourceText = Full_fidelity_source_text
module Syntax = Full_fidelity_editable_positioned_syntax
module Token = Syntax.Token
module TokenKind = Full_fidelity_token_kind
open Syntax

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
  let rec aux acc node =
    if is_lambda_expression node || is_anonymous_function node then
      acc
    else
      let acc = folder acc node in
      List.fold_left ~f:aux ~init:acc (Syntax.children node)
  in
  aux acc node

let token_to_string node =
  match syntax node with
  | Token t -> Some (Token.text t)
  | _ -> None

let param_name node =
  match syntax node with
  | ListItem
      {
        list_item = { syntax = ParameterDeclaration { parameter_name; _ }; _ };
        _;
      } ->
    token_to_string parameter_name
  | _ -> None

let use_name node =
  match syntax node with
  | ListItem { list_item = { syntax = Token _; _ } as list_item; _ } ->
    token_to_string list_item
  | _ -> None

let rec get_params_list node =
  match syntax node with
  | FunctionDeclaration { function_declaration_header; _ } ->
    get_params_list function_declaration_header
  | FunctionDeclarationHeader { function_parameter_list; _ } ->
    function_parameter_list
  | LambdaExpression { lambda_signature; _ } -> get_params_list lambda_signature
  | LambdaSignature { lambda_parameters; _ } -> lambda_parameters
  | AnonymousFunction { anonymous_parameters; _ } -> anonymous_parameters
  | MethodishDeclaration { methodish_function_decl_header; _ } ->
    get_params_list methodish_function_decl_header
  | _ -> make_missing SourceText.empty 0

let get_params node =
  let param_list = get_params_list node in
  let param_list = syntax_node_to_list param_list in
  let param_list = List.filter_map param_list ~f:param_name in
  SSet.of_list param_list

let get_body node =
  match syntax node with
  | FunctionDeclaration { function_body; _ } -> function_body
  | LambdaExpression { lambda_body; _ } -> lambda_body
  | AnonymousFunction { anonymous_body; _ } -> anonymous_body
  | MethodishDeclaration { methodish_function_body; _ } ->
    methodish_function_body
  | _ -> make_missing SourceText.empty 0

(* TODO: This does not consider situations like "${x}" as the use of a local.*)
let add_local acc node =
  match syntax node with
  | Token ({ Token.kind = TokenKind.Variable; _ } as token) ->
    SSet.add (Token.text token) acc
  | _ -> acc

(**
 * Returns a set of all local variables and parameters in a body
 * Excluded variables and parameters in lambdas
 * Locals are stripped of their initial `$`
 * Excludes `$this`
 *)
let all_locals node =
  fold_no_lambdas add_local SSet.empty node |> SSet.remove "$this"

let outer_from_use use_clause =
  match syntax use_clause with
  | AnonymousFunctionUseClause { anonymous_use_variables; _ } ->
    let use_list = syntax_node_to_list anonymous_use_variables in
    let use_list = List.filter_map use_list ~f:use_name in
    SSet.of_list use_list
  | _ -> SSet.empty

let use_clause_variables node =
  match syntax node with
  | AnonymousFunction { anonymous_use; _ } -> outer_from_use anonymous_use
  | _ -> SSet.empty

let local_variables acc node =
  let params = get_params node in
  let body = get_body node in
  let locals = fold_no_lambdas add_local params body in
  let used_vars = use_clause_variables node in
  let acc = SSet.union acc used_vars in
  SSet.union acc locals

let filter_parents parents =
  (* We want all the parents that are relevant to computing outer variables.
  Since outer variables are indicated in a "use" clause of an anonymous
  function, we can stop looking when we encounter one.  Otherwise, we just
  filter out anything that is not a lambda, function or method.
  *)
  let rec aux acc parents =
    match parents with
    | [] -> acc
    | h :: t ->
      begin
        match syntax h with
        | FunctionDeclaration _
        | MethodishDeclaration _
        | AnonymousFunction _ ->
          h :: acc
        | LambdaExpression _ -> aux (h :: acc) t
        | _ -> aux acc t
      end
  in
  List.rev (aux [] parents)

let compute_outer_variables parents node =
  match syntax node with
  | AnonymousFunction { anonymous_use; _ } -> outer_from_use anonymous_use
  | _ ->
    let parents = filter_parents parents in
    List.fold_left parents ~f:local_variables ~init:SSet.empty

let partition_used_locals parents node =
  (* Set of function parameters *)
  let params =
    get_params_list node |> syntax_node_to_list |> List.filter_map ~f:param_name
  in
  let param_set = SSet.of_list params in
  (* Set of all variables referenced in the body except for $this *)
  let all_used =
    fold_no_lambdas add_local SSet.empty (get_body node) |> SSet.remove "$this"
  in
  let all_outer = compute_outer_variables parents node in
  let used_outer = SSet.diff (SSet.inter all_used all_outer) param_set in
  let inner = SSet.diff all_used (SSet.union param_set used_outer) in
  let inner_strings = SSet.to_string inner in
  let used_outer_strings = SSet.to_string used_outer in
  let _ = (inner_strings, used_outer_strings) in
  (inner, used_outer, params)
