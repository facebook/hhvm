(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *)

module CoroutineStateMachineData = Coroutine_state_machine_data
module CoroutineSyntax = Coroutine_syntax
module EditableSyntax = Full_fidelity_editable_syntax
module EditableToken = Full_fidelity_editable_token
module Rewriter = Full_fidelity_rewriter.WithSyntax(EditableSyntax)
module TokenKind = Full_fidelity_token_kind

open EditableSyntax
open CoroutineSyntax

(*
Consider a coroutine such as
coroutine function foo() : void {
  suspend blah();
  suspend blah();
}

There is an implicit "return;" at the end, and we'll need to rewrite this as

function foo_rewritten(...) : CoroutineResult<Unit> {
   ...
   $closure->label = -1;
   return new ActualCoroutineResult(coroutine_unit());
   ...
}

However we can avoid introducing the extra return if the end point of the
method is unreachable. A method which already ends with a return or throw
has an unreachable endpoint, so there would be no way to reach the
introduced return.

As a small optimization we avoid generating the extra return if we know that
the method endpoint is reachable.

TODO: Is this optimization reasonable? We could simply generate it always.
It doesn't add that much extra code.
*)

let rec has_reachable_endpoint body =
  (* TODO: This method underestimates. Better to underestimate than to
  overestimate, but still we could be smarter. For example we do not
  detect if(x) {return;} else {return;} as having an unreachable end point. *)
  match syntax body with
  | ReturnStatement _
  | ThrowStatement _
  | GotoStatement _ -> false
  | CompoundStatement { compound_statements; _; } ->
    has_reachable_endpoint compound_statements
  | SyntaxList [] -> true
  | SyntaxList items ->
    has_reachable_endpoint (List.hd (List.rev items))
  (* TODO: Fix this -- false if all sections are false and no default *)
  | SwitchStatement _ -> true
  (* TODO: Fix this -- false if consequence and all alternatives are false. *)
  | IfStatement _ -> true
  (* TODO: Fix this -- is complicated. *)
  | TryStatement _ -> true
  | ForeachStatement _
  | GotoLabel _
  | Missing
  | ExpressionStatement _
  | UnsetStatement _
  | WhileStatement _
  | DoStatement _
  | ForStatement _
  | FunctionStaticStatement _
  | EchoStatement _
  | GlobalStatement _
  | _ -> true

let add_missing_return body =
  (* Put a "return;" on the bottom if there's not one there already. *)
  if has_reachable_endpoint body then
    make_compound_statement_syntax [
      body;
      make_return_missing_statement_syntax;
    ]
  else
    body

(* return x; --> return new ActualCoroutineResult(x);
   return;   --> return new ActualCoroutineResult(coroutine_unit());
*)
let rewrite_returns node =
  (* TODO: We need a rewriter that doesn't recurse into lambdas *)
  match syntax node with
  | ReturnStatement {
    return_keyword;
    return_expression;
    return_semicolon; } ->
    let return_expression =
      if is_missing return_expression then coroutine_unit_call_syntax
      else return_expression in
    let return_expression = make_object_creation_expression_syntax
      "ActualCoroutineResult" [return_expression] in
    let assignment = set_next_label_syntax (-1) in
    let ret = make_return_statement
      return_keyword return_expression return_semicolon in
    let result = make_compound_statement_syntax [assignment; ret] in
    Rewriter.Result.Replace result
  | _ -> Rewriter.Result.Keep

(**
 * Converts references to parameters and locals into referenced to the hoisted
 * versions of these identifiers that are part of the generated closure.
 *
 * If a function refers to a local or parameter variable $foo, then $foo should
 * be made into a member on the closure.
 *
 * $this is not hoisted, since its state will be persisted between state machine
 * resumptions automatically.
 *)
let rewrite_locals_and_params_into_closure_variables node =
  let rewrite_and_gather_locals_and_params node acc =
    match syntax node with
    | Token { EditableToken.kind = TokenKind.Variable; text; _; }
      when text <> this_variable ->
        let local_as_name_syntax =
          String_utils.lstrip text "$"
            |> make_token_syntax TokenKind.Name in
        SMap.add text node acc,
        Rewriter.Result.Replace
          (make_member_selection_expression_syntax
            closure_variable_syntax
            local_as_name_syntax)
    | _ -> acc, Rewriter.Result.Keep in
  Rewriter.aggregating_rewrite_post
    rewrite_and_gather_locals_and_params
    node
    SMap.empty

let lower_body body =
  let locals_and_params, body =
    rewrite_locals_and_params_into_closure_variables body in
  let body = add_missing_return body in
  Rewriter.rewrite_post rewrite_returns body, locals_and_params

let generate_coroutine_state_machine_body body =
  let body, locals_and_params = lower_body body in
  let body = [
    make_coroutine_switch 0; (* TODO: Need label count. *)
    goto_label_syntax 0;
    body;
    error_label_syntax;
    throw_unimplemented_syntax "A completed coroutine was resumed.";
  ] in
  body, locals_and_params

let make_function_decl_header
    classish_name
    function_name
    { function_type; _; } =
  make_function_decl_header_syntax
    (make_state_machine_method_name function_name)
    [
      make_closure_parameter_syntax classish_name function_name;
      coroutine_data_parameter_syntax;
      nullable_exception_parameter_syntax;
    ]
    (make_coroutine_result_type_syntax function_type)

let extract_parameter_declarations { function_parameter_list; _; } =
  function_parameter_list
    |> syntax_node_to_list
    |> Core_list.map ~f:syntax
    |> Core_list.map ~f:
      begin
      function
      | ListItem { list_item = { syntax = ParameterDeclaration p; _; }; _; } ->
          p
      | _ -> failwith "Parameter had unexpected type."
      end

let compute_state_machine_data locals_and_params function_node =
  let parameters = extract_parameter_declarations function_node in
  let parameter_names =
    parameters
      |> Core_list.map ~f:
        begin
        function
        | {
            parameter_name =
              {
                syntax =
                  Token { EditableToken.kind = TokenKind.Variable; text; _; };
                _;
              };
            _;
          } ->
            text
        | _ -> failwith "Parameter had unexpected token."
        end
      |> SSet.of_list in
  let local_variables =
    SMap.filter
      (fun k _ -> not (SSet.mem k parameter_names))
      locals_and_params in
  CoroutineStateMachineData.{ local_variables; parameters; }

(**
 * If the provided methodish declaration is for a coroutine, rewrites the
 * declaration header and the function body into a desugared coroutine
 * implementation.
 *)
let generate_coroutine_state_machine
    classish_name
    function_name
    { methodish_function_body; _; }
    function_node =
  let body, locals_and_params =
    generate_coroutine_state_machine_body methodish_function_body in
  let state_machine_data =
    compute_state_machine_data locals_and_params function_node in
  let state_machine_method =
    make_methodish_declaration_syntax
      (make_function_decl_header classish_name function_name function_node)
      body in
  state_machine_method, state_machine_data
