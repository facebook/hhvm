(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *)

module CoroutineClosureGenerator = Coroutine_closure_generator
module CoroutineStateMachineData = Coroutine_state_machine_data
module CoroutineSyntax = Coroutine_syntax
module EditableSyntax = Full_fidelity_editable_syntax
module EditableToken = Full_fidelity_editable_token
module Rewriter = Full_fidelity_rewriter.WithSyntax(EditableSyntax)
module Utils = Full_fidelity_syntax_utilities.WithSyntax(EditableSyntax)

open EditableSyntax
open CoroutineSyntax

type label =
  | StateLabel of int
  | ErrorStateLabel
  | LoopLabel of int

let get_label_string = function
  | StateLabel number -> Printf.sprintf "state_label_%d" number
  | ErrorStateLabel -> "state_label_error"
  | LoopLabel number -> Printf.sprintf "loop_label_%d" number

let get_label_name label =
  make_name_syntax (get_label_string label)

let make_label_declaration_syntax label =
  CoroutineSyntax.make_label_declaration_syntax (get_label_name label)

let make_goto_statement_syntax label =
  CoroutineSyntax.make_goto_statement_syntax (get_label_name label)

(* checks if one of node's parents is a try-block of try-statement
   NOTE: this function relies on physical identity of nodes being the same *)
let rec is_in_try_block node parents =
  match parents with
  | [] -> false
  | { syntax = TryStatement { try_compound_statement; _ }; _ } :: _
      when node == try_compound_statement ->
    true
  | x :: xs -> is_in_try_block x xs

(* given a node an a list of its ancestors [p1; p2; p3; ...]
   checks nodes pairwise (node, p1), (p1, p2), (p2, p3) to make sure that
   first node in the pair appear in the tail position within a second node*)
let rec is_in_tail_position node parents =
  match parents with
  | [] -> true
  | { syntax = ParenthesizedExpression {
        parenthesized_expression_expression = e; _
      }; _
    } as n :: xs when node == e -> is_in_tail_position n xs
  | _ -> false

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

(* This gives the names of all local variables and parameters in a body:
* locals and parameters occurring only in lambdas are not included.
* Unused parameters are not included.
* Locals are stripped of their initial $
* TODO: We could further filter these down to only locals that are used
        across a suspend. If we do, rename this method, as it will then
        no longer do what it says on the tin.
* TODO: Locals used in PHP style constructs like "${x}" are not captured.
*)
let all_used_locals node =
  let folder acc node =
    match syntax node with
    | Token { EditableToken.kind = TokenKind.Variable; text; _; } ->
      (* "$this" is treated as a local, but obviously we don't want to copy
      it in or out. *)
      if text = "$this" then acc else SSet.add text acc
    | _ -> acc in
  let locals = Lambda_analyzer.fold_no_lambdas folder SSet.empty node in
  locals (* TODO: Return a list *)

(* $closure->name = $name *)
let copy_in_syntax variable =
  let field_name = String_utils.lstrip variable "$" in
  make_assignment_syntax_variable
    (closure_name_syntax field_name)
    (make_variable_syntax variable)

(* $name = $closure->name *)
let copy_out_syntax variable =
  let field_name = String_utils.lstrip variable "$" in
  make_assignment_syntax_variable
    (make_variable_syntax variable)
    (closure_name_syntax field_name)

let add_try_finally used_locals body =
  let copy_in = Core_list.map ~f:copy_in_syntax used_locals in
  let copy_out = Core_list.map ~f:copy_out_syntax used_locals in
  let copy_out = make_compound_statement_syntax copy_out in
  let copy_in = make_compound_statement_syntax copy_in in
  let try_body = make_compound_statement_syntax [ body ] in
  let try_statement = make_try_finally_statement_syntax try_body copy_in in
  make_compound_statement_syntax [ copy_out; try_statement ]

(**
 * Recursively rewrites do-while constructs into while constructs.
 *
 *   do {
 *     body
 *   } while (condition);
 *
 * Becomes
 *
 *   goto loop_label_#;
 *   while (condition) {
 *     loop_label_#:
 *     body
 *   }
 *)
