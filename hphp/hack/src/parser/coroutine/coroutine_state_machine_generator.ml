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

(**
 * The set of variable names that should not be hoisted.
 *)
let unhoistable_variables =
  SSet.of_list [
    this_variable;
    coroutine_data_variable;
    coroutine_result_variable;
    exception_variable;
  ]

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
      when not (SSet.mem text unhoistable_variables) ->
        let local_as_name_syntax =
          String_utils.lstrip text "$"
            |> make_token_syntax TokenKind.Name in
        SMap.add text node acc,
        Rewriter.Result.Replace
          (make_member_selection_expression_syntax
            closure_variable_syntax
            local_as_name_syntax)
    | _ -> acc, Rewriter.Result.Keep in
  let locals_and_params, body =
    Rewriter.aggregating_rewrite_post
      rewrite_and_gather_locals_and_params
      node
      SMap.empty in
  body, locals_and_params

(**
 * Transforms suspend expressions recursively.
 *
 * Extracts a "prefix" list of statements, and transforms the suspend expression
 * into a variable that will contain the result of the suspended expression
 * after resumption.
 *
 * In particular, we perform a post-order traversal of the tree. Therefore, we
 * process the deepest suspend operations before ones that depend on them, and
 * we guarantee that we will never encounter another suspend operator while
 * processing a suspend operator. For these suspend operations, we extract a
 * list of statements to execute before the suspend operation. These statements
 * are reported to the calling function. The calling function is responsible for
 * ensuring that these statemnets are executed before the transformed node. The
 * transformed node itself is rewritten into $coroutineData. The list of
 * statements will ensure that $coroutineData contains the result of the suspend
 * operation at the point in time that the suspend operation is completed.
 *
 * As an example, consider this expression.
 *
 *   suspend outerCoroutine(suspend innerCoroutine(), otherMethod())
 *
 * This is a PrefixUnaryExpression and will produce the following statement
 * list. For demonstrative purposes, we assume that the next label number is 1.
 *
 *   $closure->nextLabel = 1;
 *   $coroutineResult = innerCoroutine();
 *   if ($coroutineResult->isSuspended()) {
 *     return $coroutineResult;
 *   }
 *   $coroutineData = $coroutineResult->getResult();
 *   label1:
 *   $closure->coroutineResultData1 = $coroutineData;
 *   if ($exception !== null) {
 *     throw $exception;
 *   }
 *   $closure->nextLabel = 2;
 *   $coroutineResult = outerCoroutine(
 *     $closure->coroutineResultData1,
 *     otherMethod(),
 *   );
 *   if ($coroutineResult->isSuspended()) {
 *     return $coroutineResult;
 *   }
 *   $coroutineData = $coroutineResult->getResult();
 *   label2:
 *   $closure->coroutineResultData2 = $coroutineData;
 *   if ($exception !== null) {
 *     throw $exception;
 *   }
 *
 * The node itself is is rewritten into the following.
 *
 *   $closure->coroutineResultData2
 *)
(*
 * TODO(t17335630): Consider nulling out the $coroutineResultData# after it's
 * been used, for earlier garbage collection. This can be done with a list of
 * statements to be executed *after* the rewritten variable.
 *)
let extract_suspend_statements node next_label =
  let rewrite_suspends_and_gather_prefix_code
      node
      (next_label, prefix_statements_acc) =
    match syntax node with
    | PrefixUnaryExpression {
        prefix_unary_operator =
          { syntax = Token { EditableToken.kind = TokenKind.Suspend; _;}; _; };
        prefix_unary_operand;
      } ->
        let update_next_label_syntax = set_next_label_syntax next_label in

        let assign_coroutine_result_syntax =
          make_assignment_syntax
            coroutine_result_variable
            prefix_unary_operand in

        let select_is_suspended_member_syntax =
          make_member_selection_expression_syntax
            coroutine_result_variable_syntax
            is_suspended_member_syntax in
        let call_is_selected_syntax =
          make_function_call_expression_syntax
            select_is_suspended_member_syntax
            [] in
        let return_coroutine_result_syntax =
          make_return_statement_syntax coroutine_result_variable_syntax in
        let return_if_suspended_syntax =
          make_if_syntax
            call_is_selected_syntax
            [ return_coroutine_result_syntax ] in

        let select_coroutine_result_syntax =
          make_member_selection_expression_syntax
            coroutine_result_variable_syntax
            get_result_member_syntax in
        let call_get_result_syntax =
          make_function_call_expression_syntax
            select_coroutine_result_syntax
            [] in
        let assign_coroutine_data_syntax =
          make_assignment_syntax
            coroutine_data_variable
            call_get_result_syntax in

        let declare_next_label_syntax = goto_label_syntax next_label in

        let coroutine_result_data_variable =
          make_coroutine_result_data_variable next_label in
        let assign_coroutine_result_data_syntax =
          make_assignment_syntax
            coroutine_result_data_variable
            coroutine_data_variable_syntax in

        let exception_not_null_syntax =
          make_not_null_syntax exception_variable_syntax in
        let throw_if_exception_not_null_syntax =
          make_if_syntax
            exception_not_null_syntax
            [ make_throw_statement_syntax exception_variable_syntax ] in

        let statements = [
          update_next_label_syntax;
          assign_coroutine_result_syntax;
          return_if_suspended_syntax;
          assign_coroutine_data_syntax;
          declare_next_label_syntax;
          assign_coroutine_result_data_syntax;
          throw_if_exception_not_null_syntax;
        ] in

        let coroutine_result_data_variable_syntax =
          make_token_syntax TokenKind.Variable coroutine_result_data_variable in

        (next_label + 1, prefix_statements_acc @ statements),
        Rewriter.Result.Replace coroutine_result_data_variable_syntax
    | _ ->
        (next_label, prefix_statements_acc), Rewriter.Result.Keep in
  Rewriter.aggregating_rewrite_post
    rewrite_suspends_and_gather_prefix_code
    node
    (next_label, [])

