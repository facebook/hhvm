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
module SuspendRewriter = Coroutine_suspend_rewriter

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
  (*TODO: if there are no used locals - just return the body ? *)
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

let get_token node =
  match EditableSyntax.get_token node with
  | Some token -> token
  | None -> failwith "expected a token"

(* Transforms
   if ($a) f1()
   elseif ($b) f2()
   elseif ($c) f3()
   ...
   elseif ($z) f26()
   else otherwise ()

   to
   if ($a) f1()
   else {
     if ($b) f2()
    else {
      if ($c) f3()
      else {
       ...
        if ($z) f26()
        else otherwise()
      }
    }
   } *)
let rec rewrite_if_statement if_stmt =
  let { if_condition; if_elseif_clauses; if_else_clause; _; } = if_stmt in
  match syntax_node_to_list if_elseif_clauses with
  | [] ->
    (* no elseif blocks - keep if statement *)
    make_if_statement_syntax if_stmt
  | h :: t ->

  (* We have, say

  if (x) a;
  elseif(y) b;
  elseif (z) c;
  else d;

  The recursion here is slightly tricky. We know that we can reduce the first
  "elseif" to an "else if". That is, the code above is equivalent to:

  if (x) a;
  else {
    if (y) b;
    elseif (z) c;
    else d;
  }

  So the first thing we do is construct the interior "if". It has one fewer
  "elseif" than before, so we've made a smaller problem that we can solve
  recursively. We recursively rewrite it, and then that becomes the "else"
  of the outer "if". *)
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
      let child_if = rewrite_if_statement child_if in
      let new_else_clause = make_else_clause else_keyword_syntax child_if in
      let new_if = { if_stmt with
        if_elseif_clauses = make_missing();
        if_else_clause = new_else_clause
      } in
      make_if_statement_syntax new_if
    | _ ->
      failwith "Malformed elseif clause"
    end

(* Replaces elseif blocks of if statements with
   nested if statements *)
let rewrite_if node =
  let rewrite node =
    match syntax node with
    | IfStatement if_stmt ->
      Rewriter.Result.Replace (rewrite_if_statement if_stmt)
    | _ ->
      Rewriter.Result.Keep
  in
  Rewriter.rewrite_post rewrite node

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
  if is_missing body then (body, []) else
  let used_locals = all_used_locals body in
  let body = add_missing_return body in
  let (next_loop_label, body) = rewrite_do 0 body in
  let body = rewrite_while body in
  let (next_loop_label, body) = rewrite_for next_loop_label body in
  let body = rewrite_if body in
  let (next_loop_label, temp_count), body =
    SuspendRewriter.rewrite_suspends body in
  let body = add_switch (next_loop_label, body) in
  let used_locals = SSet.elements used_locals in
  let body = add_try_finally used_locals body in
  let body = unnest_compound_statements body in
  let coroutine_result_data_variables =
    temp_count
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
