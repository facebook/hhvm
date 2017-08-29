(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *)

module EditableSyntax = Full_fidelity_editable_syntax
module EditableToken = Full_fidelity_editable_token
module CoroutineSyntax = Coroutine_syntax
module Rewriter = Full_fidelity_rewriter.WithSyntax(EditableSyntax)

open EditableSyntax
open CoroutineSyntax

let failwithf fmt = Printf.ksprintf failwith fmt

(* checks if one of node's parents is a try-block of try-statement
   NOTE: this function relies on physical identity of nodes being the same *)
let rec is_in_try_block node parents =
  match parents with
  | [] -> false
  | { syntax = TryStatement { try_compound_statement; _ }; _ } :: _
      when node == try_compound_statement ->
    true
  | x :: xs -> is_in_try_block x xs

(* given a node and a list of its ancestors [p1; p2; p3; ...]
   checks nodes pairwise (node, p1), (p1, p2), (p2, p3) to make sure that
   first node in the pair appear in the tail position within a second node*)
let rec is_in_tail_position node parents =
  match parents with
  | [] -> true
  | { syntax = ParenthesizedExpression {
        parenthesized_expression_expression = e; _
      }; _
    } as n :: xs when node == e ->
    is_in_tail_position n xs
  | { syntax = ConditionalExpression {
        conditional_consequence = consequence;
        conditional_alternative = alternative; _
      }; _
    } as n :: xs when node == consequence || node == alternative ->
    is_in_tail_position n xs
  | { syntax = BinaryExpression {
        binary_operator = {
          syntax = Token {
            EditableToken.kind = TokenKind.QuestionQuestion; _
          }; _
        };
        binary_right_operand = right; _
      }; _
    } as n :: xs when node == right ->
    is_in_tail_position n xs
  | _ -> false

(* Returns pair:
    - n elements from the beginning of the list in reverse order
    - rest of the list
   Raised Failure if length of the list is less than n *)
let split_n_reverse_first_exn l n =
  let rec aux acc i l =
    if i = 0 then acc, l
    else match l with
    | h :: tl -> aux (h :: acc) (i - 1) tl
    | _ -> failwithf "unexpected: list length is less than %d" n
  in
  aux [] n l

(* additional bit of information accumulated for every node when
   rewriting suspends *)
type extra_node_info =
{ (* extra statements that should be executed before the
   evaluation of target node *)
  prefix: EditableSyntax.t list;
  (* node was a suspend that was rewritten into the tail call *)
  is_tail_call: bool
}

let no_tail_call_extra_info prefix =
  { is_tail_call = false; prefix }

let no_info = no_tail_call_extra_info []

let is_no_info { prefix; is_tail_call } =
  Core_list.is_empty prefix && not is_tail_call

(* Describes kind of statement that encloses suspend being rewritten *)
type rewrite_suspend_context =
  | EnclosedByReturnStatement
  | EnclosedByReturnStatementInTailPosition
  | EnclosedByOtherStatement

(* Result of rewriting 'suspend' operator *)
type rewrite_suspend_result =
{ next_label: int
; next_temp: int
(* expression that represents the result of 'suspend' *)
; expression: EditableSyntax.t
(* additional information necessary to initialize expression *)
; extra_info: extra_node_info }

(*
 * Rewrites argument of the suspend operator into the assignment
 * to the coroutineResultData member when execution is resumed
 * after the suspension point.
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
 *)