let rewrite_do next_loop_label node =
  let rewrite node next_loop_label =
    match syntax node with
    | DoStatement { do_condition; do_body; _; } ->
        let loop_label = LoopLabel next_loop_label in
        let next_loop_label = next_loop_label + 1 in

        let goto_statement_syntax = make_goto_statement_syntax loop_label in

        let new_while_statement_syntax =
          make_while_syntax
            do_condition
            [
              make_label_declaration_syntax loop_label;
              do_body;
            ] in

        let statements =
          make_compound_statement_syntax
            [
              goto_statement_syntax;
              new_while_statement_syntax;
            ] in
        next_loop_label, Rewriter.Result.Replace statements
    | _ ->
        next_loop_label, Rewriter.Result.Keep in
  Rewriter.aggregating_rewrite_post rewrite node next_loop_label

(**
 * Recursively rewrites while-condition statements into
 * while-true-with-if-condition constructs.
 *
 *   while (condition) {
 *     body
 *   }
 *
 * Becomes
 *
 *   while (true) {
 *     if (!condition) {
 *       break;
 *     }
 *     body
 *   }
 *)
let rewrite_while node =
  let rewrite node =
    match syntax node with
    | WhileStatement { while_condition; while_body; _; } ->
        let new_while_statement =
          make_while_true_syntax
            [
              make_break_unless_syntax while_condition;
              while_body;
            ] in
        Rewriter.Result.Replace new_while_statement
    | _ ->
        Rewriter.Result.Keep in
  Rewriter.rewrite_post rewrite node

(**
 * Extracts expressions from a for statement as expression statement syntaxes.
 *
 * A for loop has the following format:
 *
 *   for (
 *     initializer_exprs;
 *     control_exprs_and_condition_expr;
 *     end_of_loop_exprs
 *   ) {
 *     body
 *   }
 *
 * initializer_exprs, control_exprs_and_condition_expr, and end_of_loop_exprs
 * are each comma-delimited lists of expressions.
 * control_exprs_and_condition_expr is a glob that represents one of the
 * following:
 *
 *   1) Zero or more control_exprs followed by one condition_expr; or
 *   2) Zero control_exprs followed by an empty condition_expr, which is
 *      implicitly considered to be an expression that always evaluates to true.
 *
 * This function extracts the following:
 *
 *   1) initializer_exprs as a list of ExpressionStatements
 *   2) control_exprs as a list of ExpressionStatements
 *   3) condition_expr as an ExpressionStatement
 *   4) end_of_loop_exprs as a list of ExpressionStatements
 *)
let extract_expressions_from_for_statement
    { for_initializer; for_control; for_end_of_loop; _; } =
  let make_expression_statement_from_list_item node =
    make_expression_statement_syntax (get_list_item node) in
  let make_expression_statements list_syntax =
    list_syntax
      |> syntax_node_to_list
      |> Core_list.map ~f:make_expression_statement_from_list_item in

  let initializer_exprs = make_expression_statements for_initializer in
  let control_exprs, condition_expr =
    match Core_list.rev (syntax_node_to_list for_control) with
    | [] -> [], true_expression_syntax
    | condition_expr :: rev_control_exprs ->
      Core_list.rev_map
        ~f:make_expression_statement_from_list_item rev_control_exprs,
      condition_expr in
  let end_of_loop_exprs = make_expression_statements for_end_of_loop in
  initializer_exprs, control_exprs, condition_expr, end_of_loop_exprs

(**
 * Re-expresses for statements as while statements.
 *
 *   for (
 *     initializer_exprs;
 *     [control_exprs...,] condition_expr;
 *     end_of_loop_exprs
 *   ) {
 *     body
 *   }
 *
 * Becomes
 *
 *   initializer_exprs
 *   goto loop_label_#;
 *   while (true) {
 *     end_of_loop_exprs
 *
 *     loop_label_#:
 *     control_exprs
 *     if (!condition_expr) {
 *      break;
 *     }
 *     body
 *   }
 *)