(**
 * Processes statements that support the suspend keyword.
 *
 * Each statement has a notion of where expressions may exist. For each of these
 * expression points, we recursively desugar the "suspend" operator using
 * extract_suspend_statements. In addition to producing a transformed expression
 * node, this produces a list of statements to be executed before the
 * transformed node. The statement transforms itself so that these statements
 * get executed before the statement itself is executed, and transforms its
 * expression node appropriately.
 *)
let rewrite_suspends node =
  let rewrite_statements node next_label =
    match syntax node with
    | ReturnStatement { return_expression; _; } ->
        let (next_label, prefix_statements), return_expression =
          extract_suspend_statements return_expression next_label in
        let return_expression =
          if is_missing return_expression then coroutine_unit_call_syntax
          else return_expression in
        let return_expression = make_object_creation_expression_syntax
          "ActualCoroutineResult" [return_expression] in
        let assignment = set_next_label_syntax (-1) in
        let ret = make_return_statement_syntax return_expression in
        let statements = prefix_statements @ [ assignment; ret ] in
        let statements = make_compound_statement_syntax statements in
        next_label, Rewriter.Result.Replace statements
    (* The if_else_clause is simply an else_clause which contains an
       if_statement. Thus, we need no special logic to rewrite the
       if_else_clause as long as the generated code is contained within a
       compound_statement. *)
    | IfStatement ({ if_condition; if_elseif_clauses; _; } as node) ->
        (* TODO(t17335630): Handle if_elseif_clauses by rewriting ones
           containing suspensions into else_caluses. *)
        assert (is_missing if_elseif_clauses);

        let (next_label, prefix_statements), if_condition =
          extract_suspend_statements if_condition next_label in
        let new_if = make_syntax (IfStatement { node with if_condition; }) in
        let statements = prefix_statements @ [ new_if ] in
        let statements = make_compound_statement_syntax statements in
        next_label, Rewriter.Result.Replace statements
    | ExpressionStatement _ (* TODO(t17335630): Support suspends here. *)
    | UnsetStatement _ (* TODO(t17335630): Support suspends here. *)
    | WhileStatement _ (* TODO(t17335630): Support suspends here. *)
    | DoStatement _ (* TODO(t17335630): Support suspends here. *)
    | ForStatement _ (* TODO(t17335630): Support suspends here. *)
    | ForeachStatement _ (* TODO(t17335630): Support suspends here. *)
    | SwitchStatement _ (* TODO(t17335630): Support suspends here. *)
    | ThrowStatement _ (* TODO(t17335630): Support suspends here. *)
    | EchoStatement _ (* TODO(t17335630): Support suspends here. *)
    (* Suspends will be handled recursively by compound statement's children. *)
    | CompoundStatement _
    (* Suspends will be handled recursively by try statements's children. *)
    | TryStatement _
    | GotoStatement _ (* Suspends are invalid in goto statements. *)
    | BreakStatement _ (* Suspends are impossible in break statements. *)
    | ContinueStatement _ (* Suspends are impossible in continue statements. *)
    | FunctionStaticStatement _ (* Suspends are impossible in these. *)
    | GlobalStatement _ (* Suspends are impossible in global statements. *)
    | _ ->
        next_label, Rewriter.Result.Keep in
  Rewriter.aggregating_rewrite_post rewrite_statements node 1

let add_switch (next_label, body) =
  make_compound_statement_syntax [
    make_coroutine_switch (next_label - 1);
    goto_label_syntax 0;
    body;
    error_label_syntax;
    throw_unimplemented_syntax "A completed coroutine was resumed.";
  ]

let lower_body body =
  body
    |> add_missing_return
    |> rewrite_suspends
    |> add_switch
    |> rewrite_locals_and_params_into_closure_variables

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
  let body, locals_and_params = lower_body methodish_function_body in
  let state_machine_data =
    compute_state_machine_data locals_and_params function_node in
  let state_machine_method =
    make_methodish_declaration_with_body_syntax
      (make_function_decl_header classish_name function_name function_node)
      body in
  state_machine_method, state_machine_data