let rewrite_suspend
  suspend_function_call_expression
  next_label
  next_temp
  in_tail_position =

  let _, _, function_call_argument_list, _ =
    get_function_call_expression_children suspend_function_call_expression in

  if in_tail_position
  then
    (* coroutine is in tail position - can call it and pass continuation
      from the enclosing function *)
    let function_call_argument_list =
      prepend_to_comma_delimited_syntax_list
        continuation_variable_syntax
        function_call_argument_list in

    let invoke_coroutine =
      FunctionCallExpression {
        suspend_function_call_expression with function_call_argument_list
      } in

    let invoke_coroutine_syntax = make_syntax invoke_coroutine in

    { next_label
    ; next_temp
    ; expression = invoke_coroutine_syntax
    ; extra_info = { is_tail_call = true; prefix = [] } }
  else

  let update_next_label_syntax = set_next_label_syntax next_label in

  let function_call_argument_list =
    prepend_to_comma_delimited_syntax_list
      closure_variable_syntax
      function_call_argument_list in
  let invoke_coroutine =
    FunctionCallExpression {
      suspend_function_call_expression with function_call_argument_list
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
    make_closure_result_data_member_name_syntax next_temp in

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

  { next_label = next_label + 1
  ; next_temp = next_temp + 1
  ; expression = coroutine_result_data_variable_syntax
  ; extra_info = { is_tail_call = false; prefix = statements } }

(* checks if specified node might need to be saved in local *)
let rec might_be_spilled node parent
  ~spill_subscript_expressions =
  match syntax node, syntax parent with
  (* do not spill subscript expressions if receiver and index should not
     be spilled when node is an immediate argument for unset.

     NOTE: Normally we should not do this, consider this case:

     f($a[0], suspend y($a[0]++))
     if initial value of $a[0] is not saved then
     value that will be passed as the first argument to f will be
     incremented. *)
  | SubscriptExpression {
      subscript_receiver = r;
      subscript_index = i;
      _ }, _ when spill_subscript_expressions ->
       might_be_spilled r node ~spill_subscript_expressions:false
    || might_be_spilled i node ~spill_subscript_expressions:false

  (* do not spill variables that are receivers in array indexed access
     or objects in member selection expressions :
     $a[...]  *)
  | VariableExpression {
      variable_expression = {
        syntax = Token { EditableToken.kind = TokenKind.Variable; _ }; _ };
      _
    },
    (
      SubscriptExpression { subscript_receiver = r; _  } |
      MemberSelectionExpression { member_object = r; _ }
    ) when node == r
    -> false
  (* spill variables except $this *)
  | VariableExpression {
      variable_expression = {
        syntax = Token { EditableToken.kind = TokenKind.Variable; text; _ }; _ };
      _
    }, _ ->
    text <> "$this"
  (* do not spill tokens, literals or qualified names *)
  | Token _, _
  | LiteralExpression _, _
  | QualifiedNameExpression _, _ ->
    false
  (* do not spill member accesses on $closure variable *)
  | MemberSelectionExpression {
    member_object = {
      syntax = Token { EditableToken.kind = TokenKind.Variable; text; _  };
      _ };
    _ }, _ ->
    text <> closure_variable
  (* do not spill binary expressions where operands don't need spilling *)
  | BinaryExpression {
      binary_left_operand = left;
      binary_right_operand = right;
      _
    }, _ ->
       might_be_spilled left node ~spill_subscript_expressions:false
    || might_be_spilled right node ~spill_subscript_expressions:false
  (* do not spill parenthesised expression if
     inner expression does not need spilling *)
  | ParenthesizedExpression {
      parenthesized_expression_expression = e;
      _
    }, _ ->
    might_be_spilled e node ~spill_subscript_expressions
  (* do not spill list items if inner expression does not need spilling *)
  | ListItem { list_item; _ }, _ ->
    might_be_spilled list_item node ~spill_subscript_expressions

  (* TODO: add more cases *)

  (* spill only non-missing nodes *)
  | _ -> not (is_missing node)

(* store the 'value' in the temporary variable
   value - r-value or list item
   prefix - statements that should be evaluated to initialize the value,
   i.e. if value is a node for the addition operation '$_temp + $a' where
   $_temp is a temporary variable then prefix is the part that computes
  the value of _temp, i.e. $_temp = foo(). *)
let spill_value value prefix next_temp =
  (* unwrap list item *)
  let value, separator_opt =
    match syntax value with
    | ListItem { list_item; list_separator; _ } ->
      list_item, Some list_separator
    | _ ->
      value, None in

  let temp_data_member_selection =
    make_closure_result_data_member_name_syntax next_temp in

  (* $closure->temp = value *)
  let assignment =
    make_assignment_syntax_variable temp_data_member_selection value in

  (* append assignment to temp to the previous prefix *)
  let prefix = prefix @ [assignment] in
  (* if previous value was list item -  make new one list item as well *)
  let new_value =
    match separator_opt with
    | Some list_separator ->
      make_list_item temp_data_member_selection list_separator
    | _ ->
      temp_data_member_selection in

  new_value, prefix, (next_temp + 1)

let make_assignment_or_return_syntax member_selection value extra_info =
  if extra_info.is_tail_call
  then make_return_statement_syntax value
  else make_assignment_syntax_variable member_selection value

(* Replaces short circuit operator with a matching if statement.
   - left - left hand side of the operator
   - left_extra_info - prefix that should be executed before running 'left'
   - right - right hand side of the operator
   - right_extra_info - prefix that should be executed before running 'right'
   - next_temp - counter to generate temporary variables
   - result_is_bool - whether result of short-circuit opertor must be bool
   - f - function that converts left value into the condition that will
      determine whether right hand side should be evaluated

  For example
  logical || operator (e1 || e2)
  should be transformed to
     if (!$temp = boolval(e1)) {
         $temp = boolval(e2);
     }
     NOTE: boolval is necessary since result of
     || operator is always boolean so result_is_bool will be true
  *)
let rewrite_short_circuit_operator
  ~result_is_bool left left_extra_info right right_extra_info next_temp f =

  let temp_data_member_selection =
    make_closure_result_data_member_name_syntax next_temp in

  (* if result of operator is boolean - convert left and right sides to bool *)
  let left =
    if result_is_bool
    then make_function_call_expression_syntax boolval_syntax [ left ]
    else left in

  let right =
    if result_is_bool
    then make_function_call_expression_syntax boolval_syntax [ right ]
    else right in

  (* ($_temp = left) *)
  let left_assignment =
    make_assignment_expression_syntax temp_data_member_selection left
    |> make_parenthesized_expression_syntax in
  (* $_temp = right;
     or
     return right

     the latter one is used in case if right hand side is tail call
     and no result conversion is required *)
  let right_assignment_or_return =
    if result_is_bool
    then make_assignment_syntax_variable temp_data_member_selection right
    else make_assignment_or_return_syntax
      temp_data_member_selection
      right
      right_extra_info in

  (* generate a condition part of an if statement (with left assignment
     being the part of the condition) *)
  let condition = f left_assignment in
  (* Generate if statement where right_init is executed inside the body
     of 'then' branch
     if (condition) {
         .. right_prefix
         $temp = right;
     } *)
  let if_statement =
    make_if_syntax
      condition
      (right_extra_info.prefix @ [right_assignment_or_return]) in
  (* Place left_prefix before the if_statement
    ...left_prefix
     if (condition) {
        ..right_prefix
        $temp = right
     } *)
  let prefix = left_extra_info.prefix @ [if_statement] in
  let extra_info = no_tail_call_extra_info prefix in

  temp_data_member_selection, extra_info, (next_temp + 1)

let get_children_count n =
  match syntax n with
  | Token _ | Missing -> 0
  | _ -> Core_list.length (EditableSyntax.children n)

(* rewrites an expression that contains nested suspends
   by introducing a temporary locals to preserve evaluation order.

   a() + suspend b();

   will become
   $temp1 = a();
   $temp2 = suspend b();
   $temp3 + temp2;

   This function uses the fact that post order traversal of the tree matches
   left to right evaluation order. After visiting a node we also record a fact
   whether node itself or one of its children needs special
   code for initialization (i.e. when expression is replaced with temp we
   need to initialize the temporary variable before it can be used).
   Parent nodes can pick initialization bits for child nodes and recombine
   them as needed:

   conditional operator:
      condition, condition_prefix,
      consequence, consequence_prefix,
      alternative, alternative_prefix

    result node for for conditional operator can be
    some temporary variable '$temp' that will hold the result
    and 'prefix' code that will initialize the temporary variable:

    condition_prefix;
    if (condition) {
      consequence_prefix;
      $temp = consequence;
    }
    else {
      alternative_prefix;
      $temp = alternative;
    } *)
let rewrite_suspends_in_statement
  node context next_label ~is_argument_to_unset =

  (* as we process child nodes prepend extra_info for every child
     to node_extra_info_list. When all children of some node are processed
     it pops all extra info entries from the head of the list (they are
     stored in reverse order) and put extra info entry for itself instead *)
  let rewrite parents node (node_extra_info_list, next_label, next_temp) =
    let children_count = get_children_count node in

    let keep_node () =
      (* pop extra info for child nodes *)
      let _, node_extra_info_rest =
        split_n_reverse_first_exn node_extra_info_list children_count in

      (no_info :: node_extra_info_rest, next_label, next_temp),
      Rewriter.Result.Keep
    in

    match syntax node with
    (* ignore tokens and missing nodes *)
    | Token _ | Missing -> keep_node ()
    (* convert applications of 'suspend' operators *)
    | PrefixUnaryExpression {
        prefix_unary_operator = {
          syntax = Token { EditableToken.kind = TokenKind.Suspend; _; };
          _ };
        prefix_unary_operand = {
          syntax = FunctionCallExpression function_call_expression;
          _ };
      _ } ->

      let node_extra_info_list, node_extra_info_rest =
        split_n_reverse_first_exn node_extra_info_list children_count in

      let operand_prefix =
        match node_extra_info_list with
        | [_; { prefix; _ }] -> prefix
        | _ -> assert false in

      let is_tail_call =
           context = EnclosedByReturnStatementInTailPosition
        && is_in_tail_position node parents in

      let { next_label; next_temp; expression = new_node; extra_info } =
        rewrite_suspend
          function_call_expression
          next_label
          next_temp
          is_tail_call in

      let extra_info =
        { extra_info with prefix = operand_prefix @ extra_info.prefix } in

      (extra_info :: node_extra_info_rest, next_label, next_temp),
      Rewriter.Result.Replace new_node

    | _ ->
      let node_extra_info_list, node_extra_info_rest =
        split_n_reverse_first_exn node_extra_info_list children_count in

      if Core_list.for_all node_extra_info_list ~f:is_no_info
      then
        keep_node ()
      else

      match syntax node with
      | ParenthesizedExpression _ ->
        let extra_info =
          match node_extra_info_list with
          | [_; i; _] -> i
          | _ -> assert false in

        (* passthrough extra info from the inner expression *)
        (extra_info :: node_extra_info_rest, next_label, next_temp),
        Rewriter.Result.Keep
      | ConditionalExpression {
          conditional_test = test;
          conditional_consequence = consequence;
          conditional_alternative = alternative; _
        } ->
        let test_extra_info, consequence_extra_info, alternative_extra_info =
          match node_extra_info_list with
          | [t; _; c; _; a] -> t, c, a
          | _ -> failwith "unexpected"
        in
        if    is_no_info consequence_extra_info
           && is_no_info alternative_extra_info
        then
          (* simple case, both consequence and alternative parts don't have any
             prefixes - rewrite as
             .. rest_prefix
             test ? consequence : alternative *)
          let new_node =
            make_conditional_expression_syntax test consequence alternative in

          (* reset is_tail_call value on extra info bit for test node *)
          let test_extra_info =
            { test_extra_info with is_tail_call = false } in

          (test_extra_info :: node_extra_info_rest, next_label, next_temp),
          Rewriter.Result.Replace new_node
        else
          (* either consequence or alternative has prefix - rewrite as if/else
             ...test_prefix
             if (test) {
                ...consequence_prefix
                $temp = consequence
             }
             else {
                ...alternative_prefix
                $temp = alternative
             }
             $temp
             NOTE: if conditional expression is immediately nested
             in return statement or consequence/alternate are emitted
             as tail calls then temp local can be omitted:
             ... test_prefix
             if (test) {
               .. consequence_prefix;
               return consequence
             }
             else {
               ..alternate_prefix;
               return alternative;
             }
          *)
          let temp_data_member_selection =
            make_closure_result_data_member_name_syntax next_temp in

          let is_top_level_in_return =
               context <> EnclosedByOtherStatement
            && Core_list.is_empty parents in

          let assign_to_temp_or_return value value_extra_info =
            if is_missing value
            then make_missing ()
            else if is_top_level_in_return
            then make_return_statement_syntax value
            else
              make_assignment_or_return_syntax
                temp_data_member_selection
                value
                value_extra_info in

          let consequence_assignment_or_return =
            assign_to_temp_or_return consequence consequence_extra_info in

          let alternative_assignment_or_return =
            assign_to_temp_or_return alternative alternative_extra_info in

          let then_block =
              consequence_extra_info.prefix
            @ [ consequence_assignment_or_return ] in
          let else_block =
              alternative_extra_info.prefix
            @ [ alternative_assignment_or_return ] in
          let if_statement =
            make_if_else_syntax test then_block else_block in

          let prefix = test_extra_info.prefix @ [ if_statement ] in
          let extra_info = no_tail_call_extra_info prefix in

          let next_temp, new_node =
            let consequence_used_temporary =
                 not (is_missing consequence_assignment_or_return)
              && not (is_return_statement consequence_assignment_or_return) in
            let alternative_used_temporary =
                 not (is_missing alternative_assignment_or_return)
              && not (is_return_statement alternative_assignment_or_return) in

            if consequence_used_temporary || alternative_used_temporary
            then next_temp + 1, temp_data_member_selection
            else next_temp, make_missing () in

          ( extra_info ::node_extra_info_rest, next_label, next_temp),
          Rewriter.Result.Replace new_node

      (* convert short-circuit binary operators *)
      | BinaryExpression {
          binary_left_operand = left;
          binary_operator = {
              syntax = Token {
                EditableToken.kind = (
                  TokenKind.BarBar |
                  TokenKind.AmpersandAmpersand |
                  TokenKind.QuestionQuestion |
                  TokenKind.And |
                  TokenKind.Or) as t;
                _ };
              _ } ;
          binary_right_operand = right;
          _ } ->

        let left_extra_info, right_extra_info =
          match node_extra_info_list with
          | [l; _; r] -> l, r
          | _ -> assert false in

        (* if right hand side does not have prefix - no need to transform
            the node into the if statement, just keep prefix
            of the left hand side *)
        if is_no_info right_extra_info
        then
          (left_extra_info :: node_extra_info_rest, next_label, next_temp),
          Rewriter.Keep
        else
          begin match t with
          | TokenKind.BarBar
          | TokenKind.Or ->
            (* transform e1 || e2 to
               if (!$temp = boolval(e1)) {
                   $temp = boolval(e2);
               }
               NOTE: boolval is necessary since result of
               || operator is always boolean *)
            let new_node, extra_info, next_temp =
              rewrite_short_circuit_operator
                ~result_is_bool:true
                left
                left_extra_info
                right
                right_extra_info
                next_temp
                make_unary_not_syntax in
            (extra_info :: node_extra_info_rest, next_label, next_temp),
            Rewriter.Result.Replace new_node

          | TokenKind.AmpersandAmpersand
          | TokenKind.And ->
            (* transform e1 && e2 to
               if ($temp = boolval(e1)) {
                 $temp = boolval(e2);
               }
               NOTE: boolval is necessary since result of
               || operator is always boolean *)
             let new_node, extra_info, next_temp =
               rewrite_short_circuit_operator
                ~result_is_bool:true
                 left
                 left_extra_info
                 right
                 right_extra_info
                 next_temp
                 (fun x -> x) in
             (extra_info :: node_extra_info_rest, next_label, next_temp),
             Rewriter.Result.Replace new_node
          | TokenKind.QuestionQuestion ->
            (*transform e1 ?? e2 to
              if (($temp = e1) === null) {
                $temp = e2
              } *)
            let new_node, extra_info, next_temp =
              rewrite_short_circuit_operator
                ~result_is_bool:false
                left
                left_extra_info
                right
                right_extra_info
                next_temp
                make_is_null_syntax in
            (extra_info :: node_extra_info_rest, next_label, next_temp),
            Rewriter.Result.Replace new_node
          | _ -> failwith "impossible"
          end

      | BinaryExpression {
          binary_left_operand = left;
          binary_operator = {
              syntax = Token {
                EditableToken.kind = (
                  TokenKind.Equal |
                  TokenKind.PlusEqual |
                  TokenKind.MinusEqual |
                  TokenKind.SlashEqual |
                  TokenKind.StarEqual |
                  TokenKind.DotEqual);
                _ };
              _ } as t;
          binary_right_operand = right;
          _ } ->

        let left_extra_info, right_extra_info =
          match node_extra_info_list with
          | [l; _; r] -> l, r
          | _ -> assert false in

        let prefix, left, next_temp =
          if is_no_info right_extra_info
          then
            (* right side does not have prefix,
               so we can just pick left prefix *)
            left_extra_info.prefix,
            left,
            next_temp
          else
            (* right side does have prefix -
               make sure that all child nodes on left hand side are spilled

               $a[f()] = suspend b();

               left hand side does not have prefix but to preserve
               the evaluation order call to f() should be stored to local

               $temp1 = f();
               $temp2 = suspend b();
               $a[$temp1] = $temp2;

               NOTE: if left hand side is a variable it should not be spilled *)
            if is_variable_expression left
            then
                left_extra_info.prefix @ right_extra_info.prefix,
                left,
                next_temp
            else
              let next_temp, children =
                EditableSyntax.children left
                |> Core_list.fold_left
                  ~init:(next_temp, [])
                  ~f:(fun (next_temp, acc) n ->
                    let n, prefix, next_temp =
                      if might_be_spilled n left ~spill_subscript_expressions:false
                      then
                        spill_value n [] next_temp
                      else
                        n, [], next_temp in
                    next_temp, (prefix, n) :: acc) in

              let prefixes, children =
                children
                |> Core_list.rev
                |> Core_list.unzip in

              (* for the sequence of child nodes c1 c2 c3 c4 c5
                 if i.e. c3 has a suspend in it - we'll spill all nodes
                 to its left meaning that we there are any nodes that are
                 not yet spilled but they should - they are on the right of c3
                 so prefixes that we extract for them should be placed after
                 prefixes that already were found for the left hand side *)
              let prefix =
                  left_extra_info.prefix
                @ Core_list.concat prefixes
                @ right_extra_info.prefix in

              let left =
                EditableSyntax.from_children (EditableSyntax.kind left) children in

              prefix, left, next_temp
          in

        let extra_info =
          no_tail_call_extra_info prefix in
        let new_node = make_binary_expression left t right in

        (extra_info :: node_extra_info_rest, next_label, next_temp),
        Rewriter.Result.Replace new_node

      | _ ->

        let spill_subscript_expressions =
             is_argument_to_unset
          && Core_list.is_empty parents in

        let spill_expr
          (child_node, { prefix; _ })
          (acc, has_prefix_on_the_right, next_temp) =

          let result, has_prefix_on_the_right, next_temp =
            let child_node_has_prefix = not (Core_list.is_empty prefix) in
            if might_be_spilled child_node node ~spill_subscript_expressions
            then
              (* if there is a suspend somewhere on the right of this child node
                 save it content in the temp
                 i.e. f(a(),  suspend b()); should become
                 $temp1 = a();
                 $temp2 = suspend b();
                 f($temp1, $temp2);
                 *)
              if has_prefix_on_the_right
              then
                let child_node, spill, next_temp =
                  spill_value child_node prefix next_temp in
                (child_node, spill),
                true,
                next_temp
              else
                (child_node, prefix),
                child_node_has_prefix,
                next_temp
            else
              (child_node, prefix),
              has_prefix_on_the_right || child_node_has_prefix,
              next_temp
          in
          result :: acc, has_prefix_on_the_right, next_temp
        in

        (* get a list of chidren for a current node *)
        let children = EditableSyntax.children node in

        (* walk child nodes from right to left and check if we need to introduce
           any extra spills:
           foo(f(), suspend b())
               ^    ^
               1    2
           (1) does not contain suspends so it should not be spilled per se
           however (2) contains a suspend so in order to preserve left to right
           evaluation order we need to save to result of f() in a temp
           *)
        let result, has_at_least_one_prefix, next_temp =
          Core_list.zip_exn children node_extra_info_list
          |> Core_list.fold_right ~init:([], false, next_temp) ~f:spill_expr in
        if has_at_least_one_prefix
        then
          (* create new list of child nodes *)
          let nodes, prefixes = Core_list.unzip result in

          let new_node =
            EditableSyntax.from_children (EditableSyntax.kind node) nodes in

          let extra_info =
            no_tail_call_extra_info (Core_list.concat prefixes) in

          ( extra_info :: node_extra_info_rest, next_label, next_temp),
          Rewriter.Result.Replace new_node
        else
          keep_node ()
  in
  Rewriter.parented_aggregating_rewrite_post rewrite node ([], next_label, 1)

let is_in_lambda_or_anonymous_function node ancestors =
  Core_list.exists ancestors
    ~f:(fun node -> is_lambda_expression node || is_anonymous_function node)

let rewrite_suspends_in_non_return_context
  node next_label temp_count f ~is_argument_to_unset =
  let (extra_node_info_list, next_label, next_temp), node =
    rewrite_suspends_in_statement
      node
      EnclosedByOtherStatement
      next_label
      ~is_argument_to_unset in

  if next_temp = 0
  then
    (next_label, temp_count), Rewriter.Result.Keep
  else
    let { prefix; _ } = List.hd extra_node_info_list in

    (* TODO: (t17335630) Generate unset call to release all intermediate data
       stored in $closure->temp_data_member*
       Result value should be stored in temporary local variable prior
       to making unset.

       let unset_call =
         Core_list.range 0 next_temp
         |> Core_list.map ~f:make_closure_temp_data_member_name_syntax
         |> (make_function_call_statement_syntax unset_syntax)
     *)
    let statements = prefix @ [f node] in
    let statements = make_compound_statement_syntax statements in

    (next_label, max next_temp temp_count), Rewriter.Result.Replace statements

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
  let rewrite ancestors node ((next_label, temp_count) as acc) =
    if is_in_lambda_or_anonymous_function node ancestors
    then
      acc, Rewriter.Result.Keep
    else
      begin match syntax node with
      | ReturnStatement { return_expression; _ } ->
        let prefix, return_expression, is_tail_call, next_label, temp_count =
          if is_missing return_expression
          then
            [], coroutine_unit_call_syntax, false, next_label, temp_count
          else
            let context =
              if not (is_in_try_block node ancestors)
              then EnclosedByReturnStatementInTailPosition
              else EnclosedByReturnStatement in

            let (extra_node_info_list, next_label, next_temp),
                return_expression =
              rewrite_suspends_in_statement
                return_expression
                context
                next_label
                ~is_argument_to_unset:false in

            let extra_node_info = List.hd extra_node_info_list in

            extra_node_info.prefix,
            return_expression,
            extra_node_info.is_tail_call,
            next_label,
            max temp_count next_temp
        in
        if is_missing return_expression
        then
          let statements = make_compound_statement_syntax prefix in

          (next_label, temp_count), Rewriter.Result.Replace statements

        else if is_tail_call
        then
          let return_statement =
            make_return_statement_syntax return_expression in
          let statements =
            let statements = prefix @ [return_statement] in
            make_compound_statement_syntax statements in

          (next_label, temp_count), Rewriter.Result.Replace statements

        else
          let return_expression = make_object_creation_expression_syntax
            "ActualCoroutineResult" [return_expression] in
          let assignment = set_next_label_syntax (-1) in
          let ret = make_return_statement_syntax return_expression in
          let statements = prefix @ [ assignment; ret ] in
          let statements = make_compound_statement_syntax statements in

          (next_label, temp_count), Rewriter.Result.Replace statements

      | IfStatement ({ if_condition; _ } as node) ->
        rewrite_suspends_in_non_return_context
          if_condition
          next_label
          temp_count
          ~is_argument_to_unset:false
          (fun if_condition ->
            make_if_statement_syntax { node with if_condition })

      | ExpressionStatement { expression_statement_expression; _ } ->
        rewrite_suspends_in_non_return_context
          expression_statement_expression
          next_label
          temp_count
          ~is_argument_to_unset:false
          make_expression_statement_syntax

      | SwitchStatement ({ switch_expression; _ } as node) ->
        rewrite_suspends_in_non_return_context
          switch_expression
          next_label
          temp_count
          ~is_argument_to_unset:false
          (fun switch_expression ->
            make_switch_statement_syntax { node with switch_expression })

      | ThrowStatement { throw_expression; _ } ->
        rewrite_suspends_in_non_return_context
          throw_expression
          next_label
          temp_count
          ~is_argument_to_unset:false
          make_throw_statement_syntax

      | ForeachStatement ({ foreach_collection; _ } as node) ->
        rewrite_suspends_in_non_return_context
          foreach_collection
          next_label
          temp_count
          ~is_argument_to_unset:false
          (fun foreach_collection ->
            make_foreach_statement_syntax { node with foreach_collection })

      | EchoStatement ({ echo_expressions; _ } as node) ->
        rewrite_suspends_in_non_return_context
          echo_expressions
          next_label
          temp_count
          ~is_argument_to_unset:false
          (fun echo_expressions ->
            make_echo_statement_syntax { node with echo_expressions })

      | UnsetStatement ({ unset_variables; _ } as node) ->
        rewrite_suspends_in_non_return_context
          unset_variables
          next_label
          temp_count
          ~is_argument_to_unset:true
          (fun unset_variables ->
            make_unset_statement_syntax { node with unset_variables })

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
        acc, Rewriter.Result.Keep
      end
  in
  Rewriter.parented_aggregating_rewrite_post rewrite node (1, 1)