let rewrite_for next_loop_label node =
  let rewrite node next_loop_label =
    match syntax node with
    | ForStatement ({ for_body; _; } as node) ->
        let
          initializer_exprs,
          control_exprs,
          condition_expr,
          end_of_loop_exprs =
            extract_expressions_from_for_statement node in
        let loop_label = LoopLabel next_loop_label in
        let next_loop_label = next_loop_label + 1 in

        let goto_statement_syntax = make_goto_statement_syntax loop_label in

        let while_statements =
          end_of_loop_exprs @
          make_label_declaration_syntax loop_label ::
          control_exprs @
          make_break_unless_syntax condition_expr ::
          for_body ::
          [] in
        let while_syntax = make_while_true_syntax while_statements in
        let statements =
          initializer_exprs @
          goto_statement_syntax ::
          while_syntax ::
          [] in
        let statements = make_compound_statement_syntax statements in
        next_loop_label, Rewriter.Result.Replace statements
    | _ ->
        next_loop_label, Rewriter.Result.Keep in
  Rewriter.aggregating_rewrite_post rewrite node next_loop_label

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
 *   $coroutineResult = innerCoroutine($closure);
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
 *     $closure,
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
let extract_suspend_statements ~parented_by_return_in_tail_position node next_label =
  let rewrite_suspends_and_gather_prefix_code
      parents
      node
      (next_label, has_suspend_in_tail_position, prefix_statements_acc) =
    match syntax node with
    | PrefixUnaryExpression {
        prefix_unary_operator = {
          syntax = Token { EditableToken.kind = TokenKind.Suspend; _; };
          _;
        };
        prefix_unary_operand = {
          syntax = FunctionCallExpression ({
            function_call_argument_list;
            _;
          } as function_call_expression);
          _;
        };
      } ->
        if parented_by_return_in_tail_position && is_in_tail_position node parents
        then
          (* coroutine is in tail position - can call it and pass continuation
            from the enclosing function *)
          let function_call_argument_list =
            prepend_to_comma_delimited_syntax_list
              continuation_variable_syntax
              function_call_argument_list in
          let invoke_coroutine =
            FunctionCallExpression {
              function_call_expression with function_call_argument_list
            } in
          let invoke_coroutine_syntax = make_syntax invoke_coroutine in
          (next_label, true, prefix_statements_acc),
          Rewriter.Result.Replace invoke_coroutine_syntax
        else
        let update_next_label_syntax = set_next_label_syntax next_label in

        let function_call_argument_list =
          prepend_to_comma_delimited_syntax_list
            closure_variable_syntax
            function_call_argument_list in
        let invoke_coroutine =
          FunctionCallExpression {
            function_call_expression with function_call_argument_list
          } in
        let invoke_coroutine_syntax = make_syntax invoke_coroutine in

        let assign_coroutine_result_syntax =
          make_assignment_syntax
            coroutine_result_variable
            invoke_coroutine_syntax in

        let select_is_suspended_member_syntax =
          make_member_selection_expression_syntax
            coroutine_result_variable_syntax
            is_suspended_member_syntax in
        let call_is_suspended_syntax =
          make_function_call_expression_syntax
            select_is_suspended_member_syntax
            [] in
        let return_coroutine_result_syntax =
          make_return_statement_syntax
            create_suspended_coroutine_result_syntax in
        let return_if_suspended_syntax =
          make_if_syntax
            call_is_suspended_syntax
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

        let declare_next_label_syntax =
          make_label_declaration_syntax (StateLabel next_label) in

        let coroutine_result_data_variable_syntax =
          make_member_selection_expression_syntax
            closure_variable_syntax
            (make_coroutine_result_data_member_name_syntax next_label) in
        let assign_coroutine_result_data_syntax =
          make_assignment_syntax_variable
            coroutine_result_data_variable_syntax
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

        (next_label + 1, has_suspend_in_tail_position, prefix_statements_acc @ statements),
        Rewriter.Result.Replace coroutine_result_data_variable_syntax
    | _ ->
        (next_label, has_suspend_in_tail_position, prefix_statements_acc),
        Rewriter.Result.Keep in
  Rewriter.parented_aggregating_rewrite_post
    rewrite_suspends_and_gather_prefix_code
    node
    (next_label, false, [])

let get_token node =
  match EditableSyntax.get_token node with
  | Some token -> token
  | None -> failwith "expected a token"

