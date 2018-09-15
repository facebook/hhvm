(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

module CoroutineSyntax = Coroutine_syntax
module Syntax = Full_fidelity_editable_positioned_syntax
module CoroutineSuspendRewriter = Coroutine_suspend_rewriter

open Syntax
open CoroutineSyntax
open Coroutine_state_machine_generator

(**
 * To desguar a coroutine, a parameter representing the subsequent continuation
 * is generated.
 *
 * To avoid conflicting with variadic function types, the contination parameter
 * is generated at the beginning rather than the end of the parameter list.
 *)
(* TODO: Consider packaging parameter-list-and-return-type into a "signature"
object *)
let compute_parameter_list function_parameter_list function_type =
  let coroutine_parameter_syntax =
    make_continuation_parameter_syntax function_type in
  prepend_to_comma_delimited_syntax_list
    coroutine_parameter_syntax
    function_parameter_list

let remove_coroutine_modifier_from_modifiers_list modifiers =
  syntax_node_to_list modifiers
  |> Core_list.filter ~f:(fun c -> not (is_coroutine c))
  |> make_list

(**
 * If the provided function declaration header is for a coroutine, rewrites the
 * parameter list and return type as necessary to implement the coroutine.
 *)
let rewrite_function_decl_header header_node =
  let make_syntax node = make_syntax (from_function_declaration_header node) in
  let function_type =
    make_coroutine_result_type_syntax header_node.function_type in
  let function_parameter_list = compute_parameter_list
    header_node.function_parameter_list header_node.function_type in
  let function_modifiers = remove_coroutine_modifier_from_modifiers_list
    header_node.function_modifiers in
  make_syntax
    { header_node with
      function_modifiers;
      function_type;
      function_parameter_list;
    }

let parameter_to_arg param =
  match syntax param with
  | ListItem { list_item; _; } ->
    begin
    match syntax list_item with
    | ParameterDeclaration { parameter_name; _; } ->
      let variable_expression = parameter_name in
      make_syntax (VariableExpression { variable_expression } )
    | _ -> failwith "expected parameter declaration in parameter list"
    end
  | _ -> failwith "expected parameter declaration in parameter list"

let parameter_list_to_arg_list function_parameter_list =
  let function_parameter_list = syntax_node_to_list function_parameter_list in
  List.map parameter_to_arg function_parameter_list

(**
 * One of the following, depending on whether the coroutine method is static or
 * not:
 *
 *   - inst_meth($this, "methodName_GeneratedStateMachine")
 *   - class_meth("ClassName", "methodName_GeneratedStateMachine")
 *)
let make_state_machine_method_reference_syntax
    classish_name
    header_node
    method_node =
  let method_name =
    make_string_literal_syntax (make_state_machine_method_name header_node) in
  if is_static_method method_node then
    make_function_call_expression_syntax
      class_meth_syntax
      [ make_string_literal_syntax classish_name; method_name; ]
  else
    make_function_call_expression_syntax
      inst_meth_syntax
      [ this_syntax; method_name; ]

(**
 * Rewrites a coroutine body to instantiate the closure corresponding to the
 * coroutine, pass in or set any necessary variables, and return the result from
 * invoking resume (with a null argument).
 *)
let create_closure_invocation
     context
     function_parameter_list
     function_type
     rewritten_body =
  (* $outer1, $outer2 *)
  let outer_variables = context.Coroutine_context.outer_variables in
  let outer_variables = SSet.elements outer_variables in
  let outer_args = Core_list.map
    ~f:make_variable_expression_syntax outer_variables in
  (* $param1, $param2 *)
  let arg_list = parameter_list_to_arg_list function_parameter_list in
  (* ($closure, $data, $exception) ==> { body } *)
  let lambda_signature = make_closure_lambda_signature_from_method
    context.Coroutine_context.original_node
    context
    function_type in
  let lambda = make_lambda_from_method_syntax
    context.Coroutine_context.original_node
    lambda_signature
    rewritten_body in
  let classname = make_closure_classname context in
  (* $continuation,
    ($closure, $data, $exception) ==> { body },
    $outer1, $outer2, $param1, $param2 *)
  let parameters =
    continuation_variable_syntax ::
    lambda ::
    (outer_args @ arg_list) in
  (* new Closure($continuation, ...) *)
  let new_closure_syntax = make_object_creation_expression_syntax
    classname parameters in
  (* (new Closure($continuation, ...)) *)
  let new_closure_syntax =
    make_parenthesized_expression_syntax new_closure_syntax in
  (* (new Closure($continuation, ...))->doResumeWithReentryGuard *)
  let select_do_resume_member_syntax =
    make_member_selection_expression_syntax
      new_closure_syntax
      do_resume_with_reentry_guard_member_name_syntax in
  (* (new Closure($continuation, ...))->doResumeWithReentryGuard(null, null) *)
  let call_do_resume_with_null_syntax =
    make_function_call_expression_syntax
      select_do_resume_member_syntax
      [ null_syntax; null_syntax; ] in
  (* return SuspendedCoroutineResult::create(); *)
  let return_statement_syntax =
    make_return_statement_syntax call_do_resume_with_null_syntax in
  [ return_statement_syntax; ]

let rewrite_coroutine_body
    context
    function_parameter_list
    function_type
    rewritten_body =
  if CoroutineSuspendRewriter.only_tail_call_suspends
    context.Coroutine_context.original_node
  then rewritten_body
  else
    match syntax rewritten_body with
    | CompoundStatement node ->
        let compound_statements = create_closure_invocation
          context
          function_parameter_list
          function_type
          rewritten_body in
        let compound_statements = make_list compound_statements in
        make_syntax (CompoundStatement { node with compound_statements })
    | Missing ->
        rewritten_body
    | _ ->
        (* Unexpected or malformed input, so we won't transform the coroutine. *)
        failwith "rewritten_body wasn't a CompoundStatement"

(**
 * If the provided methodish declaration is for a coroutine, rewrites the
 * declaration header and the function body into a desugared coroutine
 * implementation.
 *)
let rewrite_methodish_declaration
    context
    ({ function_parameter_list; function_type; _; } as header_node)
    rewritten_body =
  let methodish_function_body =
    rewrite_coroutine_body
      context
      function_parameter_list
      function_type
      rewritten_body in
  let method_node = context.Coroutine_context.original_node in
  let methodish_declaration_node =
    get_methodish_declaration (syntax method_node) in
  Syntax.synthesize_from
    method_node
    (from_methodish_declaration { methodish_declaration_node with
      methodish_function_decl_header =
        rewrite_function_decl_header header_node;
      methodish_function_body;
    })

let rewrite_function_declaration
    context
    ({ function_type; function_parameter_list; _; } as header_node)
    rewritten_body =
  let function_body = rewrite_coroutine_body
    context
    function_parameter_list
    function_type
    rewritten_body in
  let function_node = context.Coroutine_context.original_node in
  Syntax.synthesize_from
    function_node
    (from_function_declaration
      { (get_function_declaration (syntax function_node)) with
        function_declaration_header = rewrite_function_decl_header header_node;
        function_body;
      })

let rewrite_anon context anon_node =
  (* TODO: redundant to context *)
  let ({ anonymous_parameters; anonymous_type; anonymous_body; _; } as anon) =
    get_anonymous_function (syntax anon_node) in
  let anonymous_body = rewrite_coroutine_body
    context anonymous_parameters anonymous_type anonymous_body in
  let anonymous_parameters = compute_parameter_list
    anonymous_parameters anonymous_type in
  let anonymous_type = make_coroutine_result_type_syntax anonymous_type in
  let anon =
    { anon with
      anonymous_coroutine_keyword = make_missing();
      anonymous_parameters;
      anonymous_type;
      anonymous_body } in
  Syntax.synthesize_from anon_node (from_anonymous_function anon)

let rewrite_lambda
    context
    ({ lambda_parameters; lambda_type; _; } as lambda_signature)
    lambda_node =
  let ({ lambda_body; _; } as lambda) =
    get_lambda_expression (syntax lambda_node) in
  let make_sig node =
    make_syntax (from_lambda_signature node) in
  let lambda_body = rewrite_coroutine_body
    context lambda_parameters lambda_type lambda_body in
  let lambda_parameters = compute_parameter_list
    lambda_parameters lambda_type in
  let lambda_type = make_coroutine_result_type_syntax lambda_type in
  let lambda_signature = make_sig
    { lambda_signature with
      lambda_parameters;
      lambda_type } in
  let lambda =
    { lambda with
      lambda_coroutine = make_missing();
      (* TODO: This loses trivia on the original keyword. *)
      lambda_signature;
      lambda_body
    } in
  Syntax.synthesize_from lambda_node (from_lambda_expression lambda)