let rec rewrite_if_statement next_label if_stmt =
  let { if_condition; if_elseif_clauses; if_else_clause; _; } = if_stmt in
  match syntax_node_to_list if_elseif_clauses with
  | [] ->
  (* We have
    if (suspend x()) a; else b;
    Rewrite this as
    $t = suspend x();
    if ($t) a; else b;
  *)
    let (next_label, _, prefix_statements), if_condition =
      extract_suspend_statements
        ~parented_by_return_in_tail_position:false
        if_condition next_label in
    let new_if = make_syntax (IfStatement { if_stmt with if_condition; }) in
    let statements = prefix_statements @ [ new_if ] in
    let statements = make_compound_statement_syntax statements in
    (next_label, statements)
  | h :: t ->

  (* We have, say

  if (suspend x()) a;
  elseif(suspend y()) b;
  elseif (suspend z()) c;
  else d;

  The recursion here is slightly tricky. We know that we can reduce the first
  "elseif" to an "else if". That is, the code above is equivalent to:

  if (suspend x()) a;
  else {
    if(suspend y()) b;
    elseif (suspend z()) c;
    else d;
  }

  So the first thing we do is construct the interior "if". It has one fewer
  "elseif" than before, so we've made a smaller problem that we can solve
  recursively. We recursively rewrite it, and then that becomes the "else"
  of the outer "if". Finally we can recurse a second time to rewrite the outer
  "if". The final result is:

  {
    $tx = suspend x();
    if ($tx) a;
    else {
      $ty = suspend y();
      if($ty) b;
      else {
        $tz = suspend z();
        if ($tz) c;
        else d;
      }
    }
  }
  *)

    begin
    match syntax h with
    | ElseifClause {
      elseif_keyword;
      elseif_left_paren;
      elseif_condition;
      elseif_right_paren;
      elseif_statement } ->
      let child_if_keyword = get_token elseif_keyword in
      let child_if_keyword =
        EditableToken.with_kind child_if_keyword TokenKind.If in
      let child_if_keyword = EditableToken.with_text child_if_keyword "if" in
      let child_if_keyword = make_token child_if_keyword in
      let child_if = {
        if_keyword = child_if_keyword;
        if_left_paren = elseif_left_paren;
        if_condition = elseif_condition;
        if_right_paren = elseif_right_paren;
        if_statement = elseif_statement;
        if_elseif_clauses = make_list t;
        if_else_clause } in
      let (next_label, child_if) = rewrite_if_statement next_label child_if in
      let new_else_clause = make_else_clause else_keyword_syntax child_if in
      let new_if = { if_stmt with
        if_elseif_clauses = make_missing();
        if_else_clause = new_else_clause
      } in
      rewrite_if_statement next_label new_if
    | _ ->
      failwith "Malformed elseif clause"
    end

(**
 * Given a syntax representing a syntax_list of list_items of expressions,
 * this function runs extract_suspend_statements on each of those syntaxes. It
 * gathers the prefix_statements into a single list of all of the
 * prefix_statements concatenated together, and gathers all of the expressions
 * to use in place of the original expressions into a single list. It also
 * returns the next_label.
 *)
let extract_suspend_statements_and_gather_rewrite_data_for_expressions
    next_label
    expressions_syntax_list =
  let extract_suspend_statements_and_gather_rewrite_data
      expression
      (next_label, prefix_statements_acc, expressions_acc) =
    let (next_label, _, prefix_statements), expression =
      extract_suspend_statements
        ~parented_by_return_in_tail_position:false
        expression next_label in
    next_label,
    prefix_statements @ prefix_statements_acc,
    expression :: expressions_acc in
  expressions_syntax_list
    |> syntax_node_to_list
    |> Core_list.map ~f:get_list_item
    |> Core_list.fold_right
      ~f:extract_suspend_statements_and_gather_rewrite_data
      ~init:(next_label, [], [])

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
  let rewrite_statements ancestors node next_label =
    (* TODO(tingley/ericlippert): We don't want to rewrite lambdas. The Rewriter
       does not have the capability to "rewrite_where" -- we should add it,
       similarly to the lambda_analyzer. *)
    if Core_list.exists
        ~f:(fun node -> is_lambda_expression node || is_anonymous_function node)
        ancestors then
      next_label, Rewriter.Result.Keep
    else
      match syntax node with
      | ReturnStatement { return_expression; _; } ->
          let in_tail_position = not (is_in_try_block node ancestors) in
          let (next_label, has_suspend_in_tail_position, prefix_statements),
            return_expression =
            extract_suspend_statements
              ~parented_by_return_in_tail_position:in_tail_position
              return_expression next_label in
          if has_suspend_in_tail_position
          then
            (* special case for tail positioned:
                  return suspend someCoroutine()
               replace it with
                  return <rewritten return expression>
            *)
            let return_statement =
              make_return_statement_syntax return_expression in
            let statements =
              let statements = prefix_statements @ [return_statement] in
              make_compound_statement_syntax statements
            in
            next_label, Rewriter.Result.Replace statements
          else
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
      | IfStatement node ->
        let (next_label, statements) = rewrite_if_statement next_label node in
        next_label, Rewriter.Result.Replace statements
      | ExpressionStatement ({ expression_statement_expression; _; } as node) ->
          let (next_label, _, prefix_statements), expression_statement_expression =
            extract_suspend_statements
              ~parented_by_return_in_tail_position:false
              expression_statement_expression
              next_label in
          let new_expression_statement =
            make_syntax
              (ExpressionStatement
                { node with expression_statement_expression; }) in
          let statements = prefix_statements @ [ new_expression_statement ] in
          let statements = make_compound_statement_syntax statements in
          next_label, Rewriter.Result.Replace statements
      | SwitchStatement ({ switch_expression; _; } as node) ->
          let (next_label, _, prefix_statements), switch_expression =
            extract_suspend_statements
              ~parented_by_return_in_tail_position:false
              switch_expression next_label in
          let new_switch_statement =
            make_syntax (SwitchStatement { node with switch_expression; }) in
          let statements = prefix_statements @ [ new_switch_statement; ] in
          let statements = make_compound_statement_syntax statements in
          next_label, Rewriter.Result.Replace statements
      | ThrowStatement ({ throw_expression; _; } as node) ->
          let (next_label, _, prefix_statements), throw_expression =
            extract_suspend_statements
              ~parented_by_return_in_tail_position:false
              throw_expression next_label in
          let new_throw_statement =
            make_syntax (ThrowStatement { node with throw_expression; }) in
          let statements = prefix_statements @ [ new_throw_statement; ] in
          let statements = make_compound_statement_syntax statements in
          next_label, Rewriter.Result.Replace statements
      | ForeachStatement ({ foreach_collection; _; } as node) ->
          let (next_label, _, prefix_statements), foreach_collection =
            extract_suspend_statements
              ~parented_by_return_in_tail_position:false
              foreach_collection next_label in
          let new_foreach_collection=
            make_syntax (ForeachStatement { node with foreach_collection; }) in
          let statements = prefix_statements @ [ new_foreach_collection; ] in
          let statements = make_compound_statement_syntax statements in
          next_label, Rewriter.Result.Replace statements
      | EchoStatement ({ echo_expressions; _; } as node) ->
          let next_label, prefix_statements, echo_expressions =
            extract_suspend_statements_and_gather_rewrite_data_for_expressions
              next_label
              echo_expressions in
          let echo_expressions =
            make_delimited_list comma_syntax echo_expressions in
          let new_echo_statement =
            make_syntax (EchoStatement { node with echo_expressions; }) in
          let statements = prefix_statements @ [ new_echo_statement; ] in
          let statements = make_compound_statement_syntax statements in
          next_label, Rewriter.Result.Replace statements
      | UnsetStatement ({ unset_variables; _; } as node) ->
          let next_label, prefix_statements, unset_variables =
            extract_suspend_statements_and_gather_rewrite_data_for_expressions
              next_label
              unset_variables in
          let unset_variables = make_delimited_list comma_syntax unset_variables in
          let new_unset_statement =
            make_syntax (UnsetStatement { node with unset_variables; }) in
          let statements = prefix_statements @ [ new_unset_statement; ] in
          let statements = make_compound_statement_syntax statements in
          next_label, Rewriter.Result.Replace statements
      (* while-condition constructs should have already been rewritten into
         while-true-with-if-condition constructs. *)
      | WhileStatement _
      (* for constructs should have already been rewritten into
         while-true-with-if-condition constructs. *)
      | ForStatement _
      (* do-while constructs should have already been rewritten into
         while-true-with-if-condition constructs. *)
      | DoStatement _
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
  Rewriter.parented_aggregating_rewrite_post rewrite_statements node 1

(* case 0: goto state_label_0; *)
let make_switch_section_syntax number =
  let label = make_int_case_label_syntax number in
  let statement = make_goto_statement_syntax (StateLabel number) in
  let fallthrough = make_missing() in
  make_switch_section label statement fallthrough

let default_section_syntax =
  let statement = make_goto_statement_syntax ErrorStateLabel in
  make_switch_section default_label_syntax statement (make_missing())

let make_switch_sections number =
  let rec aux n acc =
    if n < 0 then acc
    else aux (n - 1) ((make_switch_section_syntax n) :: acc) in
  make_list (aux number [ default_section_syntax ])

(*
  switch($closure->nextLabel) {
    case 0: goto state_label_0;
    ...
    default: goto labelerror;
  } *)
let make_coroutine_switch label_count =
  let sections = make_switch_sections label_count in
  make_switch_statement switch_keyword_syntax left_paren_syntax
    label_syntax right_paren_syntax left_brace_syntax sections
    right_brace_syntax

let add_switch (next_label, body) =
  make_compound_statement_syntax [
    make_coroutine_switch (next_label - 1);
    make_label_declaration_syntax (StateLabel 0);
    body;
    make_label_declaration_syntax ErrorStateLabel;
    throw_unimplemented_syntax "A completed coroutine was resumed.";
  ]

(**
 * Recurively unnests compound_statements that are *direct* children of another
 * compound_statement. To be conservative, we only collapse when both the
 * compound_statement and its parent have curly braces.
 *)
let unnest_compound_statements node =
  let get_braced_statements node =
    match syntax node with
    | CompoundStatement {
        compound_left_brace;
        compound_statements;
        compound_right_brace;
      } when (
        not (is_missing compound_left_brace) &&
        not (is_missing compound_right_brace)
      ) ->
        Some (syntax_node_to_list compound_statements)
    | _ -> None in
  let rewrite node =
    match get_braced_statements node with
    | Some statements ->
        statements
          |> Core_list.concat_map
            ~f:(fun node ->
              Option.value (get_braced_statements node) ~default:[ node ])
          |> make_compound_statement_syntax
          |> fun statements -> Rewriter.Result.Replace statements
    | None -> Rewriter.Result.Keep in
  Rewriter.rewrite_post rewrite node

let lower_body body =
  let used_locals = all_used_locals body in
  let used_locals = SSet.elements used_locals in
  let body = add_missing_return body in
  let (next_loop_label, body) = rewrite_do 0 body in
  let body = rewrite_while body in
  let (next_loop_label, body) = rewrite_for next_loop_label body in
  let (next_loop_label, body) = rewrite_suspends body in
  let body = add_switch (next_loop_label, body) in
  let body = add_try_finally used_locals body in
  let body = unnest_compound_statements body in
  let coroutine_result_data_variables =
    next_loop_label
      |> Core_list.range 1
      |> Core_list.map ~f:make_coroutine_result_data_variable in
  (body, coroutine_result_data_variables)

let make_closure_lambda_signature
    context
    function_type =
  (*
  ( C_foo_GeneratedClosure $closure,
    mixed $coroutineData,
    ?Exception $exception) : CoroutineResult<Unit>
  *)
  make_lambda_signature_syntax
    [
      make_closure_parameter_syntax context;
      coroutine_data_parameter_syntax;
      nullable_exception_parameter_syntax;
    ]
    (make_coroutine_result_type_syntax function_type)

let extract_parameter_declarations function_parameter_list =
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

let make_outer_param outer_variable =
  {
    parameter_attribute = make_missing();
    parameter_visibility = public_syntax;
    parameter_type = make_missing();
    parameter_name = make_variable_syntax outer_variable;
    parameter_default_value = make_missing();
  }

let make_outer_params outer_variables =
  Core_list.map ~f:make_outer_param outer_variables

let compute_state_machine_data
    (inner_variables, outer_variables, _used_params)
    coroutine_result_data_variables
    function_parameter_list =
  (* TODO: Add a test case for "..." param. *)
  (* TODO: Right now we get this wrong; the outer variables have to be
  on the parameters list, and not the properties list.  That means that
  we'll need to fix up create_closure_invocation to pass in the outer
  variables. When that happens, fix this code too. *)

  (* TODO: Don't put the outer variables in the property list. *)
  let properties = SSet.union inner_variables outer_variables in
  let properties = SSet.elements properties in
  let properties = properties @ coroutine_result_data_variables in

  let parameters = extract_parameter_declarations function_parameter_list in
  (* TODO: Add the outer_variables to the params as follows. *)
  (* let outer_variables = SSet.elements outer_variables in
  let outer_variable_params = make_outer_params outer_variables in
  let parameters = outer_variable_params @ parameters in *)
  CoroutineStateMachineData.{ properties; parameters; }

(**
 * If the provided methodish declaration is for a coroutine, rewrites the
 * declaration header and the function body into a desugared coroutine
 * implementation.
 *)
let generate_coroutine_state_machine
    context
    original_body
    function_type
    function_parameter_list =
  let new_body, coroutine_result_data_variables =
    lower_body original_body in
  let used_locals = Lambda_analyzer.partition_used_locals
    context.Coroutine_context.parents function_parameter_list original_body in
  let state_machine_data = compute_state_machine_data
    used_locals
    coroutine_result_data_variables
    function_parameter_list in
  let closure_syntax =
    CoroutineClosureGenerator.generate_coroutine_closure
      context
      original_body
      function_type
      state_machine_data in
  new_body, closure_syntax
